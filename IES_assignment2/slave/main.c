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
#include "pinChangeInt.h"
#include "timerTestHead.h"

volatile unsigned long spi_data_rx1 = 0;	
volatile unsigned char lastState;
volatile unsigned char counter = 0;
volatile float dutyCycleResult;

float simpleSPIvalToFloat(unsigned long spiData);


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
			counter = 0;	lastState = 0;
			dutyCycleResult = simpleSPIvalToFloat(spi_data_rx1);
			if(dutyCycleResult > 90 && dutyCycleResult < 100)
			{
				DDRD ^= (1<<PIND6);	//toggling the inverted PWM output
				DDRD ^= (1<<PIND5);	//toggling the non-inverted PWM output
			}
			//OCR2B = (int)OCR2A * dutyCycleResult;					//changing the duty cycle based on the received spi value
			t0_changeDC(dutyCycleResult);
			txStringUSART("dutyCycle: ");	txFloatUSART(dutyCycleResult);	txStringUSART("\n");
			break;
			
		}
		
		//txByteUSART(spi_data_rx1 >> 24);	txByteUSART(spi_data_rx1 >> 16);	txByteUSART(spi_data_rx1 >> 8);	txByteUSART(spi_data_rx1);
		//txByteUSART(0xFF);
		
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
	
	//t2_ctcModeSetupPin3(1, 1e6);
	setupSlaveSPI();
	initUSART(9600);
	
	setupPCIportB(PINB2);	//falling edge detection of the chip select signal
	//t2_fastPWMsetupPin3(1, 100e3, 0.5);
	t0_fastPWMsetupPin5_Pin6(2, 0.5);
	
    while (1) 
    {
		//pin change interrupt to detect the falling edge of the cs pin
    }
}

float simpleSPIvalToFloat(unsigned long spiData){
	//divide by 1e6 to get back the float value
	float floatResult = spiData/1e6;
	
	return floatResult;
}

/*
float convertSPIvalToFloat(long spiData){
	float floatResult;
	uint32_t calcTool;
	uint16_t fracPart = 0;
	uint8_t expPart = 0;
	
	//1. obtain spi result after the data is combined into a long (4 bytes)
	calcTool = spiData;

	//starting from the MSB and moving to the LSB...
	//Assuming that the sign is 0, hence positive
	
	//next 8 bits are the exponent (23 - 30)
	//2. Separate the exponent part from the data
	calcTool = spiData;
	calcTool &= (0xFFUL << 23);
	expPart |= calcTool >> 23;
	
	//first 23 (0-22) bits are the mantissa/ fractional part. Lets assume we are only doing 2 decimal points, so only taking bits 23-13
	//2. Separate the fractional part from the data
	calcTool = spiData;
	calcTool &= 0x7FFUL << 12;		//applying a bitmask to remove all other bits but also to protect bits 22-12
	fracPart |= calcTool >> 12;		//shift bits over by 12 to store in an 8 bit number
	
	return floatResult = 0;
}
*/

