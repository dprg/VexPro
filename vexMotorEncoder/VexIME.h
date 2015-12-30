/*
 * VexIME.h
 *
 *  Created on: Oct 11, 2012
 *      Author: bouchier
 */

#ifndef VEXIME_H_
#define VEXIME_H_

class VexIME {
public:
	typedef enum {
		motor269,
		motor393,
		motor393Speed
	} TmotorType;
	VexIME();		// caller should pass in an I2C object
	virtual ~VexIME();
	//VexIME(CQEI2C i2c, bool clockwise, TmotorType mType)	// pass in an I2C object reference and shaft direction
	//bool readAllCountsSpeeds();
	//bool readCountSpeed(int motorId);
	//float getRadPerSec();
	//float getRevPerSec();
	//int Count();
	//int getDeltaCount();

private:
	//unsigned int count;
	//unsigned int lastCount;
	//unsigned short rawSpeed;


};

#endif /* VEXIME_H_ */
