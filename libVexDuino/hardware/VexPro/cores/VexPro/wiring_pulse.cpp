/** @file wiring_pulse.cpp  pulseIn() function
 */
/*
  Copyright (c) 2011 Paul H. Bouchier

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

#include "Arduino.h"
#include "qetime.h"

/**
 * Reads a pulse (either HIGH or LOW) on a pin. For example, if value is HIGH,
 * pulseIn() waits for the pin to go HIGH, starts timing, then waits for the pin to go LOW and stops timing.
 * Returns the length of the pulse in microseconds.
 * Gives up and returns 0 if no pulse starts within a specified time out or if an inaccurate measurement is detected.
 *
 * @param pin The digital I/O pin number on which you want to read the pulse
 * @param value The type of pulse to read: either HIGH or LOW
 * @param timeout The number of microseconds to wait for the pulse to start; default is one second. This parameter is optional
 * @return pulse width in microseconds, or 0 if error (e.g. invalid pin, inaccurate measurement due to context
 * switch. errorCode can be retrieved with getPulseInErrorCode(); Values are: -1: invalid pin; -2: timeout;
 * -3: inaccurate measurement detected
 */

#define EDGE_JITTER 20

static int errorCode;		// flag telling why we exited with 0

unsigned long pulseIn(int pin, int value, unsigned long timeout)
{
	CQETime::tick_t start;	// start time from which timeout is measured
	CQETime::tick_t currentTick;
	CQETime::tick_t lastTick;	// time we did the last measurement; used to detect latency of transition detection
	unsigned long pulseStartTime, pulseEndTime, pulseWidth;

	errorCode = 0;
	start = CQETime::ticks();
	
	if ((pin < 1) || (pin > 16)) {// check for valid pin
		errorCode = -1;
		return 0;
	}

	// wait for any previous pulse to end
	while (digitalRead(pin) == value) {
		if (CQETime::utimeout(timeout, start)) {	// exit if we timed out
			errorCode = -2;
			return 0;
		}
	}
	
	// wait for the pulse to start
	lastTick = CQETime::ticks();
	while (digitalRead(pin) != value) {
		lastTick = CQETime::ticks();	// record the last time the pin was not equal to the search value
		if (CQETime::utimeout(timeout, start)) {	// exit if we timed out
			errorCode = -2;
			return 0;
		}
	}

	pulseStartTime = CQETime::uelapsed(0);
	// check to see if we took a context switch or something else that would make the measurement inaccurate
	currentTick = CQETime::ticks();
	if (((long)(currentTick - lastTick)) > EDGE_JITTER) {
		errorCode = -3;
		return 0;
	}

	
	// wait for the pulse to stop
	while (digitalRead(pin) == value) {
		lastTick = CQETime::ticks();	// record the last time the pin was equal to the search value
		if (CQETime::utimeout(timeout, start)) {	// exit if we timed out
			errorCode = -2;
			return 0;
		}
	}

	pulseEndTime = CQETime::uelapsed(0);
	// check to see if we took a context switch or something else that would make the measurement inaccurate
	currentTick = CQETime::ticks();
	if (((long)(currentTick - lastTick)) > EDGE_JITTER) {
		errorCode = -3;
		return 0;
	}

	pulseWidth = (pulseEndTime - pulseStartTime);

	return pulseWidth;
}
