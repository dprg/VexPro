/*
 * Grab.cpp
 *
 *  Created on: May 8, 2012
 *      Author: bouchier
 */
#include <iostream>
#include <qeservo.h>
#include "Grab.h"
#include "RoombaSensors.h"

Grab::Grab(RoombaSensors *rsIn) : Layer() {
	rs = rsIn;
	velRqst = 0;		// initialize the Grab layer. Other variables initialized in Layer
	flag = 0;
	layerName = (char *)"Grab";
	axis = 1;
	servo = CQEServo::GetPtr();
	this->openGrabber();
	rs->setGrabberOpen(true);
}

void Grab::eval()
{
	int cliff = rs->getCliff();
	unsigned long range = rs->getSonarRange();

	//printf("range %uld\n", range);
	if (cliff) {
		servo->SetCommand(axis, 250);
		rs->setGrabberOpen(true);
	} else if (range < 4) {
		servo->SetCommand(axis, 0);
		rs->setGrabberOpen(false);
	}
}

void Grab::openGrabber()
{
	servo->SetCommand(axis, 250);
}
