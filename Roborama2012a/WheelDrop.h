/*
 * WheelDrop.h
 *
 *  Created on: Jan 27, 2012
 *      Author: bouchier
 */

#ifndef WHEELDROP_H_
#define WHEELDROP_H_

#include "Layer.h"

class RoombaSensors;

class WheelDrop : public Layer {
public:
	WheelDrop(RoombaSensors* rsIn);
	void eval();
private:
	RoombaSensors* rs;
	int state;
	int stateTimer;
};

#endif /* WHEELDROP_H_ */
