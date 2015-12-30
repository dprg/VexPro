/*
 * Layer.h
 *
 *  Created on: Jan 22, 2012
 *      Author: bouchier
 */

#ifndef LAYER_H_
#define LAYER_H_

// cmd defines
#define VEL_CMD 0
#define ROOMBA_MODE 1
// Layer is an abstract class. Subclasses implement eval()

class Layer {
public:
	Layer();
	Layer(int speed);
	virtual void eval() =0;	// pure virtual function
	int slew(int requested, int rate);

protected:
    int cmd;				// command to do motion or something else
    int cmdArg;				// argument to the command, other than velocity request
    int velRqst;                 // assertion command
    // rotRqst is a modified roomba radius: 0 = drive straight (no rotation) at velRqst. 1, -1 = spin right, left. other values are a turn radius
    int rotRqst;                 // assertion argument
    int flag;                // subsumption flag indicating we want to activate this later when true
    char *layerName;
    int top_speed;			// max speed
    int bot_speed;			// current bot speed

public:
    // getters & setters
    int getCmd() const
    {
        return cmd;
    }

    void setCmd(int cmdIn)
    {
    	cmd = cmdIn;
        return;
    }

    int getCmdArg() const
    {
        return cmdArg;
    }

    int getRot() const
    {
        return rotRqst;
    }

    int getVel() const
    {
        return velRqst;
    }

    int getFlag() const
    {
        return flag;
    }

    char *getLayerName() const
    {
        return layerName;
    }

};

#endif /* LAYER_H_ */
