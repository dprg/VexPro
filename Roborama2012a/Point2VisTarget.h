/*
 * Point2VisTarget.h
 *
 *  Created on: May 9, 2012
 *      Author: bouchier
 */

#ifndef POINT2VISTARGET_H_
#define POINT2VISTARGET_H_

#include "Layer.h"

#define MAX_SEEKS 24

class RoombaSensors;
class Spin;

class Point2VisTarget: public Layer {
public:
	Point2VisTarget(RoombaSensors *rs, Spin *s);
	void eval();
	void activate();
	void shmInit();
	void spin();
	int normalizeCompassHeading(int heading);
private:
	int degreesToSpin;
	RoombaSensors *rs;
	Spin *s;
	int state;
	int stateTimer;
	int targetHeading;

	// variables for seek state machine
	bool seeking;
	int currentSeekHeading, startHeading;
	int seekHeadingIncrement;
	int seekIteration;
	int seekLimit;	// how many seeks until we give up
	int confidence[MAX_SEEKS];
	int maxSeekConfidence;
	int mostConfidentIteration;
	bool gotDataFlag;
	bool seekLeft;

};
#endif /* POINT2VISTARGET_H_ */
