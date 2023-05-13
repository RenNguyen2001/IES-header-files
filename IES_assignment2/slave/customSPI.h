
#include <avr/io.h>
#include <avr/interrupt.h>
#include "util/delay.h"
#include <stdlib.h>

//SPI PIN DEFINITIONS
#define DDR_SPI DDRB
#define pinSCK PB5	//pin 13
#define pinMOSI PB3	//pin 11
#define pinMISO PB4	//pin 12
#define pinSS PB2	//pin 10



void transmitMasterSPI(char numBytes, unsigned long *pDataSpi)	//4 is the max bytes this function is intended for
{
	
	//long *pDataSpi = 0xFFABCCDD;
	PORTB &= ~(1 << pinSS);	//setting SLAVE SELECT low to initiate transfer
	
	
	for(int i = numBytes - 1; i >= 0; i--)
	{
		_delay_ms(1);
		SPDR = *pDataSpi >> i*8;
		while(!(SPSR & (1 << SPIF)));	//checking if SPI interrupt flag isn't set/serial transfer not completed
	}
	_delay_ms(1);

	PORTB |= (1 << pinSS); 	//setting SLAVE SELECT high to end transfer
	
}

void transmitSlaveSPI(char numBytes, long *pDataSpi)
{
	/*
	for(int i = numBytes - 1; i >= 0; i--)
	{
		SPDR = *pDataSpi >> i*4;
		while(!(SPSR & (1 << SPIF)));	//checking if SPI interrupt flag isn't set/serial transfer not completed
	}
	*/
	SPDR = 0x12;
	//while (!(SPSR & (1 << SPIF)));	//checking for SPI interrupt flag in SPI status register/completion of serial transfer
	
	SPDR = 0xFF;
	//while (!(SPSR & (1 << SPIF)));	//checking for SPI interrupt flag in SPI status register/completion of serial transfer
	
	SPDR = 0xDC;
	while (!(SPSR & (1 << SPIF)));	//checking for SPI interrupt flag in SPI status register/completion of serial transfer
	
	SPDR = 0xAB;
	while (!(SPSR & (1 << SPIF)));	//checking for SPI interrupt flag in SPI status register/completion of serial transfer
}



void setupMasterSPI()
{
	//cli();
	//set SS pin as output
	DDR_SPI |= (1 << pinSS) | (1 << pinSCK) | (1 << pinMOSI);
	
	//enable SPI, set as master
	SPCR |= (1 << SPE) | (1 << MSTR);
	
	//enable SPI receiving from slave
	DDR_SPI &= ~ (1 << pinMISO);
	
	//enable the SPI interrupt flag to be reset after each byte for multi byte transfer
	//SPCR |= 1 << SPIE;
	//sei();
}

void setupSlaveSPI()
{
	cli();
	
	// enable SPI, set as slave
	SPCR |= (1 << SPE) | (1 << SPIE);
	sei();	//enable global interrupts
	
	// enable SPI transmitting slave->master
	DDR_SPI |= 1 << pinMISO;
	//bitSet(DDR_SPI, pinMISO);
	sei();
}


