/*
 * RCRx.h
 *
 *  Created on: Nov 9, 2012
 *      Author: bouchier
 */
/*! \file RCRx.h
 * \brief Header file for the R/C receiver interface
 */

#ifndef RCRX_H_
#define RCRX_H_

class CQEGpioInt;

/*! \class RCRx
 * \brief Measure pulse width from an R/C receiver.
 *
 * This class measures the pulse width on a single R/C channel. It detects loss of pulses by assuming
 * it gets called every 50ms frame. If it doesn't see a pulse after being called 3 times in
 * a row (assumed to be in 3 frametimes) it returns 0 for pulsewidth.
 *
 */
class RCRx {
public:
	RCRx(int dioNumIn, CQEGpioInt& gpioIn);
	virtual ~RCRx();
	int getRCPulse();					// returns 0 for no pulse for last 2 polls, or pulse width in us

private:
	unsigned long diff(struct timeval *ptv0, struct timeval *ptv1);
	void callback(unsigned int io, struct timeval *ptv, void *userdata);

	CQEGpioInt &gpio;
	int lastPw;							// last measured pulse width
	unsigned int dioIndex;						// io index of the DIO
	timeval tvRising;			// timeval of the last rising edge
	bool waitRising;					// true if system is waiting for a rising edge
};

#endif /* RCRX_H_ */
