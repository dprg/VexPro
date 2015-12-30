/*
 * Spin.h
 *
 *  Created on: Jan 29, 2012
 *      Author: bouchier
 */

#ifndef SPIN_H_
#define SPIN_H_

#include "Layer.h"

class RoombaSensors;

class Spin: public Layer {
public:
	Spin(RoombaSensors *rs);
	void eval();
	void setSpin(int degIn);
private:
	float degrees;
	RoombaSensors *roombaSensors;
};

#endif /* SPIN_H_ */
