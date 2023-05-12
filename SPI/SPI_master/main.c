/*
 * spiTest.c
 *
 * Created: 6/05/2023 6:35:53 PM
 * Author : UTS
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "util/delay.h"
#include <stdlib.h>

#include "customSPI.h"


int main(void)
{
	unsigned long spiTxData = 0xFDADCCDD;
	setupMasterSPI(2);
	
    /* Replace with your application code */
    while (1) 
    {
		//transmitMasterSPI(4, &spiTxData);
		tx_and_rx(4, &spiTxData);
    }
}

