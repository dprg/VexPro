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
#include "RCRx.h"

CQEGpioInt &gpio = CQEGpioInt::GetRef();

void usage()
{
	printf("Usage: ControlledMotor testNum\n");
	printf("testNum can be:\n");
	printf("0: print pulse-widths\n");
}

void test1()
{
	int pw;

	RCRx rcrx = RCRx(0, gpio);
	Metro metro = Metro(50);
	while (1){
		if (metro.check()) {
			pw = rcrx.getRCPulse();
			printf("got pulse width %d\n", pw);
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
