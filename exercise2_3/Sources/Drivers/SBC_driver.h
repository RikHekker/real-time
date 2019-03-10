/*******************************************************************************/
/**
Copyright (c) 2007 Freescale Semiconductor
Freescale Confidential Proprietary
\file       SBC_driver.h
\brief      Register definitions and function prototypes for SBC handling
\author     Freescale Semiconductor
\author     Guadalajara Applications Laboratory RTAC Americas
\author     Jaime Orozco
\version    1.0
\date       Feb/16/2007
*/
/*******************************************************************************/
/*                                                                             */
/* All software, source code, included documentation, and any implied know-how */
/* are property of Freescale Semiconductor and therefore considered            */ 
/* CONFIDENTIAL INFORMATION.                                                   */
/*                                                                             */
/* This confidential information is disclosed FOR DEMONSTRATION PURPOSES ONLY. */
/*                                                                             */
/* All Confidential Information remains the property of Freescale Semiconductor*/
/* and will not be copied or reproduced without the express written permission */
/* of the Discloser, except for copies that are absolutely necessary in order  */
/* to fulfill the Purpose.                                                     */
/*                                                                             */
/* Services performed by FREESCALE in this matter are performed AS IS and      */
/* without any warranty. CUSTOMER retains the final decision relative to the   */
/* total design and functionality of the end product.                          */
/*                                                                             */
/* FREESCALE neither guarantees nor will be held liable by CUSTOMER for the    */
/* success of this project.                                                    */
/*                                                                             */
/* FREESCALE disclaims all warranties, express, implied or statutory including,*/
/* but not limited to, implied warranty of merchantability or fitness for a    */
/* particular purpose on any hardware, software or advise supplied to the      */
/* project by FREESCALE, and or any product resulting from FREESCALE services. */
/*                                                                             */
/* In no event shall FREESCALE be liable for incidental or consequential       */
/* damages arising out of this agreement. CUSTOMER agrees to hold FREESCALE    */
/* harmless against any and all claims demands or actions by anyone on account */
/* of any damage,or injury, whether commercial, contractual, or tortuous,      */
/* rising directly or indirectly as a result of the advise or assistance       */
/* supplied CUSTOMER in connection with product, services or goods supplied    */
/* under this Agreement.                                                       */
/*                                                                             */
/*******************************************************************************/

#ifndef _SBC_H
#define _SBC_H

/** Variable types and common definitions */
#include "cpu.h" 

/* Default values for SBC Configuration: */

#define SBC_TIM1_INIT   SBC_WDOG_100MS
#define SBC_RCR_INIT    0
#define SBC_CAN_INIT    SBC_SLEWRATE0
#define SBC_IOR_INIT    0
#define SBC_WUR_INIT    SBC_L0L1HIGH|SBC_L2L3BOTH
#define SBC_TIM2_INIT   SBC_CYCLICSENSE_388MS
#define SBC_LPC_INIT    0
#define SBC_INTR_INIT   0 //SBC_CANF|SBC_VDDTEMP|SBC_INTVSUPLOW

/* Read/ Write bit */
#define SBC_R           0b00000000
#define SBC_W           0b00010000

/* SBC Register adresses */
#define SBC_MCR         0b00000000
#define SBC_RCR         0b00100000
#define SBC_CAN         0b01000000
#define SBC_IOR         0b01100000
#define SBC_WUR         0b10000000
#define SBC_TIM         0b10100000
#define SBC_LPC         0b11000000
#define SBC_INTR        0b11100000


/* MCR, Mode Control register */
	/* Write */
#define SBC_MCTR2       0b00000100
#define SBC_MCTR1       0b00000010
#define SBC_MCTR0       0b00000001
	/* Read  */
#define SBC_BATFAIL     0b00001000
#define SBC_VDDPRE      0b00000100
#define SBC_GFAIL       0b00000010
#define SBC_WDRST       0b00000001
	/* MCR Control Bits */
#define SBC_DEBUGMODE   0x00
#define SBC_NORMAL      SBC_MCTR0
#define SBC_STANDBY     SBC_MCTR1
#define SBC_STOP        SBC_MCTR1|SBC_MCTR0
#define SBC_SLEEP       SBC_MCTR2
#define SBC_DBGNORMAL   SBC_MCTR2|SBC_MCTR0
#define SBC_DBGSTANDBY  SBC_MCTR2|SBC_MCTR1
#define SBC_DBGSTOP     SBC_MCTR2|SBC_MCTR1|SBC_MCTR0


  /* RCR, Reset Control Register */    
#define SBC_WDSTOP      0b00001000
#define SBC_NOSTOP      0b00000100
#define SBC_CANSLEEP    0b00000010
#define SBC_RSTTH       0b00000001


/* CAN Register */
/* Write */
#define CANCLR          0b00001000
#define SBC_SC1         0b00000100
#define SBC_SC0         0b00000010
#define SBC_MODE        0b00000001
	/* Read */
#define SBC_CANWU       0b00001000
#define SBC_CAN_F       0b00000100
#define SBC_CAN_UF      0b00000010
#define SBC_THERM_CUR   0b00000001
/* High Speed Can Transceiver Modes */
#define SBC_SLEWRATE0           0x00
#define SBC_SLEWRATE1           SBC_SC0
#define SBC_SLEWRATE2           SBC_SC1
#define SBC_SLEWRATE3           SBC_SC1|SBC_SC0
#define SBC_SLEEP_WKPDISABLE    SBC_MODE|SBC_SC0  
#define SBC_SLEEP_WKPENABLE     SBC_MODE


/* IOR, Input/Output Register */
/* Write */
#define SBC_HSON        0b00000100
	/* Read */
#define SBC_V2LOW       0b00001000
#define SBC_HSOT        0b00000100
#define SBC_VSUPLOW     0b00000010
#define SBC_DEBUG       0b00000001


/* WUR, Wake Up Register */
/* Write */
#define SBC_LCTR3       0b00001000
#define SBC_LCTR2       0b00000100
#define SBC_LCTR1       0b00000010
#define SBC_LCTR0       0b00000001
/* Read */
#define SBC_L3WU        0b00001000
#define SBC_L2WU        0b00000100
#define SBC_L1WU        0b00000010
#define SBC_L0WU        0b00000001
/* Control Bits */
#define SBC_L0L1DISABLED    0x00
#define SBC_L0L1HIGH        SBC_LCTR0
#define SBC_L0L1LOW         SBC_LCTR1
#define SBC_L0L1BOTH        SBC_LCTR1|SBC_LCTR0
#define SBC_L2L3DISABLED    0x00
#define SBC_L2L3HIGH        SBC_LCTR2
#define SBC_L2L3LOW         SBC_LCTR3
#define SBC_L2L3BOTH        SBC_LCTR3|SBC_LCTR2


/* TIM1/2, Timing Registers */
#define SBC_TIM1        0b00000000
#define SBC_TIM2        0b00001000
/* TIM1 Write */
#define SBC_WDW         0b00000100
#define SBC_WDT1        0b00000010
#define SBC_WDT0        0b00000001
/* TIM2 Write */
#define SBC_CSP2        0b00000100
#define SBC_CSP1        0b00000010
#define SBC_CSP0        0b00000001
/* TIM1/TIM2 Read */
#define SBC_CANL2VDD    0b00001000
#define SBC_CANL2BAT    0b00000100
#define SBC_CANL2GND    0b00000010
#define SBC_TXPD        0b00000001
/* Watchdog Periods */
#define SBC_WDOG_10MS           0x00
#define SBC_WDOG_45MS           SBC_WDT0
#define SBC_WDOG_100MS          SBC_WDT1
#define SBC_WDOG_350MS          SBC_WDT1|SBC_WDT0
#define SBC_WDOG_WDW_10MS       SBC_WDW
#define SBC_WDOG_WDW_45MS       SBC_WDW|SBC_WDT0
#define SBC_WDOG_WDW_100MS      SBC_WDW|SBC_WDT1
#define SBC_WDOG_WDW_350MS      SBC_WDW|SBC_WDT1|SBC_WDT0
/* Cyclic Sense Timings */
#define SBC_CYCLICSENSE_5MS     0x00
#define SBC_CYCLICSENSE_9MS     SBC_CSP0
#define SBC_CYCLICSENSE_18MS    SBC_CSP1
#define SBC_CYCLICSENSE_37MS    SBC_CSP1|SBC_CSP0
#define SBC_CYCLICSENSE_74MS    SBC_CSP2
#define SBC_CYCLICSENSE_95MS    SBC_CSP2|SBC_CSP0
#define SBC_CYCLICSENSE_191MS   SBC_CSP2|SBC_CSP1
#define SBC_CYCLICSENSE_388MS   SBC_CSP2|SBC_CSP1|SBC_CSP0


/* LPC, Low Power Control Register */
/* Write */
#define SBC_LX2HS       0b00001000
#define SBC_FWU         0b00000100
#define SBC_CAN_INT     0b00000010
#define SBC_HSAUTO	    0b00000001
/* Read */
#define SBC_CANH2VDD    0b00001000
#define SBC_CANH2BAT    0b00000100
#define SBC_CANH2GND    0b00000010
#define SBC_RXPR        0b00000001


/* INTR, Interrupt Register */
/* Write */
#define SBC_INTVSUPLOW  0b00001000
#define SBC_HSOT_V2LOW  0b00000100
#define SBC_VDDTEMP     0b00000010
#define SBC_CANF        0b00000001
/* Read */
#define SBC_HSOT        0b00000100



/** Function Prototypes */

/** Write SBC register */
void vfnSBC_Write (INT8U u8TXByte);

/** Read SBC register */
INT8U u8SBC_Read (INT8U u8TXByte);

/** Sets the SBC in standby mode with default configuration values */
INT8U u8SBC_StandbyMode (void);

/** Sets the SBC in Debug-Standby mode with default configuration values */
void vfnSBC_DebugMode (void);

/** Resets the SBC watchdog  */
void vfnSBC_ClearWatchdog(void);


#endif /* _SBC_H */

/*******************************************************************************/
