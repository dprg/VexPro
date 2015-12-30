/**
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
 */

#include "qetime.h"
#include "9302hw.h"

// Memory map for EP9302 Timer Registers
static CMemMap m_timers(0x80810000,0x100);

// Constants
#define T4				( *m_timers.Uint(0x60) )
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
#define	T4ELAPSED(start)		((unsigned long)( T4 - start ))
#define T4EXPIRED(ticks,start)	( T4ELAPSED(start) > ticks )
#define T4SLEEP(ticks,start)	while (!T4EXPIRED(ticks,start)) { }


CQETime::tick_t CQETime::ticks(void)
{
	return(T4);
}


// Microsecond accuracy functions
// Limited to ~70 minutes due to 32b resolution
void CQETime::usleep(unsigned long usec)
{
	register tick_t start = T4;
	register tick_t ticks = T4USEC(usec);
	T4SLEEP(ticks, start);
}

void CQETime::usleep(unsigned long usec, tick_t start)
{
	register tick_t ticks = T4USEC(usec);
	T4SLEEP(ticks, start);
}

bool CQETime::utimeout(unsigned long usec, tick_t start)
{
	register tick_t ticks = T4USEC(usec);
	return T4EXPIRED(ticks, start);
}

unsigned long CQETime::uelapsed(tick_t start)
{
	return T4TOUSEC(T4-start);
}

CQETime::tick_t CQETime::umetro(unsigned long usec, tick_t last)
{
	register tick_t ticks = T4USEC(usec);
	T4SLEEP(ticks, T4);
	return (ticks+last);
}


// Millisecond accuracy functions
// Limited to 1 hour due to 32b resolution
void CQETime::msleep(unsigned long msec)
{
	register tick_t start = T4;
	register tick_t ticks = T4MSEC(msec);
	T4SLEEP(ticks, start);
}

void CQETime::msleep(unsigned long msec, tick_t start)
{
	register tick_t ticks = T4MSEC(msec);
	T4SLEEP(ticks, start);
}

bool CQETime::mtimeout(unsigned long msec, tick_t start)
{
	register tick_t ticks = T4MSEC(msec);
	return T4EXPIRED(ticks, start);
}

unsigned long CQETime::melapsed(tick_t start)
{
	return T4TOMSEC(T4-start);
}

CQETime::tick_t CQETime::mmetro(unsigned long msec, tick_t last)
{
	register tick_t ticks = T4MSEC(msec);
	T4SLEEP(ticks, last);
	return (ticks+last);
}


// Second accuracy functions
// Limited to 1 hour due to 32b resolution
void CQETime::sleep(unsigned long sec)
{
	register tick_t start = T4;
	register tick_t ticks = T4SEC(sec);
	T4SLEEP(ticks, start);
}

void CQETime::sleep(unsigned long sec, tick_t start)
{
	register tick_t ticks = T4SEC(sec);
	T4SLEEP(ticks, start);
}

bool CQETime::timeout(unsigned long sec, tick_t start)
{
	register tick_t ticks = T4SEC(sec);
	return T4EXPIRED(ticks, start);
}

unsigned long CQETime::elapsed(tick_t start)
{
	return T4TOSEC(T4-start);
}

CQETime::tick_t CQETime::metro(unsigned long sec, tick_t last)
{
	register tick_t ticks = T4SEC(sec);
	T4SLEEP(ticks, last);
	return (ticks+last);
}


