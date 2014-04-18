/*******************************************************************************
* File Name: iAP2_PM.c
*
* Version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* Description: This file contains functions for handling sleep mode command for
*              iAP component
*
********************************************************************************
* Copyright 2009-2010, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
********************************************************************************/
#include <MyDevice.h>

iAP2_BACKUP_STRUCT iAP2_State;

/*******************************************************************************
* Function Name: iAP2_SaveConfig
********************************************************************************
* Summary: Saves some of the registers which needs to be restored back while
*          coming out of sleep
*
* Parameters:
*  		  void
*
* Return:
*  		  void
*
*******************************************************************************/
void iAP2_SaveConfig(void)
{
	/* Add code in future to save any of the registers which needs to be restored */
}


/*******************************************************************************
* Function Name: iAP2_RestoreConfig
********************************************************************************
* Summary: Restores all the backed up registers after coming out of sleep mode
*
* Parameters:
*  		  void
*
* Return:
*  		  void
*
*******************************************************************************/
void iAP2_RestoreConfig(void)
{
	/* Add code in future to restore any of the registers which were preserved
	   in the iAP2_BACKUP_STRUCT structure before going to sleep */
}


/*******************************************************************************
* Function Name: iAP2_Sleep
********************************************************************************
* Summary: Configures the iAP component to enter sleep state
*
* Parameters:
*  		  void
*
* Return:
*  		  void
*
*******************************************************************************/
void iAP2_Sleep(void)
{
	uint8 bTempState = iAP2_State.bEnableState;
	
	/* Put the underlying components into sleep mode */
	#if(iAP2_IAP_TRANSPORT_PROTOCOL != iAP2_IAP_USB)
	
	iAP2_UART_Sleep();
	
	#endif
	
	#if(iAP2_AUTHENTICATION_SUPPORTED)
	
	iAP2_I2C_Master_Sleep();
	
	#endif
	
	/* Save the register states which are needed after coming out of sleep */
    iAP2_SaveConfig();
	
	/* Restore original state of the component, for proper wakeup */ 
	iAP2_State.bEnableState = bTempState;
}


/*******************************************************************************
* Function Name: iAP2_Wakeup
********************************************************************************
* Summary: Configures the iAP component to its active mode state
*
* Parameters:
*  		  void
*
* Return:
*  		  void
*
*******************************************************************************/
void iAP2_Wakeup(void)
{
	/*Restore the register configuration of the iAP component*/
	iAP2_RestoreConfig();
	
	if(iAP2_State.bEnableState == 1u) 
    { 
        /* iAP block was enabled before going to sleep, reenable it*/
		
		#if(iAP2_IAP_TRANSPORT_PROTOCOL != iAP2_IAP_USB)
		
		iAP2_UART_Wakeup();
		
		#endif
		
		#if(iAP2_AUTHENTICATION_SUPPORTED)
		
		iAP2_I2C_Master_Wakeup();
		
		#endif
    }
}