/*******************************************************************************/
/**
Copyright (c) 2007 Freescale Semiconductor
Freescale Confidential Proprietary
\file       SBC_driver.c
\brief      Functions for basic SBC (MC33742) handling 
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

#include "cpu.h"

/** SPI definitions */
#include "SPI_driver.h"

/** SBC definitions */
#include "SBC_driver.h"

/*******************************************************************************/

/** Constant table for SBC Initialization (default values from SBC_driver.h) */
#pragma CONST_SEG INIT_TABLES
static const INT8U gaSBCInitTable[]={
    SBC_TIM|SBC_TIM1    |   SBC_TIM1_INIT,
    SBC_RCR             |   SBC_RCR_INIT, 
    SBC_CAN             |   SBC_CAN_INIT,
    SBC_IOR             |   SBC_IOR_INIT,   
    SBC_WUR             |   SBC_WUR_INIT,    
    SBC_TIM|SBC_TIM2    |   SBC_TIM2_INIT,    
    SBC_LPC             |   SBC_LPC_INIT,
    SBC_INTR            |   SBC_INTR_INIT
};  
#pragma CONST_SEG DEFAULT 

/*******************************************************************************/
/**
* \brief    Write SBC register 
* \author   Jaime Orozco
* \param    u8TXByte: Register to be accessed + data
* \return   void
*/
void vfnSBC_Write (INT8U u8TXByte)
{
    u8TXByte |= SBC_W;      /* Force write command */
    
    vfnSPI_Send_Byte(u8TXByte);  /* Send data to SBC */
}

/*******************************************************************************/
/**
* \brief    Read SBC register 
* \author   Jaime Orozco
* \param    u8TXByte: Register to be accessed
* \return   Data byte returned by SBC
*/
 
INT8U u8SBC_Read (INT8U u8RegByte)
{

    
    /* Variable to store received data */
    INT8U u8Temp=0;
    
    uint16_t sw = IntDisable();
        
    vfnSPI_Send_Byte(u8RegByte);    /* Send read command */
    
    u8Temp=u8SPI_Receive_Byte();    /* Read data*/
    
    RestoreSW(sw);
      
    return u8Temp;  /* Return data received from SBC */
}


/*******************************************************************************/
/**
* \brief    Sets the SBC in Standby mode with default values from SBC_driver.h
* \author   Jaime Orozco
* \param    void
* \return   1 if communication error found, 0 if initialization was successful
*/
INT8U u8SBC_StandbyMode(void)
{   
    /* Data counter */ 
    INT8U u8Counter;
            
    /* Storage for received data */
    INT8U u8Temp;

    vfnSBC_Write(SBC_TIM|SBC_TIM1_INIT);    /* Write SBC Watchdog (TIM1) */
    vfnSBC_Write(SBC_MCR|SBC_STANDBY);      /* Send standby mode command */
    vfnSBC_Write(SBC_RCR|0x0F);         /* Read command for Reset Control Register */
    
    u8Temp = u8SBC_Read(SBC_RCR);       /* Read received value */
    
        if ((u8Temp&0xF0) != 0x30 && (u8Temp&0xF0) != 0x70)
        {
            return 1;
        }
        
        
        for (u8Counter = 1; 
            u8Counter < sizeof(gaSBCInitTable) / sizeof(gaSBCInitTable[0]);
            u8Counter ++)
        {   
            /* Write configuration values */
            vfnSBC_Write(gaSBCInitTable[u8Counter]); 
        }
    
    return 0;
}

/*******************************************************************************/
/**
* \brief    Sets the SBC in Debug-Normal mode with default configuration values
* \author   Jaime Orozco
* \param    void
* \return   void
*/
void vfnSBC_DebugMode (void)
{
    /* Data counter */ 
    INT8U u8Counter;

    vfnSBC_Write(SBC_TIM|SBC_TIM1_INIT);    /* Write Watchdog (TIM1) */
    vfnSBC_Write(SBC_MCR|SBC_DEBUGMODE);    /* Send Debug mode command */
    vfnSBC_Write(SBC_MCR|SBC_DBGNORMAL);    /* Send Debug-Normal mode command */
    
        for (u8Counter = 1; 
         u8Counter < sizeof(gaSBCInitTable) / sizeof(gaSBCInitTable[0]);
         u8Counter ++)
        {   
            /* Write configuration values */
            vfnSBC_Write(gaSBCInitTable[u8Counter]);    
        }
}

/*******************************************************************************/
/**
* \brief    Resets the SBC watchdog 
* \author   Jaime Orozco
* \param    void
* \return   void
*/
void vfnSBC_ClearWatchdog(void)
{
    /* Watchdog cleared by writing the TIM1 Register */  
    vfnSBC_Write(SBC_TIM|SBC_TIM1_INIT); 
}
    

/*******************************************************************************/
	