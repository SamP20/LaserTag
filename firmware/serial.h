#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <avr/io.h>
#include <stdio.h>

#define BAUD 9600
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)
#define UART_RX_EN (1<<RXEN0)
#define UART_TX_EN (1<<TXEN0)

#define UART_BUF_POWER 5
#define UART_BUF_LEN (1<<UART_BUF_POWER)
#define UART_BUF_MASK (UART_BUF_LEN-1)

// Initialize the UART subsystem
void uart_init(uint8_t enabled);
// Return the number of bytes available to read.
uint8_t uart_available(void);

int uart_putchar(char var, FILE *stream);
int uart_getchar(FILE *stream);

extern FILE uartfile;

#endif
