/**
  @file vexpro_digital.cpp
  @brief digital input and output functions
*/
/*
  Compatible with Arduino, built on top of libqwerk

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

#include "wiring.h"
#include <qegpioint.h>
CQEGpioInt &io = CQEGpioInt::GetRef();

// number of digital I/Os on VexPro controller
#define NUM_DIOS 16

/**
 * Configures the specified pin to behave either as an input or an output.
 *
 * @param pin the number of the digital I/O whose mode you wish to set.
 * Value ranges from 1 to 16 corresponding to digital I/Os D1 - D16.
 * @param mode either INPUT or OUTPUT
 * @return none
 */
void pinMode(uint8_t pin, uint8_t mode)
{
	uint32_t direction;
	uint8_t dirBit;		// I/O direction bit

	if ((pin < 1) | (pin > NUM_DIOS))
		return;

	dirBit = pin - 1;		// Digital I/Os are 1-based, but the get/set DataDirection methods are 0-based
	direction = io.GetDataDirection();
	if (mode == OUTPUT) {	// direction = out for mode = 1 and also for Arduino OUTPUT
		direction |= (1<<dirBit);
	} else {
		direction &= ~(1<<dirBit);
	}
	io.SetDataDirection(direction);
}

/**
 * Write a HIGH or a LOW value to a digital pin.
 *
 * @param pin the number of the digital I/O whose mode you wish to set.
 * Value ranges from 1 to 16 corresponding to digital I/Os D1 - D16.
 * @param value HIGH or LOW
 */
void digitalWrite(uint8_t pin, uint8_t value)
{
	uint32_t bitNum;

	if ((pin < 1) | (pin > NUM_DIOS))
		return;

	bitNum = pin - 1; // Digital I/Os are 1-based, but the get/set DataDirection methods are 0-based
	if (value == HIGH) {
		io.SetDataBit(bitNum);
	} else {
		io.ResetDataBit(bitNum);
	}
}

/**
 * Reads the value from a specified digital pin, either HIGH or LOW
 *
 * @param pin the number of the digital I/O whose mode you wish to set.
 * Value ranges from 1 to 16 corresponding to digital I/Os D1 - D16.
 * @return HIGH or LOW or -1 if pin is invalid
 */
int digitalRead(uint8_t pin)
{
	uint32_t bitNum;
	uint32_t data;
	int rv;

	if ((pin < 1) | (pin > NUM_DIOS))
		return -1;

	bitNum = pin - 1; // Digital I/Os are 1-based, but the get/set DataDirection methods are 0-based

	data = io.GetData();

	if (data & (1<<bitNum)) {
		rv = HIGH;
	} else {
		rv = LOW;
	}
	return rv;
}
