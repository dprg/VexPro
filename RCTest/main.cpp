/*
 * main.cpp
 *
 *  Created on: Nov 9, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include <stdlib.h>
#include "qegpioint.h"
#include "Metro.h"
#include "RCTest.h"

CQEGpioInt &gpio = CQEGpioInt::GetRef();
struct RCChannel dirRcc;
struct RCChannel speedRcc;

void usage()
{
	printf("Usage: ControlledMotor testNum\n");
	printf("testNum can be:\n");
	printf("0: print pulse-widths\n");
}

void test1()
{
	// initialize the steering struct & start interrupt monitoring
	dirRcc.flag = 1;
	dirRcc.dioIndex = 0;
	initRCTest(gpio, &dirRcc);

	// initialize the speed struct & start interrupt monitoring
	speedRcc.flag = 1;
	speedRcc.dioIndex = 1;
	initRCTest(gpio, &speedRcc);

	Metro metro = Metro(50);
	while (1){
		if (metro.check()) {
			printf("Speed: %d, Dir: %d\n", speedRcc.pulseWidth, dirRcc.pulseWidth);
		}
	}

}

int main(int argc, char **argv)
{
	int test = 1;	// which test to run
	int argval;		// value to pass into test

	if (argc == 2)
		test = atoi(argv[1]);
	if (argc == 3)
		argval = atoi(argv[2]);
	printf("Running test %d\n", test);

	switch (test) {
	case 1:	test1(); break;
	//case 2: test2(); break;
	//case 3: test3(); break;
	//case 4: test4(); break;
	default:
		printf("Invalid option\n");
		usage();
	}
}
