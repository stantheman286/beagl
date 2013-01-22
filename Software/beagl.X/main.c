/*
 * File:   main.c
 * Author: matt
 *
 */

/* Definitions */
#define USE_AND_OR
#define FCY 4000000UL
// Use backwards compatible setting
// FCY set to 4 MHz (Fosc/2)
// Echo GPS data

/* Libraries */
#include "p24FJ256GB206.h"
#include "uart.h"
#include "PPS.h"
#include "libpic30.h"
#include "gps.h"
#include <string.h>

/* Definitions - MOVE TO HEADER */
#define LED_GRN_P42 0x0100
#define LED_BLU_P49 0x0002
#define LED_RED_P50 0x0004

/* Prototypes */
void gpsSetup(void);
void usbSetup(void);
void blink(unsigned int);
void uint8ToUART(uint8_t);
void uint16ToUART(uint16_t);
void booleanToUART(boolean);
void floatToUART(float);

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

char data[256];

int main(void)
{	
    /* Bring all Outputs Low */
    LATB = 0x0000;
    LATD = 0x0000;
    LATF = 0x0000;

    /* Disable analog I/O on PORT B */
    ANSB = 0x0000;

    /* Setup the GPS and USB UART */
    usbSetup();
    gpsSetup();
    
    /* Clear and enable GPS interrupt */
    U2RX_Clear_Intr_Status_Bit;
    EnableIntU2RX;

    /* Set up to echo GPS data */
    sendCommand(PMTK_SET_BAUD_9600);
    __delay_ms(100);
    sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    __delay_ms(100);
    sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    __delay_ms(100);
    
    /* Loop Forever */
    while(1)
    {
        blink(LED_GRN_P42);
        __delay_ms(500);
    }

    /* Disable Interrupts and close UART */
    DisableIntU2RX;
    CloseUART2();
    
}

void gpsSetup(void)
{
    /* Configure Port Direction */
    TRISBbits.TRISB8 = 0;   //  Turn RB8 into output for GPS EN
    TRISBbits.TRISB9 = 1;   //  Turn RB9 into input for GPS FIX
    TRISBbits.TRISB14 = 1;  //  Turn RB14 into input for GPS RX
    TRISBbits.TRISB15 = 0;  //  Turn RB15 into output for GPS TX
    TRISFbits.TRISF4 = 1;   //  Turn RF4 into input for GPS PPS

    /* Configure PPS pins for GPS */
    iPPSInput(IN_FN_PPS_U2RX,IN_PIN_PPS_RP14);      // Assign U2RX to pin RP14
    iPPSOutput(OUT_PIN_PPS_RP29,OUT_FN_PPS_U2TX);   // Assign U2TX to pin RP29

    /* Enable the GPS module */
    PORTB |= 0x0100;
    
    /* Close UART in case it's already open */
    CloseUART2();

    /*Enable UART Interface */

    ConfigIntUART2(UART_RX_INT_DIS | UART_RX_INT_PR6 | UART_TX_INT_DIS | UART_TX_INT_PR6);
    // Receive interrupt disabled
    // Priority RX interrupt 6
    // Transmit interrupt disabled
    // Priority TX interrupt 6

    OpenUART2(UART_EN, UART_TX_ENABLE, 25);
    // Module enable
    // Transmit enable
    // 9600 baud rate (@ 4 MHz internal clock)

    /* Initalize the Adafruit GPS libraries */
    common_init();

}

/* Set up the UART for the FTDI chip */
void usbSetup(void)
{
    /* Configure Port Direction */
    TRISDbits.TRISD11 = 0;  //  Turn RD11 into output for USB TX
    TRISDbits.TRISD10 = 1;  //  Turn RD10 into input for USB RX
    TRISDbits.TRISD9 = 0;   //  Turn RD9 into output for USB RTS#
    TRISDbits.TRISD8 = 0;   //  Turn RD8 into output for LED
    TRISDbits.TRISD6 = 0;   //  Turn RD6 into output for LED
    TRISDbits.TRISD2 = 0;   //  Turn RD2 into output for LED
    TRISDbits.TRISD1 = 0;   //  Turn RD2 into output for LED
    TRISDbits.TRISD0 = 1;   //  Turn RD0 into input for USB CTS#
    
    /* Configure PPS pins for USB */
    iPPSInput(IN_FN_PPS_U3RX,IN_PIN_PPS_RP3);       // Assign U3RX to pin RP3
    iPPSInput(IN_FN_PPS_U3CTS,IN_PIN_PPS_RP11);     // Assign U3CTS# to pin RP11
    iPPSOutput(OUT_PIN_PPS_RP12,OUT_FN_PPS_U3TX);   // Assign U3TX to pin RP12
    iPPSOutput(OUT_PIN_PPS_RP4,OUT_FN_PPS_U3RTS);   // Assign U3RTS# to pin RP4

    /* Close UART in case it's already open */
    CloseUART3();

    /*Enable UART Interface */

    ConfigIntUART3(UART_RX_INT_DIS | UART_TX_INT_DIS);
    // Receive interrupt disabled
    // Transmit interrupt disabled

    OpenUART3(UART_EN | UART_UEN_10 | UART_EVEN_PAR_8BIT, UART_TX_ENABLE, 25);
    // Module enable
    // UxTX, UxRX, UxCTS and UxRTS pins are enabled and used
    // BRG generates 4 clocks per bit period
    // Even parity 8 bit
    // Transmit enable
    // 9600 baud rate (@ 4 MHz FCY)
}

/* Blink the given LED */
void blink(unsigned int LED)
{
   PORTD |= LED;
   __delay_ms(75);
   PORTD &= ~LED;
}

/* Sends uint8 out on UART */
void uint8ToUART(uint8_t n)
{
    /* Value is 0 - 59 (seconds, minutes) or 0 - 23 (hours), calculate places and convert to ASCII */
    
    /* Calculate the tens and ones place */
    while(BusyUART3());
    WriteUART3((unsigned int)((n / 10) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)((n % 10) + 48));

}

/* Sends uint16 out on UART */
void uint16ToUART(uint16_t n)
{
    /* Value is 0 - 1000, calculate places and convert to ASCII */

    /* Calculate the thousands, hundreds, tens and ones place */
    while(BusyUART3());
    WriteUART3((unsigned int)((n / 1000) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)(((n % 1000) / 100) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)(((n % 100) / 10) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)((n % 10) + 48));

}

/* Sends boolean out on UART */
void booleanToUART(boolean b)
{

    /* Output true or false based on value */
    while(BusyUART3());
    if (b == true)
        putsUART3((unsigned int*)"true");
    else
        putsUART3((unsigned int*)"false");

}

/* Sends float out on UART */
void floatToUART(float f)
{
    unsigned int n_whole, n_dec;

    /* Value always has 10 digits as follows: XXXXX.XXXX */
    n_whole = (unsigned int)f;
    n_dec = (unsigned int)((f - n_whole)*10000);
    
    /* Calculate the ten thounsands, thousands, hundreds, tens and ones place */
    while(BusyUART3());
    WriteUART3((unsigned int)((n_whole / 10000) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)(((n_whole % 10000) / 1000) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)(((n_whole % 1000) / 100) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)(((n_whole % 100) / 10) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)((n_whole % 10) + 48));

    while(BusyUART3());
    WriteUART3('.');

    /* Calculate the tenths, hundredths, thousandths, ten thousandths place */
    while(BusyUART3());
    WriteUART3((unsigned int)((n_dec / 1000) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)(((n_dec % 1000) / 100) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)(((n_dec % 100) / 10) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)((n_dec % 10) + 48));
    
  //  while(BusyUART3());
   // WriteUART3((unsigned int)(((n * 10) % 100) % 10));
   // while(BusyUART3());
   // WriteUART3((unsigned int)((((n * 100) % 1000) % 100) % 10));
   // while(BusyUART3());
  //  WriteUART3((unsigned int)(((((n * 1000) % 10000) % 1000) % 100) % 10));
   // while(BusyUART3());
   // WriteUART3((unsigned int)((((((n * 10000) % 100000) % 10000) % 1000) % 100) % 10));
    
}

/* UART2 RX ISR */
void __attribute__ ((interrupt,no_auto_psv)) _U2RXInterrupt(void)
{
    float test;
    char c;

    /* Clear the interrupt status of UART3 RX */
    U2RX_Clear_Intr_Status_Bit;

    /* Wait for data to become available */
    //while(!DataRdyUART2());

    /* Read a character */
    //c = ReadUART2();

    /* Read a character */
    c = read();

    /* Valid Chracter returned */
    if (c) {

        /* Write character out to USB when ready */
        while(BusyUART3());
        WriteUART3((unsigned int)c);

        /* Clear out any garbage characters */
        while(DataRdyUART2())
            ReadUART2();

    }

    /* Received a valid NMEA sentence */
    if (newNMEAreceived()) {

        /* If parse successful (checksum passed), print data */
        if (parse(lastNMEA())) {

            while(BusyUART3());
            putsUART3((unsigned int*)"\n---------------------------");
            while(BusyUART3());
            putsUART3((unsigned int*)"---------------------------");
            while(BusyUART3());
            putsUART3((unsigned int*)"---------------------------");

            /* Time */
            while(BusyUART3());
            putsUART3((unsigned int*)"\nTime: ");
            uint8ToUART(hour);
            while(BusyUART3());
            WriteUART3(':');
            uint8ToUART(minute);
            while(BusyUART3());
            WriteUART3(':');
            uint8ToUART(seconds);
            while(BusyUART3());
            WriteUART3('.');
            uint16ToUART(milliseconds);

            /* Date */
            while(BusyUART3());
            putsUART3((unsigned int*)" Date: ");
            uint8ToUART(month);
            while(BusyUART3());
            WriteUART3('/');
            uint8ToUART(day);
            while(BusyUART3());
            putsUART3((unsigned int*)"/20");    // Make year 4 digits
            uint8ToUART(year);

            /* Location Information */
            while(BusyUART3());
            putsUART3((unsigned int*)" Fix: ");
            booleanToUART(fix);
            while(BusyUART3());
            putsUART3((unsigned int*)" Quality: ");
            uint8ToUART(fixquality);

            /* Have a fix, print out useful information */
            if(fix) {
                while(BusyUART3());
                putsUART3((unsigned int*)"\nLocation: ");
                floatToUART(latitude);
                while(BusyUART3());
                WriteUART3(lat);    // Straight chatacter, no extra conversion
                while(BusyUART3());
                putsUART3((unsigned int*)", ");
                floatToUART(longitude);
                while(BusyUART3());
                WriteUART3(lon);    // Straight chatacter, no extra conversion
//FIX                test = 12345.9876;
//FIX                floatToUART(test);
                while(BusyUART3());
                putsUART3((unsigned int*)" Speed (knots): ");
                floatToUART(speed);
                while(BusyUART3());
                putsUART3((unsigned int*)" Angle: ");
                floatToUART(angle);
                while(BusyUART3());
                putsUART3((unsigned int*)" Altitude: ");
                floatToUART(altitude);
                while(BusyUART3());
                putsUART3((unsigned int*)" Satellites: ");
                uint8ToUART(satellites);
            }
        }
    }
    
}