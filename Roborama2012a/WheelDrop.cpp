/*
 * WheelDrop.cpp
 *
 *  Created on: Jan 27, 2012
 *      Author: bouchier
 */

#include <stdio.h>
#include "WheelDrop.h"
#include "RoombaSensors.h"

WheelDrop::WheelDrop(RoombaSensors* rsIn) : Layer() {
	state = 0;
	rs = rsIn;
	// initialize Layer. Other variables initialized inside Layer
	layerName = (char *)"WheelDrop";
}

void WheelDrop::eval()
{
	bool wheeldrop = rs->getWheelDrop();

	switch(state) {
	case 0:
		if (wheeldrop) {
			state = 1;
			flag = 1;	// subsume all other behaviors till someone rescues roomba
			printf("HELP - my wheel has dropped and I can't get up\n");
		}
		break;
	case 1:				// stay in this state until wheeldrop becomes false
		if (wheeldrop)
			break;
		state = 2;			// move to state to switch back to safe mode
		stateTimer = 20;	// 1 sec to switch back to safe mode
		cmd = ROOMBA_MODE;
		cmdArg = 2;
		printf("Putting roomba back into safe mode & waiting 1 sec\n");
		break;
	case 2:				// wait for roomba to go back into safe mode
		stateTimer--;
		if (stateTimer)
			break;
		state = 0;
		flag = 0;			// stop subsuming
		break;


	}
}
