/*
 * CQEI2C.h
 *
 *  Created on: Oct 12, 2012
 *      Author: bouchier
 */

#ifndef CQEI2C_H_
#define CQEI2C_H_

#define I2C_MSB_FIRST	(true)
#define I2C_LSB_FIRST	(false)

#define I2C_READ		(true)
#define I2C_WRITE		(false)
#define I2C_DONE		(false)

// Bit definitions for the I2C register
#define I2C_DDR		(0x04)
#define I2C_SCL		(0x02)
#define I2C_SDA		(0x01)

// Delay functions - these are temporary and will be removed before "1.0"
#define USEC	*1UL
#define MSEC	*983UL
#define SEC		*983040UL

/*! \class CQEI2C
 * \brief I2C bus operation methods
 *
 * This class provides low-level and higher-level methods for controlling transactions
 * on the I2C bus
 */
class CQEI2C {
public:
	CQEI2C();
	virtual ~CQEI2C();
	void I2CInit(void);			// Initialize the I2C pins
	bool I2CStart(unsigned short addr, bool read);			// Send the Start sequence
	void I2CStop(void);										// Send the Stop sequence
	bool I2CWriteByte(unsigned char value);					// write byte
	bool I2CWriteWord(unsigned short value, bool order);	// write short
	bool I2CWriteLong(unsigned long value, bool order);		// write long
	unsigned char	I2CReadByte(bool ack=I2C_READ);			// read byte
	unsigned short	I2CReadWord(bool order, bool ack=I2C_READ);	// read short
	unsigned long	I2CReadLong(bool order, bool ack=I2C_READ);	// read long
	unsigned short I2CBusScan(unsigned short min=0x08, unsigned short max=0x77, bool quiet=false);

private:
	// Pointer to the 16b I2C register in the VEXpro FPGA
	volatile unsigned short *m_i2c_reg;

	// Memory map for EP9302 Timer Registers

	void t4sleep(unsigned long ticks);
	void I2CSetRegister(unsigned short reg, const unsigned int delay);
	bool I2CBit(bool bit);

};

#endif /* CQEI2C_H_ */
