/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_DEF_H
#define __I2C_DEF_H

#define MOTOR1                  UserMotorSm1
#define MOTOR2                  slavePtr->Motor[0]
#define MOTOR3                  slavePtr->Motor[1]
#define MOTOR4                  slavePtr->Motor[2]
#define MOTOR5                  slavePtr->Motor[3]
#define MOTOR6                  slavePtr->Motor[4]
#define MOTOR7                  slavePtr->Motor[5]
#define MOTOR8                  slavePtr->Motor[6]
#define MOTOR9                  slavePtr->Motor[7]
#define MOTOR10                 UserMotorSm2

#define I2C_GEN_CALL_ADR    0x00
#define I2C_TIMEOUT         0x3FFF   //Yields ~1ms tmo
#define MAX_DEVICES         20
#define SUPERUSER_OFS       60

#define REG_READ_VER        0x0   //Version
#define REG_READ_VDR        0x8   //Vender
#define REG_READ_DID        0x10  //Device ID
#define REG_READ_INFO       0x20

#define REG_READ_SPEED      0x3E  //0x3E-0x3F Signed Speed
#define REG_READ_TICS       0x40  //0x40-0x45 used
#define REG_READ_RSPEED     0x44  //0x44-0x45 Regular Speed
#define REG_READ_UTICS      0x46  //0x46-0x47 used

#define REG_RESET_TICS      0x4A
#define REG_NEXT_DEV        0x4B
#define REG_TERM_DEV        0x4C
#define REG_CHANGE_ADR      0x4D
#define REG_RESET_DEV       0x4E
#define REG_WR_ADR_EE       0x4F  //Write Address, Terminator to EE
#define REG_CLEAR_ADR       0x50  //Clear EE area
#define REG_CHG_FILTER      0x51  //Change Filter

#define REG_EXIT_SU         0x5D   
#define REG_WR_COLOR        0x5E   
#define REG_WR_DATA_EE      0x5F  //write scratch pad to EE
#define REG_READ_DATA       0x60  //64 bytes of read area
#define REG_WRITE_DATA      0xA0  //64 bytes of write area

#define NO_ERROR            0x00   
#define NO_DEV_FOUND        0x01  //No device found on I2C bus
#define ADDRESS_ERROR       0x02  //Change address not working
#define TERM_FAILURE        0x03  //Terminator switch not working
#define TIC_READ_ERROR      0x04  //Error when reading ticks
#define TIC_RANGE_ERROR     0x05  //Error reading tic value
#define DIAGMODE_ERROR      0x06  //Unable to put in diag mode

#define CAPTURE_BUFR_SIZE   256   //Interrupt debug capture size

#define WRITE_RESET_COUNTERS      0
#define WRITE_DEV_DIAG            1
#define WRITE_DEV_COLOR           2
#define WRITE_DEV_EXIT            3

typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

/*! \struct ImeData
 * \brief Struct for passing counter speed & data
 */
typedef struct   
{ 
  s32 counter;            
  u16 speed;    
} ImeData;

void Initialize_I2c_device_table(void);
void Print_Table(void);
void I2C_Function_Handler(void);
void Int_General_Call_Reset(void);
u8 PrintAllDevices(u8 func);
void WriteAllDevices(u8 func);
void Int_Search_For_Devices(void);
u8 CheckForInactiveDevice(u8 id);
void Send_IME_Tics(u8 addr, u8 *bufr);
void Send_Diag_Results(u8 errcode);

// FIXME Internal functions
void ReadOneDeviceFast(u8 id);
#endif
