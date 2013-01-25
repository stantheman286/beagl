
// Libraries
#include "beagl.h"

// Sends uint8 out on USB
void uint8ToUSB(uint8_t n)
{
    // Value is 0 - 59 (seconds, minutes) or 0 - 23 (hours), calculate places and convert to ASCII

    // Calculate the tens and ones place
    while(BusyUART3());
    WriteUART3((unsigned int)((n / 10) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)((n % 10) + 48));

}

// Sends uint16 out on USB
void uint16ToUSB(uint16_t n)
{
    // Value is 0 - 1000, calculate places and convert to ASCII

    // Calculate the thousands, hundreds, tens and ones place
    while(BusyUART3());
    WriteUART3((unsigned int)((n / 1000) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)(((n % 1000) / 100) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)(((n % 100) / 10) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)((n % 10) + 48));

}

// Sends boolean out on USB
void booleanToUSB(boolean b)
{

    // Output true or false based on value
    while(BusyUART3());
    if (b == true)
        putsUART3((unsigned int*)"true");
    else
        putsUART3((unsigned int*)"false");

}

// Sends double out on USB
void doubleToUSB(double d)
{
    unsigned int n_whole, n_dec;

    // Value always has 10 digits as follows: XXXXX.XXXX
    n_whole = (unsigned int)d;
    n_dec = (unsigned int)((d - n_whole)*10000);

    // Calculate the ten thounsands, thousands, hundreds, tens and ones place
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

    // Calculate the tenths, hundredths, thousandths, ten thousandths place
    while(BusyUART3());
    WriteUART3((unsigned int)((n_dec / 1000) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)(((n_dec % 1000) / 100) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)(((n_dec % 100) / 10) + 48));
    while(BusyUART3());
    WriteUART3((unsigned int)((n_dec % 10) + 48));

}

// Sends string out on USB
void strToUSB(char *s)
{
    // Wait for USB to become available
    while(BusyUART3());

    // Output string to USB
    putsUART3((unsigned int*)s);
}
