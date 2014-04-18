/*******************************************************************************
* File Name: ByteSwap_Rx.c  
* Version 1.10
*
* Description:
*  This file contains the setup, control and status commands for the ByteSwap
*  component.  
*
* Note: 
*
*******************************************************************************
* Copyright 2008-2011, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "ByteSwap_Rx.h"  

/*******************************************************************************
* Function Name: ByteSwap_Rx_Start
********************************************************************************
*
* Summary:
*  Starts the Byte swap component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void ByteSwap_Rx_Start(void)
{
	/* Configure Aux control register for FIFO status */
	ByteSwap_Rx_AUX_CONTROL_REG = ByteSwap_Rx_AUX_CONTROL_REG & (~(ByteSwap_Rx_INPUT_FIFO_0_CLR | 
	                                                                        ByteSwap_Rx_OUTPUT_FIFO_1_CLR) );
																			
	ByteSwap_Rx_AUX_CONTROL_REG = ByteSwap_Rx_AUX_CONTROL_REG | ByteSwap_Rx_INPUT_FIFO_LEVEL_HALF_EMPTY;
	
    /* Set Control register enable flag  */
	ByteSwap_Rx_CONTROL_REG = ByteSwap_Rx_CONTROL_REG | ByteSwap_Rx_EN;
        
}

/*******************************************************************************
* Function Name: ByteSwap_Rx_Stop
********************************************************************************
*
* Summary:
*  Stops the Byte swap component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void ByteSwap_Rx_Stop(void)
{
	/* Clear Aux control FIFO status */
	ByteSwap_Rx_AUX_CONTROL_REG = ByteSwap_Rx_AUX_CONTROL_REG | (ByteSwap_Rx_INPUT_FIFO_0_CLR | 
	                                                                        ByteSwap_Rx_OUTPUT_FIFO_1_CLR);
	
    /* Clears the Control register enable flag  */
	ByteSwap_Rx_CONTROL_REG = ByteSwap_Rx_CONTROL_REG & (~ ByteSwap_Rx_EN);
}


/* [] END OF FILE */
