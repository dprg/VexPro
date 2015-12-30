/*
 * roombaSensors.cpp
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 *
 *      The odometers code was created by David P. Anderson and obtained from this site:
 *      http://www.geology.smu.edu/~dpa-www/robo/Encoder/imu_odo/
 */
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/shm.h>
#include <errno.h>
#include "RoombaSensors.h"

// Sonar defines
#define USPI 150
#define BIAS 300

#define KEY2 (key_t)6061
#define SHM_SZ 1024
#define SHMFLAGS 0666
volatile int *sonar_p;


RoombaSensors::RoombaSensors(Roomba *roombaIn) {
	roomba = roombaIn;
	first = true;		// initialize first-time encoder position flag
	X_pos = 0.0;
	Y_pos = 0.0;
	theta = 0.0;
	initSonar();
}

void RoombaSensors::readSensors( char *timestampString)
{

	// Read sensors
	roomba_read_sensors(roomba);
	roomba_read_encoders(roomba);
	odometers();
	sonarRange = *sonar_p;
	//printf("Sonar read %d\n", sonarRange);

}

void RoombaSensors::printData()
{
	//printf("%s bump: %d %d\n", timestampString, bump_left(roomba->sensor_bytes[0]), bump_right(roomba->sensor_bytes[0]));
	printf("X_pos: %0.1f Y_pos: %0.1f theta %3.0f deg", X_pos, Y_pos, theta*RADS);
}

bool RoombaSensors::healthCheck()
{
	int voltage;
	int rv;

	rv = roomba_read_sensors(roomba);
	if (rv < 0) {
		printf("healthCheck failed - ** IS ROOMBA TURNED ON? ** roomba_read_sensors returned %d\n", rv);
		return false;
	}
	voltage = roomba->sensor_bytes[17] * 256 + roomba->sensor_bytes[18];
	if (voltage == 0) {
		printf("healthCheck failed; roomba_read_sensors indicated voltage = 0\n");
		return false;
	}
	printf("Roomba voltage: %d mV\n", voltage);
	return true;
}

short RoombaSensors::difference(unsigned short val, unsigned short lastval)
{
	return (short)(val - lastval);
}

void RoombaSensors::odometers()
{

    /* sample the left and right encoder counts */
	// FIXME handle rollover/under
    lsamp = roomba->lEncoder;
    rsamp = roomba->rEncoder;

    if (first) {
    	// first time through loop, initialize distance travelled to 0
        last_left = lsamp;
        last_right = rsamp;
    	first = false;
    }
    /* determine how many ticks since our last sampling? */
    L_ticks = difference(lsamp, last_left);
    R_ticks = difference(rsamp, last_right);

    /* and update last sampling for next time */
    last_left = lsamp;
    last_right = rsamp;

    /* convert longs to floats and ticks to inches */
    left_inches = (float)L_ticks/LEFT_CLICKS_PER_INCH;
    right_inches = (float)R_ticks/RIGHT_CLICKS_PER_INCH;

    /* calculate distance we have traveled since last sampling */
    inches = (left_inches + right_inches) / 2.0;

    /* accumulate total rotation around our center */
    deltaTheta = (left_inches - right_inches) / WHEEL_BASE;
    theta += deltaTheta;

    /* and clip the rotation to plus or minus 360 degrees */
    theta -= (float)((int)(theta/TWOPI))*TWOPI;

    /* now calculate and accumulate our position in inches */
    Y_pos += inches * cos(theta);
    X_pos += inches * sin(theta);

    if ( L_ticks > 1000 || R_ticks > 1000) {
    	printf("ERROR: *** ticks has an unreasonable value **** L_ticks: %d R_ticks: %d X_pos: %0.1f Y_pos: %0.1f\n",
    			L_ticks, R_ticks, X_pos, Y_pos);
    }
}

void RoombaSensors::initSonar()
{

		fprintf(stderr, "attaching shared memory for sonar data\n");
		int shmid;

		if ((shmid = shmget(KEY2, SHM_SZ, SHMFLAGS)) == -1) {
			perror("shmget");
			exit(errno);
		}

		if ((sonar_p = (int *)shmat(shmid, (void*)0, 0)) == (void*)-1) {
			perror("shmat");
			exit(errno);
		}
		fprintf(stderr, "attached shared memory at address 0x%x\n", (int)sonar_p);

}



// getters/setters
bool RoombaSensors::getLBumper()
{
	return bump_left(roomba->sensor_bytes[0])?true:false;
}

bool RoombaSensors::getRBumper()
{
	return bump_right(roomba->sensor_bytes[0])?true:false;
}

int RoombaSensors::getCliff()
{
	return roomba->sensor_bytes[2] | (roomba->sensor_bytes[3]<<1) | (roomba->sensor_bytes[4]<<2) | (roomba->sensor_bytes[5]<<3);
}

bool RoombaSensors::getWheelDrop()
{
	return ((wheeldrop_left(roomba->sensor_bytes[0])?true:false) || (wheeldrop_right(roomba->sensor_bytes[0])?true:false));
}

float RoombaSensors::getTheta()
{
    return theta;
}

float RoombaSensors::getDeltaTheta()
{
    return deltaTheta;
}

float RoombaSensors::getX_pos()
{
    return X_pos;
}

float RoombaSensors::getY_pos()
{
    return Y_pos;
}

int RoombaSensors::getSonarRange()
{
    return sonarRange;
}

