#include <stdint.h>
#include "PLL.h"

void pll_init(void){
	// configure the system to use RCC2 for 400 MHz PLL and non-integer System CLK DIVIDER
	SYSCTL->RCC2 |= 0x80000000; 
	// BYPASS2 (Bit 11) set, no system CLK divider
	SYSCTL->RCC2 |= 0x00000800; // bypass PLL  while intializing 
	/* Need to write RCC register prior to writing RCC2 regiser */
	SYSCTL->RCC = (SYSCTL->RCC & ~0x7C0U) + 0x540; //clear XTAL field bits 10-6 and configure for 16 MHz crystal (value = 0x15)
	// select XTAL value and OSC source
	SYSCTL->RCC2 &= ~0x70U; // clear OSC source field
	SYSCTL->RCC2 += 0x00; // cofigure for main OSC source (MOSC value = 0x0)
	SYSCTL->RCC2 &= ~0x2000U; //activate PLL by clearing PWRDN (0 PLL operates normally, 1 PLL is powered down)
	SYSCTL->RCC2 |= 0x40000000; // use 400 MHz PLL
	SYSCTL->RCC2 = (SYSCTL->RCC2& ~0x1FC00000U) + (SYSDIV2<<22); // clear system CLK divider field and config. for 80 MHz CLK
	while((SYSCTL->RIS & 0x0000040)==0){}; //wait for PLL to lock
	SYSCTL->RCC2  &= ~0x800U; //enable PLL_Init by clearing bypass
	
}




