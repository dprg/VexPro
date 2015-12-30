/*
 * main.cpp
 *
 *  Created on: Oct 5, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libI2C.h"
#include "i2c_def.h"

void usage()
{
	printf("Usage: vexMotorEncoder testNum\n");
	printf("testNum can be:\n");
	printf("0: print this message after enumerating encoders, then print encoder table\n");
	printf("1: print device table\n");
	printf("2: check for inactive device\n");
	printf("3: measure speed & distance on all motors & print for 20 sec\n");
}
int main(int argc, char **argv)
{
	int test = 0;	// which test to run
	if (argc == 2)
		test = atoi(argv[1]);
	printf("Running test %d\n", test);

	I2CInit();
	Initialize_I2c_device_table();
	Int_Search_For_Devices();
	switch (test) {
	case 0: usage(); Print_Table(); break;
	case 1: Print_Table(); break;
	case 2:
		Print_Table();
		CheckForInactiveDevice(0);
		break;
	case 3:
		for (int i=0; i<20; i++) {
			PrintAllDevices(READ_DEV_TICS);
			sleep(1);
		}
		break;
	case 4:
		ReadOneDeviceFast(0);
		break;
	case 5:
		WriteAllDevices(WRITE_RESET_COUNTERS);
		break;
	default:
		printf("Invalid option\n");
	}
}
