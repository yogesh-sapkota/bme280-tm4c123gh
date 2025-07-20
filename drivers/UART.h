#ifndef _UART_H
#define _UART_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "TM4C123.h"

void uart0_init(void);
void uart0_transmit(unsigned char);
unsigned char uart0_receive(void);
void uart0_tx_string(char *ptr);
void uart0_tx_uint8_hex(uint8_t value);
void uart0_tx_int32(int32_t value);
void UART0_Handler(void);
#endif
