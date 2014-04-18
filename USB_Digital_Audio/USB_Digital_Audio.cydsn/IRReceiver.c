/*******************************************************************************
* File Name: IRReceiver.c
*
* Version 4.0
*
*  Description: This file contains the  remote IR receiver decoder APIs.
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
#include <Configuration.h>
#include <device.h>
#include <Interrupts.h>
#include <IRReceiver.h>
#include <USBInterface.h>
#include <VolumeControl.h>

#ifdef IR_RECEIVER_ENABLED

extern CYPDATA uint8 audioSource;
#ifdef ENABLE_VOLUME_CONTROL
extern int16 sliderSwipeLevel;
#endif

uint8 pulseWidth;                       /* width of the IR pulse received on PSoC 3 GPIO */
uint8 irState = IR_RECEIVER_IDLE_STATE; /* current state of NEC IR decode state machine */
uint32 irReceiverData;                  /* data received over NEC IR protocol */
uint8 irStateCounter = 0;               /* number of data bits received during the data phase of NEC IR decoding */
CYBIT irCommandReceived = 0;            /* one complete IR command is received from the IR remote */
uint8 irKey = 0;                        /* key code for the received command */
uint8 irButtonMask = 0;                 /* which of the IR remote buttons are pressed */
uint8 irStickyKey = 0;                  /* Holds the state of the IR remote key pressed for sending messages on iAP */

/*******************************************************************************
* Function Name: ProcessIRReceiver
********************************************************************************
* Summary:
*    This function processes the received IR pulse based on the width of 
*    the pulse. The Apple IR remote uses the following protocol
*    IR Remote physical layer protocol : NEC protocol for remotes
*    Data format: 4 bytes data with LS bit and LS Byte first order
*    1st, 2nd and 4th bytes are fixed in the Apple remote protocol namely 0xEE,
*    0x87 and 0x11 respectively.
*    3rd bye contains the key code for the button pressed and the key codes
*    for each of the buttons are defines in IRReceiver.h file
*
* Implementation:
*   Enable IR Receiver pin falling edge interrupt
*   On every falling edge interrupt, read the width of the pulse whose falling edge
*   triggered the interrupt
*   Reset the IR pulse width counter by writing to the control register connected to 
*   fixed function timer
*   Compare the pulse width with pre-defined thresholds and determine the key code
*   received
*   The accuracy of the clock driving the IR pulse width counter and the thresholds 
*   set for decoding the pulse determines the accuracy of decoding. To minimize 
*   UDB/hardware usage on the Digital audio solution project, an 8 bit fixed function
*   counter is used for decoding pulse width. If more accurate measurement is required,
*   update the counter width from 8 to 16 bites and update the threshold values in 
*   IRReceiver.h file accordingly
* 
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ProcessIRReceiver(void)
{
    pulseWidth = IR_COUNTE_PERIOD_VALUE - IR_Counter_ReadCounter();
    
    IR_Control_Register_Write(1);
        
    PSOC_IR_RX_ClearInterrupt();
    
    IR_Counter_Start();
        
    switch(irState)
    {
        case IR_RECEIVER_IDLE_STATE:
            irState++;
        break;
        
        case IR_RECEIVER_START_STATE:
            if(pulseWidth >= IR_MIN_START_PULSE_WIDTH && pulseWidth <= IR_MAX_START_PULSE_WIDTH)
            {
                /* Start pulse burst detected */
                irState++;
                irReceiverData = 0;
                irStateCounter = 0;
            }
            else if (pulseWidth >= IR_MIN_REPEAT_PULSE_WIDTH && pulseWidth <= IR_MAX_REPEAT_PULSE_WIDTH)
            {
                irState = IR_RECEIVER_IDLE_STATE;
                irCommandReceived = TRUE;
            }
            else
            {
                irState = IR_RECEIVER_IDLE_STATE;
            }
        break;
        
        case IR_APPLE_REMOTE_DATA_STATE:
            irStateCounter++;
            
            if(pulseWidth >= IR_MIN_LOGIC0_PULSE_WIDTH && pulseWidth <= IR_MAX_LOGIC0_PULSE_WIDTH)
            {
                irReceiverData = irReceiverData;
            }
            else if(pulseWidth >= IR_MIN_LOGIC1_PULSE_WIDTH && pulseWidth <= IR_MAX_LOGIC1_PULSE_WIDTH)
            {
                irReceiverData = (irReceiverData | 0x01);
            }
            else
            {
                irState = IR_RECEIVER_IDLE_STATE;
            }
            
            if(irStateCounter == NUMBER_OF_DATA_BITS)
            {
                irStateCounter = 0;
                irState = IR_RECEIVER_IDLE_STATE;
                irCommandReceived = TRUE;
                irKey = HI8(LO16(irReceiverData));
                break;
            }
            irReceiverData = irReceiverData << 1;
            
        break;
    }
}

/*******************************************************************************
* Function Name: SetupIRReceiver
********************************************************************************
* Summary:
*        This function configures PSoC for decoding  remote key press 
*        events over IR
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void SetupIRReceiver(void)
{
    CyPins_ClearPin(PSOC_PERIPH_PWR_0); /* Turn on power to IR receiver block */
    
    PSOC_IR_RX_ClearInterrupt();     /* IR receiver pin interrupt for decoding pulse width */
    
    isr_IR_StartEx(IR_Interrupt);
    
    isr_Counter_StartEx(IR_Counter_Interrupt); /* Counter interrupt to clear the IR state machine on overflow */
}

/*******************************************************************************
* Function Name: StopIRReceiver
********************************************************************************
* Summary:
*        This function stops the IR receiver from receiving any further IR remote
*        key press events. This API is called when the  device is 
*        disconnected from the accessory (inside RestoreDefaultAccessorySettings
*        routine)
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void StopIRReceiver(void)
{
    isr_IR_Stop();
    
    PSOC_IR_RX_ClearInterrupt();/* IR receiver pin interrupt for decoding pulse width */
    
    isr_Counter_Stop();         /* Counter interrupt to clear the IR state machine on overflow */
    
    isr_Counter_ClearPending(); /* clear any pending IR counter interrupt */
    
    IR_Counter_Stop();          /* Stop the IR pulse width calculator timer */
}

/*******************************************************************************
* Function Name: ReportIRRemoteEvents
********************************************************************************
* Summary:
*        This function decodes IR remote events from the device and 
*        sends necessary iAP command to  device depending on the IR remote
*        key pressed.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ReportIRRemoteEvents(void)
{
    if(irCommandReceived)
    {
        /* Decode IR remote events only when the  device is connected to the accessory */
        switch(irKey)
        {
            case PLAY_KEY:
                irButtonMask = irButtonMask | PLAY_PAUSE_MASK;
                irStickyKey = 0;
            break;
            
            case RIGHT_KEY:
                irButtonMask = irButtonMask | NEXT_TRACK_MASK;
                irStickyKey = 0;
            break;
            
            case LEFT_KEY:
                irButtonMask = irButtonMask | PREVIOUS_TRACK_MASK;
                irStickyKey = 0;
            break;
            
            #ifdef ENABLE_VOLUME_CONTROL
            
            case UP_KEY:
                sliderSwipeLevel = IR_VOLUME_UPDATE_COUNT;
				#if ENABLE_IPOD_NANO_DIGITAL_AUDIO
                if (IsNanoDigitalConnected() == TRUE)
				{
					ControliPodVolumeOveriAP();
                }
                else
				#endif 
                {
                	ControlVolumeOverUSB(); 
				}
            break;
            
            case DOWN_KEY:
                sliderSwipeLevel = -IR_VOLUME_UPDATE_COUNT;
				#if ENABLE_IPOD_NANO_DIGITAL_AUDIO
				if (IsNanoDigitalConnected() == TRUE)
				{
					ControliPodVolumeOveriAP();                 
            	}
            	else
				#endif
            	{
                	ControlVolumeOverUSB();    
				}
            break;
            
            #endif
            
            default:
            /* Add your code for receiving remote events from other remote */
            break;
            
            /* USER_CODE: [IR commands] Placeholder for decoding other commands received from the NEC IR remote */
        }
      
        irCommandReceived = 0;
    }
}
#endif

/* [] END OF FILE */
