/*
 * roombacmd -- Roomba command-line interface
 *
 * http://hackingroomba.com/
 *
 * Copyright (C) 2006, Tod E. Kurt, tod@todbot.com
 *
 * Updates:
 * 14 Dec 2006 - added more functions to roombalib
 */

#include <stdio.h>  
#include <getopt.h>
#include <stdlib.h>   /* calloc, strtol */
#include <string.h>   /* String function definitions */

#include "roombalib.h"
void roomba_set_velocity( Roomba* roomba, int velocity );

//char *default_argv[] = {"roombaVEXPro", "-p/dev/ttyAM1", "-E", "0"}; // alg 0 bumpturn forever
//int default_argc = 5;
char *default_argv[] = {"roombaVEXPro", "-p/dev/ttyAM1", "-a2", "-E1"};	// alg 1 waypoiint pattern N
int default_argc = 4;
//char *default_argv[] = {"roombaVEXPro", "-p", "/dev/ttyAM1", "-a", "360", "-E", "2"}; // spin
//int default_argc = 7;
//char *default_argv[] = {"roombaVEXPro", "-p", "/dev/ttyAM1", "-E", "3"}; // alg 3 bumpturn forever in virtual cage
//int default_argc = 5;

void usage()
{
    printf("Usage: roombacmd [-B baudrate] -p <serialport> [OPTIONS]\n"
    "\n"
    "Options:\n"
    "  -h, --help                   Print this help message\n"
    "  -p, --port=serialport        Serial port roomba is on\n"
    "  -B, --baud=baudrate          Serial port speed (default 57600)\n"
    "  -v, --verbose=NUM            Be verbosive (use more for more verbose)\n"
    "  -f, --forward                Go forward at current speed\n"
    "  -b, --backward               Go backward at current speed\n"
    "  -l, --spin-left              Spin left at current speed\n"
    "  -r, --spin-right             Spin right at current speed\n"
    "  -s, --stop                   Stop a moving Roomba\n"
    "  -E, --explore=alg            Move using an algorithm. 0: bounce off walls & cliffs. 1: drive a pattern of waypoints. 2: spin. 3: Virtual cage\n"
    "  -w, --wait=millis            Wait some milliseconds\n"
    "  -V, --velocity=val           Set current speed to val (1 to 500)\n"
    "  -S, --sensors                Read Roomba sensors,display nicely\n"
    "  -R, --sensors-raw            Read Roomba sensors,display in hex\n"
    "  -e, --encoders               Read Roomba encoder counts, display in dec and inches\n"
    "  -a, --arg                    Numeric argument. algorithm 1: waypoint pattern. 2: spin amount.\n"
    "  -a, --arg                    Use numeric argument provided"
    "  -L  --loop                   Loop on behavior N times\n"
    "      --debug                  Print out boring details\n"
    "\n"
    "Examples:\n"
    " roombaVEXPro -p /dev/ttyAM1 -E0 // alg 0 bumpturn forever\n"
    " roombaVEXPro -p /dev/ttyAM1 -a1 -E1	// alg 1 waypoint pattern 1 \n"
    " roombaVEXPro -p /dev/ttyAM1 -a360 -E2 // spin \n"
    " roombaVEXPro -p /dev/ttyAM1 -E3 // alg 3 bumpturn forever in virtual cage \n"
    "\n"
    "Notes:\n"
    "- The '-p' port option must be first option and is required.\n"
    "- All options/commands can be cascaded & are executed in order, like:\n"
    "    roombacmd -p /dev/ttyS0 -f -w 1000 -b -w 1000 -s \n"
    "    to go forward for 1 sec, go back for 1 sec, then stop. \n"
    "- If --baud=<baudrate> option used, it must appear before --port=<port>\n"
    "- Default baudrate is 115200 (500-series Roombas), set to 57600 for 400-series\n"
    "\n");
}

int main(int argc, char *argv[]) 
{
    char serialport[256];
    int velocity = 0;
    int waitmillis = 1000;
    int baudrate = 0;
    int loopCnt = 1;
    int i;
    int algorithm;
    int arg;
    int numWaypointLists = 7;
    int waypointList0[] = {2, 0, 24, 0, 0};	// seek 12" foward, then return to 0,0
    int waypointList1[] = {16, 0, 24, 24, 24, 24, 0, 0, 0,
    		0, 24, 24, 24, 24, 0, 0, 0,
    		0, 24, 24, 24, 24, 0, 0, 0,
    		0, 24, 24, 24, 24, 0, 0, 0,
    		}; // 2 foot square
    int waypointList2[] = {1, 0, 100};	// 100" line
    int waypointList3[] = {4, 0, 72, 72, 72, 72, 0, 0, 0};	// 6' square
    int waypointList4[] = {4, 0, 72, -72, 72, -72, 0, 0, 0};	// 6' square
    int waypointList5[] = {4, 0, 24, -24, 24, -24, 0, 0, 0};	// 2' square
    int waypointList6[] = {20, 0, 60, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0,
    		0, 60, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0, 0, 60, 0, 0};	// out & back many times
    int *waypointList = waypointList0;		// default waypoint list
    int *waypointLists[] = {waypointList0, waypointList1, waypointList2, waypointList3,
							waypointList4, waypointList5, waypointList6};

    Roomba* roomba = NULL;
    
    if (!strncmp(argv[0], "roombaVEXProd", 13)) {
    	//printf("sleeping for 60 sec\n");
    	sleep(90);		// if run with d suffix, we're doing auto startup, delay for startup to finish
    }

    init();		// initialize the Arduino timing code

    if (argc == 1) {
    	printf("No args provided; using default: ");
    	for(i=0; i<default_argc; i++) {
    		printf("%s ", default_argv[i]);
    	}
    	printf("\n");
    	argc = default_argc;
    	argv = default_argv;
    }
    /* parse options */
    int option_index = 0, opt;
    static struct option loptions[] = {
        {"help",       no_argument,       0, 'h'},
        {"port",       required_argument, 0, 'p'},
        {"baud",       required_argument, 0, 'B'},
        {"reset",      no_argument,       0, 0},
        {"drive",      required_argument, 0, 'd'},
        {"forward",    optional_argument, 0, 'f'},
        {"backward",   optional_argument, 0, 'b'},
        {"spin-left",  optional_argument, 0, 'l'},
        {"spin-right", optional_argument, 0, 'r'},
        {"stop",       no_argument,       0, 's'},
        {"wait",       optional_argument, 0, 'w'},
        {"velocity",   required_argument, 0, 'V'},
        {"sensors",    no_argument,       0, 'S'},
        {"sensors-raw",no_argument,       0, 'R'},
        {"spy",        no_argument,       0, 'Y'},
        {"debug",      no_argument,       0, 'D'},
        {"encoders",   no_argument,       0, 'e'},
        {"explore",    optional_argument, 0, 'E'},
        {"waypointlist", required_argument, 0, 'W'},
        {"loop",       required_argument, 0, 'L'},
        {0,0,0,0}
    };

    while(1) {
        opt = getopt_long (argc, argv, "hp:B:d:fblrsw:V:SRDeE:a:",
                           loptions, &option_index);
        if (opt==-1) break;
        
        switch (opt) {
        case 0:
            //if (!(strcmp("dir",loptions[option_index].name)))
            //		strcpy(dir, optarg);
            //else if(!(strcmp("period",loptions[option_index].name)))
            //    period = strtol(optarg,NULL,10);
            //else if(!(strcmp("imgsize",loptions[option_index].name)))
            break;
        case 'h':
            usage();
        case 'B':
            baudrate = strtol(optarg,NULL,10);
            if( baudrate == 57600 ) 
                baudrate = B57600;  // have to do this because defines != values
            else if( baudrate == 115200 ) 
                baudrate = B115200;
            break;
        case 'p':
            strcpy(serialport,optarg);
            roomba = roomba_init( serialport, baudrate );
            if( roomba == NULL ) {
                fprintf(stderr,"error: couldn't open roomba\n");
                return -1;
            }
            break;
        case 'f':
            if(roomba) roomba_forward( roomba );
            break;
        case 'b':
            if(roomba) roomba_backward( roomba );
            break;
        case 'l':
            if(roomba) roomba_spinleft( roomba );
            break;
        case 'r':
            if(roomba) roomba_spinright( roomba );
            break;
        case 's':
            if(roomba) roomba_stop( roomba );
            break;
        case 'L':
        	loopCnt = strtol(optarg, NULL, 10);
        	break;
        case 'a':
        	arg = strtol(optarg, NULL, 10);
        	if (arg < numWaypointLists) {
        		waypointList = waypointLists[arg];
        	} else {
            	waypointList = waypointLists[0];
        	}
        	break;
        case 'E':
        	algorithm = strtol(optarg, NULL, 10);
        	if (roomba) startSubsumption(algorithm, arg, waypointList, roomba);
        	break;
        case 'w':
            waitmillis = strtol(optarg,NULL,10);
            roomba_wait( waitmillis );
            break;
        case 'V':
            velocity = strtol(optarg,NULL,10);
            if(roomba) roomba_set_velocity( roomba, velocity );
            break;
        case 'S':
            if(roomba) {
                roomba_read_sensors(roomba);
                roomba_print_sensors(roomba);
            }
            break;
        case 'R':
            if(roomba) {
                roomba_read_sensors(roomba);
                roomba_print_raw_sensors(roomba);
            }
            break;
        case 'e':
            if(roomba) {
                roomba_read_encoders(roomba);
                printf("Encoders: Left: %8d (%8.1f\") Right: %8d (%8.1f\")\n",
                		roomba->lEncoder,
                		(float)(roomba->lEncoder)/ROOMBA_COUNTS_PER_INCH,
                		roomba->rEncoder,
                		(float)(roomba->rEncoder)/ROOMBA_COUNTS_PER_INCH);
            }
            break;
        case 'D':
            roombadebug++;
        case '?':
            break;
        }
    }
    if (argc==1) {
        usage();
        return 0;
    }
    
    //printf("argv0:%s\n", argv[0]);
    if( roomba ) roomba_close( roomba );

    return 0;
}


