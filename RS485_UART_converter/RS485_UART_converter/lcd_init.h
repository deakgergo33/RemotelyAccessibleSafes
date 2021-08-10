/*LCD kezelés

Kurzor, aláhúzás, lcd be/ki beállítások  (X) /// (Y) lcd sor beállítások:    Main-be!!!
	LCD_init(X, Y)   X=0 kurzor villog ki, aláhúzás ki
					 X=1 kurzor villog ki, aláhúzás be
					 X=2 kurzor villog be, aláhúzás ki
					 X=3 kurzor villog be, aláhúzás be
					 X>=4 LCD ki
					 
					 Y=0 number of display lines: 1 line  character font 5x8
					 Y=1 number of display lines: 1 line  character font 5x10
					 Y=2 number of display lines: 2 line  character font 5x8  ezt használd
					 Y=3 number of display lines: 2 line  character font 5x10 ugyan az

--------------------------------------------------------------------------------------------
					 
Cursor helyezés: LCD_goto(unsigned char sor, unsigned char hely)

Szöveg kiírás: LCD_Puts(char *adat)  szöveg kiírása " "

Változó kiírás: LDC_data(char data)  változó kiírása  1 ASCII kód!

*/


#ifndef LCD_INIT_H_
#define LCD_INIT_H_

	#define LCD_RS (PF1)
	#define LCD_RW (PF2)
	#define LCD_EN (PF3)

	#define ENABLE {PORTF|=(1<<LCD_EN); _delay_us(2); PORTF&=~(1<<LCD_EN); _delay_us(2);}

	#define LCD_E 2  //enable
	#define LCD_CUR 0  //cursor
	#define LCD_BL 1  //blink
	
	void LCD_init(uint8_t e_bl_cur, uint8_t nodl_cf);
	void LCD_busy(void);
	void LCD_cmd(uint8_t parancs);
	void LCD_data(uint8_t data);
	void LCD_Puts(char *adat);
	void LCD_goto(unsigned char sor, unsigned char hely);
	
#endif /* LCD_INIT_H_ */

