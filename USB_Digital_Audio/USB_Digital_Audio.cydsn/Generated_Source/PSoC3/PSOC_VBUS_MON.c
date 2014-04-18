/*******************************************************************************
* File Name: PSOC_VBUS_MON.c  
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
#include "PSOC_VBUS_MON.h"


/*******************************************************************************
* Function Name: PSOC_VBUS_MON_Write
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
void PSOC_VBUS_MON_Write(uint8 value) 
{
    uint8 staticBits = (PSOC_VBUS_MON_DR & (uint8)(~PSOC_VBUS_MON_MASK));
    PSOC_VBUS_MON_DR = staticBits | ((uint8)(value << PSOC_VBUS_MON_SHIFT) & PSOC_VBUS_MON_MASK);
}


/*******************************************************************************
* Function Name: PSOC_VBUS_MON_SetDriveMode
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
void PSOC_VBUS_MON_SetDriveMode(uint8 mode) 
{
	CyPins_SetPinDriveMode(PSOC_VBUS_MON_0, mode);
}


/*******************************************************************************
* Function Name: PSOC_VBUS_MON_Read
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
*  Macro PSOC_VBUS_MON_ReadPS calls this function. 
*  
*******************************************************************************/
uint8 PSOC_VBUS_MON_Read(void) 
{
    return (PSOC_VBUS_MON_PS & PSOC_VBUS_MON_MASK) >> PSOC_VBUS_MON_SHIFT;
}


/*******************************************************************************
* Function Name: PSOC_VBUS_MON_ReadDataReg
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
uint8 PSOC_VBUS_MON_ReadDataReg(void) 
{
    return (PSOC_VBUS_MON_DR & PSOC_VBUS_MON_MASK) >> PSOC_VBUS_MON_SHIFT;
}


/* If Interrupts Are Enabled for this Pins component */ 
#if defined(PSOC_VBUS_MON_INTSTAT) 

    /*******************************************************************************
    * Function Name: PSOC_VBUS_MON_ClearInterrupt
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
    uint8 PSOC_VBUS_MON_ClearInterrupt(void) 
    {
        return (PSOC_VBUS_MON_INTSTAT & PSOC_VBUS_MON_MASK) >> PSOC_VBUS_MON_SHIFT;
    }

#endif /* If Interrupts Are Enabled for this Pins component */ 


/* [] END OF FILE */
