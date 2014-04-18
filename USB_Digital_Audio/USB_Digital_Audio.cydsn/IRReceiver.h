/*******************************************************************************
* File Name: IR_Receiver.h
*
* Version 4.0
*
*  Description: This file contains constants and declarations for  IR 
*               remote receiver implementation
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

#if !defined(IRRECEIVER_H)
#define IRRECEIVER_H

void SetupIRReceiver(void);
void StopIRReceiver(void);
void ReportIRRemoteEvents(void);
void ProcessIRReceiver(void);

extern uint8 irButtonMask;

#define IR_IsNextTrackKeyPressed()                      (irButtonMask & NEXT_TRACK_MASK)
#define IR_ClearNextTrackKeyPress()                     (irButtonMask = irButtonMask & ~NEXT_TRACK_MASK)
#define IR_IsPrevTrackKeyPressed()                      (irButtonMask & PREVIOUS_TRACK_MASK)
#define IR_ClearPrevTrackKeyPress()                     (irButtonMask = irButtonMask & ~PREVIOUS_TRACK_MASK)
#define IR_IsPlayTrackKeyPressed()                      (irButtonMask & PLAY_PAUSE_MASK)
#define IR_ClearPlayTrackKeyPress()                     (irButtonMask = irButtonMask & ~PLAY_PAUSE_MASK)

#define IR_RECEIVER_IDLE_STATE                          0x00
#define IR_RECEIVER_START_STATE                         0x01
#define IR_APPLE_REMOTE_DATA_STATE                      0x02
#define IR_RECEIVER_DATA_STATE                          0x03

#define IR_MIN_START_PULSE_WIDTH                        217    /* Ideally value of 13.5ms, 12.8ms Low and 14.8ms High */
#define IR_MAX_START_PULSE_WIDTH                        250

#define IR_MIN_LOGIC0_PULSE_WIDTH                       10    /* Ideal value of 1.125ms, .8ms Low and 1.5ms High */
#define IR_MAX_LOGIC0_PULSE_WIDTH                       27

#define IR_MIN_LOGIC1_PULSE_WIDTH                       28    /* Ideal value of 2.25ms, 2ms Low and 2.5ms High */
#define IR_MAX_LOGIC1_PULSE_WIDTH                       47

#define IR_MIN_REPEAT_PULSE_WIDTH                       165    /* Ideally value of 11.821ms, 11ms Low and 12.6ms High */
#define IR_MAX_REPEAT_PULSE_WIDTH                       214

#define NUMBER_OF_ADDRESS_BITS                          16
#define NUMBER_OF_DATA_BITS                             32

#define MENU_KEY                                        0xC0
#define PLAY_KEY                                        0xFA
#define RIGHT_KEY                                       0x60
#define LEFT_KEY                                        0x90
#define UP_KEY                                          0x50
#define DOWN_KEY                                        0x30
#define CENTER_KEY                                      0x3A

#define UP_KEY_MASK                                     0x40
#define DOWN_KEY_MASK                                   0x80

#define IR_MAXIMUM_STICKY_KEY_COUNT                     2
#define IR_COUNTE_PERIOD_VALUE                          255

#endif

/* [] END OF FILE */
