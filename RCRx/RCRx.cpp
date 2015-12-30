/*
 * RCRx.cpp
 *
 *  Created on: Nov 9, 2012
 *      Author: bouchier
 */
/*!
 * \file RCRx.cpp
 * \brief Source code for the R/C receiver class
 *
 * This class measures the pulse width of the positive pulse on the specified digital input
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "qegpioint.h"
#include "RCRx.h"


//! Instantiate an instance of an R/C receiver object
/*!

 * \param dioNumIn The Digital I/O number (1 - 16) on the VEXPro controller to which the R/C receiver
 * channel is wired
 */
RCRx::RCRx(int dioNumIn, CQEGpioInt& gpioIn) : gpio(gpioIn) {
	//CQEGpioInt &gpio = CQEGpioInt::GetRef();
	unsigned int i;

	dioIndex = dioNumIn - 1;
	i = (unsigned int)dioIndex;
	waitRising = true;
	//gpio.RegisterCallback(i, NULL, callback);
	gpio.RegisterCallback(0,NULL,&RCRx::callback);
	gpio.SetInterruptMode(0, QEG_INTERRUPT_NEGEDGE);
}

RCRx::~RCRx() {
	CQEGpioInt::Release();
}

void RCRx::callback(unsigned int io, struct timeval *ptv, void *userdata)
{
	if (waitRising)		// we just received the rising edge we were waiting for
	{
		tvRising = *ptv;
		waitRising = false;
		printf("+:");
		gpio.SetInterruptMode(dioIndex, QEG_INTERRUPT_NEGEDGE);
	} else {
		lastPw = diff(&tvRising, ptv);
		printf("-: %d\n", lastPw);
		gpio.SetInterruptMode(dioIndex, QEG_INTERRUPT_POSEDGE);
	}

}

unsigned long RCRx::diff(struct timeval *ptv0, struct timeval *ptv1)
{
  long val;

  val = ptv1->tv_usec - ptv0->tv_usec;
  val += (ptv1->tv_sec - ptv0->tv_sec)*1000000;

  return val;
}

int RCRx::getRCPulse()
{
	return lastPw;
}
