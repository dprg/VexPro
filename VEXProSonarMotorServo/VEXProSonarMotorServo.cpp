/*
 * VEXProSonarMotorServo.cpp
 *
 *  Created on: Jul 13, 2012
 *      Author: bouchier
 *  Publish the range from a Sonar connected to digital 1 (Input) &
 *  digital 2 (Output) on topic sonar1. Control motor 13 speed by
 *  publishing the desired speed on a ros topic with e.g.
 * $ rostopic pub my_topic std_msgs/Int32 120.
 * Drives a servo or motor on VEXPro motor1 connection to the requested value: 0 - 255
 *  that is received on subscribed topic servo1
 *
 * Note: connector labeled "INPUT" on sonar sensor goes to
 * digital 1 (bit 0), and connector labeled "OUTPUT" goes to
 * digital 2 (bit 1).
 */


#include <ros.h>
#include <std_msgs/Int32.h>
#include <stdio.h>
#include <unistd.h>
#include <qegpioint.h>
#include <qemotoruser.h>
#include <qeservo.h>

ros::NodeHandle  nh;
std_msgs::Int32 range;
ros::Publisher sonar1("sonar1", &range);
CQEMotorUser &motor = CQEMotorUser::GetRef();	// motor singleton
CQEGpioInt &gpio = CQEGpioInt::GetRef();		// GPIO singleton
CQEServo &servo = CQEServo::GetRef();			// servo singleton

char *rosSrvrIpDefault = "192.168.15.149";

#define USPI 150
#define BIAS 300

/*
 * Motor callback - called when new motor speed is published
 */
void motorCb(const std_msgs::Int32& motor13_msg){
	int speed = motor13_msg.data;
	printf("Received subscribed motor speed %d\n", speed);
    motor.SetPWM(0, speed);
}
ros::Subscriber<std_msgs::Int32> motorSub("motor13", motorCb );

/*
 * Servo callback - called when new servo position is published
 */
void servoCb(const std_msgs::Int32& servo1_msg){
	int position = servo1_msg.data;
	printf("Received subscribed servo position %d\n", position);
	servo.SetCommand(0, position);
}
ros::Subscriber<std_msgs::Int32> servoSub("servo1", servoCb );

/*
 * Calculate difference in usec between sonar start & end timevals
 */
unsigned long diff(struct timeval *ptv0, struct timeval *ptv1)
{
	long val;

	val = ptv1->tv_usec - ptv0->tv_usec;
	val += (ptv1->tv_sec - ptv0->tv_sec)*1000000;

	return val;
}

/*
 * Sonar callback. Called at interrupt level when sonar output transitions,
 * indicating end of range measurement
 */
void callback(unsigned int io, struct timeval *ptv, void *userdata)
{
	static struct timeval tv0;
	static int flag = 0;
	int sonarVal;

	if (io==0)
	{
		flag = 1;
		tv0 = *ptv;
	}

	if (io==1 && flag)
	{
		sonarVal = diff(&tv0, ptv);
		if (sonarVal>BIAS)
			sonarVal = (sonarVal-BIAS)/USPI;
		range.data = sonarVal;
		printf("%d\n", sonarVal);
	}
}

int main(int argc, char **argv)
{
	volatile unsigned int d;

	// set IP of rosserial server first from cmd line, then env, then default
	char *rosSrvrIp = getenv("ROSSERIAL_SRVR");	//See if we have an env variable set
	if (rosSrvrIp == NULL)
		rosSrvrIp = rosSrvrIpDefault;		// use the hard-coded default if no environment
	if (argc == 2)				// TODO handle port #
		rosSrvrIp = argv[1];		// override server IP with supplied argument

	/* initialize ROS & subscribers & publishers */
	//nh.initNode();
	nh.initNode(rosSrvrIp);
	nh.advertise(sonar1);	// advertise sonar range topic
	nh.subscribe(motorSub);		// subscribe to motor speed topic
	nh.subscribe(servoSub);		// subscribe to servo position

	// reset bit 0, set as output for sonar trigger
	gpio.SetData(0x0000);
	gpio.SetDataDirection(0x0001);

	// set callbacks on negative edge for both bits 0 (trigger)
	// and 1 (echo)
	gpio.RegisterCallback(0, NULL, callback);
	gpio.RegisterCallback(1, NULL, callback);
	gpio.SetInterruptMode(0, QEG_INTERRUPT_NEGEDGE);
	gpio.SetInterruptMode(1, QEG_INTERRUPT_NEGEDGE);

	// trigger sonar by toggling bit 0
	while(1)
	{
		gpio.SetData(0x0001);
		for (d=0; d<120000; d++);
		gpio.SetData(0x0000);
		usleep(100000);		// the interrupt breaks us out of this sleep
		usleep(500000);		// now really sleep
		sonar1.publish( &range );
		nh.spinOnce();
	}
}


