/*******************************************************************************
* File Name: FM_I2S_IN.c  
* Version 1.90
*
* Description:
*  This file contains API to enable firmware control of a Pins component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "cytypes.h"
#include "FM_I2S_IN.h"


/*******************************************************************************
* Function Name: FM_I2S_IN_Write
********************************************************************************
*
* Summary:
*  Assign a new value to the digital port's data output register.  
*
* Parameters:  
*  prtValue:  The value to be assigned to the Digital Port. 
*
* Return: 
*  None 
*  
*******************************************************************************/
void FM_I2S_IN_Write(uint8 value) 
{
    uint8 staticBits = (FM_I2S_IN_DR & (uint8)(~FM_I2S_IN_MASK));
    FM_I2S_IN_DR = staticBits | ((uint8)(value << FM_I2S_IN_SHIFT) & FM_I2S_IN_MASK);
}


/*******************************************************************************
* Function Name: FM_I2S_IN_SetDriveMode
********************************************************************************
*
* Summary:
*  Change the drive mode on the pins of the port.
* 
* Parameters:  
*  mode:  Change the pins to this drive mode.
*
* Return: 
*  None
*
*******************************************************************************/
void FM_I2S_IN_SetDriveMode(uint8 mode) 
{
	CyPins_SetPinDriveMode(FM_I2S_IN_0, mode);
}


/*******************************************************************************
* Function Name: FM_I2S_IN_Read
********************************************************************************
*
* Summary:
*  Read the current value on the pins of the Digital Port in right justified 
*  form.
*
* Parameters:  
*  None 
*
* Return: 
*  Returns the current value of the Digital Port as a right justified number
*  
* Note:
*  Macro FM_I2S_IN_ReadPS calls this function. 
*  
*******************************************************************************/
uint8 FM_I2S_IN_Read(void) 
{
    return (FM_I2S_IN_PS & FM_I2S_IN_MASK) >> FM_I2S_IN_SHIFT;
}


/*******************************************************************************
* Function Name: FM_I2S_IN_ReadDataReg
********************************************************************************
*
* Summary:
*  Read the current value assigned to a Digital Port's data output register
*
* Parameters:  
*  None 
*
* Return: 
*  Returns the current value assigned to the Digital Port's data output register
*  
*******************************************************************************/
uint8 FM_I2S_IN_ReadDataReg(void) 
{
    return (FM_I2S_IN_DR & FM_I2S_IN_MASK) >> FM_I2S_IN_SHIFT;
}


/* If Interrupts Are Enabled for this Pins component */ 
#if defined(FM_I2S_IN_INTSTAT) 

    /*******************************************************************************
    * Function Name: FM_I2S_IN_ClearInterrupt
    ********************************************************************************
    *
    * Summary:
    *  Clears any active interrupts attached to port and returns the value of the 
    *  interrupt status register.
    *
    * Parameters:  
    *  None 
    *
    * Return: 
    *  Returns the value of the interrupt status register
    *  
    *******************************************************************************/
    uint8 FM_I2S_IN_ClearInterrupt(void) 
    {
        return (FM_I2S_IN_INTSTAT & FM_I2S_IN_MASK) >> FM_I2S_IN_SHIFT;
    }

#endif /* If Interrupts Are Enabled for this Pins component */ 


/* [] END OF FILE */
