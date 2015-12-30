/*
 * main.cpp
 *
 *  Created on: Nov 3, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include "pid.h"

int main()
{
	PID pid = PID();	// default is kI = 1.0

	pid.setDebugFlag(true);

	pid.computePid(0.0, 1.0);
	pid.computePid(1.0, 1.0);
	pid.computePid(1.5, 1.0);

	printf("\n");
	pid.initPid(2.0, 1.0, 0.0);

	pid.computePid(0.0, 1.0);
	pid.computePid(0.5, 1.0);
	pid.computePid(0.75, 1.0);
	pid.computePid(1.0, 1.0);
	pid.computePid(1.2, 1.0);
	pid.computePid(1.2, 1.0);
	pid.computePid(1.0, 1.0);
	pid.computePid(1.0, 1.0);

}
