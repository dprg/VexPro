/*
 * main.cpp
 *
 *  Created on: Oct 12, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "qegpioint.h"
#include "RCTest.h"
#include "Metro.h"
#include "CQEI2C.h"
#include "ControlledMotor.h"

#define DEFAULT_TEST 6

#define PRINT_RATE 5
#define PI 3.14159265359
#define WHEEL_CIRCUMFERENCE 2 * PI * 2

#define RF_MOTOR 1
#define RF_STEERING 2
#define LF_MOTOR 13
#define MOT15 13

// PID parameters for motors 13 - 16 (on H-bridge
#define DRIVE_HMOTOR_KP 15.0
#define DRIVE_HMOTOR_KI 2.0
#define DRIVE_HMOTOR_KD 1.0
#define DRIVE_SMOTOR_KP 10.0
#define DRIVE_SMOTOR_KI 1.0
#define DRIVE_SMOTOR_KD 0.0
#define SERVO_SMOTOR_KP 3.0
#define SERVO_SMOTOR_KI 1.0
#define SERVO_SMOTOR_KD 2.0
#define SERVO_HMOTOR_KP 10.0
#define SERVO_HMOTOR_KI 1.0
#define SERVO_HMOTOR_KD 0.0

CQEI2C i2c = CQEI2C();		// instantiate the I2C driver
CQEGpioInt &gpio = CQEGpioInt::GetRef();

struct RCChannel dirRcc;
struct RCChannel speedRcc;


void fatal( int motNum)
{
	printf("ERROR initializing motor %d\n", motNum);
	exit(0);
}

#define TEST1_SPEED 10
void test1(int speed = TEST1_SPEED)			// default to TEST1_SPEED
{
	int runtime = 100;		// # of 50 msec frames
	int printRate = PRINT_RATE;

	// instantiate the controlled motors. Must do this in the order of the I2C chain
	ControlledMotor rfMotor = ControlledMotor(RF_MOTOR, i2c, ControlledMotor::speedControlledMotor,
			true, true, 0, 1.0, WHEEL_CIRCUMFERENCE);
	ControlledMotor mot15 = ControlledMotor(MOT15, i2c, ControlledMotor::speedControlledMotor,
			false, true, 0, 1.0, WHEEL_CIRCUMFERENCE);

	// initialize the motors with their encoders & anything else. Order must match the I2C chain order
	if (!rfMotor.init(DRIVE_SMOTOR_KP, DRIVE_SMOTOR_KI, DRIVE_SMOTOR_KD)) fatal(RF_MOTOR);
	if (!mot15.init(DRIVE_HMOTOR_KP, DRIVE_HMOTOR_KI, DRIVE_HMOTOR_KD)) fatal(MOT15);

	// Set their speed
	rfMotor.setSpeed(speed);	// ips
	mot15.setSpeed(speed);	// ips

	Metro metro = Metro(50);		// 50ms metronome

	while(runtime) {
		if (metro.check()) {		// if 50ms have passed & it's time to do the control loop
			rfMotor.updateMotor();		// run the PID loop
			mot15.updateMotor();		// run the PID loop
			if (printRate-- == 0) {
				printRate = PRINT_RATE;
				printf("MOT15 speed: %0.1f, power %0.1f, distance: %0.1f; RF speed: %0.1f, power: %0.1f; distance: %0.1f\n",
						mot15.getSpeed(), mot15.getMotorPower(), mot15.getDistance(),
						rfMotor.getSpeed(), rfMotor.getMotorPower(), rfMotor.getDistance());
			}
			runtime--;
		}
	}
	return;
}

void test2()
{
	int target;
	int runtime = 200;		// # of 50 msec frames
	int printRate = PRINT_RATE;
	//int rv = 0;

	// instantiate the controlled motors. Order must match the I2C chain order
	ControlledMotor rfMotor = ControlledMotor(RF_MOTOR, i2c, ControlledMotor::servo,
			true, true, 90);

	// initialize them with their encoders & anything else
	if (!rfMotor.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(RF_MOTOR);

	printf("Servo initialized\n");
	sleep(5);

	// Set desired position
	target = -90;
	printf("Seeking to %d\n", target);
	rfMotor.setDegrees(target);	// ips

	Metro metro = Metro(50);		// 50ms metronome

	while(runtime) {
		if (runtime == 100) {				// move the servo a bit
			// Set desired position
			target = 45;
			printf("Seeking to %d\n", target);
			rfMotor.setDegrees(target);	// ips
			runtime--;
		}

		if (metro.check()) {		// if 50ms have passed & it's time to do the control loop
			rfMotor.updateMotor();		// run the PID loop
			if (printRate-- == 0) {
				printRate = PRINT_RATE;
				printf("RF degrees: %d, power %0.1f, \n",
						rfMotor.getDegrees(), rfMotor.getMotorPower());
			}
			runtime--;
		}
	}
	return;
}

void test3()
{
	int target;
	int runtime = 200;		// # of 50 msec frames
	int printRate = PRINT_RATE;
	//int rv = 0;

	// instantiate the controlled motors. Order must match the I2C chain order
	ControlledMotor rfMotor = ControlledMotor(RF_MOTOR, i2c, ControlledMotor::servo,
			false, true, 90);
	ControlledMotor mot15 = ControlledMotor(MOT15, i2c, ControlledMotor::servo,
			true, true, 90);

	// initialize them with their encoders & anything else
	if (!rfMotor.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(RF_MOTOR);
	if (!mot15.init(SERVO_HMOTOR_KP, SERVO_HMOTOR_KI, SERVO_HMOTOR_KD)) fatal(MOT15);

	printf("Servo initialized\n");
	sleep(5);

	// Set desired position
	target = -90;
	printf("Seeking to %d\n", target);
	rfMotor.setDegrees(target);	// ips
	mot15.setDegrees(target);	// ips

	Metro metro = Metro(50);		// 50ms metronome

	while(runtime) {
		if (runtime == 100) {				// move the servo a bit
			// Set desired position
			target = 45;
			printf("Seeking to %d\n", target);
			rfMotor.setDegrees(target);	// ips
			mot15.setDegrees(target);	// ips
			runtime--;
		}

		if (metro.check()) {		// if 50ms have passed & it's time to do the control loop
			rfMotor.updateMotor();		// run the PID loop
			mot15.updateMotor();
			if (printRate-- == 0) {
				printRate = PRINT_RATE;
				printf("RF degrees: %d, power %0.1f, MOT15 degrees: %d, power %0.1f,\n",
						rfMotor.getDegrees(), rfMotor.getMotorPower(),
						mot15.getDegrees(), mot15.getMotorPower());
			}
			runtime--;
		}
	}
	return;
}

void test4()
{
	int target;
	int runtime = 999;		// # of 50 msec frames
	int printRate = PRINT_RATE;
	//int rv = 0;

	// instantiate the controlled motors.
	ControlledMotor rfMotor = ControlledMotor(RF_MOTOR, i2c, ControlledMotor::servo,
			false, true, 90);
	ControlledMotor mot15 = ControlledMotor(MOT15, i2c, ControlledMotor::servo,
			true, true, 1.0, 90);

	// initialize them with their encoders & anything else. Order must match the I2C chain order
	if (!rfMotor.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(RF_MOTOR);
	if (!mot15.init(SERVO_HMOTOR_KP, SERVO_HMOTOR_KI, SERVO_HMOTOR_KD)) fatal(MOT15);

	printf("Servo initialized\n");
	sleep(5);

	// Set desired position
	target = -90;
	printf("Seeking to %d\n", target);
	rfMotor.setDegrees(target);	// ips
	mot15.setDegrees(target);	// ips

	Metro metro = Metro(50);		// 50ms metronome

	while(runtime) {
		if (runtime%50 == 0) {				//calculate a new target
			// Set desired position
			//target = (int)(((float)rand()/RAND_MAX * 180.0) - 90.0);	// compute a random angle to seek to
			target = 0 - target;		// temp code to make it swing -90 -> +90
			printf("Seeking to %d\n", target);
			rfMotor.setDegrees(target);	// ips
			mot15.setDegrees(target);	// ips
			runtime--;
		}

		if (metro.check()) {		// if 50ms have passed & it's time to do the control loop
			rfMotor.updateMotor();		// run the PID loop
			mot15.updateMotor();
			if (printRate-- == 0) {
				printRate = PRINT_RATE;
				printf("RF degrees: %d, power %0.1f, MOT15 degrees: %d, power %0.1f,\n",
						rfMotor.getDegrees(), rfMotor.getMotorPower(),
						mot15.getDegrees(), mot15.getMotorPower());
			}
			runtime--;
		}
	}
	return;
}

void test5()
{
	int targetAngle;
	float rqSpeed;
	int printRate = PRINT_RATE;
	//int rv = 0;

	// instantiate the controlled motors.
	ControlledMotor rfMotor = ControlledMotor(RF_MOTOR, i2c, ControlledMotor::servo,
			false, true, 90);
	ControlledMotor mot15 = ControlledMotor(MOT15, i2c, ControlledMotor::speedControlledMotor,
			true, true, 0, 1.0, WHEEL_CIRCUMFERENCE);

	// initialize them with their encoders & anything else. Order must match the I2C chain order
	if (!rfMotor.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(RF_MOTOR);
	if (!mot15.init(SERVO_HMOTOR_KP, SERVO_HMOTOR_KI, SERVO_HMOTOR_KD)) fatal(MOT15);

	printf("Servo & motor initialized\n");

	// initialize the steering struct & start interrupt monitoring
	dirRcc.flag = 1;
	dirRcc.dioIndex = 0;
	initRCTest(gpio, &dirRcc);

	// initialize the speed struct & start interrupt monitoring
	speedRcc.flag = 1;
	speedRcc.dioIndex = 1;
	initRCTest(gpio, &speedRcc);

	Metro metro = Metro(50);
	while (1){
		if (metro.check()) {

			// convert R/C values to desired speed range -20 - +20 ips & angle range +/-90
			rqSpeed = (float)(speedRcc.pulseWidth-1460)/25.0;
			mot15.setSpeed(rqSpeed);
			targetAngle = (dirRcc.pulseWidth-1500)/5;
			rfMotor.setDegrees(targetAngle);

			// update the motors
			rfMotor.updateMotor();		// run the PID loop
			mot15.updateMotor();
			if (printRate-- == 0) {
				printRate = PRINT_RATE;
				printf("R/C Speed: %d, R/C Dir: %d, ", speedRcc.pulseWidth, dirRcc.pulseWidth);
				printf("MOT15 speed: %0.1f, power %0.1f, RF angle: %d, power %0.1f\n",
						mot15.getSpeed(), mot15.getMotorPower(),
						rfMotor.getDegrees(), rfMotor.getMotorPower());
			}
		}
	}

}

void test6()
{
	ControlledMotor *rfMotor;

	/*
	 * instantiate the controlled motor & reset the encoder with fwd = ccw, seeklimit = ccw
	 */
	i2c.I2CInit();
	rfMotor = new ControlledMotor(RF_MOTOR, i2c, ControlledMotor::servo,
			true, true, 90);
	if (!rfMotor->init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(RF_MOTOR);
	printf("Servo initialized with true true, 90 - ccw limit-seek, cw center seek expected\n");
	sleep(5);
	delete rfMotor;

	/*
	 * instantiate the controlled motor & reset the encoder with fwd = cw, seeklimit = ccw
	 */
	i2c.I2CInit();
	rfMotor = new ControlledMotor(RF_MOTOR, i2c, ControlledMotor::servo,
			false, true, 90);
	if (!rfMotor->init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(RF_MOTOR);
	printf("Servo initialized with true true, 90 - cw limit-seek, cw center seek expected\n");
	sleep(5);
	delete rfMotor;

	/*
	 * instantiate the controlled motor & reset the encoder with fwd = ccw, seeklimit = cw
	 */
	i2c.I2CInit();
	rfMotor = new ControlledMotor(RF_MOTOR, i2c, ControlledMotor::servo,
			true, false, 90);
	if (!rfMotor->init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(RF_MOTOR);
	printf("Servo initialized with true true, 90 - cw limit-seek, cw center seek expected\n");
	sleep(5);
	delete rfMotor;

	/*
	 * instantiate the controlled motor & reset the encoder with fwd = cw, seeklimit = cw
	 */
	i2c.I2CInit();
	rfMotor = new ControlledMotor(RF_MOTOR, i2c, ControlledMotor::servo,
			false, false, 90);
	if (!rfMotor->init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(RF_MOTOR);
	printf("Servo initialized with true true, 90 - cw limit-seek, cw center seek expected\n");
	sleep(5);
	delete rfMotor;

	return;
}


void usage()
{
	printf("Usage: ControlledMotor testNum\n");
	printf("testNum can be:\n");
	printf("1: run motor at constant speed for 10 sec\n");
	printf("2: seek one servo to -90 then 45 degrees\n");
	printf("3: seek two servos - one on servo port & one on h-bridge - to -90 then 45\n");
	printf("4: seek two servos - one on servo port & one on h-bridge - either randomly or to +/-90\n");
	printf("5: control a servo & a motor with R/C\n");
}

int main(int argc, char **argv)
{
	int test = DEFAULT_TEST;	// which test to run
	int argval;		// value to pass into test

	if (argc == 2)
		test = atoi(argv[1]);
	if (argc == 3)
		argval = atoi(argv[2]);
	printf("Running test %d\n", test);

	switch (test) {
	case 1:
		if (argc == 3)
			test1(argval);
		else
			test1();
		 break;
	case 2: test2(); break;
	case 3: test3(); break;
	case 4: test4(); break;
	case 5: test5(); break;
	case 6: test6(); break;
	default:
		printf("Invalid option\n");
		usage();
	}
}
