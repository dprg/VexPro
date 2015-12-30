/*
 * CQEIMEncoder.h
 *
 *  Created on: Oct 12, 2012
 *      Author: bouchier
 *
 * This code is provided under the terms of the
 * GNU General Public License v2 (http://www.gnu.org/licenses/gpl-2.0.html).
 *
 */
/*! \file CQEIMEncoder.h
 * \brief Header file for CQEIMEncoder - the class which manages the Integrated motor encoder
 *
 * <H1>
 * Build Configuration
 * </H1>
 *
 * This project depends on the following peer projects being at the same directory level:
 * - CQEI2C
 * - qetime
 *
 * Edit the project properties as follows to reference them as includes, link objects, and referenced projects.
 *
 * Under C/C++ General -> Paths & Symbols, on the Includes tab, select GNU C++ and add the following paths
 * to the terkos paths that are already there. This adds them to the include path for compilation
 * - ../../CQEI2C
 * - ../../qetime
 *
 * Under C/C++ Build -> Settings, Tool Settings tab, TerkOS C++ Linker group, Miscellaneous settings, add
 * the following "other objects". This tells the linker to link to qetime.o & CQEI2C.o.
 * - ../../CQEI2C/Debug/CQEI2C.o
 * - ../../qetime/Debug/qetime.o
 *
 * In the Project References group, check the following projects. This builds them before the current project.
 * CQEI2C
 * qetime
 *
 */

#ifndef CQEIMENCODERS_H_
#define CQEIMENCODERS_H_

#define PI 3.14159265359

#define I2C_BOOT_ADR        0x60
#define I2C_START_ADR       0x20

// parameters for printDevice
#define READ_DEV_TICS       0
#define READ_DEV_INFO       1
#define READ_DEV_DATA       2
#define READ_DEV_STATUS     3
#define READ_DEV_UTICS      4

typedef unsigned char ubyte_t;

/*! \class CQEIMEncoder
 * \brief Handle the enumeration of IMEs on the I2C bus, and provide read/clear/test facilities.
 *
 * To instantiate this class:
 * \code
 * CQEIMEncoder IMEsDriver = CQEIMEncoder(CQEI2C& i2c);
 * \endcode
 * The user should have separately instantiated an I2C controller object, which may be used for
 * other purposes as well. This class uses the I2C reference to drive the bus and handle access to
 * encoders.
 */
class CQEIMEncoder {
public:
	/*! \var typedef enum TmotorType
	 * \brief Motor type that this encoder is mounted on
	 *
	 * Valid values are motor269, motor393Torque, motor393Speed.
	 * motor393Speed is the 393 motor with the gears changed out to make it the high speed config
	 */
	typedef enum {
		motor269,
		motor393Torque,		// factory default
		motor393Speed
	} TmotorType;

	CQEIMEncoder(CQEI2C& i2cRef, TmotorType mTypeIn, bool ccwFwdIn=true, float scaleIn=1.0);
	virtual ~CQEIMEncoder();

	// Methods to get encoder reading in various ways. The caller is responsible for calling
	// readEncoder() first, to get the current encoder values into the object for subsequent
	// manipulation
	bool readEncoder();	// read 4-bytes of count & 2 of speed & save in object
	float getDistance();		// get absolute distance reading
	float getSpeed();			// get linear velocity
	float getRadPerSec();		// get angular velocity
	float getRevPerSec();		// get angular velocity
	int getDegrees();			// get angular position
	bool getRawCountSpeed(unsigned int &count, short &speed);	// read 4-bytes of count & 2 of speed & pass to caller

	bool clearEncoder();	// clear the count
	bool initNextDevice(void);		// returns true if device found & initialized

	// utility methods
	ubyte_t printDevice(ubyte_t func);			// print imeRecord data for this device
	bool test(int testNum);		// run a requested test
	void WriteDevice(ubyte_t func);
	ubyte_t getDeviceAddr();

    static ubyte_t activeDeviceIndex;
	static ubyte_t totalDevicesActive;
	static ubyte_t devAddress;


private:
	// parameters & values for this encoder
    unsigned int rawCount; 		// count read from encoder
    unsigned int lastRawCount;
    int count;					// signed version of rawCount
    short rawSpeed; 			// current motor speed
    float countScale; 			// scale factor to turn count into units like inches & inches/sec. 1.0 for no scale
    float degreeScale;			// scale to apply to raw count to turn it into degrees
    TmotorType motorType;
    bool ccwFwd;				// true if counter-clockwise rotation viewing shaft should result in increasing count, +ve speed
    ubyte_t  addr;				// I2C address of encoder
    ubyte_t  registered;		// encoder has been enumerated
    ubyte_t  active;			// encoder is marked active
    ubyte_t  terminated;		// this encoder terminates the I2C bus
    ubyte_t  retry;				// count of retry attempts in case of error
    float gearRatio;			// number of encoder revolutions per output shaft revolution

	CQEI2C &i2c;				// reference to the I2C comms object

	typedef struct
	{
	  ubyte_t  deviceAddr;       // the "8-bit" address of device. Divided by 2 when passed to libI2C
	  bool  direction;        // set to I2C_READ or I2C_WRITE from libI2C.h
	  ubyte_t bytesToWrite;     //# of bytes to write
	  int writeIndex;       //write buffer index
	  ubyte_t writeBufr[256];
	  ubyte_t writeComplete;    //write has completed
	  int bytesToRead;      //# of bytes to read
	  ubyte_t readIndex;        //read buffer index
	  ubyte_t readBufr[256];
	  ubyte_t readBufrReady;
	  ubyte_t status;           //1=Ok, 0=Timeout Error
	} I2C_Record;

	I2C_Record i2cBlock;// The I2C command struct

	void Initialize_I2c_device_table(void);
	void Execute_Command(void);
	void Int_Change_Filter(ubyte_t address,ubyte_t value);
	void Int_GetData(ubyte_t address);
	void Int_GetData_ext(ubyte_t address);
	void Int_Read_Regular_Speed(ubyte_t address);
	void Int_Read_Signed_Speed(ubyte_t address);
	void Int_GetDeviceInfo(ubyte_t address);
	void Int_WriteRegister(ubyte_t address, ubyte_t regValue, ubyte_t bytesToWrite);
	void Int_ReadRegister(ubyte_t address, ubyte_t bytesToRead);
	void Int_ChangeAddress(ubyte_t address);
	void Int_General_Call_Reset(void);
	void Int_PropagateClock(ubyte_t address,ubyte_t Command);
	void Int_PutInDiagMode(ubyte_t address);
	void Int_ChangeColor(ubyte_t address,ubyte_t color);
	void Int_ExitDiagMode(ubyte_t address);
	void Int_CheckDevice(ubyte_t address);
	bool checkDevice(ubyte_t address);
	void Int_Search_For_Devices(void);
	int signedDiff(unsigned int val, unsigned int lastval);
	void testSignedDiff();
};

#endif /* CQEIMENCODERS_H_ */
