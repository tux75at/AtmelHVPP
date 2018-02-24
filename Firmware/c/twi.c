# include <avr/io.h>
# include <avr/interrupt.h>
# include "twi.h"



static uint8_t twi_error_count = 0;

// i2c initialisation
uint8_t twi_init (void)
{
  // setting prescaler to 4
  // TWPS = 00
  TWSR = I2C_PRESCALER;
  // setting bitrate to 20
  // TWBR=20
  TWBR = I2C_BITRATE;
  // enabling TWI
  // disable internal PULLUPS
  PORTD &= ~(1<<PD0);
  PORTD &= ~(1<<PD1);
  TWCR = (1<<TWEN);
  return (0);
}


// i2c write
uint8_t twi_write (uint8_t twi_addr, uint8_t reg_addr, uint8_t length, uint8_t* data) {

  // TWI start and TWI address (SLA+W)
  twi_start (twi_addr << 1);
  // TWI register address
   if (TWSR == 0x20) {
    twi_stop();
    return (-1);
  }
  twi_write_data (reg_addr);
  // TWSR = 0x18 ACK received
  // TWSR = 0x20 NACK received
  // TWSR = 0x38 0x68 0x78 0xB0 Arbitration lost
  if (((TWSR == 0x18) | (TWSR == 0x28)) & (length > 0)) {
     // data transfer if there is any
    while (length) {
      twi_write_data (*data++);
      if (TWSR == 0x30) {
        twi_stop();
        return (length);
      } else if (TWSR == 0x38) {
        return (length);
      }
      length--;
    }
  } else if (TWSR == 0x20) {
     // sending STOP and return with -1
    twi_stop ();
    return (-1);
  }
  // sending stop
  twi_stop ();

  // return bytes not transfered
  return (length);
}

/*
uint8_t twi_write (uint8_t twi_addr, uint8_t data) {

	// TWI start and TWI address (SLA+W)
	twi_start (twi_addr << 1);
	// TWI register address
	if (TWSR == 0x20) {
		twi_stop();
		return (-1);
	}
	twi_write_data (data);
	// TWSR = 0x18 ACK received
	// TWSR = 0x20 NACK received
	// TWSR = 0x38 0x68 0x78 0xB0 Arbitration lost
	if (~(TWSR == 0x18) & ~(TWSR == 0x28)) {
		// sending STOP and return with -1
		twi_stop ();
		return (-1);
	}
	// sending stop
	twi_stop ();

	return (0);
}
*/

// i2c read
uint8_t twi_read (uint8_t twi_addr, uint8_t reg_addr, uint8_t length, uint8_t* data) {
  
  // TWI START and TWI address (SLA+W)
  twi_start (twi_addr << 1);

  // TWSR = 0x18 ACK received
  // TWSR = 0x20 NACK received
  // TWSR = 0x38 0x78 0xB0 Arbitration lost
  if (TWSR == 0x20) {
    twi_stop();
    return (-1);
  } else if (TWSR == 0x18) {
    // TWI register address;
    twi_write_data (reg_addr);
    // TWSR = 0x28 ACK received
    // TWSR = 0x30 NACK received
    // TWSR = 0x38 (0x68 0x78 0xB0) Arbitration lost
    if (TWSR == 0x30) {
      twi_stop();
      return (-1);
    } else if (TWSR == 0x28) {
      // ACK received, starting to read data
      // if length is zero, then only register address is written
      // no rep start with SLA+R needs to be generated
      if (length > 0) {
        // TWI REP START and TWI address (SLA+R)
        twi_rep_start ((twi_addr << 1) | 0x01);
  
        // TWSR = 0x40 ACK received
        // TWSR = 0x48 NACK received
        // TWSR = 0x38 0x68 0x78 0xB0 Arbitration lost
        if (TWSR == 0x48) {
          twi_stop();
          return (-1);
        } else if (TWSR == 0x40) {
          // data transfer if there is any
          while (length > 0) {
            *data = twi_read_data ((length-1) > 0);
            data++;
            // TWSR = 0x50 ACK transmitted
            // TWSR = 0x38 Arbitration lost
            if (TWSR == 0x38)
              return (length);
            length--;
          }
        } else
          return (length);
      }
    }
  } else
    return (length);

  // twi stop
  twi_stop();

  // return bytes not transfered
  return (length);
}


void twi_start (uint8_t address) {
  // generate start condition
  TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
  // wait for transmission end
  twi_wait_for_transmission_end();
  // write slave address to twi bus
  twi_write_data (address);
  // wait for transmission end
//                  twi_wait_for_transmission_end();
}


void twi_rep_start (uint8_t address) {
  // generate rep start condition
  TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
  // wait for transmission end
  twi_wait_for_transmission_end();
  // write slave address to twi bus
  twi_write_data (address);
                    // wait for transmission end
                    //  twi_wait_for_transmission_end();
}


void twi_stop (void) {
  //generate stop condition
  TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
  // wait for transmission end
                //  twi_wait_for_transmission_end();
}


uint8_t twi_read_data (uint8_t ack) {
  // prepair TWI for data reception
  TWCR = (1<<TWINT) | (ack? (1<<TWEA) : 0) | (1<<TWEN);
  // wait for transmitt end
  twi_wait_for_transmission_end();
  // return data
  return (TWDR);
}


void twi_write_data (uint8_t data) {
  // prepair TWI for data transmitt
  TWDR = data;
  TWCR = (1<<TWINT) | (1<<TWEN);
  // wait for transmitt end
  twi_wait_for_transmission_end();
}


void twi_wait_for_transmission_end(void) {
  uint8_t count = 255;
  // as long the TWINT flag is not set (TWI still busy)
  while (!(TWCR & (1<<TWINT))) {
    count--;
    if (count == 0) {
      twi_error_count++;
      break;
    }
  }
}

