/*
 * Target.cpp
 *
 *  Created on: Jan 28, 2012
 *      Author: bouchier
 */
#include <math.h>
#include <stdio.h>
#include "Target.h"
#include "RoombaSensors.h"

#define RADS 57.2958			/* radians to degrees conversion */
#define APPROACH_RADIUS 5
#define TARGET_RADIUS 1
#define TO_MM 25.4

Target::Target(RoombaSensors *rs) : Layer() {
	roombaSensors = rs;
	velRqst = 100;		// initialize the Target layer. Other variables initialized in Layer
	seekSpeed = 100;
	flag = 1;			// keep driving to targets until we arrive at the last one, then clear flag
	layerName = (char *)"Target";

	defaultTarget[0] = 1;
	defaultTarget[1] = 0;
	defaultTarget[2] = 0;

	targets = defaultTarget;
	targetCnt = defaultTarget[0];
	targetIndex = 0;	// first target location
	X_target = targets[2*targetIndex + 1];
	Y_target = targets[2*targetIndex + 2];
}

#define LOOP_GAIN 3

void Target::eval()
{
	if (flag == 0)
		return;		// already arrived

	// compute target location from where we are now
	locate_target();

	if (target_distance < TARGET_RADIUS) {		// reached the current target
		if (targetIndex == targetCnt-1) {
			flag = 0;		// finished course. Let stop layer take over
			printf("Target layer finished mission, reached target at X_target: %0.1f Y_target: %0.1f with X_pos: %0.1f Y_pos: %0.1f, theta: %0.1f",
					X_target, Y_target, roombaSensors->getX_pos(), roombaSensors->getY_pos(),
					roombaSensors->getTheta());
			return;
		} else {			// there's another target in the list - make it active & calculate a course to it
			printf("reached waypoint %d at X_target: %0.1f Y_target: %0.1f with X_pos: %0.1f Y_pos: %0.1f, theta: %0.1f\n",
					targetIndex, X_target, Y_target, roombaSensors->getX_pos(), roombaSensors->getY_pos(),
					roombaSensors->getTheta());
			targetIndex++;
			X_target = targets[2*targetIndex + 1];
			Y_target = targets[2*targetIndex + 2];
			locate_target();
			printf("seeking to next target at X: %0.1f Y: %0.1f\n", X_target, Y_target);
		}
	}
	// run at full speed until within 5" of target, then slow to half-speed
	if (target_distance < APPROACH_RADIUS) {
		velRqst = 50;
	} else {
		velRqst = seekSpeed;
	}

	// if heading error is greater than 30 degrees, spin in place until it's less than,
	// else set radius to 1/4 of distance to target
	if (heading_error > 45)
		rotRqst = -1;
	else if (heading_error < -45)
		rotRqst = 1;
	else if (heading_error > 2)	// deadband
		rotRqst = -1 - (int)((TO_MM * target_distance)/LOOP_GAIN);	// radius in mm is 1/4 of distance to target
	else if (heading_error < -2)
		rotRqst = 1 + (int)((TO_MM * target_distance)/LOOP_GAIN);
	else
		rotRqst = 0;
}

/**
 * calculate distance and bearing to target.
 * inputs are:  X_target, X_pos, and Y_target, Y_pos
 * outputs are: target_distance, heading_error
*/

void Target::locate_target()
{
	float x,y;

	x = X_target - roombaSensors->getX_pos();
	y = Y_target - roombaSensors->getY_pos();

	target_distance = sqrt((x*x)+(y*y));

	/* no divide-by-zero allowed! */
	if (x > 0.00001) target_bearing = 90.0 - (atan(y/x) * RADS);
	else if (x < -0.00001) target_bearing = -90.0 - (atan(y/x) * RADS);

	heading_error = target_bearing - (roombaSensors->getTheta()*RADS);
	if (heading_error > 180.0) heading_error -= 360.0;
	else if (heading_error < -180.0) heading_error += 360.0;
}

void Target::printData()
{
	printf(" target_bearing: %0.1f, heading_error %0.0f target_distance %0.1f ",
			target_bearing, heading_error, target_distance);
}

void Target::setTargets(int *targetsListIn)
{
	targets = targetsListIn;
	targetCnt = targets[0];
	targetIndex = 0;	// first target location
	X_target = targets[2*targetIndex + 1];
	Y_target = targets[2*targetIndex + 2];
	flag = 1;
	printf("Setting first target to X: %0.1f Y: %0.1f\n", X_target, Y_target);
}
