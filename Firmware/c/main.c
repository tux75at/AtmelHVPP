#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "twi.h"
#include "twi_display.h"


#define C_VERSION_MAJOR			"1"
#define C_VERSION_MINOR			"0"

// define the outputs
// data high nibble and low nibble on different ports 
// due to sca and scl beeing on the only full port
// data high nibble is PB[7:4]
// data low nibble is PD[7:4]
#define DATA_HIGH_PORT			PORTB
#define DATA_LOW_PORT			PORTF
#define DATA_HIGH_DDR			DDRB
#define DATA_LOW_DDR			DDRF
#define DATA_HIGH_PIN           PINB
#define DATA_LOW_PIN            PINF
#define DATA_BIT_7				PB7	// IO11
#define DATA_BIT_6				PB6	// IO10
#define DATA_BIT_5				PB5	// IO9
#define DATA_BIT_4				PB4	// IO8
#define DATA_BIT_3				PF7	// A0
#define DATA_BIT_2				PF6	// A1
#define DATA_BIT_1				PF5	// A2
#define DATA_BIT_0				PF4	// A3

#define BS_PORT					PORTD
#define BS_DDR					DDRD
#define BS_BIT_1				PD2	// IO0
#define BS_BIT_2				PD3	// IO1

#define XA_PORT_0				PORTF
#define XA_PORT_1				PORTC
#define XA_DDR_0				DDRF
#define XA_DDR_1				DDRC
#define XA_BIT_0				PF0	// A5
#define XA_BIT_1				PC6	// IO5

#define OE_PORT					PORTD
#define OE_DDR					DDRD
#define OE						PD7 // IO6

#define	WR_PORT					PORTE
#define	WR_DDR					DDRE
#define	WR						PE6 // IO7

#define	RDY_PORT				PORTF
#define	RDY_PIN 				PINF
#define	RDY_DDR					DDRF
#define	RDY						PF1 // A4

/* Allowas ground, no pin available
#define	PAGEL_PORT				PORTF
#define	PAGEL_DDR				DDRF
#define	PAGEL					PF0 // A5
*/
#define XTAL_PORT				PORTC
#define XTAL_DDR				DDRC
#define XTAL					PC7 // IO13
/*
#define GO_PORT	    			PORTF
#define GO_PIN                  PINF
#define GO_DDR  				DDRF
#define GO  					PF0 // A5
*/
#define VCC_PORT				PORTD
#define VCC_DDR	    			DDRD
#define VCC 					PD4 // IO4

#define HV_PORT	    			PORTD
#define HV_DDR	    			DDRD
#define HV  					PD6 // IO12


uint8_t signature_byte_0;
uint8_t signature_byte_1;
uint8_t signature_byte_2;

uint8_t fuse_byte_low;
uint8_t fuse_byte_high;
uint8_t fuse_byte_ext;
uint8_t fuse_byte_lock;

uint8_t focus_on (void);
uint8_t focus_off (void);
uint8_t exposure_start (void);
uint8_t exposure_end (void);
uint8_t move (void);

uint8_t hvpp_init(void);
uint8_t hvpp_command(uint8_t command);
uint8_t hvpp_load_data_low_byte(uint8_t data);
uint8_t hvpp_address_low_byte(uint8_t address);
uint8_t hvpp_clock(void);
uint8_t hvpp_read_signature(void);
uint8_t hvpp_read_fuses(void);
uint8_t hvpp_prog_fuses(void);
uint8_t hvpp_set_data_direction(uint8_t direction);
uint8_t hvpp_read_byte(void);
uint8_t hvpp_set_data(uint8_t value);

int main(void) {

	// Port initialization

	
	twi_init();
	twi_display_init();


	// display welcome message
	twi_display_clear();
	twi_display_write_text(0x00, 0x00, 0x0D, "Welcome!    v" );
	twi_display_write_text(0x00, 0x0D, 0x01, C_VERSION_MAJOR);
	twi_display_write_text(0x00, 0x0E, 0x01, ".");
	twi_display_write_text(0x00, 0x0F, 0x01, C_VERSION_MINOR);
	twi_display_write_text(0x01, 0x00, 0x10, "    H V P P     " );

	// wait for three seconds

	uint8_t rv = 0;
    // time to insert the chip
    // first turn off the VCC and HV to be sure
    HV_DDR |= 1<<HV;
    HV_PORT &= ~(1<<HV);
    VCC_DDR |= 1<<VCC;
    // need to set VCC port in order to turn off power supply
    VCC_PORT |= (1<<VCC);
   // GO_DDR &= ~(1<<GO);

	_delay_ms(3000);
    while (1) {
    	twi_display_clear();
    	twi_display_write_text(0x00, 0x00, 0x0B, "insert chip" );


        _delay_ms(5000);
        _delay_ms(5000);
    	rv = hvpp_init();

	
    	// read id, data is stored in global variable
	    rv = hvpp_read_signature();

    	twi_display_clear();
    	twi_display_write_text(0x00, 0x00, 0x07, "Sig: 0x" );
        twi_display_write_hex(0x00, 0x07, signature_byte_0);
        twi_display_write_hex(0x00, 0x09, signature_byte_1);
        twi_display_write_hex(0x00, 0x0b, signature_byte_2);

    	rv = hvpp_read_fuses();
	    twi_display_write_text(0x01, 0x00, 0x02, "h:" );
        twi_display_write_hex(0x01, 0x02, fuse_byte_high);
	    twi_display_write_text(0x01, 0x04, 0x02, "l:" );
        twi_display_write_hex(0x01, 0x06, fuse_byte_low);
	    twi_display_write_text(0x01, 0x08, 0x02, "e:" );
        twi_display_write_hex(0x01, 0x0A, fuse_byte_ext);
    	twi_display_write_text(0x01, 0x0C, 0x02, "l:" );
        twi_display_write_hex(0x01, 0x0E, fuse_byte_lock);

        // programming fuses
        _delay_ms(5000);
        rv = hvpp_prog_fuses();
        _delay_ms(5000);

        hvpp_set_data_direction(0x00);
        OE_DDR &= ~(1<<OE);
        WR_DDR &= ~(1<<WR);
        BS_DDR &= ~(1<<BS_BIT_1) & (1<<BS_BIT_2);
        XTAL_DDR &= ~(1<<XTAL);
        XA_DDR_1 &= ~(1<<XA_BIT_1);
        XA_DDR_0 &= ~(1<<XA_BIT_0);
        // turn off voltages
        _delay_ms(10);
        HV_PORT &= ~(1<<HV);
        _delay_ms(10);
        VCC_PORT |= (1<<VCC);
    }

	return(0);
}

uint8_t hvpp_init(void) {
	
	// wait for one seconds
	_delay_ms(1000);

	// set pins to enter programming mode
	// PAGEL -> GND, XA[0:1] -> GND, BS1 -> GND
	// PAGEL and XA on PF, BS on PD
	// PAGEL pin to 0
    // PAGEL is constant ground
	// XA pins to 0
    // XA_BIT_0 is constant ground
    XA_PORT_1    &= ~(1<<XA_BIT_1);
	XA_DDR_1     |= (1<<XA_BIT_1);
    XA_PORT_0    &= ~(1<<XA_BIT_0);
	XA_DDR_0     |= (1<<XA_BIT_0);
	// BS1 to 0
	BS_PORT    &= ~(1<<BS_BIT_1);
	// BS1 to output
	BS_DDR     |= 1<<BS_BIT_1;
	// set also write to low just in case

    // correct sequence to enter HVPP mode
    // RESET PIN and VCC at 0V
    // apply 4.5-5.5V at VCC (ensure that VCC reaches at least 1.8V within the next 20µs
    // wait 20-0 µs
    // apply 11.5 to 12.5V at RESET
    // keep the program enable pins unchanged for at least 10 µs after HV has been applied to ensure
    // that the signals have been latched
    // wait 300 µs before giving any parallel programming commands

    _delay_ms(500);

    // power up
    VCC_PORT &= ~(1<<VCC);
    _delay_us(30);
    // 12V at RESET
    HV_PORT |= 1<<HV;
    _delay_us(500);
    // giving a few XTAL cycles
	XTAL_DDR  |= 1<<XTAL;
	XTAL_PORT &= ~(1<<XTAL);
	// toggle XTAL1 at least 6 times

    // set all direction bits
	BS_DDR     |= 1<<BS_BIT_1;
	
	// set OE to output and high
	OE_PORT    |= 1<<OE;
	OE_DDR     |= 1<<OE;
    // set WR to output and high
    WR_PORT |= 1<<WR;
    WR_DDR  |= 1<<WR;
	// set data direction to output
	DATA_HIGH_PORT |= (1<<DATA_BIT_7) | (1<<DATA_BIT_6) | (1<<DATA_BIT_5) | (DATA_BIT_4);
	DATA_LOW_PORT  |= (1<<DATA_BIT_3) | (1<<DATA_BIT_2) | (1<<DATA_BIT_1) | (DATA_BIT_0);
	DATA_HIGH_DDR  |= (1<<DATA_BIT_7) | (1<<DATA_BIT_6) | (1<<DATA_BIT_5) | (DATA_BIT_4);
	DATA_LOW_DDR   |= (1<<DATA_BIT_3) | (1<<DATA_BIT_2) | (1<<DATA_BIT_1) | (DATA_BIT_0);
	
	// set ready to input
	RDY_DDR &= ~(1<<RDY);
	
	// init done
	return (0);
}

uint8_t hvpp_command(uint8_t command) {
/*
	load command:
	1) set XA[1:0] to "10" -- this enables command loading
	2) set BS1 to 0
	3) set data to command
	4) give 1 XTAL1 pulse (positive)
*/
	XA_PORT_1 |= 1<<XA_BIT_1;    // set bit
    // XA_BIT_0 is removed
	XA_PORT_0 &= ~(1<<XA_BIT_0); // clear bit
	BS_PORT &= ~(1<<BS_BIT_1);
	// set data port to command value
    hvpp_set_data_direction(0xFF);
	hvpp_set_data(command);
	hvpp_clock();
	return (0);
}

uint8_t hvpp_load_data_low_byte(uint8_t data) {
/*
	load data low byte:
	1) set XA[1:0] to "01" -- this enables data loading
	2) set BS1 to 0
	3) set data to data
	4) give 1 XTAL1 pulse (positive)
*/
	XA_PORT_1 &= ~(1<<XA_BIT_1);    // clear bit
    // XA_BIT_0 is removed
	XA_PORT_0 |= (1<<XA_BIT_0); // set bit
	BS_PORT &= ~(1<<BS_BIT_1);
	// set data port to command value
    hvpp_set_data_direction(0xFF);
	hvpp_set_data(data);
	hvpp_clock();
	return (0);
    
}

uint8_t hvpp_address_low_byte(uint8_t address) {
/*
	load address:
	1) set XA[1:0] to "00" -- this enables address low byte loading
	2) set BS1 to 0
	3) set data to address low byte
	4) give 1 XTAL1 pulse (positive)
*/
	XA_PORT_1 &= ~(1<<XA_BIT_1); // clear bit
    // XA_BIT_0 is removed
	XA_PORT_0 &= ~(1<<XA_BIT_0); // clear bit
	BS_PORT &= ~(1<<BS_BIT_1);
	// set data port to command value
    hvpp_set_data_direction(0xFF);
	hvpp_set_data(address);
	hvpp_clock();
	return (0);
}

uint8_t hvpp_read_signature(void) {
	// command read signature
	hvpp_command(0x08);
	// load address for first signature byte
    hvpp_set_data_direction(0xFF);
	hvpp_address_low_byte(0x00);
	// set data direction port of data pins to input
	hvpp_set_data_direction(0x00);
	DATA_HIGH_DDR  &= ~(1<<DATA_BIT_7) & ~(1<<DATA_BIT_6) & ~(1<<DATA_BIT_5) & ~(DATA_BIT_4);
	DATA_LOW_DDR   &= ~(1<<DATA_BIT_3) & ~(1<<DATA_BIT_2) & ~(1<<DATA_BIT_1) & ~(DATA_BIT_0);

	//set OE low
	OE_PORT &= ~(1<<OE);
	//set BS1 low
	BS_PORT &= ~(1<<BS_BIT_1);
	// read data byte
    _delay_ms(10);
	signature_byte_0 = hvpp_read_byte();
	// set OE high
	OE_PORT |= 1<<OE;
	
	// load address for second signature byte
    hvpp_set_data_direction(0xFF);
	hvpp_address_low_byte(0x01);
    hvpp_set_data_direction(0x00);
	//set OE low
	OE_PORT &= ~(1<<OE);
	//set BS1 low
	BS_PORT &= ~(1<<BS_BIT_1);
	// read data byte
    _delay_ms(10);
	signature_byte_1 = hvpp_read_byte();
	// set OE high
	OE_PORT |= 1<<OE;

	// load address for third signature byte
    hvpp_set_data_direction(0xFF);
	hvpp_address_low_byte(0x02);
    hvpp_set_data_direction(0x00);
	//set OE low
	OE_PORT &= ~(1<<OE);
	//set BS1 low
	BS_PORT &= ~(1<<BS_BIT_1);
	// read data byte
    _delay_ms(10);
	signature_byte_2 = hvpp_read_byte();
	// set OE high
	OE_PORT |= 1<<OE;

	return (0);
}

uint8_t hvpp_read_fuses(void) {
	// command read fuses
	hvpp_command(0x04);
    hvpp_set_data_direction(0x00);

	//set OE low
	OE_PORT &= ~(1<<OE);

	//set BS to read fuse byte low
	BS_PORT &= ~(1<<BS_BIT_1);
	BS_PORT &= ~(1<<BS_BIT_2);
	// read data byte
    _delay_ms(10);
	fuse_byte_low = hvpp_read_byte();

	//set BS to read fuse byte high
	BS_PORT |= (1<<BS_BIT_1);
	BS_PORT |= (1<<BS_BIT_2);
	// read data byte
    _delay_ms(10);
	fuse_byte_high = hvpp_read_byte();

	//set BS to read fuse byte ext
	BS_PORT &= ~(1<<BS_BIT_1);
	BS_PORT |= (1<<BS_BIT_2);
	// read data byte
    _delay_ms(10);
	fuse_byte_ext = hvpp_read_byte();

	//set BS to read fuse byte lock
	BS_PORT |= (1<<BS_BIT_1);
	BS_PORT &= ~(1<<BS_BIT_2);
	// read data byte
    _delay_ms(10);
	fuse_byte_lock = hvpp_read_byte();

	// set OE high
	OE_PORT |= 1<<OE;

	return (0);
}

uint8_t hvpp_prog_fuses(void) {
    // command programm fuse byte high
    hvpp_command(0x40);
    hvpp_load_data_low_byte(0x62);
    // set BS to program fuse byte low
	BS_PORT &= ~(1<<BS_BIT_1);
	BS_PORT &= ~(1<<BS_BIT_2);
    _delay_ms(10);
    WR_PORT &= ~(1<<WR);
    _delay_ms(10);
    WR_PORT |= (1<<WR);
    hvpp_load_data_low_byte(0xDF);
	//set BS to programm fuse byte high
	BS_PORT |= (1<<BS_BIT_1);
	BS_PORT &= ~(1<<BS_BIT_2);
    // give wr a low pulse
    _delay_ms(10);
    WR_PORT &= ~(1<<WR);
    _delay_ms(10);
    WR_PORT |= (1<<WR);
    hvpp_load_data_low_byte(0xFF);
	//set BS to programm fuse byte ext
	BS_PORT &= ~(1<<BS_BIT_1);
	BS_PORT |= (1<<BS_BIT_2);
    // give wr a low pulse
    _delay_ms(10);
    WR_PORT &= ~(1<<WR);
    _delay_ms(10);
    WR_PORT |= (1<<WR);
    _delay_ms(10);
    while (((RDY_PIN>>RDY) & 0x01) != 0x01);
    return(0);
}

uint8_t hvpp_set_data_direction(uint8_t direction) {
	// read direction register from high nibble and store it in tmp with a mask for the low nibble
	// high nibble will be set with new data, low nibble needs to stay as it is
	uint8_t tmp = DATA_HIGH_DDR & 0x0F;
	// mask the high nibble and put it in the high nibble of tmp
	tmp = (direction & 0xF0) | tmp;
	// write data high port back with new value
	DATA_HIGH_DDR = tmp;
	
	// one line method:
	// DATA_HIGH_DDR = (direction & 0xF0) | (DATA_HIGH_DDR & 0x0F);
	
	// do the same with the low nibble port and the low nibble data
	// the low nibble is also stored in the high nibble of the low nibble port
	tmp = DATA_LOW_DDR & 0x0F;
	tmp = ((direction & 0x0F)<<4) | tmp;
	DATA_LOW_DDR = tmp;
	
	// one line method:
	// DATA_LOW_DDR = ((direction & 0x0F)<<4) | (DATA_LOW_DDR & 0x0F);
	
	return (0);
}

uint8_t hvpp_read_byte(void) {
	uint8_t rv = (DATA_HIGH_PIN & 0xF0) | ((DATA_LOW_PIN & 0xF0)>>4);
	return (rv);
}

uint8_t hvpp_clock(void) {
	// delay in beginning to be sure, that all signals are stable
	// we have the time
	_delay_ms(10);
	// rising clock edge
	XTAL_PORT |= 1<<XTAL;
	// delay for high time
	_delay_ms(10);
	// falling clock edge
	XTAL_PORT &= ~(1<<XTAL);
	// delay for low time
	_delay_ms(10);
	return (0);
}

uint8_t hvpp_set_data(uint8_t value) {
	// read port from high nibble and store it in tmp with a mask for the low nibble
	// high nibble will be set with new data, low nibble needs to stay as it is
	uint8_t tmp = DATA_HIGH_PORT & 0x0F;
	// mask the high nibble and put it in the high nibble of tmp
	tmp = (value & 0xF0) | tmp;
	// write data high port back with new value
	DATA_HIGH_PORT = tmp;
	
	// one line method:
	// DATA_HIGH_PORT = (value & 0xF0) | (DATA_HIGH_PORT & 0x0F);
	
	// do the same with the low nibble port and the low nibble data
	// the low nibble is also stored in the high nibble of the low nibble port
	tmp = DATA_LOW_PORT & 0x0F;
	tmp = ((value & 0x0F)<<4) | tmp;
	DATA_LOW_PORT = tmp;
	
	// one line method:
	// DATA_LOW_PORT = ((value & 0x0F)<<4) | (DATA_LOW_PORT & 0x0F);
	
	return (0);
}

