/*
 * MissionControl.h
 *
 *  Created on: May 10, 2012
 *      Author: bouchier
 */

#ifndef MISSIONCONTROL_H_
#define MISSIONCONTROL_H_

#include "Layer.h"

class RoombaSensors;
class Target;
class Point2VisTarget;
class Spin;
class Subsumption;
class Bump;
class Cruise;

class MissionControl : public Layer {
public:
	MissionControl(RoombaSensors *rs, Target *t, Point2VisTarget *pv, Spin *s, Bump *b, Cruise *c);
	void eval();
	void setMission(int algorithm, int param);
	void setSubPtr(Subsumption *subIn);

private:
	Subsumption *sub;
	RoombaSensors *rs;
	Target *t;
	Point2VisTarget *pv;
	Spin *s;
	Bump *b;
	Cruise *c;
	int state;
	int stateTimer;
	int target[3];	// 20" line

	void roboColumbusSm();
	void point2TargetSm();
	void tableBotSm();
};

#endif /* MISSIONCONTROL_H_ */
