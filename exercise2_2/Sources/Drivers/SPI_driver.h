/*******************************************************************************/
/**
Copyright (c) 2007 Freescale Semiconductor
Freescale Confidential Proprietary
\file       SPI_driver.h
\brief      Function prototypes for SPI port basic tasks
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

#ifndef _SPI_H
#define _SPI_H

#include "cpu.h"

/** Function Prototypes */

/** SPI module initializing in Master mode @ 2.5MHz */
void vfnSPI_Init(void);

/** Byte transmission */
void vfnSPI_Send_Byte(INT8U u8DataByte);

/** Frame transmission */
void vfnSPI_Send_Frame(INT8U *paDataPointer, INT16U u16Size);

/** Byte reception */
INT8U u8SPI_Receive_Byte(void);

#endif /* _SPI_H */

/*******************************************************************************/