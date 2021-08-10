#define F_CPU 8000000UL
#include <util/delay.h>
#include <inttypes.h>
#include <avr/io.h>
#include "lcd_init.h"

void LCD_init(uint8_t e_bl_cur, uint8_t nodl_cf)
{
	DDRE|=0xF0;  //data7...4 out    DDRE utols� 4 bitj�t kimenetre �ll�tja
	DDRF|=(1<<LCD_RS)|(1<<LCD_RW)|(1<<LCD_EN);  //RS, R/W, EN out    DDRF 1,2,3 bitj�t kimenetre �ll�tja
	PORTF&=~(1<<LCD_RW);  //PORTF 2. bitj�t 0-ba rakja   Write data
	PORTF&=~(1<<LCD_RS);  //PORTF 1. bitj�be 0-�t rak
	PORTE=0x20;  //0x20 parancs: Function set -Sets interface data length    24.oldal hitachi, alap be�ll�t�sok

	ENABLE  //delay
	ENABLE  //__| |__  EN bit 0-1 v�ltakoz�sa tal�n az el�z� parancs biztos be�llta miatt
	ENABLE  //4 bites �zemm�d, 8x5pixel, 2soros//�zemm�d v�laszt�s//nem tudjuk, hogy bekapcsol�s ut�n �ppen milyenben van
		
	switch(nodl_cf)
	{
		case 0: LCD_cmd(0x20);
				LCD_cmd(0x20);
				LCD_cmd(0x20); break; // number of display lines: 1 line  character font 5x8
		case 1: LCD_cmd(0x24);
				LCD_cmd(0x24);
				LCD_cmd(0x24); break; // number of display lines: 1 line  character font 5x10
		case 2:	LCD_cmd(0x28);
				LCD_cmd(0x28);
				LCD_cmd(0x28); break; // number of display lines: 2 line  character font 5x8  ezt haszn�ld
		case 3: LCD_cmd(0x2C);
				LCD_cmd(0x2C);
				LCD_cmd(0x2C); break; // number of display lines: 2 line  character font 5x10 ugyan az
	}
	
	LCD_cmd(0x02);
	LCD_cmd(0x01);
	
	switch(e_bl_cur)
	{
		case 0: LCD_cmd(0x08 | (1<<LCD_E) | (0<<LCD_CUR) | (0<<LCD_BL)); break; //kurzor villog ki, al�h�z�s ki, LCD be 0x08
		case 1: LCD_cmd(0x08 | (1<<LCD_E) | (0<<LCD_CUR) | (1<<LCD_BL)); break; //kurzor villog ki, al�h�z�s be, LCD be 0x10
		case 2: LCD_cmd(0x08 | (1<<LCD_E) | (1<<LCD_CUR) | (0<<LCD_BL)); break; //kurzor villog be, al�h�z�s ki, LCD be 0x12
		case 3: LCD_cmd(0x08 | (1<<LCD_E) | (1<<LCD_CUR) | (1<<LCD_BL)); break; //kurzor villog be, al�h�z�s be, LCD be 0x14
	}
}

void LCD_busy(void)
{													//BF olvas�sa
	uint8_t busy;
	DDRE&=~(1<<PE7);  //DDRE 7. bitj�be 0-�t rak? bemenet    ott olvassuk majd a BF-et (D7-PE7)
	PORTF&=~(1<<LCD_RS);  //St�tusz inf�   PORTF 1. bitje 0
	PORTF|=(1<<LCD_RW);  //olvas�s    PORTF 2. bitje 1

	do
	{
		busy=0;
		PORTF|=(1<<LCD_EN);  //EN<-1 PORTF 3. bitje 1
		_delay_us(1);  //felfut�
		busy=(PINE&(1<<PE7));  //�tadjuk a BF �rt�k�t  PINE 0x80-ba
		PORTF&=~(1<<LCD_EN);  //EN<-0 PORTF 3. bit 0,1,0
		_delay_us(1);
		PORTF|=(1<<LCD_EN);  //EN<-1
		_delay_us(1);
		PORTF&=~(1<<LCD_EN);  //EN<-0
		_delay_us(1);
	}
	while(busy);  //ha busy 0 akk kil�p
	
	PORTF&=~(1<<LCD_RW);  //R/W<-0 write
	DDRE|=(1<<PE7);  //PE7<-1
}

void LCD_cmd(uint8_t parancs)
{
	LCD_busy();  //Megv�rjuk m�g felszabadul
	PORTF&=~(1<<LCD_RS);  //Parancs  PORTF 1,2,3 bitje 0-ba
	PORTF&=~(1<<LCD_RW);  //K�ld�s
	PORTF&=~(1<<LCD_EN);  //EN<-0
	PORTE&=~(0xF0);  //PORTE fels� 4 bitje 0-ba
	PORTE|=(parancs&0xF0);  //fels� 4 bit k�ld�se  PORTE  cmd fels� 4 bitj�nek a kirak�sa a fels� 4 bitre
	ENABLE
	PORTE&=~(0xF0);  // PORTE 0-z�s
	PORTE|=((parancs<<4)&0xF0);  //als� 4 bit k�ld�se cmd als� kik�ld�s
	ENABLE
}

void LCD_data(uint8_t data)
{
	LCD_busy();  //Megv�rjuk m�g felszabadul
	PORTF|=(1<<LCD_RS);  //Adatregiszter
	PORTF&=~(1<<LCD_RW);  //�r�s
	PORTF&=~(1<<LCD_EN);  //EN<-0
	PORTE&=~(0xF0);
	PORTE|=(data&0xF0);  //4 fels� bit kit�tele
	ENABLE
	PORTE&=~(0xF0);
	PORTE|=((data<<4)&0xF0);  //als� 4 bit kit�tele
	ENABLE
}

void LCD_Puts(char *adat)
{
	while(*adat)
	{
		LCD_data(*adat);
		adat++;
	}
}

void LCD_goto(unsigned char sor, unsigned char hely)
{
	if(sor>=2)
	{
		sor=sor-2;
		hely=hely+16;
	}
	LCD_cmd((1<<7)|(sor<<6)|hely);
}


