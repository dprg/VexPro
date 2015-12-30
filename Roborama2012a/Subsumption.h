/*
 * Subsumption.h
 *
 *  Created on: Jan 20, 2012
 *      Author: bouchier
 */

#ifndef SUBSUMPTION_H_
#define SUBSUMPTION_H_

class WheelDrop;
class Cruise;
class Bump;
class Target;
class Point2VisTarget;
class MissionControl;
class StopBot;
class Layer;
class Metro;
class RoombaSensors;
class MotorCmd;
class CTextLcd;
class Spin;
class VirtualCage;
class Grab;


class Subsumption {
public:
	Subsumption(int algorithm, int arg, int *waypointList, Roomba *roomba);
	int go();
	char *getActiveLayerName() 	{return(activeLayerName);}
	int getAlgorithm() 	{return(algorithm);}
	int getParam() 		{return(param);}

private:
	unsigned int endMillis;	// when to end the program & exit
	char timestampString[30];	// printable string for timestamp
	int now;
	Metro *heartMetro;
	Roomba *roomba;
	RoombaSensors *roombaSensors;
	MotorCmd *motorCmd;
	int *wayPointList;
	char *activeLayerName;
	int algorithm;
	int param;

	WheelDrop *wheeldrop;
	Grab *grab;
	Bump *bump;
	Target *target;
	Spin *spin;
	VirtualCage *virtualCage;
	Cruise *cruise;
	Point2VisTarget *point2VisTarget;
	MissionControl *missionControl;
	StopBot *stopBot;                     // the default layer
	Layer *this_layer;      // output, layer chosen by arbitrator()
	Layer *algLayers[16];				// allow up to 16 layers
	Layer *alg0Layers[16];				// allow up to 16 layers
	Layer *alg1Layers[16];
	Layer *alg2Layers[16];

	Layer **job;                    // pointer to job priority list
	int job_size;                   // number of tasks in priority list
	int arbitrateEnable;              // global flag to enable subsumption
	int halt;                   // global flag to halt robot

	int arbitrate();
};

extern "C" void startSubsumption(int algorithm, int arg, int *waypointList, Roomba *roomba);

#endif /* SUBSUMPTION_H_ */
