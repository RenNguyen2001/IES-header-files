/*
 * slaveSPI_test.c
 *
 * Created: 7/05/2023 12:27:13 AM
 * Author : UTS
 */ 

#include <avr/io.h>

#include "customSPI.h"
#include "usart9600.h"
#include "timerTestHead.h"

unsigned long spi_data_rx1 = 0;	//just for troubleshooting
char counter = 0;


ISR(SPI_STC_vect){	//serial complete interrupt
	if(!( DDR_SPI & (1 << pinSS) ) ){	//slave waits for SS to be low (waiting for master to transfer)
		unsigned long temp = SPDR;	//something wrong with SPDR (it is 1 byte/8 bits)
		//txByteUSART(temp);
		
		//unsigned long temp = 0x1F;	//this works
		//unsigned long temp = 0xF;		//this works too
		//unsigned long temp = 0x123;	//just shows the last two numbers
		
		//spi_data_rx1 |= (temp << counter*8);
		//spi_data_rx1 |= (SPDR);	//this doesn't work
		//spi_data_rx1 = (SPDR << 8);	//this works
		//spi_data_rx1 = spi_data_rx1 + (SPDR << counter*8);	//this produces non-FF results
		//spi_data_rx1 = (SPDR << counter*8);	//this produces all 0 results
		
		/*
		if(counter == 0)		//THIS WORKS
		{
			spi_data_rx1 = (temp << 8);
		}
		else if(counter == 1)
		{
			spi_data_rx1 |= (temp << 16);
		}
		*/
		
		
		// THIS WORKS TOO, ALTHOUGH A BIT INCONSISTENT
		/*
		if(counter == 0)		
		{
			spi_data_rx1 = (temp);
		}
		else if(counter == 1)
		{
			spi_data_rx1 |= (temp << 8);
		}
		else if(counter == 2)
		{
			spi_data_rx1 |= (temp << 16);
		}
		*/
		
		switch(counter)
		{
			case 0:	spi_data_rx1 = (temp << 24);
			break;
			
			case 1:	spi_data_rx1 |= (temp << 16);
			break;
			
			case 2:	spi_data_rx1 |= (temp << 8);
			break;
			
			case 3: spi_data_rx1 |= (temp);
			break;
			
		}
		
		
		/*
		spi_data_rx1 |= 0x1F;
		spi_data_rx1 |= (0xDCUL << 8);
		spi_data_rx1 |= (0x12UL << 16);
		*/
		//txByteUSART(SPDR);
			
		txByteUSART(spi_data_rx1 >> 24);	txByteUSART(spi_data_rx1 >> 16);	txByteUSART(spi_data_rx1 >> 8);	txByteUSART(spi_data_rx1);		
		//txByteUSART(SPDR);
		//txLongUSART(spi_data_rx1);	
		txByteUSART('n');
		counter++;

		while(!(SPSR & (1 << SPIF)));	//checking if SPI interrupt flag isn't set/serial transfer not completed
		
	}
}


int main(void)
{
	//char bytesReceived;
	t2_ctcModeSetupPin3(1, 39e5);
	setupSlaveSPI();
	initUSART(4e6);	//around baud of 85000 is where the UART starts giving incorrect readings
	
	
	
    /* Replace with your application code */
    while (1) 
    {
	
		//txStringUSART("; rx_slave: ");
		
		//txStringUSART("\n");
		if(counter >= 3)
		{
			counter = 0;
			//txLongUSART(0xFFCCDDEE);	txByteUSART('n');
		}
		
    }
}

