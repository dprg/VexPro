/*
 * Stop.h
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 */

#ifndef STOP_H_
#define STOP_H_

#include "Layer.h"

class RoombaSensors;

class StopBot : public Layer {
public:
	StopBot(RoombaSensors *rs);
	void eval();
private:
	RoombaSensors *roombaSensors;
};

#endif /* STOP_H_ */
