/*
 * ControlledMotor.cpp
 *
 *  Created on: Nov 3, 2012
 *      Author: bouchier
 */

#include "CQEI2C.h"
#include "CQEIMEncoder.h"
#include "qemotoruser.h"
#include "qeservo.h"
#include "pid.h"
#include "qetime.h"
#include "ControlledMotor.h"

#define SEEK_LIMIT_POWER 200

static CQEMotorUser &hMotor = CQEMotorUser::GetRef();
static CQEServo &sMotor = CQEServo::GetRef();

//! Instantiate a ControlledMotor object, which manages one motor
/*!
 *
 * Set up the operating mode of the motor.
 *
 * \param motorPortIn The VEXPro controller motor port number (1 - 16) for this motor
 * \param i2cRef The reference to the I2C bus control object. Used by the encoder.
 * \param typeIn The enum value which defines the desired behavior as speed-controlled or
 * position-controlled (servo)
 * \param ccwFwdIn Set to true if a positively increasing requested speed or velocity request (which is
 * interpreted as "forward") should result in counter-clockwise shaft rotation, false otherwise
 * \param degreesToCenter Degrees motor should travel from mechanical stop to servo center. Only used on servo.
 * Provide a positive value; it will seek in the opposite direction of the direction to the limit.
 * \param motorScaleIn unused for now
 * \param encoderScaleIn A value used to scale the number of rotations or rotations/sec or distance to inches or
 * other unit of measurement. Normally 2 * PI * wheel radius in inches to get distance in inches,
 * speed in ips.
 */
ControlledMotor::ControlledMotor(int motorPortIn, CQEI2C& i2cRef, TControlledMotorType typeIn,
		bool ccwFwdIn, bool seekLimitCcwIn, int degreesToCenterIn, float motorScaleIn, float encoderScaleIn) : i2c(i2cRef) {
	motorPort = motorPortIn;
	type = typeIn;
	ccwFwd = ccwFwdIn;
	seekLimitCcw = seekLimitCcwIn;
	degreesToCenter = abs(degreesToCenterIn);
	motorScale = motorScaleIn;
	encoderScale = encoderScaleIn;

	// set motor paramters depending on the type of port its hooked to
	if (motorPort < 13) {
		motorPort -= 1;					// transform port 1 to axis 0 on servo
		servoPortType = true;
		motorStopVal = 125;
		motorMaxBackVal = 0;
		motorMaxFwdVal = 250;
	} else {
		motorPort -= 13;		// transform port 13 to axis 0 on H-bridge
		servoPortType = false;
		motorStopVal = 0;
		motorMaxBackVal = -255;
		motorMaxFwdVal = 255;
	}

	// Instantiate the encoder. ccwFwd if true causes cw rotation to increase counts & that's forward
	encoder = new CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, ccwFwd, encoderScale);
}

//! Instantiate a ControlledMotor object, which manages one motor of the drive motor type
/*!
 *
 * Set up the operating mode of the motor.
 *
 * \param motorPortIn The VEXPro controller motor port number (1 - 16) for this motor
 * \param i2cRef The reference to the I2C bus control object. Used by the encoder.
 * \param ccwFwdIn Set to true if a positively increasing requested speed or velocity request (which is
 * interpreted as "forward") should result in counter-clockwise shaft rotation, false otherwise
 * \param encoderScaleIn A value used to scale the number of rotations or rotations/sec or distance to inches or
 * other unit of measurement. Normally 2 * PI * wheel radius in inches to get distance in inches,
 * speed in ips.
 */
ControlledMotor::ControlledMotor(int motorPortIn, CQEI2C& i2cRef, bool ccwFwdIn, float encoderScaleIn) : i2c(i2cRef) {
	motorPort = motorPortIn;
	type = speedControlledMotor;
	ccwFwd = ccwFwdIn;
	encoderScale = encoderScaleIn;

	// set motor paramters depending on the type of port its hooked to
	if (motorPort < 13) {
		motorPort -= 1;					// transform port 1 to axis 0 on servo
		servoPortType = true;
		motorStopVal = 125;
		motorMaxBackVal = 0;
		motorMaxFwdVal = 250;
	} else {
		motorPort -= 13;		// transform port 13 to axis 0 on H-bridge
		servoPortType = false;
		motorStopVal = 0;
		motorMaxBackVal = -255;
		motorMaxFwdVal = 255;
	}

	// Instantiate the encoder. ccwFwd if true causes cw rotation to increase counts & that's forward
	encoder = new CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, ccwFwd, encoderScale);
}

//! Instantiate a ControlledMotor object, which manages one motor of the servo type
/*!
 *
 * Set up the operating mode of the motor.
 *
 * \param motorPortIn The VEXPro controller motor port number (1 - 16) for this motor
 * \param i2cRef The reference to the I2C bus control object. Used by the encoder.
 * \param ccwFwdIn Set to true if a positively increasing angle request (which is
 * interpreted as "forward") should result in counter-clockwise shaft rotation, false otherwise
 * \param seekLimitCcwIn Set to true if servo should seek mech stop in ccw direction, false otherwise
 * \param degreesToCenter Degrees motor should travel from mechanical stop to servo center. Only used on servo.
 * Provide a positive value; it will seek in the opposite direction of the direction to the limit.
 */
ControlledMotor::ControlledMotor(int motorPortIn, CQEI2C& i2cRef, bool ccwFwdIn, bool seekLimitCcwIn, int degreesToCenterIn) : i2c(i2cRef) {
	motorPort = motorPortIn;
	type = servo;
	ccwFwd = ccwFwdIn;
	seekLimitCcw = seekLimitCcwIn;
	degreesToCenter = abs(degreesToCenterIn);

	// set motor paramters depending on the type of port its hooked to
	if (motorPort < 13) {
		motorPort -= 1;					// transform port 1 to axis 0 on servo
		servoPortType = true;
		motorStopVal = 125;
		motorMaxBackVal = 0;
		motorMaxFwdVal = 250;
	} else {
		motorPort -= 13;		// transform port 13 to axis 0 on H-bridge
		servoPortType = false;
		motorStopVal = 0;
		motorMaxBackVal = -255;
		motorMaxFwdVal = 255;
	}

	// Instantiate the encoder. ccwFwd if true causes cw rotation to increase counts & that's forward
	encoder = new CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, ccwFwd, 1.0);
}

void ControlledMotor::stopMotor()
{
	if (servoPortType)
		sMotor.SetCommand(motorPort, motorStopVal);
	else
		hMotor.SetPWM(motorPort, motorStopVal);
}

ControlledMotor::~ControlledMotor() {
	stopMotor();
}

//! Initialize the motor & encoder
/*!
 * Enumerates the encoder. If a servo, drives it to mechanical stop. Initializes PID parameters. init() must
 * be called in the same order as the I2C chain, since it grabs the next encoder down the chain. i.e. if LF_MOTOR
 * is first in the I2C chain, its init() must be called first, so that the correct object gets the first encoder
 * in the chain.
 *
 * \param KP PID KP	Proportional factor
 * \param KI PID KI Integral factor
 * \param KD PID KD Derivative factor
 * \return True for success, false otherwise
 */
bool ControlledMotor::init(float KP, float KI, float KD)
{
	bool foundDevice;
	float rps;
	CQETime timer;
	int stoppedCount;

	foundDevice = encoder->initNextDevice();
	if (!foundDevice) {
		printf("ERROR initializing encoder on motor %d\n", motorPort + servoPortType?1:13);
		return false;		// error initializing encoder
	}
	printf("ControlledMotor.init() found encoder at 0x%x\n", encoder->getDeviceAddr());

	// stop motors in case they're running
	stopMotor();
	sleep(1);		// let the pwm on motor29 stabilize

	// intantiate the PID object & initialize it. Used during servo zeroing
	pid = new PID(KP, KI, KD);
	pid->setDebugFlag(false);

	if (type == servo) {						// if it's intended to work like a servo
		// drive servo to mech stop, which we call zero
		printf("Slow seek to mechanical stop on motor %d\nWARNING: motor must stop for initialization to proceed\n",
				motorPort+1);
		// drive in the direction configured for this motor when seeking the mech stop
		motorPower = seekLimitCcw ? SEEK_LIMIT_POWER : (0 - SEEK_LIMIT_POWER);
		driveMotor();
		timer.msleep(50);						// let the motor get going

		// run until it hits the mech stop
		stoppedCount = 0;
		while(stoppedCount < 5) {				// we want to drive it against stop & see it stopped for 5 reads
			encoder->readEncoder();
			rps = encoder->getRevPerSec();
			//printf("motor %d speed: %f\n", motorPort, rps);
			int rps_i = (int)(rps * 1000.0);
			if (abs(rps_i) < 50)
				stoppedCount++;
			else
				stoppedCount = 0;				// clear stopped count if it's still moving
			timer.msleep(50);					// let the motor get going
		}
		stopMotor();

		// zero the encoder
		encoder->clearEncoder();				// set encoder to zero

		// Now seek back to center and re-zero the encoder
		if (ccwFwd)
			rqDegrees = seekLimitCcw ? (0-degreesToCenter) : degreesToCenter;	// +ve rqDegrees produces ccw rotation
		else
			rqDegrees = seekLimitCcw ? degreesToCenter : 0-degreesToCenter;		// +ve rqDegrees produces cw rotation
		printf("Found mech stop, seeking back to center at %d\n", rqDegrees);

		// run until it stops at center
		stoppedCount = 0;
		encoder->readEncoder();
		while(stoppedCount < 5) {				// we want to drive it to center & see it stopped for 5 reads
			updateMotor();
			timer.msleep(50);					// let the motor get going
			rps = encoder->getRevPerSec();		// relies on the encoder read done by updateMotor() previous frame
			//printf("motor %d speed: %0.2f, angle: %d, power: %0.1f\n", motorPort+1, rps, encoder->getDegrees(), motorPower);
			int rps_i = (int)(rps * 1000.0);
			if (abs(rps_i) < 50)
				stoppedCount++;
			else
				stoppedCount = 0;				// clear stopped count if it's still moving
		}
		stopMotor();

		// zero the encoder
		printf("moved from stop: %d degrees\n", encoder->getDegrees());
		encoder->clearEncoder();				// set encoder to zero
		encoder->readEncoder();
		printf("zeroed servo motor %d, angle: %d\n", motorPort+1, encoder->getDegrees());
		rqDegrees = 0;							// set it pointing in current (straight) direction before next operation
	}
	return true;
}

//! Drive the motor at requested power
/*!
 * Clamps the requested power to limits, & scales for servo motor port, then
 * tells motor to run
 */
void ControlledMotor::driveMotor()
{
	// clamp motor power to max for this motor type
	// motorPower is a float centered on 0.0
	if (servoPortType) {
		motorPower /= 2;				// power range on servos (125) is half of that on hmotor (250), so halve power
		motorPower += motorStopVal;		// bias it up to mid-range because it's a servo
	}
	if (motorPower > motorMaxFwdVal)
		motorPower = (float)motorMaxFwdVal;
	if (motorPower < motorMaxBackVal)
		motorPower = motorMaxBackVal;

	// tell the motors to run at motorPower
	if (servoPortType) {
		sMotor.SetCommand(motorPort, (int)motorPower);
	} else {
		hMotor.SetPWM(motorPort, (int)motorPower);
	}
}

//! Poll the encoder & run the PID algorithm & apply the new drive power value to the motor
/*!
 * Reads the encoder, & based on the mode (servo or speed control), computes the error term.
 * Applies the error to the PID object, and applies the resulting power output to the motor
 *
 * \return True for success
 */
bool ControlledMotor::updateMotor()
{
	bool rv;
	float encoderValue;
	int degrees;

	// read the raw data from the IME into the encoder object
	rv = encoder->readEncoder();
	if (!rv) {
		printf("ERROR reading encoder on motor %d\n", motorPort + servoPortType?1:13);
		return rv;
		// for now, ignore error return - catch it next time round
	}

	// run the PID algorithm to control the motor
	if (type == speedControlledMotor) {
		encoderValue = encoder->getSpeed(); 	// if it's a speed controlled motor get the actual speed
		motorPower = pid->computePid(encoderValue, rqSpeed);
	} else {
		degrees = encoder->getDegrees();	// get the angle of the dangle
		motorPower = pid->computePid(degrees, rqDegrees);
	}

	/*
	 * Reverse motor direction based on the definition of ccwFwd. (Positive motor power
	 * drives the motor ccw when viewing shaft, so if forward is cw shaft rotation, we want to drive negative
	 * motor power for a forward speed request.)
	 */
	if (!ccwFwd)
		motorPower = 0 - motorPower;
	driveMotor();							// drive motor at motorPower

	return true;
}

// getters, setters
float ControlledMotor::getSpeed()
{
	return encoder->getSpeed();
}

float ControlledMotor::getDistance()
{
	return encoder->getDistance();
}

float ControlledMotor::getMotorPower()
{
	return motorPower;
}

void ControlledMotor::setSpeed(float rqSpeed)
{
    this->rqSpeed = rqSpeed;
}

void ControlledMotor::setDegrees(int rqDegreesIn)
{
	if (rqDegreesIn > 90) {
		printf("WARNING: request for angle > 90 received: %d, limited to 90\n", rqDegreesIn);
		rqDegrees = 90;
	} else if (rqDegreesIn < -90) {
		printf("WARNING: request for angle < -90 received: %d, limited to -90\n", rqDegreesIn);
		rqDegrees = -90;

	} else {
		rqDegrees = rqDegreesIn;
	}
}

int ControlledMotor::getDegrees()
{
	return encoder->getDegrees();
}
