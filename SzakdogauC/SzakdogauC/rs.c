// rs.c
#include "rs.h"						//180.o, 61.o interrupt priority, 188
#include <avr/interrupt.h>
#include <string.h>
#include "UART.h"
#define F_CPU 8000000UL
#include <util/delay.h>

extern char data_fogad[9];
uint8_t counter=0;
uint8_t seged=0;
uint8_t bejovo=0;

void RS_init() 
{
	DDRD |= 0b00001000;
	DDRD &= 0b11111011;		//PD2 külsõ UART interrupt receive, PD3 külsõ UART interrupt transmit
	
	DDRC &= ~(1<<7);
	DDRE &= ~(1<<2);
	DDRC |= (1<<7);
	DDRE |= (1<<2);			//USART0 külsõ órajel
}

ISR(USART1_RX_vect)
{	
	TIMSK&=0b11111101;
	
	bejovo=UDR1;

	if (seged==0)
	{
		switch(bejovo)
		{
			case 0x11: seged=1; break;
			case 0x22: seged=2; break;
			case 0x33: seged=3; break;
			case 0x55: seged=4; break;
		}
	}
	
	switch(seged)
	{
		case 1:	data_fogad[counter]=bejovo; 
				counter++;
				if (counter==8)
				{
					counter=0;
					seged=0;
				} break;
		case 2: data_fogad[counter]=bejovo;
				counter++;
				if (counter==9)
				{
					counter=0;
					seged=0;
				} break;
		case 3: data_fogad[counter]=bejovo;
				counter++;
				if (counter==3)
				{
					counter=0;
					seged=0;
				} break;
		case 4: data_fogad[counter]=bejovo;
				counter++;
				if (counter==8)
				{
					counter=0;
					seged=0;
				} break;
	}

	TIMSK|=0b00000010;
}

void RS_drivebus() 
{
	PORTC |= (1<<7);
	PORTE |= (1<<2);
}

void RS_releasebus() 
{
	PORTC &= ~(1<<7);
	PORTE &= ~(1<<2);
}

void RS_adatkuldes(char *adatok)
{
	UCSR1B&=~(1<<RXCIE1);
	
	RS_drivebus();
	while(!(UCSR1A&(1<<UDRE1))) {}
	UDR1 = 0x44;
	
	for(char i = 0; i < 2; i++) 
	{
		while(!(UCSR1A&(1<<UDRE1))) {}
		UDR1 = adatok[i];
	}
	
	while(!(UCSR1A&(1<<UDRE1))) {}
	UDR1 = 0xBB;
	while(!(UCSR1A&(1<<UDRE1))) {}
		
	_delay_ms(2);
	RS_releasebus();
	
	UCSR1B|=(1<<RXCIE1);
}

char* bejovo_adatok()
{
	return data_fogad;
}

