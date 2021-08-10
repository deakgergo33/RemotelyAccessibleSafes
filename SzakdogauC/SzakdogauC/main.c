#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include "billmatrix.h"
#include "lcd_init.h"
#include "UART.h"
#include "rs.h"

void init();

void menu();
	void felhasznalo_belepes();
		void felhasznalo_fiok(char szekrenyszam);
			void felhasznalo_nyitas(char szekrenyszam);
			void felhasznalo_zaras(char szekrenyszam);
			void felhasznalo_kodmodosit(char szekrenyszam);
	
	void admin_belepes();
		void admin_fiok();
			void admin_zarolas();
			void admin_zarolasfeloldas();
			void admin_felszabaditas();
	
	char szekrenyszam_ellenorzes(char szekrenyszam);
	char kod_ellenorzes(char szekrenyszam, long int kod);
	void uzenet(char sorszam);
	void elozmeny_kuldes(char szekrenyszam, char muvelet, char siker);
	char ujelozmeny[3]={0,0,0};
	
	long int szamkeres(char sor, char hely, char szamjegy, char jelszo);
	void pergesgatlas();
	
unsigned long int adatbazis [4][4];
unsigned char EEPROM_read(unsigned int uiAddress);
void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
void adatbazis_init();
void eeprom_iras(long int adat, char sor, char szekrenyszam);

const unsigned long int admin_kod=99999999;
char admin_zar=0;
char gomb_seged=0;

char data_fogad[9]={0};
void kijelzes(char asd);

int main()
{
	adatbazis_init();
	init();
	
	while(1)
	{
		menu();	
	}
}

ISR(TIMER0_COMP_vect)
{
	static int szamlalo[5]={0, 0, 0, 0, 0};
		
	for (char i=0; i<4; i++)
	{
		if ((adatbazis[(int)i][3]>=1)&&(adatbazis[(int)i][3]<4)) szamlalo[(int)i]++;
		else szamlalo[(int)i]=0;	//ha admin felszabadítaná a szekrenyt, megszunik a zarolas
		if (szamlalo[(int)i]==7200)
		{
			adatbazis[(int)i][3]--;
			eeprom_iras(adatbazis[(int)i][3], 3, i+1);
			szamlalo[(int)i]=0;
		}
	}
	
	if (admin_zar>=1) szamlalo[4]++;
	if (szamlalo[4]==7200)
	{
		admin_zar--;
		szamlalo[4]=0;
	}
	
	char data[2];
	if(((data_fogad[0]==0x55)&&(data_fogad[1]==1)&&(data_fogad[7]==0xAA))||((data_fogad[0]==0x33)&&(data_fogad[1]==1)&&(data_fogad[2]==0xCC))||((data_fogad[0]==0x11)&&(data_fogad[1]==1)&&(data_fogad[7]==0xEE))||((data_fogad[0]==0x22)&&(data_fogad[1]==1)&&(data_fogad[8]==0xDD)))	//keret+cím
	{
		switch(data_fogad[0])	//3 vagy 8 vagy 9 byte hosszú üzenet
		{
			case 0x33:		//master lekérdezi, hogy van-e új elõzményüzenet
				if ((ujelozmeny[0]!=0)||(ujelozmeny[1]!=0)||(ujelozmeny[2]!=0))
				{
					data[0]=ujelozmeny[1]|(ujelozmeny[2]<<3);
					data[1]=ujelozmeny[0];
				}
				else
				{
					data[0]=0;
					data[1]=0;
				}
				RS_adatkuldes(data);
				elozmeny_kuldes(0, 0, 0); 
				break;
			case 0x11:		//nyitás, zárás, zárolás, zárolás feloldás, felszabadítás kezdeményezése távolról
				if((data_fogad[3]<5)&&(data_fogad[3]>0))		//szekrenyszam ellenorzes
				{
					if ((adatbazis[data_fogad[3]-1][3]<3)||(adatbazis[data_fogad[3]-1][3]==5))
					{
						switch(data_fogad[2])
						{
							case 0: if ((((unsigned long int)data_fogad[4])+(((unsigned long int)data_fogad[5])<<8)+(((unsigned long int)data_fogad[6])<<16))==adatbazis[data_fogad[3]-1][1])
									{
										adatbazis[data_fogad[3]-1][3]=0;	//nyitás
										eeprom_iras(0, 3, data_fogad[3]);
										adatbazis[data_fogad[3]-1][2]=0;
										eeprom_iras(0, 2, data_fogad[3]);
										
										switch(data_fogad[3])
										{
											case 1: PORTD|=8<<4; PORTA=0; break;
											case 2: PORTD|=4<<4; break;
											case 3: PORTD|=2<<4; break;
											case 4: PORTD|=1<<4; break;
										}
										
										data[0]=(1<<7);
										data[1]=data_fogad[3];
										RS_adatkuldes(data);
									}
									else if (adatbazis[data_fogad[3]-1][3]==5)		//felszabadított hibaüzenet
									{
										data[0]=(3<<3)|(1<<7);
										data[1]=data_fogad[3];
										RS_adatkuldes(data);
									}
									else	//hibás kód
									{
										adatbazis[data_fogad[3]-1][3]++;
										eeprom_iras(adatbazis[data_fogad[3]-1][3], 3, data_fogad[3]);
										
										data[0]=(1<<3)|(1<<7);
										data[1]=data_fogad[3];
										RS_adatkuldes(data);
									}
									break;
							case 1: if ((((unsigned long int)data_fogad[4])+(((unsigned long int)data_fogad[5])<<8)+(((unsigned long int)data_fogad[6])<<16))==adatbazis[data_fogad[3]-1][1])
									{
										adatbazis[data_fogad[3]-1][3]=0;	//zárás
										eeprom_iras(0, 3, data_fogad[3]);
										adatbazis[data_fogad[3]-1][2]=1;
										eeprom_iras(1, 2, data_fogad[3]);
										
										switch(data_fogad[3])
										{
											case 1: PORTD&=7<<4; PORTA=0xFF; break;
											case 2: PORTD&=11<<4; break;
											case 3: PORTD&=13<<4; break;
											case 4: PORTD&=14<<4; break;
										}
								
										data[0]=1|(1<<7);
										data[1]=data_fogad[3];
										RS_adatkuldes(data);
									}
									else if (adatbazis[data_fogad[3]-1][3]==5)		//felszabadított hibaüzenet
									{
										data[0]=(3<<3)|1|(1<<7);
										data[1]=data_fogad[3];
										RS_adatkuldes(data);
									}
									else	//hibás kód
									{
										adatbazis[data_fogad[3]-1][3]++;
										eeprom_iras(adatbazis[data_fogad[3]-1][3], 3, data_fogad[3]);
								
										data[0]=(1<<3)|1|(1<<7);
										data[1]=data_fogad[3];
										RS_adatkuldes(data);
									}
									break;
							case 2: if (adatbazis[data_fogad[3]-1][3]==5)	//felszabaditott ertekadas
									{
										adatbazis[data_fogad[3]-1][1]=(((unsigned long int)data_fogad[4])+(((unsigned long int)data_fogad[5])<<8)+(((unsigned long int)data_fogad[6])<<16));
										eeprom_iras(adatbazis[data_fogad[3]-1][1], 1, data_fogad[3]);
										adatbazis[data_fogad[3]-1][3]=0;
										eeprom_iras(0, 3, data_fogad[3]);
										
										data[0]=2|(1<<7);
										data[1]=data_fogad[3];
										RS_adatkuldes(data);
									}
									else	//zárolt hibaüzenet
									{
										data[0]=(2<<3)|(1<<7);
										data[1]=data_fogad[3];
										RS_adatkuldes(data);
									}
									break;
						}
					}
					else	//zárolt hibaüzenet
					{
						data[0]=(2<<3)|data_fogad[2]|(1<<7);
						data[1]=data_fogad[3];
						RS_adatkuldes(data);
					}
				}
				else
				{
					data[0]=(1<<3)|data_fogad[2]|(1<<7);
					data[1]=data_fogad[3];
					RS_adatkuldes(data);
				}
				break;
			case 0x22:		//Kód módosítás
				if((data_fogad[2]<5)&&(data_fogad[2]>0))		//szekrenyszam
				{
					if (adatbazis[data_fogad[2]-1][3]<3)
					{
						if ((((unsigned long int)data_fogad[3])+(((unsigned long int)data_fogad[4])<<8)+(((unsigned long int)data_fogad[5]&0b00001111)<<16))==adatbazis[data_fogad[2]-1][1])
						{
							adatbazis[data_fogad[2]-1][1]=(((unsigned long int)data_fogad[5]&0b11110000)>>4)+(((unsigned long int)data_fogad[6])<<4)+(((unsigned long int)data_fogad[7])<<12);
							eeprom_iras(adatbazis[data_fogad[2]-1][1], 1, data_fogad[2]);
							
							data[0]=(1<<3)|3|(1<<7);
							data[1]=data_fogad[2];
							RS_adatkuldes(data);
						}
						else
						{
							adatbazis[data_fogad[2]-1][3]++;
							eeprom_iras(adatbazis[data_fogad[2]-1][3], 3, data_fogad[2]);
							
							data[0]=(1<<3)|3|(1<<7);
							data[1]=data_fogad[2];
							RS_adatkuldes(data);
						}
					}
					else if (adatbazis[data_fogad[2]-1][3]==5)		//felszabadított hibaüzenet
					{
						data[0]=(3<<3)|3|(1<<7);
						data[1]=data_fogad[2];
						RS_adatkuldes(data);
					}
					else	//zárolt hibaüzenet
					{
						data[0]=(2<<3)|3|(1<<7);
						data[1]=data_fogad[2];
						RS_adatkuldes(data);
					}	
				}
				else
				{
					data[0]=(1<<3)|3|(1<<7);
					data[1]=data_fogad[2];
					RS_adatkuldes(data);
				}
				break;
			case 0x55:											//ADMIN
				if((data_fogad[3]<5)&&(data_fogad[3]>0))		//szekrenyszam
				{
					switch(data_fogad[2]&0b111)
					{
						case 4: if ((((unsigned long int)data_fogad[4])+(((unsigned long int)data_fogad[5])<<8)+(((unsigned long int)data_fogad[6])<<16)+(((unsigned long int)data_fogad[2]>>3)<<24))==admin_kod)
								{
									adatbazis[data_fogad[3]-1][3]=4;
									eeprom_iras(4, 3, data_fogad[3]);
									
									data[0]=4|(1<<7);
									data[1]=data_fogad[3];
									RS_adatkuldes(data);
								}
								else
								{
									data[0]=(1<<3)|4|(1<<7);
									data[1]=data_fogad[3];
									RS_adatkuldes(data);
								}
								break;
						case 5: if ((((unsigned long int)data_fogad[4])+(((unsigned long int)data_fogad[5])<<8)+(((unsigned long int)data_fogad[6])<<16)+(((unsigned long int)data_fogad[2]>>3)<<24))==admin_kod)
								{
									adatbazis[data_fogad[3]-1][3]=0;
									eeprom_iras(0, 3, data_fogad[3]);
							
									data[0]=5|(1<<7);
									data[1]=data_fogad[3];
									RS_adatkuldes(data);
								}
								else
								{
									data[0]=(1<<3)|5|(1<<7);
									data[1]=data_fogad[3];
									RS_adatkuldes(data);
								}
								break;
						case 6: if ((((unsigned long int)data_fogad[4])+(((unsigned long int)data_fogad[5])<<8)+(((unsigned long int)data_fogad[6])<<16)+(((unsigned long int)data_fogad[2]>>3)<<24))==admin_kod)
								{
									adatbazis[data_fogad[3]-1][3]=5;
									eeprom_iras(5, 3, data_fogad[3]);
									
									data[0]=6|(1<<7);
									data[1]=data_fogad[3];
									RS_adatkuldes(data);
								}
								else
								{
									data[0]=(1<<3)|6|(1<<7);
									data[1]=data_fogad[3];
									RS_adatkuldes(data);
								}
								break;
					}
				}
				else
				{
					data[0]=(1<<3)|data_fogad[2]|(1<<7);
					data[1]=data_fogad[3];
					RS_adatkuldes(data);
				}
				break;
		}
		for (char x=0; x<9; x++)
		{
			data_fogad[x]=0;
		}
	}
}

void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE))
	;
	/* Set up address and data registers */
	EEAR = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMWE */
	EECR |= (1<<EEMWE);
	/* Start eeprom write by setting EEWE */
	EECR |= (1<<EEWE);
}

unsigned char EEPROM_read(unsigned int uiAddress)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEWE))
	;
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from data register */
	return EEDR;
}

void adatbazis_init()
{
	for (char i=0; i<4; i++)
	{
		adatbazis[(int)i][0]=i+1;
		adatbazis[(int)i][1]=((long int)EEPROM_read(i*4)<<24)+((long int)EEPROM_read(i*4+1)<<16)+((long int)EEPROM_read(i*4+2)<<8)+(long int)EEPROM_read(i*4+3);
		adatbazis[(int)i][2]=(long int)EEPROM_read(i+100);
		adatbazis[(int)i][3]=(long int)EEPROM_read(i+200);
	}
}

void init()
{
	LCD_init(0, 2);	//LCD
	USART_Init(MYUBRR);
	RS_init();
	DDRC|=0x78;		//billmatrix
	DDRG=0;			//gombok
	DDRD|=0xF0;		//ledek
	DDRA=0xFF;		//zár vezérlõjel
	PORTA=0xFF;
	PORTD=0;
	TCCR0|=0b00001111;	//1024 elooszto
	OCR0=194;			//Compare kb. 0.025s megszakitas, 194
	TIMSK|=0b00000010;
	sei();
}

void menu()
{
	while(1)
	{
		LCD_goto(0, 0); LCD_Puts("Belepes         ");
		LCD_goto(1, 0); LCD_Puts("                ");
		LCD_goto(2, 0); LCD_Puts("G1: Felhasznalo ");
		LCD_goto(3, 0); LCD_Puts("G2: Admin       ");
		
		if ((PING==0b10000)&&(gomb_seged==0)) {gomb_seged=1; felhasznalo_belepes();}
		if ((PING==0b01000)&&(gomb_seged==0)) {gomb_seged=1; admin_belepes();}
		if (PING==0) {gomb_seged=0;}
	}
}

void felhasznalo_belepes()
{	
	LCD_goto(0, 0); LCD_Puts(">Szekrenyszam:  ");
	LCD_goto(1, 0); LCD_Puts(" Kod:           ");
	LCD_goto(2, 0); LCD_Puts("                ");
	LCD_goto(3, 0); LCD_Puts("                ");
	
	signed char szekrenyszam=0;
	char ellenorzes=2;
	
	while (1)
	{
		szekrenyszam=(signed char)szamkeres(0, 15, 1, 0);	//szekrenyszam ellenorzes
		if(szekrenyszam==-1) return;
		
		ellenorzes=szekrenyszam_ellenorzes(szekrenyszam);
		if(ellenorzes==1) uzenet(0);
		if(ellenorzes==0)
		{
			long int kod=0;
			
			while (1)
			{
				LCD_goto(0, 0); LCD_data(' ');
				
				ellenorzes=2;
				
				if(adatbazis[szekrenyszam-1][3]==5)	//felszabadított szekrény kód adás
				{
					LCD_goto(1, 0); LCD_Puts(">Uj kod:        ");
					kod=szamkeres(1, 9, 6, 0);
					if(kod==-1) return;
					adatbazis[szekrenyszam-1][1]=kod;
					eeprom_iras(kod, 1, szekrenyszam);
					adatbazis[szekrenyszam-1][3]=0;
					eeprom_iras(0, 3, szekrenyszam);
					uzenet(1);
					while(1)
					{
						if ((PING==0b00001)&&(gomb_seged==0)) {gomb_seged=1; return;}
						if (PING==0) gomb_seged=0;
					}
				}
				
				LCD_goto(1, 0); LCD_Puts(">Kod:           ");
				
				while((adatbazis[szekrenyszam-1][3]==3)||(adatbazis[szekrenyszam-1][3]==4))	//túl sok próbálkozás vagy admin zár
				{
					uzenet(2);
					if ((PING==0b00001)&&(gomb_seged==0)) {gomb_seged=1; return;}
					if (PING==0) gomb_seged=0;
				}
				
				kod=szamkeres(1, 6, 6, 1);	//kód ellenõrzés
				if(kod==-1) return;
				ellenorzes=kod_ellenorzes(szekrenyszam, kod);
				if(ellenorzes==1)
				{
					uzenet(4);
					kod=0;
					adatbazis[szekrenyszam-1][3]++;
					eeprom_iras(adatbazis[szekrenyszam-1][3], 3, szekrenyszam);
				}
				if(ellenorzes==0)
				{
					adatbazis[szekrenyszam-1][3]=0;
					eeprom_iras(0, 3, szekrenyszam);
					
					felhasznalo_fiok(szekrenyszam);
					return;
				}
			}
		}
		
		if ((PING==0b00001)&&(gomb_seged==0)) {gomb_seged=1; return;}
		if (PING==0) gomb_seged=0;
	}
}

void felhasznalo_fiok(char szekrenyszam)
{
	elozmeny_kuldes(szekrenyszam, 6, 1);
	
	while(1)
	{
		LCD_goto(0, 0); LCD_Puts("G1: Nyitas      ");
		LCD_goto(1, 0); LCD_Puts("G2: Zaras       ");
		LCD_goto(2, 0); LCD_Puts("G3: Kod modosit ");
		LCD_goto(3, 0); LCD_Puts("G5: Kilepes     ");
		
		if (gomb_seged==0)
		{
			switch(PING)
			{
				case 0b10000: gomb_seged=1; felhasznalo_nyitas(szekrenyszam); break;
				case 0b01000: gomb_seged=1; felhasznalo_zaras(szekrenyszam); break;
				case 0b00100: gomb_seged=1; felhasznalo_kodmodosit(szekrenyszam); break;
				case 0b00001: elozmeny_kuldes(szekrenyszam, 7, 1); return;
			}
		}
		if (PING==0) gomb_seged=0;
	}
}

void felhasznalo_nyitas(char szekrenyszam)
{
	adatbazis[szekrenyszam-1][2]=0;
	eeprom_iras(0, 2, szekrenyszam);
	
	switch(szekrenyszam)
	{
		case 1: PORTD|=8<<4; PORTA=0; break;
		case 2: PORTD|=4<<4; break;
		case 3: PORTD|=2<<4; break;
		case 4: PORTD|=1<<4; break;
	}
	
	elozmeny_kuldes(szekrenyszam, 0, 1);
	
	uzenet(11);
	uzenet(5);
	while(1)
	{
		if ((PING==0b00001)&&(gomb_seged==0)) {gomb_seged=1; return;}
		if (PING==0) gomb_seged=0;
	}
}

void felhasznalo_zaras(char szekrenyszam)
{
	adatbazis[szekrenyszam-1][2]=1;
	eeprom_iras(1, 2, szekrenyszam);
	
	switch(szekrenyszam)
	{
		case 1: PORTD&=7<<4; PORTA=0xFF; break;
		case 2: PORTD&=11<<4; break;
		case 3: PORTD&=13<<4; break;
		case 4: PORTD&=14<<4; break;
	}
	
	elozmeny_kuldes(szekrenyszam, 1, 1);
	
	uzenet(11);
	uzenet(6);
	while(1)
	{
		if ((PING==0b00001)&&(gomb_seged==0)) {gomb_seged=1; return;}
		if (PING==0) gomb_seged=0;
	}
}

void felhasznalo_kodmodosit(char szekrenyszam)
{
	LCD_goto(0, 0); LCD_Puts("Uj kod:         ");
	LCD_goto(1, 0); LCD_Puts("                ");
	LCD_goto(2, 0); LCD_Puts("                ");
	LCD_goto(3, 0); LCD_Puts("                ");
	
	long int uj_kod=0;
	
	uj_kod=szamkeres(0, 8, 6, 0);
	if(uj_kod==-1) return;
	adatbazis[szekrenyszam-1][1]=uj_kod;
	eeprom_iras(uj_kod, 1, szekrenyszam);
	
	elozmeny_kuldes(szekrenyszam, 2, 1);
	
	uzenet(7);
	while(1)
	{
		if ((PING==0b00001)&&(gomb_seged==0)) {gomb_seged=1; return;}
		if (PING==0) gomb_seged=0;
	}
}

void admin_belepes()
{
	LCD_goto(0, 0); LCD_Puts("Kod:            ");
	LCD_goto(1, 0); LCD_Puts("                ");
	LCD_goto(2, 0); LCD_Puts("                ");
	LCD_goto(3, 0); LCD_Puts("                ");
	
	long int kod=0;
	
	while(1)
	{
		while(admin_zar==3)
		{
			uzenet(3);
			if ((PING==0b00001)&&(gomb_seged==0)) {gomb_seged=1; return;}
			if (PING==0) gomb_seged=0;
		}
		
		kod=szamkeres(0, 5, 8, 1);
		
		if (kod==-1) return;
		
		if (kod==admin_kod)
		{
			admin_zar=0;
			admin_fiok();
			return;
		}
		else
		{
			elozmeny_kuldes(0, 6, 0);
			uzenet(0);
			admin_zar++;
		}
	}
}

void admin_fiok()
{
	elozmeny_kuldes(0, 6, 1);
	
	while(1)
	{
		LCD_goto(0, 0); LCD_Puts("G1: Zarolas     ");
		LCD_goto(1, 0); LCD_Puts("G2: Feloldas    ");
		LCD_goto(2, 0); LCD_Puts("G3: Felszabadit ");
		LCD_goto(3, 0); LCD_Puts("G5: Kilepes     ");
		
		if (gomb_seged==0)
		{
			switch(PING)
			{
				case 0b10000: gomb_seged=1; admin_zarolas(); break;
				case 0b01000: gomb_seged=1; admin_zarolasfeloldas(); break;
				case 0b00100: gomb_seged=1; admin_felszabaditas(); break;
				case 0b00001: elozmeny_kuldes(0, 7, 1); return;
			}
		}
		if (PING==0) gomb_seged=0;
	}
}

void admin_zarolas()
{
	LCD_goto(0, 0); LCD_Puts("Szekrenyszam:   ");
	LCD_goto(1, 0); LCD_Puts("                ");
	LCD_goto(2, 0); LCD_Puts("                ");
	LCD_goto(3, 0); LCD_Puts("                ");
	
	signed char szekrenyszam=0;
	char ellenorzes=2;
	
	while(1)
	{
		szekrenyszam=(signed char)szamkeres(0, 14, 1, 0);
		if (szekrenyszam==-1) return;
			
		ellenorzes=szekrenyszam_ellenorzes(szekrenyszam);
		if(ellenorzes==1) uzenet(0);
		if(ellenorzes==0)
		{
			adatbazis[szekrenyszam-1][3]=4;
			eeprom_iras(4, 3, szekrenyszam);
			
			elozmeny_kuldes(szekrenyszam, 3, 1);
			
			uzenet(8);
			while(1)
			{
				if ((PING==0b00001)&&(gomb_seged==0)) {gomb_seged=1; return;}
				if (PING==0) gomb_seged=0;
			}
		}
	}
}

void admin_zarolasfeloldas()
{
	LCD_goto(0, 0); LCD_Puts("Szekrenyszam:   ");
	LCD_goto(1, 0); LCD_Puts("                ");
	LCD_goto(2, 0); LCD_Puts("                ");
	LCD_goto(3, 0); LCD_Puts("                ");
	
	signed char szekrenyszam=0;
	char ellenorzes=2;
	
	while(1)
	{
		szekrenyszam=(signed char)szamkeres(0, 14, 1, 0);
		if (szekrenyszam==-1) return;
		
		ellenorzes=szekrenyszam_ellenorzes(szekrenyszam);
		if(ellenorzes==1) uzenet(0);
		if(ellenorzes==0)
		{
			adatbazis[szekrenyszam-1][3]=0;
			eeprom_iras(0, 3, szekrenyszam);
			
			elozmeny_kuldes(szekrenyszam, 4, 1);
			
			uzenet(9);
			while(1)
			{
				if ((PING==0b00001)&&(gomb_seged==0)) {gomb_seged=1; return;}
				if (PING==0) gomb_seged=0;
			}
		}
	}
}

void admin_felszabaditas()
{
	LCD_goto(0, 0); LCD_Puts("Szekrenyszam:   ");
	LCD_goto(1, 0); LCD_Puts("                ");
	LCD_goto(2, 0); LCD_Puts("                ");
	LCD_goto(3, 0); LCD_Puts("                ");
	
	signed char szekrenyszam=0;
	char ellenorzes=2;
	
	while(1)
	{
		szekrenyszam=(signed char)szamkeres(0, 14, 1, 0);
		if (szekrenyszam==-1) return;
		
		ellenorzes=szekrenyszam_ellenorzes(szekrenyszam);
		if(ellenorzes==1) uzenet(0);
		if(ellenorzes==0)
		{
			adatbazis[szekrenyszam-1][3]=5;
			eeprom_iras(5, 3, szekrenyszam);
			
			elozmeny_kuldes(szekrenyszam, 5, 1);
			
			uzenet(10);
			while(1)
			{
				if ((PING==0b00001)&&(gomb_seged==0)) {gomb_seged=1; return;}
				if (PING==0) gomb_seged=0;
			}
		}
	}
}

char szekrenyszam_ellenorzes(char szekrenyszam)
{
	if ((szekrenyszam>0)&&(szekrenyszam<5))
	{
		return 0;
	}
	return 1;
}

char kod_ellenorzes(char szekrenyszam, long int kod)
{
	if (adatbazis[szekrenyszam-1][1]==kod) 
	{
		elozmeny_kuldes(szekrenyszam, 6, 1);
		return 0;
	}
	elozmeny_kuldes(szekrenyszam, 6, 0);
	return 1;
}

void elozmeny_kuldes(char szekrenyszam, char muvelet, char siker)
{
	ujelozmeny[0]=szekrenyszam;
	ujelozmeny[1]=muvelet;
	ujelozmeny[2]=siker;
}

void pergesgatlas()
{
	while(1)
	{
		if((billmatrix(0)==12)&&(billmatrix(1)==12)&&(billmatrix(2)==12)&&(billmatrix(3)==12)) return;
	}
}

long int szamkeres(char sor, char hely, char szamjegy, char jelszo)
{
	char szamlalo=0;
	long int szam=0;
	long int tizhatvany=1;
	for (char i=1; i<szamjegy; i++)
	{
		tizhatvany*=10;
	}
	
	for (char i=0; i<szamjegy; i++)
	{
		LCD_goto(sor, i+hely); LCD_data(' ');
	}
	
	while(1)
	{
		for (char i=0; i<4; i++)
		{
			if (billmatrix(i)<10)
			{
				szam=(szam%tizhatvany)*10+billmatrix(i);
				pergesgatlas();
				LCD_goto(3, 0); LCD_Puts("                ");
				szamlalo++;
			}
			if (billmatrix(i)==10)
			{
				szam=0;
				pergesgatlas();
				LCD_goto(3, 0); LCD_Puts("                ");
				szamlalo=0;
				for (char i=0; i<szamjegy; i++)
				{
					LCD_goto(sor, i+hely); LCD_data(' ');
				}
			}
			
			if (jelszo==0)
			{
				long int kiiras_seged=tizhatvany;
				for (char i=0; i<szamjegy; i++)
				{
					LCD_goto(sor, i+hely); LCD_data((szam/kiiras_seged)%10+48);
					kiiras_seged/=10;
				}
			}
			else
			{
				for (char i=0; (i<szamlalo)&&(i<szamjegy); i++)
				{
					LCD_goto(sor, hely+szamjegy-i-1); LCD_data('*');
				}
			}
			
			if (billmatrix(i)==11)
			{
				pergesgatlas();
				return szam;
			}
		}
		
		if ((PING==0b00001)&&(gomb_seged==0)) {gomb_seged=1; return -1;}
		if (PING==0) gomb_seged=0;
	}
}

void uzenet(char sorszam)
{
	switch(sorszam)
	{
		case 0: LCD_goto(3, 0); LCD_Puts("Hibas szam!     "); break;
		case 1: LCD_goto(3, 0); LCD_Puts("Kod elfogadva!  "); break;
		case 2: LCD_goto(3, 0); LCD_Puts("Zarolt szekreny!"); break;
		case 3: LCD_goto(3, 0); LCD_Puts("Az admin zarolt!"); break;
		case 4: LCD_goto(3, 0); LCD_Puts("Hibas kod!      "); break;
		case 5: LCD_goto(3, 0); LCD_Puts("Szekreny nyitva!"); break;
		case 6: LCD_goto(3, 0); LCD_Puts("Szekreny zarva! "); break;
		case 7: LCD_goto(3, 0); LCD_Puts("Kod modositva!  "); break;
		case 8: LCD_goto(3, 0); LCD_Puts("Sikeres zarolas!"); break;
		case 9: LCD_goto(3, 0); LCD_Puts("Zarolas torolve!"); break;
		case 10: LCD_goto(3, 0); LCD_Puts("Felszabaditva!  "); break;
		case 11: LCD_goto(0, 0); LCD_Puts("                ");
				 LCD_goto(1, 0); LCD_Puts("                ");
				 LCD_goto(2, 0); LCD_Puts("                "); break;
	}
}

void eeprom_iras(long int adat, char sor, char szekrenyszam)
{
	switch(sor)
	{
		case 1: EEPROM_write((szekrenyszam-1)*4, (char)(adat>>24));
				EEPROM_write((szekrenyszam-1)*4+1, (char)(adat>>16));
				EEPROM_write((szekrenyszam-1)*4+2, (char)(adat>>8));
				EEPROM_write((szekrenyszam-1)*4+3, (char)adat); break;
		case 2:	EEPROM_write(100+szekrenyszam-1, (char)adat); break;
		case 3:	EEPROM_write(200+szekrenyszam-1, (char)adat); break;
	}
}