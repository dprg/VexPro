/******************** (C) COPYRIGHT 2011 IFI ********************
 * File Name: i2c_int_utilities.cpp
 * Purpose: Vex Slave template code
 * Version: V1.00
 *
 * These routines provide support for I2C devices.  The i2cBlock must be filled
 * out correctly before calling Execute_Command.  See i2c_def.h for fields.
 *
 *----------------------------------------------------------------------------*/

#include <string.h>
#include "stdio.h"
#include "qetime.h"
#include "i2c_def.h"
#include "libI2C.h"

I2C_Record i2cBlock;

Dev_Record devTable[MAX_DEVICES];
u8 activeDeviceIndex = 0;
u8 totalDevicesActive = 0;
u8 debugFlags[2] = {0x02,0x03};
u8 colorRay[6]   = {0,2,0,3,0,1};
u8 devAddress = I2C_START_ADR;
//u32 I2CErrors[14] = {I2C_IT_SB,I2C_IT_ADDR,I2C_IT_BTF,I2C_IT_ADD10,I2C_IT_STOPF,
//                   I2C_IT_RXNE,I2C_IT_TXE,I2C_IT_BERR,I2C_IT_ARLO,I2C_IT_AF,
//                   I2C_IT_OVR, I2C_IT_PECERR,I2C_IT_TIMEOUT,I2C_IT_SMBALERT};

/*  debugFlags[1];  
		Bit0 = Init I2C Table
		Bit1 = Print I2C Table
		Bit2 = Invoke diagMode
		Bit3 = Get Tics for GUI
    Bit4 = Stop Diag Mode or Get Tic mode
*/

CQETime timer;

#ifdef __GNUC__
#define MAX_RETRY     16000
#else
#define MAX_RETRY     16000
#endif

static void waitForBufrReady(void)
{
  u32 retry;
  u8 Done = 0;

  retry = 0;
  Done = i2cBlock.readBufrReady; 
  while (!Done)
  {
    if ((i2cBlock.status) || (i2cBlock.readBufrReady)) 
    {
      Done = 1; 
    } 
    retry++;
    if (retry > MAX_RETRY) Done = 1;
  }
  //PORTD_Toggle(1);  //Dig12
  if (retry > MAX_RETRY) 
  {
    printf("wait TMO\r\n");
    timer.msleep(2);
  }
  //printf("retry %d\r\n",retry);
}

static void waitForWriteComplete(void)
{
  u32 retry;
  u8 Done = 0;

  retry = 0;
  Done = i2cBlock.writeComplete;
  while (!Done)
  {
    if ((i2cBlock.status) || (i2cBlock.writeComplete)) 
    {
      Done = 1; 
    } 
    retry++;
    if (retry > MAX_RETRY/2) Done = 1;
  }
  //PORTD_Toggle(1);  //Dig12
  //printf("retry %d\r\n",retry);
}

// WARNING: THIS FUNCTION INITIALIZES THE I2C MODULE, WHICH SHOULD ONLY BE DONE ONCE
void Initialize_I2c_device_table(void)
{
  u8 addr,i;

  addr = I2C_START_ADR;
  for (i=0;i<MAX_DEVICES;i++)
  {
    devTable[i].addr = addr;  
    devTable[i].registered = 0;
    devTable[i].active = 0;
    devTable[i].terminated = 1;
    devTable[i].retry = 0;
    addr += 2;
  }
  totalDevicesActive = 0;
}

void Print_Table(void)
{
  int i;
  printf("Current Dev Address %x\r\n",devAddress);
  for (i=0;i<10;i++)
  {
    printf("idx %d adr %x reg %d act %d term %d\r\n",i,devTable[i].addr,
      devTable[i].registered,devTable[i].active,devTable[i].terminated);
  }
}

static void Execute_Command(void)
{
	bool rv;
	timer.usleep(100);  //add a 100us delay
	i2cBlock.readBufrReady = 0;
	i2cBlock.writeComplete = 0;
	i2cBlock.status = 0;
	i2cBlock.writeIndex = 0;
	i2cBlock.readIndex = 0;

	// Issue the start sequence
	if (i2cBlock.bytesToWrite > 0) {
		rv = I2CStart(i2cBlock.deviceAddr/2, I2C_WRITE);
		if (!rv) {
			I2CStop();
			i2cBlock.status--;
			return;
		}
		// write the register number & any command bytes
		for (; i2cBlock.writeIndex<i2cBlock.bytesToWrite; i2cBlock.writeIndex++) {
			rv = I2CWriteByte(i2cBlock.writeBufr[i2cBlock.writeIndex]);
			if (!rv) {
				I2CStop();
				i2cBlock.status--;
				return;
			}
		}
		i2cBlock.writeComplete = 1;
		if (i2cBlock.bytesToRead == 0) {	// if this was a write operation return
			I2CStop();
			return;
		}
	}

	if (i2cBlock.bytesToRead > 0) {
		// Issue the restart sequence
		rv = I2CStart(i2cBlock.deviceAddr/2, I2C_READ);
		if (!rv) {
			I2CStop();
			i2cBlock.status--;
			return;
		}

		int i2cDone = I2C_READ;
		// read the requested number of bytes (if any). readIndex is 0 for a write operation
		for (; i2cBlock.readIndex<i2cBlock.bytesToRead; i2cBlock.readIndex++) {
			if (i2cBlock.readIndex == (i2cBlock.bytesToRead-1))
				i2cDone = I2C_DONE;
			i2cBlock.readBufr[i2cBlock.readIndex] = I2CReadByte(i2cDone);
		}
		I2CStop();
		i2cBlock.readBufrReady = 1;

	}
}

static void Int_Reset_Counters(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_RESET_TICS;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

static void Int_Change_Filter(u8 address,u8 value)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 2;
  i2cBlock.writeBufr[0] = REG_CHG_FILTER;
  i2cBlock.writeBufr[1] = value;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

static void Int_GetData(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_TICS;
  i2cBlock.bytesToRead = 6;
  Execute_Command();
}

static void Int_GetData_ext(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_UTICS;
  i2cBlock.bytesToRead = 2;
  Execute_Command();
}

static void Int_Read_Regular_Speed(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_RSPEED;
  i2cBlock.bytesToRead = 2;
  Execute_Command();
}

static void Int_Read_Signed_Speed(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_SPEED;
  i2cBlock.bytesToRead = 3;
  Execute_Command();
}

static void Int_GetDeviceInfo(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_INFO;
  i2cBlock.bytesToRead = 9;
  Execute_Command();
}

static void Int_WriteRegister(u8 address, u8 regValue, u8 bytesToWrite)
{
  u8 i;
  //printf("dev address %x\r\n",address);
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 1 + bytesToWrite;
  i2cBlock.writeBufr[0] = REG_WRITE_DATA;
  for (i=0;i<bytesToWrite;i++)
    i2cBlock.writeBufr[i+1] = regValue+i;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

static void Int_ReadRegister(u8 address, u8 bytesToRead)
{
  //printf("dev address %x\r\n",address);
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_DATA;
  i2cBlock.bytesToRead = bytesToRead;
  Execute_Command();
}

static void Int_ChangeAddress(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 2;
  i2cBlock.writeBufr[0] = REG_CHANGE_ADR;
  i2cBlock.writeBufr[1] = devAddress;
  //printf("new address %x\r\n",devAddress);
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

void Int_General_Call_Reset(void)
{
  devAddress = I2C_START_ADR;
  i2cBlock.deviceAddr = I2C_GEN_CALL_ADR;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 3;
  i2cBlock.writeBufr[0] = REG_RESET_DEV;
  i2cBlock.writeBufr[1] = 0xCA;
  i2cBlock.writeBufr[2] = 0x03;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

static void Int_PropagateClock(u8 address,u8 Command)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = Command;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

static void Int_PutInDiagMode(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 5;
  i2cBlock.writeBufr[0] = REG_WRITE_DATA + SUPERUSER_OFS;
  i2cBlock.writeBufr[1] = 0xDE;
  i2cBlock.writeBufr[2] = 0xAD;
  i2cBlock.writeBufr[3] = 0xFA;
  i2cBlock.writeBufr[4] = 0xCE;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

static void Int_ChangeColor(u8 address,u8 color)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 2;
  i2cBlock.writeBufr[0] = REG_WR_COLOR;
  i2cBlock.writeBufr[1] = color;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

static void Int_ExitDiagMode(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_EXIT_SU;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

static void Int_CheckDevice(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 0;
  i2cBlock.bytesToRead = 1;
  Execute_Command();
}

void Int_Search_For_Devices(void)
{
  u8 NoMoreDevices = 1;
  u8 retry = 0;
  u8 i;

  if (devTable[0].registered) //If device have already been initialized reset them
  {
    for (i=0;i<activeDeviceIndex+1;i++)
    {
      devTable[i].registered = 0;
      devTable[i].active = 0;
      devTable[i].terminated = 0;
    }
    Int_General_Call_Reset();
    while ((!i2cBlock.writeComplete) && (retry < 5))
    {
      timer.msleep(1); //Give some time for device to reset
      retry++;
    }
    if (!i2cBlock.writeComplete) 
    {
      printf("Reset Device Failure\r\n");
      return;
    }
    timer.msleep(1); //Give some time for device to reset
  }

  devAddress = I2C_START_ADR;
  activeDeviceIndex = 0;
  while (NoMoreDevices)
  {
    NoMoreDevices = 0;
    Int_CheckDevice(I2C_BOOT_ADR);
    waitForBufrReady();
    if (i2cBlock.readBufrReady)
    {
      Int_ChangeAddress(I2C_BOOT_ADR);
      waitForWriteComplete();
      timer.msleep(300); //Give some time for device to change address  (300ms)
      if (i2cBlock.writeComplete)
      {
        Int_PropagateClock(devAddress,REG_NEXT_DEV);  
        waitForWriteComplete();
        if (i2cBlock.writeComplete)
        {
          devTable[activeDeviceIndex].registered = 1;
          devTable[activeDeviceIndex].active = 1;
          devTable[activeDeviceIndex].terminated = 0;
          activeDeviceIndex++;
          devAddress += 2;
          NoMoreDevices = 1;
        }
      }
    }
    else
    {
      if (!i2cBlock.readBufrReady)
        printf("Terminating I2c at device %d\r\n",activeDeviceIndex);
    }
  }
  //PORTD_Toggle(1);  //Dig12
  if (devAddress != I2C_START_ADR)
  {
    devAddress -= 2;
    Int_CheckDevice(devAddress);
    waitForBufrReady();
    if (!i2cBlock.readBufrReady) 
    {
      printf("Check Failure\r\n");
      return;  //Failure
    }
    Int_PropagateClock(devAddress,REG_TERM_DEV);  
    waitForWriteComplete();
    if (activeDeviceIndex) activeDeviceIndex--;
    devTable[activeDeviceIndex].terminated = 1;

  }

  if (devTable[0].registered) 
    totalDevicesActive = activeDeviceIndex+1;
  printf("Devices Found %d last address %x Terminated %d\r\n",
    totalDevicesActive,devAddress,i2cBlock.writeComplete);
  i2cBlock.writeComplete = 0;
  devAddress = I2C_START_ADR;
}

u8 CheckForInactiveDevice(u8 index)
{
  static u8 tableAddress = 1;
  u8 addr;
  u8 modedTerminator = 0;
  u8 found = 0;


  if (index) //Must not be 1st device in chain 
  {
    if (devTable[index-1].terminated)
    {
      Int_PropagateClock(devTable[index-1].addr,REG_NEXT_DEV);
      timer.msleep(2); //Give some time for device to ropagateClock
      modedTerminator = 1;
    }
  }
  if (tableAddress)
    addr = devTable[index].addr;
  else
    addr = I2C_BOOT_ADR;
  //printf("cfid %x\r\n",addr);
  //Delay(5); //Give some time to printf  
  Int_CheckDevice(addr);
  timer.msleep(2); //Give some time to CheckDevice
  if (i2cBlock.readBufrReady) 
  {
    if (!tableAddress)
    {
      devAddress = devTable[index].addr;
      Int_ChangeAddress(I2C_BOOT_ADR);
      timer.msleep(2); //Give some time for device to change address
      if (i2cBlock.writeComplete)
      {
        Int_PropagateClock(devAddress,REG_TERM_DEV);  
        timer.msleep(2); //Give some time for device to ropagateClock
        if (i2cBlock.writeComplete)
        {
          printf("Found Dev %x\r\n",devAddress);
          devTable[index].terminated = 1;
          devTable[index].active = 1;
          found = 1;
        }
      }
    }
    if (index) //Must not be 1st device in chain
    {
      devTable[index-1].terminated = 0;
    }
  }

  if ((index) && (modedTerminator)) //Revert to terminator?
  {
    if (devTable[index-1].terminated)
    {
      Int_PropagateClock(devTable[index-1].addr,REG_TERM_DEV);
      timer.msleep(2); //Give some time for device to ropagateClock
    }
  }
  tableAddress ^= 1;
  i2cBlock.writeComplete = 0;
  return found;
}

void ReadOneDeviceFast(u8 device)
{
  u8 cnt;
  u8 errorIndex = 0;
  ImeData motorpos;
  u8 *ptr;
  static u8 printfThrottle = 0;

  cnt = 0;
  while (cnt < 20)
  {
    if (devTable[device].active)
    {
      Int_GetData(devTable[device].addr);
      waitForBufrReady();
      if (i2cBlock.readBufrReady)
      {
        i2cBlock.readBufrReady = 0;
        ptr = (u8 *)&motorpos.counter;
        *ptr++ = i2cBlock.readBufr[1];
        *ptr++ = i2cBlock.readBufr[0];
        *ptr++ = i2cBlock.readBufr[3];
        *ptr++ = i2cBlock.readBufr[2];
        motorpos.speed = (u16)(i2cBlock.readBufr[4] << 8);
        motorpos.speed |= i2cBlock.readBufr[5];
        //if ((devTable[i].clockwise == 0) && (motorpos.counter)) 
        //  motorpos.counter = ~motorpos.counter;
        //if ((motorpos.counter != 0) && (printfThrottle == 0))
          printf("%x %08x:%4x\r\n",devTable[device].addr,motorpos.counter,motorpos.speed);
      }
      else
      {
        devTable[device].active = 0;
        if (errorIndex == 0) errorIndex = device+1; //fudge to save the 1st index
        printf("TMO addr %x status %d\r\n",devTable[device].addr,i2cBlock.status);
      }
    }
    cnt++;
  }
  printfThrottle = (printfThrottle + 1) % 20;
}

u8 PrintAllDevices(u8 func)
{
  u8 i,j,*ptr;
  u8 retSuccessful = 1;
  ImeData motorpos;
  u16 uspeed;
  short speed;
  static u8 printfThrottle = 0;
  static u8 devIndex = 0;

  for (i=0;i<totalDevicesActive;i++)
  //i = devIndex;
  {
    if (devTable[i].active)
    {
      switch (func)
      {
      case READ_DEV_TICS:
        Int_GetData(devTable[i].addr);
        break;
      case READ_DEV_INFO:
        Int_GetDeviceInfo(devTable[i].addr);
        break;
      case READ_DEV_DATA:
        Int_ReadRegister(devTable[i].addr,8);
        break;
      case READ_DEV_STATUS:
        Int_CheckDevice(devTable[i].addr);
        break;
      case READ_DEV_UTICS:
        Int_GetData_ext(devTable[i].addr);
        break;
      case REG_READ_RSPEED:
        Int_Read_Regular_Speed(devTable[i].addr);
        break;
      case REG_READ_SPEED:
        Int_Read_Signed_Speed(devTable[i].addr);
        break;
      }
      waitForBufrReady();
      if (i2cBlock.readBufrReady)
      {
        i2cBlock.readBufrReady = 0;
        switch (func)
        {
        case READ_DEV_TICS:
          ptr = (u8 *)&motorpos.counter;
          *ptr++ = i2cBlock.readBufr[1];
          *ptr++ = i2cBlock.readBufr[0];
          *ptr++ = i2cBlock.readBufr[3];
          *ptr++ = i2cBlock.readBufr[2];
          motorpos.speed = (u16)(i2cBlock.readBufr[4] << 8);
          motorpos.speed |= i2cBlock.readBufr[5];
          //if ((motorpos.counter & 0xFFFF) && (printfThrottle == 0))
            printf("%x %08x:%4x\t",devTable[i].addr,motorpos.counter,motorpos.speed);
          break;
        case READ_DEV_INFO:
          if (printfThrottle == 0)
            printf("V%d T%d Id%d S%x %x %x %x %x %x\t",
              i2cBlock.readBufr[0],i2cBlock.readBufr[1],i2cBlock.readBufr[2],
              i2cBlock.readBufr[3],i2cBlock.readBufr[4],i2cBlock.readBufr[5],
              i2cBlock.readBufr[6],i2cBlock.readBufr[7],i2cBlock.readBufr[8]);
          break;
        case READ_DEV_DATA:
          for (j=0;j<i2cBlock.readIndex;j++)
            printf("%02x ",i2cBlock.readBufr[j]);
          printf("\t");
          break;
        case READ_DEV_UTICS:
          if (printfThrottle == 0)
            printf("%x %x%x\t",devTable[i].addr,i2cBlock.readBufr[0],i2cBlock.readBufr[1]);
          break;
        case REG_READ_RSPEED:
          uspeed = i2cBlock.readBufr[0] << 8 | i2cBlock.readBufr[1];
          if (printfThrottle == 0)
            printf("%x %d\t",devTable[i].addr,uspeed);
          break;
        case REG_READ_SPEED:
          speed = i2cBlock.readBufr[0] << 8 | i2cBlock.readBufr[1];
          if (printfThrottle & 1)
            printf("%x %d\t",devTable[i].addr,speed);
          break;
        default:
          for (j=0;j<i2cBlock.readIndex;j++)
            printf("%02x ",i2cBlock.readBufr[j]);
          printf("\t");
          break;
        }
        devTable[i].retry = 0;
        devIndex = (devIndex + 1) % totalDevicesActive;
      }
      else
      {
        //devTable[i].retry++;
        timer.msleep(1);
        devTable[i].active = 0;
        if (i == 0)
          devIndex = 0; 
        else
          devIndex--; 
        printf("TMO @ %x sts %d ret %d\r\n",devTable[i].addr,i2cBlock.status,devTable[i].retry);
        retSuccessful = 0;
      }
    }
    else  //Device went inactive so look for it again
    {
      devTable[i].retry++;
      if (devTable[i].retry > 10)
      {
        devTable[i].retry = 0;
        //printf("Reset\r\n");
        //I2C_IO_Reset();
        //DeviceState = 1;
        retSuccessful = 0;
      } 
      else
      {
        retSuccessful = CheckForInactiveDevice(i);
      }    
      devIndex = 0; //Start from beginning on next call
    }
  }
  printf("\n");
  printfThrottle = (printfThrottle + 1) % 20;
  return retSuccessful;
}

u8 ColorRay[] = {0,1,0,3};

void WriteAllDevices(u8 func)
{
  static u8 printfThrottle = 0;
  static u8 diagColor = 0;
  static u8 colorDelay = 0;
  u8 i;
  u8 errorIndex = 0;

  for (i=0;i<totalDevicesActive;i++)
  {
    if (devTable[i].active)
    {
      switch (func)
      {
      case WRITE_RESET_COUNTERS:
        Int_Reset_Counters(devTable[i].addr);
        break;
      case WRITE_DEV_DIAG:
        Int_PutInDiagMode(devTable[i].addr);
        break;
      case WRITE_DEV_COLOR:
        Int_ChangeColor(devTable[i].addr,ColorRay[diagColor]);
        break;
      case WRITE_DEV_EXIT:
        Int_ExitDiagMode(devTable[i].addr);
        break;
      }
      waitForWriteComplete();
      if (i2cBlock.writeComplete)
      {
        i2cBlock.writeComplete = 0;
        //if (printfThrottle == 0)
      }
      else
      {
        devTable[i].active = 0;
        if (errorIndex == 0) errorIndex = i+1; //fudge to save the 1st index
        printf("TMO addr %x status %d\r\n",devTable[i].addr,i2cBlock.status);
      }
    }
  }
  colorDelay = (colorDelay + 1) % 10;
  if (colorDelay == 0) diagColor = (diagColor + 1) % 4;
  printfThrottle = (printfThrottle + 1) % 20;
  if (errorIndex)
  {
    if (errorIndex) errorIndex--;  //subtract off fudge
    if (errorIndex) errorIndex--;  //Terminate valid device
    Int_PropagateClock(devTable[errorIndex].addr,REG_TERM_DEV);  
    devTable[errorIndex].terminated = 1;
  }
}



