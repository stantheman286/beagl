/* 
 * File:   usb.h
 * Author: matt
 *
 * Created on January 24, 2013, 10:38 PM
 */

#ifndef USB_H
#define	USB_H

void uint8ToUSB(uint8_t, int, boolean);
void uint16ToUSB(uint16_t, int, boolean);
void booleanToUSB(boolean);
void doubleToUSB(double);
void strToUSB(char*);

#endif	/* USB_H */

