#include <stdio.h>
#include <unistd.h>
#include "qegpioint.h"
#include "RCTest.h"

#define USPI 150
#define BIAS 300

extern CQEGpioInt &gpio;

unsigned long diff(struct timeval *ptv0, struct timeval *ptv1)
{
  long val;

  val = ptv1->tv_usec - ptv0->tv_usec;
  val += (ptv1->tv_sec - ptv0->tv_sec)*1000000;

  return val;
}

void callback(unsigned int io, struct timeval *ptv, void *userData)
{
	int pw;
	struct RCChannel *rcc_p;

	rcc_p = (struct RCChannel *)userData;
	pw = diff(&(rcc_p->tv0), ptv);
	rcc_p->tv0 = *ptv;
	if (rcc_p->flag) {
		gpio.SetInterruptMode(rcc_p->dioIndex, QEG_INTERRUPT_NEGEDGE);
		rcc_p->flag = 0;

	} else {
		gpio.SetInterruptMode(rcc_p->dioIndex, QEG_INTERRUPT_POSEDGE);
		rcc_p->flag = 1;
		if (pw < 2500)
			rcc_p->pulseWidth = pw;
		else
			rcc_p->pulseWidth = 0;
		//printf("%d\n", pw);
	}
}

int initRCTest(CQEGpioInt& gpio, struct RCChannel *rcc_p)
{
  gpio.SetDataDirection(0x0000);
  gpio.RegisterCallback(rcc_p->dioIndex, (void *)rcc_p, callback);
  gpio.SetInterruptMode(rcc_p->dioIndex, QEG_INTERRUPT_POSEDGE);
  return 0;
}

