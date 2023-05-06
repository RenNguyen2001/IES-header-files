#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

// ================================================== USING THIS HEADER FILE =======================================================

// GENERAL FORMAT

// t0_normalModeSetup( prescalarSetting, desiredTimeDelay)
// t0_ctcModesetupPin6( prescalarSetting, desiredFrequency, dutyCycle)	e.g. t0_ctcModesetupPin6( {1-5}, {your choise}, {0.0-1.0} )
// t0_fastPWMsetupPin6( prescalarSetting, desiredFrequency, dutyCycle)
// t0_phasePWMsetupPin5( prescalarSetting, desiredFrequency,, dutyCycle)

//FOR timer0 and timer1, choose setting 5 for the prescalar
//For timer2, choose setting 7 for the prescalar

// ================================================= PRESCALAR LOOKUP VALUES =======================================================
const char t01PrescalarsSettings[] = {	//t0 and t1 prescalars are the same
	0,	//clock off
	1,	//0 - no prescaling -  16us overflow
	2,	//8 - 128us overflow
	3,	//64 - 1.024us overflow
	4,	//256 - 4.096ms overflow
	5	//1024 - 16ms overflow
};

const int t01PrescalarsVals[] = {	//t0 and t1 prescalars are the same
	0,	
	1,	
	8,	
	64,	
	256,	
	1024	
};

const float t01_ofTimes[] = {
	0,
	16e-6,
	128e-6,
	1.024e-6,
	4.096e-3,
	16e-3
};

const char t2PrescalarsSettings[] = {
	0,	//clock off
	1,	//0 - no prescaling
	2,	//8
	3,	//32
	4,	//64
	5,	//128
	6,	//256
	7	//1024
};

const int t2PrescalarsVals[] = {
	0,		//clock off
	1,		//0 - no prescaling
	8,		//8
	32,		//32
	64,		//64
	128,	//128
	256,	//256
	1024	//1024
};

// ================================================================================================================================

const float f_cpu = 16e6;


// ======================================================= TIMER 0 SETUPS ==========================================================

int t0_normalModeSetup(int psVal, int delay) //t0_normalModeSetup( prescalarSetting, desiredTimeDelay)
{
	const int MAX = 255;
	int numOF;
	
	cli();
	TCCR0B |= t01PrescalarsSettings[psVal];	//2. Setting prescalar
	TIMSK0 |= (1<<TOIE0);
	sei();
	
	//calculating the num of overflows required for certain delay
	
	float ofTime = (MAX + 1)*( t01PrescalarsVals[psVal] / f_cpu );
	
	numOF = ((float)delay/ofTime);
	return numOF;
}

void t0_ctcModeSetupPin5(int psVal, float f_req){	//t0_ctcModesetupPin6( prescalarSetting, desiredFrequency)
	
	DDRD = 1<<5;	//setting OC0A to output
	
	TCCR0A |= (1<<WGM01);	//1. setting to CTC mode
	TCCR0B |= t01PrescalarsSettings[psVal];	//2. Setting prescalar
	TCCR0A |= 1<<COM0B0;	
	
	//calculate the OCR0A value required for a certain frequency
	OCR0A = ( f_cpu/(2 * t01PrescalarsVals[psVal] * f_req) ) - 1;	//e.g. 200 for 38 HZ (with prescalar of 1024)
	
	
}

void t0_fastPWMsetupPin5(int psVal, float f_req, float dutyCycle)	//t0_fastPWMsetupPin6( prescalarSetting, desiredFrequency)
{
	
	DDRD = 1<<5;	//setting OC0A to output
	
	TCCR0A |= (1<<WGM01) + (1<<WGM00);	TCCR0B |= (1<<WGM02);	//setting to fastPWM mode - TOP = OCRA
	TCCR0B |= t01PrescalarsSettings[psVal];
	TCCR0A |= 1<<COM0B1;
	
	//calculate the OCR0A value required for a certain frequency
	OCR0A = (((f_cpu/(t01PrescalarsVals[psVal] * f_req) )/2) - 1)*2;
	OCR0B = (int)OCR0A*dutyCycle;
	
	//lowest possible frequency for each psVal setting
	/*
	1 - 1:		62.5 kHz
	2 - 8:		7.8 kHz
	3 - 64:		976 Hz
	4 - 256:	244 Hz
	5 - 1024:	61 Hz
	*/
}

void t0_phasePWMsetupPin5(int psVal, float f_req, float dutyCycle)
{
	DDRD = 1<<5;	//setting OC0A to output
	
	TCCR0A |= (1<<WGM00);	TCCR0B |= (1<<WGM02);	//setting to phase correct mode
	TCCR0B |= t01PrescalarsSettings[psVal];
	TCCR0A |= 1<<COM0B1;
	
	//calculate the OCR0A value required for a certain frequency
	OCR0A = (f_cpu/(2*t01PrescalarsVals[psVal] * f_req) );
	OCR0B = (int)OCR0A*dutyCycle;
}


// ================================================================================================================================



// ======================================================= TIMER 1 SETUPS ==========================================================

int t1_normalModeSetup(int psVal, int delay) //t1_normalModeSetup( prescalarSetting, desiredTimeDelay)
{
	// IMPORTANT NOTE: AT PRESCALER OF 1024, THE DELAY IS LARGER THAN 1 SECOND
	const long MAX = 65535;	//int max value is 65536/2bytes, calculations didn't work when using int, so switching to long instead
	int numOF;
	
	cli();
	TCCR1B |= t01PrescalarsSettings[psVal];	//2. Setting prescalar
	TIMSK1 |= (1<<TOIE1);
	sei();
	
	//calculating the num of overflows required for certain delay
	
	float ofTime = (MAX + 1)*( t01PrescalarsVals[psVal] / f_cpu );
	
	numOF = ((float)delay/ofTime);
	return numOF;
}


void t1_ctcModeSetupPin10(int psVal, float f_req){	//t1_ctcModesetupPin6( prescalarSetting, desiredFrequency)
	DDRB = 1<<2;	//setting OC0A to output
	
	TCCR1B |= (1<<WGM12);	//1. setting to CTC mode
	TCCR1B |= (t01PrescalarsSettings[psVal]);	//2. Setting prescalar
	TCCR1A |= (1<<COM1A0) + (1<<COM1B0);
	
	//calculate the OCR1A value required for a certain frequency
	OCR1A = ( f_cpu/(2 * t01PrescalarsVals[psVal] * f_req) ) - 1;
	//OCR1AH = (OCR1A_tmp>>8);
	//OCR1AL = (OCR1A_tmp & 0xFF);	
	
	
	
}

void t1_fastPWMsetupPin10(int psVal, float f_req, float dutyCycle)	//t1_fastPWMsetupPin6( prescalarSetting, desiredFrequency, duty)
{
	
	DDRB = 1<<2;	//setting OC0A to output
	
	TCCR1A |= (1<<WGM11) + (1<<WGM10);	TCCR1B |= (1<<WGM13) + (1<<WGM12);	//setting to fastPWM mode
	TCCR1B |= t01PrescalarsSettings[psVal];
	TCCR1A |= (1<<COM1A1) + (1<<COM1B1);
	
	//calculate the OCR1A value required for a certain frequency
	OCR1A = (((f_cpu/(t01PrescalarsVals[psVal] * f_req) )/2) - 1)*2;
	OCR1B = (int)OCR1A*dutyCycle;
	
	//lowest possible frequency for each psVal setting
	/*
	1 - 1:		62.5 kHz
	2 - 8:		7.8 kHz
	3 - 64:		976 Hz
	4 - 256:	244 Hz
	5 - 1024:	61 Hz
	*/

}

void t1_phasePWMsetupPin10(int psVal, float f_req, float dutyCycle)
{
	DDRB = 1<<2;	//setting OC0A to output
	
	TCCR1A |= (1<<WGM11) + (1<<WGM10);	TCCR1B |= (1<<WGM13);	//setting to phase correct mode
	TCCR1B |= t01PrescalarsSettings[psVal];
	TCCR1A |= (1<<COM1A1) + (1<<COM1B1);
	
	//calculate the OCR0A value required for a certain frequency
	OCR1A = (f_cpu/(2*t01PrescalarsVals[psVal] * f_req) );
	OCR1B = (int)OCR1A*dutyCycle;
}


// ================================================================================================================================

int t2_normalModeSetup(int psVal, int delay) //t2_normalModeSetup( prescalarSetting, desiredTimeDelay)
{
	const int MAX = 255;
	int numOF;
	
	cli();
	TCCR2B |= t2PrescalarsSettings[psVal];	//2. Setting prescalar
	TIMSK2 |= (1<<TOIE2);
	sei();
	
	//calculating the num of overflows required for certain delay
	
	float ofTime = (MAX + 1)*( t2PrescalarsVals[psVal] / f_cpu );
	
	numOF = ((float)delay/ofTime);
	return numOF;
}

void t2_ctcModeSetupPin3(int psVal, float f_req){	//t0_ctcModesetupPin6( prescalarSetting, desiredFrequency)
	
	DDRD = 1<<3;	//setting OC2A to output
	
	TCCR2A |= (1<<WGM21);	//1. setting to CTC mode
	TCCR2B |= t2PrescalarsSettings[psVal];	//2. Setting prescalar
	TCCR2A |= 1<<COM2B0;
	
	//calculate the OCR0A value required for a certain frequency
	OCR2A = ( f_cpu/(2 * t2PrescalarsVals[psVal] * f_req) ) - 1;	//e.g. 200 for 38 HZ (with prescalar of 1024)
	
}

void t2_fastPWMsetupPin3(int psVal, float f_req, float dutyCycle)	//t2_fastPWMsetupPin6( prescalarSetting, desiredFrequency)
{
	
	DDRD = 1<<3;	//setting OC2A to output
	
	TCCR2A |= (1<<WGM21) + (1<<WGM20);	TCCR2B |= (1<<WGM22);	//setting to fastPWM mode - TOP = OCRA
	TCCR2B |= t2PrescalarsSettings[psVal];
	TCCR2A |= 1<<COM2B1;
	
	//calculate the OCR0A value required for a certain frequency
	OCR2A = (((f_cpu/(t2PrescalarsVals[psVal] * f_req) )/2) - 1)*2;
	OCR2B = (int)OCR2A*dutyCycle;
	
	//lowest possible frequency for each psVal setting
	/*
	1 - 1:		62.5 kHz
	2 - 8:		7.8 kHz
	4 - 64:		976 Hz
	6 - 256:	244 Hz
	7 - 1024:	61 Hz
	*/
}

void t2_phasePWMsetupPin3(int psVal, float f_req, float dutyCycle)
{
	DDRD = 1<<3;	//setting OC0A to output
	
	TCCR2A |= (1<<WGM20);	TCCR2B |= (1<<WGM22);	//setting to phase correct mode
	TCCR2B |= t2PrescalarsSettings[psVal];
	TCCR2A |= 1<<COM2B1;
	
	//calculate the OCR0A value required for a certain frequency
	OCR2A = (f_cpu/(2*t2PrescalarsVals[psVal] * f_req) );
	OCR2B = (int)OCR2A*dutyCycle;
}

