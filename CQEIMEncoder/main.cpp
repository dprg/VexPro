/*
 * main.cpp
 *
 *  Created on: Oct 12, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "CQEI2C.h"
#include "CQEIMEncoder.h"
#include "qemotoruser.h"
#include "qeservo.h"

#define DEFAULT_TEST 8

#define NUM_ENCODERS 2

#define LF_DRIVE 15
#define LF_STEER 3
#define RF_DRIVE 16
#define RF_STEER 1
#define LM_DRIVE 2
#define LM_STEER 7
#define RM_DRIVE 12
#define RM_STEER 6
#define LB_DRIVE 14
#define LB_STEER 4
#define RB_DRIVE 13
#define RB_STEER 5


CQEI2C i2c = CQEI2C();		// instantiate the I2C driver
CQEMotorUser &motor = CQEMotorUser::GetRef();
CQEServo &servo = CQEServo::GetRef();
CQEIMEncoder *imeTable[12];

/*
 * Read & print encoder count & speed, and computed value (revs/sec, radian/sec, in/sec, distance
 */
void printCountSpeed()
{
	unsigned int count;
	short spd;
	float rps, radps, speed, distance;
	int rv;
	int i;

	for (i=0; i<NUM_ENCODERS; i++) {
		rv = imeTable[i]->getRawCountSpeed(count, spd);		// read encoder count & speed
		rps = imeTable[i]->getRevPerSec();						// convert speed to rps with sign

		if (rv)
			printf("motor %d cnt: %d spd: %d | ", i, count, spd);
		else
			printf("Error reading getRawCountSpeed\n");
	}
	printf("\n");
	for (i=0; i<NUM_ENCODERS; i++) {
		rps = imeTable[i]->getRevPerSec();
		radps = imeTable[i]->getRadPerSec();
		speed = imeTable[i]->getSpeed();
		distance = imeTable[i]->getDistance();
		printf("Motor %d: rev/sec: %0.2f rad/sec: %0.2f speed: %0.2f distance: %0.2f\n", i, rps, radps, speed, distance);
	}
}

void printDevices()
{
	int i;

	for (i=0; i<NUM_ENCODERS; i++) {
		printf("Encoder enumerated at 0x%x\n", imeTable[i]->getDeviceAddr());
	}
}
/*
 * Run the motors for a bit then read & print encoder count & speed
 */
void runMotor(int motorNum, int rqSpeed, bool quiet=false)
{
	if (motorNum > 12) {
		motorNum -= 13;
		motor.SetPWM(motorNum, rqSpeed);
	} else {
		motorNum -= 1;
		servo.SetCommand(motorNum, rqSpeed);
	}
	sleep(1);
	if (!quiet)
		printCountSpeed();
}

void runMotors(int rqSpeed)
{
	motor.SetPWM(LF_DRIVE - 13, rqSpeed);
	motor.SetPWM(RF_DRIVE - 13, 0-rqSpeed);
	motor.SetPWM(LB_DRIVE - 13, rqSpeed);
	motor.SetPWM(RB_DRIVE - 13, 0-rqSpeed);
	servo.SetCommand(LM_DRIVE - 1, 125+rqSpeed/2);
	servo.SetCommand(RM_DRIVE - 1, 125-rqSpeed/2);

	sleep(3);
	printCountSpeed();
}

/*
 * Run motors forward then back for same amount of time & print count & speed at each point
 */
void test2()
{
	runMotors(64);
	runMotors(128);
	runMotors(255);
	runMotors(0);
	runMotors(-64);
	runMotors(-128);
	runMotors(-255);
	runMotors(0);
}

void test5()
{
	float rps, radps, speed, distance;

	motor.SetPWM(0, 64);
	sleep(1);
	printCountSpeed();
	rps = imeTable[0]->getRevPerSec();
	radps = imeTable[0]->getRadPerSec();
	speed = imeTable[0]->getSpeed();
	distance = imeTable[0]->getDistance();
	printf("rev/sec: %0.2f rad/sec: %0.2f speed: %0.2f distance: %0.2f\n", rps, radps, speed, distance);
}

void test6()
{
	int i, j;
	int rqSpeed;

	for (j=0; j<240; j+=20) {
		rqSpeed = j;
		for (i=0; i<4; i++) {
		    motor.SetPWM(i, rqSpeed);
		}
		for (i=0; i<2; i++) {
			servo.SetCommand(i, (rqSpeed/2) + 127);
		}
		sleep(1);
	}
	for (j=250; j>-240; j-=20) {
		rqSpeed = j;
		for (i=0; i<4; i++) {
		    motor.SetPWM(i, rqSpeed);
		}
		for (i=0; i<2; i++) {
			servo.SetCommand(i, (rqSpeed/2)+127);
		}
		sleep(1);
	}
	for (j=-240; j<0; j+=20) {
		rqSpeed = j;
		for (i=0; i<4; i++) {
		    motor.SetPWM(i, rqSpeed);
		}
		for (i=0; i<2; i++) {
			servo.SetCommand(i, (rqSpeed/2)+127);
		}
		sleep(1);
	}

}

void test7()
{
	printDevices();
	motor.SetPWM(0, 50);
	servo.SetCommand(0, 140);
	sleep(1);
	printCountSpeed();
	sleep(1);
	printCountSpeed();

	// stop motor
	servo.SetCommand(0, 125);
}

void test8()
{
	imeTable[0]->clearEncoder();
	imeTable[1]->clearEncoder();
	runMotor(RF_STEER, 200);
	runMotor(RB_DRIVE, 80);
	runMotor(RF_STEER, 40);
	runMotor(RB_DRIVE, -80);
	runMotor(RF_STEER, 125);
	runMotor(RB_DRIVE, 0);
}

/*
 * Run tests on the first encoder in the chain, using the given motor number
 */
void test9(int encNum, int motNum)
{
	float rps, radps, speed, distance;
	float lastrps, lastradps, lastspeed, lastdistance;
	bool foundDevice;

	i2c.I2CInit();			// reset the encoder chain
	CQEIMEncoder encoder = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, true, 2 * PI);
	foundDevice = encoder.initNextDevice();
	printf("encoder %s found; ", foundDevice?"was":"NOT");
	if (foundDevice)
		encoder.printDevice(READ_DEV_TICS);
	printf("Motor should run CCW\n");

	// run motor for 1 sec
	runMotor(motNum, RF_STEER);
	encoder.readEncoder();
	lastrps = encoder.getRevPerSec();
	lastradps = encoder.getRadPerSec();
	lastspeed = encoder.getSpeed();
	lastdistance = encoder.getDistance();
	// wait then read again
	sleep(1);
	encoder.readEncoder();
	rps = encoder.getRevPerSec();
	radps = encoder.getRadPerSec();
	speed = encoder.getSpeed();
	distance = encoder.getDistance();

	// check the readings
	if (rps < lastrps) {printf("ERROR rps < lastrps\n"); return;}
}

void usage()
{
	printf("Usage: vexMotorEncoder testNum\n");
	printf("testNum can be:\n");
	printf("0: print encoder values\n");
	printf("1: run motors for a bit then print\n");
	printf("2: run motor forward then back & print encoders at some points\n");
	printf("3: run motor, print, clear encoders, print\n");
	printf("4: run test() on motor 0\n");
	printf("5: run motor 12 a short distance then print counts\n");
	printf("6: ramp 6 motors up & down fwd & back\n");
	printf("7: run motors 13 & 1 for 1 sec & print counts\n");
	printf("8: run motors forward\n");
}

int main(int argc, char **argv)
{

	CQEIMEncoder enc0 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, false, 2 * PI);
	CQEIMEncoder enc1 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, false, 2 * PI); // count should increase for ccw rot
	CQEIMEncoder enc2 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, true, 2 * PI); // count should increase for ccw rot
	CQEIMEncoder enc3 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, true, 2 * PI); // count should increase for ccw rot
	CQEIMEncoder enc4 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, true, 2 * PI);
	CQEIMEncoder enc5 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, true, 2 * PI); // count should increase for ccw rot
	CQEIMEncoder enc6 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, true, 2 * PI); // count should increase for ccw rot
	CQEIMEncoder enc7 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, true, 2 * PI); // count should increase for ccw rot
	CQEIMEncoder enc8 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, true, 2 * PI);
	CQEIMEncoder enc9 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, true, 2 * PI); // count should increase for ccw rot
	CQEIMEncoder enc10 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, true, 2 * PI); // count should increase for ccw rot
	CQEIMEncoder enc11 = CQEIMEncoder(i2c, CQEIMEncoder::motor393Torque, true, 2 * PI); // count should increase for ccw rot
	imeTable[0] = &enc0;
	imeTable[1] = &enc1;
	imeTable[2] = &enc2;
	imeTable[3] = &enc3;
	imeTable[4] = &enc4;
	imeTable[5] = &enc5;
	imeTable[6] = &enc6;
	imeTable[7] = &enc7;
	imeTable[8] = &enc8;
	imeTable[9] = &enc9;
	imeTable[10] = &enc10;
	imeTable[11] = &enc11;
	int imeCnt = NUM_ENCODERS;

	bool foundDevice;
	int test = DEFAULT_TEST;	// which test to run
	int i;

	if (argc == 2)
		test = atoi(argv[1]);
	printf("Running test %d\n", test);

	// Initialize & print the chain of devices
	for (i=0; i<imeCnt; i++) {
		foundDevice = imeTable[i]->initNextDevice();
		printf("device %d %s found; ", i, foundDevice?"was":"NOT");
		if (foundDevice)
			imeTable[i]->printDevice(READ_DEV_TICS);
		else
			return -1;			// exit if we get an error enumerating encoders
	}

	switch (test) {
	case 0: printCountSpeed(); break;
	case 1:  runMotors(64); break;
	case 2: test2(); break;
	case 3: runMotors(64); imeTable[0]->clearEncoder(); printCountSpeed(); break;
	case 4:
		imeTable[0]->test(0);
		break;
	case 5:
		test5(); break;
	case 6: test6(); break;
	case 7: test7(); break;
	case 8: test8(); break;
	case 9: test9(0, RF_STEER); break;
	default:
		printf("Invalid option\n");
		usage();
	}

}
