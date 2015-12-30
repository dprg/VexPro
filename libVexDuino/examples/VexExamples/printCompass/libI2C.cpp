//============================================================================
// Name        : libI2C.h
// Version     : 0.1  (2011.11.29)
// Description : I2C library for VEXpro
//               Supports all mandatory features of the I2C Bus Spec Rev 03:
//                  Standard-mode (100KHz)
//                  Single master
//                  7-bit addressing
//                  Clock stretching
//                  No buffering (bytes are sent and received "live")
// Planned     : The following optional features are planned:
//                  10-bit addressing
//                  General Call
//                  Software Reset
//                  START Byte
// Not Planned : These features would require reworking the FPGA image:
//                  Multi-master
//                  Slave
//                  Arbitration
//                  Synchronization
//                  Fast-mode (400KHz)
//                  Fast-mode Plus (1MHz)
//============================================================================

#include "9302hw.h"
#include "libI2C.h"


// Pointer to the 16b I2C register in the VEXpro FPGA
volatile unsigned short *m_i2c_reg;

// Bit definitions for the I2C register
#define I2C_DDR		(0x04)
#define I2C_SCL		(0x02)
#define I2C_SDA		(0x01)


// Memory map for EP9302 Timer Registers
CMemMap m_timers(0x80810000,0x100);

// t4usleep is an internal library function that sets uses the Timer4 in the
// EP9302 CPU to implement a microsecond-resolution busy-wait loop.  Timer4
// is actually a 40-bit timer, but we only look at the bottom 32-bits, which
// is more than enough for an hour.
//
// Note: The timer runs at 983.04KHz, so the actual wait time will be no less
// than 2% longer than requested.
//
// TODO: This probably belongs elswhere - it is generally useful and not
// specifically related to I2C.
inline void t4usleep(unsigned long usec)
{
	unsigned long start;

	start = *m_timers.Uint(0x60);	// remember entry time
	while ((*m_timers.Uint(0x60) - start) < usec) {
		;	// We are counting on modulo-arithmetic ignoring underflows:
	}
}


// I2CSetRegister is an internal library function that sets the FPGA's
// I2C control register.  It is also responsible for implementing
// clock stretching (where the slave can delay a bus cycle), and
// it will also delay the number of microseconds specified by the
// second parameter (default is 5us, which is appropriate for 100KHz
// standard-speed operation).
inline void I2CSetRegister(unsigned short reg, const unsigned int delay=5/*usec*/)
{
	*m_i2c_reg = reg | I2C_DDR;     // SDA is open-drain, so leave as an output
	while ( I2C_SCL & reg & ~(*m_i2c_reg) ) {	
		; // empty loop while SCL remains low (clock stretching)
	}
	t4usleep(delay);
}


// I2CBit is an internal library function that sends a bit,
// and is also used to read one bit.
inline bool I2CBit(bool bit)
{
	I2CSetRegister( bit, 1/*usec*/ );	// SDA is high, SCL stays low
	I2CSetRegister( bit | I2C_SCL );	// SDA stays high, SCL goes high
	bool rd = I2C_SDA & *m_i2c_reg;		// Optional read
	I2CSetRegister( bit );				// SDA stays high, SCL returns low
	return rd;
}


// Write 1 byte to the I2C bus
inline bool I2CWriteByte(unsigned char value)
{
	register unsigned char bit = 0x80;

	while (bit) {
		I2CBit(value&bit?I2C_SDA:0);
		bit >>= 1;
	}
	return !I2CBit(1);	// read ack bit
}


// Write 2 bytes to the I2C bus
bool I2CWriteWord(unsigned short value, bool order)
{
	if (!I2CWriteByte( (unsigned short)(value) >> (order?8:0) ))
		return false;
	if (!I2CWriteByte( (unsigned short)(value) >> (order?0:8) ))
		return false;
	return true;
}


// Write 4 bytes to the I2C bus
bool I2CWriteLong(unsigned long value, bool order)
{
	if (!I2CWriteByte( (unsigned long)(value) >> (order?24:0) ))
		return false;
	if (!I2CWriteByte( (unsigned long)(value) >> (order?16:8) ))
		return false;
	if (!I2CWriteByte( (unsigned long)(value) >> (order?8:16) ))
		return false;
	if (!I2CWriteByte( (unsigned long)(value) >> (order?0:24) ))
		return false;
	return true;
}


// Read 1 byte from the I2C bus
inline unsigned char I2CReadByte(bool ack)
{
	register unsigned char bit = 0x80, byte = 0x00;

	while (bit) {
		if (I2CBit(I2C_SDA))
			byte |= bit;
		bit >>= 1;
	}
    I2CBit(ack?0:1);
	return byte;
}


// Read 2 bytes from the I2C bus
unsigned short I2CReadWord(bool order, bool ack)
{
	unsigned short value;
	value  = (unsigned short)(I2CReadByte(I2C_READ)) << (order?8:0);
	value |= (unsigned short)(I2CReadByte(ack))      << (order?0:8);
	return value;
}


// Read 4 bytes from the I2C bus
unsigned long I2CReadLong(bool order, bool ack)
{
	unsigned long value;
	value  = (unsigned long)(I2CReadByte(I2C_READ)) << (order?24:0);
	value |= (unsigned long)(I2CReadByte(I2C_READ)) << (order?16:8);
	value |= (unsigned long)(I2CReadByte(I2C_READ)) << (order?8:16);
	value |= (unsigned long)(I2CReadByte(ack))      << (order?0:24);
	return value;
}


// Start or re-start an I2C packet.
// 7-bit target device address is provided by "addr" parameter.
// Data direction is specified by the "read" parameter.
// Returns true if address was acknowleged and false otherwise.
bool I2CStart(unsigned short addr, bool read)
{
	if (addr > 0x07f)
		return false;					// Too large for 7b addressing

    // Ensure the bus is idle
	I2CSetRegister( I2C_SCL | I2C_SDA ); // SDA is input, SCL is high
	while ( (*m_i2c_reg & (I2C_SDA | I2C_SCL)) != (I2C_SDA | I2C_SCL) ) {
		; // Empty Wait Loop
	}
    
	I2CSetRegister( I2C_SCL ); 			// SDA goes low, SCL stays high
	I2CSetRegister( 0 );				// SDA stays low, SCL goes low

    return I2CWriteByte(addr<<1 | read);
}


// Call once at the end of each I2C "packet".
// Leaves I2C Bus in idle state
void I2CStop()
{
	I2CSetRegister( 0 );					// SDA is low, SCL is low
	I2CSetRegister( I2C_SCL ); 				// SDA stays low, SCL goes high
	I2CSetRegister( I2C_SCL | I2C_SDA );	// SDA goes high, SCL stays high
}


// Initialize the I2C hardware and prepare the bus for I/O.
// Call exactly once before calling any other I2C library functions.
void I2CInit()
{
    C9302Hardware *m_p9302hw = C9302Hardware::GetPtr();

    if (m_p9302hw->GetBitstreamMajorVersion()!=0xa0)
        throw std::runtime_error("wrong FPGA bitstream version");        
    m_i2c_reg = m_p9302hw->m_fpga.Ushort(0x480);    // get a pointer to the I2C register
	I2CSetRegister( I2C_SCL | I2C_SDA );            // SDA is input, SCL is high
}


// Handy function scans the I2C bus in the specified range
// and prints a line for each device discovered.
// Returns the total number of devices discovered.
unsigned short I2CBusScan(unsigned short min, unsigned short max)
{
	unsigned short addr, count=0;

	printf("Scanning I2C Bus range 0x%02x..0x%02x\n", min, max);
	for (addr=min; addr<=max; addr++) {
		if (I2CStart(addr, I2C_WRITE)) {
			printf("\tFound device at 0x%02x (%d)\n", addr, addr);
			count++;
		}
		I2CStop();
		t4usleep(100); // Wait 100us between bus probes
	}
	printf("Found %d devices\n", count);
	return count;
}
