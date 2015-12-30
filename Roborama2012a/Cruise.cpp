/*
 * Cruise.cpp
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 */

#include "Cruise.h"

Cruise::Cruise(int speed) : Layer(speed) {
	velRqst = 100;		// initialize the Cruise layer. Other variables initialized in Layer
	flag = 1;
	layerName = (char *)"Cruise";
}

void Cruise::eval()
{
	// run at cruise speed or not based on cruiseEnable
	if (cruiseEnable)
		flag = 1;
	else
		flag = 0;
}
