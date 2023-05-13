/*
 * Lab4.c
 *
 * Created: 16/03/2023 9:38:59 AM
 * Author : joshg
 */ 

//****************** Header Files ******************
#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <math.h>
#define F_CPU 16000000UL
#include "util/delay.h"

//****************** User Defined Header Files ******************
#include "bitManipulation.h"
#include "myUSART.h"
#include "customSPI.h"

//****************** Pinout ******************
/*
	Analog read (potentiometer)			- A0 
	button (pulled up)					- PD2 / 2
	pwm output							- PD3 / 3
	analog compare (3.3V ref pin)		- PD7 / 7
	analog compare (conn to PD3)		- PD6 / 6

*/


//****************** Global Variables ******************
char flag_rising = 1;
char flag_ready = 0; // ready for uploading

volatile float overflow_cnt = 0;
volatile unsigned int overflow_cnt_array[4] = {};

char ici_cnt = 0;

volatile float tLow = 0; // the time interval between the falling edge and rising edge of the waveform
volatile float tHigh = 0; // the time interval between the rising edge and falling edge of the waveform
volatile unsigned int tcnt1_new = 0;
volatile unsigned int tcnt1_old = 0;

unsigned char ocr0b = 10;

float t1 = 0, t2 = 0;

//****************** Function Declaration ******************
float myMap(float x, float x1, float x2, float y1, float y2);
void initADC_SC(void);
int readADC(char);

//****************** Mapping Function ******************
float myMap(float x, float x1, float x2, float y1, float y2)
{
	return (y2 - y1) / (x2 - x1) * (x - x1) + y1;
}

//****************** ADC ******************
void initADC_SC(void)
{
	bitSet(ADMUX, REFS0);
	ADCSRA |= 7; // Prescaler 128 (10bit resolution)
	bitSet(ADCSRA, ADEN);
}

int readADC(char channel)
{
	ADMUX &= 0xF0;
	ADMUX |= channel;
	
	bitSet(ADCSRA, ADSC);
	
	while(bitCheck(ADCSRA, ADSC)); // wait for ADC to finish conversion
	
	int tmp = ADC;
	return tmp;
}

//****************** Input Capture ******************
ISR(TIMER1_CAPT_vect)
{
	if(flag_rising)
	{
		tcnt1_new = ICR1;
		bitClear(TCCR1B, ICES1); //Clears the input capture edge select bit (ICES1) in the Timer1 control register (TCCR1B) to prepare for the next falling edge.
		
		t1 = (float)((tcnt1_new + 1) + overflow_cnt*65536)
		/ (float) 16e6 * 1000.; // the time interval of the rising edge
		
		tLow = t1 - t2; // the duration of the low phase of the signal
		
		flag_rising = 0;
	}
	else
	{
		tcnt1_new = ICR1;
		bitSet(TCCR1B, ICES1); // Sets the ICES1 bit to prepare for the next rising edge.
		
		t2 = (float)((tcnt1_new + 1) + overflow_cnt*65536)
		/ (float) 16e6 * 1000.; // the time interval of the falling edge
		
		tHigh = - t1 + t2; // the duration of the high phase of the signal
		
		flag_rising = 1;
		
		ici_cnt++; // counts the number of consecutive rising edges that have been detected.
		
		if(ici_cnt == 3) // indicating the measurement is complete
		{
			flag_ready = 1;
			ici_cnt = 0;
			bitClear(TIMSK1, ICIE1);
		}
	}
}

ISR(TIMER1_OVF_vect){
	overflow_cnt++;
}

//****************** Button ******************
#define Btn1 PORTD2

void initPins(void)
{
	bitClear(DDRD, Btn1); //Set as input
	bitSet(PORTD, Btn1); //Enable pull-up
}

void initExtInt(void)
{
	bitSet(EICRA, ISC00);
	bitClear(EICRA, ISC01);
	
	bitSet(EIMSK, INT0);
}

char btnStatusOld_EN = 1; //Enable Button Status Tracking

ISR(INT0_vect)
{
	char btnStatusNew_EN = bitCheck(PIND, Btn1);
	
	if (btnStatusNew_EN != btnStatusOld_EN){
		
		_delay_ms(20); //Debounce Delay
		btnStatusNew_EN = bitCheck(PIND, Btn1);
		
		if (btnStatusNew_EN != btnStatusOld_EN ){
			
			btnStatusOld_EN = btnStatusNew_EN;
			
			if (!btnStatusNew_EN ){
				TCCR2A ^= (1 << COM2B1); //toggle PWM Output
			}
		}
	}
}

//****************** Main ******************
int main()
{
	initADC_SC(); // enable ADC for single conversion mode
	initPins();
	initExtInt();
	setupMasterSPI(3);	//enable spi for master at f_ocs/128
	
	//****************** PWM ******************
	bitSet(DDRD, PORTD3);
	//Set Fast PWM signal mode: 7
	bitSet(TCCR2A, WGM20);
	bitSet(TCCR2A, WGM21);
	bitSet(TCCR2B, WGM22);
	
	//Set Output to non-inverted PWM output
	bitSet(TCCR2A, COM2A0);
	bitSet(TCCR2A, COM2B1);

	// Set Prescaler to 1024
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
	
	float freq = 0;
	float duty_cycle = 0;
	float duty = 0;
	
	// PWM Frequency
	freq = 100;
	OCR2A = (int)freq;
	
	//****************** Analogue Comparator ******************
	// In AC, both pins are input for comparison
	bitClear(DDRD, PD6); // connected to PD3
	bitClear(DDRD, PD7); // 3.3V input
	
	bitSet(ACSR, ACIC); // enable comparator-based input capture
	
	bitSet(TCCR1B, ICES1); // start with rising edge trigger
	bitSet(TIMSK1, ICIE1); // enable input capture interrupt
	bitSet(TIMSK1, TOIE1); // overflow interrupt of TC1
	
	bitSet(TCCR1B, CS10); // start TC1 by effecting clock

	initUSART(MY_UBRR);
	
	sei(); //enable global interrupts
	
	float tHighTrue;
	float tLowTrue;
	unsigned long dutycSPI;

	while (1)
	{
		// Duty Cycle
		uint16_t adc_reading2 = readADC(0); // connected to potentiometer 1
		duty = myMap(adc_reading2, 0, 1023, 30, 100);	//txStringUSART("ADC reading:");	txFloatUSART(adc_reading2);	txStringUSART("\n");
		
		duty_cycle = (float)((duty * freq)/100);
		OCR2B = (int)duty_cycle; // set the duty cycle of OCR2B
		
		_delay_ms(100); // add a delay to prevent too frequent updates
		
		if(flag_ready){
			
			tHighTrue = freq*1024.0/16.0e6 *
			((float) duty_cycle + 1.0)/freq * 1000.;
			tLowTrue = freq*1024.0/16.0e6 *
			(freq - (float) duty_cycle)/freq * 1000.;
			
			//Period Calculation
			float tTotalTimeTrue = (float)(tHighTrue + tLowTrue)/1000.0;
			float tTotalTime = (float)(tHigh + tLow)/1000.0;
			
			//Duty Cycle Calculations
			float tDutyTrue = (float)tHighTrue/tTotalTimeTrue/1000;
			txFloatUSART(tDutyTrue);
			float tDuty = (float)tHigh/tTotalTime/1000;
			txFloatUSART(tDuty);
			txStringUSART("\n");
			
			dutycSPI = tDuty * 1e6;
			
			transmitMasterSPI(4, &dutycSPI);
			
			flag_ready = 0;
			bitSet(TIMSK1, ICIE1);
		}
	}
}