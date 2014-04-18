/*******************************************************************************
* File Name: SPDIF_INT.c
* Version 1.20
*
* Description:
*  This file provides all Interrupt Service Routine (ISR) for the S/PDIF
*  Transmitter component.
*
* Note:
*  None.
*
********************************************************************************
* Copyright 2011-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "SPDIF_PVT.h"


#if(0u != SPDIF_MANAGED_DMA)
    /***************************************************************************
    * Function Name: SPDIF_Cst0Copy
    ****************************************************************************
    *
    * Summary:
    *  Interrupt Service Routine to implement double buffered DMA for Channel 0
    *  Status.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global variables:
    *  SPDIF_cst0BufOffset - used as offset for buffer management.
    *  SPDIF_cstStream0[SPDIF_CST_LENGTH] - destination
    *  array for Channel 0 status. Used as source data for status DMA.
    *  SPDIF_wrkCstStream0[SPDIF_CST_LENGTH] - source
    *  array for Channel 0 status. Used to allow Channel 0 status changing at
    *  run time.
    *
    ***************************************************************************/
    CY_ISR(SPDIF_Cst0Copy)
    {
        uint8 offset;
        
        offset = SPDIF_cst0BufOffset;
        
        (void) memcpy((void *) &SPDIF_cstStream0[offset],
                      (void *) &SPDIF_wrkCstStream0[offset],
                      SPDIF_CST_HALF_LENGTH);

        SPDIF_cst0BufOffset ^= SPDIF_CST_HALF_LENGTH;
    }


    /***************************************************************************
    * Function Name: SPDIF_Cst1Copy
    ****************************************************************************
    *
    * Summary:
    *  Interrupt Service Routine to implement double buffered DMA for Channel 1
    *  Status.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global variables:
    *  SPDIF_cst1BufOffset - used as offset for buffer management.
    *  SPDIF_cstStream1[SPDIF_CST_LENGTH] - destination
    *  array or Channel 1 status. Used as source data for status DMA.
    *  SPDIF_wrkCstStream1[SPDIF_CST_LENGTH] - source
    *  array for Channel 1 status. Used to allow Channel 1 status changing at run
    *  time.
    *
    ***************************************************************************/
    CY_ISR(SPDIF_Cst1Copy)
    {
        uint8 offset;
        
        offset = SPDIF_cst1BufOffset;
        
        (void) memcpy((void *) &SPDIF_cstStream1[offset],
                      (void *) &SPDIF_wrkCstStream1[offset],
                      SPDIF_CST_HALF_LENGTH);

        SPDIF_cst1BufOffset ^= SPDIF_CST_HALF_LENGTH;
    }

#endif  /* (0u != SPDIF_MANAGED_DMA) */


/* [] END OF FILE */
