/*******************************************************************************
* File Name: LCD.c
*
* Version 4.0
*
* Description: This file contains routines which handles the communication 
*              interface with the 16x2 I2C LCD module. All the necessary wrappers 
*              for communicating with the LCD module are part of this file. The 
*              APIs in this file are similar or same as PSoC 3 LCD component APIs
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
#include <device.h>
#include <Configuration.h>
#include <LCD.h>

#ifdef LCD_MODULE_ENABLED

uint8 lcdCursorPos;             /* LCD display position */
uint8 lcdLocalDataArray[32];    /* 2x16 LCD data RAM */
uint8 lcdRefreshInterval = LCD_REFRESH_INTERVAL_COUNT; /* How often to refresh LCD display */

static void LCD_SendCmd1(uint8 cmd);
static void LCD_SendData(uint8 dataByte);

/*******************************************************************************
* Function Name: LCD_PutChar
********************************************************************************
* Summary:
*   Write one character on to LCD display, update local cursor position so can 
*   wrap to next line.
*
* Parameters:
*  txdata - character to write to LCD
*
* Return:
*  void
*******************************************************************************/
void LCD_PutChar(char txdata)
{
    #define LCD_NO_POS  0xFF
    CYDATA uint8 lcdCmd;

    /***************************************************************************
    * Manage cursor position if extended beyond an end-of-line
    ***************************************************************************/
    switch (lcdCursorPos)
    {
        case LCD_LINE1_MAX_CHARACTER:
            lcdCmd = LCD_LINE2_BASE;                        
        break;
        
        case LCD_LINE2_MAX_CHARACTER:
            lcdCmd = LCD_LINE1_BASE;
            lcdCursorPos = 0;  
        break;
        
        default:    
            lcdCmd = LCD_NO_POS;
        break;
    }
    
    if (lcdCmd != LCD_NO_POS)
    {
        /* Set the DDRAM address to current cursor position */
        LCD_SendCmd1(lcdCmd);
    }

    /***************************************************************************
    * Send character for display
    ***************************************************************************/
    LCD_SendData(txdata);

    ++lcdCursorPos;
}

/*******************************************************************************
* Function Name: LCD_SendData
********************************************************************************
*
* Summary:
*  Writes a data byte to the buffer (with a "command" prefix) to LCD
*
* Parameters:
*  dataByte - data to send
*
* Return:
*  void
*******************************************************************************/
static void LCD_SendData(uint8 dataByte) 
{
    uint8 dataBuffer[2];
    dataBuffer[0] = LCD_DATA_SEND;
    dataBuffer[1] = LCD_CHARACTER_SET_S_MASK ^ dataByte;
    WriteToSlave(LCD_ADDRESS, &dataBuffer[0], sizeof(dataBuffer));
    
    /* Store the LCD display area data in a local buffer for refreshing later */
    lcdLocalDataArray[lcdCursorPos] = dataByte;
}

/*******************************************************************************
* Function Name: LCD_SendCmd1
********************************************************************************
*
* Summary:
*  Sends a command (with a "command" prefix) to LCD
*
* Parameters:
*  cmd - command to send
*
* Return:
*  void
*******************************************************************************/
static void LCD_SendCmd1(uint8 cmd) 
{
    uint8 cmdBuffer[2];
    
    cmdBuffer[0] = LCD_COMMAND_SEND;
    cmdBuffer[1] = cmd;
    WriteToSlave(LCD_ADDRESS, &cmdBuffer[0], sizeof(cmdBuffer));
}

/*******************************************************************************
* Function Name: LCD_Start
********************************************************************************
*
* Summary:
*  Initialize the LCD by configuring the LCD module through I2C
*
* Parameters:
*  void
*
* Return:
*  void
*******************************************************************************/
void LCD_Start(void)
{
    /* Initialization data buffer */
    const uint8 lcdInitBuffer[6] = {LCD_COMMAND_SEND, LCD_FUNCTION_EXTENDED, LCD_BIAS_VOLTAGE, 
                                    LCD_FUNCTION_BASIC, LCD_DISPLAY_CURSOR_ON, LCD_ENTRY_MODE};

    CyDelay(1); /* Provide enough time for LCD reset */
    
    /* Configure the LCD at startup */
    WriteToSlave(LCD_ADDRESS, &lcdInitBuffer[0], sizeof(lcdInitBuffer));

    /* Initialize the cursor position */
    lcdCursorPos = 0;
}

/*******************************************************************************
* Function Name: LCD_ClearDisplay
********************************************************************************
*
* Summary:
*  Clear the LCD display screen and reset the cursor pointers
*
* Parameters:
*  void
*
* Return:
*  void
*******************************************************************************/
void LCD_ClearDisplay(void)
{
    uint8 loopIndex;
    
    LCD_SendCmd1(LCD_CLEAR);
    lcdCursorPos = 0;
    
    for(loopIndex = 0; loopIndex <LCD_LINE2_MAX_CHARACTER; loopIndex++)
    {
        lcdLocalDataArray[loopIndex] = ' ';
    }    
}

/*******************************************************************************
* Function Name: LCD_Position
********************************************************************************
*
* Summary:
*  Set the LCD display cursor position to row and column indicated by the 
*  parameters
*
* Parameters:
*  row - cursor row number
*  column - cursor column number
*
* Return:
*  void
*******************************************************************************/
void LCD_Position(uint8 row, uint8 column)
{
    CYDATA uint8 lcdCmd;
    switch (row)
    {
        case LCD_FIRST_ROW:
            lcdCursorPos = column;
            lcdCmd = LCD_LINE1_BASE + column;
            break;
        case LCD_SECOND_ROW:
            lcdCursorPos = LCD_CHARACTERS_PER_LINE + column;
            lcdCmd = LCD_LINE2_BASE + column;
            break;

        default:
            break;
    }
    
    /* set the DDRAM address pointer to current position */
    LCD_SendCmd1(lcdCmd);
}

/*******************************************************************************
* Function Name: LCD_PrintString
********************************************************************************
*
* Summary:
*  Prints the parameter string on the LCD screen starting from the currently 
*  set cursor position
*
* Parameters:
*  pch - Pointer to the string which should be displayed on the LCD
*
* Return:
*  void
*******************************************************************************/
void LCD_PrintString(char *pch)
{
    while (*pch != 0)
    {
        LCD_PutChar(*pch);
        ++pch;
    }
}

/*******************************************************************************
* Function Name: LCD_PrintNumber
********************************************************************************
*
* Summary:
*  Accepts an 8 bit variable and displays the decimal equivalent of the number
*  on the LCD
*
* Parameters:
*  number - 8 bit number to be printed on the LCD
*
* Return:
*  void
*******************************************************************************/
void LCD_PrintNumber(uint8 number)
{
    uint8 charArray[MAX_NUMBER_PER_BYTE];
    uint8 index = MAX_NUMBER_PER_BYTE-1;
    
    /* String terminator */
    charArray[index] = 0;
    
    do 
    {
        index--;
        charArray[index] = (number % DECIMAL_DIVIDER) + '0';
        number = number / DECIMAL_DIVIDER;
    }while(number);
    
    LCD_PrintString(&charArray[index]);
}

/*******************************************************************************
* Function Name: LCD2LineDisplay
********************************************************************************
* Summary:
*        This function displays 1st and 2nd parameter strings on 1st and 2nd 
*           line respectively of 16x2 character LCD display connected to the DVK
*
* Parameters:
*        sFirstLineString - String to be displayed on the first line of the LCD
*        sSecondLineString -String to be displayed on the second line of the LCD
*
* Return:
*  void
*
*******************************************************************************/
void LCD2LineDisplay(char* sFirstLineString, char* sSecondLineString)
{
    LCD_Position(0,0);
    LCD_PrintString(sFirstLineString);
    LCD_Position(1,0);
    LCD_PrintString(sSecondLineString);
}

/*******************************************************************************
* Function Name: LCD_Refresh
********************************************************************************
* Summary:
*        This function refreshes the LCD display area to recover from any LCD
*        display errors because of a data communication mismatch or ESD strike
*        events on the LCD glass
*
* Parameters:
*        None
*
* Return:
*        None
*
*******************************************************************************/
void LCD_Refresh(void)
{
    uint8 loopIndex = 0;
    LCD_Position(0,0);
    for(loopIndex = 0; loopIndex < LCD_LINE2_MAX_CHARACTER; loopIndex++)
    {
        LCD_PutChar(lcdLocalDataArray[loopIndex]);
    }
}

/*******************************************************************************
* Function Name: LCD_Reset
********************************************************************************
* Summary:
*        This function sets the PSOC_LCD_RST pin to HIGH for LCD_RESET_TIME (ms)
*        and then to LOW so as to reset the I2C LSD module
*
* Parameters:
*        None
*
* Return:
*        None
*
*******************************************************************************/
void LCD_Reset(void)
{
    PSOC_LCD_RST_Write(LCD_RESET_HIGH);
    CyDelay(LCD_RESET_TIME);
    PSOC_LCD_RST_Write(LCD_RESET_LOW);
}
#endif

/* [] END OF FILE */
