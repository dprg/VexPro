/*
  timerTests.cpp
  Test various Arduino timing functions
 */

/*
 * Tools should remove the need for this section
 */
#include <stdio.h>
//#include <time.h>
#include "wiring.h"

void setup() {

	struct timespec res;
	int rv;

	rv = clock_getres(CLOCK_MONOTONIC, &res);
	if (!rv)
		printf("CLOCK_MONOTONIC has resolution %ld ns\n", res.tv_nsec);

}

void loop() {
	int start, end, microsDelay, milliDelay;
	start = micros();
	end = micros();
	microsDelay = end - start;
	printf("micros() took %d us\n", microsDelay);
	start = micros();
	delay(50);
	end = micros();
	milliDelay = end - start;
	printf("delay(50) took %d us\n", milliDelay + microsDelay);
	delay(1000);
}
