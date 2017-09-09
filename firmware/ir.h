#ifndef IR_H
#define IR_H

#include <stdint.h>


#define IR_HEADER_TICKS 134
#define IR_SPACE_TICKS 34
#define IR_ONE_TICKS 67
#define IR_ZERO_TICKS 34
#define IR_WAIT_TICKS 540

#define IR_TIMEOUT 134
#define IR_HEADER_TOLERANCE 20
#define IR_TOLERANCE 10

#define IR_ONEZERO_THRESHOLD (IR_ONE_TICKS+IR_ZERO_TICKS)/2

// Initialize the IR subsystem.
void ir_init(void);
// Transmit an IR packet. Will block if a packet is alredy being transmitted.
void ir_send(uint8_t* data, uint8_t bits);
// Check if ir_send can transmit without blocking (Not implemented yet).
uint8_t ir_ready(void);
// Attempt to read a packet. Returns 1 if a packet was read, 0 otherwise.
uint8_t ir_read(uint8_t* data, uint8_t *bits);


#endif
