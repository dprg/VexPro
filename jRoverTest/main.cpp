/*
 * main.cpp
 *
 *  Created on: Oct 27, 2012
 *      Author: bouchier
 */
/*! \file jRoverTest/main.cpp
 * \brief Header file for jRoverTest - the file which does whole-system exercising of jRover
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
 * - ControlledMotor
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
 * - ../../ControlledMotor
 *
 * Under C/C++ Build -> Settings, Tool Settings tab, TerkOS C++ Linker group, Miscellaneous settings, add
 * the following "other objects". This tells the linker to link to qetime.o & CQEI2C.o.
 * - ../../CQEI2C/Debug/CQEI2C.o
 * - ../../Metro/Debug/Metro.o
 * - ../../CQEIMEncoder/Debug/CQEIMEncoder.o
 * - ../../qetime/Debug/qetime.o
 * - ../../PID/Debug/pid.o
 * - ../../RCTest/Debug/RCTest.o
 * - ../../ControlledMotor/Debug/ControlledMotor.o
 *
 * In the Project References group, check the following projects. This builds them before the current project.
 * CQEI2C
 * Metro
 * CQEIMEncoder
 * qetime
 * PID
 * ControlledMotor
 *
 */

#define DEFAULT_TEST 6

// default command lines to run. Add new command lines here & comment out all but one
char *default_argv[] = {"jRoverTest"};
int default_argc = 1;

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>   /* calloc, strtol */
#include <string.h>   /* String function definitions */
#include <unistd.h>
#include "qegpioint.h"
#include "RCTest.h"
#include "Metro.h"
#include "CQEI2C.h"
#include "ControlledMotor.h"
#include <ros.h>
#include <std_msgs/Int32.h>

#define PRINT_RATE 5
#define PI 3.14159265359
#define WHEEL_CIRCUMFERENCE 2 * PI * 2

/*
 * Wiring assumptions:
 * Motor ports
 * 15: left front drive
 * 3: left front steer
 * 16: right front drive
 * 1: right front steer
 * 2: left middle drive
 * 7: left middle steer
 * 12: right middle drive
 * 6: right middle steer
 * 14: left back drive
 * 4: left back steer
 * 13: right back drive
 * 5: right back steer
 */

// Motor numbers, in order of encoder wiring
#define LM_STEER 7
#define LM_DRIVE 2
#define LB_DRIVE 14
#define LB_STEER 4
#define LF_DRIVE 15
#define LF_STEER 3
#define RM_STEER 6
#define RB_DRIVE 13
#define RB_STEER 5
#define RM_DRIVE 12
#define RF_STEER 1
#define RF_DRIVE 16

// PID parameters for motors 1 - 16 (on servo ports & H-bridge for both servo & motor use)
#define DRIVE_HMOTOR_KP 15.0
#define DRIVE_HMOTOR_KI 2.0
#define DRIVE_HMOTOR_KD 1.0
#define DRIVE_SMOTOR_KP 10.0
#define DRIVE_SMOTOR_KI 1.0
#define DRIVE_SMOTOR_KD 0.0
#define SERVO_SMOTOR_KP 3.5
#define SERVO_SMOTOR_KI 2.0
#define SERVO_SMOTOR_KD 2.0
#define SERVO_HMOTOR_KP 10.0
#define SERVO_HMOTOR_KI 1.0
#define SERVO_HMOTOR_KD 0.0

CQEI2C i2c = CQEI2C();		// instantiate the I2C driver
CQEGpioInt &gpio = CQEGpioInt::GetRef();
void driveRover(float linear, float angular);

ros::NodeHandle  nh;
std_msgs::Int32 range;
ros::Publisher motorTelemetry("motorTelemetry", &range);
char *rosSrvrIp = "192.168.15.149";
bool rosFlag = false;	// true to enable connection to ROS

struct RCChannel dirRcc;
struct RCChannel speedRcc;

int printRate = PRINT_RATE;

// instantiate the controlled motors.
ControlledMotor lfDrive = ControlledMotor(LF_DRIVE, i2c, true, WHEEL_CIRCUMFERENCE);
ControlledMotor lfSteer = ControlledMotor(LF_STEER, i2c, true, true, 135);
ControlledMotor rfDrive = ControlledMotor(RF_DRIVE, i2c, false, WHEEL_CIRCUMFERENCE);
ControlledMotor rfSteer = ControlledMotor(RF_STEER, i2c, true, false, 135);
ControlledMotor lmDrive = ControlledMotor(LM_DRIVE, i2c, true, WHEEL_CIRCUMFERENCE);
ControlledMotor lmSteer = ControlledMotor(LM_STEER, i2c, true, true, 140);
ControlledMotor rmDrive = ControlledMotor(RM_DRIVE, i2c, false, WHEEL_CIRCUMFERENCE);
ControlledMotor rmSteer = ControlledMotor(RM_STEER, i2c, true, false, 145);
ControlledMotor lbDrive = ControlledMotor(LB_DRIVE, i2c, true, WHEEL_CIRCUMFERENCE);
ControlledMotor lbSteer = ControlledMotor(LB_STEER, i2c, true, false, 140);
ControlledMotor rbDrive = ControlledMotor(RB_DRIVE, i2c, false, WHEEL_CIRCUMFERENCE);
ControlledMotor rbSteer = ControlledMotor(RB_STEER, i2c, true, true, 140);

void fatal( int motNum)
{
	printf("ERROR initializing motor %d\n", motNum);
	exit(0);
}

/*
 * driveMotorMsgCb is called when the Velocity message is published to the driveRover topic.
 * It calls driveRover to calculate and apply the angle and speed for each motor, with the
 * result that the robot drives at the requested linear and angular velocities.
 */
//void driveRoverMsgCb(const jrover::Velocity& velocity_msg){
//        float linear = velocity_msg.linear;
//        float angular = velocity_msg.angular;
//        driveRover(linear, angular);
//}
//ros::Subscriber<jrover::Velocity> driveRoverSub("driveRover", driveRoverMsgCb );

//! initialize all motors, in order of the I2C chain
/*!
 * WARNING: do not change the order of the I2C chain without changing the order init()
 * is called in this function.
 */
void initMotors()
{
	printf("Initializing all motors\n");
	if (!lmSteer.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(LM_STEER);
	if (!lmDrive.init(DRIVE_SMOTOR_KP, DRIVE_SMOTOR_KI, DRIVE_SMOTOR_KD)) fatal(LM_DRIVE);
	if (!lbDrive.init(DRIVE_HMOTOR_KP, DRIVE_HMOTOR_KI, DRIVE_HMOTOR_KD)) fatal(LB_DRIVE);
	if (!lbSteer.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(LB_STEER);
	if (!lfDrive.init(DRIVE_HMOTOR_KP, DRIVE_HMOTOR_KI, DRIVE_HMOTOR_KD)) fatal(LF_DRIVE);
	if (!lfSteer.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(LF_STEER);
	if (!rmSteer.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(RM_STEER);
	if (!rbDrive.init(DRIVE_HMOTOR_KP, DRIVE_HMOTOR_KI, DRIVE_HMOTOR_KD)) fatal(RB_DRIVE);
	if (!rbSteer.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(RB_STEER);
	if (!rmDrive.init(DRIVE_SMOTOR_KP, DRIVE_SMOTOR_KI, DRIVE_SMOTOR_KD)) fatal(RM_DRIVE);
	if (!rfSteer.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD)) fatal(RF_STEER);
	if (!rfDrive.init(DRIVE_HMOTOR_KP, DRIVE_HMOTOR_KI, DRIVE_HMOTOR_KD)) fatal(RF_DRIVE);
}

//! Set all drive motors running at the given speed
void setAllDriveSpeed(float rqSpeed)
{
	lfDrive.setSpeed(rqSpeed);
	rfDrive.setSpeed(rqSpeed);
	lmDrive.setSpeed(rqSpeed);
	rmDrive.setSpeed(rqSpeed);
	lbDrive.setSpeed(rqSpeed);
	rbDrive.setSpeed(rqSpeed);
}

//! Set all steering motors to the given angle
void setAllSteerAngle(int rqDegrees)
{
	lfSteer.setDegrees(rqDegrees);
	rfSteer.setDegrees(rqDegrees);
	lmSteer.setDegrees(rqDegrees);
	rmSteer.setDegrees(rqDegrees);
	lbSteer.setDegrees(rqDegrees);
	rbSteer.setDegrees(rqDegrees);
}

void updateAllMotors()
{
	if (!lmSteer.updateMotor()) fatal(LM_STEER);
	if (!lmDrive.updateMotor()) fatal(LM_DRIVE);
	if (!lbDrive.updateMotor()) fatal(LB_DRIVE);
	if (!lbSteer.updateMotor()) fatal(LB_STEER);
	if (!lfDrive.updateMotor()) fatal(LF_DRIVE);
	if (!lfSteer.updateMotor()) fatal(LF_STEER);
	if (!rmSteer.updateMotor()) fatal(RM_STEER);
	if (!rbDrive.updateMotor()) fatal(RB_DRIVE);
	if (!rbSteer.updateMotor()) fatal(RB_STEER);
	if (!rmDrive.updateMotor()) fatal(RM_DRIVE);
	if (!rfSteer.updateMotor()) fatal(RF_STEER);
	if (!rfDrive.updateMotor()) fatal(RF_DRIVE);
}

//! Just initialize the first servo then exit
void jRoverTest1()
{
	// initialize motors up to the first servo. This must be done in order of the I2C chain
	printf("test 1\n");
	lmSteer.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD);
}

//! Just initialize the first two servos then exit
void jRoverTest2()
{
	// initialize motors up to the first servo. This must be done in order of the I2C chain
	printf("test 2\n");
	lmSteer.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD);
	lmDrive.init(DRIVE_SMOTOR_KP, DRIVE_SMOTOR_KI, DRIVE_SMOTOR_KD);
	lbDrive.init(DRIVE_HMOTOR_KP, DRIVE_HMOTOR_KI, DRIVE_HMOTOR_KD);
	lbSteer.init(SERVO_SMOTOR_KP, SERVO_SMOTOR_KI, SERVO_SMOTOR_KD);
}

//! initialize all motors
void jRoverTest3()
{
	printf("test 3\n");
	// initialize all motors, in order of the I2C chain
	initMotors();
}

#define SPEED_RUN_TIME 100
//! Ramp the drive motors
void jRoverTest4()
{
	float rampSpeeds[] = {5.0, 10.0, 15.0, 0.0, -5.0, -10.0, -15.0, 0.0};
	int speedRunTime;
	int rampCount = 8;
	int i;

	printf("test 4\n");
	initMotors();					// always start with this
	Metro metro = Metro(50);		// 50ms metronome

	// iterate over the ramp speeds, setting all motors to each value
	for (i=0; i<rampCount; i++) {	// for each speed setting
		setAllDriveSpeed(rampSpeeds[i]);	// set all motors to that speed
		speedRunTime = SPEED_RUN_TIME;		// set how long we'll run at this speed

		// while running at this speed, keep updating motors
		while(speedRunTime) {
			if (metro.check()) {		// if 50ms have passed & it's time to do the control loop
				updateAllMotors();			// run the PID loop
				speedRunTime--;
			}
		}
	}
}

void jRoverTest5()
{
	float driveAngVel[] = {0.0, 0.1, 0.3, 0.5, 0.0, -0.1, -0.3, -0.5, 0.0};
	int rampCount = 9;
	float linear = 10.0;
	int speedRunTime;
	int i;

	printf("test 5\n");
	initMotors();					// always start with this
	Metro metro = Metro(50);		// 50ms metronome

	// iterate over the angular velocities, setting all motors to each value
	for (i=0; i<rampCount; i++) {	// for each speed setting
		driveRover(linear, driveAngVel[i]);	// calculate speed for all motors & set them to run at it
		speedRunTime = SPEED_RUN_TIME;		// set how long we'll run at this speed

		// while running at this speed, keep updating motors
		while(speedRunTime) {
			if (metro.check()) {		// if 50ms have passed & it's time to do the control loop
				updateAllMotors();			// run the PID loop
				speedRunTime--;
			}
		}
	}
}

void jRoverTest6()
{
	float linear;
	float angularVelocity;

	printf("test 6\n");

	if (rosFlag){
		// initialize ROS
		nh.initNode(rosSrvrIp);
		nh.advertise(motorTelemetry);
	}

	initMotors();					// always start with this

	// initialize the R/C steering struct & start interrupt monitoring
	dirRcc.flag = 1;
	dirRcc.dioIndex = 0;
	initRCTest(gpio, &dirRcc);

	// initialize the R/C speed struct & start interrupt monitoring
	speedRcc.flag = 1;
	speedRcc.dioIndex = 1;
	initRCTest(gpio, &speedRcc);


	// check motors every 50ms, and R/C every 250ms
	Metro metro = Metro(50);
	Metro metro250 = Metro(250);

	driveRover(0.0, 0.0);		// start off stopped
	while (1){
		if (metro250.check()) {
			// convert R/C values to desired speed range -20 - +20 ips & angle range +/-90
			if ((speedRcc.pulseWidth == 0) || (dirRcc.pulseWidth == 0)) {
				linear = angularVelocity = 0.0;
			} else {
				linear = 0.0 - (float)(speedRcc.pulseWidth-1460)/25.0;
				angularVelocity = 0.0 - (float)(dirRcc.pulseWidth-1500)/1000.0;
			}
			driveRover(linear, angularVelocity);
		}
		if (metro.check()) {		// if 50ms have passed & it's time to do the control loop
			updateAllMotors();			// run the PID loop
			range.data = rfSteer.getDegrees();
			if (rosFlag)
				motorTelemetry.publish( &range );
		}
		if (rosFlag)
			nh.spinOnce();
	}
}

void usage()
{
    printf("Usage: jRover [OPTIONS]\n"
    "\n"
    "Options:\n"
    "  -h, --help                   Print this help message\n"
    "  -t<testnum>					Run the specified test\n"
    "  -r							Initialize ROS & publish to it\n"
    "Tests:\n"
    "1: initialize the first steering motor\n"
    "2: initialize the rirst 2 steering motors\n"
    "3: initialize all  motors\n"
    "4: initialize all motors then ramp the drive motors"
    "5: init motors then drive with driveRover, setting correct motor speeds/angles"
    "6: R/C control"
    "\n"
    );
}


int main(int argc, char *argv[])
{
	int i;
	int testNum = DEFAULT_TEST;			// which test to execute

	// if no arguments (e.g. run from TerkIDE) use default arguments from list above
    if (argc == 1) {
    	printf("No args provided; using default: ");
    	for(i=0; i<default_argc; i++) {
    		printf("%s ", default_argv[i]);
    	}
    	printf("\n");
    	argc = default_argc;
    	argv = default_argv;
    }

    // parse options & arguments
    int option_index = 0, opt;
    static struct option loptions[] = {
        {"help",       no_argument,       0, 'h'},
        {"ros",			no_argument,	0, 'r'},
        {0,0,0,0}
    };

    // initialize the encoder table. Encoder table entry should match motor #

    while(1) {
        opt = getopt_long (argc, argv, "hrt:",
                           loptions, &option_index);
        if (opt==-1) break;

        switch (opt) {
        case 'h': usage(); break;
        case 'r': rosFlag = true; break;
        case 't': testNum = strtol(optarg, NULL, 10); break;
        default: usage(); exit(0);
        }
    }

    switch (testNum) {
    case 1: jRoverTest1(); break;
    case 2: jRoverTest2(); break;
    case 3: jRoverTest3(); break;
    case 4: jRoverTest4(); break;
    case 5: jRoverTest5(); break;
    case 6: jRoverTest6(); break;
    default: printf("Invalid test number\n"); exit(0);
    }
}
