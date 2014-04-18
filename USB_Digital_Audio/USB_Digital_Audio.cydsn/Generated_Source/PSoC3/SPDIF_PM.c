/*******************************************************************************
* File Name: SPDIF_PM.c
* Version 1.20
*
* Description:
*  This file contains the setup, control and status commands to support
*  component operations in low power mode.
*
* Note:
*
********************************************************************************
* Copyright 2011-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "SPDIF.h"

#if(CY_UDB_V0)
    static SPDIF_BACKUP_STRUCT SPDIF_backup =
    {
        /* By default the interrupt will be generated only for errors */
        SPDIF_DEFAULT_INT_SRC
    };
#endif /* (CY_UDB_V0) */


/*******************************************************************************
* Function Name: SPDIF_SaveConfig
********************************************************************************
*
* Summary:
*  Saves SPDIF_Tx configuration.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  SPDIF_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPDIF_SaveConfig(void) 
{
    #if(CY_UDB_V0)
        SPDIF_backup.interruptMask = SPDIF_STATUS_MASK_REG;
    #endif /* (CY_UDB_V0) */
}


/*******************************************************************************
* Function Name: SPDIF_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores SPDIF_Tx configuration.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  SPDIF_backup - used when non-retention registers are restored.
*
*******************************************************************************/
void SPDIF_RestoreConfig(void) 
{
    #if(CY_UDB_V0)
        uint8 enableInterrupts;

        enableInterrupts = CyEnterCriticalSection();
        /* Set FIFOs in the Single Buffer Mode */
        SPDIF_FCNT_AUX_CTL_REG    |= SPDIF_FX_CLEAR;
        SPDIF_PREGEN_AUX_CTL_REG  |= SPDIF_FX_CLEAR;
        CyExitCriticalSection(enableInterrupts);

    #endif /* (CY_UDB_V0) */

    /* Restore Frame and Block Intervals */
    /* Preamble and Post Data Period */
    SPDIF_FCNT_PRE_POST_REG = SPDIF_PRE_POST_PERIOD;
    /* Number of frames in block */
    SPDIF_FCNT_BLOCK_PERIOD_REG = SPDIF_BLOCK_PERIOD;

    #if(CY_UDB_V0)
        /* Frame Period */
        SPDIF_FRAME_PERIOD_REG = SPDIF_FRAME_PERIOD;
        /* Audio Sample Word Length */
        SPDIF_FCNT_AUDIO_LENGTH_REG = SPDIF_AUDIO_DATA_PERIOD;
    #endif /* (CY_UDB_V0) */

    /* Restore Preamble Patterns */
    #if(CY_UDB_V0)
        SPDIF_PREGEN_PREX_PTRN_REG = SPDIF_PREAMBLE_X_PATTERN;
        SPDIF_PREGEN_PREY_PTRN_REG = SPDIF_PREAMBLE_Y_PATTERN;
    #endif /* (CY_UDB_V0) */
    SPDIF_PREGEN_PREZ_PTRN_REG = SPDIF_PREAMBLE_Z_PATTERN;

    /* Restore Interrupt Mask */
    #if(CY_UDB_V0)
        SPDIF_STATUS_MASK_REG = SPDIF_backup.interruptMask;
    #endif /* (CY_UDB_V0) */
}


/*******************************************************************************
* Function Name: SPDIF_Sleep
********************************************************************************
*
* Summary:
*  Prepares SPDIF_Tx goes to sleep.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  SPDIF_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPDIF_Sleep(void) 
{
    /* Stop component */
    SPDIF_Stop();

    /* Save registers configuration */
    SPDIF_SaveConfig();
}


/*******************************************************************************
* Function Name: SPDIF_Wakeup
********************************************************************************
*
* Summary:
*  Prepares SPDIF_Tx to wake up.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  SPDIF_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPDIF_Wakeup(void)  
{
    /* Restore registers values */
    SPDIF_RestoreConfig();
}


/* [] END OF FILE */
