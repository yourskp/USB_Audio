/*******************************************************************************
* File Name: Configuration.h
*
* Version 4.0
*
*  Description: Configuration.h provides various configuration options for user 
*               to enable/disable and control different features provided by firmware 
*
********************************************************************************
* Copyright (2008-2013), Cypress Semiconductor Corporation.
********************************************************************************
* This software is  by Cypress Semiconductor Corporation (Cypress) and is
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

#if !defined(CONFIGURATION_H)
#define CONFIGURATION_H
#include <cytypes.h>
#include <project.h>


/* Compiler directives for enabling different firmware features */

/* Enables the MAC/PC support along with iOS */
#define ENABLE_MAC_PC

/* Note: The below #defines only disables the code for corresponding features, to reduce the code size completely,
 * remove the components corresponding to each of these features from the project schematic. For example,
 * if #CAPSENSE_ENABLED is commented out below, remove the CapSense component from the schematic "UI" page */
 
#define CAPSENSE_ENABLED                  /* Enables CapSense scanning and button/slider trigger events */

#define ENABLE_VOLUME_CONTROL             /* Volume control through USB and CapSense slider enabled */

#define LCD_MODULE_ENABLED                /* Character LCD display enabled */

#define WATCHDOG_ENABLED                  /* Enable watchdog for recovering from system faults */


	/* Enables the code for audio aux mode with CapSense button control. If you want to automatically 
	* detect aux source without using CapSense button then comment the below line and uncomment 
	* the "AUX_DETECTION_ENABLE" */
//	#define ENABLE_AUX_MODE

	/* Enables detection of aux source and switches automatically from aux to USB audio.
	* For aux detection to work, place an ISR component in TopDesign with name isr_Aux
	* and connect to nrq terminal of the I2S_Rx_DMA component. If you want CapSense button 
	* control for the aux mode then comment the below line and uncomment the "ENABLE_AUX_MODE" */
//	#define AUX_DETECTION_ENABLE 


	#define TRACK_INFO_DISPLAY_ENABLED        /* Track info display on the LCD is enabled */
	    
//	#define IR_RECEIVER_ENABLED               /* Enables IR remote receiver that controls Apple device playlist */
		
		
	/* Some of the codecs requires audio master clock (MCLK) to be always ON (even when audio streaming is inactive).
	 * Uncomment MCLK_ALWAYS_ENABLED directive to support such a codec/design. */
	//#define MCLK_ALWAYS_ENABLED 
	
/* Below is the macro which enable/disable USB suspend feature available in apple device powered mode. 
 * Apple device powered mode can be enabled by selecting iAP2 component "Power Source" as "Apple Device Powered" */

	/* In USB bus powered mode, uncomment HANDLE_USB_SUSPEND directive to support USB suspend and wakeup features */
	//#define HANDLE_USB_SUSPEND    


/* Enable ENABLE_DIGITAL_AUDIO_OUT_ONLY directive when audio out stream alone is supported (Speaker docks for example).
 * Enabling ENABLE_DIGITAL_AUDIO_OUT_ONLY disables all the code related to audio in stream handling.
 * If you wish to support both audio in and out streams, then comment out both "ENABLE_DIGITAL_AUDIO_OUT_ONLY" and 
 * "ENABLE_DIGITAL_AUDIO_IN_ONLY" directives */
//#define ENABLE_DIGITAL_AUDIO_OUT_ONLY

/* Enable ENABLE_DIGITAL_AUDIO_IN_ONLY directive when audio in stream alone is supported (Microphone design for example).
 * Enabling ENABLE_DIGITAL_AUDIO_IN_ONLY disables all the code related to audio out stream handling.
 * If you wish to support both audio in and out streams, then comment out both "ENABLE_DIGITAL_AUDIO_OUT_ONLY" and 
 * "ENABLE_DIGITAL_AUDIO_IN_ONLY" directives.
 * To enable audio in stream, you should also add linker Overlay command for EP2(audio in EP) DMA initialization routine, 
 * to remove reentrancy warnings. Go to  Build Settings -> DP8051 Keil 9.03 -> Linker -> Command line and add 
 * "USBFS_ep2_DmaInitialize ! *" to existing OVERLAY directive parameter list.*/
//#define ENABLE_DIGITAL_AUDIO_IN_ONLY      

/* To enable MIDI uncomment below macro. 
 * In Music_Creation example project MIDI is enabled without external mode (MIDI with UART) by default. 
 * To enable external mode, double click on USBFS component, go to  "MIDI Descriptor" tab and check the "External Mode". 
*/
//#define MIDI_ENABLED                      

//#define SPDIF_ENABLED                     /* Enable USB to SPDIF Audio Data transfer (TX Only) */

/* Define the following macros based on the system requirement. Refer to CY8CKIT-033A user guide FAQ section for 
 * details of these macros. Contact Cypress technical support if you are not sure of when to enable the following 
 * macro definitions */

//#define MIDI_OUT_SERVICE_FROM_MAIN              /* Based on the system requirement, to prevent interrupt priority
                                                  /* issues, enable USB MIDI out service from main. If this macro is
                                                  /* enabled, disable USBFS_MIDI_OUT_EP_Service call inside USB EP8
                                                   * ISR (located inside USBFS_episr.c file)*/
#define PHONE_CONTROL_ENABLED
    
#ifndef MIDI_ENABLED	
#define CDC_ENABLED                         /* CDC and MIDI are using the same endpoint, make sure they both are
                                             * not simultaneously enabled */
#endif

#endif

/* [] END OF FILE */
