/*
 * MissionControl.cpp
 *
 *  Created on: May 10, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include <math.h>
#include "roombalib.h"
#include "MissionControl.h"
#include "Subsumption.h"
#include "RoombaSensors.h"
#include "Target.h"
#include "Point2VisTarget.h"
#include "Spin.h"
#include "Bump.h"
#include "Cruise.h"

MissionControl::MissionControl(RoombaSensors *rsIn, Target *tIn, Point2VisTarget *pvIn, Spin *sIn, Bump *bIn, Cruise *cIn)
{
	velRqst = 0;	// initialize the Mission Control layer - always wants to run, always stops robot
	rotRqst = 0;
	flag = 1;
	layerName = (char *)"missionControl";

	// save away the pointers to layers this class manipulates
	rs = rsIn;
	t = tIn;
	pv = pvIn;
	s = sIn;
	b = bIn;
	c = cIn;

	state = 0;
	target[0] = 1;
	target[1] = 0;
	target[2] = 0;
}

void MissionControl::setSubPtr(Subsumption *subIn)
{
	sub = subIn;
}

void MissionControl::eval()
{
	switch(sub->getAlgorithm()) {
	case 5:
		point2TargetSm();
		break;
	case 6:
		roboColumbusSm();
		break;
	case 7:
		tableBotSm();
		break;
	default:
		flag = 0;	// exit if we get called with an invalid algorithm
	}
}

void MissionControl::point2TargetSm()
{
	switch (state) {
	case 0:
		fprintf(stderr, "starting point2Target state machine\n");
		pv->activate();
		state = 1;
		stateTimer = 500;
		break;
	case 1:
		if (!stateTimer--) {	//Time out this state & quit
			flag = 0;
			state = 0;
		}
		if (rs->getVisTargetValid()) {
			fprintf(stderr, "found target at range %d\n", rs->getSonarRange());
			flag = 1;
			state = 0;
		}
		break;
	default:
		state = 0;
	}
}

void MissionControl::roboColumbusSm()
{
	int seekDistance;
	float seekAngle;

	switch (state) {
	case 0:
		target[2] = sub->getParam();	// set target Y coordinate to the argument passed with -a
		t->setTargets(target);			// set the target in Target
		t->setSeekSpeed(200);

		state = 1;
		fprintf(stderr, "starting roboColumbus state machine\n");
		break;
	case 1:		// have traveled initial distance, now point to target
		fprintf(stderr, "arrived at destination, starting Point2Target state machine\n");
		pv->activate();
		b->setBounceOffBumps(false);	// get bump to stop the robot when it touches cone
		t->setSeekSpeed(70);

		state = 2;
		stateTimer = 500;
		break;
	case 2:		// wait for target to be acquired, then travel toward it. First detect if we already arrived
		if (!stateTimer--) {	//Time out this state & quit
			flag = 0;
			state = 0;
		}
		if (rs->getLBumper() || rs->getRBumper()) {
			flag = 0;
			state = 0;
			fprintf(stderr, "RoboColumbus completed mission by hitting something\n");
			break;
		}
		if (rs->getVisTargetValid()) {
			fprintf(stderr, "found target at sonar range %d\n", rs->getSonarRange());
			if (rs->getSonarRange() < 24) {
				seekDistance = rs->getSonarRange() + 5;
			} else {
				seekDistance  = 12;		// seek 12" if we haven't found target with sonar yet
			}
			seekAngle = rs->getTheta();	// we want to travel in current direction
			target[1] = rs->getX_pos() + (int)(seekDistance * sin(seekAngle));
			target[2] = rs->getY_pos() + (int)(seekDistance * cos(seekAngle));
			t->setTargets(target);
			fprintf(stderr, "seeking toward target, traveling %d inches to X: %d Y: %d at angle %0.1f",
					seekDistance, target[1], target[2], seekAngle * RADS);
			flag = 1;
			state = 1;		// next time we visually search for target again
		}
		break;
	default:
		state = 0;
	}
}

void MissionControl::tableBotSm()
{
	int seekDistance;
	float seekAngle;

	switch (state) {
	case 0:
		target[2] = sub->getParam();	// set target Y coordinate to the argument passed with -a
		t->setTargets(target);			// set the target in Target
		state = 1;
		fprintf(stderr, "starting tableBot state machine\n");
		break;
	case 1:		// have traveled initial distance, now head back to home
		target[2] = 0;
		t->setTargets(target);			// set the target in Target
		state = 2;
		fprintf(stderr, "heading back to start point\n");
		break;
	case 2:
		s->spinTo(0);
		state = 3;
		fprintf(stderr, "spinning back to 0 degrees\n");
		break;
	case 3:
		fprintf(stderr, "starting Point2Target state machine\n");
		pv->activate();
		state = 4;
		stateTimer = 500;
		break;
	case 4:		// wait for target to be acquired, then travel toward it. First detect if we already arrived
		if (!stateTimer--) {	//Time out this state & quit
			flag = 0;
			state = 0;
		}
		if (rs->getGrabberOpen() == false) {	// we've grabbed the can
			state = 5;
			fprintf(stderr, "tableBot grabbed a can\n");
			s->spinTo(90);		// point at side of table
			break;
		}
		if (rs->getVisTargetValid()) {
			fprintf(stderr, "found target at sonar range %d\n", rs->getSonarRange());
			if (rs->getSonarRange() < 24) {
				seekDistance = rs->getSonarRange();
			} else {
				seekDistance  = 6;		// seek 6" if we haven't found target with sonar yet
			}
			seekAngle = rs->getTheta();	// we want to travel in current direction
			target[1] = rs->getX_pos() + (int)(seekDistance * sin(seekAngle));
			target[2] = rs->getY_pos() + (int)(seekDistance * cos(seekAngle));
			t->setTargets(target);
			fprintf(stderr, "seeking toward target, traveling %d inches to X: %d Y: %d at angle %0.1f",
					seekDistance, target[1], target[2], seekAngle * RADS);
			state = 3;		// next time we visually search for target again
		}
		break;
	case 5:	// wait for spin to 90 to end, and start heading for the edge of the table
		c->setCruiseEnable(true);	// cruise to the edge of the table. The grabber should open & bump should back away
		if (rs->getCliff()) {
			fprintf(stderr, "found the cliff, turning off cruise\n");
			c->setCruiseEnable(false);
			state = 6;
		}
		break;
	case 6:			// wait till we've backed away from the cliff
		if (!rs->getCliff()) {
			target[2] = 0;
			t->setTargets(target);			// set the target in Target
			state = 7;
			fprintf(stderr, "heading back to start point\n");
		}
		break;
	case 7:
		s->spinTo(0);
		state = 8;
		fprintf(stderr, "spinning back to 0 degrees\n");
		break;
	case 8:
		fprintf(stderr, "starting Point2Target state machine\n");
		pv->activate();
		state = 9;
		stateTimer = 500;
		break;
	case 9:
		if (!stateTimer--) {	//Time out this state & quit
			flag = 0;
			state = 0;
		}
		if (rs->getGrabberOpen() == false) {	// we've grabbed the can
			state = 10;
			fprintf(stderr, "tableBot grabbed a can\n");
			s->spinTo(0);		// point at end of table
			break;
		}
		if (rs->getVisTargetValid()) {
			fprintf(stderr, "found target at sonar range %d\n", rs->getSonarRange());
			if (rs->getSonarRange() < 24) {
				seekDistance = rs->getSonarRange();
			} else {
				seekDistance  = 6;		// seek 6" if we haven't found target with sonar yet
			}
			seekAngle = rs->getTheta();	// we want to travel in current direction
			target[1] = rs->getX_pos() + (int)(seekDistance * sin(seekAngle));
			target[2] = rs->getY_pos() + (int)(seekDistance * cos(seekAngle));
			t->setTargets(target);
			fprintf(stderr, "seeking toward target, traveling %d inches to X: %d Y: %d at angle %0.1f",
					seekDistance, target[1], target[2], seekAngle * RADS);
			state = 8;		// next time we visually search for target again
		}
		break;
	case 10:
		target[2] = sub->getParam();	// set target Y coordinate to the argument passed with -a - seek to near end
		t->setTargets(target);			// set the target in Target
		state = 11;
		fprintf(stderr, "seeking to end of table\n");
		break;
	case 11:
		c->setCruiseEnable(true); 	// cruise to the end of the table, where the gripper will open
		state = 12;
		break;
	case 12:
		c->setCruiseEnable(false);	// stop cruise
		state = 0;
		flag = 0;		// end of run
		break;

	default:
		state = 0;
	}
}
