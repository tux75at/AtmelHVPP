# include <avr/io.h>
//# include <avr/interrupt.h>
//# include <stdlib.h>

#ifndef TWI_H_
#define TWI_H_

# define I2C_PRESCALER 0
// 100 kHz normal mode
# define I2C_SPEED 100000L
// 400 kHz fast mode
//# define I2C_SPEED 400000L
# define I2C_BITRATE (((F_CPU / I2C_SPEED) - 16UL) / 2)


// i2c initialisation
uint8_t twi_init (void);

// i2c write
uint8_t twi_write (uint8_t twi_addr, uint8_t reg_addr, uint8_t length, uint8_t* data);
// set up write command and wait till i2c_state is idle
//uint8_t twi_write (uint8_t twi_addr, uint8_t data);
// just write a value to the address (e.g. for Shift Register)

// i2c read
uint8_t twi_read (uint8_t twi_addr, uint8_t reg_addr, uint8_t length, uint8_t* data);
// set up write command and wait till i2c_state is idle
// read data is stored in memory given with char* data


// private functions header
// these functions should not be used outside this library
void twi_start (uint8_t address);
void twi_rep_start (uint8_t address);
void twi_stop (void);
uint8_t twi_read_data (uint8_t ack);
void twi_write_data (uint8_t data);
void twi_wait_for_transmission_end(void);

#endif
