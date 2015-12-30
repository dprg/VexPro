/*
 * Point2VisTarget.cpp
 *
 *  Created on: May 9, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include "Point2VisTarget.h"
#include "Spin.h"
#include "RoombaSensors.h"
#include "rrclient.h"

// shared memory defines
#define KEY (key_t)6060
#define SHM_SZ 1024
#define SHMFLAGS 0666
void *shm_p;


Point2VisTarget::Point2VisTarget(RoombaSensors *rsIn, Spin *sIn)
{
	velRqst = 0;	// initialize the layer to ask to stop the robot
	rotRqst = 0;
	flag = 0;		// initially inactive
	layerName = (char *)"Point2VisTarget";
	rs = rsIn;
	s = sIn;
	state = 0;

	// seeking variables
	seekHeadingIncrement = 5;
	seekLimit = MAX_SEEKS;	// how many seeks until we give up
	seeking = false;
	seekLeft = true;
	maxSeekConfidence = 0;
	mostConfidentIteration = 0;
}

/**
 * Attach the shared memory segment so we can request the X coordinate of the target
 */
void Point2VisTarget::shmInit()
{
	fprintf(stderr, "attaching shared memory for shape data requests\n");
	int shmid;

	if ((shmid = shmget(KEY, SHM_SZ, SHMFLAGS)) == -1) {
		perror("shmget");
		exit(errno);
	}

	if ((shm_p = (void *)shmat(shmid, (void*)0, 0)) == (void*)-1) {
		perror("shmat");
		exit(errno);
	}
	fprintf(stderr, "attached shared memory at address 0x%x\n", (int)shm_p);

	// check that shared memory struct is initialized
	rrClientStruct rrc;
	memcpy(&rrc, shm_p, sizeof(rrClientStruct));	// initialize struct in shared memory to tell client we're available
	if (rrc.structInitialized != 6060) {
		fprintf(stderr, "shared memory struct rrClientStruct not initialized, start rrclient. Exiting\n");
		exit(-1);
	}

}

/**
 * activate should only be called when this layer is going to become active, because
 * it needs to delay for an image to get captured & analyzed. Effectively, this means it
 * should only be activated by a layer below it. Activating will result in it determining
 * (if possible) where the target is, sending a spin request to Spin, storing the target
 * offset in roombaSensors, where stopBot can determine if it should continue visually seeking,
 * or move or do something else.
 */
void Point2VisTarget::activate()
{
	state = 1;		// kick off the state machine to seek to pointing at the visual target
}
void Point2VisTarget::eval()
{
	rrClientStruct rrc;

	switch (state) {
	case 0:
		seekIteration = 1;
		break;		// do nothing, inactive
	case 1:
		flag = 1;	// request to stop the robot
		stateTimer = 40;	// give it 2 sec to stop & stabilize image
		state = 2;
		break;
	case 2:
		if (!stateTimer--) { // wait for robot to stop, & image to stabilize
			rrc.getShapeParams = 1;		// ask for the x coord of shape
			memcpy(shm_p, &rrc, sizeof(rrClientStruct));	// set request in shared mem struct
			stateTimer = 200;	// give it a 10 second to find the object & return params
			state = 3;
		}
		break;
	case 3:
		memcpy(&rrc, shm_p, sizeof(rrClientStruct));	// get shared mem struct which may have target params
		rs->setVisTargetValid(false);
		if (stateTimer-- == 0) {
			flag = 0;
			state = 0;
			fprintf(stderr, "ERROR: FAILED TO GET OBJECT PARAMETERS IN 10 SEC\n");
		} else if (rrc.getShapeParams == 0) {	// got a result back from rrclient
			if (rrc.shapeParams[0] > 0) {		// and it's valid - sufficient confidence
				rs->setVisTargetValid(true);
				rs->setVisTargetBearing(rrc.shapeParams[1]);

				degreesToSpin = rrc.shapeParams[1];
				fprintf(stderr, "got bearing to visual target, confidence: %d, angle: %d, spinning bot to point\n", rrc.shapeParams[0], rrc.shapeParams[1]);
				s->setSpin(degreesToSpin);
				state = 0;
				flag = 0;
			} else {
				// alternately spin in opposite directions seeking target
				seeking = true;
				degreesToSpin = seekIteration * seekHeadingIncrement;		// spin all the way left
				s->setSpin(degreesToSpin);		// go off & spin to first seek direction
				if (seekIteration > 0)
					seekIteration++;
				else
					seekIteration--;
				seekIteration = 0 - seekIteration;
				fprintf(stderr, "ERROR: Got parameters but too low confidence, seeking %d degrees, seekIteration is %d\n", degreesToSpin, seekIteration);

				// limit how long we seek
				if (seekIteration > MAX_SEEKS) {
					flag = 0;
					state = 0;
				} else {
					flag = 1;
					state = 1;
				}
			}
		}
		break;
	case 4:
		fprintf(stderr, "Point2VisTarget state 4 - seeking object\n");

		break;
	}
}

void Point2VisTarget::spin()
{

	currentSeekHeading = rs->getTheta() * RADS;	// convert current heading to degrees

	// seek from one end to other recording confidence at each stopping point
	for (seekIteration=0; seekIteration<seekLimit; seekIteration++) {
		currentSeekHeading = normalizeCompassHeading(currentSeekHeading);	// fixup wraparound
		s->spinTo(currentSeekHeading);		// point in the next direction to search
		currentSeekHeading += seekHeadingIncrement;	// increment direction for next iteration

	}

}

int Point2VisTarget::normalizeCompassHeading(int heading)
	{
		if (heading > 359)
			return (heading % 360);
		else if (heading < 0)
			return (heading + 360);
		return heading;
	}
