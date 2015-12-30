/*
 * qetime.cpp
 *
 * The CQETime class provides (nearly) microsecond-resolution busy
 * wait functions that can be used to provide reasonably precise
 * timing for single and repeating events, as well as timeouts.
 *
 * CPU Timer 4 is used for all timing functions.  The actual clock
 * frequency is 983.04KHz, making the time resolution approximately
 * 1.017us.  For efficiency, we only use the lower 32b of Timer 4,
 * resulting in an approximate overflow time of 72 min.  All intervals
 * should be kept under an hour, and microsecond intervals should be
 * kept below 1 minute since 6 bits of range are lost in the
 * conversion between ticks and microseconds.
 *
 * The CQETime class also provides millisecond utility functions delay() and millis(),
 * derived from the Arduino code of the same name.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

#include "qetime.h"
#include "9302hw.h"

// Memory map for EP9302 Timer Registers
static CMemMap m_timers(0x80810000,0x100);

// Constants
#define T4				( *m_timers.Uint(0x60) )
#define T4HZ			( 983040UL )

// Time-to-ticks conversions
#define T4USEC(usec)	( (unsigned long)((usec<60) ? (usec) : ((usec<<6)/65)) )
#define T4MSEC(msec)	( (unsigned long)(msec * (T4HZ/1000UL)) )
#define T4SEC(sec)		( (unsigned long)(sec * T4HZ) )

// Ticks-to-time conversions
#define T4TOUSEC(ticks)	( (unsigned long)((ticks*65)>>6) )
#define T4TOMSEC(ticks)	( (unsigned long)((ticks) / (T4HZ/1000)) )
#define T4TOSEC(ticks)	( (unsigned long)((ticks) / T4HZ) )

// Basic tick-evaluation functions
#define	T4ELAPSED(start)		( T4 - start )
#define T4EXPIRED(ticks,start)	( T4ELAPSED(start) > ticks )
#define T4SLEEP(ticks,start)	while (!T4EXPIRED(ticks,start)) { }

/*! Get current timer value
 *
 * ticks() returns the current timer tick_t value for use as the
 * 'start' or 'last' parameter in a subsequent call to one of
 * the other CQETimer functions
 * \return Current timer tick_t value
 */
CQETime::tick_t CQETime::ticks(void)
{
	return(T4);
}


// Microsecond accuracy functions

/*! Do a simple busy-wait for the specified number of microseconds.
 *
 * Example of a 100 microsecond delay:	CQETime::usleep(100);
 *
 * Limited to 1 minute due to 32b resolution
 * This may take longer than the specified time due to other
 * system activity, but it will never take less than the
 * requested time.
 *
 * \param usec Number of microseconds to sleep
 */
void CQETime::usleep(unsigned long usec)
{
	register tick_t start = T4;
	register tick_t ticks = T4USEC(usec);
	T4SLEEP(ticks, start);
}


/*! Busy-wait for delay microseconds from a start-time
 *
 * This function busy-waits for the specified
 * number of microseconds.
 * The 'start' parameter specifies the tick_t value to use
 * as the starting time.  This can be used to allow an
 * accurate delay to be done in parallel with other work.
 *
 * Example of a 100 microsecond delay:
 * 	start = QETimer::ticks();
 *  "Do some work in parallel with wait"
 *  QETimer::usleep(100, start);
 *
 * If the specified interval has already passed, it will
 * return immediately.  It may take longer than the
 * specified time due to other system activity, but it
 * will never take less than the requested time.
 */
void CQETime::usleep(unsigned long usec, tick_t start)
{
	register tick_t ticks = T4USEC(usec);
	T4SLEEP(ticks, start);
}

/*! Microseconds timout check
 *
 * Rather than waiting, simply check
 * the elapsed time and return true if it has elapsed
 * and false if not.
 *
 * Example of a 1 second timeout:
 * 	start = QETimer::ticks();
 * 	while (condition) {
 * 		if (timeout(1,start)) break;
 * 	}
 *  "Do some work in parallel with wait"
 *  QETimer::usleep(100, start);
 */
bool CQETime::utimeout(unsigned long usec, tick_t start)
{
	register tick_t ticks = T4USEC(usec);
	return T4EXPIRED(ticks, start);
}

/*! Get elapsed time in usec
 *
 * Get the time that has elapsed
 * since the tick_t value specified by 'start'.  These
 * can be used to time external events.
 *
 * Example of millisecond-resolution event timer:
 * 	start = QETimer::ticks();
 * 	while (condition) { ... }
 * 	msec = QETimer::melapsed(start);
 *
 * \return Elapsed time in usec
 */
unsigned long CQETime::uelapsed(tick_t start)
{
	return T4TOUSEC(T4-start);
}

/* Busy-wait for specified microseconds from start time
 *
 *
 * Example of a 2Hz (1/4s on, 1/4s off) LED flasher:
 * 	last = QETimer::ticks();
 *  while (keep flashing) {
 *  	toggle LED;
 *  	QETimer::mmetro(250, last);
 *  }
 *
 * If the specified interval has already passed, they will
 * return immediately.  They may take longer than the
 * specified time due to other system activity, but they
 * will never take less than the requested time.
 *
 * \param usec The interval
 * \param last The 'last' parameter specifies the tick_t value to use
 * as the starting time.
 * \return These functions return the
 * scheduled wake time for use as the 'last' parameter
 * to subsequent calls.  This can be used to implement an
 * accurate repeating event
 */
CQETime::tick_t CQETime::umetro(unsigned long usec, tick_t last)
{
	register tick_t ticks = T4USEC(usec);
	T4SLEEP(ticks, T4);
	return (ticks+last);
}


// Millisecond accuracy functions
//

/*! Do a simple busy-wait for the specified number of milliseconds.
 *
 * Example of a 100 millisecond delay:	CQETime::msleep(100);
 *
 * Limited to 1 hour due to 32b resolution.
 * This may take longer than the specified time due to other
 * system activity, but it will never take less than the
 * requested time.
 * \param msec Number of milliseconds to sleep
 */
void CQETime::msleep(unsigned long msec)
{
	register tick_t start = T4;
	register tick_t ticks = T4MSEC(msec);
	T4SLEEP(ticks, start);
}


/*! Busy-wait for delay milliseconds from a start-time
 *
 * This function busy-waits for the specified
 * number of milliseconds.
 * The 'start' parameter specifies the tick_t value to use
 * as the starting time.  This can be used to allow an
 * accurate delay to be done in parallel with other work.
 *
 * Example of a 100 millisecond delay:
 * 	start = QETimer::ticks();
 *  "do some work in parallel with wait"
 *  QETimer::msleep(100, start);
 *
 * If the specified interval has already passed, it will
 * return immediately.  It may take longer than the
 * specified time due to other system activity, but it
 * will never take less than the requested time.
 */
void CQETime::msleep(unsigned long msec, tick_t start)
{
	register tick_t ticks = T4MSEC(msec);
	T4SLEEP(ticks, start);
}

/*! Milliseconds timout check
 *
 * Rather than waiting, simply check
 * the elapsed time and return true if it has elapsed
 * and false if not.
 *
 * Example of a 1 second timeout:
 * 	start = QETimer::ticks();
 * 	while (condition) {
 * 		if (timeout(1,start)) break;
 * 	}
 *  do some work in parallel with wait
 *  QETimer::usleep(100, start);
 */
bool CQETime::mtimeout(unsigned long msec, tick_t start)
{
	register tick_t ticks = T4MSEC(msec);
	return T4EXPIRED(ticks, start);
}

/*! Get elapsed time in msec
 *
 * Get the time that has elapsed
 * since the tick_t value specified by 'start'.  These
 * can be used to time external events.
 *
 * Example of millisecond-resolution event timer:
 * 	start = QETimer::ticks();
 * 	while (condition) { ... }
 * 	msec = QETimer::melapsed(start);
 *
 * \return Elapsed time in msec
 */
unsigned long CQETime::melapsed(tick_t start)
{
	return T4TOMSEC(T4-start);
}

/* Busy-wait for specified milliseconds from start time
 *
 *
 * Example of a 2Hz (1/4s on, 1/4s off) LED flasher:
 * 	last = QETimer::ticks();
 *  while (<keep flashing>) {
 *  	toggle LED;
 *  	QETimer::mmetro(250, last);
 *  }
 *
 * If the specified interval has already passed, they will
 * return immediately.  They may take longer than the
 * specified time due to other system activity, but they
 * will never take less than the requested time.
 *
 * \param usec The interval
 * \param last The 'last' parameter specifies the tick_t value to use
 * as the starting time.
 * \return These functions return the
 * scheduled wake time for use as the 'last' parameter
 * to subsequent calls.  This can be used to implement an
 * accurate repeating event
 */
CQETime::tick_t CQETime::mmetro(unsigned long msec, tick_t last)
{
	register tick_t ticks = T4MSEC(msec);
	T4SLEEP(ticks, last);
	return (ticks+last);
}


// Second accuracy functions

/*! Do a simple busy-wait for the specified number of seconds.
 *
 * Example of a 100 second delay:	CQETime::sleep(100);
 *
 * Limited to 1 hour due to 32b resolution.
 * This may take longer than the specified time due to other
 * system activity, but it will never take less than the
 * requested time.
 */
void CQETime::sleep(unsigned long sec)
{
	register tick_t start = T4;
	register tick_t ticks = T4SEC(sec);
	T4SLEEP(ticks, start);
}


/*! Busy-wait for delay seconds from a start-time
 *
 * This function busy-waits for the specified
 * number of seconds.
 * The 'start' parameter specifies the tick_t value to use
 * as the starting time.  This can be used to allow an
 * accurate delay to be done in parallel with other work.
 *
 * Example of a 100 second delay:
 * 	start = QETimer::ticks();
 *  do some work in parallel with wait
 *  QETimer::sleep(100, start);
 *
 * If the specified interval has already passed, it will
 * return immediately.  It may take longer than the
 * specified time due to other system activity, but it
 * will never take less than the requested time.
 */
void CQETime::sleep(unsigned long sec, tick_t start)
{
	register tick_t ticks = T4SEC(sec);
	T4SLEEP(ticks, start);
}

/*! Seconds timout check
 *
 * Rather than waiting, simply check
 * the elapsed time and return true if it has elapsed
 * and false if not.
 *
 * Example of a 1 second timeout:
 * 	start = QETimer::ticks();
 * 	while (condition) {
 * 		if (timeout(1,start)) break;
 * 	}
 *  do some work in parallel with wait
 *  QETimer::usleep(100, start);
 */
bool CQETime::timeout(unsigned long sec, tick_t start)
{
	register tick_t ticks = T4SEC(sec);
	return T4EXPIRED(ticks, start);
}

/*! Get elapsed time in sec
 *
 * Get the time that has elapsed
 * since the tick_t value specified by 'start'.  These
 * can be used to time external events.
 *
 * Example of millisecond-resolution event timer:
 * 	start = QETimer::ticks();
 * 	while (condition) { ... }
 * 	msec = QETimer::melapsed(start);
 *
 * \return Elapsed time in sec
 */
unsigned long CQETime::elapsed(tick_t start)
{
	return T4TOSEC(T4-start);
}

/* Busy-wait for specified seconds from start time
 *
 *
 * Example of a 2Hz (1/4s on, 1/4s off) LED flasher:
 * 	last = QETimer::ticks();
 *  while (<keep flashing>) {
 *  	toggle LED;
 *  	QETimer::mmetro(250, last);
 *  }
 *
 * If the specified interval has already passed, they will
 * return immediately.  They may take longer than the
 * specified time due to other system activity, but they
 * will never take less than the requested time.
 *
 * \param usec The interval
 * \param last The 'last' parameter specifies the tick_t value to use
 * as the starting time.
 * \return These functions return the
 * scheduled wake time for use as the 'last' parameter
 * to subsequent calls.  This can be used to implement an
 * accurate repeating event
 */
CQETime::tick_t CQETime::metro(unsigned long sec, tick_t last)
{
	register tick_t ticks = T4SEC(sec);
	T4SLEEP(ticks, last);
	return (ticks+last);
}

CQETime::tick_t * CQETime::getTimer4Ptr(void)
{
	return (unsigned long int *)m_timers.Uint(0x60);
}

/* Return 1 if the difference is negative, otherwise 0.  */
int CQETime::timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

static struct timeval milliStartTime;

/*! Get the number of milliseconds since initTime() was called.
 *
 * initTime() must be called before using this function.
 * Note that the value returned from millis is an unsigned long,
 * errors may be generated if a programmer tries to do math with other datatypes such as ints.
 *
 * \return Number of milliseconds since the program started (unsigned long)
 */
unsigned long CQETime::millis()
{
	struct timeval now, sinceStart;
	long int msSinceStart;

    gettimeofday(&now, NULL);
    timeval_subtract(&sinceStart, &now, &milliStartTime);
    msSinceStart = ((sinceStart.tv_sec) * 1000) + (sinceStart.tv_usec/1000);
	return msSinceStart;
}

/*! Pause the program for the amount of time (in miliseconds) specified as parameter.
 *
 * Note that as of the first release, timing is only accurate to about 10ms (high-resolution
 * timers are not enabled). Use delayMicroseconds() if you need sub-10ms resolution on delay()
 * @param msec the number of milliseconds to pause (unsigned long)
 */
void CQETime::delay(unsigned long msec)
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

/*! Initializes the timer used by millis()
 *
 * This needs to be called before the first call to millis()
 */
void CQETime::initTime()
{
    gettimeofday(&milliStartTime, NULL);
}
