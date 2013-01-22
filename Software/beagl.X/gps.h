/* 
 * File:   gps.h
 * Author: matt
 *
 * Created on November 8, 2012, 11:21 PM
 */

#ifndef GPS_H
#define	GPS_H

#ifdef	__cplusplus
extern "C" {
#endif

/***********************************
This is the Adafruit GPS library - the ultimate GPS library
for the ultimate GPS module!

Tested and works great with the Adafruit Ultimate GPS module
using MTK33x9 chipset
    ------> http://www.adafruit.com/products/746
Pick one up today at the Adafruit electronics shop
and help support open source hardware & software! -ada

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above must be included in any redistribution
****************************************/

/* Typedefs and Definitions to make this compatible with C */
typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef unsigned long uint32_t;
typedef int boolean;

#define false 0
#define true 1

// different commands to set the update rate from once a second (1 Hz) to 10 times a second (10Hz)
#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F\r\n"
#define PMTK_SET_NMEA_UPDATE_5HZ  "$PMTK220,200*2C\r\n"
#define PMTK_SET_NMEA_UPDATE_10HZ "$PMTK220,100*2F\r\n"


#define PMTK_SET_BAUD_57600 "$PMTK250,1,0,57600*2C\r\n"
#define PMTK_SET_BAUD_9600 "$PMTK250,1,0,9600*17\r\n"

// turn on only the second sentence (GPRMC)
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n"
// turn on GPRMC and GGA
#define PMTK_SET_NMEA_OUTPUT_RMCGGA "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"
// turn on ALL THE DATA
#define PMTK_SET_NMEA_OUTPUT_ALLDATA "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"
// turn off output
#define PMTK_SET_NMEA_OUTPUT_OFF "$PMTK314,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n"

// to generate your own sentences, check out the MTK command datasheet and use a checksum calculator
// such as the awesome http://www.hhhh.org/wiml/proj/nmeaxor.html

#define PMTK_LOCUS_STARTLOG  "$PMTK185,0*22\r\n"
#define PMTK_LOCUS_LOGSTARTED "$PMTK001,185,3*3C\r\n"
#define PMTK_LOCUS_QUERY_STATUS "$PMTK183*38\r\n"
#define PMTK_LOCUS_ERASE_FLASH "$PMTK184,1*22\r\n"
#define LOCUS_OVERLAP 0
#define LOCUS_FULLSTOP 1

// ask for the release and version
#define PMTK_Q_RELEASE "$PMTK605*31\r\n"

// how long to wait when we're looking for a response
#define MAXWAITSENTENCE 5

//#if ARDUINO >= 100
// #include "Arduino.h"
// #include "SoftwareSerial.h"
//#else
// #include "WProgram.h"
// #include "NewSoftSerial.h"
//#endif


//class Adafruit_GPS {
// public:

void begin(uint16_t baud);

//#if ARDUINO >= 100
//  Adafruit_GPS(SoftwareSerial *ser); // Constructor when using SoftwareSerial
//#else
//  Adafruit_GPS(NewSoftSerial  *ser); // Constructor when using NewSoftSerial
//#endif
//  Adafruit_GPS(HardwareSerial *ser); // Constructor when using HardwareSerial

char *lastNMEA(void);
boolean newNMEAreceived();
void common_init(void);
void sendCommand(char *);
void pause(boolean b);

boolean parseNMEA(char *response);
uint8_t parseHex(char c);

char read(void);
boolean parse(char *);
void interruptReads(boolean r);

extern uint8_t hour, minute, seconds, year, month, day;
extern uint16_t milliseconds;
extern float latitude, longitude, geoidheight, altitude;
extern float speed, angle, magvariation, HDOP;
extern char lat, lon, mag;
extern boolean fix;
extern uint8_t fixquality, satellites;

boolean waitForSentence(char *wait, uint8_t max);
boolean LOCUS_StartLogger(void);
boolean LOCUS_ReadStatus(void);

extern uint16_t LOCUS_serial, LOCUS_records;
extern uint8_t LOCUS_type, LOCUS_mode, LOCUS_config, LOCUS_interval, LOCUS_distance, LOCUS_speed, LOCUS_status, LOCUS_percent;

extern boolean paused;

uint8_t parseResponse(char *response);

//#if ARDUINO >= 100
//  SoftwareSerial *gpsSwSerial;
//#else
//  NewSoftSerial  *gpsSwSerial;
//#endif
//  HardwareSerial *gpsHwSerial;

#ifdef	__cplusplus
}
#endif

#endif	/* GPS_H */

