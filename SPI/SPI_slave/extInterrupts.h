#include <avr/io.h>

const unsigned char extMode[] =
{
	0,	//Interrupt on low signal
	1,	//Interrupt on any signal change
	2,	//Interrupt on falling edge
	3	//Interrupt on rising edge
};

//ISR vector names
/*
	INT0_vect
	INT1_vect
*/


void setupExtInt0(unsigned char modeSel)
{
	cli();	//disabling interrupts
	EICRA = extMode[modeSel];	//selecting the interrupt mode
	EIMSK = (1 << INT0);	//enabling the external interrupts
	sei();	//enabling all interrupts
}