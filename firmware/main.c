#include "ir.h"
#include "serial.h"
#include "millis.h"
#include <stdint.h>
#include <util/delay.h>

static uint8_t buf[32];
static uint8_t bits;

int main(void)
{

    DDRB |= _BV(PB2);

    uint8_t data[] = {0x12, 0x34};
    ir_init();
    millis_init();
    uart_init(UART_TX_EN);
    stdout = stdin = &uartfile; // printf will send over serial TX (PD1)
    unsigned long last_mill=0;

    while(1)
    {
        unsigned long mill = millis();
        if(mill-last_mill > 100) // Transmit every 100ms
        {
            last_mill += 100;
            // Create some arbritrary data
            data[0]++;
            data[1]+=3;
            // Send it
            ir_send(data, 16);
        }

        if(ir_read(buf, &bits)) // If there's data available
        {
            // Convert bits to a whole number of bytes
            uint8_t bytes = (uint8_t)(((uint16_t)bits+7)>>3);
            for(uint8_t b=0;b<bytes;b++)
            {
                printf("%#04x ", buf[b]);
            }
            printf("\n");
        }
    }
}
