#ifndef H_CPU
#define H_CPU

#pragma CODE_SEG DEFAULT

#include  "app_cfg.h"

#pragma MESSAGE DISABLE C1420
#pragma MESSAGE DISABLE C2705
#pragma MESSAGE DISABLE C5660
#pragma MESSAGE DISABLE C5703
#pragma MESSAGE DISABLE C5905
#pragma MESSAGE DISABLE C12053
#pragma MESSAGE DISABLE C12056

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;
typedef signed   char  INT8S;
typedef unsigned int   INT16U;
typedef signed   int   INT16S;
typedef unsigned long  INT32U;
typedef signed   long  INT32S;
typedef float          FP32;
typedef double         FP64;

typedef unsigned char  uint8_t;
typedef signed   char  int8_t;
typedef unsigned int   uint16_t;
typedef signed   int   int16_t;
typedef unsigned long  uint32_t;
typedef signed   long  int32;

#include <derivative.h>

uint16_t IntDisable(void);
void RestoreSW(uint16_t sw);

void CPUInitSystem(void);
void CPUStartTimer(void);

#define CPUResetTimer() CRGFLG_RTIF = 1;

#define _DINT() { asm sei; }
#define _EINT() { asm cli; }

#endif

