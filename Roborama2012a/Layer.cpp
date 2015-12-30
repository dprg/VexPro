/*
 * Layer.cpp
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 */
#include <stdio.h>
#include "Layer.h"

Layer::Layer() {
	cmd = VEL_CMD;			// default is motion command
	rotRqst = 0;		// initialize the stop layer
	velRqst = 0;
	flag = 0;
	top_speed = 0;
	//printf("setting speed to 0\n");
}

Layer::Layer(int speed) {
	cmd = VEL_CMD;			// default cmd is motion command
	rotRqst = 0;		// initialize the stop layer
	velRqst = 0;
	flag = 0;
	top_speed = speed;
	//printf("setting speed to %d\n", speed);
}

int Layer::slew(int requested, int rate)	// FIXME - give this access to bot_speed
{
     if (requested > bot_speed) return bot_speed + rate;
     else if (requested < bot_speed) return bot_speed - rate;
     else return bot_speed;
}

