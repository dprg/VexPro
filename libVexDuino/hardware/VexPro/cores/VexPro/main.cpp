#include "Arduino.h"
#include <stdio.h>
#include <math.h>	// For sine function
#include <unistd.h>	// For sleep function
#include <qeservo.h>
#include <qemotortraj.h>
#include <qeanalog.h>


CQEAnalog &analog = CQEAnalog::GetRef();
CQEServo &servo = CQEServo::GetRef();
CQEMotorUser &motor = CQEMotorUser::GetRef();

void init();

int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

