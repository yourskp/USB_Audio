/*******************************************************************************
* File Name: Application.c
*
* Version 4.0
*
*  Description: This file contains all the application layer code which uses
*               iAP commands and user interface handling routines
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
#include <device.h>
#include <Interrupts.h>
#include <IRReceiver.h>
#include <LCD.h>
#include <stdio.h>
#include <USBInterface.h>
#include <VolumeControl.h>
#include <I2C_Master.h>

/* function declaration */
static void HandleUserInputs(void);
static void DisplayTrackInfo(void);
static void CheckNextTrackButton(void);
static void CheckPreviousTrackButton(void);
static void CheckPlayPauseButton(void);
static void CheckPlaybackMIDIButton(void);
static void CheckAuxSelection(void);

/* Extern declarations */
#ifdef AUX_DETECTION_ENABLE
extern uint8 auxDetected;

#endif
extern CYBIT lowPowerIdle;
extern CYPDATA uint8 audioSource;
extern CYDATA uint8 auxConfigured;
extern uint8 USBiAP_initVar;
extern CYBIT auxInStatus;
extern uint8 CapSense_SensorEnableMask[];
extern volatile uint8 USBFS_currentVolume[];
extern volatile uint8 USBFS_currentMute;
extern uint8 irStickyKey;

extern CYPDATA uint8 setRate;
extern CYBIT sendStartHID;
extern CYPDATA uint8 systemAudioSource;

extern CYBIT codecInit;
#ifdef MIDI_ENABLED
extern CYBIT inPlaying;
extern volatile uint8 usbMidiActive;
extern volatile uint8 USBFS_device;
#endif
extern CYBIT outPlaying;

extern CYPDATA uint8 clockSwitchTimer;
/* Indicates that track info data has been received from Apple device. 
 * It is used to prevent any LCD message overwriting the track info */
CYBIT displayTrackInfoEnabled = FALSE;
#ifdef TRACK_INFO_DISPLAY_ENABLED
/* Global variables used for track info display */
CYBIT getCurrentTrackIndex = TRUE;
CYBIT trackChanged = FALSE;
uint8 trackinfoState = TRACK_TITLE_STATE;
uint8 currentStartIndex = 0;
uint32 currentTrackIndex;
uint8 delayCount = 0;
#endif

/* Global variables used for playlist-control buttons */
uint8 buttonStatus = 0;
uint32 buttonMask;
uint8 previousTrackTimer = 0;
uint8 nextTrackTimer = 0;
uint8 playPauseTimer = 0;
CYBIT applicationInitiate = TRUE;
#ifdef PHONE_CONTROL_ENABLED
uint16 playlistControlReport = ((uint16)MAC_PC_PHONE_REPORT_ID << 8);    
#else    
uint16 playlistControlReport = ((uint16)MAC_PC_PLAYLIST_REPORT_ID << 8);
#endif
uint16 prevReport = 0;

/* Global variables used for the switch "PSOC_SW", which is used for switching CapSense between 
 * playlist control mode and MIDI mode */
uint8 switchDebounceTimer = 0;
uint8 lcdSwitchTimer = 0;
/* Indicates whether MIDI mode of CapSense is enabled. 
 * In MIDI mode, CapSense buttons are used for generating MIDI messages and slider is used to control the pitch. */
CYBIT midiControlEnabled = FALSE;
/* The global variable "usbMidiActive" indicates the status of the MIDI interface */
volatile uint8 usbMidiActive = FALSE;  
/* Global variables used with CapSense buttons and slider for generating MIDI messages */
#ifdef MIDI_ENABLED
volatile uint8 csBtnStates = 0;
volatile uint8 csBtnChange = 0;
volatile uint8 csSliderVal = 0;
#endif

/* Stores CapSense centroid. This is used in MIDI pitch control with CapSense and also in volume control depending 
 * on the CapSense mode selected by "PSOC_SW" switch */
volatile uint8 centroid = 0;

/* Indicates whether system mode is valid or invalid 
 * based on one or multiple hosts are connected respectively. It is set in CheckSystemMode() function. */
uint8 systemModeFlag = VALID_MODE;


/* Indicates whether the IR is started or not */
#ifdef IR_RECEIVER_ENABLED
CYBIT startIR= TRUE; 
#endif

#ifdef AUX_DETECTION_ENABLE
/* The global variable "inAuxMode" is used to indicate that aux mode is active. This variable is used only when 
 * aux detection is enabled */
CYBIT inAuxMode=0;
#endif 


uint8 userDisplayTimer = 0;
extern uint8 USBFS_initVar;
/*******************************************************************************
* Function Name: ConfigureApplication
********************************************************************************
* Summary:
*        This function configures the application layer hardware modules and
*        clocks to be in sync with the audio clock updates done in 
*        ConfigureAudioPath API
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ConfigureApplication(void)
{
    CYGlobalIntEnable;
	
    /*Change CapSense scan clock from XTAL to IMO so it will work when XTAL is turned off for idle mode */
    #ifdef CAPSENSE_ENABLED
    CapSense_IntClock_SetSource(CYCLK_SRC_SEL_IMO);
    #endif
   
//    I2C_Master_BusClock_SetSource(CYCLK_SRC_SEL_IMO);
    
    I2C_Master_Start();
            
    
    SetMaxI2CBusSpeed();
    
    #ifdef LCD_MODULE_ENABLED
    LCD_Reset();
    LCD_Start();
    LCD2LineDisplay("  USB Audio DVK ",
                    "                ");
    
    #ifdef WATCHDOG_ENABLED
    /* If watchdog reset occurred, display user message. CyResetStatus is a global variable exposed by
     * CY_Boot component, reading the reset status register will give an erroneous value */
	if(CyResetStatus & WATCHDOG_RESET_MASK)
    {
       LCD2LineDisplay("System Recovery ",
                        "   Kicked In    ");
        CyDelay(500);
    }
    #endif
    #endif
        
    /* Initialize CapSense component*/
    #ifdef CAPSENSE_ENABLED
    CapSense_Start();                    /* Start the CapSense module */

    CapSense_InitializeAllBaselines();  /* Initialize the baseline for all the buttons and slider sensors */
    #endif
    
    /* Enable 8ms tick sleep ISR */
    SystemTickTimer_Start();
    isr_Tick_StartEx(Tick_Interrupt);
    
    #ifdef WATCHDOG_ENABLED
    /* Watchdog timer is clocked from internal 1KHz ILO, for USB Operation 100KHz ILO is selected in the clock
     * configuration wizard, explicitly enable 1KHz ILO for WDT */
    CyILO_Start1K();
    
    /* Enable Watchdog for recovering from unresponsive system, The time for WDT is set to 2 1sec tick events */
    CyWdtStart(CYWDT_1024_TICKS, CYWDT_LPMODE_DISABLED);
    #endif
	
	/* Initialize the aux detection code */
	#ifdef AUX_DETECTION_ENABLE
	lowPowerIdle = FALSE;
	StartAudioComponents();            /* Turn on XTAL/PLL */
    StartAnalogAudioComponents();      /* Turn on Analog path for Audio-In/Apple device Analog */
    CyPins_SetPin(PSOC_CODEC_RST_0);   /* Turn on CODEC by releasing reset */
	SetClockRate(RATE_48KHZ);
	AuxDetection_Start();
	#endif
    
    /* USER_CODE: [Accessory Initialization] Placeholder for initializing additional user added components and 
     * external peripherals. This block of code is called once after accessory reset event and the system clocks 
     * will be in idle mode (See TopDesign.cysch for idle mode clock details) */
}

/*******************************************************************************
* Function Name: HandleApplicationInterface
********************************************************************************
* Summary:
*        This function handles all the application layer related initialization
*        and Apple device playlist control and power updates
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void HandleApplicationInterface(void)
{

	RunApplication(); /* application layer code for the audio dock platform */

	/* Check for Aux input selection only in self powered mode */
    if (midiControlEnabled == FALSE)
    {
        #ifdef ENABLE_AUX_MODE
       /* Check if Aux In CapSense button is pressed. 
        * Aux button is inactive when MIDI control mode is enabled for CapSense interface */
        CheckAuxSelection(); 
        #endif
    }
}

/*******************************************************************************
* Function Name: RunApplication
********************************************************************************
* Summary:
*      This function runs the application layer firmware for handling the Apple
*      device playlist (based on CapSense button user inputs), display the 
*      extracted track information on the character LCD, manage power updates,
*	   service Aux activity and handle volume updates
*   
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void RunApplication(void)
{
	/* Start the IR */
	#ifdef IR_RECEIVER_ENABLED
	if ((IsUSBConfigured() != FALSE))
	{
	    if (startIR)
	    {
            SetupIRReceiver(); 
            startIR=0;
	    }
	}
    #endif
	
    HandleUserInputs(); /* handle CapSense button and slider user inputs */
    
	UpdateAudioStatusUI(); /* Update audio playback, recording status on the LEDs */
	
	#ifdef IR_RECEIVER_ENABLED
	/* Checks IR activity and sends necessary  command to Host depending on the IR key pressed */
	ReportIRRemoteEvents();
	#endif 
	
    #ifdef CAPSENSE_ENABLED
        if(!CapSense_IsBusy()) /* scan all CapSense buttons and sliders */
        {
            CapSense_UpdateEnabledBaselines(); /* update the baseline for the CapSense algorithm */
            CapSense_ScanEnabledWidgets();
        }
        
        #ifdef MIDI_ENABLED
        if (midiControlEnabled == TRUE && IsUSBConfigured())
        {
            ProcessMidiControlMessages();
        }
        #endif
    #endif
    
    #ifdef ENABLE_VOLUME_CONTROL
    HandleUSBVolumeUpdate();
    #endif
	
	#ifdef AUX_DETECTION_ENABLE
	ServiceAuxActivity();		
	#endif
}

/*******************************************************************************
* Function Name: HandleUserInputs
********************************************************************************
* Summary:  Handles CapSense button user inputs for next/previous/play track 
*           functions and press button for playback/MIDI mode switch
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void HandleUserInputs(void)
{
    /* check if playback/MIDI mode switch button is pressed */            
    CheckPlaybackMIDIButton();
    
    /* If there is a flow control message from the Apple device, iAP1_IsTimerFree returns false till the time indicated
     * by the flow control message has elapsed. CapSense is used for MIDI control when MIDI control is enabled */
    if(IS_AUX_NOT_SELECTED())
    {
		CheckNextTrackButton();
        
        CheckPreviousTrackButton();
        
        CheckPlayPauseButton();
    }
    
	#ifdef ENABLE_MAC_PC
    if(IS_MAC_PC_PLAYLIST_CONTROL_NEEDED())
    {
        prevReport = playlistControlReport;
        USBFS_LoadInEP(MAC_PC_HID_CONTROL_ENDPOINT, USBFS_NULL, sizeof(playlistControlReport));
    }
	#endif
}


/*******************************************************************************
* Function Name: CheckNextTrackButton
********************************************************************************
* Summary: Check for Next Track button press and take necessary action
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void CheckNextTrackButton(void)
{
    /* Check if either CapSense button is pressed or an IR remote key code for Next track button is received */
    if(IS_NEXT_KEY_PRESSED())
    {
        if((!(buttonStatus & NEXT_TRACK_MASK) || ((buttonStatus & NEXT_TRACK_MASK) && (nextTrackTimer == 0))))
        {
            /* Indicate next track button press */
            buttonStatus = buttonStatus | NEXT_TRACK_MASK;
            
            nextTrackTimer = SIMPLE_REMOTE_TIMEOUT_INTERVAL;
            
            /* Handle playlist control for Mac/PC through HID consumer page controls and for iOS devices through
             * iAP commands */
            if(IsMacPCConnected())
            {
                #ifdef PHONE_CONTROL_ENABLED
                playlistControlReport = playlistControlReport | MAC_PC_PHONE_DROP_MASK;    
                #else    
                playlistControlReport = playlistControlReport | MAC_PC_NEXT_TRACK_MASK;
                #endif    
            }
			            
            #ifdef IR_RECEIVER_ENABLED
                /* Apple remote IR repeat interval is 108ms, whereas the Simple remote lingo, repeat key interval
                 * is between 30 to 100ms, to handle IR repeat key events properly over iAP simple remote lingo
                 * commands, a sticky key is defined, where for every IR key press, the simple remote lingo command is
                 * sent twice before indicating all button up command. If the IR key does send a repeat key within the
                 * 100 ms interval, then simple remote lingo command is repeated thereby providing fast forward and rewind
                 * functionality from the remote */
                irStickyKey++;
                
                if(irStickyKey == IR_MAXIMUM_STICKY_KEY_COUNT)
                {
                    IR_ClearNextTrackKeyPress();
                }
            #endif
        }
    }
    else if (((buttonStatus & NEXT_TRACK_MASK) && (nextTrackTimer == 0)))
    {
        buttonStatus = buttonStatus & (~NEXT_TRACK_MASK);
        
        if(IsMacPCConnected())
        {
            #ifdef PHONE_CONTROL_ENABLED
                playlistControlReport = playlistControlReport & (~MAC_PC_PHONE_DROP_MASK);    
            #else    
                playlistControlReport = playlistControlReport & (~MAC_PC_NEXT_TRACK_MASK);
            #endif    
        }
    }
}

/*******************************************************************************
* Function Name: CheckPreviousTrackButton
********************************************************************************
* Summary: Check for Previous Track button press and take necessary action
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void CheckPreviousTrackButton(void)
{
    /* Check if either CapSense button is pressed or an IR remote key code for Previous track button is received */
    if(IS_PREV_KEY_PRESSED())
    {
        if((!(buttonStatus & PREVIOUS_TRACK_MASK) || ((buttonStatus & PREVIOUS_TRACK_MASK) && previousTrackTimer == 0)))
        {
            /* register button press event */
            buttonStatus = buttonStatus | PREVIOUS_TRACK_MASK;
			
            previousTrackTimer = SIMPLE_REMOTE_TIMEOUT_INTERVAL;
            
            if(IsMacPCConnected())
            {
                #ifdef PHONE_CONTROL_ENABLED
                playlistControlReport = playlistControlReport | MAC_PC_PHONE_MUTE_MASK;    
                #else    
                playlistControlReport = playlistControlReport | MAC_PC_PREVIOUS_TRACK_MASK;
                #endif    
            }
			
            
            #ifdef IR_RECEIVER_ENABLED                                        
            irStickyKey++;
            
            if(irStickyKey == IR_MAXIMUM_STICKY_KEY_COUNT)
            {
                IR_ClearPrevTrackKeyPress();
            }
            #endif
        }
    }
    else if (((buttonStatus & PREVIOUS_TRACK_MASK) && (previousTrackTimer == 0)))
    {    
        /* button pressed and released, take action now */
        buttonStatus = buttonStatus & (~PREVIOUS_TRACK_MASK);
        
        if(IsMacPCConnected())
        {
            #ifdef PHONE_CONTROL_ENABLED
                playlistControlReport = playlistControlReport & (~MAC_PC_PHONE_MUTE_MASK);
            #else    
                playlistControlReport = playlistControlReport & (~MAC_PC_PREVIOUS_TRACK_MASK);
            #endif    
        }
        
    }
}

/*******************************************************************************
* Function Name: CheckPlayPauseButton
********************************************************************************
* Summary: Check whether Play/Pause CapSense or IR remote button is pressed and 
*          take necessary action
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void CheckPlayPauseButton(void)
{   
    /* Check if either CapSense button is pressed or an IR remote key code for Play/Pause button is received */
    if(IS_PLAY_PAUSE_KEY_PRESSED())
    {
        if((!(buttonStatus & PLAY_PAUSE_MASK) || ((buttonStatus & PLAY_PAUSE_MASK) && playPauseTimer == 0)))
        {
            buttonStatus = buttonStatus | PLAY_PAUSE_MASK; /* register button press event */
            
            playPauseTimer = SIMPLE_REMOTE_TIMEOUT_INTERVAL;
            
            if(IsMacPCConnected())
            {
                playlistControlReport = playlistControlReport | MAC_PC_PLAY_PAUSE_MASK;
            }
			                                                                
            #ifdef IR_RECEIVER_ENABLED                                        
            irStickyKey++;
            
            if(irStickyKey == IR_MAXIMUM_STICKY_KEY_COUNT)
            {                                        
                IR_ClearPlayTrackKeyPress();                                
            }
            #endif
        }
    }
    else if(((buttonStatus & PLAY_PAUSE_MASK) && (playPauseTimer == 0)))
    {
        buttonStatus = buttonStatus & (~PLAY_PAUSE_MASK);
        
        if(IsMacPCConnected())
        {
            playlistControlReport = playlistControlReport & (~MAC_PC_PLAY_PAUSE_MASK);
        }
		
    }
}

/*******************************************************************************
* Function Name: CheckPlaybackMIDIButton
********************************************************************************
* Summary: Check if MIDI/Playback toggle switch is pressed and switch the CapSense
*          button functionality accordingly. Debouncing of the mechanical switch
*          is also handled within this routine. Also handles additional feature of
*		   Siri or Home button emulation depending on double tap or continuous 
*		   press, if SIRI_ENABLED macro is enable in Configuration.h file
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void CheckPlaybackMIDIButton(void)
{   
    #if defined (MIDI_ENABLED)
    if(PSOC_SW_Read() == FALSE)
    {
        /* Register the button press for the first time and check if button debouncing is completed */
        if(!(buttonStatus & (MIDI_PLAYBACK_PUSHED_MASK | MIDI_PLAYBACK_DETECTED_MASK)))
        {
            
            /* If button press not registered, register the MIDI button press event*/
            buttonStatus = buttonStatus | MIDI_PLAYBACK_PUSHED_MASK;
            
            /* Initiate a debounce counter */
            switchDebounceTimer = SWITCH_DEBOUNCE_DELAY;
        }
        else if( (buttonStatus & MIDI_PLAYBACK_PUSHED_MASK) && (switchDebounceTimer == 0) )
        {
            /* If debounce counter has elapsed, then register a button press detect event */
            buttonStatus = buttonStatus & (~MIDI_PLAYBACK_PUSHED_MASK);
            buttonStatus = buttonStatus | MIDI_PLAYBACK_DETECTED_MASK;
        }
    }
    else
    {
        if((buttonStatus & MIDI_PLAYBACK_DETECTED_MASK))
        {
            /* USER_CODE: [Push button event] Push button SW101 pressed and released on 033A Baseboard. Add your custom
             * code for push button based events or debugging code */
            
            /* MIDI control is enabled only when Digital Audio is active */
            #ifdef MIDI_ENABLED
            if(GetSystemAudioSource() == AUDIO_SOURCE_DIGITAL)
            {
                /* toggle the MIDI control flag so as to toggle CapSense buttons/slider functionality
                 * from MIDI control to Playback control and vice versa */
                midiControlEnabled = midiControlEnabled ^ 0x01;
                
                if(midiControlEnabled)
                {
                    #ifdef LCD_MODULE_ENABLED
                        LCD_Position(1,12);
                        LCD_PrintString("MIDI");
                    #endif
                }
                else
                {
                    #ifdef LCD_MODULE_ENABLED
                        LCD_Position(1,12);
                        LCD_PrintString(" Dig");
                    #endif
                }
                
                #ifdef LCD_MODULE_ENABLED
                lcdSwitchTimer = LCD_SWITCHBACK_DELAY;
                #endif
            }
            #endif
            buttonStatus = buttonStatus & ( ~(MIDI_PLAYBACK_DETECTED_MASK) );
        }
        /* If button press is not detected and the switch debounce timer has expired, clear the button press status
         * in the ButtonStatus register */
        else if((buttonStatus & MIDI_PLAYBACK_PUSHED_MASK) && switchDebounceTimer == 0)
        {
             buttonStatus = buttonStatus & (~MIDI_PLAYBACK_PUSHED_MASK);
        }
    }
    #endif
 }

#ifdef MIDI_ENABLED
/*******************************************************************************
* Function Name: ProcessMidiControlMessages
********************************************************************************
* Summary: This function processes the CapSense buttons and slider for MIDI  
*  		   Control messages and puts the MIDI Note-On/Off message into the 
*		   MIDI IN end-point
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ProcessMidiControlMessages(void)
{
    /* MIDI control message handling is active only when CapSense is enabled in the project */
	#ifdef CAPSENSE_ENABLED
    uint8 tmp8 = 0;
    uint16 tmp16;
    uint8 midiMsg[4];
	uint8 buttonCheckIndex;
    
    if (CapSense_CheckIsWidgetActive(CapSense_PSOC_NEXT__BTN))
    {
        tmp8 |= BTN2;
    }    
    if (CapSense_CheckIsWidgetActive(CapSense_PSOC_PREV__BTN))
    {
        tmp8 |= BTN1;
    }    
    if (CapSense_CheckIsWidgetActive(CapSense_PSOC_PLAY__BTN))
    {
        tmp8 |= BTN3;
    }    
    if (CapSense_CheckIsWidgetActive(CapSense_PSOC_LOOPBACK__BTN))
    {
        tmp8 |= BTN4;
    }
    
    if ((csBtnChange = tmp8 ^ csBtnStates) != 0) /* Process any button changes */
    {
        csBtnStates = tmp8;
        
        midiMsg[0] = USBFS_MIDI_NOTE_ON; /* all buttons are mapped to Note-On/Off messages */
       
	    for(buttonCheckIndex = BTN1; buttonCheckIndex <=BTN4;buttonCheckIndex = buttonCheckIndex<<1)
	    {
	    	if (csBtnChange & buttonCheckIndex) /* Button 1 */
	        {
                midiMsg[1] = NOTE_NUMBER + (buttonCheckIndex<<2); /* Button determines note number */
                 
                if (csBtnStates & buttonCheckIndex)
                {
                    midiMsg[2] = NOTE_ON;        /* Note On */
                }    
                else
                {
                    midiMsg[2] = NOTE_OFF;       /* Note Off */
                }    
                USBFS_PutUsbMidiIn(3, midiMsg, USBiAP_MIDI_CABLE_01);
	        }
	    }
    }
    
    centroid = CapSense_GetCentroidPos(CapSense_PSOC_SLIDER__LS); /* Check for the slider being touched */
    
    if(centroid != SLIDER_NOT_ACTIVE)
    {
        if (centroid != csSliderVal) 
        {
            csSliderVal = centroid;
            tmp8 = csSliderVal;
            if (tmp8 < PITCH_CONTROL_MIN)
            {
                tmp8 = PITCH_CONTROL_MIN;
            }    
            else if (tmp8 > PITCH_CONTROL_MAX)
            {
                tmp8 = PITCH_CONTROL_MAX;
            }
            
            tmp16 = (uint16)tmp8 - PITCH_CONTROL_MIN;
            
            tmp16 = tmp16 * LINEAR_SWIPE_COUNT_MULTIPLIER;
            
            midiMsg[0] = USBFS_MIDI_PITCH_BEND_CHANGE;
            midiMsg[1] = HI8(tmp16);
            midiMsg[2] = LO8(tmp16) & 0xFE;
            USBFS_PutUsbMidiIn(3, midiMsg, USBFS_MIDI_CABLE_01);
        }
    }
	
	/* USER_CODE: Placeholder for users to send any other embedded MIDI messages */
	
    /* Indicate the MIDI service to send the previously constructed MIDI messages to the host, enable the below
     * commented line of code if MIDI input through external MIDI jack is serviced in ISR instead of main loop */
    if((IsUSBMidiActive() 
	  #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
	  && !inPlaying 
	  #endif
	  #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
	  && !outPlaying
	  #endif
	  ))
	{
		USBFS_MIDI_IN_Service();
	}	
    #endif
}
#endif

/*******************************************************************************
* Function Name: USBiAP_callbackLocalMidiEvent
********************************************************************************
* Summary:
*  This function is a callback routine for MIDI out events. Depending on the 
*  type of MIDI message being sent out, a particular LED is turned On or off on 
*  the RDK
*
* Parameters:
*  cable -  USBiAP_MIDI_CABLE_01 or MIDI_CABLE_02 which is the source of this event
*  midiMsg - Pointer to MIDI message buffer
*
* Return:
*  void
*
*******************************************************************************/
void USBiAP_callbackLocalMidiEvent(uint8 cable, uint8 *midiMsg) CYREENTRANT
{
    /* MIDI callback is not handled in this project */
    cable = *midiMsg; /* Preventing compiler warning */
    
    /* USER_CODE:[MIDI] Users can add their custom code here to handle embedded MIDI events received from the USB host */
}

/*******************************************************************************
* Function Name: CheckAuxSelection
********************************************************************************
* Summary: This function scans the Aux-select CapSense button and process 
*          changes in audio path for AUX mode when button is pushed.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void CheckAuxSelection(void)
{
        #ifdef CAPSENSE_ENABLED
        /* Check whether Aux button is pressed */
        if(CapSense_CheckIsWidgetActive(CapSense_PSOC_LOOPBACK__BTN) && 
        !(buttonStatus & AUX_MASK))
        {
            /* Indicate AUX button press */
            buttonStatus = buttonStatus | AUX_MASK;
        }
        else if(!CapSense_CheckIsWidgetActive(CapSense_PSOC_LOOPBACK__BTN) &&
                (buttonStatus & AUX_MASK))
        {
            /* Toggle the Aux button status */
            buttonStatus = buttonStatus & ~AUX_MASK;
                       
            /* Toggle the Aux button status */
            if (!auxInStatus)
                auxInStatus = AUX_IN_MASK;
            else
                auxInStatus = FALSE;        
            
            if(auxInStatus & AUX_IN_MASK)
            {
                /* Configure Audio DMAs to handle AUX In Audio path Data */
                ConfigureAuxDMA();
                
                if(USBFS_initVar) /* Checking for USBiAP init state before stopping iAP */
                {
                    USBFS_Stop();
					/* start the I2C interface for communicating with peripherals */
                    USBDeviceState = USB_INTERFACE_INACTIVE;
                }
            }
            else
            {
                outPlaying = FALSE;
				USBFS_Start(PC_MAC_AUDIO_WITH_VOLUME_DEVICE, USBFS_3V_OPERATION);                
                I2S_Out_Select_Write(I2S_OUT_TO_I2S_OUT); //Swapping Dig Mux pins for USB 
            }
        }
        #endif
}



/*******************************************************************************
* Function Name: RestoreDefaultAccessorySettings
********************************************************************************
* Summary:
*       This routine restores the default state of the accessory once an Apple 
*       device is unplugged from the accessory. After completing this routine,
*       accessory is ready for processing new Apple device connection event.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void RestoreDefaultAccessorySettings(void)
{
    /* Restore the accessory state only when the accessory is previously configured properly  */
    if(USBDeviceState != USB_INTERFACE_INACTIVE && !usbMiniBActive)
    {
        /* When the Apple device is disconnected from the accessory, configure the accessory USB mode to inactive and 
         * wait for Apple device or PC plug-in event */ 
        USBDeviceState = USB_INTERFACE_INACTIVE;
        
        #ifdef MIDI_ENABLED
        midiEPInitialized = FALSE;
		midiPowerIdle = TRUE;
        #endif
        
        /* If Aux audio is not playing, enter low power mode irrespective of the previous state */
        if(IS_AUX_NOT_SELECTED())
        {
            lowPowerIdle = TRUE;
            StopAudioComponents();            /* Changes to 24MHz IMO for USB */
            StopAnalogAudioComponents();      /* Turn OFF Analog path for Audio-In path */
            
            auxConfigured = FALSE;
            
            #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
            Stop_I2S_Tx();
            #endif
            
            #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
            Stop_I2S_Rx();
            #endif
        }
        clockSwitchTimer = 0;
        
        /* USER_CODE: [Accessory Disconnection] Placeholder for user code when a self powered accessory is disconnected 
         * from an enumerated USB host (Mac/PC) and Aux loopback is not in active */
    }
    
    #ifdef IR_RECEIVER_ENABLED
    StopIRReceiver(); 
	startIR= TRUE; 
    #endif
        
    if(IS_AUX_NOT_SELECTED())               /* If Aux input is not selected and PC is disconnected */
    {
        #ifdef LCD_MODULE_ENABLED
        LCD2LineDisplay(" Connect MAC/PC ",
                        "                ");
        #endif
    }
    
    applicationInitiate = TRUE; /* Reset all the application layer flags */
  
    /* USER_CODE: [Accessory Disconnection] Placeholder for user code when a self powered accessory is disconnected from  
     * a USB host (Mac/PC) and Aux loopback is not in active */
}

/* [] END OF FILE */
