
#include <stdio.h>
#include <sys/time.h>
#include "Metro.h"

/*! Instantiate a Metro object with a set interval in milliseconds with no autoreset
 *
 * \param interval_millis Duration of the frame in milliseconds
 */
Metro::Metro(unsigned long interval_millis)
{
        interval(interval_millis);
        this->autoreset = false;
        gettimeofday(&milliStartTime, NULL);
        reset();
}

/*! Instantiate a Metro object with a set interval in milliseconds.
 *
 * \param interval_millis The interval before check returns true
 * \param autoreset If the autoreset is set to true (1), the internal timer will reset,
 * ignoring missed events. If you want to catch up with missed events (because you don't
 * call the check method regularly), set autoreset to false.
 */
Metro::Metro(unsigned long interval_millis, uint8_t autoreset)
{   
        this->autoreset = autoreset;
        interval(interval_millis);
        gettimeofday(&milliStartTime, NULL);
        reset();
}

/*
 * Return 1 if the difference is negative, otherwise 0.
 */
int Metro::timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

/*! Get the number of milliseconds since this object was instantiated
 * @return Milliseconds since instantiation
 */
unsigned long Metro::millis()
{
	struct timeval now, sinceStart;
	long int msSinceStart;

    gettimeofday(&now, NULL);
    timeval_subtract(&sinceStart, &now, &milliStartTime);
    msSinceStart = ((sinceStart.tv_sec) * 1000) + (sinceStart.tv_usec/1000);
	return msSinceStart;
}

/*! Change the frame time
 * @param interval_millis The new frame time in milliseconds
 */
void Metro::interval(unsigned long interval_millis)
{
  this->interval_millis = interval_millis;
}

/*! Check whether the frame has elapsed yet
 * @return True if the frame has lapsed. Returns false if not.
 */
char Metro::check()
{
  if (millis() - this->previous_millis >= this->interval_millis) {
    // As suggested by benjamin.soelberg@gmail.com, the following line 
    // this->previous_millis = millis();
    // was changed to
    // this->previous_millis += this->interval_millis;
    
    // But then a bug would sometimes occur when the interval was set with random, 
    // so I added the following check to reinstate the previous behavior, but I am 
    // not sure this fixes it
    
    if (this->interval_millis == 0 || this->autoreset ) {
        this->previous_millis = millis();
        printf("resetting");
    } else {
        this->previous_millis += this->interval_millis;
    }
    //printf("previous_millis: %d\n", this->previous_millis);
    return 1;
  }
  return 0;
}

/*! Restart/reset the Metro
 */
void Metro::reset() 
{
	this->previous_millis = millis();
}


