
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

volatile unsigned char spi_data_tx = 0;
volatile unsigned char spi_data_rx = 0;



void transmitSPI(char numBytes, long *pDataSpi)	//4 is the max bytes this function is intended for
{
	
	//long *pDataSpi = 0xFFABCCDD;
	PORTB &= ~(1 << pinSS);	//setting SLAVE SELECT low to initiate transfer
	
	
	for(int i = 3; i >= 0; i--)
	{
		SPDR = *pDataSpi >> i*8;
		while(!(SPSR & (1 << SPIF)));	//checking if SPI interrupt flag isn't set/serial transfer not completed
	}

	PORTB |= (1 << pinSS); 	//setting SLAVE SELECT high to end transfer
	
	/*
	//============================= TX USART ================================
	PORTB &= ~(1 << pinSS);	//setting SLAVE SELECT low to initiate transfer
	
	spi_data_tx = 0xF2;
	SPDR = spi_data_tx;	//putting data into the SPI Data register
	
	
	asm volatile("nop");
	while(!(SPSR & (1 << SPIF)));	//checking if SPI interrupt flag isn't set/serial transfer not completed
	
	spi_data_tx = 0b00011001;
	SPDR = spi_data_tx;	//putting data into the SPI Data register
	
	while(!(SPSR & (1 << SPIF)));	//checking if SPI interrupt flag isn't set/serial transfer not completed

	PORTB |= (1 << pinSS); 	//setting SLAVE SELECT high to end transfer
	*/
}

void setupSPI()
{
	cli();
	//set SS pin as output
	DDRB |= 1 << pinSS;
	DDR_SPI |= (1 << pinSS) | (1 << pinSCK) | (1 << pinMOSI);
	
	//enable SPI, set as master
	SPCR |= (1 << SPE) | (1 << MSTR);
	
	//enable SPI receiving from slave
	DDR_SPI &= ~ (1 << pinMISO);
	
	//enable the SPI interrupt flag to be reset after each byte for multi byte transfer
	//SPCR |= 1 << SPIE;
	//sei();
}

void receiveSPI()
{
	
}