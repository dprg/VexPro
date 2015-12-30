/*
 * Spin.cpp
 *
 *  Created on: Jan 29, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include "Spin.h"
#include "RoombaSensors.h"

Spin::Spin(RoombaSensors *rs) {
	velRqst = 50;		// initialize the Spin layer. Other variables initialized in Layer
	flag = 0;
	layerName = (char *)"Spin";
	roombaSensors = rs;
	degreesToSpin = 0;
}

int Spin::headingChange(int target, int heading)
{
	int diff = target - heading;
	if (diff > 180) diff -= 360;
	else if (diff < -180) diff += 360;
	return diff;
}

void Spin::setSpin(int degIn)
{
	degreesToSpin = (float)degIn;
	flag = 1;
}

void Spin::spinTo(int degIn)
{
	degreesToSpin = this->headingChange(degIn, (roombaSensors->getTheta() * RADS));
	printf("spinTo set target of %d, spinning %f\n", degIn, degreesToSpin);
	flag = 1;
}

void Spin::eval()
{
	if (flag == 0)
		return;
	float dt = roombaSensors->getDeltaTheta();		// positive for a clockwise turn
	dt *= RADS;		// convert deltaTheta to degrees
	degreesToSpin -= dt;	// update number of degrees we have left to turn
	//printf("degrees: %4.1f\n", degrees);

	// set defaults, then figure out how to modify them based on how much we still have to spin
	velRqst = 50;
	flag = 1;
	if (degreesToSpin > 1.0) {	// deadband
		rotRqst = -1;
	} else if (degreesToSpin < -1.0) {
		rotRqst = 1;
	} else {
		velRqst = 0;
		flag = 0;		// reached target direction
	}
}
