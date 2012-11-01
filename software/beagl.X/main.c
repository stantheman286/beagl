/*
 * File:   main.c
 * Author: matt
 *
 */

/* Definitions */
#define USE_AND_OR
#define FCY 8000000UL
// Use backwards compatible setting
// FCY set to 8 MHz

/* Libraries */
#include "p24FJ256GB206.h"
#include "uart.h"
#include "PPS.h"
#include "libpic30.h"

/* Prototypes */
void usbSetup(void);
void blink(unsigned int);

/* Configuration Settings */

_CONFIG1 (JTAGEN_OFF & ICS_PGx1 & FWDTEN_OFF & GWRP_OFF & GCP_OFF)
// JTAG disabled
// Communication Channel: PGC1/EMUC1 and PGD1/EMUD1
// Watchdog TimeFWDTEN_OFFr disabled
// Writes to program memory are allowed
// Code protection is disabled

_CONFIG2 (POSCMOD_NONE & OSCIOFNC_OFF & FNOSC_FRC)
// Primary oscillator is disabled
// OSCO/CLKO/RC15 functions as CLKO (FOSC/2)
// Fast RC Oscillator (FRC)

unsigned char Data[256];

int main(void)
{	
    /* Bring all Outputs Low */
    LATD = 0x0000;

    /* Configure Port Direction */
    TRISDbits.TRISD11 = 0;  //  Turn RD11 into output for UART TX
    TRISDbits.TRISD10 = 1;  //  Turn RD10 into input for UART RX
    TRISDbits.TRISD9 = 0;   //  Turn RD9 into output for UART RTS#
    TRISDbits.TRISD8 = 0;   //  Turn RD8 into output for LED
    TRISDbits.TRISD6 = 0;   //  Turn RD6 into output for LED
    TRISDbits.TRISD0 = 1;   //  Turn RD0 into input for UART CTS#

    /* Setup the USB UART */
    usbSetup();
    
    /* Done */
    while(1)
    {
        blink(0);
        __delay_ms(1000);
    }

    /* Disable Interrupts and close UART */
    DisableIntU3TX;
    DisableIntU3RX;
    CloseUART3();
    
}

/* Set up the UART for the FTDI chip */
void usbSetup(void)
{
    /* Configure PPS pins for UART */
    iPPSInput(IN_FN_PPS_U3RX,IN_PIN_PPS_RP3);       // Assign U3RX to pin RP3
    iPPSInput(IN_FN_PPS_U3CTS,IN_PIN_PPS_RP11);     // Assign U3CTS# to pin RP11
    iPPSOutput(OUT_PIN_PPS_RP12,OUT_FN_PPS_U3TX);   // Assign U3TX to pin RP12
    iPPSOutput(OUT_PIN_PPS_RP4,OUT_FN_PPS_U3RTS);   // Assign U3RTS# to pin RP4

    /* Close UART in case it's already open */
    CloseUART3();

    /*Enable UART Interface */

    ConfigIntUART3(UART_RX_INT_EN | UART_RX_INT_PR7 | UART_TX_INT_EN | UART_TX_INT_PR7);
    // Receive interrupt enabled
    // Priority RX interrupt 7
    // Transmit interrupt enabled
    // Priority TX interrupt 7

    OpenUART3(UART_EN | UART_UEN_10 | UART_BRGH_FOUR | UART_EVEN_PAR_8BIT, UART_TX_ENABLE, 8);
    // Module enable
    // UxTX, UxRX, UxCTS and UxRTS pins are enabled and used
    // BRG generates 4 clocks per bit period
    // Even parity 8 bit
    // Transmit enable
    // 115200 baud rate (@ 8 MHz internal clock)
}

/* Blink the given LED */
void blink(unsigned int LED)
{
   PORTD |= 0x0100;
   __delay_ms(75);
   PORTD &= ~(0x0100);
}

/* UART3 TX ISR */
void __attribute__ ((interrupt,no_auto_psv)) _U3TXInterrupt(void)
{
    static unsigned int i = 0;

    /* Clear the interrupt status of UART3 TX */
    U3TX_Clear_Intr_Status_Bit;

    /* Check of end of string */
    if(Data[i] != '\0')
    {
        /* Wait until the UART is busy*/
        while(BusyUART3());

        /* Transmit the data */
        WriteUART3((unsigned int)Data[i++]);
  }
}

/* UART3 RX ISR */
void __attribute__ ((interrupt,no_auto_psv)) _U3RXInterrupt(void)
{
    static unsigned int j=0;

    /* Clear the interrupt status of UART3 RX */
    U3RX_Clear_Intr_Status_Bit;

    /* Wait for data reception on RX */
    while(!DataRdyUART3());
    
    /* Receive the data */
    Data[j++] = ReadUART3();

    /* Force a TX interrupt to immediately transmit the data */
    IFS5bits.U3TXIF = 1;
}
