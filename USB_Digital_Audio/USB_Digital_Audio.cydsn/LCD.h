/*******************************************************************************
* File Name: LCD.h
*
* Version 4.0
*
* Description: This file contains public APIs and constants for communicating
*              with the I2C 16x2 LCD. The API syntax is similar or same as PSoC 3
*              character LCD component
*
********************************************************************************
* Copyright (2008-2013), Cypress Semiconductor Corporation.
********************************************************************************
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
*******************************************************************************/

#if !defined(LCD_H)
#define LCD_H

#include <project.h>
#include <Application.h>

/*******************************************************************************
* Various defines for Newhaven Display LCD
*******************************************************************************/
#define LCD_ADDRESS                 0x3Au
#define LCD_COMMAND_SEND            0x00u
#define LCD_DATA_SEND               0x40u
#define LCD_WAKEUP                  0x30u
#define LCD_FUNCTION_BASIC          0x34u /* 8 bits data, 2x16 display, use basic instruction set */
#define LCD_FUNCTION_EXTENDED       0x35u /* 8 bits data, 2x16 display, use extended instruction set */
#define LCD_CONTRAST                0x25u
#define LCD_ENTRY_MODE              0x06u
#define LCD_ICON_CONTROL            0x06u
#define LCD_CLEAR                   0x01u
#define LCD_CURSOR_HOME             0x02u
#define LCD_DISPLAY_CURSOR_ON       0x0Cu
#define LCD_DISPLAY_OFF             0x08u
#define LCD_BIAS_VOLTAGE            0xA0u
#define LCD_2X_MULTIPLIER           0x40u

#define LCD_LINE1_BASE              0x80u
#define LCD_LINE2_BASE              0xC0u

#define LCD_CHARACTERS_PER_LINE     16u

#define LCD_CHARACTER_SET_S_MASK    0x80

#define MAX_NUMBER_PER_BYTE         4
#define DECIMAL_DIVIDER             10

#define LCD_LINE1_MAX_CHARACTER     16
#define LCD_LINE2_MAX_CHARACTER     32

#define LCD_FIRST_ROW               0
#define LCD_SECOND_ROW              1

#define LCD_REFRESH_INTERVAL_COUNT  25

#define LCD_RESET_HIGH              1
#define LCD_RESET_LOW               0
#define LCD_RESET_TIME              1

void LCD_Start          (void);
void LCD_PrintString    (char *pch);
void LCD_PutChar        (char ch);
void LCD_PrintNumber    (uint8 number);
void LCD_Position       (uint8 row, uint8 column);
void LCD_ClearDisplay   (void);
void LCD_Refresh        (void);
void LCD2LineDisplay    (char* sFirstLineString, char* sSecondLineString); /* Displays parameter strings on character LCD*/

void ReadFromSlave(uint8 address, uint8 bStartIndex, uint8 bLength, uint8 *abReadBuffer);
void WriteToSlave(uint8 address, uint8* abWriteBuffer, uint8 bLength);
void LCD_Reset(void);

#endif

/* [] END OF FILE */
