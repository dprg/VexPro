/*
 * Stop.cpp
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include <math.h>
#include "Stop.h"
#include "RoombaSensors.h"

StopBot::StopBot(RoombaSensors *rs) {
	velRqst = 0;	// initialize the stop layer
	rotRqst = 0;
	flag = 1;
	layerName = (char *)"stop";
	roombaSensors = rs;
}

void StopBot::eval() {
	float th = roombaSensors->getTheta();
	/*
	th = fmod(th, TWOPI);
	if (th > TWOPI/2.0)
		th -= TWOPI;
	else if (th < TWOPI/2.0)
		th += TWOPI;
		*/
	//printf("Rotating back to theta = 0 from theta = %0.1f\n", th);
	flag = 1;	// StopBot always tries to run

	// if StopBot gets to run, it tries to drive it to theta = 0 before stopping
	if (th > 0.1) {
		velRqst = 50;
		rotRqst = 1;
	} else if (th < -0.1) {
		velRqst = 50;
		rotRqst = -1;
	} else {
		velRqst = 0;
		rotRqst = 0;
		flag = 0;		// indicate stop doesn't want to control the robot anymore - should stop if this happens
		//printf("StopBot cleared run flag\n");
	}
}
