#include <stdint.h>
#include <stdlib.h>
#include "drivers/gpio.h"
#include "drivers/UART.h"
#include "drivers/I2C.h"
#include "drivers/PLL.h"
#include "drivers/SysTick.h"
#include "drivers/bme280.h"
#include "TM4C123.h"


//#define DEBUG_LED

//SystemCoreClock = 80MHz

#define DEBOUNCE_TIME 						5 	  // 5 ms debounce time
//volatile uint8_t buttonState = 0; 		//Stores current button states
//volatile uint8_t buttonPressed = 0; 	//Button press flag
//volatile uint8_t buttonDebounced = 0; //Debounced state flag

void GPIOF_Handler(void); 						//Interrupt handler for GPIOF (Buttons)
void ButtonPressHandler(void);
void keyHandler(void);

	
	
int main(void){
	
		// Initialize required peripherals
	  pll_init();
		gpio_portf_init();
		uart0_init();
		i2c0_init();
		systick_init();
	
		// bme280 init and config
		uint8_t bme280_id;		
		uart0_tx_string("Initializing...\r\n");
		bme280_id = bme280_init();
		uart0_tx_uint8_hex(bme280_id); uart0_tx_string("\r\n");
		bme280_config();
		systick_delay_ms(50);
		uart0_tx_string("Getting ready for readings...\r\n");

		
    while(1)
    {   
				#ifdef DEBUG_LED
					ButtonPressHandler();
					keyHandler();
        #endif
				
				uart0_tx_string("Temperature (C) : ");
				uart0_tx_int32(read_temperature());
				uart0_tx_string("\r\n");
				uart0_tx_string("Pressure (Pa): ");
				uart0_tx_int32(read_pressure());
				uart0_tx_string("\r\n");
				uart0_tx_string("Relative Humidity : ");
				uart0_tx_int32(read_humidity());
				uart0_tx_string("%\r\n");
			
			  systick_delay_ms(100); //delay in the main loop
    }
		
}

#ifdef DEBUG_LED

void GPIOF_Handler(void){
	uint8_t currentState = ~(GPIOF->DATA & 0x11); //Read the current button states(active low)
	buttonState = currentState; 									//Store the current state 
	buttonPressed = 1; 														// Set flag to indicate button is pressed, signals to the main loop from ISR
	GPIOF->ICR |= 0x11; 													//Clear interrupt flags on both SW1 and SW2 pins
}

void ButtonPressHandler(void){
	    if(buttonPressed) {
        systick_delay_ms(DEBOUNCE_TIME);       
        uint8_t currentState = ~(GPIOF->DATA & 0x11);
        if(currentState == buttonState) {
            if(currentState & 0x10) {
                GPIOF->DATA = LED_RED_PIN;   // Turn on RED LED
								uart0_tx_string("RED LED ON\r\n");
            }
            if(currentState & 0x01) {
                GPIOF->DATA = LED_BLUE_PIN;  // Turn on BLUE LED
								uart0_tx_string("BLUE LED ON\r\n");
            }
        }
        buttonPressed = 0;
				
    }
	
}

void keyHandler(void){
		unsigned char keyPress = uart0_receive();
		if(keyPress == 'g'){
				//UART0_TRANSMIT(keyPress);
				GPIOF-> DATA = LED_GREEN_PIN;       //GREEN LED ON
				uart0_tx_string("GREEN LED ON\r\n");
			}
}

#endif




