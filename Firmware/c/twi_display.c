#include "twi.h"
#include "twi_display.h"
#include <util/delay.h>

static uint8_t backlight = 0x08; //(1<<3);

uint8_t twi_display_init (void) {
	// activate backlight, set all IOs for dispaly to zero
	twi_write(twi_display_address, 0x08, 0x00, 0x00);
	// soft reset
	twi_display_reset();
	// set 4 bit mode
	twi_display_set_4bit_mode();
	// set display on/off, cursur on/off, blink on/off
	// command 0000 1 D C B
	twi_display_command(0x08 | (TWI_DISP_ON<<2) | (TWI_DISP_CURSOR_ON<<1) | (TWI_DISP_CURSOR_BLINK));
	// entry mode set
	// command 0000 0 1 I/D S
	twi_display_command(0x04 | (TWI_DISP_CURSOR_INC<<1) | (TWI_DISP_SHIFT));
	// write 06 with RS = 0 and RW = 0
	twi_display_clear();
	twi_display_cursor_home();
	twi_display_set_DDRAM_address(0x40);
	twi_display_cursor_home();
	// write "Hello World!"
	twi_display_write_text(0x00, 0x03, 0x0A, "Welcome to");
	twi_display_write_text(0x01, 0x00, 0x0F, "the real World!");
	// write H
	// write 48 with RS = 1 and RW = 0
	return (0);
}


uint8_t twi_display_reset (void) {
	// wait for display boot and then keep timing for display soft reset
	_delay_ms(45);
	twi_write(twi_display_address, 0x3C, 0x00, 0x00);
	twi_write(twi_display_address, 0x38, 0x00, 0x00);
	_delay_ms(5);
	twi_write(twi_display_address, 0x3C, 0x00, 0x00);
	twi_write(twi_display_address, 0x38, 0x00, 0x00);
	_delay_ms(1);
	twi_write(twi_display_address, 0x3C, 0x00, 0x00);
	twi_write(twi_display_address, 0x38, 0x00, 0x00);
	_delay_ms(5);
	return (0);
}


uint8_t twi_display_command (uint8_t command) {
	uint8_t command_a = (command & 0xF0);
	uint8_t command_b = (command << 4) & 0xF0;

	uint8_t enable = backlight ? 0x0C : 0x04;
	uint8_t disable = backlight ? 0x08 : 0x00;

	twi_write(twi_display_address, (command_a | enable), 0x00, 0x00);
	twi_write(twi_display_address, (command_a | disable), 0x00, 0x00);
	twi_write(twi_display_address, (command_b | enable), 0x00, 0x00);
	twi_write(twi_display_address, (command_b | disable), 0x00, 0x00);

	_delay_ms(1);
	return (0);
}

// TODO MAKRO
uint8_t twi_display_clear (void) {
	twi_display_command(0x01);
	return (0);
}

// TODO MAKRO
uint8_t twi_display_cursor_home (void) {
	twi_display_command(0x02);
	return (0);
}

// TODO MAKRO
uint8_t twi_display_set_DDRAM_address (uint8_t address) {
	twi_display_command(0x80 | (address & 0x7F));
	return (0);
}

uint8_t twi_display_set_cursor_position (uint8_t line, uint8_t position) {
	uint8_t address = (line == 0) ? (position & 0x0F) : 0x40 | (position & 0x0F);
	twi_display_set_DDRAM_address(address);
	return (0);
}

uint8_t twi_display_set_backlight (uint8_t on) {
	backlight = ((on&0x01)<<3);
	twi_write(twi_display_address, backlight, 0x00, 0x00);
	return (0);
}

uint8_t twi_display_set_4bit_mode (void) {
	// activate 4 bit mode
	// need to write DB7-DB4 0010, RS = 0, R/W = 0, E = 1 one pulse
	twi_write(twi_display_address, 0x2C, 0x00, 0x00);
	twi_write(twi_display_address, 0x28, 0x00, 0x00);
	_delay_ms(2);
	// 4 bit mode set, data still there, but enable is low
	// function set, 4 bit mode, 1 line display, 5x7 dot character
	// write 28, 4 bit mode method write 2 then write write 8
	// setting RS = 0 and RW = 0, only E has to be set for one pulse, including backlight
	//twi_display_command (0x28);
	twi_write(twi_display_address, 0x2C, 0x00, 0x00);
	twi_write(twi_display_address, 0x28, 0x00, 0x00);
	twi_write(twi_display_address, 0x8C, 0x00, 0x00);
	twi_write(twi_display_address, 0x88, 0x00, 0x00);
	_delay_ms(2);
	return (0);
}

uint8_t twi_display_write_char(char c) {
	uint8_t char_a = (c & 0xf0);
	uint8_t char_b = ((c<<4) & 0xf0);

	uint8_t enable = backlight | 0x05;
	uint8_t disable = backlight | 0x01;

	twi_write(twi_display_address, char_a | enable, 0x00, 0x00);
	twi_write(twi_display_address, char_a | disable, 0x00, 0x00);
	twi_write(twi_display_address, char_b | enable, 0x00, 0x00);
	twi_write(twi_display_address, char_b | disable, 0x00, 0x00);
	return (0);
}

uint8_t twi_display_write_text (uint8_t line, uint8_t position, uint8_t length, char* text) {
	// only if something has to be written (length > 0)
	if (length > 0) {
		// calculate address
		// first line 0x00 to 0x0F
		// second line 0x40 to 0x4F
		uint8_t address = (line == 0) ? (position & 0x0F) : 0x40 | (position & 0x0F);
		// set cursor position
		twi_display_set_DDRAM_address(address);
		// now write the text with a maximum length of 16
		for (uint8_t i = 0; i<0x10 && i<length; i++)
			twi_display_write_char(*(text+i));
	}
	return (0);
}

uint8_t twi_display_write_hex (uint8_t line, uint8_t position, char c) {
    // split character into two nibble and convert them into
    // printable character corresponding to the nibble value
    uint8_t high_nibble = ((c>>4) > 9) ? (c>>4)+0x37 : (c>>4)+0x30;
    uint8_t low_nibble = ((c&0x0F) > 9) ? (c&0xF)+0x37 : (c&0x0F)+0x30;
	// calculate address
	// first line 0x00 to 0x0F
	// second line 0x40 to 0x4F
	uint8_t address = (line == 0) ? (position & 0x0F) : 0x40 | (position & 0x0F);
	// set cursor position
	twi_display_set_DDRAM_address(address);
	// now write the high nibble
	twi_display_write_char(high_nibble);
	twi_display_set_DDRAM_address(++address);
	// now write the low nibble
	twi_display_write_char(low_nibble);
	return (0);
}
