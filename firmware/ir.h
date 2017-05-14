#ifndef IR_H
#define IR_H

#include <stdint.h>

/*
#define IR_HEADER_TICKS 6000
#define IR_SPACE_TICKS 1500
#define IR_ONE_TICKS 3000
#define IR_ZERO_TICKS 1500
#define IR_WAIT_TICKS 24000

#define IR_HEADER_TIMEOUT IR_HEADER_TICKS+1000
#define IR_SPACE_TIMEOUT IR_SPACE_TICKS+250
#define IR_BIT_TIMEOUT IR_ONE_TICKS+250

#define IR_HEADER_MINIMUM IR_HEADER_TICKS-1000
#define IR_SPACE_MINIMUM IR_SPACE_TICKS-250
#define IR_BIT_MINIMUM IR_ZERO_TICKS-250
*/

/*
#define IR_HEADER_TICKS 750
#define IR_SPACE_TICKS 188
#define IR_ONE_TICKS 375
#define IR_ZERO_TICKS 188
#define IR_WAIT_TICKS 3000

#define IR_TIMEOUT 750
#define IR_HEADER_TOLERANCE 64
#define IR_TOLERANCE 32
*/

#define IR_HEADER_TICKS 134
#define IR_SPACE_TICKS 34
#define IR_ONE_TICKS 67
#define IR_ZERO_TICKS 34
#define IR_WAIT_TICKS 540

#define IR_TIMEOUT 134
#define IR_HEADER_TOLERANCE 20
#define IR_TOLERANCE 10

#define IR_ONEZERO_THRESHOLD (IR_ONE_TICKS+IR_ZERO_TICKS)/2

void ir_init(void);
void ir_send(uint8_t* data, uint8_t bits);
uint8_t ir_ready(void);

uint8_t ir_read(uint8_t* data, uint8_t *bits);


#endif
