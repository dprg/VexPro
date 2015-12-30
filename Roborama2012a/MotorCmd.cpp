/*
 * MotorCmd.cpp
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include "roombalib.h"
#include "MotorCmd.h"
#include "Layer.h"

MotorCmd::MotorCmd(Roomba *roombaIn) {
	// Force motors to stop
	roomba = roombaIn;
	lastVel = 0;
	lastRot = 0;
}

MotorCmd::~MotorCmd() {
	// TODO Auto-generated destructor stub
}

void MotorCmd::setSpeed(Layer * activeLayer)
{
	int vel, rot;

	switch (activeLayer->getCmd()) {
	case VEL_CMD:
		vel = activeLayer->getVel();
		rot = activeLayer->getRot();
		if ((vel != lastVel) || (rot != lastRot)) {
			printf("\nActive layer %s, vel: %d rot: %d\n", activeLayer->getLayerName(), vel, rot);
		}
		lastVel = vel;
		lastRot = rot;

		if (rot == 0) {
			//printf("Driving forward at %d\n", cmd);
			roomba_forward_at(roomba, vel);
		} else if (rot == 1) {
			//printf("Spin right at %d\n", arg);
			roomba_spinleft_at(roomba, vel);
		} else if (rot == -1) {
			//printf("Spin left at %d\n", arg);
			roomba_spinright_at(roomba, vel);
		} else {
			roomba_drive(roomba, vel, rot);
		}
		break;
	case ROOMBA_MODE:
		printf("setting roomba mode to %d\n", activeLayer->getCmdArg());
		roomba_mode(roomba, activeLayer->getCmdArg());
		activeLayer->setCmd(VEL_CMD);		// set mode back to velocity
		break;
	default:
		printf("Error: invalid motor command %d\n", activeLayer->getCmd());
		return;
	}
}

void MotorCmd::printData()
{
	printf("lastVel: %d lastRot: %d\n", lastVel, lastRot);
}
