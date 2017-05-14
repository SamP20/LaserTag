#include "millis.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

volatile unsigned long timer_millis;

void millis_init(void)
{
    TCCR2A = _BV(WGM21);
    OCR2A = 155;
    TCCR2B = _BV(CS20) | _BV(CS22);
    TIMSK2 = _BV(OCIE2A);
}

unsigned long millis(void)
{
    unsigned long millis_return;

    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        millis_return = timer_millis;
    }

    return millis_return;
}

ISR(TIMER2_COMPA_vect)
{
    timer_millis++;
}
