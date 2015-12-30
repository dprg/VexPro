/*
 * CQEIMEncoder.cpp
 *
 *  Created on: Oct 12, 2012
 *      Author: bouchier
 */
/*! \file CQEIMEncoder.cpp
 * \brief The encoder management file
 * <PRE>
 The following is a description of LED patterns:
 ==============================================

 Yellow - not initialized (default Address)

 Every 3 sec
   Slow Green Blip - initialized and encoder is not changing
   Slow Green Double Blip - initialized, neutral and terminated
   Slow Green Micro Blip - initialized, device is idle

 Blinking Green - relative to speed
 Solid Green - full speed in either direction

 Yellow (same as Green patterns) - counter overflow

 Solid Red - data is being written to EEprom (1sec)
 Blinking Red - address range error (a valid address will clear error)

 Velocity Bits:
 =============
 </PRE>
 The "velocity bits" are really delta-time per encoder gear revolution, so 1/speed.
 The units are in 64 microsecond tics per encoder revolution for the 269 IME and 64
 microsecond tics per encoder half-revolution for the 393 IME. The values increase as
 the rotation rate slows, with stopped for more than 4 seconds being reported as 0xFFFF.
 The values are only Magnitude and do not include a Sign bit for direction.
 Doing sequential Rotation reads can get you direction.

 For the 269 IME, the encoder wheel turns 30.056 times per output revolution.
 To get velocity bits into rpm, do the following. Multiple the value (in tics / encoder
 rev) times 0.000064 (seconds / tic) to get a value in (seconds / encoder rev). Take the
 reciprocal to get a value in (encoder rev / second). Multiply by 60 (seconds / minute)
 and 1/30.056 (output rev / encoder rev) to get your value in (output rev / minute).

 For the 393 IME with the factory (torque) gearing, the encoder wheel turns 39.2 times per
 output revolution. To get velocity bits into rpm, do the following. Multiply the value
 (in tics / half-encoder rev) times 0.000064 (seconds / tic) and times 2 (half-encoder
 rev / encoder-rev) to get a value in (seconds / encoder rev). Take the reciprocal to get
 a value in (encoder rev / second). Multiply by 60 (seconds / minute) and 1/39.2 (output
 rev / encoder rev) to get your value in (output rev / minute).

 For the 393 IME with the speed gearing, the encoder wheel turns 24.5 times per output
 revolution. To get velocity bits into rpm, do the following. Multiple the value (in tics
 / half-encoder rev) times 0.000064 (seconds / tic) and times 2 (half-encoder rev / encoder-rev)
 to get a value in (seconds / encoder rev). Take the reciprocal to get a value in (encoder rev
 / second). Multiply by 60 (seconds / minute) and 1/24.5 (output rev / encoder rev) to get
 your value in (output rev / minute).

<PRE>
 Tics
 ====
 </PRE>
 There are four each black to white and white to black transitions of the encoder.
 Each tic represents a b->w or w->b transition at one of the sensors, meaning 16 tics/encoder rev.
 To get count to rotations, divide tics by (16 * encoder gearing)
 */

#include <string.h>
#include "stdio.h"
#include "CQEI2C.h"
#include "i2c_def.h"
#include "CQEIMEncoder.h"
#include "qetime.h"

//#define MAX_RETRY     16000
#define TICS_PER_ENCODER_REV 16

CQETime timer;
unsigned char CQEIMEncoder::activeDeviceIndex = 0;
unsigned char CQEIMEncoder::totalDevicesActive = 0;
unsigned char CQEIMEncoder::devAddress = I2C_START_ADR;

/*! Instantiate an encoder object, which manages one encoder.
 *
 * After instantiation, the caller should
 * call initNextDevice() & check for true return; this will grab the next uninitialized encoder
 * and enumerate it. It is then ready for use.
 *
 * Thereafter, call readEncoder() to get the count & speed out of the encoder, then get the value
 * in preferred units using e.g. getDistance(), getRevPerSec(), etc.
 *
 * @param i2cRef A reference to the I2C object that runs the bus
 * @param mTypeIn The motor type, should be one of motor269, motor393Torque (default), motor393Speed
 * @param ccwFwd Set true if counter-clockwise shaft rotation should result in increasing counts
 * and positive speed
 * @param scaleIn A value used to scale the number of rotations or rotations/sec to inches or
 * other unit of measurement. Normally 2 * PI * wheel radius in inches to get distance in inches,
 * speed in ips.
 */
CQEIMEncoder::CQEIMEncoder(CQEI2C& i2cRef, TmotorType mTypeIn, bool ccwFwdIn,
		float scaleIn) : i2c(i2cRef)
{
	motorType = mTypeIn;
	ccwFwd = ccwFwdIn;
	countScale = scaleIn;

	rawCount = lastRawCount = 0;
	switch (motorType) {
	case motor269: gearRatio = 30.056; break;
	case motor393Torque: gearRatio = 39.2; break;		// factory default
	case motor393Speed: gearRatio = 24.5; break;
	}
	degreeScale = 360.0 / (16.0 * gearRatio);			//Ratio to turn counts into degrees - 16 counts/enc wheel rev

	totalDevicesActive = 0;
	devAddress = I2C_START_ADR;
}

CQEIMEncoder::~CQEIMEncoder() {
}

//! Initialize the next encoder down the I2C bus
/*!
 * Instantiate this object then call initNextDevice(), and repeat for each encoder that should
 * be on the bus. Initialization leaves the encoder count at zero.
 *
 * \return True if another encoder was found & initialized, false otherwise
 */
bool CQEIMEncoder::initNextDevice(void)
{
	registered = 0;
	active = 0;
	terminated = 0;

	// initialize the I2C bus for the encoders if it needs it
	if (!i2c.I2CBusScan(I2C_START_ADR/2, I2C_BOOT_ADR/2, true))	// if no encoders respond
		i2c.I2CInit();				// initialize i2c

	// check if the encoder is already enumerated & mark as enumerated & return if so
	if (checkDevice(devAddress)) {
		printf("Encoder at 0x%x found already enumerated\n", devAddress);
		registered = 1;
		active = 1;
		terminated = 1;
		addr = devAddress;
		if (!clearEncoder())			// clear the encoder
			printf("ERROR: clearEncoder failed during init\n");
		activeDeviceIndex++;
		devAddress += 2;
		return true;
	}
	// unterminate & pass through I2C from the previous device, except if this is the first device
	if (devAddress != I2C_START_ADR)
	{
		if (!checkDevice(devAddress-2))
		{
			printf("Check Failure at address 0x%x\r\n", devAddress-2);
			return false;  //Failure
		}

		printf("Propagating I2c at address 0x%x\r\n",devAddress-2);
		Int_PropagateClock(devAddress-2,REG_NEXT_DEV);
		timer.msleep(500); //Give some time for device to propagateClock

	} else {	// this is the first device we're trying to enumerate, so reset all devices
		//printf("Issuing a general call reset to all devices\n");
		Int_General_Call_Reset();
		timer.msleep(500);
	}

	//i2c.I2CBusScan(I2C_START_ADR/2, I2C_BOOT_ADR/2);

	checkDevice(I2C_BOOT_ADR);	// check if there's an encoder at the boot address
	timer.msleep(1);			// don't know why I have to do it twice, but the 1st time often fails
	if (checkDevice(I2C_BOOT_ADR))		// if yes
	{
		printf("Found an encoder at the boot address - changing its address\n");
		Int_ChangeAddress(I2C_BOOT_ADR);	// change its address to current devAddress
		timer.msleep(1000); //Give some time for device to change address  (300ms)
		if (i2cBlock.writeComplete)
		{
			//printf("Terminating I2c at address 0x%x\r\n",devAddress);
			Int_PropagateClock(devAddress,REG_TERM_DEV);
			timer.msleep(50);
			//printf("checkDevice gives 0x%x\n", checkDevice(devAddress));

			if (i2cBlock.writeComplete)
			{
				registered = 1;
				active = 1;
				terminated = 1;
				addr = devAddress;
				if (!clearEncoder())			// clear the encoder
					printf("ERROR: clearEncoder failed during init\n");
				activeDeviceIndex++;
				devAddress += 2;
			} else {
				printf("Failed to terminate device at new address %d\n", devAddress);
			}
		} else {
			printf("Failed to change device address from I2C_BOOT_ADR\n");
		}
	}
	else	// failed to find a device when trying to initialize one
	{
		printf("ERROR: checkDevice on I2C_BOOT_ADR failed\n");

		// do a diagnostic scan to see if its anywhere out there
		i2c.I2CBusScan(I2C_START_ADR/2, I2C_BOOT_ADR/2);

		// set up the terminator on the previous device if there is one
		if (devAddress != I2C_START_ADR)	// there's been at least one device discovered
		{
			devAddress -= 2;
			Int_CheckDevice(devAddress);
			if (!i2cBlock.readBufrReady)
			{
				printf("Check Failure\r\n");
				return false;  //Failure
			}
			printf("Terminating I2c at address 0x%x owing to failed to find next device in chain\r\n",devAddress);
			Int_PropagateClock(devAddress,REG_TERM_DEV);
			if (activeDeviceIndex) activeDeviceIndex--;
			terminated = 1;
		}
		return false;
	}
	//printf("address: %x active: %d terminated: %d\r\n",addr,active,terminated);

	return true;
}

//! Read encoder count & speed into object, ready to get from the object in different units
/*!
 * This method must be called to pull the count & speed out of the encoder into the object.
 * Then a method that returns count or speed in appropriate units should be called.
 *
 * \return true if encoder was read successfully, false otherwise
 */
bool CQEIMEncoder::readEncoder()
{
	u8 *ptr;
	bool rv;

	//printf("Reading encoder at 0x%x\n", addr);
	if (active)
	{
		Int_GetData(addr);
		if (i2cBlock.readBufrReady)
		{
			//printf("Got encoder data from 0x%x\n", addr);
			i2cBlock.readBufrReady = 0;

			// get the encoder tics & convert to signed delta from program start
			lastRawCount = rawCount;		// save away the last count for computing speed direction
			ptr = (u8 *)&rawCount;
			*ptr++ = i2cBlock.readBufr[1];
			*ptr++ = i2cBlock.readBufr[0];
			*ptr++ = i2cBlock.readBufr[3];
			*ptr++ = i2cBlock.readBufr[2];

			// get encoder raw speed & adjust for intended forward direction
			rawSpeed = (u16)(i2cBlock.readBufr[4] << 8);
			rawSpeed |= i2cBlock.readBufr[5];
			// On 2-wire motors, CW rotation viewed looking at shaft gives -ve counts
			if (!ccwFwd) {
				rawCount = ~rawCount + 1;		// 2s-complement: 0-rawCount
			}
			count = signedDiff(rawCount, 0);
			//printf("%x %08x:%4x\r\n",addr,rawCount,rawSpeed);

			rv = true;
		}
		else
		{
			active = 0;
			printf("TMO addr %x status %d\r\n",addr,i2cBlock.status);
			rv = false;
		}
	} else
		rv = false;
	return rv;
}

void CQEIMEncoder::Execute_Command(void)
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
		rv = i2c.I2CStart(i2cBlock.deviceAddr/2, I2C_WRITE);
		if (!rv) {
			i2c.I2CStop();
			i2cBlock.status = 1;
			return;
		}
		// write the register number & any command bytes
		for (; i2cBlock.writeIndex<i2cBlock.bytesToWrite; i2cBlock.writeIndex++) {
			rv = i2c.I2CWriteByte(i2cBlock.writeBufr[i2cBlock.writeIndex]);
			if (!rv) {
				i2c.I2CStop();
				i2cBlock.status = 2;
				return;
			}
		}
		i2cBlock.writeComplete = 1;
		if (i2cBlock.bytesToRead == 0) {	// if this was a write operation return
			i2c.I2CStop();
			return;
		}
	}

	if (i2cBlock.bytesToRead > 0) {
		// Issue the restart sequence
		rv = i2c.I2CStart(i2cBlock.deviceAddr/2, I2C_READ);
		if (!rv) {
			i2c.I2CStop();
			i2cBlock.status = 3;
			return;
		}

		int i2cDone = I2C_READ;
		// read the requested number of bytes (if any). readIndex is 0 for a write operation
		for (; i2cBlock.readIndex<i2cBlock.bytesToRead; i2cBlock.readIndex++) {
			if (i2cBlock.readIndex == (i2cBlock.bytesToRead-1))
				i2cDone = I2C_DONE;
			i2cBlock.readBufr[i2cBlock.readIndex] = i2c.I2CReadByte(i2cDone);
		}
		i2c.I2CStop();
		i2cBlock.readBufrReady = 1;

	}
}
//! Get raw count & speed in tics & period counts from object
/*!
 * This method reads the encoder using the readEncoder() method then returns the results.
 * It is different from the other get methods, which just use the last read value to compute
 * their result. This is a test-method, not envisioned for real use.
 *
 * \param countRef A reference to an int into which the count will be placed
 * \param speedRef A reference to a short into which the raw encoder tic period will be placed
 * \return True if
 */
bool CQEIMEncoder::getRawCountSpeed(unsigned int& countRef, short& speedRef)
{
	bool rv = readEncoder();
	countRef = rawCount;
	speedRef = rawSpeed;
	return rv;
}

/*
 * Subtract unsigned encoders to come up with a signed difference that handles overflow. Test
 * cases in testSignedDiff(). Uses the special properties of subtracting unsigned numbers & casting
 * the result to signed - deep magic but it works.
 */
int CQEIMEncoder::signedDiff(unsigned int val, unsigned int lastval)
{
	return (int)(val - lastval);
}
//! Get revs/sec in forward direction.
/*!
 * The first reading may have the wrong speed, but thereafter
 * it uses the count to know what direction speed is in. Positive speed corresponds to positive encoder
 * increments.
 *
 * @return Revs per second
 */
float CQEIMEncoder::getRevPerSec()
{
	float rps;

	if (rawSpeed == -1)	// if stopped
		return 0.0;

	rps = 1.0 / (rawSpeed * 2.0	// measured tics per enc-rev
			* 0.000064 			// 64 usec/tic
			* gearRatio);			// enc-revs/shaft-rev

	// handle wrapping.
	if (signedDiff(rawCount, lastRawCount) < 0)
		rps = 0.0 - rps;		// if motion is backward

	return rps;
}

//! Get radians/sec in forward direction.
/*!
 * The first reading may have the wrong speed, but thereafter
 * it uses the count to know what direction speed is in. Positive speed corresponds to positive encoder
 * increments.
 *
 * @return Radians per second
 */
float CQEIMEncoder::getRadPerSec()
{
	return getRevPerSec() * 2.0 * PI;
}

//! Get speed in units determined by countScale.
/*!
 * Default is 1.0, meaning it returns rotations/sec.
 * Normally, countScale should be set to 2 * PI * wheel_radius so that, for example,
 * a radius in inches results in a speed in inches/sec
 *
 * @return speed in computed units per sec
 */
float CQEIMEncoder::getSpeed()
{
	return getRevPerSec() * countScale;
}

//! Get distance in units determined by countScale.
/*!
 * Default is 1.0, meaning it returns tics.
 * Normally, countScale should be set to 2 * PI * wheel_radius so that, for example,
 * a radius in inches results in a distance in inches
 *
 * @return distance in computed units
 */
float CQEIMEncoder::getDistance()
{
	float rotations, distance;

	rotations = count / (TICS_PER_ENCODER_REV * gearRatio);
	distance = rotations * countScale;
	return distance;
}

//! Get servo position in degrees
/*!
 * \return Offset in degrees from the point it was last zeroed
 */
int CQEIMEncoder::getDegrees()
{
	int degrees;

	degrees = (int)(count * degreeScale);
	return degrees;
}

//! Print selected data from encoder
/*!
 * Diagnostic function to print data from encoder. Requested data is selected by input function
 * parameter.
 * \param func The data that should be retrieved & printed. Valid values are:
 * - READ_DEV_TICS
 * - READ_DEV_INFO
 * - READ_DEV_DATA
 * - READ_DEV_STATUS
 * - READ_DEV_UTICS
 * - REG_READ_RSPEED
 * - REG_READ_SPEED
 */

bool CQEIMEncoder::clearEncoder()
{
  i2cBlock.deviceAddr = addr;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_RESET_TICS;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
  if (i2cBlock.writeComplete == 0)
	  return false;
  lastRawCount = 0;
  rawCount = 0;
  count = 0;
  rawSpeed = 0xffff;
  return true;
}

void CQEIMEncoder::Int_Change_Filter(u8 address,u8 value)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 2;
  i2cBlock.writeBufr[0] = REG_CHG_FILTER;
  i2cBlock.writeBufr[1] = value;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

void CQEIMEncoder::Int_GetData(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_TICS;
  i2cBlock.bytesToRead = 6;
  Execute_Command();
}

void CQEIMEncoder::Int_GetData_ext(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_UTICS;
  i2cBlock.bytesToRead = 2;
  Execute_Command();
}

void CQEIMEncoder::Int_Read_Regular_Speed(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_RSPEED;
  i2cBlock.bytesToRead = 2;
  Execute_Command();
}

void CQEIMEncoder::Int_Read_Signed_Speed(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_SPEED;
  i2cBlock.bytesToRead = 3;
  Execute_Command();
}

void CQEIMEncoder::Int_GetDeviceInfo(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_INFO;
  i2cBlock.bytesToRead = 9;
  Execute_Command();
}

void CQEIMEncoder::Int_WriteRegister(u8 address, u8 regValue, u8 bytesToWrite)
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

void CQEIMEncoder::Int_ReadRegister(u8 address, u8 bytesToRead)
{
  //printf("dev address %x\r\n",address);
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_READ_DATA;
  i2cBlock.bytesToRead = bytesToRead;
  Execute_Command();
}

void CQEIMEncoder::Int_ChangeAddress(u8 address)
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

void CQEIMEncoder::Int_General_Call_Reset(void)
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

void CQEIMEncoder::Int_PropagateClock(u8 address,u8 Command)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = Command;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

void CQEIMEncoder::Int_PutInDiagMode(u8 address)
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

void CQEIMEncoder::Int_ChangeColor(u8 address,u8 color)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 2;
  i2cBlock.writeBufr[0] = REG_WR_COLOR;
  i2cBlock.writeBufr[1] = color;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

void CQEIMEncoder::Int_ExitDiagMode(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_WRITE;
  i2cBlock.bytesToWrite = 1;
  i2cBlock.writeBufr[0] = REG_EXIT_SU;
  i2cBlock.bytesToRead = 0;
  Execute_Command();
}

/*
 * Read a byte from an unspecified register. The result is not used - only the fact that something
 * was read back.
 */
void CQEIMEncoder::Int_CheckDevice(u8 address)
{
  i2cBlock.deviceAddr = address;
  i2cBlock.direction = I2C_READ;
  i2cBlock.bytesToWrite = 0;
  i2cBlock.bytesToRead = 1;
  Execute_Command();
}

/*
 * A different way of checking: send its address in a write start operation
 */
bool CQEIMEncoder::checkDevice(u8 address)
{
	bool rv;
	rv = i2c.I2CStart(address/2, I2C_WRITE);
	i2c.I2CStop();
	return rv;
}

u8 CQEIMEncoder::printDevice(u8 func)
{
  u8 i,j,*ptr;
  u8 retSuccessful = 1;
  ImeData motorpos;
  u16 uspeed;
  short speed;
  static u8 printfThrottle = 0;
  static u8 devIndex = 0;

    if (active)
    {
      switch (func)
      {
      case READ_DEV_TICS:
        Int_GetData(addr);
        break;
      case READ_DEV_INFO:
        Int_GetDeviceInfo(addr);
        break;
      case READ_DEV_DATA:
        Int_ReadRegister(addr,8);
        break;
      case READ_DEV_STATUS:
        Int_CheckDevice(addr);
        break;
      case READ_DEV_UTICS:
        Int_GetData_ext(addr);
        break;
      case REG_READ_RSPEED:
        Int_Read_Regular_Speed(addr);
        break;
      case REG_READ_SPEED:
        Int_Read_Signed_Speed(addr);
        break;
      }
      //waitForBufrReady();
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
            printf("address 0x%x count: 0x%08x speed: 0x%4x\t",addr,motorpos.counter,motorpos.speed);
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
            printf("%x %x%x\t",addr,i2cBlock.readBufr[0],i2cBlock.readBufr[1]);
          break;
        case REG_READ_RSPEED:
          uspeed = i2cBlock.readBufr[0] << 8 | i2cBlock.readBufr[1];
          if (printfThrottle == 0)
            printf("%x %d\t",addr,uspeed);
          break;
        case REG_READ_SPEED:
          speed = i2cBlock.readBufr[0] << 8 | i2cBlock.readBufr[1];
          if (printfThrottle & 1)
            printf("%x %d\t",addr,speed);
          break;
        default:
          for (j=0;j<i2cBlock.readIndex;j++)
            printf("%02x ",i2cBlock.readBufr[j]);
          printf("\t");
          break;
        }
        retry = 0;
      }
      else
      {
        //retry++;
        timer.msleep(1);
        active = 0;
        if (i == 0)
          devIndex = 0;
        else
          devIndex--;
        printf("TMO @ %x sts %d ret %d\r\n",addr,i2cBlock.status,retry);
        retSuccessful = 0;
      }
    }
  printf("\n");
  printfThrottle = (printfThrottle + 1) % 20;
  return retSuccessful;
}



u8 ColorRay[] = {0,1,0,3};

void CQEIMEncoder::WriteDevice(u8 func)
{
  static u8 printfThrottle = 0;
  static u8 diagColor = 0;
  static u8 colorDelay = 0;
  u8 i;

  for (i=0;i<totalDevicesActive;i++)
  {
    if (active)
    {
      switch (func)
      {
      case WRITE_RESET_COUNTERS:
        clearEncoder();
        break;
      case WRITE_DEV_DIAG:
        Int_PutInDiagMode(addr);
        break;
      case WRITE_DEV_COLOR:
        Int_ChangeColor(addr,ColorRay[diagColor]);
        break;
      case WRITE_DEV_EXIT:
        Int_ExitDiagMode(addr);
        break;
      }
      if (i2cBlock.writeComplete)
      {
        i2cBlock.writeComplete = 0;
        //if (printfThrottle == 0)
      }
      else
      {
        active = 0;
        printf("TMO addr %x status %d\r\n",addr,i2cBlock.status);
      }
    }
  }
  colorDelay = (colorDelay + 1) % 10;
  if (colorDelay == 0) diagColor = (diagColor + 1) % 4;
  printfThrottle = (printfThrottle + 1) % 20;
}

ubyte_t CQEIMEncoder::getDeviceAddr()
{
	return addr;
}

void CQEIMEncoder::testSignedDiff()
{
	int diff;
	unsigned int testArray[] =
		{4, 1,
		0xfffffffe, 0xfffffff0,
		1, 3,
		0xfffffff8, 0xfffffffc,
		5, 0xfffffffe,
		0xfffffffe, 2
	};
	int numTests = sizeof(testArray)/8;

	for (int i=0; i<numTests; i++) {
		diff = signedDiff(testArray[2*i], testArray[2*i+1]);
		printf("0x%x - 0x%x = %d\n", testArray[2*i], testArray[2*i+1], diff);
	}
}

bool CQEIMEncoder::test(int testNum)
{
	bool rv = true;

	switch(testNum) {
	case 0: testSignedDiff(); break;
	default: printf("Invalid test number\n");
	}

	return rv;
}
