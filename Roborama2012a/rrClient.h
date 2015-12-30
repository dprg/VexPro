/*
 * rrClient.h
 *
 *  Created on: May 6, 2012
 *      Author: bouchier
 */

#ifndef RRCLIENT_H_
#define RRCLIENT_H_

typedef struct {
	volatile int structInitialized;
	volatile int getShapeParams;
	volatile int shapeParams[2];
	volatile int sonarRange;
} rrClientStruct;

#endif /* RRCLIENT_H_ */
