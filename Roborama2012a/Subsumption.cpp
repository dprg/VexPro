/*
 * bumpturn.c
 *
 *  Created on: Jan 20, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include <keypad.h>
#include <textlcd.h>
#include "roombalib.h"
#include "Subsumption.h"
#include "Metro.h"
#include "Layer.h"
#include "WheelDrop.h"
#include "Bump.h"
#include "Grab.h"
#include "VirtualCage.h"
#include "Spin.h"
#include "Target.h"
#include "Cruise.h"
#include "Point2VisTarget.h"
#include "MissionControl.h"
#include "Stop.h"
#include "MotorCmd.h"
#include "RoombaSensors.h"


#define RUNTIME 0

CKeypad &keypad = CKeypad::GetRef();
CTextLcd &lcd = CTextLcd::GetRef();
Subsumption *sub_p;

// Explore a space using bump-turn & other algorithms
void startSubsumption(int algorithm, int arg, int *waypointList, Roomba *roomba) {
	int rv;

	sub_p = new Subsumption(algorithm, arg, waypointList, roomba);
	rv = sub_p->go();
	printf("Subsumption.go ended with status %d\n", rv);
	return;
}
/**
 * Constructor is responsible for understanding the requested algorithm & setting up the layers properly.
 * Each run only uses one algorithm.
 */
Subsumption::Subsumption(int algorithmIn, int paramIn, int *wpl, Roomba *roombaIn) {
	// initialize subsumption object
	algorithm = algorithmIn;
	param = paramIn;
	printf("Starting subsumption with algorithm %d\n", algorithm);
	roomba = roombaIn;
	endMillis = millis() + RUNTIME;	// set endtime
	heartMetro = new Metro(50);		// metronome ticks every 50ms
	wayPointList = wpl;

	// create other robot objects, & layer objects
	roombaSensors = new RoombaSensors(roomba);
	motorCmd = new MotorCmd(roomba);
	grab = new Grab(roombaSensors);
	bump = new Bump(roombaSensors, roomba->velocity);
	virtualCage = new VirtualCage(roombaSensors,roomba->velocity, 40, 40);
	spin = new Spin(roombaSensors);
	target = new Target(roombaSensors);
	wheeldrop = new WheelDrop(roombaSensors);
	cruise = new Cruise(roomba->velocity);
	point2VisTarget = new Point2VisTarget(roombaSensors, spin);
	missionControl = new MissionControl(roombaSensors, target, point2VisTarget, spin, bump, cruise);
	stopBot = new StopBot(roombaSensors);


	// initialize subsumption algorithms
	switch (algorithm) {
	case 0:
		// algorithm 0: just wanders around a space bouncing off walls & cliffs.
		// cruise layer runs forever
		alg0Layers[0] = wheeldrop;
		alg0Layers[1] = bump;
		alg0Layers[2] = cruise;
		alg0Layers[3] = stopBot;
		job = &alg0Layers[0];
		job_size = 4;		// default, stopped
		break;
	case 1:			// algorithm 1 seeks to a list of waypoints passed in with waypointLIst
		// algorithm 1 takes a list of targets
		// target layer replaces cruise layer in wander algorithm, & drives through a list of waypoints
		alg1Layers[0] = wheeldrop;
		alg1Layers[1] = bump;
		alg1Layers[2] = target;
		target->setTargets(wayPointList);
		alg1Layers[3] = stopBot;
		job = &alg1Layers[0];
		job_size = 4;
		break;
	case 2:
		// algorithm 2 spins in place a number of degrees
		alg2Layers[0] = spin;
		spin->setSpin(param);
		job = alg2Layers;
		job_size = 1;
		break;
	case 3:
		// algorithm 3 bounces around inside a virtual cage
		// cruise layer runs forever
		algLayers[0] = wheeldrop;
		algLayers[1] = bump;
		algLayers[2] = virtualCage;
		algLayers[3] = cruise;
		algLayers[4] = stopBot;
		job = &algLayers[0];
		job_size = 5;
		break;
	case 4:
		// algorithm 4 drives a distance and grabs a can if one comes in range. Grabber is active
		algLayers[0] = wheeldrop;
		algLayers[1] = bump;
		algLayers[2] = grab;
		algLayers[3] = target;
		target->setTargets(wayPointList);
		algLayers[4] = stopBot;
		job = &algLayers[0];
		job_size = 5;
		break;
	case 5:
		// algorithm 5 spins back and forth to find the visual target and end up pointing to it
		algLayers[0] = spin;
		algLayers[1] = target;
		algLayers[2] = point2VisTarget;
		point2VisTarget->shmInit();
		algLayers[3] = missionControl;
		algLayers[4] = stopBot;
		job = &algLayers[0];
		job_size = 5;
		break;
	case 6:
		// algorithm 6 runs Robocolumbus
		algLayers[0] = bump;
		algLayers[1] = spin;
		algLayers[2] = target;
		algLayers[3] = point2VisTarget;
		point2VisTarget->shmInit();
		algLayers[4] = missionControl;
		algLayers[5] = stopBot;
		job = &algLayers[0];
		job_size = 6;		// default, stopped
		break;
	case 7:
		// algorithm 6 runs TableBot
		algLayers[0] = grab;
		algLayers[1] = bump;
		algLayers[2] = spin;
		algLayers[3] = target;
		algLayers[4] = cruise;
		algLayers[5] = point2VisTarget;
		point2VisTarget->shmInit();
		algLayers[6] = missionControl;
		algLayers[7] = stopBot;
		job = &algLayers[0];
		job_size = 8;		// default, stopped
		break;
	default:
		// invalid algorithm
		fprintf(stderr, "invalid algorithm %d, exiting\n", algorithm);
		exit(-1);
	}
	this_layer = job[0];	// start with first layer in current job
	arbitrateEnable = 1;
	halt = 0;

	// initialize subsumption layers for this algorithm

}

int Subsumption::go() {
	int loopCnt = 0;
	int rv;
	missionControl->setSubPtr(sub_p);	// set pointer to Subsumption object in missionControl

	// main run loop for exploring
	if (!roombaSensors->healthCheck()) {
		printf("roombaSensors::healthCheck failed, exiting subsumption\n");
		roomba_delay(COMMANDPAUSE_MILLIS);
		return -1;
	}

	lcd.Clear();
	lcd.printf("Press X to exit");

	while (((millis() < endMillis) || (RUNTIME == 0)) && !keypad.KeyCancel()) {
		if (heartMetro->check()) {
			// metronome has ticked, time to run the algorithm
			now = millis();
			sprintf(timestampString, "%d.%03d:", now/1000, now%1000);

			// read sensor data
			roombaSensors->readSensors(timestampString);	// read sensors from Roomba

			// evaluate each layer & choose the active one & pass its output to the motors
			rv = arbitrate();		// do the subsumption layer evaluation & arbitration

			// quit if no layers want to control robot
			if (rv < 0)
				break;				// quit running subsumption & go do something more interesting

			// periodically print debug data
			if (!((loopCnt++)%10)) {
				printf("%s: ", timestampString);
				roombaSensors->printData();
				//target->printData();
				//motorCmd->printData();
			}
		}
	}

	// finished this run, return & maybe get asked to do another run
	roomba_stop(roomba);
	grab->openGrabber();
	return 0;
}

/**
 * @return 0 for normal return, -1 if no layers want to run. This is a way to stop processing.
 */
int Subsumption::arbitrate()
{
    int i = 0;

    if (arbitrateEnable) {
    	// step through all Layer objects running the eval method, so any side-effects always happen
//        for (i = 0; i < job_size; i++) {
//            job[i]->eval();
//        }

        // step through Layer objects in priority order until we find the first one that wants to take control
        for (i = 0; i < job_size; i++) { // step through tasks
        	job[i]->eval();				// evaluate whether this layer wants to run
            if (job[i]->getFlag()) break;       // if yes, subsume lower priority layers by breaking to run its motor request
        }

        if ((i == job_size) && (!(job[i-1]->getFlag()))) {	// iterated through last layer & its flag is clear
        	printf("arbitrate: no layers want to run\n");
        	return -1;					// no layers want to run
        }
        else {
        	this_layer = job[i];        // highest priority layer is winner
        	activeLayerName = this_layer->getLayerName();
        }
        motorCmd->setSpeed(this_layer);             // send command to motors
        return 0;
    }
    return 0;
}


