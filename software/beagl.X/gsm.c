// Libraries
#include "beagl.h"

#define SINDMAXLINELENGTH 100   //ms: TEST


// we double buffer: read one line in and leave one for the main program
volatile char gsmLine1[SINDMAXLINELENGTH];
volatile char gsmLine2[SINDMAXLINELENGTH];
// our index into filling the current line
volatile uint8_t gsmLineIdx=0;
// pointers to the double buffers
volatile char *gsmCurrentLine;
volatile char *gsmLastLine;
volatile boolean gsmRcvdFlag;

/* Read GSM data */
char gsmRead(void) {
  char c = 0;

  /* Grab data from GSM if available */
  if (!DataRdyUART1()) {
    return c;
  } else {
      c = ReadUART1();
  }

  if (c == '+') {

    gsmCurrentLine[gsmLineIdx] = 0;
    gsmLineIdx = 0;
  }
  if (c == '\n') {
    gsmCurrentLine[gsmLineIdx] = 0;

    if (gsmCurrentLine == gsmLine1) {
      gsmCurrentLine = gsmLine2;
      gsmLastLine = gsmLine1;
    } else {
      gsmCurrentLine = gsmLine1;
      gsmLastLine = gsmLine2;
    }

    gsmLineIdx = 0;
    gsmRcvdFlag = true;
  }

  gsmCurrentLine[gsmLineIdx++] = c;
  if (gsmLineIdx >= SINDMAXLINELENGTH)
    gsmLineIdx = SINDMAXLINELENGTH-1;

  return c;

}


/* Send command to GSM via UART */
void gsmSendCommand(char *str) {
    while(BusyUART1());
    putsUART1((unsigned int*)str);
}

/* Detect when a new SIND sentence is detected */
int newSINDreceived(void) {
  return gsmRcvdFlag;
}

/* Returns last SIND sentence and resets receive flag */
char *lastSIND(void) {
  gsmRcvdFlag = false;
  return (char *)gsmLastLine;
}

/* Detects if GSM module is ready for service */
int gsmReady(void) {
  char *sind = lastSIND();

  return (strstr(sind, "+SIND: 4"));
}

/* Calls a phone number */
void gsmCall(void) {
  gsmSendCommand("ATD2066397758");
}