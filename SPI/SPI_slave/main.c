/*
 * slaveSPI_test.c
 *
 * Created: 7/05/2023 12:27:13 AM
 * Author : UTS
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "customSPI.h"
#include "usart9600.h"
#include "timerTestHead.h"
#include "extInterrupts.h"
#include "pinChangeInt.h"

volatile unsigned long spi_data_rx1 = 0;	
volatile unsigned char lastState;
volatile unsigned char counter = 0;


ISR(SPI_STC_vect){	//serial complete interrupt
	//txStringUSART("wow");
	if((lastState == 1) && (!( DDR_SPI & (1 << pinSS))))
	{
		unsigned long temp = SPDR;	//something wrong with SPDR (it is 1 byte/8 bits)
		
		switch(counter)
		{
			case 0:
			spi_data_rx1 = 0;
			spi_data_rx1 |= (temp << 24);
			counter++;
			break;
			
			case 1:	spi_data_rx1 |= (temp << 16);
			counter++;
			break;
			
			case 2:	spi_data_rx1 |= (temp << 8);
			counter++;
			break;
			
			case 3: spi_data_rx1 |= (temp);
			counter = 0;
			lastState = 0;
			break;
			
		}
		
		txByteUSART(spi_data_rx1 >> 24);	txByteUSART(spi_data_rx1 >> 16);	txByteUSART(spi_data_rx1 >> 8);	txByteUSART(spi_data_rx1);
		txByteUSART('n');
	}
	while(!(SPSR & (1 << SPIF)));	//checking if SPI interrupt flag isn't set/serial transfer not completed
}

ISR(PCINT0_vect)
{
	if(!( DDR_SPI & (1 << pinSS)))
	{
		lastState = 1;	//txStringUSART("hi");
	}
	
}


int main(void)
{
	t2_ctcModeSetupPin3(1, 1e6);
	setupSlaveSPI();
	initUSART(1e6);
	
	setupPCIportB(PINB2);
	
    while (1) 
    {
		//pin change interrupt to detect the falling edge of the cs pin
    }
}


