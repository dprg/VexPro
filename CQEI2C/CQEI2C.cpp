/*! file CQEI2C.cpp
 *
 *  Created on: Oct 12, 2012
 *      Author: bouchier
 *
 * \brief Provide I2C bus API
 * Supports all mandatory features of the I2C Bus Spec Rev 03:
 * Standard-mode (100KHz)
 * Single master
 * 7-bit addressing
 * Clock stretching
 * No buffering (bytes are sent and received "live")
 *
 */

#include "9302hw.h"
#include "qepower.h"
#include "qegpioint.h"
#include "CQEI2C.h"

CMemMap m_timers(0x80810000,0x100);
CQEGpioInt *cqei2cGpio;

CQEI2C::CQEI2C() {
    C9302Hardware &m_p9302hw = C9302Hardware::GetRef();
    m_i2c_reg = m_p9302hw.m_fpga.Ushort(0x480);		// get a pointer to the I2C register
    cqei2cGpio = CQEGpioInt::GetPtr();					// get ptr to the GPIO object
}

CQEI2C::~CQEI2C() {
}

/**
 t4usleep is an internal library function that sets uses the Timer4 in the
 EP9302 CPU to implement a microsecond-resolution busy-wait loop.  Timer4
 is actually a 40-bit timer, but we only look at the bottom 32-bits, which
 is more than enough for an hour.

 Note: The timer runs at 983.04KHz, so the actual wait time will be no less
 than 2% longer than requested.

 TODO: This probably belongs elsewhere - it is generally useful and not
 specifically related to I2C.
*/

inline void CQEI2C::t4sleep(unsigned long  ticks)
{
	unsigned long start = *m_timers.Uint(0x60);
	while ((*m_timers.Uint(0x60) - start) < ticks) {
		;	// We are counting on modulo-arithmetic ignoring underflows:
	}

}

// I2CSetRegister is an internal library function that sets the FPGA's
// I2C control register.  It is also responsible for implementing
// clock stretching (where the slave can delay a bus cycle), and
// it will also delay the number of microseconds specified by the
// second parameter (default is 5us, which is appropriate for 100KHz
// standard-speed operation).
void CQEI2C::I2CSetRegister(unsigned short reg, const unsigned int delay=5 USEC)
{
	*m_i2c_reg = reg | I2C_DDR;     // SDA is open-drain, so leave as an output
	if (reg & I2C_SCL) {			// Trying to bring SCL high
		while ( ! (*m_i2c_reg & I2C_SCL) ) {
			*m_i2c_reg = reg | I2C_DDR;
			cqei2cGpio->SetDataBit(0);
		}
		cqei2cGpio->ResetDataBit(0);
	}
	t4sleep(delay);
}


// I2CBit is an internal library function that sends a bit,
// and is also used to read one bit.
inline bool CQEI2C::I2CBit(bool bit)
{
	I2CSetRegister( bit, 1 USEC );	// SDA is high, SCL stays low
	I2CSetRegister( bit | I2C_SCL );	// SDA stays high, SCL goes high
	bool rd = I2C_SDA & *m_i2c_reg;		// Optional read
	I2CSetRegister( bit );				// SDA stays high, SCL returns low
	return rd;
}


//! Write 1 byte to the I2C bus
/*!
 * \param value Byte to write
 * \return true if receiver acked byte, false otherwise
 */
inline bool CQEI2C::I2CWriteByte(unsigned char value)
{
	register unsigned char bit = 0x80;
	bool ack;

	while (bit) {
		I2CBit(value&bit?I2C_SDA:0);
		bit >>= 1;
	}
	ack = !I2CBit(1);
#ifdef DEBUG
	printf(">%02x%c", value, ack?'+':'-');
#endif
	return ack;	// read ack bit
}


//! Write 2 bytes to the I2C bus
/*!
 * \param value Bytes to write.
 * \param order True to write high-byte first, false to write low byte first
 * \return True if successful, false if I2CWriteByte returned a error
 */
bool CQEI2C::I2CWriteWord(unsigned short value, bool order)
{
	if (!I2CWriteByte( (unsigned short)(value) >> (order?8:0) ))
		return false;
	if (!I2CWriteByte( (unsigned short)(value) >> (order?0:8) ))
		return false;
	return true;
}


//! Write 4 bytes to the I2C bus
/*!
 * \param value Bytes to write.
 * \param order True to write high-bytes first, false to write low bytes first
 * \return True if successful, false if I2CWriteByte returned a error
 */
bool CQEI2C::I2CWriteLong(unsigned long value, bool order)
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


//! Read 1 byte from the I2C bus
/*!
 * \param ack Defaults to I2C_READ and must be set to I2C_DONE when reading the last byte
 */
inline unsigned char CQEI2C::I2CReadByte(bool ack)
{
	register unsigned char bit = 0x80, byte = 0x00;

	while (bit) {
		if (I2CBit(I2C_SDA))
			byte |= bit;
		bit >>= 1;
	}
    I2CBit(ack?0:1);
#ifdef DEBUG
	printf("<%02x%c", byte, ack?'+':'-');
#endif
	return byte;
}


//! Read 2 bytes from the I2C bus
/*!
 * \param order True returns first received byte in high byte, false in low byte
 * \param ack Defaults to I2C_READ and must be set to I2C_DONE when reading the last byte
 */
unsigned short CQEI2C::I2CReadWord(bool order, bool ack)
{
	unsigned short value;
	value  = (unsigned short)(I2CReadByte(I2C_READ)) << (order?8:0);
	value |= (unsigned short)(I2CReadByte(ack))      << (order?0:8);
	return value;
}


//! Read 4 bytes from the I2C bus
/*!
 * \param order True returns first received byte in high byte, false in low byte
 * \param ack Defaults to I2C_READ and must be set to I2C_DONE when reading the last byte
 */
unsigned long CQEI2C::I2CReadLong(bool order, bool ack)
{
	unsigned long value;
	value  = (unsigned long)(I2CReadByte(I2C_READ)) << (order?24:0);
	value |= (unsigned long)(I2CReadByte(I2C_READ)) << (order?16:8);
	value |= (unsigned long)(I2CReadByte(I2C_READ)) << (order?8:16);
	value |= (unsigned long)(I2CReadByte(ack))      << (order?0:24);
	return value;
}


//! Start or re-start an I2C packet.
/*!
 * \param addr 7-bit target device address is provided by "addr" parameter.
 * \param read set to I2C_READ to make this a read operation, I2C_WRITE otherwise
 * \return True if successful, false if not
 */
bool CQEI2C::I2CStart(unsigned short addr, bool read)
{
	if (addr > 0x07f)
		return false;					// Too large for 7b addressing

    // Ensure the bus is idle
	I2CSetRegister( I2C_SCL | I2C_SDA ); // SDA is input, SCL is high
	while ( (*m_i2c_reg & (I2C_SDA | I2C_SCL)) != (I2C_SDA | I2C_SCL) ) {
		printf("Waiting for I2C bus idle\n"); // Empty Wait Loop
	}

	I2CSetRegister( I2C_SCL ); 			// SDA goes low, SCL stays high
	I2CSetRegister( 0 );				// SDA stays low, SCL goes low

#ifdef DEBUG
	printf("[S");
#endif
    return I2CWriteByte(addr<<1 | read);
}


//! Call once at the end of each I2C "packet" to leave I2C Bus in idle state
void CQEI2C::I2CStop()
{
	I2CSetRegister( 0 );					// SDA is low, SCL is low
	I2CSetRegister( I2C_SCL ); 				// SDA stays low, SCL goes high
	I2CSetRegister( I2C_SCL | I2C_SDA );	// SDA goes high, SCL stays high
#ifdef DEBUG
	printf("P]");
#endif
}


//! Initialize the I2C hardware and prepare the bus for I/O.
/*!
 * Call exactly once before calling any other I2C library functions.
 */
void CQEI2C::I2CInit()
{
    C9302Hardware &m_p9302hw = C9302Hardware::GetRef();
    cqei2cGpio = CQEGpioInt::GetPtr();
    cqei2cGpio->SetDataDirection(0x0007);
    cqei2cGpio->SetData(0x0000);

    int i=10;

    if (m_p9302hw.GetBitstreamMajorVersion()!=0xa0)
        throw std::runtime_error("wrong FPGA bitstream version");

    m_i2c_reg = m_p9302hw.m_fpga.Ushort(0x480);		// get a pointer to the I2C register

	I2CSetRegister( 0 );          				  	// Minimize Vcc leakage via SDA & SCL pullups
    *m_p9302hw.PortHData() &= ~0x0020;				// Turn off 5V supply to the I/O Ports
    t4sleep(1 SEC);
    *m_p9302hw.PortHData() |= 0x0020;				// Turn on 5V supply to the I/O Ports
    I2CSetRegister( I2C_SDA, 100 USEC );            // SDA high
    I2CSetRegister( I2C_SCL | I2C_SDA );            // SDA and SCL are high
	t4sleep(1 SEC);									// wait second before doing anything on the bus

	while (i--) {	// pump the SCL line so all slaves are looking for Start
		I2CSetRegister( I2C_SDA, 10 );				// SDA is input, SCL is high
		I2CSetRegister( I2C_SCL | I2C_SDA, 10 );	// SDA is input, SCL is high
	}

	t4sleep(100 MSEC);								// wait another 100ms why not
}


//! Handy function scans the I2C bus in the specified range
/*!
 * Prints a line for each device discovered if the quiet parameter is not true.
 * \param min The 7-bit address to start searching at (8-bit addres divided by 2)
 * \param max The 7-bit address to stop searching at (8-bit addres divided by 2)
 * \param quiet False (default) prints results of I2C bus scan, true supresses printing
 * \return Number of devices discovered
 */
unsigned short CQEI2C::I2CBusScan(unsigned short min, unsigned short max, bool quiet)
{
	unsigned short addr, count=0;

	if (!quiet)
		printf("Scanning I2C Bus range 0x%02x..0x%02x\n", min, max);
	for (addr=min; addr<=max; addr++) {
		if (I2CStart(addr, I2C_WRITE)) {
			if (!quiet)
				printf("\tFound device at 0x%02x (%d)\n", addr, addr);
			count++;
		}
		I2CStop();
		t4sleep(100 USEC); // Wait 100us between bus probes
	}
	if (!quiet)
		printf("Found %d devices\n", count);
	return count;
}

