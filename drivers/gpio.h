#ifndef _GPIO_H
#define _GPIO_H

//#include "TM4C123.h"
#define LED_RED 	0
#define LED_BLUE  1
#define LED_GREEN 2

#define LED_RED_PIN   0x02 //PF1
#define LED_BLUE_PIN  0x04 //PF2
#define LED_GREEN_PIN 0x08 //PF3

void gpio_portf_init(void);

#endif