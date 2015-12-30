#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "RR_API.h"

// we're going to run RoboRealm on our local machine
#define SERVER "192.168.15.104"

// use the default API port number of 6060
#define PORTNUM 6060

/////////////////////////////// Main //////////////////////////////////////////

// This is where the program first starts
int main(int argc, char *argv[])
{
	int width, height;
	unsigned char *image = (unsigned char *)malloc(320*240*3);
	char buffer[64];

	RR_API rr;

	// open up RoboRealm if it is not already running. Note you may need to change
	// the path on this. We use the default port number ... if you need two instances of RoboRealm
	// running start one up on one port number and another on a different port.
	//rr.open("c:\\www\\RoboRealm\\bin\\RoboRealm.exe", PORTNUM);

	// connect to the RoboRealm application given the default port number ... or a different one
	// but it should be the same one as used above in the open command.
	if (!rr.connect(SERVER, PORTNUM))
	{
		printf("RoboRealm does not appear to be running.\nPlease run RoboRealm and try again.");
		exit(0);
	}

	// now we are connected to the RR server we can try many of the API routines. Note that
	// the routines below are shown as examples and do not perform a specific task
/*
	// get the current RoboRealm version
	char version[16];
	rr.getVersion(version);

	// first check the size is 320x240
	rr.getCameraFormat(&width, &height);
	if ((width!=320)||(height!=240))
		rr.setCameraFormat(320, 240);

	char *propertyNames[] = {"brightness", "contrast"};
	int propertyValues[2];
	int propertyMin[2];
	int propertyMax[2];
	int propertyAuto[2];
	rr.getCameraProperties(propertyNames, propertyValues, propertyMin, propertyMax, propertyAuto, 2);

	// if things are too bright then
	if (propertyValues[0]>1000)
	{
		// set the brightness down to 1000
		propertyValues[0]=1000;
		// we are resuing the above array structure but only the first (brightness) entry
		rr.setCameraProperties(propertyNames, propertyValues, propertyAuto, 1);
	}
*/
/*
	// Or get all properties ...
	char propertyStore[16*32];
	char *propertyNames[16];
	int propertyValues[16];
	int propertyMin[16];
	int propertyMax[16];
	int propertyAuto[16];
	for (int i=0;i<16;i++) 
	{
		propertyNames[i]=&propertyStore[i*32]; 
		propertyNames[i][0]=0;
	}
	rr.getCameraProperties(propertyNames, propertyValues, propertyMin, propertyMax, propertyAuto, 16);
*/

	// set a custom variable to test
	rr.setVariable("custom_var", "test");

	// read back our custom variable ... should be equal to 'test'
	rr.getVariable("custom_var", buffer, 64);
	if (strcasecmp(buffer, "test")!=0)
	{
		printf("Error in custom_var");
		getchar();
		exit(-1);
	}

	// delete our custom variable
	if (!rr.deleteVariable("custom_var"))
	{
		printf("Error in delete variable");
		getchar();
		exit(-1);
	}

	// try to get it back again ... should be empty
	rr.getVariable("custom_var", buffer, 64);
	if (buffer[0]!=0)
	{
		printf("Error in delete custom_var");
		getchar();
		exit(-1);
	}

	char *values[2];
	char valueBuffer[128];
	values[0]=valueBuffer;
	values[1]=&valueBuffer[64];
	char *names[2];
	names[0]="custom_var_1";
	names[1]="custom_var_2";

	strcpy(values[0], "test1");
	strcpy(values[1], "test2");
	rr.setVariables(names, values, 2);

	// get the both cogx and cogy variable
	char *results[2];
	char resultBuffer[128];
	memset(resultBuffer, 0 ,sizeof(resultBuffer));
	results[0]=resultBuffer;
	results[1]=&resultBuffer[64];
	if (rr.getVariables("custom_var_1, custom_var_2", results, 128, 2)!=2)
	{
		printf("Error in GetVariables, did not return 2 results");
		getchar();
		exit(-1);
	}
	else
	{
		if (strcasecmp(results[0], "test1")!=0)
		{
			printf("Error in get/set multiple variables");
			getchar();
			exit(-1);
		}
		if (strcasecmp(results[1], "test2")!=0)
		{
			printf("Error in get/set multiple variables");
			getchar();
			exit(-1);
		}
	}
/*
	// ensure that the camera is on and processing images
	rr.setCamera("on");
	rr.run("on");

	// get the current image dimention
	rr.getDimension(&width, &height);

	// get the current processed image from RoboRealm and save as a PPM
	if (rr.getImage(image, &width, &height, 320*240*3))
	{
		if (width*height*3>1280*1024*3)
		{
			printf("Processed image width/height error!");
			exit(0);
		}
		rr.savePPM("c:\\temp\\test.ppm", image, width, height);
	}

	// get the current "source" image from RoboRealm and save as a PPM
	if (rr.getImage("source", image, &width, &height, 320*240*3, "RGB"))
	{
		if (width*height*3>1280*1024*3)
		{
			printf("Source image width/height error!");
			exit(0);
		}
		rr.savePPM("c:\\temp\\test2.ppm", image, width, height);
	}

	// we can also grab other variables such as the image_count
	rr.getVariable("image_count", buffer, 64);
	if (atoi(buffer)<=0)
	{
		printf("Error in getting image_count");
		getchar();
		exit(-1);
	}

	// turn off live camera
	rr.setCamera("off");
*/
	// load an image for experimentation
	rr.loadPPM("c:\\Program\\ Files(x86)\\RoboRealm\\remo.ppm", image, &width, &height, 320*240*3);
/*
	// change the current image
	rr.setImage(image, width, height);

	// add a marker image called mt_new_image
	rr.setImage("my_new_image", image, width, height);

	// execute a RGB filter on the loaded image
	rr.execute("<head><version>1.50</version></head><RGB_Filter><min_value>40</min_value><channel>3</channel></RGB_Filter>");

	// get the current RGB_Filter parameter
	rr.getParameter("RGB_Filter", 1, "min_value", buffer, 64);

	// change it to 60
	rr.setParameter("RGB_Filter", 1, "min_value", "60");

	// run a .robo program
	rr.loadProgram("c:\\www\\RoboRealm\\scripts\\red.robo");

	// grab the current running .robo program
	char xml[8192];
	rr.getProgram(xml, 8192);

	// load an image from disk
	rr.loadImage(NULL, "c:\\www\\RoboRealm\\bin\\remo.gif");

	// save that image back to disk .. note that we can switch extensions
	rr.saveImage(NULL, "c:\\temp\\remo.jpg");

	rr.setCamera("on");
	// change the camera to another one
	rr.setCamera("CompUSA PC Camera");
	Sleep(2000);
	// now set it back
	rr.setCamera("Logitech");

	// turn off processing
	rr.run("off");
	Sleep(2000);
	// run once
	rr.run("once");
	Sleep(2000);
	// run for 100 frames (~3.3 seconds)
	rr.run("100");
	Sleep(2000);
	// turn processing back on
	rr.run("on");

	// wait for the image count to exceed 1000 (assuming a 30 fps here)
	rr.waitVariable("image_count", "500", 100000);

	// wait for a new image
	rr.waitImage();

	// save a processed version of an image on disk
	rr.loadImage(NULL, "c:\\www\\RoboRealm\\bin\\remo.gif");
	rr.waitImage();
	rr.saveImage("processed", "c:\\temp\\test.jpg");

	// load an image based on compressed bytes
	FILE *fp = fopen("c:\\www\\RoboRealm\\bin\\remo.gif", "rb");
	if (fp!=NULL)
	{
		int len = fread((char *)image, 1, 320*240*3, fp);
		fclose(fp);
		rr.setCompressedImage(image, len);
	}

	// test out some window interface routines
	rr.minimizeWindow();
	rr.maximizeWindow();
	rr.moveWindow(100,100);
	rr.resizeWindow(200,200);

	// get and set a gray image instead of RGB
	rr.getImage("processed", image, &width, &height, 320*240*3, "GRAY");
	rr.setImage("source", image, width, height, false, "GRAY");

	// close the RoboRealm application .. if you want too ... otherwise leave it running
	rr.close();
*/
	// disconnect from API Server
	rr.disconnect();

	// clean up our image buffer
	free(image);

	printf("Done");
	getchar();

	return 1;
}
