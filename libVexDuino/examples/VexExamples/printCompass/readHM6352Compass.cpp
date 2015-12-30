/**
 * @file readHM6352Compass.cpp Routines to initialize the HM6352 compass
 *
 * The I2C pinout, counting from the I2C label to the right of the connector, is:
 * @li
 * 1: GND
 * 2: VCC
 * 3: SCL
 * 4: SDA
 */
/*
   This library is free software; you can redistribute it and/or
  modify it without limitation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <unistd.h>
#include "libI2C.h"
#include "readHM6352Compass.h"

static int compassMode = MODE_INVALID;	// ensure we don't read compass if it hasn't been initialized

/**
 * Put HM6352 compass into continuous read mode at 20Hz or standby mode. In continuous read mode,
 * compass can be read with quite low latency, and will return the last read value. In standby mode,
 * @return true if operation succeeded, false otherwise
 */
bool SetHM6352Mode(int mode)
{
	bool rv = true;
	unsigned char modeReg;

	compassMode = mode;
	if (mode == MODE_CONTINUOUS) {
		modeReg = 0x62;		// sample 20Hz
	} else {
		modeReg = 0;		// Standby mode
	}

	// do a bunch of I2C writes; if any fail & return false, rv will be set to false
    rv &= I2CStart(HMC6352_ADDR, I2C_WRITE);
    rv &= I2CWriteByte('G');  // Write to RAM
    rv &= I2CWriteByte(0x74); // Mode Control Register
    rv &= I2CWriteByte(modeReg); // set compass mode
    I2CStop();
    if (rv == false) return false;
    usleep(100);   // Compass needs 70us after RAM write

    I2CStart(HMC6352_ADDR, I2C_WRITE);
    I2CWriteByte('G');  // Write to RAM
    I2CWriteByte(0x4e); // Data Format Register
    I2CWriteByte(0x00); // heading mode
    I2CStop();
    usleep(100);   // Compass needs 70us after RAM write

#if 0
    I2CStart(HMC6352_ADDR, I2C_WRITE);
    I2CWriteByte('W');  // Wakeup
    I2CStop();
    usleep(100);   // Compass needs 100us to wake up
#endif

    return true;
}

/**
 * Read compass using A command if in standby mode - wakes it from sleep. Takes 6ms to read.
 * Compass goes back to sleep after returning value.
 * If in continuous mode, just read it
 * @return compass heading in 10ths of a degree
 */
unsigned short ReadHM6352(void)
{
    unsigned short heading;
    bool rv = true;

    if (compassMode == MODE_STANDBY) {
        // send the A command then sleep 10ms
        rv &= I2CStart(HMC6352_ADDR, I2C_WRITE);
        I2CWriteByte('A');  // Read heading
        I2CStop();
        usleep(10000);   // Compass needs 6ms to wake up & do a measurement
        if (rv == false) return -1;
    } else if (compassMode == MODE_INVALID) {
    	return -1;
    }

    // the 'A' command has been sent if necessary, read the heading
    rv = I2CStart(HMC6352_ADDR, I2C_READ);
    if (rv == false) return -1;
    heading = I2CReadWord(I2C_MSB_FIRST, I2C_DONE);
    I2CStop();

    return heading;
}

/**
 * Send HM6352 to sleep
 */
bool SleepHM6352()
{
    bool rv = true;

    rv &= I2CStart(HMC6352_ADDR, I2C_WRITE);
    I2CWriteByte('S');  // Read heading
    I2CStop();
    usleep(10);
	return rv;
}

/**
 * Wakeup HM6352
 */
bool WakeHM6352()
{
    bool rv = true;

    rv &= I2CStart(HMC6352_ADDR, I2C_WRITE);
    I2CWriteByte('W');  // Read heading
    I2CStop();
    usleep(100);
	return rv;
}
