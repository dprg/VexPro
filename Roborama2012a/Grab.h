/*
 * Grab.h
 *
 *  Created on: May 8, 2012
 *      Author: bouchier
 */

#ifndef GRAB_H_
#define GRAB_H_

#include <qeservo.h>
#include "Layer.h"

class RoombaSensors;

class Grab : public Layer {
public:
	Grab(RoombaSensors *rsIn);
	void eval();
	void openGrabber();

private:
	RoombaSensors *rs;
	int cliff;
	CQEServo *servo;
	int axis;	// motor port 2 runs the grabber
};

#endif /* GRAB_H_ */
