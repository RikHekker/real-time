#include "ATD_driver.h"

/*
  Some notes on the ATD converter:

  Sampling time is calculated as follows:
    
    (36 + 2*f(SMP)) * (PRS + 1) cycles,
  
  where SMP are the highest 3 bits and PRS are the lowest 5 bits of ATDCTL4,
  and f(SMP) is given in the data sheet (see Table 24.13).
  Default sampling time is set to ATDCTL4 = #$05, i.e. 
   
    (36 + 8) * (5 + 1) = 264 cycles.

  The ATDCTL2 register allows to set the ATD Fast Flag Clear All bit, which will clear
  the ATD flag whenever the result is read from a register. This, however, gives rise to 
  an exception 96 (?) after the register was read if multiple conversions are performed.
  An alternative approach is to clear the flag manually by setting the ATDSTAT0_SCF bit
  as soon as the conversion was completed. 

  The A/D resolution is set by writing to the ATDCTL1 register. E.g.:
  
  MOVB #$10,ATDCTL1 will set A/D resolution to 8 bit 
  MOVB #$50,ATDCTL1 will set A/D resolution to 12 bit 
*/

int ATDReadChannel(uint8_t channel) {
  int value = 0;
  asm {
    MOVB #$50,ATDCTL1 /* set A/D resolution to 12 bit */
    MOVB #$80,ATDCTL3 /* set conversion sequence length to 16 */ 
    MOVB #$FF,ATDCTL4 /* set sampling time to 2688 cycles */
    MOVB channel,ATDCTL5 /* set which channel to read */
    SCF: BRCLR ATDSTAT0,#$80,SCF /* wait for the reading */
    BSET ATDSTAT0, #$80 /* clear the ATD result flag */  
    LDX ATDDR0 /* load the reading from the result register ATDDR0, */
    STX value  /* since conversion sequence length = 1 */
  }
  return value;
}

int ATDReadTemp() {
  int value = 0;
  asm {
    MOVB #$18,VREGHTCL
    MOVB #$0F,ATDCTL1 /* set A/D resolution to 8bit */
    MOVB #$C0,ATDCTL3
    MOVB #$41,ATDCTL5
    SCF: BRCLR ATDSTAT0,#$80,SCF
    BSET ATDSTAT0, #$80
    LDX ATDDR0
    STX value
  }
  return value;
}
