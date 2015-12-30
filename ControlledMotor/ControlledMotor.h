/*
 * ControlledMotor.h
 *
 *  Created on: Nov 3, 2012
 *      Author: bouchier
 */
/*! \file ControlledMotor.h
 * \brief Header file for ControlledMotor - the class which does closed-loop control of motor position or speed
 *
 * <H1>
 * Build Configuration
 * </H1>
 *
 * This project depends on the following peer projects being at the same directory level:
 * - CQEI2C
 * - Metro
 * - CQEIMEncoder
 * - qetime
 * - PID
 * - RCTest
 *
 * Edit the project properties as follows to reference them as includes, link objects, and referenced projects.
 *
 * Under C/C++ Build -> Settings, Tool Settings tab, TerkOS C++ Compiler group, Directories settings
 * add the following paths
 * to the terkos paths that are already there. This adds them to the include path for compilation
 * - ../../CQEI2C
 * - ../../Metro
 * - ../../CQEIMEncoder
 * - ../../qetime
 * - ../../PID
 * - ../../RCTest
 *
 * Under C/C++ Build -> Settings, Tool Settings tab, TerkOS C++ Linker group, Miscellaneous settings, add
 * the following "other objects". This tells the linker to link to qetime.o & CQEI2C.o.
 * - ../../CQEI2C/Debug/CQEI2C.o
 * - ../../Metro/Debug/Metro.o
 * - ../../CQEIMEncoder/Debug/CQEIMEncoder.o
 * - ../../qetime/Debug/qetime.o
 * - ../../PID/Debug/pid.o
 * - ../../RCTest/Debug/RCTest.o
 *
 * In the Project References group, check the following projects. This builds them before the current project.
 * CQEI2C
 * Metro
 * CQEIMEncoder
 * qetime
 * PID
 *
 */

#ifndef CONTROLLEDMOTOR_H_
#define CONTROLLEDMOTOR_H_

class PID;
class CQEIMEncoder;

/*! \class ControlledMotor
 * \brief This class controls a motor using closed loop speed or position feedback from the IME
 *
 * This class uses the Integrated Motor Encoder (IME) to read motor position or speed, and does
 * closed-loop speed or position control using a PID object to control the effort applied to
 * the motor. It can handle motors connected to the servo ports (1 - 12) or the 2-wire motor ports
 * (13 - 16).
 *
 * The constructor sets up the operating mode of the motor.
 *
 * Next, init() should be called to discover the encoder & initialize it, and optionally
 * seek the motor to a mechanical stop, if it's in servo mode,
 * and to do any other motor setup which could fail. <B>Note: motors MUST be initialized in the
 * order they appear on the I2C bus chain</B>, or the wrong motor will get associated to the wrong
 * encoder. This means that in order to use the 3rd motor in the string, the previous two must
 * be initialized.
 *
 * Next, the PID parameters should be set.
 *
 * Finally, the motor should be polled with updateMotor() at the regular interval (e.g. 50ms),
 * and given commands to set its position or speed. The PID controller will drive it to
 * the requested speed or position.
 *
 * The getSpeed() & getDistance() functions are only valid after calling updateMotor(), because updateMotor()
 * calls the readEncoder() function
 */
class ControlledMotor {
public:
	/*! \var typedef enum TControlledMotorType
	 * \brief Type of controlled behavior desired from the motor (speed control or servo position)
	 *
	 * Valid values are seedControlledMotor, servo.
	 */
	typedef enum {
		speedControlledMotor,
		servo
	} TControlledMotorType;

	ControlledMotor(int motorPort, CQEI2C& i2cRef, TControlledMotorType typeIn,
			bool cwFwdIn, bool seekLimitCcwIn=true, int degreesToCenter=0,
			float motorScaleIn=1.0, float encoderScaleIn=1.0);
	ControlledMotor(int motorPortIn, CQEI2C& i2cRef, bool ccwFwdIn,
			bool seekLimitCcwIn, int degreesToCenterIn); // servo constructor
	ControlledMotor(int motorPortIn, CQEI2C& i2cRef, bool ccwFwdIn, float encoderScaleIn); // drive motor constructor
	virtual ~ControlledMotor();
	float getSpeed();
	float getDistance();
	bool updateMotor();					// update the motor with new values & read the encoder
	bool init(float KP, float KI, float KD);	// initialize the motor & PID
	void setSpeed(float rqSpeed);
	void setDegrees(int rqDegreesIn);		// set servo position
	int getDegrees();						// get servo position
    float getMotorPower();

private:
    int motorPort;				// the motor port number on the vexpro (1 - 16)
	float rqSpeed;				// requested speed in whatever units
	int rqDegrees;				// requested position in degrees for servo
	CQEI2C& i2c;
	TControlledMotorType type;
	bool ccwFwd;				// true if "forward" (+ve input) should mean run clockwise looking at shaft
	bool seekLimitCcw;			// true if motor shaft should turn ccw when seeking servo mech stop
	int degreesToCenter;		// how far to seek to servo midpoint
	float motorScale;
	float encoderScale;
	int motorStopVal;			// value that makes motor stop
	int motorMaxBackVal;		// maximum backward value (used for clamping)
	int motorMaxFwdVal;			// maximum forward value (used for clamping)
	float motorPower;			// The requested motor power before clamping
	bool servoPortType;			// motor is on a servo port (1 - 12)

	CQEIMEncoder *encoder;		// encoder connected to this motor
	PID *pid;					// default KP=1.0 for now
	void stopMotor();
	void driveMotor();			// drive motor with requested power
};

#endif /* CONTROLLEDMOTOR_H_ */
