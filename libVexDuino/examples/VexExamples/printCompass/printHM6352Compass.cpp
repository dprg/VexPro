/**
 * @file printHM6352Compass.cpp Program to print the HM6352 compass value
 */
/*
   This program is free software; you can redistribute it and/or
  modify it without limitation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <iostream>
#include <unistd.h>
#include "libI2C.h"
#include "readHM6352Compass.h"

int main()
{
	short heading;
	int mode = MODE_CONTINUOUS;	// set to true to read in continuous mode, otherwise uses A command to read

	I2CInit();
	sleep(1);
	SleepHM6352();
	sleep(1);
	WakeHM6352();
	I2CBusScan(0x21, 0x21);
	if (!SetHM6352Mode(mode)) {
		fprintf(stderr, "Error in initializing compass, Exiting\n");
		exit(-1);
	}

	while (1) {
		heading = ReadHM6352();
		printf("heading: %d.%d\n", heading/10, heading%10);
		usleep(250000);	// pause 1/4 sec between prints
	}
	return 0;
}
