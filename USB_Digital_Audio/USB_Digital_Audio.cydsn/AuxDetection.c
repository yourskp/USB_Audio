/*******************************************************************************
* File Name: AuxDetection.c 
*
* Version 4.0
*
*  Description: This file contains the routines for automatic aux detection 
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
#include <Application.h>
#include <AudioControl.h>
#include <project.h>
#include <USBInterface.h>

#ifdef AUX_DETECTION_ENABLE

CY_ISR_PROTO(Aux_Interrupt);

#define AUX_THRESHOLD       0
#define AUX_SAMPLE_WINDOW   10
#define AUX_TD_SIZE         288

extern uint8 I2SRxMDAChan;
extern uint8 setRate;
extern uint8 USBiAP_initVar ;
extern CYBIT inAuxMode;

CYBIT auxDetectionEnabled = 1;

uint8 auxDMA_Td;
uint8 auxBuffer[AUX_TD_SIZE];
uint8 auxTermOut = (I2S_Rx_DMA__TERMOUT0_EN ? TD_TERMOUT0_EN : 0) | (I2S_Rx_DMA__TERMOUT1_EN ? TD_TERMOUT1_EN : 0);
uint8 debug = 0;

uint8 auxFirstDetected=0;
uint8 auxDetected=0;
uint16 auxTimer=0;
uint8 runFilter = 0;
uint8 countAux=0, countAuxSuccess=0;

uint16 auxTriggerafterSampleRateChangeTimer = 0;/* CORRUPT_AUX_DATA_AFTER_RATE_CHANGE_TIMER; */

/*******************************************************************************
* Function Name: AuxDetection_Initialization
********************************************************************************
* Summary:
*       This routine sets up the DMA path for obtaining the AUX I2S data in
*		SRAM buffer for comparing.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void AuxDetection_Initialization(void)
{
    uint16 index;
    
    I2SRxMDAChan = I2S_Rx_DMA_DmaInitialize(1, 1, HI16(CYDEV_PERIPH_BASE),HI16(CYDEV_SRAM_BASE));
    auxDMA_Td = CyDmaTdAllocate();
    CyDmaTdSetConfiguration(auxDMA_Td, AUX_TD_SIZE, auxDMA_Td, TD_INC_DST_ADR | auxTermOut);
    CyDmaTdSetAddress(auxDMA_Td, (uint16)(I2S_RX_FIFO_0_PTR), (uint16)auxBuffer );
    CyDmaChSetInitialTd(I2SRxMDAChan, auxDMA_Td);  
   
    for(index = 0; index < AUX_TD_SIZE; index++)
    {
        auxBuffer[index] = 0;
    }
}

/*******************************************************************************
* Function Name: AuxDetection_Start
********************************************************************************
* Summary:
*       This routine starts the AUX detection process and enable I2S receive.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void AuxDetection_Start(void)
{
    isr_Aux_StartEx(Aux_Interrupt);

    CyDmaChEnable(I2SRxMDAChan, 1);
    I2S_ClearRxFIFO();  
    I2S_RX_AUX_CONTROL_REG = I2S_RX_AUX_CONTROL_REG & (~FIFO_HALF_EMPTY_MASK);
    I2S_EnableRx();
}

/*******************************************************************************
* Function Name: Aux_Detect_Reinitiate
********************************************************************************
* Summary:
*       This routine resets the flags and audio buffer related to AUX detection.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void Aux_Detect_Reinitiate(void)
{
  uint16 index;
 
	 auxFirstDetected=0;
	 auxDetected=0;
	 auxTimer=0;
	 runFilter = 0;
	 countAux = 0;
	 countAuxSuccess = 0;
	 for(index = 0; index < AUX_TD_SIZE; index++)
	{
	    auxBuffer[index] = 0;
	}
}

/*******************************************************************************
* Function Name: DetectAuxPresence
********************************************************************************
* Summary:
*       This routine runs the logic to detect whether AUX audio is present or not.
*		sets appropriate flag if AUX detected.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void DetectAuxPresence(void)
{
    uint8 byte[6];
	uint8 channel_1_flag, channel_2_flag;

	if(auxDetectionEnabled && !auxTriggerafterSampleRateChangeTimer)
	{
        
		byte[0] = auxBuffer[AUX_TD_SIZE/2] ;
        byte[1] = auxBuffer[(AUX_TD_SIZE/2)+1] ;
        byte[2] = auxBuffer[(AUX_TD_SIZE/2)+2] ;
        byte[3] = auxBuffer[(AUX_TD_SIZE/2)+3] ;
		byte[4] = auxBuffer[(AUX_TD_SIZE/2)+4] ;
		byte[5] = auxBuffer[(AUX_TD_SIZE/2)+5] ;
		
        countAux++;
        
		/* Checking the aux detection on first channel */
	    channel_1_flag=0;
        if ( (0xFF == byte[0]))
        {
            if (byte[1] < 192 )
            {
                channel_1_flag=1;
            }
        }
        else if(0x00 == byte[0] )
        {
            if (byte[1] > 64 )
            {
                channel_1_flag=1;
            }
        }
        else
        {
             channel_1_flag=1;
        }
			
		channel_2_flag=0;
		/* Checking the aux detection on second channel */
		if ( (0xFF == byte[3]))
        {
            if (byte[4] < 192 )
            {
                channel_2_flag=1;
			}
        }
        else if(0x00==byte[3])
        {
            if (byte[4] > 64 )
            {
                channel_2_flag=1;
            }
        }
        else
        {
            channel_2_flag=1;
        }
		if (channel_1_flag || channel_2_flag)	
		{
		    countAuxSuccess++;
		}
		else
		{
		
		}
        
        if (AUX_SAMPLE_WINDOW == countAux)
        {
            if (countAuxSuccess > AUX_THRESHOLD)
            {
                auxFirstDetected=1;
            }
            else
            {
                auxFirstDetected=0;
            }
            countAux = 0; 
            countAuxSuccess=0; 
            
            runFilter = 1;
        }
	}
	else
	{
		auxFirstDetected = 0;
		auxTimer = 0;
	}
}

/*******************************************************************************
* Function Name: AuxDetectFilter
********************************************************************************
* Summary:
*       This routine runs the firmware filter for AUX detection logic to prevent
*		false trigger.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void AuxDetectFilter(void)
{
	if(auxDetectionEnabled && runFilter && !auxTriggerafterSampleRateChangeTimer)
	{    
		if (auxFirstDetected)
	    {
	        if (auxTimer < 400)
	        {
	            auxTimer++;
	        }    
	    }
	    else
	    {
	       if (auxTimer > 0)
	       {
	           auxTimer--;
	       }
	           
	    }
        
	    if (auxTimer > 100)
	    {
	        auxDetected = 1; 
	    }
	    else if (auxTimer < 20)
	    {
	        auxDetected = 0; 
	    }
        
        runFilter = 0;
	}
}

/*******************************************************************************
* Function Name: DisableAuxDetection
********************************************************************************
* Summary:
*       This routine clears the AUX detection flag.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void DisableAuxDetection(void)
{
	auxDetectionEnabled = 0;
}

/*******************************************************************************
* Function Name: EnableAuxDetection
********************************************************************************
* Summary:
*       This routine sets the AUX detection flag.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void EnableAuxDetection(void)
{
	auxDetectionEnabled = 1;
}

/*******************************************************************************
* Function Name: ServiceAuxActivity
********************************************************************************
* Summary:
*       This routine handles automatic switching between AUX and digital audio
*		mode depending on AUX detection logic.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ServiceAuxActivity(void)
{
	AuxDetectFilter();
		
	if(auxDetected && !inAuxMode)
	{
        I2S_Out_Select_Write(I2S_IN_TO_I2S_OUT);
		
		inAuxMode = 1;
        
        if(USBiAP_initVar)  
        {
           iAP2_Stop();
            
           USBDeviceState = USB_INTERFACE_INACTIVE;
         }
                       
        /* start the I2C interface for communicating with peripherals */
        I2C_Master_Start();

	}

    else if ((0==auxDetected) && (inAuxMode))
    {
        
		iAP2_Start(0);
        I2S_Out_Select_Write(I2S_OUT_TO_I2S_OUT);
        inAuxMode = 0;
    }
}

#endif

/* [] END OF FILE */
