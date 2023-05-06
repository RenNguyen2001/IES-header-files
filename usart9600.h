#define bitSet(reg, ind) (reg |= 1 << ind)
#define bitClear(reg, ind) (reg &= ~(1 << ind) )
#define bitCheck(reg, ind) ( (reg >> ind) & 1 )

void txByteUSART(unsigned char x){
	//wait for empty transmit buffer
	while(!(UCSR0A & (1 << UDRE0)));
	
	//Put data into buffer, sends the data
	UDR0 = x;
}

void txStringUSART(char *x){
	while (*x != '\0')
	{
		txByteUSART(*x);
		x++;
	}
}

void txIntUSART(int x){
	char array[10] = {};
	dtostrf(x, 10, 0, array);
	array[10] = '\0';
	
	txStringUSART(array);
}

void txFloatUSART(float x){
	char array[15] = {};
	dtostrf(x, 14, 3, array);
	array[14] = '\0';
	txStringUSART(array);
}

void transmitByteUSART(char x){
	while( !bitCheck( UCSR0A, UDRE0 ) );
	UDR0 = x;
}

void transmitStringUSART(char* x){
	while(*x != '\0'){
		transmitByteUSART(*x);
		x++;
	}
}

void initUSART9600()
{
	UBRR0L = (uint8_t)(103 & 0xFF);	UBRR0H = (uint8_t)(103 >> 8);	//1. Setting the baud rate to 9600
	
	bitSet(UCSR0B, TXEN0);	bitSet(UCSR0B, RXEN0);	//2. Enabling the transmit and receive functions
	UCSR0C = 0x06;	//3. set default frame format
}

void initUSART(){
	UBRR0 = 103;
	
	UCSR0C |= 3 << UCSZ00;
	UCSR0B |= 3 << TXEN0;
}