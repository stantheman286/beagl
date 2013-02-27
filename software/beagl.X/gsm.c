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
    while(BusyUART3());
    putsUART3((unsigned int*)str);
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
void gsmCall(char *number) {

    /* Store the beginning and ending sequences */
    char *prefix = "ATD";
    char *suffix = "\r\n";

    /* Allocate space and then store concatenated call command */
    size_t len1 = strlen(prefix);
    size_t len2 = strlen(number);
    size_t len3 = strlen(suffix);

    char *s = malloc(len1 + len2 + len3 + 1);
    memcpy(s, prefix, len1);
    memcpy(s + len1, number, len2);
    memcpy(s + len1 + len2, suffix, len3 + 1);

    /* Send call */
    gsmSendCommand(s);

    /* Free memory */
    free(s);
}

/* Text a phone number */
void gsmText(char *number, char *msg) {

    /* Store the beginning and ending sequences */
    char *prefix = "AT+CMGS=\"";
    char *suffix = "\"\r\n";

    /* Allocate space and then store concatenated text command */
    size_t len1 = strlen(prefix);
    size_t len2 = strlen(number);
    size_t len3 = strlen(suffix);

    char *s = malloc(len1 + len2 + len3 + 1);
    memcpy(s, prefix, len1);
    memcpy(s + len1, number, len2);
    memcpy(s + len1 + len2, suffix, len3 + 1);

    /* Set GSM module to text mode */
    gsmSendCommand("AT+CMGF=1\r");

    /* Set the number */
    gsmSendCommand(s);

    /* Delay to allow GSM module to get ready */
    DELAY_MS(100);
    
    /* Set the message */
    gsmSendCommand(msg);

    /* Send Text */
    while(BusyUART1());
    WriteUART1((unsigned int)CTRL_Z);

    /* Free memory */
    free(s);
}
