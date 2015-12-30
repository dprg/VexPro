/*
 * main.cpp
 *
 *  Created on: Nov 3, 2012
 *      Author: bouchier
 */

#include "CQEI2C.h"

CQEI2C i2c = CQEI2C();					// instantiate the I2C driver

int main()
{
	i2c.I2CInit();
	i2c.I2CBusScan(1, 127, false);		// print what's found on the I2C bus
}
