/*******************************************************************************
* File Name: Timer.c
*
* Version 4.0
*
* Description: This file contains the system timer tick event updates
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
#include <Configuration.h>
#include <device.h>
#include <LCD.h>
#include <VolumeControl.h>
#include <USBInterface.h>

extern uint8 previousTrackTimer;    
extern uint8 nextTrackTimer;
extern uint8 playPauseTimer;
extern uint8 switchDebounceTimer;
extern uint8 lcdSwitchTimer;

extern uint8 startIDPSDelay;
extern CYPDATA uint8 clockSwitchTimer;

extern volatile uint8 midiEpPolled;
extern volatile uint8 usbMidiActive;


#ifdef TRACK_INFO_DISPLAY_ENABLED
extern uint8 delayCount;
#endif 

extern uint8 sliderTimer;
extern CYBIT updateLCD;
extern uint8 USBFS_device;
#ifdef LCD_MODULE_ENABLED
extern uint8 lcdRefreshInterval;
extern uint8 userDisplayTimer;
#endif
#ifdef AUX_DETECTION_ENABLE
extern uint16 auxTriggerafterSampleRateChangeTimer;
#endif

CYBIT applicationTimerTick = FALSE;
uint8 midiPollFrequency = 0;               /* MIDI polling frequency counter */
volatile uint8 midiPollFailCount = 0;      /* The number of consecutive MIDI endpoint polling fail count */
#ifdef MIDI_ENABLED
uint8 midiInWaitTimer;                     /* MIDI IN endpoint arming wait timer */
#endif

/*******************************************************************************
* Function Name: HandleApplicationTimerTick
********************************************************************************
* Summary:
*       This function updates all the Application layer timers based on sleep
*       timer tick event.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void HandleApplicationTimerTick(void)
{
    if(applicationTimerTick)
    {
        #ifdef AUX_DETECTION_ENABLE
		if(auxTriggerafterSampleRateChangeTimer)
		{
			auxTriggerafterSampleRateChangeTimer--;
		}
		#endif
        
		#ifdef HANDLE_USB_SUSPEND
        ServiceUSBSuspend();          /*Check for USB Suspend event*/
        #endif
        
        if(previousTrackTimer)
        {
            previousTrackTimer--;
        }
        
        if(nextTrackTimer)
        {
            nextTrackTimer--;
        }
        
        if(playPauseTimer)
        {
            playPauseTimer--;
        }
        
        if(switchDebounceTimer)
        {
            switchDebounceTimer--;
        }
        
        #ifdef LCD_MODULE_ENABLED
        if(lcdSwitchTimer)
        {
            lcdSwitchTimer--;
            
            if(lcdSwitchTimer == 0)
            {
                updateLCD = TRUE;
            }
        }
        #endif
        
        #ifdef LCD_MODULE_ENABLED
        if(userDisplayTimer)
        {
            userDisplayTimer--;
            
            if(userDisplayTimer == 0)
            {
                LCD_Position(0,0);
                if(IsMacPCConnected())
                {
                    LCD_PrintString("Mac/PC Interface");
                }
            }
        }
        #endif
        
      	#ifdef TRACK_INFO_DISPLAY_ENABLED
        if(delayCount)
        {
            delayCount--;
        }
        #endif 
        
        if(clockSwitchTimer)
        {
            clockSwitchTimer--;
        }
        
        #ifdef MIDI_ENABLED
        if(midiEpPolled == 0)
        {
            /* MIDI In is not polled, increment the MIDI polled counter */
            midiPollFailCount++;
            
            /* If MIDI IN endpoint is not polled consecutively for MIDI_POLLING_FAIL_MAX_COUNT, then MIDI interface
             * is inactive and the accessory must be in low power mode */
            if(midiPollFailCount == MIDI_POLLING_FAIL_MAX_COUNT)
            {
                usbMidiActive = FALSE;
                midiPollFailCount = 0;
            }
        }
        else
        {
            /* Once the MIDI endpoint is polled, reset all the polling and timing flags and set MIDI active flag for 
             * the application */
            usbMidiActive = TRUE;
            
            midiEpPolled = 0;
            
            midiPollFailCount = 0;
        }
        
        /* Enable USB MIDI In endpoint NAK transaction interrupt periodically to check if MIDI interface is active */
        if(IsUSBConfigured() && ( IsMacPCConnected()) &&  USBFS_device != 1)
        {
            midiPollFrequency++;
            
            if(midiPollFrequency == MIDI_POLL_FREQUENCY_COUNT)
            {
                midiPollFrequency = 0;
                EnableNAKBulkIN(MIDI_IN_ENDPOINT);
            }
        }
        
        if(midiInWaitTimer)
        {
            midiInWaitTimer--;
        }
        #endif
        
		#ifdef ENABLE_VOLUME_CONTROL
        if(sliderTimer)
        {
            sliderTimer--;
        }
		#endif
        
        #ifdef LCD_MODULE_ENABLED
        lcdRefreshInterval--;
        if(lcdRefreshInterval == 0)
        {
   
            /* Do not enable LCD refresh in  device powered mode to reduce power consumption */
            LCD_Refresh();
            lcdRefreshInterval = LCD_REFRESH_INTERVAL_COUNT;
        }
        #endif
        
        /* USER_CODE: [System tick timer] Placeholder for adding additional timers for custom user timing events. This
         * routine is executed approximately at every 8ms */
        
        applicationTimerTick = FALSE;
    }
}

/* [] END OF FILE */
