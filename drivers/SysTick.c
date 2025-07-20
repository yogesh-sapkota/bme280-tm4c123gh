/* SysTick Timer */
#include "TM4C123.h"
#include "SysTick.h"

void systick_init(void){
    SysTick->CTRL = 0x00;                    // Disable SysTick during setup
    SysTick->LOAD = 0xFFFFFF;                // Set reload value to maximum
    SysTick->VAL = 0;                        // Clear current value
    SysTick->CTRL = 0x05;                    // Enable SysTick with processor clock
}

void systick_wait(unsigned long delay){
	SysTick->LOAD = delay-1; 									//number of counts to wait
	SysTick->VAL = 0; 												//clears current value
	while((SysTick->CTRL & 0x10000) == 0); 		// wait for count flag
}

/*
	For CLK FREQ = 80 MHz, T = 12.5ns
	1 count = 12.5 ns
	1 ms = 80000 count
*/
void systick_delay_ms(unsigned long delay){
	unsigned long i;
	for(i=0;i<delay;i++){
		systick_wait(80000);
	}
	}


