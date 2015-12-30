This program connects to Roborealm running on a PC, sends images from the webcam to roborealm, and pulls back the shape data and prints it. It assumes the video processing chain running in Roborealm includes Shape_Match module, which generates the SHAPES data below.

The PC running roborealm is at the IP address specified with the RRSERVER #define near the beginning of rrclient.c, and can be overridden on the command-line with the -i flag. Be sure you've opened up port 6060 on the Windows firewall or the program won't be able to connect.

The output data is defined here:
http://www.roborealm.com/help/Shape_Match.php
And is:
Offset      Contents
0           Match Confidence 0-100
1           Orientation 0-360
2           Relative Size
3           X min coordinate of bounding box
4           X max coordinate of bounding box
5           Y min coordinate of bounding box
6           Y max coordinate of bounding box
7           Path start index
8           Length of path

Import the project into the Terk IDE by right-clicking in the project explorer and selecting "Import", to import the project. It will build automatically.

A suitable Roborealm program is included: "can.robo". You'll need to capture your own can images (snap them in roborealm and save them away somewhere) and configure the Shape_Match module to look where you saved the templates. Paint is useful for cleaning up the templates.