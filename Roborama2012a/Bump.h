/*
 * Bump.h
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 */

#ifndef BUMP_H_
#define BUMP_H_

#include "Layer.h"

class RoombaSensors;

class Bump : public Layer {
public:
	Bump(RoombaSensors *rs, int speed);
	void eval();
    void setBounceOffBumps(bool bounceOffBumps)
    {
        this->bounceOffBumps = bounceOffBumps;
    }

private:
	RoombaSensors *roombaSensors;
	int state;			// state variable for FSM used to manage ballistic behavior to escape from bump
	int stateTimer;		// for timing how many ticks to stay in each state
	bool lBump, rBump;
	int cliff;		// cliff sensors: bit 0: left; 1: front left; 2: front right; 3: right
	bool bounceOffBumps;
};

#endif /* BUMP_H_ */
