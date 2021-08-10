#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <inttypes.h>
#include "rs.h"
#include "UART.h"
#include "lcd_init.h"

extern uint8_t data_fogad;
void Init();
void kijelzes(char asd);

int main(void)
{  
    Init();
	char receive;
	char counter=0;
	char hely=0;
	LCD_goto(0, 0);

    while(1)
    {	
		LCD_data(data_fogad);
				
		receive=USART_Receive();
		RS_sendframe(receive);
		
		kijelzes(receive);
		
		if (++counter==4)
		{
			counter=0;
			if (++hely==4)
			{
				hely=0;
			}
			LCD_goto(hely, 0);
		}
    }
}

void Init()
{
	USART_Init(MYUBRR);
	RS_init();
	LCD_init(0, 2);
}

void kijelzes(char asd)
{
	if (((asd&0xF0)>>4)<10) LCD_data(((asd&0xF0)>>4)+48);
	else LCD_data(((asd&0xF0)>>4)+55);
	
	if ((asd&0x0F)<10) LCD_data((asd&0x0F)+48);
	else LCD_data((asd&0x0F)+55);
	
	LCD_data(' ');
}