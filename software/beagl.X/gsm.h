/* 
 * File:   gsm.h
 * Author: matt
 *
 * Created on February 21, 2013, 12:15 AM
 */

#ifndef GSM_H
#define	GSM_H

#define false 0
#define true 1

char gsmRead(void);
void gsmSendCommand(char *);
int newSINDreceived(void);
char *lastSIND(void);
int gsmReady(void);
void gsmCall(char *);
void gsmText(char *, char *);

#endif	/* GSM_H */

