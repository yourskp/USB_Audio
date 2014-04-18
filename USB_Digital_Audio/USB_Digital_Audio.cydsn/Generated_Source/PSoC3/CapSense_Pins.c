/*******************************************************************************
* File Name: CapSense_Pins.c
* Version 3.30
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

#include "CapSense_Pins.h"


/*******************************************************************************
* Function Name: CapSense_SetAllSensorsDriveMode
********************************************************************************
*
* Summary:
*  Sets the drive mode for the all pins used by capacitive sensors within 
*  CapSense component.
* 
* Parameters:  
*  mode: Desired drive mode.
*
* Return: 
*  None
*
*******************************************************************************/
void CapSense_SetAllSensorsDriveMode(uint8 mode) 
{
    /* Set pins drive mode */
    CyPins_SetPinDriveMode(CapSense_PortCH0__PSOC_NEXT__BTN, mode);
    CyPins_SetPinDriveMode(CapSense_PortCH0__PSOC_PREV__BTN, mode);
    CyPins_SetPinDriveMode(CapSense_PortCH0__PSOC_SLIDER_e0__LS, mode);
    CyPins_SetPinDriveMode(CapSense_PortCH0__PSOC_SLIDER_e1__LS, mode);
    CyPins_SetPinDriveMode(CapSense_PortCH0__PSOC_SLIDER_e2__LS, mode);
    CyPins_SetPinDriveMode(CapSense_PortCH0__PSOC_SLIDER_e3__LS, mode);
    CyPins_SetPinDriveMode(CapSense_PortCH0__PSOC_SLIDER_e4__LS, mode);
    CyPins_SetPinDriveMode(CapSense_PortCH0__PSOC_PLAY__BTN, mode);
    CyPins_SetPinDriveMode(CapSense_PortCH0__PSOC_LOOPBACK__BTN, mode);

}



/*******************************************************************************
* Function Name: CapSense_SetAllCmodsDriveMode
********************************************************************************
*
* Summary:
*  Sets the drive mode for the all pins used by Cmod capacitors within CapSense 
*  component.
* 
* Parameters:  
*  mode: Desired drive mode.
*
* Return: 
*  None
*
*******************************************************************************/
void CapSense_SetAllCmodsDriveMode(uint8 mode) 
{
   /* Set pins drive mode */
    CyPins_SetPinDriveMode(CapSense_CmodCH0_Cmod_CH0, mode);

}


#if (CapSense_CURRENT_SOURCE == CapSense_EXTERNAL_RB)
    /*******************************************************************************
    * Function Name: CapSense_SetAllRbsDriveMode
    ********************************************************************************
    *
    * Summary:
    *  Sets the drive mode for the all pins used by bleed resistors (Rb) within 
    *  CapSense component. Only available when Current Source is external resistor.
    * 
    * Parameters:  
    *  mode: Desired drive mode.
    *
    * Return: 
    *  None
    *
    *******************************************************************************/
    void CapSense_SetAllRbsDriveMode(uint8 mode) 
    {
        /* Set pins drive mode */
      
    }
#endif  /* (CapSense_CURRENT_SOURCE == CapSense_EXTERNAL_RB) */


/* [] END OF FILE */
