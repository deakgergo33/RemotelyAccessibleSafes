#include <avr/io.h>

#ifndef UART_H_
#define UART_H_

#define FOSC 8000000// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

unsigned char USART_Receive( void );
void USART_Init( unsigned int ubrr );
void USART_Transmit( unsigned char data );

#endif /* UART_H_ */