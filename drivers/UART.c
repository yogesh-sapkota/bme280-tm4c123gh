#include "UART.h"

#define UART_RX_BUFFER_SIZE 64 //handle for command strings
static volatile char rx_buffer[UART_RX_BUFFER_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;


void uart0_init(void){

SYSCTL->RCGCUART |= 0x01;  //enable UART module 0
SYSCTL->RCGCGPIO |= 0x01;  //enable PORT A

GPIOA->AFSEL |= 0x03;  //enable alternate pins on PA0/PA1
GPIOA->DEN |= 0x03;		 //enable digital I/O PA0/PA1
GPIOA->PCTL |= 0x11;   //configure PA0/1 as UART peripheral

/* configure UART
Clock : 50 MHz / 80 MHz
Baud Rate : 9600 / 115200
Data Length : 8 Bits
One Stop Bit
No parity
FIFOs enable
Enable interrupt
*/

UART0->CTL &= ~0x301; //disable UART
UART0->IBRD = 43; // integer part, BRD = 80,000,000/(16*115200) = 43.40 --325.5208333
UART0->FBRD = 26; //33; //fraction part = integer(BRD-fractional part * 64 + 0.5)
UART0->LCRH = 0x70; //8 bit word length, FIFO enabled
UART0->CTL |= 0x301; //unable UART

//enable interrupt
UART0->ICR = 0x7FF; //clear all UART interrupt
UART0->IM |= 0x10;
NVIC->ISER[0] = 0x00000020;
}

/* UART RX */
unsigned char uart0_receive(void){
//	while((UART0->FR & 0x10) != 0); // wait until the receive FIFO is not empty
//	return ((unsigned char)(UART0->DR & 0xFF)); 
	
	if(rx_head == rx_tail) {
		return 0;  // No data available
	}
	unsigned char data = rx_buffer[rx_tail];
	rx_tail = (rx_tail + 1) % UART_RX_BUFFER_SIZE;
	return data;
}

/* UART TX */
void uart0_transmit(unsigned char data){
  while((UART0->FR & 0x20) != 0);
  UART0->DR= data;
}

void uart0_tx_string(char *ptr){
		while(*ptr != 0){
			uart0_transmit(*ptr);
			ptr++;
		}
}

// UART0 Interrupt Handler
void UART0_Handler (void){
	if(UART0->MIS & 0x10) {  // RX interrupt
		uint8_t next_head = (rx_head + 1) % UART_RX_BUFFER_SIZE;
		if(next_head != rx_tail) {  // Buffer not full
			rx_buffer[rx_head] = UART0->DR & 0xFF;
			rx_head = next_head;
		}
		UART0->ICR = 0x10;  // Clear RX interrupt
	}
}

void uart0_tx_uint8_hex(uint8_t value) {
    uart0_tx_string("0x");
    
    // Convert each hex digit
    for (int i = 1; i >= 0; i--) {
        uint8_t digit = (value >> (i * 4)) & 0x0F;
        char hex_char;
        
        if (digit < 10) {
            hex_char = '0' + digit;
        } else {
            hex_char = 'A' + (digit - 10);
        }
        
        uart0_transmit(hex_char);
    }
}


void uart0_tx_int32(int32_t value) {
    char buffer[12]; // Enough for -2147483648\0
    int i = 0;
    int is_negative = 0;

    // Handle zero explicitly
    if (value == 0) {
        uart0_transmit('0');
        return;
    }

    // Handle negative numbers
    if (value < 0) {
        is_negative = 1;
        // Handle INT_MIN edge case
        if (value == (int32_t)0x80000000) {
            // -2147483648
            const char *min_str = "-2147483648";
            for (i = 0; min_str[i] != '\0'; i++)
                uart0_transmit(min_str[i]);
            return;
        }
        value = -value;
    }

    // Convert integer to string in reverse order
    while (value > 0) {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (is_negative) {
        buffer[i++] = '-';
    }

    // Send the string in correct order
    while (i--) {
        uart0_transmit(buffer[i]);
    }
}
