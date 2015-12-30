/*
 * MotorCmd.h
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 */

#ifndef MOTORCMD_H_
#define MOTORCMD_H_

class Layer;

class MotorCmd {
public:
	MotorCmd(Roomba *roomba);
	virtual ~MotorCmd();
	void setSpeed(Layer * activeLayer);
	void printData();

private:
	Roomba *roomba;
	int lastVel, lastRot;
};

#endif /* MOTORCMD_H_ */
