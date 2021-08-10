// rs.c
#include "rs.h"
#include <avr/interrupt.h>
#include <string.h>
#include "UART.h"
#define F_CPU 8000000UL
#include <util/delay.h>
#include "lcd_init.h"

uint8_t data_fogad;

void RS_init() {
	
	DDRB |= 0xF0;
	DDRD |= 0b11111000;
	DDRD &= 0b11111011;
	
	DDRC |= (1<<7);
	DDRE |= (1<<2);
	
	sei();
}

ISR(USART1_RX_vect)
{	
	USART_Transmit(UDR1);
	data_fogad=UDR1;
	
	
	
	//if (((data_fogad&0xF0)>>4)<10) LCD_data(((data_fogad)>>4)+48);
	//else LCD_data(((data_fogad)>>4)+55);
	//
	//if ((data_fogad&0x0F)<10) LCD_data((data_fogad)+48);
	//else LCD_data((data_fogad&0x0F)+55);
	//
	//LCD_data(' ');
}

ISR(USART1_TX_vect) {
	UCSR1B &= ~(1<<TXCIE1);
	RS_releasebus();
}

void RS_drivebus() {
	PORTC |= (1<<7);
	PORTE |= (1<<2);
}

void RS_releasebus() {
	PORTC &= ~(1<<7);
	PORTE &= ~(1<<2);
}

void RS_sendframe(char data) 
{
	RS_drivebus();
	
	UDR1 = data;
	while(!(UCSR1A&(1<<UDRE1))) {}
		
	UCSR1B |= (1<<TXCIE1);
}
