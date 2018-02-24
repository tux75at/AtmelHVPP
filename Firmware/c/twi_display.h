#ifndef TWI_DISPLAY_H_
#define TWI_DISPLAY_H_


// display is connected to twi (i2c) with a shift register in 4 bit mode
// connection as following:
// P0 - RS
// P1 - RW
// P2 - En
// P3 - Backlight
// P4 - D4
// P5 - D5
// P6 - D6
// P7 - D7

#define twi_display_address		0x27
//#define twi_display_address		0x3F
// 3F -> 0011 1111
//       0011 1AAA
// 27 -> 0010 0111
//       0010 0AAA

// define some command values for better readability
#define C_DISP_ON_CURSOR_OFF_CORSUR_STEADY	0x0C
#define C_DISP_ON_CURSOR_ON_CORSUR_STEADY	0x0E
#define C_DISP_ON_CURSOR_ON_CORSUR_BLINK	0x0F

// Settings
// 4 bit mode is hard coded and can not be changed
// number of lines 0 - 1 line, 2 - 2/4 lines
#define TWI_DISP_LINES			1
// font 0 - 5x7 font, 1 - 5x10 font (does not change for 2x16 display?)
#define TWI_DISP_FONT			0
// display on/off
#define TWI_DISP_ON				1
// cursur on/off
#define TWI_DISP_CURSOR_ON		0
// cursur blink
#define TWI_DISP_CURSOR_BLINK	0
// cursor increment/decrement (1/0)
#define TWI_DISP_CURSOR_INC		1
// display shift (0 off, 1 on)
#define TWI_DISP_SHIFT			0
// shift direction (0 left, 1 right)
#define TWI_DISP_SHIFT_DIR		0
// shift cursor or display (0 cursor, 1 display)
#define TWI_DISP_SHIFT_DISP		0

// i2c display initialization
uint8_t twi_display_init (void);

// i2c display command
uint8_t twi_display_reset (void);
uint8_t twi_display_command (uint8_t command);
uint8_t twi_display_clear (void);
uint8_t twi_display_cursor_home (void);
uint8_t twi_display_set_DDRAM_address (uint8_t address);
uint8_t twi_display_set_backlight (uint8_t on);
uint8_t twi_display_set_4bit_mode (void);
uint8_t twi_display_write_char (char c);
uint8_t twi_display_write_text (uint8_t line, uint8_t position, uint8_t length, char* text);
uint8_t twi_display_set_cursor_position (uint8_t line, uint8_t position);
uint8_t twi_display_write_text_hex (uint8_t line, uint8_t position, char c);

#endif
