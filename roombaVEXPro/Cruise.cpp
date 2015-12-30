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
	layerName = "Cruise";
}

void Cruise::eval()
{
	// figure out if we've arrived at a final destination, & if so, clear flag to allow StopBot to activate

}
