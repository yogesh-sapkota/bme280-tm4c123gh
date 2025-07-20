#ifndef _SYSTICK_H
#define _SYSTICK_H

#include "TM4C123.h"

void systick_init(void);
void systick_wait(unsigned long delay);
void systick_delay_ms(unsigned long delay);

#endif