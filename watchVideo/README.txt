This is c++ code for the vexpro controller which sets up a videoserver providing video on port 5005, 
and java code for a PC which connects to the vexpro, pulls off raw video, and displays it. It can optionally
connect to roborealm and send the video to roborealm and pull data back. Run each command with -h to get a help listing,
or read the source for details of options.

uvcserver -m sets it to use MJPEG mode from the camera; default is YUYV encoding. Default resolution is 160 x 120 px.

Open the java app in eclipse or compile it. Run it with the parameter --videoServer <IP of VEXpro>. Default resolution is 160 x 120 px.


