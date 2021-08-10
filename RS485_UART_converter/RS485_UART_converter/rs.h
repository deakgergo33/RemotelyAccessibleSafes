//rs.h
#ifndef __RS485_H_
#define __RS485_H_

#include <inttypes.h>
#include <avr/io.h>

extern void RS_init();

extern void RS_drivebus();
extern void RS_releasebus();

extern void RS_sendframe(char data);

#endif
