#include <ucos_ii.h>

#include "cpu.h"
#include "SPI_Driver.h"
#include "SBC_Driver.h"

void CPUInitPLL(void){
  /* PLL initialization */
  
  /* Bus Clock = OSCCLK * (SYNR+1) / ((REFDV+1)(2*POSTDIV))    */
  /*     20MHz =  4MHz  * (4+1) / (0+1)                        */
  PLLCTL_PLLON = 0;       /* Disable the PLL */
  SYNR_SYNDIV = 0x04;     /* Set PLL synthesizer register to 4 */
  REFDV_REFDIV = 0x00;    /* Set PLL divider register to 0 */

  /* Fvco = 2*OSCCLK*(SYNR+1)/(REFDV+1) = 8MHz* 5/1 = 40MHz   */
  /*                                    -> VCOFRQ = 00b        */        
  SYNR_VCOFRQ1 = 0;
  SYNR_VCOFRQ0 = 0;
      
  /* Fref = OSCCLK/(REFDV+1) = 4MHz/1 = 4MHz -> REFRQ= 01b     */    
  REFDV_REFFRQ1 = 0;
  REFDV_REFFRQ0 = 1;

  PLLCTL_PLLON = 1;   /* PLL enabled */

  while(!CRGFLG_LOCK)
    ;       /* Wait until the PLL is within the desired frequency */
              
  CLKSEL_PLLSEL = 1;    /* Select clock source from PLL */       
}

void CPUInitMMU(void){
  MMCCTL1 = 1;    /* Enables Flash in the memory map */                  
  DIRECT = 0;     /* Default location for direct page */              
  IVBR = 0xFF;    /* Default vector base address */    
}

void CPUInitCOP(void){
  /* COP module configuration */
  COPCTL = 0x00;              /* COP disable */ 
}

void CPUInitPorts(void){
  /* Unused ports are configured as outputs set to zero */
  PORTA = 0;      /* Port A */
  DDRA = 0xFF;    
  PORTB = 0;      /* Port B */
  DDRB = 0xFF;    
  PORTC = 0;      /* Port C */
  DDRC = 0xFF;    
  PORTD = 0;      /* Port D */
  DDRC = 0xFF;
  PORTK = 0;      /* Port K  */
  DDRK= 0xFF;     
  RDRIV = 0xFF;   /* Reduced drive on ports A, B, C, D, E, K. */        
  
  
  /* Port E as input with pull-ups enabled */
  DDRE = 0;       
  PUCR_PUPEE = 1;            
      
      
  /* Port T low nibble used for general purpose LEDs D26 to D29 */     
  PTT = 0x0F;         /* Initial output state */     
  RDRT = 0xFF;        /* Reduced drive */
  DDRT = 0x7F;        /* PT7 as input, remaining pins are outputs */      
  PPST_PPST7 = 0;     /* Pull-up enabled for input pin PT7 */
  PERT_PERT7 = 1; 
  
  
  /* Port S outputs cleared, the signals assigned to SPI0 */
  /* are configured when enabling the SPI0 module */
  PTS = 0;        
  DDRS = 0xFF;    
  RDRS = 0xFF;  
       
       
  /* Unused ports are configured as outputs set to zero and reduced drive */    
  PTP = 0;        /* Port P */
  DDRP = 0xFF;
  RDRP = 0xFF; 
  
  
  /* Port H outputs 3 and 7 are used to enable FlexRay Transceivers TJA1080 */ 
  /* the other pins are configured by the FlexRay module */               
  DDRH_DDRH3 = 1;    
  DDRH_DDRH7 = 1;    
  RDRH = 0xFF;     /* Reduced drive */        
  PERH = 0;        /* Pull devices disabled */
  PTH = 0x88;      /* FlexRay Transceivers enabled */
  
  
  /* Port J inputs 5 and 6 receive signals from FlexRay transceivers */
  PTJ = 0;
  DDRJ_DDRJ5 = 0;    
  DDRJ_DDRJ6 = 0;     
  PERJ = 0;        /* Pull devices disabled */ 
    
                      
  /* Port AD0 unused, outputs set to zero */      
  PT0AD = 0;           
  RDR0AD = 0xFF;  
  DDR0AD = 0xFF;   
  
  
  /* PortAD1 used for general purpose LEDs D22 to D25 */ 
  PT1AD = 0x0F;   /* Initial output state */ 
  RDR1AD = 0xFF;  /* Reduced drive */
  DDR1AD = 0xFF;  /* Port AD as output */ 

}

void CPUInitInterrupts(void){
  IRQCR_IRQEN = 0;      /* Ignore IRQ interrupt from SBC */

  INT_CFADDR = 0xF0;    /* Select interrupts from 0xFFF0 to 0xFFFF */
  INT_CFDATA0 = 7;      /* Real Time Interrupt, priority 7 */
  INT_CFDATA2 = 1;      /* !XIRQ interrupt, priority 1 */
  INT_CFDATA3 = 6;      /* SWI interrupt, priority 6 */
}

void CPUInitRTI(void){
  RTICTL = 0x90;        /* RTI Decimal divider = 4x10^3; RTI period = 4MHz/2k = 2kHz */
}

/* Initialize hardware, copied from example application */
void CPUInitSystem(void){
  CPUInitCOP();
  CPUInitMMU();
  CPUInitPLL();
  CPUInitPorts();
  CPUInitRTI();
  CPUInitInterrupts();

  vfnSPI_Init();
  vfnSBC_DebugMode();

  // Clear S,X and I flags
  asm andcc #$2f+(1-CPU_ENABLE_XIRQ)*0x40;
}

uint16_t IntDisable(void)
{
  uint16_t sw;

  // sw = CCR
  asm {
    pshcw
    sei
    movw 2,sp+,sw
  }
  return (sw);
}

void RestoreSW(uint16_t sw)
{
  // CCR = sw
  asm {
    movw sw,2,-sp
    pulcw
  }
}  

/* Setup uC/OS-II timer */
void CPUStartTimer(void){
#if OS_CRITICAL_METHOD == 3u
  OS_CPU_SR cpu_sr = 0u;
#endif

  OS_ENTER_CRITICAL();
  CRGFLG_RTIF = 1;      /* Clear RTI flag */
  CRGINT_RTIE = 1;      /* Enable RTI */
  OS_EXIT_CRITICAL();

#if OS_TASK_STAT_EN > 0u
  OSStatInit();
#endif
}

#if CPU_ENABLE_ATD > 0u || CPU_CATCH_ALL_INTERRUPTS > 0u
void interrupt VectorNumber_Vatd CPUIntAtdHandler(void){
}
void interrupt VectorNumber_Vatdcompare CPUIntAtdcompareHandler(void){
}
#endif

#if CPU_ENABLE_XIRQ > 0u || CPU_CATCH_ALL_INTERRUPTS > 0u
/* Handle !XIRQ interrupt, connected to SW2 */
void interrupt VectorNumber_Vxirq CPUIntXirqHandler(void) {
}
#endif

#if CPU_CATCH_ALL_INTERRUPTS > 0u
void interrupt VectorNumber_Vsi CPUIntSiHandler(void){
}

void interrupt VectorNumber_Vsyscall CPUIntSyscallHandler(void){
}

void interrupt VectorNumber_Vmpuaccesserr CPUIntMpuaccesserrHandler(void){
}

void interrupt VectorNumber_Vxsei CPUIntXseiHandler(void){
}

void interrupt VectorNumber_VReserved115 CPUIntReserved115Handler(void){
}

void interrupt VectorNumber_VReserved114 CPUIntReserved114Handler(void){
}

void interrupt VectorNumber_VReserved113 CPUIntReserved113Handler(void){
}

void interrupt VectorNumber_VReserved112 CPUIntReserved112Handler(void){
}

void interrupt VectorNumber_VReserved111 CPUIntReserved111Handler(void){
}

void interrupt VectorNumber_VReserved110 CPUIntReserved110Handler(void){
}

void interrupt VectorNumber_VReserved109 CPUIntReserved109Handler(void){
}

void interrupt VectorNumber_VReserved108 CPUIntReserved108Handler(void){
}

void interrupt VectorNumber_VReserved107 CPUIntReserved107Handler(void){
}

void interrupt VectorNumber_VReserved106 CPUIntReserved106Handler(void){
}

void interrupt VectorNumber_VReserved105 CPUIntReserved105Handler(void){
}

void interrupt VectorNumber_VReserved104 CPUIntReserved104Handler(void){
}

void interrupt VectorNumber_VReserved103 CPUIntReserved103Handler(void){
}

void interrupt VectorNumber_VReserved102 CPUIntReserved102Handler(void){
}

void interrupt VectorNumber_VReserved101 CPUIntReserved101Handler(void){
}

void interrupt VectorNumber_VReserved100 CPUIntReserved100Handler(void){
}

void interrupt VectorNumber_VReserved99 CPUIntReserved99Handler(void){
}

void interrupt VectorNumber_VReserved98 CPUIntReserved98Handler(void){
}

void interrupt VectorNumber_Vatdcompare CPUIntAtdcompareHandler(void){
}

void interrupt VectorNumber_VReserved96 CPUIntReserved96Handler(void){
}

void interrupt VectorNumber_VReserved95 CPUIntReserved95Handler(void){
}

void interrupt VectorNumber_VReserved94 CPUIntReserved94Handler(void){
}

void interrupt VectorNumber_VReserved93 CPUIntReserved93Handler(void){
}

void interrupt VectorNumber_VReserved92 CPUIntReserved92Handler(void){
}

void interrupt VectorNumber_VReserved91 CPUIntReserved91Handler(void){
}

void interrupt VectorNumber_VReserved90 CPUIntReserved90Handler(void){
}

void interrupt VectorNumber_VReserved89 CPUIntReserved89Handler(void){
}

void interrupt VectorNumber_VReserved88 CPUIntReserved88Handler(void){
}

void interrupt VectorNumber_VReserved87 CPUIntReserved87Handler(void){
}

void interrupt VectorNumber_VReserved86 CPUIntReserved86Handler(void){
}

void interrupt VectorNumber_VReserved85 CPUIntReserved85Handler(void){
}

void interrupt VectorNumber_Vintrig CPUIntIntrigHandler(void){
}

void interrupt VectorNumber_Vpit7 CPUIntPit7Handler(void){
}

void interrupt VectorNumber_Vpit6 CPUIntPit6Handler(void){
}

void interrupt VectorNumber_Vpit5 CPUIntPit5Handler(void){
}

void interrupt VectorNumber_Vpit4 CPUIntPit4Handler(void){
}

void interrupt VectorNumber_VReserved79 CPUIntReserved79Handler(void){
}

void interrupt VectorNumber_VReserved78 CPUIntReserved78Handler(void){
}

void interrupt VectorNumber_Vxst7 CPUIntXst7Handler(void){
}

void interrupt VectorNumber_Vxst6 CPUIntXst6Handler(void){
}

void interrupt VectorNumber_Vxst5 CPUIntXst5Handler(void){
}

void interrupt VectorNumber_Vxst4 CPUIntXst4Handler(void){
}

void interrupt VectorNumber_Vxst3 CPUIntXst3Handler(void){
}

void interrupt VectorNumber_Vxst2 CPUIntXst2Handler(void){
}

void interrupt VectorNumber_Vxst1 CPUIntXst1Handler(void){
}

void interrupt VectorNumber_Vxst0 CPUIntXst0Handler(void){
}

void interrupt VectorNumber_Vpit3 CPUIntPit3Handler(void){
}

void interrupt VectorNumber_Vpit2 CPUIntPit2Handler(void){
}

void interrupt VectorNumber_Vpit1 CPUIntPit1Handler(void){
}

void interrupt VectorNumber_Vpit0 CPUIntPit0Handler(void){
}

void interrupt VectorNumber_Vhti CPUIntHtiHandler(void){
}

void interrupt VectorNumber_Vapi CPUIntApiHandler(void){
}

void interrupt VectorNumber_Vlvi CPUIntLviHandler(void){
}

void interrupt VectorNumber_VReserved62 CPUIntReserved62Handler(void){
}

void interrupt VectorNumber_VReserved61 CPUIntReserved61Handler(void){
}

void interrupt VectorNumber_VReserved60 CPUIntReserved60Handler(void){
}

void interrupt VectorNumber_VReserved59 CPUIntReserved59Handler(void){
}

void interrupt VectorNumber_VReserved58 CPUIntReserved58Handler(void){
}

void interrupt VectorNumber_Vpmffault3 CPUIntPmffault3Handler(void){
}

void interrupt VectorNumber_Vpmffault2 CPUIntPmffault2Handler(void){
}

void interrupt VectorNumber_Vpmffault1 CPUIntPmffault1Handler(void){
}

void interrupt VectorNumber_Vpmffault0 CPUIntPmffault0Handler(void){
}

void interrupt VectorNumber_Vpmfgcr CPUIntPmfgcrHandler(void){
}

void interrupt VectorNumber_Vpmfgbr CPUIntPmfgbrHandler(void){
}

void interrupt VectorNumber_Vpmfgar CPUIntPmfgarHandler(void){
}

void interrupt VectorNumber_Vfrprot CPUIntFrprotHandler(void){
}

void interrupt VectorNumber_Vfrchi CPUIntFrchiHandler(void){
}

void interrupt VectorNumber_Vfrwakeup CPUIntFrwakeupHandler(void){
}

void interrupt VectorNumber_Vfrrxfifob CPUIntFrrxfifobHandler(void){
}

void interrupt VectorNumber_Vfrrxfifoa CPUIntFrrxfifoaHandler(void){
}

void interrupt VectorNumber_Vfrrxmessbuff CPUIntFrrxmessbuffHandler(void){
}

void interrupt VectorNumber_Vfrtxmessbuff CPUIntFrtxmessbuffHandler(void){
}

void interrupt VectorNumber_VReserved43 CPUIntReserved43Handler(void){
}

void interrupt VectorNumber_VReserved42 CPUIntReserved42Handler(void){
}

void interrupt VectorNumber_VReserved41 CPUIntReserved41Handler(void){
}

void interrupt VectorNumber_VReserved40 CPUIntReserved40Handler(void){
}

void interrupt VectorNumber_Vcan0tx CPUIntCan0txHandler(void){
}

void interrupt VectorNumber_Vcan0rx CPUIntCan0rxHandler(void){
}

void interrupt VectorNumber_Vcan0err CPUIntCan0errHandler(void){
}

void interrupt VectorNumber_Vcan0wkup CPUIntCan0wkupHandler(void){
}

void interrupt VectorNumber_Vflash CPUIntFlashHandler(void){
}

void interrupt VectorNumber_Vflashfd CPUIntFlashfdHandler(void){
}

void interrupt VectorNumber_VReserved33 CPUIntReserved33Handler(void){
}

void interrupt VectorNumber_Vspi1 CPUIntSpi1Handler(void){
}

void interrupt VectorNumber_VReserved31 CPUIntReserved31Handler(void){
}

void interrupt VectorNumber_Vcrgipllcol CPUIntCrgipllcolHandler(void){
}

void interrupt VectorNumber_Vcrgscm CPUIntCrgscmHandler(void){
}

void interrupt VectorNumber_Vcrgplllck CPUIntCrgplllckHandler(void){
}

void interrupt VectorNumber_Vtimpabovf CPUIntTimpabovfHandler(void){
}

void interrupt VectorNumber_Vtimmdcu CPUIntTimmdcuHandler(void){
}

void interrupt VectorNumber_VReserved25 CPUIntReserved25Handler(void){
}

void interrupt VectorNumber_VReserved24 CPUIntReserved24Handler(void){
}

void interrupt VectorNumber_VReserved22 CPUIntReserved22Handler(void){
}

void interrupt VectorNumber_Vsci1 CPUIntSci1Handler(void){
}

void interrupt VectorNumber_Vsci0 CPUIntSci0Handler(void){
}

void interrupt VectorNumber_Vspi0 CPUIntSpi0Handler(void){
}

void interrupt VectorNumber_Vtimpaie CPUIntTimpaieHandler(void){
}

void interrupt VectorNumber_Vtimpaaovf CPUIntTimpaaovfHandler(void){
}

void interrupt VectorNumber_Vtimovf CPUIntTimovfHandler(void){
}

void interrupt VectorNumber_Vtimch7 CPUIntTimch7Handler(void){
}

void interrupt VectorNumber_Vtimch6 CPUIntTimch6Handler(void){
}

void interrupt VectorNumber_Vtimch5 CPUIntTimch5Handler(void){
}

void interrupt VectorNumber_Vtimch4 CPUIntTimch4Handler(void){
}

void interrupt VectorNumber_Vtimch3 CPUIntTimch3Handler(void){
}

void interrupt VectorNumber_Vtimch2 CPUIntTimch2Handler(void){
}

void interrupt VectorNumber_Vtimch1 CPUIntTimch1Handler(void){
}

void interrupt VectorNumber_Vtimch0 CPUIntTimch0Handler(void){
}

//void interrupt VectorNumber_Vrti CPUIntRtiHandler(void){
//}

void interrupt VectorNumber_Virq CPUIntIrqHandler(void){
}

//void interrupt VectorNumber_Vswi CPUIntSwiHandler(void){
//}

void interrupt VectorNumber_Vtrap CPUIntTrapHandler(void){
}

void interrupt VectorNumber_Vcop CPUIntCopHandler(void){
}

void interrupt VectorNumber_Vclkmon CPUIntClkmonHandler(void){
}

#endif
