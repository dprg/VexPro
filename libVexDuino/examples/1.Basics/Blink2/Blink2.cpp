/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  This example code is in the public domain.
 */

/*
 * Tools should remove the need for this section
 */
#include <stdio.h>
#include "Arduino.h"

int waitABit(int);

void setup() {
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode(13, OUTPUT);
}

void loop() {
  int rv;

  digitalWrite(13, HIGH);   // set the LED on
  printf("wrote 13 high\n");
  rv = waitABit(1000);              // wait for a second
  digitalWrite(13, LOW);    // set the LED off
  printf("wrote 13 low\n");
  rv = waitABit(1000);              // wait for a second
}

int waitABit(int x)
{
	delay(x);
	return 0;
}
