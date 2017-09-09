#include "ir.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include <string.h>
#include <util/delay.h>

static inline void ir_send_on(void);
static inline void ir_send_off(void);
static inline void ir_capt_on(void);
static inline void ir_capt_off(void);
static inline void ir_listen_reset(void);
static inline void ir_timeout_rst(void);

#define IR_BIT_LOW 0
#define IR_BIT_HIGH 1
#define IR_HEADER_HIGH 2
#define IR_SILENCE 3
#define IR_DONE 4

static uint8_t out_buffer[32];
static volatile uint8_t out_bit=0;
static uint8_t out_total_bits=0;
static volatile uint8_t out_state = IR_SILENCE;

void ir_init(void)
{
    DDRB |= _BV(PB1);

    TCCR1A = _BV(COM1A1); // Clear OC1A on compare match

    //TCCR1B = _BV(CS11) | _BV(CS10); // 2.5MhZ clock
    //TCCR1B = _BV(CS11) | _BV(CS10); // 312.5 kHz clock
    //TCCR1B = _BV(CS12);
    //TCCR1B = _BV(CS12) | _BV(CS10); // 19.53kHz clock

    // External 56kHz clock on T1 (PD5).
    TCCR1B = _BV(CS12) | _BV(CS11) | _BV(CS10);

    TIMSK1 = _BV(ICIE1); // Enable input capture
    ir_listen_reset();
    sei();

    // Generate 56kHz carrier on OCOB (PD5). This is connected internally to
    // the T1 clock input.
    OCR0A = 178;
    TCCR0A = _BV(WGM01) | _BV(COM0B0);
    TCCR0B = _BV(CS00);
    TCNT0 = 0;
    DDRD |= _BV(PD5);
}

void ir_send(uint8_t* data, uint8_t bits)
{
    while(TIMSK1 & _BV(OCIE1A));
    out_total_bits = bits;
    out_bit = 0;
    uint8_t bytes = (bits>>3) + ((bits&0x07)?1:0);
    memcpy((void*)out_buffer, data, bytes);

    out_state = IR_SILENCE;
    OCR1A = TCNT1+100;
    ir_send_on(); // Next output shall be high

    TIFR1 |= _BV(OCF1A); // Clear interrupt flag before enabling.
    TIMSK1 |= _BV(OCIE1A); // Enable Compare A interrupt

}

static inline void ir_send_on(void)
{
    TCCR1A |= _BV(COM1A0);
}

static inline void ir_send_off(void)
{
    TCCR1A &= ~_BV(COM1A0);
}

ISR(TIMER1_COMPA_vect)
{
    switch (out_state) {

    // Triggered on header start
    case IR_SILENCE:
        ir_send_off(); // Next output shall be low
        out_state = IR_HEADER_HIGH;
        OCR1A += IR_HEADER_TICKS;
    break;

    // Triggered on end of bit pulse
    case IR_BIT_HIGH:
        out_bit++;
    // FALLTHROUGH
    // Triggered on end of header pulse
    case IR_HEADER_HIGH:
        if(out_bit >= out_total_bits)
        {
            out_state = IR_DONE;
            OCR1A += IR_WAIT_TICKS;
        }
        else
        {
            ir_send_on(); // Next output shall be high
            out_state = IR_BIT_LOW;
            OCR1A += IR_SPACE_TICKS;
        }
    break;

    // Triggered on start of bit pulse
    case IR_BIT_LOW:
        ir_send_off(); // Next output shall be low
        out_state = IR_BIT_HIGH;
        OCR1A += (out_buffer[out_bit>>3] & _BV(out_bit & 0x07))? IR_ONE_TICKS : IR_ZERO_TICKS;
    break;

    // Triggered when message send has finished
    case IR_DONE:
        TIMSK1 &= ~_BV(OCIE1A); // Disable COMPA interrupt
    break;
    }
}

/****************
 * IR Receiving *
 ****************/

static uint8_t in_buffer[32];
static volatile uint8_t in_bit=0;
static volatile uint8_t in_state = IR_SILENCE;

static inline void ir_capt_off(void)
{
    TCCR1B |= _BV(ICES1); // Next capture on rising edge (IR off)
}

static inline void ir_capt_on(void)
{
    TCCR1B &= ~_BV(ICES1); // Next capture on falling edge (IR on)
}

// Bad message. Reset back to default listening state
static inline void ir_listen_reset(void)
{
    TIMSK1 &= ~_BV(OCIE1B); // Disable output compare B for timeout.
    in_bit=0;
    in_state = IR_SILENCE; // Reset back to listening for header pulse
    ir_capt_on();
}

// Extend timeout duration
static inline void ir_timeout_rst(void)
{
    OCR1B = TCNT1 + IR_TIMEOUT;
}

uint8_t ir_read(uint8_t* data, uint8_t *bits)
{
    if(in_state != IR_DONE)
    {
        return 0;
    }

    uint8_t bytes = (in_bit>>3) + ((in_bit&0x07)?1:0);
    memcpy((void*)data, in_buffer, bytes);
    (*bits) = in_bit;
    ir_listen_reset();
    return 1;
}

// This timer is used to signal the end of a message
ISR(TIMER1_COMPB_vect)
{
    TIMSK1 &= ~_BV(OCIE1B); // Disable output compare B for timeout.
    in_state = IR_DONE;
}

ISR(TIMER1_CAPT_vect)
{
    static uint16_t last_icr=0;

    uint16_t icr = ICR1;
    uint16_t delta = icr-last_icr;
    last_icr = icr;

    // in_state represents the previous state before the interrupt
    switch (in_state) {

    // Triggered on header start
    case IR_SILENCE:
        ir_capt_off();
        in_state = IR_HEADER_HIGH;
    break;

    // Triggered on end of header pulse
    case IR_HEADER_HIGH:
        if(delta < IR_HEADER_TICKS-IR_HEADER_TOLERANCE)
        {
            ir_listen_reset();
            return;
        }

        ir_timeout_rst();
        TIFR1 |= _BV(OCF1B); // Clear interrupt flag before enabling.
        TIMSK1 |= _BV(OCIE1B); // Enable end of message timer
        ir_capt_on();
        in_state = IR_BIT_LOW;
    break;

    // Triggered on start of bit pulse
    case IR_BIT_LOW:
        ir_timeout_rst();
        if(delta < IR_SPACE_TICKS-IR_TOLERANCE || delta > IR_SPACE_TICKS+IR_TOLERANCE)
        {
            ir_listen_reset();
            return;
        }

        ir_capt_off();
        in_state = IR_BIT_HIGH;
    break;

    // Triggered on end of bit pulse
    case IR_BIT_HIGH:
        ir_timeout_rst();
        if(delta < IR_ZERO_TICKS-IR_TOLERANCE || delta > IR_ONE_TICKS+IR_TOLERANCE)
        {
            ir_listen_reset();
            return;
        }

        // Measure the duration to determine if it is a zero or one.
        if(delta > IR_ONEZERO_THRESHOLD)
        {
            in_buffer[in_bit>>3] |= _BV(in_bit & 0x07);
        }
        else
        {
            in_buffer[in_bit>>3] &= ~_BV(in_bit & 0x07);
        }

        if(++in_bit == 0xff)
        {
            // Buffer had been filled. Discard message
            ir_listen_reset();
            return;
        }

        ir_capt_on();
        in_state = IR_BIT_LOW;
    break;
    }
}
