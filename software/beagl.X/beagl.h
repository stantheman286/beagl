/* 
 * File:   beagl.h
 * Author: matt
 *
 * Created on January 24, 2013, 11:36 PM
 */

#ifndef BEAGL_H
#define	BEAGL_H

// Set uC type (for xc.h)
#define __P24FJ256GB206__

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <xc.h>
#include <libpic30.h>
#include <uart.h>
#include <PPS.h>
#include "gsm.h"
#include "gps.h"
#include "usb.h"

// Definitions
#define FCY 4000000UL

// FCY set to 4 MHz (Fosc/2)
// Use backwards compatible setting

#define CYCLES_PER_MS ((unsigned long)(FCY * 0.001))                    // instruction cycles per millisecond
#define CYCLES_PER_US ((unsigned long)(FCY * 0.000001))                 // instruction cycles per microsecond
#define DELAY_MS(ms)  __delay32(CYCLES_PER_MS * ((unsigned long) ms));  // __delay32 is provided by the compiler, delay some # of milliseconds
#define DELAY_US(us)  __delay32(CYCLES_PER_US * ((unsigned long) us));  // delay some number of microseconds

#define LED_GRN_P42 0x0100
#define LED_BLU_P49 0x0002
#define LED_RED_P50 0x0004

// Prototypes
void blink(unsigned int LED);

#endif	/* BEAGL_H */

