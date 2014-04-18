/*******************************************************************************
* File Name: Application.h
*
* Version 4.0
*
*  Description: Application.h provides various function prototypes for all the
*               Application tasks
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

#if !defined(APPLICATION_H)
#define APPLICATION_H
#include <Configuration.h>

/*  Function Prototype  */
void ConfigureApplication(void);          /* Configures application clocks and initializes components (iAP, CapSense etc.) */
void RunApplication(void);                /* User Application main routine */
void ProcessMidiControlMessages(void);    /* MIDI control inputs are processed and sent to MIDI USB Host */ 
void HandleApplicationTimerTick(void);    /* Application timer tick process routine */
void SetMaxI2CBusSpeed(void);             /* Set I2C Bus speed to I2C_MAX_BUS_SPEED value. Assumption:BUS_CLK = 49MHz */
void SetMinI2CBusSpeed(void);             /* Set I2C Bus speed to I2C_MIN_BUS_SPEED value. Assumption:BUS_CLK = 49MHz */
void CheckSystemMode(void);               /* Check if multiple USB hosts are connected to CY8CKIT-033A or not */
void RestoreDefaultAccessorySettings(void);
void HandleApplicationInterface(void);

/* Constants for USB descriptors */
#define PC_MAC_AUDIO_WITH_VOLUME_DEVICE        0x00
#define AUDIO_OUT_ENDPOINT                     0x01
#define AUDIO_IN_ENDPOINT                      0x02
#define MIDI_IN_ENDPOINT                       7
#define MIDI_OUT_ENDPOINT                      8

/* Macros for MIDI messages */
#define NOTE_ON                                100
#define NOTE_OFF                               0
#define NOTE_NUMBER                            72
#define BTN1                                   0x01
#define BTN2                                   0x02
#define BTN3                                   0x04
#define BTN4                                   0x08

/* Macros for MIDI pitch control */
#define PITCH_CONTROL_MIN                      5
#define PITCH_CONTROL_MAX                      95
#define PITCH_CONTROL_RANGE                    PITCH_CONTROL_MAX - PITCH_CONTROL_MIN)
#define LINEAR_SWIPE_COUNT_MULTIPLIER          364 /* Maximum pitch value / (PITCH_CONTROL_RANGE) */

/* Constants for Track Info Display  */
#define TRACK_TITLE_STATE                      0x01
#define ARTIST_NAME_STATE                      0x02
#define ALBUM_NAME_STATE                       0x03
#define INVALID_TRACK_INDEX                    -1
#define MAX_METADATA_SIZE                      32 /* Limits the number of characters for track title, artist name and album name */

/* Constants for LCD Display */
#define DELAY_COUNT_MAX                        40
#define MAX_LCD_COL_LENGTH                     16
#define INITIAL_SONG_INDEX                     0x00000000
#define INITIAL_CHAPTER_INDEX                  0x0000
#define SLIDER_NOT_ACTIVE                      0xFF
#define LCD_SWITCHBACK_DELAY                   250

/* Constants for elapsed time display */
#define NUM_SECONDS_PER_MINUTE                 60

/* Constants for CapSense buttons */
#define NEXT_TRACK_MASK                        0x01
#define PREVIOUS_TRACK_MASK                    0x02
#define PLAY_PAUSE_MASK                        0x04
#define AUX_MASK                               0x08
#define MIDI_PLAYBACK_PUSHED_MASK              0x10
#define MIDI_PLAYBACK_DETECTED_MASK            0x20

/* Constants for switch debounce */
#define SWITCH_DEBOUNCE_DELAY                  4

/* Constants for aux mode */
#define AUX_IN_MASK                            0x01

/* Constants for watchdog */
#define WATCHDOG_RESET_MASK                    0x08

/* Constants for playlist and volume control */
#define VOLUME_MUTE_CHANGE_MASK                0x10
#define PLAY_STATUS_MASK                       0x08
#define SIMPLE_REMOTE_TIMEOUT_INTERVAL         8

/* Constants for MAC/PC playlist control */
#define MAC_PC_PLAYLIST_REPORT_ID              0x01
#define MAC_PC_PLAY_PAUSE_MASK                 0x01
#define MAC_PC_NEXT_TRACK_MASK                 0x02
#define MAC_PC_PREVIOUS_TRACK_MASK             0x04
#define MAC_PC_HID_CONTROL_ENDPOINT            0x04

#define MAC_PC_PHONE_REPORT_ID                 0x02
#define MAC_PC_PHONE_DROP_MASK                 0x01
#define MAC_PC_PHONE_MUTE_MASK                 0x02
#define MAC_PC_HID_OUT_ENDPOINT                0x05


/* Constants for MIDI EP */
#define MIDI_POLLING_FAIL_MAX_COUNT            10
#define MIDI_POLL_FREQUENCY_COUNT              10

/* Constants for low power timing */
#define CLOCK_SWITCHING_TIMEOUT                250
#define CLOCK_SWITCH_TIMED_OUT                 1

/* Constants for I2C  */
#define I2C_BUS_SPEED_50KHZ                    0
#define I2C_BUS_SPEED_100KHZ                   1
#define I2C_BUS_SPEED_400KHZ                   2
#define I2C_DELAY_COUNT                        200
#define I2C_DIVIDER_400KHZ_OPERATION           8 /* I2C block clock divider for 400KHz operation when BUS_CLK = 49MHz */
#define I2C_DIVIDER_100KHZ_OPERATION           16/* I2C block clock divider for 100KHz operation when BUS_CLK = 49MHz */
#define I2C_DIVIDER_50KHZ_OPERATION            29/* I2C block clock divider for 50KHz operation when BUS_CLK = 49MHz */
#define I2C_DIVIDER_25KHZ_OPERATION            58/* I2C block clock divider for 50KHz operation when BUS_CLK = 49MHz */
#define iAP2_FIXED_FUNCTION_I2C_MODE           2

/* Only if Coprocessor version is B, then switch I2C minimum speed to 50KHz */
#define I2C_MIN_BUS_SPEED                      I2C_BUS_SPEED_400KHZ
#define I2C_MAX_BUS_SPEED                      I2C_BUS_SPEED_400KHZ

/* Different constants used in code  */
#define TRUE                                   1
#define FALSE                                  0
#define ZERO                                   0
#define FOREVER                                1
#define ENABLE                                 1
#define DISABLE                                0
#define VALID_MODE                             1
#define INVALID_MODE                           0


/* Macro definition for checking if host is connected and aux is not active */
#define IS_HOST_CONNECTED()           		   ( IsMacPCConnected()) 

#ifdef 	ENABLE_AUX_MODE
	#ifdef AUX_DETECTION_ENABLE
	#define IS_AUX_NOT_SELECTED()  ( !(inAuxMode || auxInStatus) )
	#else
	#define IS_AUX_NOT_SELECTED()  (!auxInStatus)
	#endif
#else
	#ifdef AUX_DETECTION_ENABLE
	#define IS_AUX_NOT_SELECTED()  (!inAuxMode)
	#else
	#define IS_AUX_NOT_SELECTED()  1
	#endif
#endif	

/* Macro definition for a next track button press */
#ifdef CAPSENSE_ENABLED 	
	#ifdef IR_RECEIVER_ENABLED  
	#define IS_NEXT_KEY_PRESSED() 	((CapSense_CheckIsWidgetActive(CapSense_PSOC_NEXT__BTN))&&(!midiControlEnabled))||(IR_IsNextTrackKeyPressed()) 
	#else 
	#define IS_NEXT_KEY_PRESSED()   (CapSense_CheckIsWidgetActive(CapSense_PSOC_NEXT__BTN) && !midiControlEnabled)
	#endif
#else
	#ifdef IR_RECEIVER_ENABLED  
	#define IS_NEXT_KEY_PRESSED() 	(IR_IsNextTrackKeyPressed()) 	
	#else 
	#define IS_NEXT_KEY_PRESSED()    FALSE
	#endif
#endif 	

/* Macro definition for a previous track button press */
#ifdef CAPSENSE_ENABLED 	
	#ifdef IR_RECEIVER_ENABLED  
	#define IS_PREV_KEY_PRESSED() 	(CapSense_CheckIsWidgetActive(CapSense_PSOC_PREV__BTN) && !midiControlEnabled) || (IR_IsPrevTrackKeyPressed()) 
	#else 
	#define IS_PREV_KEY_PRESSED()   (CapSense_CheckIsWidgetActive(CapSense_PSOC_PREV__BTN) && !midiControlEnabled)
	#endif
#else
	#ifdef IR_RECEIVER_ENABLED  
	#define IS_PREV_KEY_PRESSED() 	(IR_IsPrevTrackKeyPressed()) 	
	#else 
	#define IS_PREV_KEY_PRESSED()    FALSE
	#endif
#endif 	

/* Macro definition for a previous track button press */
#ifdef CAPSENSE_ENABLED 	
	#ifdef IR_RECEIVER_ENABLED  
	#define IS_PLAY_PAUSE_KEY_PRESSED()    (CapSense_CheckIsWidgetActive(CapSense_PSOC_PLAY__BTN) && !midiControlEnabled) || (IR_IsPlayTrackKeyPressed()) 
	#else 
	#define IS_PLAY_PAUSE_KEY_PRESSED()    (CapSense_CheckIsWidgetActive(CapSense_PSOC_PLAY__BTN) && !midiControlEnabled)
	#endif
#else
	#ifdef IR_RECEIVER_ENABLED  
	#define IS_PLAY_PAUSE_KEY_PRESSED()    (IR_IsPlayTrackKeyPressed()) 	
	#else 
	#define IS_PLAY_PAUSE_KEY_PRESSED()    FALSE
	#endif
#endif 	

/* Macro definition for MAC/PC playlist control */
#define IS_MAC_PC_PLAYLIST_CONTROL_NEEDED()		(IsMacPCConnected() && IsUSBConfigured()  \
												 && USBFS_GetEPState(MAC_PC_HID_CONTROL_ENDPOINT) == USBFS_EVENT_PENDING && prevReport \
												 != playlistControlReport )

extern uint8 systemModeFlag;
extern CYBIT auxInStatus;
#ifdef AUX_DETECTION_ENABLE
extern CYBIT inAuxMode;
#endif

#endif

/* [] END OF FILE */
