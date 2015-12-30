/*
 * Bump.cpp
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 */
#include <stdio.h>
#include "Bump.h"
#include "RoombaSensors.h"

Bump::Bump(RoombaSensors *rs, int speed) : Layer(speed) {
	velRqst = 0;
	rotRqst = 0;		// initialize the Bump layer
	flag = 0;
	layerName = (char *)"Bump";
	roombaSensors = rs;
	state = 0;
	bounceOffBumps = true;
}
/**
 * Ballistic behavior runs a state machine when it detects bumper press
 */
void Bump::eval()
{
	switch (state) {
	case 0:		// test for bumps
		// test for bumpers pressed
		rBump = roombaSensors->getRBumper();
		lBump = roombaSensors->getLBumper();
		cliff = roombaSensors->getCliff();
		if (rBump || lBump || cliff) {
			printf("Bump or cliff detected: L: %d R: %d Cliff %d\n", lBump, rBump, cliff);
			velRqst = 0;	// stop
			rotRqst = 0;
			flag = 1;	// subsume lower priority behaviors	and stop
			if (bounceOffBumps) {
				// start the escape state machine
				state = 1;
				stateTimer = 3;		// 150ms stopped to stabilize
			} else {
				printf("bounceOffBumps is false, stopping to let something else recover from bump\n");
				state = 5;	// we don't want to bounce off bumps, just stop
			}

			if (cliff) {
				printf("Escaping from cliff\n");
				stateTimer = 20;	// 1 second to get it into full mode
				cmd = ROOMBA_MODE;
				cmdArg = 3;
			}
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
		if (rBump || (cliff & 0xc))	// right cliff or right bumper
			rotRqst = 1;		// spin at 100, radius -1
		else
			rotRqst = -1;		// left or both bumpers, spin clockwise
		break;
	case 3:
		stateTimer--;
		if (stateTimer)
			break;
		if (!cliff) {
			state = 0;
			flag = 0;			// stop subsuming
		} else {
			state = 4;
			stateTimer = 20;	// 1 sec to switch back to safe mode
			cmd = ROOMBA_MODE;
			cmdArg = 2;
		}
		break;
	case 4:
		stateTimer--;
		if (stateTimer)
			break;
		state = 0;
		flag = 0;			// stop subsuming
		break;
	case 5:
		flag = 0;			// stop subsuming until bumpers are released. we've already stopped the bot
		// test for bumpers pressed
		rBump = roombaSensors->getRBumper();
		lBump = roombaSensors->getLBumper();
		cliff = roombaSensors->getCliff();
		if (!rBump && !lBump & !cliff) {
			state = 0;		// go back to normal checking for bumpers
		}

	default:
		printf("Error in Bump state machine %d", state);
		state = 0;
	}
}
