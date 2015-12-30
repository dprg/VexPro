/*
 * VirtualCage.cpp
 *
 *  Created on: Jan 29, 2012
 *      Author: bouchier
 */
#include <math.h>
#include <stdio.h>
#include "VirtualCage.h"
#include "RoombaSensors.h"

VirtualCage::VirtualCage(RoombaSensors *rs, int speed, int cageXIn, int cageYIn) {
	velRqst = speed;
	cageXMax = cageXIn/2;
	cageXMin = 0 - cageXIn/2;
	cageYMax = cageYIn;
	cageYMin = 0;
	rotRqst = 0;		// initialize the layer
	flag = 0;
	layerName = (char *)"VirtualCage";
	roombaSensors = rs;
	state = 0;
}

/**
 * Ballistic behavior runs a state machine when it detects excursion outside virtual box.
 * Two virtual sensors are emulated, each at radius 6" from robot center, and angle +/- 30
 * degrees
 */
#define VSENSOR_RADIUS 3.0
void VirtualCage::eval()
{
	float robotX, robotY, theta;
	float lVirtSensorX, lVirtSensorY, rVirtSensorX, rVirtSensorY;
	bool lVBump, rVBump;		// virtual bump sensors

	switch (state) {
	case 0:		// test for virtual box boundary excursions (virtual bumps)
		// get current position & angle
		robotX = roombaSensors->getX_pos();
		robotY = roombaSensors->getY_pos();
		theta = roombaSensors->getTheta();

		// compute position of virtual sensors, which are defined to be an an angle of 30 degrees to left & right of centerline
		// and 6" away from and in front of center of robot
		lVirtSensorX = (robotX + VSENSOR_RADIUS * sin(theta + 30));
		lVirtSensorY = (robotY + VSENSOR_RADIUS * cos(theta + 30));
		rVirtSensorX = (robotX + VSENSOR_RADIUS * sin(theta - 30));
		rVirtSensorY = (robotY + VSENSOR_RADIUS * cos(theta - 30));

		// Check if left or right sensor has gone out of bounds
		lVBump = (lVirtSensorX > cageXMax || lVirtSensorX < cageXMin || lVirtSensorY > cageYMax || lVirtSensorY < cageYMin);
		rVBump = (rVirtSensorX > cageXMax || rVirtSensorX < cageXMin || rVirtSensorY > cageYMax || rVirtSensorY < cageYMin);
		if (rVBump || lVBump) {
			velRqst = 0;	// stop
			rotRqst = 0;
			flag = 1;	// subsume lower priority behaviors

			// start the escape state machine
			state = 1;
			stateTimer = 3;		// 150ms stopped to stabilize
			roombaSensors->printData();
			if (rVBump)
				printf("Right virtual bump detected at X: %3.1f Y: %3.1f\n", rVirtSensorX, rVirtSensorY);
			else
				printf("Left virtual bump detected at X: %3.1f Y: %3.1f\n", lVirtSensorX, lVirtSensorY);
		}
		break;
	case 1:			// stop & wait for motion to cease
		stateTimer--;
		if (stateTimer)
			break;
		state = 2;
		stateTimer = 10;	// back up for 500ms
		velRqst = -100;			// go backwards, no change in flag or arg (still active)
		break;
	case 2:
		stateTimer--;
		if (stateTimer)
			break;
		state = 3;
		stateTimer = 20;	// rotate for 1000ms
		velRqst = 100;			// rotate in place, no change in flag (still active)
		if (rVBump)	// right cliff or right bumper
			rotRqst = 1;		// spin at 100, radius -1
		else
			rotRqst = -1;		// left or both bumpers, spin clockwise
		break;
	case 3:
		stateTimer--;
		if (stateTimer)
			break;
		state = 0;
		flag = 0;			// stop subsuming
		break;
	default:
		printf("Error in Bump state machine %d", state);
		state = 0;
	}
}
