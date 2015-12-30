/*
 * roombalib -- Roomba C API
 *
 * http://hackingroomba.com/
 *
 * Copyright (C) 2006, Tod E. Kurt, tod@todbot.com
 * 
 * Updates:
 * 14 Dec 2006 - added more functions to roombalib
 */

#ifndef ROOMBALIB_H_
#define ROOMBALIB_H_

#include <stdint.h>   /* Standard types */
#include <termios.h>  /* POSIX terminal control definitions */

 #ifdef __cplusplus
 extern "C" {
 #endif

#define DEFAULT_VELOCITY 200
#define COMMANDPAUSE_MILLIS 100
#define ROOMBA_COUNTS_PER_INCH 57.0

// holds all the per-roomba info
// consider it an opaque blob, please
typedef struct Roomba_struct {
    int fd;
    char portpath[80];
    uint8_t sensor_bytes[80];
    unsigned short lEncoder, rEncoder;
    int velocity;
} Roomba;

// set to non-zero to see debugging output
extern int roombadebug;

// given a serial port name, create a Roomba object and return it
// or return NULL on error
// if baud is 0, default to standard baudrate (57600)
Roomba* roomba_init( const char* portname, speed_t baud );

// frees the memory of the Roomba object created with roomba_init
// will close the serial port if it's open
void roomba_free( Roomba* roomba );

// sets roomba mode
// Mode: 0 = off, 1 = passive; 2 = safe; 3 = full
void roomba_mode( Roomba* roomba, int mode);

// close the serial port connected to the Roomba
void roomba_close( Roomba* roomba );

// is this Roomba pointer valid (but not necc connected)
int roomba_valid( Roomba* roomba );

// return the serial port path for the given roomba
const char* roomba_get_portpath( Roomba* roomba );

// send an arbitrary length roomba command
int roomba_send( Roomba* roomba, const uint8_t* cmd, int len );

// Move Roomba with low-level DRIVE command
void roomba_drive( Roomba* roomba, int velocity, int radius );

// stop the Roomba 
void roomba_stop( Roomba* roomba );

// Move Roomba forward at current velocity
void roomba_forward( Roomba* roomba );
void roomba_forward_at( Roomba* roomba, int velocity );

// Move Roomba backward at current velocity
void roomba_backward( Roomba* roomba );
void roomba_backward_at( Roomba* roomba, int velocity );

// Spin Roomba left at current velocity
void roomba_spinleft( Roomba* roomba );
void roomba_spinleft_at( Roomba* roomba, int velocity );

// Spin Roomba right at current velocity
void roomba_spinright( Roomba* roomba );
void roomba_spinright_at( Roomba* roomba, int velocity );

// Set current velocity for higher-level movement commands
void roomba_set_velocity( Roomba* roomba, int velocity );

// Get current velocity for higher-level movement commands
int roomba_get_velocity( Roomba* roomba );

// play a musical note
void roomba_play_note( Roomba* roomba, uint8_t note, uint8_t duration );

// Turns on/off the non-drive motors (main brush, vacuum, sidebrush).
void roomba_set_motors( Roomba* roomba, uint8_t mainbrush, uint8_t vacuum, uint8_t sidebrush);

// Turns on/off the various LEDs.
void roomba_set_leds( Roomba* roomba, uint8_t status_green, uint8_t status_red,
                      uint8_t spot, uint8_t clean, uint8_t max, uint8_t dirt, 
                      uint8_t power_color, uint8_t power_intensity );

// Turn all vacuum motors on or off according to state
void roomba_vacuum( Roomba* roomba, uint8_t state );

// Get the sensor data from the Roomba
// returns -1 on failure
int roomba_read_sensors( Roomba* roomba );

// Get the raw encoder counts
int roomba_read_encoders( Roomba* roomba );

// print existing sensor data nicely
void roomba_print_sensors( Roomba* roomba );

// print existing sensor data as string of hex chars
void roomba_print_raw_sensors( Roomba* roomba );

// utility function
void roomba_delay( int millisecs );
#define roomba_wait roomba_delay

// read requested size of input from Roomba into buffer
// returns # of bytes read, 0 if timeout, -1 if select error, -2 if wrong data size read
int input_timeout (int fd, uint8_t *buf, int read_size);

// some simple macros of bit manipulations
#define bump_right(b)           ((b & 0x01)!=0)
#define bump_left(b)            ((b & 0x02)!=0)
#define wheeldrop_right(b)      ((b & 0x04)!=0)
#define wheeldrop_left(b)       ((b & 0x08)!=0)
#define wheeldrop_caster(b)     ((b & 0x10)!=0)

#define motorover_sidebrush(b)  ((b & 0x01)!=0) 
#define motorover_vacuum(b)     ((b & 0x02)!=0) 
#define motorover_mainbrush(b)  ((b & 0x04)!=0) 
#define motorover_driveright(b) ((b & 0x08)!=0) 
#define motorover_driveleft(b)  ((b & 0x10)!=0) 

// Arduino declarations
unsigned long millis(void);
unsigned long micros(void);
void delay(unsigned long);
void init();

#ifdef __cplusplus
}
#endif

#endif	// ROOMBALIB_H_
