/*
 * main.cpp
 *
 *  Created on: Nov 3, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include <sys/time.h>
#include "Metro.h"

int main()
{
	Metro metro = Metro(50);
	struct timeval tNow;

	for (int i=0; i<20; i++) {
		while (!metro.check())
			;
		gettimeofday(&tNow, NULL);
		printf("Metro expired at %d.%d\n", (int)tNow.tv_sec, (int)tNow.tv_usec);
	}
}
