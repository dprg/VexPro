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
	layerName = "Spin";
	roombaSensors = rs;
	degrees = 0;
}

void Spin::setSpin(int degIn)
{
	degrees = (float)degIn;
}

void Spin::eval()
{
	float dt = roombaSensors->getDeltaTheta();		// positive for a clockwise turn
	dt *= RADS;		// convert deltaTheta to degrees
	degrees -= dt;	// update number of degrees we have left to turn
	//printf("degrees: %4.1f\n", degrees);

	// set defaults, then figure out how to modify them based on how much we still have to spin
	velRqst = 50;
	flag = 1;
	if (degrees > 1.0) {	// deadband
		rotRqst = -1;
	} else if (degrees < -1.0) {
		rotRqst = 1;
	} else {
		velRqst = 0;
		flag = 0;		// reached target direction
	}
}
