// Global Variables
#define BAUD 9600
#define MY_UBRR (F_CPU/16.0/BAUD - 1)

// Function Declarations
// USART
void initUSART(int ubrr);
void txByteUSART(unsigned char x);
void txStringUSART(char *x);
void txIntUSART(float x);
void txFloatUSART(float x);

//**************User Defined Functions**************
void initUSART(int ubrr)
{
	UBRR0 = ubrr;
	
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void txByteUSART(unsigned char x)
{
	// Wait for empty transmit buffer
	while(!(UCSR0A & (1 << UDRE0)));
	// Put data into buffer, send the data
	UDR0 = x;
}

void txStringUSART(char *x)
{
	while(*x != '\0')
	{
		txByteUSART(*x);
		x++;
	}
}

void txIntUSART(float x)
{
	char array[10] = {};
	dtostrf(x, 10, 0, array);
	array[10] = '\0';
	
	txStringUSART(array);
}

void txFloatUSART(float x)
{
	char array[15] = {};
	dtostrf(x, 14, 3, array);
	array[14] = '\0';
	
	txStringUSART(array);
}

