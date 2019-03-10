/*******************************************************************************/
/**
Copyright (c) 2007 Freescale Semiconductor
Freescale Confidential Proprietary
\file       SPI_driver.c
\brief      Functions for basic SPI operation
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

#include "os_cpu.h"      

/*******************************************************************************/
/**
* \brief    SPI module initializing in Master mode @ 500kHz
* \author   Jaime Orozco
* \param    void
* \return   void
*/
void vfnSPI_Init(void)
{  
    SPI0CR1 = 0x04;     /* SPI module disabled */                    
    SPI0CR2 = 0x00;
    
    (void)SPI0SR;       /* Read the status register and */
    (void)SPI0DR;       /* read the data register to clear the status flags*/
                                                                               
    /* Desired SPI baud rate = 500kHz, baud rate divider = (20MHz/0.5MHz) = 40   */    
    SPI0BR_SPPR = 0x04;  /* Baud rate divider = (SPPR+1)*2^(SPR+1) = (5)*(2^3) */
    SPI0BR_SPR = 0x02;
    
    SPI0CR2_MODFEN = 1; /* Slave select output pin used by SPI module */  
    SPI0CR1_CPHA = 1;   /* Data sampling at falling edges */
    SPI0CR1_SSOE = 1;   /* Slave select output enable */
    SPI0CR1_MSTR = 1;   /* SPI in Master mode */
    SPI0CR1_SPE = 1;    /* SPI module enabled */
}

/*******************************************************************************/
/**
* \brief    Byte transmission through SPI
* \author   Jaime Orozco
* \param    u8DataByte: Byte to be sent
* \return   void
*/
void vfnSPI_Send_Byte(INT8U u8DataByte)
{  
    while (!SPI0SR_SPTEF)   /* Wait for empty data register */   
        ; 
    
    SPI0DR = u8DataByte;    /* Load byte in data transmission register */ 
}

/*******************************************************************************/
/**
* \brief    Frame transmission through SPI
* \author   Jaime Orozco
* \param    *paDataPointer: Pointer to the data array to be transmited,\n
            u16Size: Number of bytes to be sent
* \return   void
*/
void vfnSPI_Send_Frame(INT8U *paDataPointer, INT16U u16Size)
{
    /* Sent bytes counter */
    INT16U u16Counter = 0;   

        while (u16Counter < u16Size )  /* While there are data to be sent */   
        {
            vfnSPI_Send_Byte(*paDataPointer++);  /* send one byte */
            u16Counter++;       
        }                         
}

/*******************************************************************************/
/**
* \brief    Byte reception through SPI
* \author   Jaime Orozco
* \param    void
* \return   Byte received through SPI channel
*/

INT8U u8SPI_Receive_Byte(void)
{
    while ((SPI0SR & SPI0SR_SPIF_MASK))  /* Wait for data in the receive buffer */
        ;  
                        
    return SPI0DR;     /* Return received data */
}

/*******************************************************************************/
    

