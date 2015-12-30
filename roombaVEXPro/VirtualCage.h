/*
 * VirtualCage.h
 *
 *  Created on: Jan 29, 2012
 *      Author: bouchier
 */

#ifndef VIRTUALCAGE_H_
#define VIRTUALCAGE_H_

#include "Layer.h"

class RoombaSensors;

class VirtualCage: public Layer {
public:
	VirtualCage(RoombaSensors *rs, int speed, int cageXIn, int cageYIn);
	void eval();
private:
	RoombaSensors *roombaSensors;
	int state;			// state variable for FSM used to manage ballistic behavior to escape from bump
	int stateTimer;		// for timing how many ticks to stay in each state
	int cageXMin, cageXMax, cageYMin, cageYMax;	// cage extends to either side of robot initial position & straight ahead of it

};

#endif /* VIRTUALCAGE_H_ */
