/*
 * roombaSensors.h
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 */

#ifndef ROOMBASENSORS_H_
#define ROOMBASENSORS_H_

#include "roombalib.h"

#define WHEEL_BASE 9.13
// clicks/inch calibrated on concrete at DMS 1/31/12
#define LEFT_CLICKS_PER_INCH 58.5
#define RIGHT_CLICKS_PER_INCH 58.5

#define TWOPI 6.2831853070		/* nice to have float precision */
#define RADS 57.2958			/* radians to degrees conversion */

class RoombaSensors {
public:
	RoombaSensors(Roomba *roombaIn);
	bool healthCheck();
	void readSensors(char * timestampString);
	bool getLBumper();
	bool getRBumper();
	int getCliff();
	bool getWheelDrop();
	float getTheta();
	float getDeltaTheta();
	float getX_pos();
	float getY_pos();
	void printData();

private:
	Roomba *roomba;
	/* odometer maintains these global variables: */
	float theta;                    /* bot heading */
	float deltaTheta;				/* how much we span since last iteration */
	float X_pos;                    /* bot X position in inches */
	float Y_pos;                    /* bot Y position in inches */

	// variables used to calculate X, Y, theta
	unsigned short lsamp, rsamp, last_left, last_right;
	short L_ticks, R_ticks;
	float left_inches, right_inches, inches;
	bool first;		// first time through the position tracking loop when true

	void odometers();
	short difference(unsigned short val, unsigned short lastval);

};

#endif /* ROOMBASENSORS_H_ */
