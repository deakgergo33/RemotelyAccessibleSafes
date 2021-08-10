#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include "billmatrix.h"

unsigned char billmatrix(unsigned char row)
{
	const unsigned char billtomb[12]={69, 14, 13, 11, 22, 21, 19, 38, 37, 35, 70, 67};
	unsigned char billgomb, bill, num;
	switch(row)
	{
		case 0: PORTC=(PORTC&0x80)+0x08; break;
		case 1: PORTC=(PORTC&0x80)+0x10; break;
		case 2: PORTC=(PORTC&0x80)+0x20; break;
		case 3: PORTC=(PORTC&0x80)+0x40; break;
	}
	
	_delay_us(20);
	bill=PINC&0x7F;
	for (num=0; num<12; num++)
	{
		if (bill==billtomb[num])
		{
			billgomb=num;
			return billgomb;
		}
	}
	return 12;
}