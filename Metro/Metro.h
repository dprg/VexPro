/*! \file Metro.h
 * \brief Header file for the Metro class
 *
 * The Metro library facilitates the implementation of recurring timed events like:
 * blinking LEDs, servo motor control, Serial communication
 */

#ifndef Metro_h
#define Metro_h

#include <inttypes.h>

/*! \class Metro
 * \brief This class provides support for running operations at a fixed rate
 *
 * The Metro library facilitates the implementation of recurring timed events like:
 * blinking LEDs, servo motor control, Serial communication. The Metro object is instantiated
 * with the intended frame period. Subsequent calls to check() return false if time does
 * not indicate the next frame should start, or true if it should.
 */

class Metro
{

public:
  Metro(unsigned long interval_millis);
  Metro(unsigned long interval_millis, uint8_t autoreset);
  void interval(unsigned long interval_millis);
  char check();
  void reset();
  unsigned long millis();
  timeval milliStartTime;
	
private:
  uint8_t autoreset;
  unsigned long  previous_millis, interval_millis;
  int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1);
};

#endif


