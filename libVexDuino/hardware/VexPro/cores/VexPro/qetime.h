/*
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

#ifndef _QETIME_H
#define _QETIME_H

class CQETime
{
public:
	typedef unsigned long tick_t;

	/*
	 * ticks() returns the current timer tick_t value for use as the
	 * 'start' or 'last' parameter in a subsequent call to one of
	 * the other CQETimer functions
	 */
	static tick_t ticks(void);

	/*
	 * These three functions simple busy-wait for the specified
	 * number of microseconds, milliseconds, or seconds.
	 *
	 * Example of a 100 microsecond delay:
	 * 	CQETime::usleep(100);
	 *
	 * They may take longer than the specified time due to other
	 * system activity, but they will never take less than the
	 * requested time.
	 */
	static void usleep(unsigned long usec);
	static void msleep(unsigned long msec);
	static void sleep(unsigned long sec);


	/*
	 * These three functions busy-wait for the specified
	 * number of microseconds, milliseconds, or seconds.
	 * The 'start' parameter specifies the tick_t value to use
	 * as the starting time.  This can be used to allow an
	 * accurate delay to be done in parallel with other work.
	 *
	 * Example of a 100 millisecond delay:
	 * 	start = QETimer::ticks();
	 *  <do some work in parallel with wait>
	 *  QETimer::msleep(100, start);
	 *
	 * If the specified interval has already passed, they will
	 * return immediately.  They may take longer than the
	 * specified time due to other system activity, but they
	 * will never take less than the requested time.
	 */
	static void usleep(unsigned long usec, tick_t start);
	static void msleep(unsigned long msec, tick_t start);
	static void sleep(unsigned long sec, tick_t start);

	/*
	 * These three functions implement a simple timeout
	 * solution.  Rather than waiting, they simply check
	 * the elapsed time and return true if it has elapsed
	 * and false if not.
	 *
	 * Example of a 1 second timeout:
	 * 	start = QETimer::ticks();
	 * 	while (<condition>) {
	 * 		if (timeout(1,start)) break;
	 * 	}
	 *  <do some work in parallel with wait>
	 *  QETimer::usleep(100, start);
	 */

	static bool utimeout(unsigned long usec, tick_t start);
	static bool mtimeout(unsigned long msec, tick_t start);
	static bool timeout(unsigned long sec, tick_t start);

	/*
	 * These three functions return the time that has elapsed
	 * since the tick_t value specified by 'start'.  These
	 * can be used to time external events.
	 *
	 * Example of millisecond-resolution event timer:
	 * 	start = QETimer::ticks();
	 * 	while (<condition>) { ... }
	 * 	msec = QETimer::melapsed(start);
	 */
	static unsigned long uelapsed(tick_t start);
	static unsigned long melapsed(tick_t start);
	static unsigned long elapsed(tick_t start);

	/*
	 * These three functions busy-wait for the specified
	 * number of microseconds, milliseconds, or seconds.
	 * The 'last' parameter specifies the tick_t value to use
	 * as the starting time.  These functions return the
	 * scheduled wake time for use as the 'last' parameter
	 * to subsequent calls.  This can be used to implement an
	 * accurate repeating event
	 *
	 * Example of a 2Hz (1/4s on, 1/4s off) LED flasher:
	 * 	last = QETimer::ticks();
	 *  while (<keep flashing>) {
	 *  	<toggle LED>;
	 *  	QETimer::mmetro(250, last);
	 *  }
	 *
	 * If the specified interval has already passed, they will
	 * return immediately.  They may take longer than the
	 * specified time due to other system activity, but they
	 * will never take less than the requested time.
	 */
	static tick_t umetro(unsigned long usec, tick_t last);
	static tick_t mmetro(unsigned long msec, tick_t last);
	static tick_t metro(unsigned long sec, tick_t last);
};

#endif // _QETIME_H
