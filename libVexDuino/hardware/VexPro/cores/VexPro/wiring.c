/**
 * @file wiring.c Partial implementation of the Wiring API for the VEXpro.

 * This file implements the time functions millis and delay(). It is a Linux
 * implementation of the functions in arduino wiring.c. See vexpro_time.cpp for
 * the microsecond-resolution time functions.
 * FIXME millis() is not correlated with micros(), unlike Arduino, where I think it is. Is this an issue?
 */
/*
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2011 Paul H Bouchier

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
//#include "memmap.h"
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

static struct timeval arduinoTimer0Start;	// startTime per getTimeOfDay()

/* Return 1 if the difference is negative, otherwise 0.  */
int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

/**
 * Returns the number of milliseconds since the Arduino library began running the current program.
 *
 * Note that the value returned from millis is an unsigned long,
 * errors may be generated if a programmer tries to do math with other datatypes such as ints.
 *
 * @return Number of milliseconds since the program started (unsigned long)
 */
unsigned long millis()
{
	struct timeval now, sinceStart;
	long int msSinceStart;

    gettimeofday(&now, NULL);
    timeval_subtract(&sinceStart, &now, &arduinoTimer0Start);
    msSinceStart = ((sinceStart.tv_sec) * 1000) + (sinceStart.tv_usec/1000);
	return msSinceStart;
}



/**
 * Pauses the program for the amount of time (in miliseconds) specified as parameter.
 * (There are 1000 milliseconds in a second.)
 * Note that as of the first release, timing is only accurate to about 10ms (high-resolution
 * timers are not enabled). Use delayMicroseconds() if you need sub-10ms resolution on delay()
 * @param msec the number of milliseconds to pause (unsigned long)
 * @return None
 */
void delay(unsigned long msec)
{
	struct timespec timeout0;
	struct timespec timeout1;
	struct timespec* tmp;
	struct timespec* t0 = &timeout0;
	struct timespec* t1 = &timeout1;

	t0->tv_sec = msec / 1000;
	t0->tv_nsec = (msec % 1000) * 1000000;

	while ((nanosleep(t0, t1) == (-1)) && (errno == EINTR))
	{
		tmp = t0;
		t0 = t1;
		t1 = tmp;
	}
}

void init()
{
	// this needs to be called before setup() or some functions won't work there
	// record when we started arduino emulation
    gettimeofday(&arduinoTimer0Start, NULL);
}
