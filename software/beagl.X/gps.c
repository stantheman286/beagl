/***********************************
This is our GPS library

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
BSD license, check license.txt for more information
All text above must be included in any redistribution
****************************************/

// Libraries
#include "beagl.h"

// Global variables
uint8_t hour, minute, seconds, year, month, day;
uint16_t milliseconds;
double latitude, longitude, geoidheight, altitude;
double speed, angle, magvariation, HDOP;
char lat, lon, mag;
boolean fix;
uint8_t fixquality, satellites;

uint16_t LOCUS_serial, LOCUS_records;
uint8_t LOCUS_type, LOCUS_mode, LOCUS_config, LOCUS_interval, LOCUS_distance, LOCUS_speed, LOCUS_status, LOCUS_percent;

boolean paused;

// how long are max NMEA lines to parse?
#define MAXLINELENGTH 120

#define false 0
#define true 1

// we double buffer: read one line in and leave one for the main program
volatile char line1[MAXLINELENGTH];
volatile char line2[MAXLINELENGTH];
// our index into filling the current line
volatile uint8_t lineidx=0;
// pointers to the double buffers
volatile char *currentline;
volatile char *lastline;
volatile boolean recvdflag;


boolean parse(char *nmea) {
   uint8_t i;

  // do checksum check

  // first look if we even have one
  if (nmea[strlen(nmea)-4] == '*') {
    uint16_t sum = parseHex(nmea[strlen(nmea)-3]) * 16;
    sum += parseHex(nmea[strlen(nmea)-2]);

    // check checksum
    for (i=1; i < (strlen(nmea)-4); i++) {
      sum ^= nmea[i];
    }
    if (sum != 0) {
      // bad checksum :(
      //return false;
    }
  }

  // look for a few common sentences
  if (strstr(nmea, "$GPGGA")) {
    // found GGA
    char *p = nmea;
    // get time
    p = strchr(p, ',')+1;
    double timef = atof(p);
    uint32_t time = timef;
    hour = time / 10000;
    minute = (time % 10000) / 100;
    seconds = (time % 100);

    milliseconds = fmod(timef, 1.0) * 1000;

    // parse out latitude
    p = strchr(p, ',')+1;
    latitude = atof(p);

    p = strchr(p, ',')+1;
    if (p[0] == 'N') lat = 'N';
    else if (p[0] == 'S') lat = 'S';
    else if (p[0] == ',') lat = 0;
    else return false;

    // parse out longitude
    p = strchr(p, ',')+1;
    longitude = atof(p);

    p = strchr(p, ',')+1;
    if (p[0] == 'W') lon = 'W';
    else if (p[0] == 'E') lon = 'E';
    else if (p[0] == ',') lon = 0;
    else return false;

    p = strchr(p, ',')+1;
    fixquality = atoi(p);

    p = strchr(p, ',')+1;
    satellites = atoi(p);

    p = strchr(p, ',')+1;
    HDOP = atof(p);

    p = strchr(p, ',')+1;
    altitude = atof(p);
    p = strchr(p, ',')+1;
    p = strchr(p, ',')+1;
    geoidheight = atof(p);
    return true;
  }
  if (strstr(nmea, "$GPRMC")) {
   // found RMC
    char *p = nmea;

    // get time
    p = strchr(p, ',')+1;
    double timef = atof(p);
    uint32_t time = timef;
    hour = time / 10000;
    minute = (time % 10000) / 100;
    seconds = (time % 100);

    milliseconds = fmod(timef, 1.0) * 1000;

    p = strchr(p, ',')+1;
    // Serial.println(p);
    if (p[0] == 'A')
      fix = true;
    else if (p[0] == 'V')
      fix = false;
    else
      return false;

    // parse out latitude
    p = strchr(p, ',')+1;
    latitude = atof(p);

    p = strchr(p, ',')+1;
    if (p[0] == 'N') lat = 'N';
    else if (p[0] == 'S') lat = 'S';
    else if (p[0] == ',') lat = 0;
    else return false;

    // parse out longitude
    p = strchr(p, ',')+1;
    longitude = atof(p);

    p = strchr(p, ',')+1;
    if (p[0] == 'W') lon = 'W';
    else if (p[0] == 'E') lon = 'E';
    else if (p[0] == ',') lon = 0;
    else return false;

    // speed
    p = strchr(p, ',')+1;
    speed = atof(p);

    // angle
    p = strchr(p, ',')+1;
    angle = atof(p);

    p = strchr(p, ',')+1;
    uint32_t fulldate = atof(p);
    day = fulldate / 10000;
    month = (fulldate % 10000) / 100;
    year = (fulldate % 100);

    // we dont parse the remaining, yet!
    return true;
  }

  return false;
}

char read(void) {
  char c = 0;

  if (paused) return c;

  /* Grab data from GPS if available */
  if (!DataRdyUART2()) {
    return c;
  } else {
      c = ReadUART2();
  }

  if (c == '$') {

    currentline[lineidx] = 0;
    lineidx = 0;
  }
  if (c == '\n') {
    currentline[lineidx] = 0;

    if (currentline == line1) {
      currentline = line2;
      lastline = line1;
    } else {
      currentline = line1;
      lastline = line2;
    }

    lineidx = 0;
    recvdflag = true;
  }

  currentline[lineidx++] = c;
  if (lineidx >= MAXLINELENGTH)
    lineidx = MAXLINELENGTH-1;

  return c;

}

// Initialization code used by all constructor types
void common_init(void) {
  recvdflag   = false;
  paused      = false;
  lineidx     = 0;
  currentline = line1;
  lastline    = line2;

  hour = minute = seconds = year = month = day =
    fixquality = satellites = 0; // uint8_t
  lat = lon = mag = 0; // char
  fix = false; // boolean
  milliseconds = 0; // uint16_t
  latitude = longitude = geoidheight = altitude =
    speed = angle = magvariation = HDOP = 0.0; // double
}

/* Send command to GPS via UART */
void sendCommand(char *str) {
    while(BusyUART2());
    putsUART2((unsigned int*)str);
}

boolean newNMEAreceived(void) {
  return recvdflag;
}

void pause(boolean p) {
  paused = p;
}

char *lastNMEA(void) {
  recvdflag = false;
  return (char *)lastline;
}

// read a Hex value and return the decimal equivalent
uint8_t parseHex(char c) {
    if (c < '0')
      return 0;
    if (c <= '9')
      return c - '0';
    if (c < 'A')
       return 0;
    if (c <= 'F')
       return (c - 'A')+10;
    return -1;
}

boolean waitForSentence(char *wait4me, uint8_t max) {
  char str[20];

  uint8_t i=0;
  while (i < max) {
    if (newNMEAreceived()) {
      char *nmea = lastNMEA();
      strncpy(str, nmea, 20);
      str[19] = 0;
      i++;

      if (strstr(str, wait4me))
	return true;
    }
  }

  return false;
}

boolean LOCUS_StartLogger(void) {
  sendCommand(PMTK_LOCUS_STARTLOG);
  recvdflag = false;
  return waitForSentence(PMTK_LOCUS_LOGSTARTED, MAXWAITSENTENCE);
}

boolean LOCUS_ReadStatus(void) {
  sendCommand(PMTK_LOCUS_QUERY_STATUS);

  if (! waitForSentence("$PMTKLOG", MAXWAITSENTENCE))
    return false;

  char *response = lastNMEA();
  uint16_t parsed[10];
  uint8_t i;

  for (i=0; i<10; i++) parsed[i] = -1;

  response = strchr(response, ',');
  for (i=0; i<10; i++) {
    if (!response || (response[0] == 0) || (response[0] == '*'))
      break;
    response++;
    parsed[i]=0;
    while ((response[0] != ',') &&
	   (response[0] != '*') && (response[0] != 0)) {
      parsed[i] *= 10;
      char c = response[0];
      if (isdigit(c))
        parsed[i] += c - '0';
      else
        parsed[i] = c;
      response++;
    }
  }
  LOCUS_serial = parsed[0];
  LOCUS_type = parsed[1];
  if (isalpha(parsed[2])) {
    parsed[2] = parsed[2] - 'a' + 10;
  }
  LOCUS_mode = parsed[2];
  LOCUS_config = parsed[3];
  LOCUS_interval = parsed[4];
  LOCUS_distance = parsed[5];
  LOCUS_speed = parsed[6];
  LOCUS_status = !parsed[7];
  LOCUS_records = parsed[8];
  LOCUS_percent = parsed[9];

  return true;
}

// Start the GPS logger function
void startLOCUS(void)
{
    DELAY_MS(500);
    strToUSB("\nSTARTING LOGGING....");
    if (LOCUS_StartLogger())
        strToUSB(" STARTED!");
    else
        strToUSB(" no response :(");
    DELAY_MS(1000);
}

// Dump GPS logger information
void dumpLOCUSData(void)
{
    DELAY_MS(1000);
    sendCommand(PMTK_LOCUS_LOG_DUMP);
}

// Erase all GPS logger information
void eraseLOCUSData(void)
{
    strToUSB("This code will ERASE the data log stored in the FLASH - Permanently!\n");
    DELAY_MS(5000);
    sendCommand(PMTK_LOCUS_ERASE_FLASH);
    strToUSB("Erased!\n");
}

// Displays GPS logger information to USB
void displayLOCUSInfo(void)
{
    if (LOCUS_ReadStatus()) {
        strToUSB("\n\nLog #");
        uint16ToUSB(LOCUS_serial, 5, false);
        if (LOCUS_type == LOCUS_OVERLAP) strToUSB(", Overlap, ");
        else if (LOCUS_type == LOCUS_FULLSTOP) strToUSB(", Full Stop, Logging");

        if (LOCUS_mode & 0x1) strToUSB(" AlwaysLocate");
        if (LOCUS_mode & 0x2) strToUSB(" FixOnly");
        if (LOCUS_mode & 0x4) strToUSB(" Normal");
        if (LOCUS_mode & 0x8) strToUSB(" Interval");
        if (LOCUS_mode & 0x10) strToUSB(" Distance");
        if (LOCUS_mode & 0x20) strToUSB(" Speed");

        strToUSB(", Content ");
        uint8ToUSB(LOCUS_config, 3, false);
        strToUSB(", Interval ");
        uint8ToUSB(LOCUS_interval, 3, false);
        strToUSB(" sec, Distance ");
        uint8ToUSB(LOCUS_distance, 3, false);
        strToUSB(" m, Speed ");
        uint8ToUSB(LOCUS_speed, 3, false);
        strToUSB(" m/s, Status ");
        if (LOCUS_status) strToUSB("LOGGING, ");
        else strToUSB("OFF, ");
        uint8ToUSB(LOCUS_records, 3, false);
        strToUSB(" Records, ");
        uint16ToUSB(LOCUS_percent, 5, false);
        strToUSB("% Used ");
    }
}

// Displays GPS information to USB
void displayGPSInfo(void)
{
    // Received a valid NMEA sentence
    if (newNMEAreceived()) {

        // If parse successful (checksum passed), print data
        if (parse(lastNMEA())) {

            // Line Break
            strToUSB("\n---------------------------------------------------------------------------------");

            // Time
            strToUSB("\nTime: ");
            uint8ToUSB(hour, 2, true);
            strToUSB(":");
            uint8ToUSB(minute, 2, true);
            strToUSB(":");
            uint8ToUSB(seconds, 2, true);
            strToUSB(".");
            uint16ToUSB(milliseconds, 4, true); // Also max of 1000 (1s, next variable)

            // Date
            strToUSB(" Date: ");
            uint8ToUSB(month, 2, true);
            strToUSB("/");
            uint8ToUSB(day, 2, true);
            strToUSB("/20");   // Make year 4 digits
            uint8ToUSB(year, 2, true);

            /* Location Information */
            strToUSB(" Fix: ");
            booleanToUSB(fix);
            strToUSB(" Quality: ");
            uint8ToUSB(fixquality, 2, false);   // Not using GPGGA, ignore

            // Have a fix, print out useful information
            if(fix) {
                strToUSB("\nLocation: ");
                doubleToUSB(latitude);
                WriteUART3(lat);                // Straight chatacter, no extra conversion
                strToUSB(", ");
                doubleToUSB(longitude);
                WriteUART3(lon);                // Straight chatacter, no extra conversion
                strToUSB(" Speed (knots): ");
                doubleToUSB(speed);
                strToUSB(" Angle: ");
                doubleToUSB(angle);
                strToUSB(" Altitude: ");
                doubleToUSB(altitude);              // Not using GPGGA, ignore
                strToUSB(" Satellites: ");
                uint8ToUSB(satellites, 2, false);   // Not using GPGGA, ignore
            }
        }
    }
}
