/****************************************************************************
*     Copyright (C) 2009  Paul Bouchier                                     *
*                                                                           *
*     This program is free software: you can redistribute it and/or modify  *
*     it under the terms of the GNU General Public License as published by  *
*     the Free Software Foundation, either version 3 of the License, or     *
*     (at your option) any later version.                                   *
*                                                                           *
*     This program is distributed in the hope that it will be useful,       *
*     but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*     GNU General Public License for more details.                          *
*                                                                           *
*     You should have received a copy of the GNU General Public License     *
*     along with this program.  If not, see <http://www.gnu.org/licenses/>, *
*     or the LICENSE-gpl-3 file in the root directory of this repository.   *
****************************************************************************/

#include <stdio.h>
#include "pid.h"

//! Construct the PID controller
PID::PID(float p, float i, float d)
{
	debugFlag = false;
    initPid(p, i, d);
}

//! Construct the PID controller
PID::PID(float p)
{
	debugFlag = false;
	initPid(p, 0.0, 0.0);
}

void PID::initPid(float p, float i, float d)
{
	k_p = p;
	k_i = i;
	k_d = d;
	d_state = 0.0;
	i_state = 0.0;
	i_state_max = 100.0;
}

float PID::computePid( float value, float target )
{
    float error;
    float p, i, d;
    float ret;

    error = target - value;
    p = k_p * error;
    d = k_d * (error - d_state );
    d_state = error;
    i_state += error;

    // cap I term windup
    if( i_state > i_state_max )
            i_state = i_state_max;
    if( i_state < -i_state_max )
            i_state = -i_state_max;

    i = k_i * i_state;

    ret = p + i + d;

    if (debugFlag) {
    	printf("value: %0.2f target %0.2f output: %0.2f\n", value, target, ret);
    }
    return ret;
}
