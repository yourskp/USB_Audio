/*******************************************************************************
* File Name: USBInterface.c
*
* Version 4.0
*
* Description: This file contains routines for handling Apple device or PC/Mac
*              USB interface requests.
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
#include <IRReceiver.h>
#include <LCD.h>
#include <USBInterface.h>
#include <USBUART.h>

extern volatile uint8 USBFS_interfaceSetting[];
extern CYBIT lowPowerIdle;
extern CYBIT auxInStatus;
extern uint8 MIDIOutBuffer[];
extern uint8 USBFS_initVar;
extern volatile uint8 USBFS_midiOutBuffer[];
extern uint16 playlistControlReport;
extern CYBIT outPlaying;
extern CYBIT inPlaying;
extern CYDATA uint8 auxConfigured;
extern CYPDATA uint8 audioSource;

#ifdef MIDI_ENABLED
extern volatile uint8 USBFS_midi_in_ep;                               /* Input endpoint number */
extern volatile uint8 USBFS_midi_out_ep;                              /* Output endpoint number */
extern volatile uint8 USBFS_midiInBuffer[USBFS_MIDI_IN_BUFF_SIZE];   /* Input endpoint buffer */
extern volatile uint8 USBFS_midiOutBuffer[USBFS_MIDI_OUT_BUFF_SIZE]; /* Output endpoint buffer */
extern CYBIT usbReset;
extern volatile uint8 USBFS_midiInPointer;
extern uint8 midiInWaitTimer;
void USBFS_MIDI_IN_EP_Service(void);
#endif

#ifdef ENABLE_VOLUME_CONTROL
extern uint8 currentLCDVolume;
extern uint8 currentLCDMute;
#endif

#ifdef WINDOWS7_WORKAROUND
extern volatile uint8 USBFS_configurationChanged;
void USBFS_Config(uint8 clearAltSetting) CYREENTRANT;
#endif
extern CYBIT codecInit;  
extern CYDATA uint8 midiPowerIdle;

uint8 usbActivityCounter = 0;
CYBIT usbMiniBActive = FALSE;
uint8 USBDeviceState = USB_INTERFACE_INACTIVE;
CYBIT midiEPInitialized = FALSE;

#if(USBFS_EP_MM == USBFS__EP_DMAAUTO) 
    #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
    uint8 outRam[OUT_AUDIOMAXPKT];
    #endif
  
    #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
    uint8 inRam[IN_AUDIOMAXPKT];
    uint16 inCnt;
    #endif
#endif

uint8 altSetting[NO_OF_AUDIO_STREAM_INTERFACE] = {0xFF, 0xFF};

#ifdef PHONE_CONTROL_ENABLED
uint8 hidOutReport;
#endif    

/*******************************************************************************
* Function Name: ServiceUSB
********************************************************************************
* Summary: This routine performs tasks that should be done soon after USB 
*          enumeration is completed (configure DMA, initialize state machine etc).
*          When the USB configuration is changed, this routine reinitializes all
*          the USB endpoints as required by the application.       
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ServiceUSB(void)
{
    CYBIT macPC_flag=FALSE;
	if(USB_INTERFACE_INACTIVE == USBDeviceState)
    {
        USBDeviceState = USB_INIT_AFTER_ENUMERATION_REQUIRED;
    }
	
	/* Initialization sequence for every USB host enumeration event */
    if(USBDeviceState == USB_INIT_AFTER_ENUMERATION_REQUIRED)
    {
        uint16 index = 0;
        
        USBDeviceState = USB_INIT_AFTER_ENUMERATION_COMPLETED;
        SetAppleDeviceAudioSource(AUDIO_SOURCE_DIGITAL);
        macPC_flag = IsMacPCConnected();
        
        #if(USBFS_EP_MM == USBFS__EP_DMAAUTO)
            /* USER_CODE: [Audio Buffers] Add a separate for loop if the playback and recording audio buffer size are 
             * not equal */
            for(index=0; index< OUT_AUDIOMAXPKT; index++)
            {
                #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
                inRam[index] = 0;
                #endif
                #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
                outRam[index] = 0;
                #endif
            }
            #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
            inCnt = IN_AUDIOMAXPKT;
            #endif
        #endif
        
        #ifdef CDC_ENABLED
        USBUARTStart(); /* Initializes the USB UART interface */
        #endif
                
        /* Configure the HID input endpoint buffer for Mac/PC playlist control */
        if(macPC_flag)
        {
            USBFS_LoadInEP(MAC_PC_HID_CONTROL_ENDPOINT, (uint8 *)&playlistControlReport, sizeof(playlistControlReport));
            
            #ifdef PHONE_CONTROL_ENABLED
            USBFS_ReadOutEP(MAC_PC_HID_OUT_ENDPOINT, &hidOutReport, sizeof(hidOutReport));
            USBFS_EnableOutEP(MAC_PC_HID_OUT_ENDPOINT);
            #endif
        }
        
        /* If Aux is not currently configured, then switch to digital audio mode */
        if(IS_AUX_NOT_SELECTED())
        {
            ConfigureDigitalAudioDMA();
        }
        else
        {
            #ifdef LCD_MODULE_ENABLED
            /* Else Display Aux Configured message on the LCD */
            LCD2LineDisplay("Analog Loopback ",
                            "                ");
            #endif                
        }
        
        /* USER_CODE: [USB enumeration] placeholder for initializing custom user code after the USB host enumerates the
         * accessory. This routine will be called once per accessory connection after the host issues SET_CONFIGURATION
         * request */
    }
    
    #ifdef MIDI_ENABLED
    if (midiEPInitialized == FALSE || usbReset)
    {
        /* Initialize MIDI only when a valid USB host is connected */
        if ((IsUSBConfigured() && IsMacPCConnected()))
        {
            USBFS_MIDI_EP_Init();
            
            /* USB Component internally sets the priority of the UART TX and RX ISRs to 4 and 2 respectively, change the
             * interrupt priority in the application code to match the system interrupt setup */
             
            CyIntSetPriority(MIDI1_UART_TX_VECT_NUM, MIDI_UART_INTERRUPT_PRIORITY_SIX);
            CyIntSetPriority(MIDI1_UART_RX_VECT_NUM, MIDI_UART_INTERRUPT_PRIORITY_FIVE);
            #if (USBFS_MIDI_EXT_MODE >= USBFS_TWO_EXT_INTRF)
            CyIntSetPriority(MIDI2_UART_TX_VECT_NUM, MIDI_UART_INTERRUPT_PRIORITY_SIX);
            CyIntSetPriority(MIDI2_UART_RX_VECT_NUM, MIDI_UART_INTERRUPT_PRIORITY_FIVE);
            #endif
            midiEPInitialized = TRUE;
            usbReset = 0;
        }
    }
    
    if(USBFS_midiInPointer%USBFS_EVENT_LENGTH == 0 && USBFS_midiInPointer!=0)
    {
        if(midiInWaitTimer == 0)
        {
            midiInWaitTimer = MIDI_IN_EP_WAIT_TIME;
            USBFS_MIDI_IN_EP_Service();
        }
    }
    else
    {
        midiInWaitTimer = MIDI_IN_EP_WAIT_TIME;
    }
    #endif
        
    /* USBFS_IsConfigurationChanged() is a clear on read status update therefore, only one read of 
     * USBFS_IsConfigurationChanged() should ever exist in user code */
    if(USBFS_IsConfigurationChanged())
    {
        macPC_flag = IsMacPCConnected();
		#ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY    
        
        /* Get Alternate setting */
        altSetting[AUDIO_OUT_INTERFACE_INDEX] = (macPC_flag? USBFS_GetInterfaceSetting(1):USBFS_GetInterfaceSetting(2)); 
        
        /* ByteSwap control register bit is set to 1 if alt setting 2 is selected so that  
         * Byte swap digital logic processes data as 16 bits. ByteSwap control register is set to 0 
         * if alt setting 1 is selected and byte swap processes data as 24 bits */
        if (altSetting[AUDIO_OUT_INTERFACE_INDEX]==ALT_SETTING_ACTIVE_24_BIT)
        {
            ByteSwap_Tx_CONTROL_REG = ByteSwap_Tx_CONTROL_REG & (~ByteSwap_Tx_RES_CTRL_16);  
        }
        else if (altSetting[AUDIO_OUT_INTERFACE_INDEX]==ALT_SETTING_ACTIVE_16_BIT)
        {
            ByteSwap_Tx_CONTROL_REG = ByteSwap_Tx_CONTROL_REG | ByteSwap_Tx_RES_CTRL_16;            
        }
        
        /* Arming the audio out EP if it is not zero bandwidth alt setting */
        if (altSetting[AUDIO_OUT_INTERFACE_INDEX]!= ALT_SETTING_ZERO_BANDWIDTH && 
            (CY_GET_REG8(USBFS_SIE_EP1_CR0_PTR + ((AUDIO_OUT_ENDPOINT - USBFS_EP1) << USBFS_EPX_CNTX_ADDR_SHIFT)) & USBFS_MODE_MASK) 
                                                                                    == USBFS_MODE_NAK_IN_OUT)
        {
            /* Init DMA configurations for audio OUT EP */
            USBFS_ReadOutEP(AUDIO_OUT_ENDPOINT, &outRam[0], OUT_AUDIOMAXPKT);
            USBFS_EnableOutEP(AUDIO_OUT_ENDPOINT);
        }
        #endif
        
        #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
            #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY 
            if(altSetting[AUDIO_IN_INTERFACE_INDEX] != (macPC_flag? USBFS_GetInterfaceSetting(2):USBFS_GetInterfaceSetting(3)))
        {
                  altSetting[AUDIO_IN_INTERFACE_INDEX] = (macPC_flag? USBFS_GetInterfaceSetting(2):USBFS_GetInterfaceSetting(3));
            #else
            if(altSetting[AUDIO_IN_INTERFACE_INDEX] != (macPC_flag? USBFS_GetInterfaceSetting(1):USBFS_GetInterfaceSetting(2)))
        {
                  altSetting[AUDIO_IN_INTERFACE_INDEX] = (macPC_flag? USBFS_GetInterfaceSetting(1):USBFS_GetInterfaceSetting(2));
            #endif
            
            /* Setting the ByteSwap control register bit to 0 regardless of alt setting is selected. Because audio in 
            *  interface both the alternate settings alt setting1 and alt setting 2 both use 3 byte subframe size. */            
            ByteSwap_Rx_CONTROL_REG = ByteSwap_Rx_CONTROL_REG & (~ByteSwap_Rx_RES_CTRL_16); 
            
            /* Arming the audio in EP if it is not zero bandwidth alt setting */
            if (altSetting[AUDIO_IN_INTERFACE_INDEX]!= ALT_SETTING_ZERO_BANDWIDTH &&
                (CY_GET_REG8(USBFS_SIE_EP1_CR0_PTR + ((AUDIO_IN_ENDPOINT - USBFS_EP1) << USBFS_EPX_CNTX_ADDR_SHIFT)) & USBFS_MODE_MASK) 
                                                                                        == USBFS_MODE_NAK_IN_OUT)
            {
                /* Init DMA configurations for audio IN EP */  
                inCnt = IN_AUDIOMAXPKT;
                USBFS_LoadInEP(AUDIO_IN_ENDPOINT, &inRam[0], inCnt);
                /* Pre-arm first audio IN request */
                USBFS_LoadInEP(AUDIO_IN_ENDPOINT, USBFS_NULL, inCnt);
            }
        }
        #endif
        
        /* USER_CODE: [USB configuration changed] Placeholder for adding additional USB endpoint initialization code 
         * when the host issues either a SET_INTERFACE or SET_CONFIGURATION request to the accessory. After receiving
         * a SET_INTERFACE request from the host, the endpoint belonging to the alternate setting being configured
         * by the USB host is reset and must be reinitialized here for proper operation of the USB block */
    }
}

/*******************************************************************************
* Function Name: ServiceUSBSuspend
********************************************************************************
* Summary:
*       This function handles USB suspend event from USB host and forces PSoC 3 
*       to enter low power mode. Once the USB resume event is detected, PSoC3 
*       wakes up and starts operating in normal mode.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
#ifdef HANDLE_USB_SUSPEND
void ServiceUSBSuspend(void)
{
    if(!IsMacPCConnected() || ! USBFS_initVar)
    {
      return;
    }
  
    /* Check if the host is active */
    if(USBFS_bCheckActivity() != 0 ) 
    {
      usbActivityCounter = 0;
    } 
    else 
    {
      usbActivityCounter++;
    }
        
    /* USB Suspend event is lack of greater than 3 consecutive SOF's */
    if(usbActivityCounter > USB_SUSPEND_TIME_TICKS )
    {
        /* The debounce delay is taken care by increasing the suspend time to 40ms (5 * 8ms) */                   
        if(IsMacPCConnected() && IsUSBConfigured()) 
        {  
            /* USER_CODE: [USB suspend] Placeholder for configuring ALL the additional components added by the user in 
             * sleep mode before calling USB suspend/PSoC 3 sleep API */
             
            #ifdef LCD_MODULE_ENABLED
              LCD_Position(0,0);
              LCD_PrintString("  USB Suspend   ");
            #endif
            
            /* If the accessory is not in low power mode, enter low power mode on seeing a USB suspend */
            if(!lowPowerIdle) 
            {
                lowPowerIdle = TRUE;
                StopAudioComponents();             /* Changes to 24 MHz IMO for USB */                        
                StopAnalogAudioComponents();       /* Turn OFF Analog path for Audio-In/iPod Analog */
            }
            
			if(!midiPowerIdle)
            {
                if(!lowPowerIdle)
                {
                    StopAudioComponents();        /* Changes to 24 MHz IMO for USB */
                }
                CyPins_SetPin(PSOC_MIDI_PWR_0);   /* Turn off the MIDI I/O hardware */
                midiPowerIdle = TRUE;             /* MIDI low power mode enabled */
            }
			
            CyPins_ClearPin(PSOC_CODEC_PWR_0);     /* Turn off the regulator to reduce suspend mode current */     
   
            USBFS_Suspend();
			
			I2C_Master_Sleep();                          /* Configure I2C master block in sleep mode */ 
           			
            #ifdef CAPSENSE_ENABLED
            while(CapSense_IsBusy());              /* Wait for current scan to complete */
            CapSense_Sleep();
            #endif
            
            #ifdef MIDI_ENABLED
            MIDI1_UART_Sleep();
			#if (USBFS_MIDI_EXT_MODE >= USBFS_TWO_EXT_INTRF)
            MIDI2_UART_Sleep();
			#endif
            #endif
            
            CyPmSaveClocks();            
            CyPmSleep(PM_SLEEP_TIME_NONE, PM_SLEEP_SRC_PICU); /* PSoC 3 is in sleep mode */
            CyPmRestoreClocks();
            USBFS_Resume();
			I2C_Master_Wakeup();
			
            #ifdef CAPSENSE_ENABLED
            CapSense_Wakeup();
            CapSense_IntClock_SetSource(CYCLK_SRC_SEL_IMO);
            #endif
            
            #ifdef MIDI_ENABLED
            MIDI1_UART_Wakeup();
			#if (USBFS_MIDI_EXT_MODE >= USBFS_TWO_EXT_INTRF)
            MIDI2_UART_Wakeup();                              
			#endif
            #endif
            
            CyPins_SetPin(PSOC_CODEC_PWR_0);        /* Turn ON the CODEC regulator after wake up */            
           
            #ifdef WINDOWS7_WORKAROUND
            if(USBFS_GetConfiguration() != 0)
            {
                USBFS_configurationChanged = USBFS_TRUE;
                USBFS_Config(USBFS_FALSE);
            }
            #endif
     
            #ifdef LCD_MODULE_ENABLED
                LCD_Position(0,0);
                LCD_PrintString("Mac/PC Interface");
            #endif
            
            /* USER_CODE: [USB wakeup] Placeholder for re-configuring ALL the additional components added by the user in 
             * wakeup mode after calling USB wakeup */
        }
        usbActivityCounter = 0;
		/* After coming out of USB suspend, MIDI end point should be re-initialized */
		midiEPInitialized = 0;
    }
}
#endif

/*******************************************************************************
* Function Name: HandlePCMacUSBInterface
********************************************************************************
* Summary: Checks if PC/Mac is connected/disconnected and start the USB component
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void HandlePCMacUSBInterface(void)
{
    /* If Aux mode is enabled, then Mac/PC connection disconnection is handled only when the system is not in 
     * aux In mode. For self powered case, Apple device connection is also checked before starting the Mac/PC 
	 * interface */
    if(IS_AUX_NOT_SELECTED() && !USBFS_initVar && IsMacPCConnected())
    {  
        /* Switch the PSoC USB D+ and D- lines to USB Mini B */
        CyPins_ClearPin(PSOC_USB_SEL_0);
        
        /* Start the USB component when PC/Mac is connected */
        USBFS_Start(PC_MAC_AUDIO_WITH_VOLUME_DEVICE, USBFS_DWR_VDDD_OPERATION);
        USBDeviceState = USB_INIT_AFTER_ENUMERATION_REQUIRED;
        
        #ifdef LCD_MODULE_ENABLED
        if(IS_AUX_NOT_SELECTED())
        {
            LCD2LineDisplay("Mac/PC Interface",
                            "                ");
        }                
        #endif
        
        #ifdef ENABLE_VOLUME_CONTROL
        currentLCDVolume--; /* dirty the LCD volume and mute update flag to update volume and mute info on the LCD */
        currentLCDMute--;
        #endif

        usbMiniBActive = TRUE;
        
        /* USER_CODE: [Mac/PC connection] Placeholder for initializing components or external peripherals when the
         * accessory is plugged into Mac/PC (USB VBus = High) */
    }
	
	/* Check for PC/Mac USB Audio device disconnection in self powered mode.
	 *  In device powered mode project, no need of checking disconnection event as power is shut off 
	 *  as soon as USB cable is disconnected from USB mini connector. */
	else if(usbMiniBActive && (USBFS_bGetConfiguration() || USBDeviceState == USB_INIT_AFTER_ENUMERATION_REQUIRED))
    {
        /* If VBUS voltage from mini B is now gone and was previous present then stop USB interface */
        if(!IsMacPCConnected())
        {
            if(USBFS_initVar)  
        	{
				USBFS_Stop();
			}	
                    
            CyPins_SetPin(PSOC_USB_SEL_0); /* Switch the PSoC USB D+ and D- lines back to Apple device */        
            
            /* If Aux was not configured when PC is unplugged, then switch off CODEC */
            if(!auxConfigured)
            {
                CyPins_ClearPin(PSOC_CODEC_RST_0); /* Hold CODEC in reset */
				codecInit = FALSE;
            }
            usbMiniBActive = FALSE;
            
            /* USER_CODE: [Mac/PC disconnection] Placeholder for shutting down components or external peripherals when 
             * the accessory is disconnected from Mac/PC (USB VBus transitioned from High to Low) */
        }
    }
}

/*******************************************************************************
* Function Name: EnableNAKBulkIN
********************************************************************************
* Summary: Enables the NAK interrupt on a USB endpoint 
*
* Parameters:
*  Endpoint number for which NAK interrupt is to be enabled
*
* Return:
*  void
*
*******************************************************************************/
void EnableNAKBulkIN(uint8 bEPNumber)
{
    uint8 index = (bEPNumber - USBFS_EP1) << USBFS_EPX_CNTX_ADDR_SHIFT;
    
    if((CY_GET_REG8(&USBFS_SIE_EP1_CR0_PTR[index]) & USBFS_MODE_MASK) != USBFS_MODE_ACK_IN)
	{
    	CY_SET_REG8(&USBFS_SIE_EP1_CR0_PTR[index], 
                                    CY_GET_REG8(&USBFS_SIE_EP1_CR0_PTR[index]) | NAK_IN_INTERRUPT_ENABLE_MASK);
	}								
}

/*******************************************************************************
* Function Name: DisableNAKBulkIN
********************************************************************************
* Summary: Disables the NAK interrupt on a USB endpoint 
*
* Parameters:
*  Endpoint number for which NAK interrupt is to be disabled
*
* Return:
*  void
*
*******************************************************************************/
void DisableNAKBulkIN(uint8 bEPNumber)
{
    uint8 index = (bEPNumber - USBFS_EP1) << USBFS_EPX_CNTX_ADDR_SHIFT;
    
    CY_SET_REG8(&USBFS_SIE_EP1_CR0_PTR[index], 
                                    CY_GET_REG8(&USBFS_SIE_EP1_CR0_PTR[index]) & (~NAK_IN_INTERRUPT_ENABLE_MASK));
}

/* [] END OF FILE */
