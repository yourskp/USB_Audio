/*****************************************************************************
* File Name: USBUART.h
* Version 2.03
*
* Description:
*  This file contains the function prototypes and constants used in
*  the USBUART.c.
*
* Note:
*
******************************************************************************
* Copyright (2013), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/
#if !defined(USBUART_H) 
#define USBUART_H 

/*****************************************************************************
* Function Prototypes
*****************************************************************************/
void UARTBridgeComm(void);
void USBUARTStart(void);

/*****************************************************************************
* API Constants
*****************************************************************************/
/* CDC interface endpoints */
#define UART_NOTIFICATION_EP 	    0x06
#define UART_IN_EP	 				0x07
#define UART_OUT_EP 				0x08

/* UART Source Clock Frequency */
#define SOURCECLK 			24000000  												

/* Divider for Baud Rates */
#define DIVIDER115200 		SOURCECLK/(UART_Bridge_OVER_SAMPLE_COUNT * 115200) 		
#define DIVIDER57600 		SOURCECLK/(UART_Bridge_OVER_SAMPLE_COUNT * 57600) 		
#define DIVIDER38400 		SOURCECLK/(UART_Bridge_OVER_SAMPLE_COUNT * 38400) 		
#define DIVIDER19200 		SOURCECLK/(UART_Bridge_OVER_SAMPLE_COUNT * 19200) 		
#define DIVIDER9600 		SOURCECLK/(UART_Bridge_OVER_SAMPLE_COUNT * 9600) 		
#define DIVIDER4800 		SOURCECLK/(UART_Bridge_OVER_SAMPLE_COUNT * 4800) 		
#define DIVIDER2400 		SOURCECLK/(UART_Bridge_OVER_SAMPLE_COUNT * 2400) 		
#define DIVIDER1200 		SOURCECLK/(UART_Bridge_OVER_SAMPLE_COUNT * 1200) 		

/* USB IN EndPoint Packet size */
#define USBINPACKETSIZE		                64
#define MAX_USB_OUT_PACKET_SIZE             64
#define MAX_NOTIFICATION_PACKET_SIZE        8

/*****************************************************************************
* Global Variable Declaration
*****************************************************************************/
extern volatile T_USBFS_EP_CTL_BLOCK USBFS_EP[];

/*****************************************************************************
* Enumerated Data Definition
*****************************************************************************/


#endif /* USBUART.h */
