/*
 * Cruise.h
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 */

#ifndef CRUISE_H_
#define CRUISE_H_

#include "Layer.h"

class Cruise : public Layer {
public:
	Cruise(int speed);
	void eval();
};

#endif /* CRUISE_H_ */
