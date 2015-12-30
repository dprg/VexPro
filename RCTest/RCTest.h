/*
 * RCTest.h
 *
 *  Created on: Nov 9, 2012
 *      Author: bouchier
 */

#ifndef RCTEST_H_
#define RCTEST_H_

struct RCChannel
{
	struct timeval tv0;
	int flag;
	int dioIndex;
	int pulseWidth;
};

int initRCTest(CQEGpioInt& gpio, struct RCChannel *rcc_p);

#endif /* RCTEST_H_ */
