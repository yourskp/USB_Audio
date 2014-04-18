/*******************************************************************************
* File Name: VolumeControl.c
*
* Version 4.0
*
*  Description: This file contains all the Volume control code
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
#include <USBInterface.h>

#ifdef ENABLE_VOLUME_CONTROL

#include <Codec.h>
#include <LCD.h>
#include <project.h>
#include <VolumeControl.h>

#ifdef GRAPHICAL_VOLUME_UPDATE_DISPLAY
static void UpdateLCDVolumeDisplay(void);
#endif

uint8 currentVolume = 0;                  /* Value of set volume level */
uint8 currentMute = 0;                    /* Mute status */
uint8 currentLCDVolume = NON_ZERO_NUMBER; /* Initialize currentLCDVolume to a non zero value to enable volume display
                                           * on LCD as soon as firmware comes up */
uint8 currentLCDMute = 0;                 /* LCD update flag for mute status */                        
uint8 volumeUpdateSent = 0;               /* Volume update info sent over USB flag */
uint8 volumeUpdateReceived = 0;           /* USB host read the updated volume over USB status IN endpoint */
int16 sliderSwipeLevel = 0;               /* CapSense slider swipe magnitude */
uint8 sliderInitialValue = 0;             /* Initial finger position on the slider */
uint16 volumeChangeFlag = OUTPUT_VOLUME_CHANGED; /* Indicate a change in the Volume/Mute feature unit*/
uint8 sliderTimer = 0;                    /* How long the swipe event must be accumulated before determining swipe magnitude */
#ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
uint8 currentInputVolume;                 /* Audio IN path current volume level */
uint8 currentInputMute;                   /* Audio IN path mute status */
uint8 inEventPending = 0; 				  /* Flag to Check Audio IN volume after Audio OUT volume update is done */      
#endif

extern uint8 centroid;                    /* CapSense slider centroid position */
extern CYBIT codecInit;                   /* Check if codec is initialized before updating volume */
extern volatile uint8 USBFS_currentVolume[]; /* USB audio class volume setting buffer */
extern CYBIT auxInStatus;                 /* Aux audio status */
extern uint8 iAPVolumeLevel;              /* Volume level received over an iAP command from the Apple device */
extern CYBIT midiControlEnabled;          /* CapSense interface mode (MIDI pitch or volume control) */
extern volatile uint8 USBFS_currentMute; /* USB audio class mute buffer */
extern CYPDATA uint8 audioSource;         /* Accessory audio source value */
#ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
extern volatile uint8 USB_inputMute;      /* USB audio IN path mute setting */
extern volatile uint8 USB_inputVolume[];  /* USB component audio IN path volume level */
#endif

extern volatile uint8 USBFS_minimumVolume[];
extern volatile uint8 USBFS_maximumVolume[];
extern volatile uint8 USBFS_resolutionVolume[];

/*******************************************************************************
* Function Name: HandleUSBVolumeUpdate
********************************************************************************
* Summary:  
*		Extract the volume change USB request and update the codec with the
*       received volume information.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void HandleUSBVolumeUpdate(void)
{
    int16 usbVolume;
    int8 codecVolume;
    
    /* USB volume information high byte increments for every 1dB change in volume. The Cirrus codec has a step size of
     * 0.5dB. To match the USB step size to Cirrus codec step size, multiply the volume gain in dB by 2 (1 USB unit = 2 
     * Cirrus codec unit).
     * Note : This mapping is specific to Cirrus codec on CY8CKIT-033A. You should add appropriate code to map the 
     * USB volume level to the codec used in your design */
    usbVolume =  ((int16)USBFS_currentVolume[1] << 8) | USBFS_currentVolume[0];
    usbVolume = usbVolume * VOLUME_STEP_SIZE_MAPPING;
    codecVolume = HI8(usbVolume);
    
    /* Update LCD display to indicate the currently set volume level. The actual volume control register will be
     * updated only when the CODEC is initialized */
    if(currentLCDVolume != codecVolume)
    {
        #ifdef LCD_MODULE_ENABLED
            #ifdef GRAPHICAL_VOLUME_UPDATE_DISPLAY
                UpdateLCDVolumeDisplay();
            #else    
                LCD_Position(1,9);
                LCD_PrintString("   ");
                LCD_Position(1,5);
                LCD_PrintString("Vol:");
                LCD_PrintNumber(codecVolume);
            #endif
        #endif
        
        currentLCDVolume = codecVolume;
    }
    
    if(currentLCDMute != USBFS_currentMute)
    {
        if(USBFS_currentMute == 1)
        {
            #ifdef LCD_MODULE_ENABLED
            LCD_Position(1,5);
            LCD_PrintString("Mute  ");
            #endif
        }
        else
        {
            currentLCDVolume--; /* Dirty LCD volume update flag after coming out of Mute state */
        }

        currentLCDMute = USBFS_currentMute;
    }    
    
    /* If the volume level has changed and the codec is initialized, then update the codec volume control register with
     * the updated volume level */
	if (codecInit == TRUE)
	{
	    if((currentVolume != codecVolume || currentMute != USBFS_currentMute))
	    {
	        currentVolume = codecVolume;
	        currentMute = USBFS_currentMute;
	        UpdateCodecVolume(codecVolume);
		}
		#ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
		if((currentInputVolume != USB_inputVolume[1] || currentInputMute != USB_inputMute))
	    {
	        currentInputVolume = USB_inputVolume[1];
	        currentInputMute = USB_inputMute;
	        UpdateCodecAttenuation(USB_inputVolume[1]);
		}
        #endif
    }
    
	/* USER_CODE:[CODEC] If different codec is used than onboard Cirrus codec (CS42L51), then
	 * Comment out the functions InitCirrusCodec().
	 * Add your code for codec initialization in place of InitCirrusCodec(). 
	 * Update the UpdateCodecVolume() and UpdateCodecAttenuation() functions as required by new codec. 
	 * Update the codec.h file with the I2C address of codec and the register addresses of the new codec. */     
	
    /* If USB host has scanned for the updated volume information from the accessory, send a "No Volume Change"
     * status to the Host */
    if((USBFS_GetEPState(AUDIO_INT_STATUS_ENDPOINT) == USBFS_EVENT_PENDING) 
        && volumeUpdateSent && volumeUpdateReceived)
    {
        volumeChangeFlag = NO_VOLUME_CHANGE;
        USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, (uint8 *)&volumeChangeFlag, sizeof(volumeChangeFlag));
        USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, USBFS_NULL, sizeof(volumeChangeFlag));

        volumeUpdateSent = 0;
        volumeUpdateReceived = 0;
    }
    
    /* Use CapSense slider as volume control input only when MIDI control is disabled (SW101 on the board can be used
     * for toggling between MIDI control and volume control) */
    if(!midiControlEnabled)
    {
        HandleVolumeSlider();
    }
}

/*******************************************************************************
* Function Name: HandleVolumeSlider
********************************************************************************
* Summary:  
*		Handles Apple device volume control using CapSense slider
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void HandleVolumeSlider(void)
{
    #ifdef CAPSENSE_ENABLED
    centroid = CapSense_GetCentroidPos(CapSense_PSOC_SLIDER__LS);

    /* slider is swiped */
    if( (centroid != SLIDER_NOT_ACTIVE) && (!sliderInitialValue))
    {
        /* Get the centroid when the slider is touched and make it the initial value for the swipe */
        sliderInitialValue = centroid;
        
        /* For Analog audio mode where volume control notifications are enabled over iAP, increase the time window
         * for which the CapSense slider swipe is checked before sending a volume update. This increased time is required 
         * to make sure the volume change notification is received from the Apple device for the current volume
         * update before sending the next volume control update command */
        if(GetAppleDeviceAudioSource() == AUDIO_SOURCE_DIGITAL )
        {
        	sliderTimer = SLIDER_TIMER_COUNT;
		}
    }

    if(centroid != SLIDER_NOT_ACTIVE)  /* If the slider is touched */
    {
        /* update slider swipe level */
        sliderSwipeLevel = (int16)centroid - (int16)sliderInitialValue;
        
        /* hysteresis for slider swipe */
        if(sliderSwipeLevel > -MINIMUM_SWIPE_COUNT && sliderSwipeLevel < MINIMUM_SWIPE_COUNT && (sliderTimer == 0))
        {
            sliderSwipeLevel = 0;
            
            sliderTimer = SLIDER_TIMER_COUNT;

        }
        
        /* Within the preset time, if the slider swipe value is above the hysteresis value, then detect that as
         * a swipe event and update the volume */
        if(sliderSwipeLevel != 0 && sliderTimer == 0)
        {
            if( IS_AUX_NOT_SELECTED() )
            {
	            {
	            	/* Update the volume on the Apple device/Mac/PC using USB volume control feature unit */
                	ControlVolumeOverUSB();
	            }
			}
        }
    }
    else
    {
        sliderInitialValue = 0; /* Slider swipe failed, reinitialize the swipe algorithm */
    }
	#ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
	if (inEventPending == 1 && USBFS_GetEPState(AUDIO_INT_STATUS_ENDPOINT) == USBFS_EVENT_PENDING)		
	/*Send Audio IN volume update only after Audio OUT volume update has been polled*/
		{
			volumeChangeFlag = INPUT_VOLUME_CHANGED;
			USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, (uint8 *)&volumeChangeFlag, sizeof(volumeChangeFlag));
			#if(USBFS_EP_MM == USBFS__EP_DMAAUTO)
    			USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, USBFS_NULL, sizeof(volumeChangeFlag));
			#endif
			inEventPending = 0;
		}
	#endif		
    #endif
}


/*******************************************************************************
* Function Name: ControlVolumeOverUSB
********************************************************************************
* Summary: 
*		Updates Apple device/Mac/PC volume using USB audio class volume control
*       feature unit
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ControlVolumeOverUSB(void)
{
    int16 currentVolume;
    int32 tempVolume;
	#ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY	
	int16 currentInputVolume;		
    int32 tempInputVolume;			
	#endif
    #ifdef USB_AUDIO_CLASS_TWO_VOLUME_CONTROL
    InterruptDataMessage volumeInterrupt;
    #endif
    
    currentVolume = ((int16)USBFS_currentVolume[1] << 8) | USBFS_currentVolume[0];
    tempVolume = ((int32)currentVolume + (sliderSwipeLevel* LINEAR_SWIPE_MULTIPLIER_USB));
    
    USBFS_currentMute = 0;

    if(tempVolume > USB_MAXIMUM_VOLUME) 
    {
        USBFS_currentVolume[0] = USB_MAXIMUM_VOLUME_LOW;
        USBFS_currentVolume[1] = USB_MAXIMUM_VOLUME_HIGH;
    }
    else if(tempVolume < USB_MINIMUM_VOLUME)
    {
        USBFS_currentVolume[0] = USB_MINIMUM_VOLUME_LOW;
        USBFS_currentVolume[1] = USB_MINIMUM_VOLUME_HIGH;
        
        USBFS_currentMute = 1;
    }
    else
    {
        currentVolume = (int16) tempVolume;
        USBFS_currentVolume[0] = LO8(currentVolume);
        USBFS_currentVolume[1] = HI8(currentVolume);
    }
    
	#ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY		
		currentInputVolume = ((int16)USB_inputVolume[1] << 8) | USB_inputVolume[0];
	    tempInputVolume = ((int32)currentInputVolume + (sliderSwipeLevel* LINEAR_SWIPE_MULTIPLIER_USB));
	    
	    USB_inputMute = 0;

	    if(tempInputVolume > USB_MAXIMUM_VOLUME)
	    {
	        USB_inputVolume[0] = USB_MAXIMUM_VOLUME_LOW;
	        USB_inputVolume[1] = USB_MAXIMUM_VOLUME_HIGH;
	    }
	    else if(tempInputVolume < USB_MINIMUM_VOLUME)
	    {
	        USB_inputVolume[0] = USB_MINIMUM_VOLUME_LOW;
	        USB_inputVolume[1] = USB_MINIMUM_VOLUME_HIGH;
	        
	        USB_inputMute = 1;
	    }
	    else
	    {
	        currentInputVolume = (int16) tempInputVolume;
	        USB_inputVolume[0] = LO8(currentInputVolume);
	        USB_inputVolume[1] = HI8(currentInputVolume);
	    }
	#endif

	if(IsUSBConfigured())
    {
        #ifdef USB_AUDIO_CLASS_TWO_VOLUME_CONTROL
            /* Piece of code below is for USB Audio class 2.0 Volume update */
            volumeInterrupt.bInfo = AUDIO_CLASS_INTERFACE_INFO;
            volumeInterrupt.bAttribute = CUR_VOLUME_ATTRIBUTE;                
            volumeInterrupt.bChannelNumber = MASTER_CHANNEL;
            volumeInterrupt.bControlSelect = USBFS_VOLUME_CONTROL;
            volumeInterrupt.bInterface = AUDIO_CONTROL_INTERFACE;
            volumeInterrupt.bEntityID = VOLUME_CONTROL_FEATURE_UNIT;
            
            USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, (uint8 *)&volumeInterrupt[bInfo], 
                            sizeof(volumeChangeFlag));
            #if(USBFS_EP_MM == USBFS__EP_DMAAUTO)
                USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, USBFS_NULL, sizeof(volumeChangeFlag));
            #endif
        #else
			#ifdef ENABLE_DIGITAL_AUDIO_OUT_ONLY		
            	volumeChangeFlag = OUTPUT_VOLUME_CHANGED;
            	USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, (uint8 *)&volumeChangeFlag, sizeof(volumeChangeFlag));
            	#if(USBFS_EP_MM == USBFS__EP_DMAAUTO)
                	USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, USBFS_NULL, sizeof(volumeChangeFlag));
            	#endif
			#else
				#ifdef ENABLE_DIGITAL_AUDIO_IN_ONLY		
					volumeChangeFlag = INPUT_VOLUME_CHANGED;
            		USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, (uint8 *)&volumeChangeFlag, sizeof(volumeChangeFlag));
            		#if(USBFS_EP_MM == USBFS__EP_DMAAUTO)
                		USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, USBFS_NULL, sizeof(volumeChangeFlag));
            		#endif
				#else			/*If both Audio IN and OUT are enabled*/
				
					volumeChangeFlag = OUTPUT_VOLUME_CHANGED;
            		USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, (uint8 *)&volumeChangeFlag, sizeof(volumeChangeFlag));
            		#if(USBFS_EP_MM == USBFS__EP_DMAAUTO)
                		USBFS_LoadInEP(AUDIO_INT_STATUS_ENDPOINT, USBFS_NULL, sizeof(volumeChangeFlag));
            		#endif

					inEventPending	=	1;
										
				#endif
			#endif
        #endif
    }
    volumeUpdateSent = 1;
    sliderInitialValue = 0;
}

/*******************************************************************************
* Function Name: UpdateLCDVolumeDisplay
********************************************************************************
* Summary: 
*		Update the LCD display with current system volume as a graph
*
* Parameters:
*  None
*
* Return:
*  void
*
*******************************************************************************/
#ifdef GRAPHICAL_VOLUME_UPDATE_DISPLAY
static void UpdateLCDVolumeDisplay(void)
{
    uint8 index;
    
    /* Initialize the array with default blocks for display and change the end of the string according to the 
     * volume level */
    uint8 displayString[7] = {BLOCK_DISPLAY_VALUE, BLOCK_DISPLAY_VALUE, BLOCK_DISPLAY_VALUE, BLOCK_DISPLAY_VALUE,
                              BLOCK_DISPLAY_VALUE, BLOCK_DISPLAY_VALUE, 0};
    
    /* Negative number, adjust the values accordingly */
    if(USBFS_currentVolume[1] > MAXIMUM_POSITIVE_INT8_NUMBER)
    {
        index = (USBFS_currentVolume[1] - USB_MINIMUM_VOLUME_HIGH) / VOLUME_COUNT_PER_GRAPHICAL_BLOCK;
    }
    else
    {
        index = ZERO_DB_GRAPHICAL_BLOCK_INDEX + (USBFS_currentVolume[1] / VOLUME_COUNT_PER_GRAPHICAL_BLOCK);
    }
    
    /* Insert the string terminator depending on the actual volume */
    displayString[index+1] = 0;
    
    #ifdef LCD_MODULE_ENABLED
    /* Clear the volume display area on the LCD and then print the block representation of the volume */
    LCD_Position(1,5);
    LCD_PrintString("      ");
    LCD_Position(1,5);
    LCD_PrintString(&displayString[0]);
    #endif
}
#endif

/*******************************************************************************
* Function Name: InitUSBVolumeLevel
********************************************************************************
* Summary: 
*		Initializes USB audio class volume range (minimum, maximum and resolution)
*       values based on the hardware volume control range supported by the codec
*
* Parameters:
*  None
*
* Return:
*  void
*
*******************************************************************************/
void InitUSBVolumeLevel(void)
{
    USBFS_minimumVolume[0] = USB_MINIMUM_VOLUME_LOW;
    USBFS_minimumVolume[1] = USB_MINIMUM_VOLUME_HIGH;
    USBFS_maximumVolume[0] = USB_MAXIMUM_VOLUME_LOW;
    USBFS_maximumVolume[1] = USB_MAXIMUM_VOLUME_HIGH;
    USBFS_resolutionVolume[0] = USB_VOLUME_RESOLUTION_LOW;
    USBFS_resolutionVolume[1] = USB_VOLUME_RESOLUTION_HIGH;
    USBFS_currentVolume[1] = SYSTEM_STARTUP_VOLUME_HIGH;
}

#endif  /* End of ENABLE_VOLUME_CONTROL */

/* [] END OF FILE */
