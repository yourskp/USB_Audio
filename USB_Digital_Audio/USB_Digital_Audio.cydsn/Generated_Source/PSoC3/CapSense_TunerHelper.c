/*******************************************************************************
* File Name: CapSense_TunerHelper.c
* Version 3.30
*
* Description:
*  This file provides the source code of Tuner helper APIs for the CapSense CSD 
*  component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "CapSense_TunerHelper.h"

#if (CapSense_TUNER_API_GENERATE)
    void CapSense_ProcessAllWidgets(volatile CapSense_OUTBOX *outbox)
	                                        ;
    
    volatile CapSense_MAILBOXES CapSense_mailboxesComm;

    extern uint8 CapSense_SensorOnMask[(((CapSense_TOTAL_SENSOR_COUNT - 1u) / 8u) + 1u)];
#endif  /* (CapSense_TUNER_API_GENERATE) */


/*******************************************************************************
* Function Name: CapSense_TunerStart
********************************************************************************
*
* Summary:
*  Initializes CapSense CSD component and EzI2C communication componenet to use
*  mailbox data structure for communication with Tuner GUI.
*  Start the scanning, after initialization complete.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void CapSense_TunerStart(void) 
{
    #if (CapSense_TUNER_API_GENERATE)
        
        /* Init mbx and quick check */
        CapSense_InitMailbox(&CapSense_mailboxesComm.csdMailbox);
        CapSense_mailboxesComm.numMailBoxes = CapSense_DEFAULT_MAILBOXES_NUMBER;
        
        /* Start CapSense and baselines */
        CapSense_Start();
        
        /* Initialize baselines */ 
        CapSense_InitializeAllBaselines();
        CapSense_InitializeAllBaselines();
        
        /* Start EzI2C, clears buf pointers */
        EzI2C_Start();
        
        /* Setup EzI2C buffers */
        EzI2C_SetBuffer1(sizeof(CapSense_mailboxesComm), sizeof(CapSense_mailboxesComm),
                                        (void *) &CapSense_mailboxesComm);
        
        /* Starts scan all enabled sensors */
        CapSense_ScanEnabledWidgets();
    
    #endif  /* (CapSense_TUNER_API_GENERATE) */
}


/*******************************************************************************
* Function Name: CapSense_TunerComm
********************************************************************************
*
* Summary:
*  This function is blocking. It waits till scaning loop is completed and apply
*  new parameters from Tuner GUI if available (manual tuning mode only). Updates
*  enabled baselines and state of widgets. Waits while Tuner GUI reports that 
*  content of mailbox could be modified. Then loads the report data into outbox 
*  and sets the busy flag. Starts new scanning loop.
*  
* Parameters:
*  None
*
* Return:
*  None
*
* Reentrant:
*  No
*
*******************************************************************************/
void CapSense_TunerComm(void) 
{
    #if (CapSense_TUNER_API_GENERATE)
        if (0u == CapSense_IsBusy())
        {   
            /* Apply new settings */
            #if (CapSense_TUNING_METHOD == CapSense_MANUAL_TUNING)
                CapSense_ReadMessage(&CapSense_mailboxesComm.csdMailbox);
            #endif  /* (CapSense_TUNING_METHOD == CapSense_MANUAL_TUNING) */

            /* Update all baselines and process all widgets */
            CapSense_UpdateEnabledBaselines();
            CapSense_ProcessAllWidgets(&CapSense_mailboxesComm.csdMailbox.outbox);
            CapSense_PostMessage(&CapSense_mailboxesComm.csdMailbox);

            /* Enable EZI2C interrupts, after scan complete */
            EzI2C_EnableInt();

            while((CapSense_mailboxesComm.csdMailbox.type != CapSense_TYPE_ID) || \
                  (EzI2C_GetActivity() & EzI2C_STATUS_BUSY)){}
            
            /* Disable EZI2C interrupts, while scanning */
            EzI2C_DisableInt();
            
            /* Start scan all sensors */
            CapSense_ScanEnabledWidgets();
        }
    #endif /* (CapSense_TUNER_API_GENERATE) */
}


#if (CapSense_TUNER_API_GENERATE)
    /*******************************************************************************
    * Function Name: CapSense_ProcessAllWidgets
    ********************************************************************************
    *
    * Summary:
    *  Call required functions to update all widgets state:
    *   - CapSense_GetCentroidPos() - calls only if linear sliders 
    *     available.
    *   - CapSense_GetRadialCentroidPos() - calls only if radial slider 
    *     available.
    *   - CapSense_GetTouchCentroidPos() - calls only if touch pad slider 
    *     available.
    *   - CapSense_CheckIsAnyWidgetActive();
    *  The results of opeartions are copied to OUTBOX.
    *   
    * Parameters:
    *  None
    *
    * Return:
    *  None
    *
    * Global Variables:
    *  CapSense_OUTBOX outbox - structure which is used as ouput 
    *  buffer for report data to Tuner GUI.
    *  Update fields:
    *    - position[];
    *    - OnMask[];
    *
    * Reentrant:
    *  No
    *
    *******************************************************************************/
    void CapSense_ProcessAllWidgets(volatile CapSense_OUTBOX *outbox)
	                                        
    {
        uint8 i = 0u;

        #if (CapSense_TOTAL_TOUCH_PADS_COUNT)
            uint16 pos[2];
        #endif  /* (CapSense_TOTAL_TOUCH_PADS_COUNT) */
        
        #if ( (CapSense_TOTAL_RADIAL_SLIDERS_COUNT) || (CapSense_TOTAL_TOUCH_PADS_COUNT) || \
              (CapSense_TOTAL_MATRIX_BUTTONS_COUNT) )
            uint8 widgetCnt = 0u;
        #endif  /* ((CapSense_TOTAL_RADIAL_SLIDERS_COUNT) || (CapSense_TOTAL_TOUCH_PADS_COUNT)) || 
                *   (CapSense_TOTAL_MATRIX_BUTTONS_COUNT)
                */
        
        /* Calculate widget with centroids */
        #if (CapSense_TOTAL_LINEAR_SLIDERS_COUNT)
            for(; i < CapSense_TOTAL_LINEAR_SLIDERS_COUNT; i++)
            {
                outbox->position[i] = CapSense_SWAP_ENDIAN16(CapSense_GetCentroidPos(i));
            }
        #endif /* (CapSense_TOTAL_LINEAR_SLIDERS_COUNT) */
        
        #if (CapSense_TOTAL_RADIAL_SLIDERS_COUNT)
            widgetCnt = i;
            for(; i < widgetCnt + CapSense_TOTAL_RADIAL_SLIDERS_COUNT; i++)
            {
                outbox->position[i] = CapSense_SWAP_ENDIAN16(CapSense_GetRadialCentroidPos(i));
            }
        #endif /* (CapSense_TOTAL_RADIAL_SLIDERS_COUNT) */
        
        #if (CapSense_TOTAL_TOUCH_PADS_COUNT)
            widgetCnt = i;
            for(; i < (widgetCnt + (CapSense_TOTAL_TOUCH_PADS_COUNT * 2)); i=i+2)
            {
                if(CapSense_GetTouchCentroidPos(i, pos) == 0u)
                {
                    outbox->position[i] = 0xFFFFu;
                    outbox->position[i+1] = 0xFFFFu;
                }
                else
                {
                    outbox->position[i] = CapSense_SWAP_ENDIAN16( (uint16) pos[0u]);
                    outbox->position[i+1] = CapSense_SWAP_ENDIAN16( (uint16) pos[1u]);
                }
            }
        #endif /* (CapSense_TOTAL_TOUCH_PADS_COUNT) */

        #if (CapSense_TOTAL_MATRIX_BUTTONS_COUNT)
            i += CapSense_TOTAL_BUTTONS_COUNT;
            widgetCnt = 0;
            for(; widgetCnt < (CapSense_TOTAL_MATRIX_BUTTONS_COUNT * 2); i += 2)
            {
                if(CapSense_GetMatrixButtonPos(i, ((uint8*) &outbox->mb_position[widgetCnt])) == 0)
                {
                    outbox->mb_position[widgetCnt] = 0xFFu;
                    outbox->mb_position[widgetCnt+1] = 0xFF;
                }
                widgetCnt += 2;
            }
        #endif /* (CapSense_TOTAL_MATRIX_BUTTONS_COUNT) */

        /* Update On/Off State */
        CapSense_CheckIsAnyWidgetActive();

        /* Copy OnMask */
        for(i=0; i < CapSense_TOTAL_SENSOR_MASK_COUNT; i++)
        {
            outbox->onMask[i]  = CapSense_SensorOnMask[i];
        }
    }
#endif /* (CapSense_TUNER_API_GENERATE) */


/* [] END OF FILE */
