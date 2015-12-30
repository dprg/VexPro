/*
 * pid.h
 *
 *  Created on: Nov 3, 2012
 *      Author: bouchier
 */

#ifndef PID_H_
#define PID_H_

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

/*! \class PID
 * \brief This class implements the PID algorithm
 *
 * Instantiate the class with default KP, KI, KD parameters. These can
 * be changed with initPid
 */
class PID {

	float k_p, k_i, k_d, i_state_max;               // the PID constants
	float d_state, i_state;   // the PID states

public:
	PID(float p, float i, float d);
	PID(float p=1.0);
	void initPid(float p, float i, float d);
	float computePid( float value, float target );

    void setDebugFlag(bool debugFlag = false)
    {
        this->debugFlag = debugFlag;
    }

private:
        bool debugFlag;
};

#endif /* PID_H_ */
