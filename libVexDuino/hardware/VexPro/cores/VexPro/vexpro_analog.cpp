/**
  @file vexpro_analog.cpp - analog input and output

  API for analog I/O that is compatible with Arduino (except the pins are different).
  Uses libqwerk.
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

#include "wiring.h"
/**
 * Unsupported on VexPro
 */
void analogReference(uint8_t mode)
{

}

int analogRead(uint8_t pin)
{
return 0;
}

// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
void analogWrite(uint8_t pin, int val)
{

}
