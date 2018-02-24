# Atmel HVPP (High Voltage Parallel Programming)

This little project was created after a delivery of some ATMEGA48 from Aliexpress which did not work in the biginning.
After some research and asking in a forum, i came to the conclusion that the fuses might be defect and it might be worth fixing them.
Most ideas are from [Mighty Ohm HV Rescue Shield](http://mightyohm.com/blog/products/hv-rescue-shield-2-x/) and [Fusebit Doctor](http://mdiy.pl/atmega-fusebit-doctor-hvpp/?lang=en). The only problem was, that there is no full description and no sourcecode available, so i started to develop my own code and at the end i successfully revived these ATMega chips.
If you have only 3 defect chips like me, then it might not be worth fixing them, i did this, because i was interested on this reviving method and now i have everything to fix any other chips with the same package.

## Features
* shows the signature on the display
* shows the actual fuses on the display

## Needed hadware
* In case you have a TQFP package you need a [TQFP adapter (Aliexpress ~ € 10](https://www.aliexpress.com/item/Free-shipping-CHIP-PROGRAMMER-SOCKET-TQFP32-QFP32-LQFP32-TO-DIP28-TQFP32-to-dip32-adapter-socket-support/32425007152.html)
* Breadboard or [Proto Shield (Aliexpress ~ € 1)] (https://www.aliexpress.com/item/1-Set-Standard-Proto-Screw-Shield-Board-For-Arduino-Compatible-Improved-Version-Support-A6-A7-In/32837045842.html)
* Arduino Leonardo (Should work on most Arduinos, you need the correct ammount of IOs)
* For the correct rise time of VCC and 12 V you need 2 PNP and 1 NPN transistor (e.g. BC547 and BC557)
* Some resistors (see schematics in doc folder)
* An [I2C display (Aliexpress > € 2)] (https://www.aliexpress.com/item/1602-16x2-HD44780-Character-LCD-w-IIC-I2C-Serial-Interface-Adapter-Module/2029819446.html) if you want to see whats wrong with the chip and verify the fuses
* The switch button is not used in this version
* ATmel ISP (cheap version from aliexpress should work)

## Software requrements
* building environment only tested under linux, might need some adaption for other operating system but using Oracle Virtual Box or other virtualisation, it is easy to run linux on any OS
* avr tools (apt-get install gcc-avr avr-libc)

## How to compile and flash
The makfile has the target "program" which compiles and flashes using the settings in the Makefile. 
If you only want to build just type "make" in a terminal.
Make sure to change the device if you use a different Arduino board, also make sure to change the IO settings in the main.c file at the beginning. If you flash with this Makefile, the bootloader will be removed. You can fix the bootloader with the Arduino IDE and the Atmel ISP device.

## disclaimers
This code is provided as-is, with NO guarantees or liabilities.
If you dont know how to fix the bootloader of your arduino or dont have an Atmel ISP, you should consider changing the code to Arduino syntax and use the Arduino system to flash the binary. The resistors within the signal lines might be too small if all pins are driven from the Atmel chip with the defect fuses. If you are sure the chip is unprogrammed then this value should work (check data sheet, VCC and resistor value to calculate current of one IO and sum up the current of all IOs of a bank and possible also all IOs, check all values, single IO max current, max current of IO bank and max current of all IOs for specified values). If the maximum current is exceeded both chips may be permanently damaged.


