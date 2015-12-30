/**
 * @file vexpro_time.cpp Arduino time functions for the VEXpro.

 * This file implements the time functions micros() and delayMicroseconds(). It wraps
 * the CQETime class, which uses the low 32-bits of timer4 in the EP9302 processor.
 *
 * FIXME millis() is not correlated with micros(), unlike Arduino
 */
/*
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

#include "qetime.h"

/**
 * Returns the number of microseconds since the FPGA started. This will wrap approximately
 * every 4000 seconds (~70 minutes) due to 32 bit rollover
 *
 * @return Microseconds since the VEXpro started
 */
unsigned long micros()
{
  return CQETime::uelapsed(0);
}

/**
 *  delayMicroseconds is a libVexDuino library function that wraps the usleep()
 *  method in qetime. usleep() uses Timer4 in the
 *  EP9302 CPU to implement a microsecond-resolution busy-wait loop.  Timer4
 *  is actually a 40-bit timer, but we only look at the bottom 32-bits, which
 *  is more than enough for an hour.
 */
inline void delayMicroseconds(unsigned long usec)
{
	CQETime::usleep(usec);
}



