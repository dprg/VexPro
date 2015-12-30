/*
 * Target.h
 *
 *  Created on: Jan 28, 2012
 *      Author: bouchier
 *  The locate_target() function is adapted from David P. Anderson's tutorial at
 *  http://www.geology.smu.edu/dpa-www/robots/doc/odometry.txt
 */

#ifndef TARGET_H_
#define TARGET_H_

#include "Layer.h"

class RoombaSensors;

class Target: public Layer {
public:
	Target(RoombaSensors *rs);
	void eval();
	void setTargets(int *targetListIn);
	void printData();

    void setSeekSpeed(int seekSpeed)
    {
        this->seekSpeed = seekSpeed;
    }

private:
	RoombaSensors *roombaSensors;
	int *targets;				// array of targets, element 0 is number of targets
	int targetCnt;
	int targetIndex;
	int defaultTarget[3];		// array of [#_of_waypoints, wp1_x, wp1_y}

	float X_target;                 /* X lateral target position */
	float Y_target;                 /* Y vertical target position */
	float target_bearing;           /* bearing in radians from current position */
	float target_distance;         	/* current distance in inches from position (computed by locate_target() ) */
	float heading_error;            /* current heading error in degrees */
	int seekSpeed;

	void locate_target();
};

#endif /* TARGET_H_ */
