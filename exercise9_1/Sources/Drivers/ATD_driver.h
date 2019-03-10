/*
 * ATD driver
 *
 * MH, 15-08-2011
 */

#ifndef _ATD_H
#define _ATD_H

#include "cpu.h" 
#include "ucos_ii.h"

#define PAD5 5
#define PAD6 6
#define PAD7 7
#define PAD8 8
#define PAD9 9
#define PAD10 10
#define PAD11 11
#define PAD12 12
#define PAD13 13
#define PAD14 14

/*
 * Return an unsigned 8-bit integer representing the analogue input
 * on the specified channel.
 */

INT8U ATDReadChannel(uint8_t channel);

/*
 * Return an unsigned 8-bit integer representing the processor temperature.
 */ 
 
INT8U ATDReadTemp();

#endif /* _ATD_H */