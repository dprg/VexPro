/*
 * main.cpp
 *
 *  Created on: Dec 18, 2011
 *      Author: bouchier
 */
#include <stdio.h>
#include "qetime.h"

static CQETime::tick_t * timer4;

// Constants
#define T4				( *timer4 )
#define T4HZ			( 983040UL )

// Time-to-ticks conversions
#define T4USEC(usec)	( (unsigned long)((usec<60) ? (usec) : (usec - (usec>>6) - (usec>>10)) ) )
#define T4MSEC(msec)	( (unsigned long)(msec * (T4HZ/1000UL)) )
#define T4SEC(sec)		( (unsigned long)(sec * T4HZ) )

// Ticks-to-time conversions
#define T4TOUSEC(ticks)	( (unsigned long)(ticks + (ticks>>6) + (ticks>>10)) )
#define T4TOMSEC(ticks)	( (unsigned long)((ticks) / (T4HZ/1000)) )
#define T4TOSEC(ticks)	( (unsigned long)((ticks) / T4HZ) )

// Basic tick-evaluation functions
#define	T4ELAPSED(start)		( T4 - start )
#define T4EXPIRED(ticks,start)	( T4ELAPSED(start) > ticks )
#define T4SLEEP(ticks,start)	while (!T4EXPIRED(ticks,start)) { }


static void myUsleep(unsigned long usec)
{
	register CQETime::tick_t start = T4;
	register CQETime::tick_t ticks = T4USEC(usec);
	// printf("start %ld ticks %ld\n", start, ticks);
	T4SLEEP(ticks, start);
}

int main()
{
	register CQETime::tick_t start, end;
	register int delay;
	register int i;
	int measurementCount = 10;
	float perfectCount;

	for (delay=1; delay<1200000; delay*=2) {
		// measure # of ticks to sleep delay us using CQETime::usleep functions
		perfectCount = (((float)delay)/1000000.0) * 983040.0;
		printf("Delay: %d should be %8.0f ticks, CQETime::usleep() took ", delay, perfectCount);
		for (i=0; i<measurementCount; i++) {
			start = CQETime::ticks();
			CQETime::usleep(delay);
			end = CQETime::ticks();
			printf("%ld ", end-start);
		}

		// get pointer to timer4 register so we can access it
		timer4 = CQETime::getTimer4Ptr();
		// printf("Got pointer to timer4: 0x%x\n", timer4);

		// measure # of ticks to sleep 66us using modified macros
		printf("ticks; modified usleep() took ");
		for (i=0; i<measurementCount; i++) {
			start = T4;
			myUsleep(delay);
			end = T4;
			printf("%ld ", end-start);
		}
		printf("ticks\n");
	}


}
