/*
 * File:   main.c
 * Author: matt
 *
 */

// Definitions
#define USE_AND_OR
// Use backwards compatible setting

// Libraries
#include "beagl.h"

// Prototypes
void usbSetup(void);
void gpsSetup(void);
void gsmSetup(void);

// Global variables (located in gps.c)
extern uint8_t hour, minute, seconds, year, month, day;
extern uint16_t milliseconds;
extern double latitude, longitude, geoidheight, altitude;
extern double speed, angle, magvariation, HDOP;
extern char lat, lon, mag;
extern boolean fix;
extern uint8_t fixquality, satellites;

extern uint16_t LOCUS_serial, LOCUS_records;
extern uint8_t LOCUS_type, LOCUS_mode, LOCUS_config, LOCUS_interval, LOCUS_distance, LOCUS_speed, LOCUS_status, LOCUS_percent;

extern boolean paused;

unsigned char incoming_char=0;   // Will hold the incoming character from the Serial Port.

// Configuration Settings

_CONFIG1 (JTAGEN_OFF & ICS_PGx1 & FWDTEN_OFF & GWRP_OFF & GCP_OFF)
// JTAG disabled
// Communication Channel: PGC1/EMUC1 and PGD1/EMUD1
// Watchdog Timer disabled
// Writes to program memory are allowed
// Code protection is disabled

_CONFIG2 (POSCMOD_NONE & OSCIOFNC_OFF & FNOSC_FRC)
// Primary oscillator is disabled
// OSCO/CLKO/RC15 functions as CLKO (FOSC/2)
// Fast RC Oscillator (FRC)

char data[256];

int main(void)
{	
    // Bring all Outputs Low
    LATB = 0x0000;
    LATD = 0x0000;
    LATF = 0x0000;
    LATG = 0x0000;

    // Disable analog I/O on PORT B and G
    ANSB = 0x0000;
    ANSG = 0x0000;

    // Setup the GPS and USB UART
    usbSetup();
    gpsSetup();
    gsmSetup();

    // Clear and enable GSM and USB interrupts
    U1RX_Clear_Intr_Status_Bit;
    U3RX_Clear_Intr_Status_Bit;
    EnableIntU1RX;
    EnableIntU3RX;
    
    // Clear and enable GPS interrupt
//    U2RX_Clear_Intr_Status_Bit;
//    EnableIntU2RX;

    // Set up to echo GPS data
//    sendCommand(PMTK_SET_BAUD_9600);
//    DELAY_MS(100);
//    sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
//    sendCommand(PMTK_SET_NMEA_OUTPUT_OFF);    // Enable for log dump
//    DELAY_MS(100);
//    sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);      // Disable for log dump
//    DELAY_MS(100);

    // Start GPS data logger
//    startLOCUS();

    strToUSB("Starting SM5100B Communication...\n");

    // Loop Forever
    while(1)
    {
        blink(LED_GRN_P42);
        DELAY_MS(1000);

        // Print out logger info
//        displayLOCUSInfo();

    }

    // Disable Interrupts and close UART
    DisableIntU1RX;
    DisableIntU2RX;
    DisableIntU3RX;
    CloseUART1();
    CloseUART2();
    CloseUART3();
    
}

// Set up the UART for the FTDI chip
void usbSetup(void)
{
    // Configure Port Direction
    TRISDbits.TRISD11 = 0;  //  Turn RD11 into output for USB TX
    TRISDbits.TRISD10 = 1;  //  Turn RD10 into input for USB RX
    TRISDbits.TRISD9 = 0;   //  Turn RD9 into output for USB RTS#
    TRISDbits.TRISD8 = 0;   //  Turn RD8 into output for LED
    TRISDbits.TRISD6 = 0;   //  Turn RD6 into output for LED
    TRISDbits.TRISD2 = 0;   //  Turn RD2 into output for LED
    TRISDbits.TRISD1 = 0;   //  Turn RD2 into output for LED
    TRISDbits.TRISD0 = 1;   //  Turn RD0 into input for USB CTS#

    // Configure PPS pins for USB
    iPPSInput(IN_FN_PPS_U3RX,IN_PIN_PPS_RP3);       // Assign U3RX to pin RP3
    iPPSInput(IN_FN_PPS_U3CTS,IN_PIN_PPS_RP11);     // Assign U3CTS# to pin RP11
    iPPSOutput(OUT_PIN_PPS_RP12,OUT_FN_PPS_U3TX);   // Assign U3TX to pin RP12
    iPPSOutput(OUT_PIN_PPS_RP4,OUT_FN_PPS_U3RTS);   // Assign U3RTS# to pin RP4

    // Close UART in case it's already open
    CloseUART3();

    // Enable UART Interface

    ConfigIntUART3(UART_RX_INT_DIS | UART_RX_INT_PR5 | UART_TX_INT_DIS | UART_TX_INT_PR5);
    // Receive interrupt disabled
    // Transmit interrupt disabled

    OpenUART3(UART_EN | UART_UEN_10, UART_TX_ENABLE, 25);
    // Module enable
    // UxTX, UxRX, UxCTS and UxRTS pins are enabled and used
    // BRG generates 4 clocks per bit period
    // Even parity 8 bit
    // Transmit enable
    // 9600 baud rate (@ 4 MHz FCY)
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

// Set up the UART for the GSM chip
void gsmSetup(void)
{
    // Configure Port Direction
    TRISGbits.TRISG6 = 0;   //  Turn RG6 into output for GSM TX
    TRISGbits.TRISG7 = 1;   //  Turn RG7 into input for GSM RX

    // Configure PPS pins for GSM
    iPPSInput(IN_FN_PPS_U1RX,IN_PIN_PPS_RP26);       // Assign U1RX to pin RP26
    iPPSOutput(OUT_PIN_PPS_RP21,OUT_FN_PPS_U1TX);   // Assign U1TX to pin RP21

    // Close UART in case it's already open
    CloseUART1();

    // Enable UART Interface

    ConfigIntUART1(UART_RX_INT_DIS | UART_RX_INT_PR7 | UART_TX_INT_DIS | UART_TX_INT_PR7);
    // Receive interrupt disabled
    // Transmit interrupt disabled

    OpenUART1(UART_EN, UART_TX_ENABLE, 25);
    // Module enable
    // BRG generates 4 clocks per bit period
    // Transmit enable
    // 9600 baud rate (@ 4 MHz FCY)
}

// UART1 RX ISR
void __attribute__ ((interrupt,no_auto_psv)) _U1RXInterrupt(void)
{
    char c;
    
    // Clear the interrupt status of UART1 RX
    U1RX_Clear_Intr_Status_Bit;

    // Get the character from the GSM UART
    c = gsmRead();

    // If a character comes in from the GSM module...
    if (c)
    {    
        while(BusyUART3());
        WriteUART3((unsigned int)c);      // Print the incoming character to USB UART

        // Clear out any garbage characters
        while(DataRdyUART1())
            ReadUART1();
    }

    // Detect if GSM antenna is ready
    //ms: ADD PARSER
    if(newSINDreceived() && gsmReady()) {
        strToUSB("\nWOO, READY!\n");
        gsmCall();  //ms: ADD NUMBER
    }
}

// UART2 RX ISR
void __attribute__ ((interrupt,no_auto_psv)) _U2RXInterrupt(void)
{
    char c;

    // Clear the interrupt status of UART2 RX
    U2RX_Clear_Intr_Status_Bit;

    // Wait for data to become available
    //while(!DataRdyUART2());

    // Read a character
    //c = ReadUART2();

    // Read a character
    c = read();

    // Valid Chracter returned
    if (c) {

        // Write character out to USB when ready
        while(BusyUART3());
        WriteUART3((unsigned int)c);

        // Clear out any garbage characters
        while(DataRdyUART2())
            ReadUART2();

    }

    // Print out GPS info
//    displayGPSInfo();
    
}

// UART3 RX ISR
void __attribute__ ((interrupt,no_auto_psv)) _U3RXInterrupt(void)
{
    // Clear the interrupt status of UART1 RX
    U3RX_Clear_Intr_Status_Bit;
    
    if(DataRdyUART3())
    {
        incoming_char = ReadUART3();    // Get the character coming from USB UART
        while(BusyUART1());
        WriteUART1(incoming_char);      // Send the character to the GSM UART
        while(BusyUART3());
        WriteUART3(incoming_char);      // Send the character to the USB UART
    }
}
