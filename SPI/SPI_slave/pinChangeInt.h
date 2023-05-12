#include <avr/io.h>
#include <avr/interrupt.h>

const unsigned char portSel[] =
{
	1,			//turn on port b
	2,			//turn on port c
	0b100,		//turn on port d
	0b111		//turn on all ports
};

//ISR vector names
/*
	PCINT0_vect		//port B
	PCINT1_vect		//port C
	PCINT2_vect		//port D
*/

void setupPCIportB(unsigned char intPin){	//PCINT0
	cli();	//clearing interrupts
	PCICR = portSel[0];		//choosing which port(s) to turn on
	PCMSK0 |= 1 << intPin;			//turn on the a certain pin in port B
	sei();
}