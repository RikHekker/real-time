#ifndef H_CPU
#define H_CPU

typedef unsigned char  uint8_t;
typedef signed   char  int8_t;
typedef unsigned int   uint16_t;
typedef signed   int   int16_t;
typedef unsigned long  uint32_t;
typedef signed   long  int32;

#pragma CODE_SEG DEFAULT

#include <derivative.h>

void CPUInitSystem(void);
void CPUStartTimer(void);

#define CPUResetTimer() CRGFLG_RTIF = 1;

/*
 * Functions for entering and exiting critical sections.
 */

uint16_t IntDisable(void);
void RestoreSW(uint16_t sw);

#endif

