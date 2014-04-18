/*****************************************************************************
* File Name: USBUART.c
* Version 1.0
*
* Description:
*  This file provides the source code to handle USBUART Bridge functionality
*
* Owner:
*	Krishnaprasad M V (kris@cypress.com)
*
*
* Code Tested With:
* 	Creator 3.0
*	GCC1 4.4.1
*	CY8CKIT-33A Baseboard
* 
******************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation.
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
#include <cytypes.h>
#include <Configuration.h>
#include <USBUART.h>

#ifdef CDC_ENABLED
    
extern volatile uint8 USBFS_cdc_data_in_ep;
extern volatile uint8 USBFS_cdc_data_out_ep;    

/* Gloabl USB UART buffers for mode 3 transfers */
uint8 usbUartRxBuffer[USBINPACKETSIZE];
uint8 usbUartTxBuffer[MAX_USB_OUT_PACKET_SIZE];
uint8 usbUartNotificationBuffer[MAX_NOTIFICATION_PACKET_SIZE];

/*******************************************************************************
* Macro Name: CheckLine
********************************************************************************
* Summary:
* Checks for line change, Reconfigures the UART clock to match host baud rate
*******************************************************************************/

void CheckLine(void) 
{																				                    \
																									\
	uint32 dDTERate;																				\
	uint16 wDivider;																				\
																									\
	if(USBFS_IsLineChanged())																		\
	{																								\
		/* Get Baud Rate */																			\
		dDTERate = USBFS_GetDTERate();  															\
																									\
		/* Check for Baud Rate Upper Limit */														\
		if(dDTERate > 115200)																		\
		{																							\
			dDTERate  = 115200;																		\
		}																							\
																									\
		/* Check for Baud Rate Lower Limit */														\
		if(dDTERate < 1200)																			\
		{																							\
			dDTERate  = 1200;																		\
		}																							\
																									\
		/* Sets the required Clock divider for UART */												\
		switch(dDTERate)																			\
	    {																							\
			case 115200:																			\
	            wDivider = DIVIDER115200;															\
	            break;																				\
	        case 57600:																				\
	            wDivider = DIVIDER57600;															\
	            break;																				\
	        case 38400:																				\
	            wDivider = DIVIDER38400;															\
	            break;																				\
	        case 19200:																				\
	            wDivider = DIVIDER19200;															\
	            break;																				\
	        case 9600:																				\
	            wDivider = DIVIDER9600;																\
	            break;																				\
	        case 4800:																				\
	            wDivider = DIVIDER4800;																\
	            break;																				\
	        case 2400:																				\
	            wDivider = DIVIDER2400;																\
	            break;																				\
	        case 1200:																				\
	            wDivider = DIVIDER1200;																\
	            break;																				\
	        default: 																				\
	            wDivider = DIVIDER115200;															\
	            break;																				\
	    }																							\
																									\
		/* Stop UART for new Clock */																\
	    UART_Bridge_Stop();   																		\
	    																							\
		/* Set new Clock Frequency */																\
		Clock_UART_SetDivider(wDivider-1);															\
	    																							\
		/* Restart UART */																			\
		UART_Bridge_Start();																		\
		UART_Bridge_ClearRxBuffer();																\
		UART_Bridge_ClearTxBuffer();																\
	}																								\
}																									


/*******************************************************************************
* Function Name: KRIS_UART_Bridge_GetTxBufferSize
********************************************************************************
*
* Summary:
*  Determine the amount of space left in the TX buffer and return the count in
*  bytes. 
*  KRIS Updates - The UART component API has a bug when the read and write pointes 
*  are equal, this is my custom implementation (hence the prefix KRIS_)
*
* Parameters:
*  None.
*
* Return:
*  Integer count of the number of bytes left in the TX buffer
*
* Global Variables:
*  UART_Bridge_txBufferWrite - used to calculate left space.
*  UART_Bridge_txBufferRead - used to calculate left space.
*
* Reentrant:
*  No.
*
* Theory:
*  Allows the user to find out how full the TX Buffer is.
*
*******************************************************************************/
uint8 KRIS_UART_Bridge_GetTxBufferSize(void)
{
    uint8 size;

    #if(UART_Bridge_TXBUFFERSIZE > UART_Bridge_FIFO_LENGTH)

        /* Disable Tx interrupt. */
        /* Protect variables that could change on interrupt. */
        #if(UART_Bridge_TX_INTERRUPT_ENABLED)
            UART_Bridge_DisableTxInt();
        #endif /* End UART_Bridge_TX_INTERRUPT_ENABLED */

        if(UART_Bridge_txBufferRead == UART_Bridge_txBufferWrite)
        {
            size = UART_Bridge_TXBUFFERSIZE;
        }
        else if(UART_Bridge_txBufferRead < UART_Bridge_txBufferWrite)
        {
            size = (UART_Bridge_txBufferWrite - UART_Bridge_txBufferRead);
        }
        else
        {
            size = (UART_Bridge_TXBUFFERSIZE - UART_Bridge_txBufferRead) + UART_Bridge_txBufferWrite;
        }

        /* Enable Tx interrupt. */
        #if(UART_Bridge_TX_INTERRUPT_ENABLED)
            UART_Bridge_EnableTxInt();
        #endif /* End UART_Bridge_TX_INTERRUPT_ENABLED */

    #else /* UART_Bridge_TXBUFFERSIZE > UART_Bridge_FIFO_LENGTH */

        size = UART_Bridge_TXSTATUS_REG;

        /* Is the fifo is full. */
        if((size & UART_Bridge_TX_STS_FIFO_FULL) != 0u)
        {
            size = UART_Bridge_FIFO_LENGTH;
        }
        else if((size & UART_Bridge_TX_STS_FIFO_EMPTY) != 0u)
        {
            size = 0u;
        }
        else
        {
            /* We only know there is data in the fifo. */
            size = 1u;
        }

    #endif /* End UART_Bridge_TXBUFFERSIZE > UART_Bridge_FIFO_LENGTH */

    return(size);
}

/*******************************************************************************
* Macro Name: USBUARTTransmit
********************************************************************************
* Summary:
* Checks if USB host has sent data and transfers it to the UART Transmitter 
*
*******************************************************************************/

void USBUARTTransmit(void)   
{																		\
																									\
	uint8 size;																						\
																									\
	/* Check for USB host packet */																	\
	if(USBFS_DataIsReady() && KRIS_UART_Bridge_GetTxBufferSize() >= MAX_USB_OUT_PACKET_SIZE)	            \
	{  																								\
		/* Get size of packet */ 																	\
		size = USBFS_GetCount();										  				            \
																									\
		/* Send to UART Tx */																		\
		UART_Bridge_PutArray(&usbUartTxBuffer[0] , size);								 		    \
                                                                                                    \
        USBFS_EnableOutEP(USBFS_cdc_data_out_ep);                                                   \
	}                           																	\
}


/*******************************************************************************
* Macro Name: USBUARTReceive
********************************************************************************
* Summary:
* Checks if the UART has received data and sends it to the USB host 
*******************************************************************************/

void USBUARTReceive(void)
{																											
	uint8 wCount; 																													
	uint8 wRxBufferRead;																											
	uint8 index;																													
																																	
	if(UART_Bridge_ReadRxStatus() == UART_Bridge_RX_STS_SOFT_BUFF_OVER)																
	{																																\
		UART_Bridge_ClearRxBuffer();																								\
	}																																\
																																	\
	/* Get the Received UART data size */																							\
	wCount = UART_Bridge_GetRxBufferSize();                  		 																\
																																	\
	/* Check if Rx has data and USB has finished sending old data */																\
	if( (wCount != 0) && 																									        \
	( USBFS_CDCIsReady()))																		                                    \
	{																																\
	    /* Limit data size to USB EP Packet Size */																					\
		if( wCount > USBINPACKETSIZE )																								\
	    {																															\
	        wCount = USBINPACKETSIZE;                       																		\
	    }																															\
	    																															\
	    /* Read current Rx Buffer pointer to a temporary pointer */ 																\
		wRxBufferRead = UART_Bridge_rxBufferRead;																					\
	    																															\
	    for( index = 0; index < wCount; index++, wRxBufferRead++)																    \
	    {																															\
	        /* Reset temporary Buffer pointer on overflow */ 																		\
	        if(wRxBufferRead >= UART_Bridge_RXBUFFERSIZE)																			\
	        {																														\
	            wRxBufferRead = 0u;																									\
	        }																														\
			/* Load from UART RX Buffer to USB */ 																					\
	        usbUartRxBuffer[index] = UART_Bridge_rxBuffer[wRxBufferRead]; 								                            \
	    }																															\
	  																																\
		/* Send out data */																											\
	    USBFS_LoadInEP(USBFS_cdc_data_in_ep, USBFS_NULL, wCount);        															\
																																	\
		/* Load new Rx Buffer Pointer */																							\
	    UART_Bridge_rxBufferRead = wRxBufferRead;																					\
		UART_Bridge_rxBufferLoopDetect = 0;																							\
																																	\
	}																																\
}

/*******************************************************************************
* Function Name: UARTBridgeComm()
********************************************************************************
* Summary:
* This function handles the USB-UART bridge interface.
* It handles,
* a) Selection of UART clock based on host baud rate
* b) Transmit host packet to UART TX
* c) Transmit UART RX packet to host
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/

void UARTBridgeComm (void)
{
	/* Check for Line configuration change, Reconfigure UART based on host parameters */
	CheckLine();
	
	/* USB to UART Transmit */
	USBUARTTransmit();
	
    /* UART to USB Receive */
	USBUARTReceive();
}

/*******************************************************************************
* Function Name: USBUARTStart()
********************************************************************************
* Summary:
* Initializes CDC Interface, Sets Tx Pin to Strong Drive
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/

void USBUARTStart (void)
{
    UART_Bridge_Start();
    
    USBFS_ReadOutEP(USBFS_cdc_data_out_ep, &usbUartTxBuffer[0], MAX_USB_OUT_PACKET_SIZE);
    
    USBFS_LoadInEP(USBFS_cdc_data_in_ep, &usbUartRxBuffer[0], USBINPACKETSIZE);
    USBFS_LoadInEP(UART_NOTIFICATION_EP, &usbUartNotificationBuffer[0], MAX_NOTIFICATION_PACKET_SIZE);
    
	/* Initialize CDC Interface for USB-UART Bridge */
	USBFS_CDC_Init();
}

#endif
/* [] END OF FILE */

