#ifndef _PLL_H
#define _PLL_H

#include "TM4C123.h"

#define SYSDIV2 4

// bus frequency = 400 MHz/(SYSDIV2+1) = 80 MHz

void pll_init(void); //configure the system to get its CLK from PLL

#endif