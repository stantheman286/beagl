
#include "beagl.h"

// Blink the given LED
void blink(unsigned int LED)
{
   PORTD |= LED;
   DELAY_MS(75);
   PORTD &= ~LED;
}
