#include "gpio.h"
#include "TM4C123.h"

void gpio_portf_init(void){

  SYSCTL->RCGCGPIO |= 0x20;   /* enable clock to GPIOF */
  GPIOF->LOCK = 0x4C4F434B;   // unlock GPIOCR register
  GPIOF->CR = 0x1F;           // allow changes to PF4-0
  GPIOF->PUR |= 0x11;         // Enable Pull Up resistor PF4
  GPIOF->DIR |= 0x0E;         //set PF1 as an output and PF4 as an input pin
  GPIOF->DEN |= 0x1F;         // Enable PF1 and PF4 as a digital GPIO pins 
	
  /*Enable interrupts for Buttons*/
	GPIOF->IM |= 0x11;  				// Enable interrupts for SW1 and SW2
	GPIOF->IS &= ~(0x11U); 			// Edge-triggered
	GPIOF->IBE &= ~(0x11U); 		// Single edge
	GPIOF->IEV &= ~(0x11U); 		// Falling edge trigger
	NVIC->ISER[0] |= 1<<30; 		// Enable GPIOF interrupt
	
	
}