
#ifndef _QETIME_H
#define _QETIME_H

/*! \file qetime.h
 * \brief Header file for the CQETime class
 */

/*! \class CQETime
 * \brief Microsecond & millisecond functions for getting time, busy-wait, delay-until.
 *
 * The CQETime class provides microsecond-resolution busy wait functions
 * that can be used to provide reasonably precise timing for single
 * and repeating events, as well as timeouts.
 *
 * Parameter definitions for all functions:
 *	'usec' specifies the time interval in microseconds, up to 1 min
 *	'msec' specifies the time interval in milliseconds, up to 1 hour
 *	'sec' specifies the time interval in seconds, up to 1 hour
 *	'last' specifies the tick_t value at the last scheduled event
 *	'start' specifies the tick_t value at the start of the interval
 *
 * Tick values represent the state of the internal timer used by this
 * library to measure time.  They are only exposed in this API for
 * efficiency.  Tick values should be considered as opaque and unitless.
 * However, they are universal; the tick_t value returned by any function
 * in this library can be used as a 'start' or 'last' parameter to any
 * other function in this library.
 */
class CQETime
{
public:
	typedef unsigned long tick_t;

	static tick_t ticks(void);
	static void usleep(unsigned long usec);
	static void msleep(unsigned long msec);
	static void sleep(unsigned long sec);

	static void usleep(unsigned long usec, tick_t start);
	static void msleep(unsigned long msec, tick_t start);
	static void sleep(unsigned long sec, tick_t start);

	static bool utimeout(unsigned long usec, tick_t start);
	static bool mtimeout(unsigned long msec, tick_t start);
	static bool timeout(unsigned long sec, tick_t start);

	static unsigned long uelapsed(tick_t start);
	static unsigned long melapsed(tick_t start);
	static unsigned long elapsed(tick_t start);

	static tick_t umetro(unsigned long usec, tick_t last);
	static tick_t mmetro(unsigned long msec, tick_t last);
	static tick_t metro(unsigned long sec, tick_t last);

	/*! This function returns a pointer to the timer4 register
	 *
	 */
	static tick_t * getTimer4Ptr();


	/*
	 * The following functions provide utility timekeeping
	 */
	static void delay(unsigned long msec);	// delay msec milliseconds in a spinloop
	static unsigned long millis();	// get the number of milliseconds since the program started.
	static void initTime();					// initialize millis

private:
	static int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1);
};

#endif // _QETIME_H
