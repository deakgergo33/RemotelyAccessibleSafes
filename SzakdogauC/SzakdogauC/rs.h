//rs.h
#ifndef __RS485_H_
#define __RS485_H_

#include <inttypes.h>
#include <avr/io.h>

struct RSframe	{
	uint8_t start;
	uint8_t data0;
	uint8_t data1;
	uint8_t stop;
};

extern char data_fogad[9];

extern void RS_init();

extern void RS_drivebus();
extern void RS_releasebus();

extern void RS_adatkuldes(char *adatok);
char* bejovo_adatok();

#endif
