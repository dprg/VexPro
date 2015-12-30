/**
 * @file readHM6352Compass.h Routines to initialize & read the HM6352 compass
 */
/*
   This library is free software; you can redistribute it and/or
  modify it without limitation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef READCOMPASS_H_
#define READCOMPASS_H_

#define HMC6352_ADDR (0x21)

#define MODE_CONTINUOUS 0
#define MODE_STANDBY 1
#define MODE_INVALID -1

bool SetHM6352Mode(int);
unsigned short ReadHM6352(void);
bool SleepHM6352();
bool WakeHM6352();

#endif /* READCOMPASS_H_ */
