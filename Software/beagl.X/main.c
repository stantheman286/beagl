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

// Configuration Settings

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
    // Bring all Outputs Low
    LATB = 0x0000;
    LATD = 0x0000;
    LATF = 0x0000;

    // Disable analog I/O on PORT B
    ANSB = 0x0000;

    // Setup the GPS and USB UART
    usbSetup();
    gpsSetup();
    
    // Clear and enable GPS interrupt
    U2RX_Clear_Intr_Status_Bit;
    EnableIntU2RX;

    // Set up to echo GPS data
    sendCommand(PMTK_SET_BAUD_9600);
    DELAY_MS(100);
    sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    DELAY_MS(100);
    sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    DELAY_MS(100);

    // Start GPS data logger
//    startLOCUS();

    // Loop Forever
    while(1)
    {
        blink(LED_GRN_P42);
        DELAY_MS(1000);
/*
        if (LOCUS_ReadStatus()) {
            while(BusyUART3());
            putsUART3((unsigned int*)"\n\nLog #");
       //FIX  //   Serial.print(GPS.LOCUS_serial, DEC);
            if (LOCUS_type == LOCUS_OVERLAP) {
              while(BusyUART3());
              putsUART3((unsigned int*)", Overlap, ");
            }
            else if (LOCUS_type == LOCUS_FULLSTOP) {
              while(BusyUART3());
              putsUART3((unsigned int*)", Full Stop, Logging");
            }

            if (LOCUS_mode & 0x1) {
                while(BusyUART3());
                putsUART3((unsigned int*)" AlwaysLocate");
            }
            if (LOCUS_mode & 0x2) {
                while(BusyUART3());
                putsUART3((unsigned int*)" FixOnly");
            }
            if (LOCUS_mode & 0x4) {
                while(BusyUART3());
                putsUART3((unsigned int*)" Normal");
            }
            if (LOCUS_mode & 0x8) {
                while(BusyUART3());
                putsUART3((unsigned int*)" Interval");
            }
            if (LOCUS_mode & 0x10) {
                while(BusyUART3());
                putsUART3((unsigned int*)" Distance");
            }
            if (LOCUS_mode & 0x20) {
                while(BusyUART3());
                putsUART3((unsigned int*)" Speed");
            }

            while(BusyUART3());
            putsUART3((unsigned int*)", Content ");
            uint8ToUSB(LOCUS_config);
            while(BusyUART3());
            putsUART3((unsigned int*)", Interval ");
            uint8ToUSB(LOCUS_interval);
            while(BusyUART3());
            putsUART3((unsigned int*)" sec, Distance ");
            uint8ToUSB(LOCUS_distance);
            while(BusyUART3());
            putsUART3((unsigned int*)" m, Speed ");
            uint8ToUSB(LOCUS_speed);
            while(BusyUART3());
            putsUART3((unsigned int*)" m/s, Status ");
            if (LOCUS_status) {
              while(BusyUART3());
              putsUART3((unsigned int*)"LOGGING, ");
            }
            else {
              while(BusyUART3());
              putsUART3((unsigned int*)"OFF, ");
            }
            uint8ToUSB(LOCUS_records);
            while(BusyUART3());
            putsUART3((unsigned int*)" Records, ");
          //  uint16ToUSB(LOCUS_percent);
          //  while(BusyUART3());
          //  putsUART3((unsigned int*)"% Used ");
        }*/
    }

    // Disable Interrupts and close UART
    DisableIntU2RX;
    CloseUART2();
    
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

// UART2 RX ISR
void __attribute__ ((interrupt,no_auto_psv)) _U2RXInterrupt(void)
{
    char c;

    // Clear the interrupt status of UART3 RX
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
        //while(BusyUART3());
        //WriteUART3((unsigned int)c);

        // Clear out any garbage characters
        while(DataRdyUART2())
            ReadUART2();

    }

    // Print out GPS info
    displayGPSInfo();
    
}