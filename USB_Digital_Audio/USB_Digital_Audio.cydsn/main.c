/*******************************************************************************
* File Name: main.c
*
* Version 4.0
*
* Description: This file contains the entry point for the application and 
*              handles the USB audio configuration, audio event handling and 
*              communication with USB hosts (Mac/PC).
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
#include <AuxDetection.h>
#include <Configuration.h>
#include <IRReceiver.h>
#include <LCD.h>
#include <USBInterface.h>
#ifdef CDC_ENABLED
#include <USBUART.h>
#endif    

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*        Main routine does following tasks.
*        - Initializes all the components for digital/analog audio 
*        - Initializes all the components for user interface and other system functions
*        - Handles hot plug of USB host - Apple device/Mac/PC
*		 - Handles USB host disconnection events
*        - Handles dynamic sample rate change, audio stream start/stop events
*        
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void main(void)
{
    
     /* Setup the Audio pipe from USB to I2S. After the call to this API, the system clock architecture changes 
      * from start up mode to idle mode. System clocks such as PLL, external crystal are shutdown. Any of the application 
	  * layer components which are using XTAL or PLL as the source of clock would seize to function after this function call.
      * The clocks are shutdown to minimize idle mode current consumption. The clocks are turned back on when one of 
      * the following events occur:
      * 1. Audio Input or Output stream becomes active
      * 2. MIDI polling starts
      * 3. Source of audio is selected as Aux in using the CapSense button on the board
      * Details of all the system clocks in idle mode are shown in "Idle Mode Clock Info" page of the TopDesign. */
    ConfigureAudioPath();
    
    /* Initialize the components other than audio components such as CapSense, LCD and timer etc. 
	 * Also configure the clock sources of all these components such that these components work in both idle and active modes.
	 * Details of all the system clocks in idle mode are shown in "Idle Mode Clock Info" page of the TopDesign.
     * Details of all the system clocks in idle mode are shown in "Idle Mode Clock Info" page of the TopDesign. */
    ConfigureApplication();
    
    /* USER_CODE: Placeholder for users to add any other application specific initialization code.
     * CAUTION: System will be in Idle mode at this point, which means some of the clocks are not active. 
	 * Details of all the system clocks in idle mode are shown in "Idle Mode Clock Info" page of the TopDesign. */
    
    while(FOREVER)
    {
		
        /* If not in Aux mode, check for Mac/PC connection 
		 * IS_HOST_CONNECTED() function does following tasks 
		 * 1. Checks the PSOC_VBUS_MON pin to determine whether Mac/PC is connected and returns 1 if connected.
		*/
        if(IS_HOST_CONNECTED() && IS_AUX_NOT_SELECTED())
        {
            /* Check for USB enumeration with either Apple device or Mac/PC USB host */
            if(IsUSBConfigured() != FALSE)
            {
                /* Accessory initialization routine after USB enumeration completion 
				 * Also loads the EP's when audio playback/recording starts */
				ServiceUSB();  
                
                /* Enter low power mode if Audio stream is inactive. In low power mode all the audio components
                 * (I2S, PGA, Opamp) and some of the system clocks (PLL, XTAL) are shutdown. 
				 * Details of all the system clocks in idle mode are shown in "Idle Mode Clock Info" page of the TopDesign.
				 * Details of all the system clocks in active mode are shown in "Active Mode Clock 44.1/48 kHz" page of the TopDesign.						 *  */
				#ifndef AUX_DETECTION_ENABLE
                HandleDigitalAudioLowPowerMode(); 
                #endif
                
                /* USB audio sampling frequency change handler.
				 * Checks whether host has changed audio sampling rate.
				 * Configures AudioClkGen to generate a clock synchonized to new audio sampling rate. 
				 * Note: Change the I2C address of codec in Codec.h file, according to Codec used in your design. 
                 * Currently, codec I2C address is set to Cirrus codec (CY8CKIT-033A onboard codec) address.
				 */
                HandleSamplingFrequencyChangeRequest(); 
                
                #ifdef CDC_ENABLED
                UARTBridgeComm();    
                #endif
                
                #ifdef MIDI_ENABLED
                    #ifdef MIDI_OUT_SERVICE_FROM_MAIN
				/* Disable USBiAP_MIDI_OUT_Service routine call from USB EP8 ISR (located inside USBiAP_episr.c file) */	
                    USBiAP_MIDI_OUT_EP_Service(); 
                    #endif
                #endif
            }
        }
        else
        {
            /* After Apple device/PC/Mac disconnection, restore default accessory settings */
            RestoreDefaultAccessorySettings(); 
        }
       
        if(1)
        {
            HandleApplicationInterface();  /* Handles all the Apple device track related application layer functions */
			
			#ifdef ENABLE_MAC_PC
			HandlePCMacUSBInterface();     /* Check for PC/Mac USB Audio device connection */
			#endif
        }
        
        HandleApplicationTimerTick();      /* Handle all the timeout events */

        
        #ifdef WATCHDOG_ENABLED
        CyWdtClear();                      /* Service watchdog timer */
        #endif
                
        #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
        HandleAudioInBuffer();             /* Clear audio IN buffer when IN stream is stopped */
        #endif
    }
}

/* [] END OF FILE */
