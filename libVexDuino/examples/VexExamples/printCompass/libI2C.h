//============================================================================
// Name        : libI2C.h
// Version     : 0.1  (2011.11.29)
// Description : I2C library for VEXpro
//               Supports all mandatory features of the I2C Bus Spec Rev 03:
//                  Standard-mode (100KHz)
//                  Single master
//                  7-bit addressing
//                  Clock stretching
//============================================================================

#ifndef _LIBI2C_H
#define _LIBI2C_H


#define I2C_MSB_FIRST	(true)
#define I2C_LSB_FIRST	(false)

#define I2C_READ		(true)
#define I2C_WRITE		(false)
#define I2C_DONE		(false)


// Initialize the I2C hardware and prepare the bus for I/O.
// Call exactly once before calling any other I2C functions.
void I2CInit(void);


// Call I2CStart to begin a packet, or any time you need to change
// the direction of I/O within a packet.  It is common to see multiple
// calls to I2CStart with one single call to I2CStop at the end of
// the packet.
//
// The first parameter is the 7-bit I2C address of the target device,
// which must be in the range of 0..127.  Note that some docs provide
// the "8 bit" address.  If you are having trouble selecting your
// device, try dividing the address by 2.
//
// The second parameter specifies the direction of the transfer.
// If you specify I2C_READ, then you should only call I2CRead*
// functions until after the next I2CStart or I2CStop call.
// If you specify I2C_WRITE, then you should only call I2CWrite*
// functions until after the next I2CStart or I2CStop call.
//
// This function will return true if the target device acknowledged
// the bus start, or false if otherwise.
bool I2CStart(unsigned short addr, bool read);


// The I2CWrite functions write 1, 2, or 4 bytes to the I2C bus.
// They should only be called after a successful call to I2CStart
// with the I2C_WRITE option specified.
// 
// The "value" parameter specifies the value to be sent.
//
// The "order" parameter is either I2C_MSB_FIRST or I2C_LSB_FIRST.
// This specifies the order that the bytes are sent on the bus.
// Bits are always sent most-significant-bit first.
//
// These functions return true if all bytes were acknowledged
// by the addressee, or return false otherwise.
bool I2CWriteByte(unsigned char value);
bool I2CWriteWord(unsigned short value, bool order);
bool I2CWriteLong(unsigned long value, bool order);


// The I2CRead functions read 1, 2, or 4 bytes from the I2C bus.
// They should only be called after a successful call to I2CStart
// with the I2C_READ option specified.
// 
// The "ack" parameter indicates if this is the last read operation
// before the next call to I2CStart.  Specifying I2C_READ (the
// default) indicates more reads will follow; specifying I2C_DONE
// indicates this is the last read.
//
// The "order" parameter is either I2C_MSB_FIRST or I2C_LSB_FIRST.
// This specifies the order that the bytes are sent on the bus.
// Bits are always sent most-significant-bit first.
//
// These functions return the value read from the bus.
unsigned char	I2CReadByte(bool ack=I2C_READ);
unsigned short	I2CReadWord(bool order, bool ack=I2C_READ);
unsigned long	I2CReadLong(bool order, bool ack=I2C_READ);


// Call once at the end of each I2C "packet".  Note that there may be
// multiple successive calls to I2CStart prior to a single call to I2CStop,
// but there should not be multiple successive calls to I2CStop.
void I2CStop(void);


// Handy function to scan the I2C bus over the specified range.
// Prints a line to stdout for each device and returns the total number
// of devices discovered.
unsigned short I2CBusScan(unsigned short min=0x08, unsigned short max=0x77);

#endif // _LIBI2C_H
