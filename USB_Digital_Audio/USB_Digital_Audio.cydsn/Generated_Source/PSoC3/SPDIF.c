/*******************************************************************************
* File Name: SPDIF.c
* Version 1.20
*
* Description:
*  This file contains the setup, control and status commands for the S/PDIF TX
*  component.
*
* Note:
*
*******************************************************************************
* Copyright 2011-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "SPDIF_PVT.h"

uint8 SPDIF_initVar = 0u;

#if(0u != SPDIF_MANAGED_DMA)

    /* Channel status streams used for DMA transfer */
    volatile uint8 SPDIF_cstStream0[SPDIF_CST_LENGTH];
    volatile uint8 SPDIF_cstStream1[SPDIF_CST_LENGTH];

    /* Channel status streams to change from API at run time */
    volatile uint8 SPDIF_wrkCstStream0[SPDIF_CST_LENGTH];
    volatile uint8 SPDIF_wrkCstStream1[SPDIF_CST_LENGTH];

    /* Buffer offset variables */
    volatile uint8 SPDIF_cst0BufOffset = 0u;
    volatile uint8 SPDIF_cst1BufOffset = 0u;

    /* Cst DMA channels and transfer descriptors */
    static uint8 SPDIF_cst0Chan;
    static uint8 SPDIF_cst1Chan;

    static uint8 SPDIF_cst0Td[2u] = {CY_DMA_INVALID_TD, CY_DMA_INVALID_TD};
    static uint8 SPDIF_cst1Td[2u] = {CY_DMA_INVALID_TD, CY_DMA_INVALID_TD};

    /* Function prototype to set/release DMA */
    static void SPDIF_CstDmaInit(void)       ;
    static void SPDIF_CstDmaRelease(void)    ;

#endif /* 0u != SPDIF_MANAGED_DMA */


/*******************************************************************************
* Function Name: SPDIF_Enable
********************************************************************************
*
* Summary:
*  Enables S/PDIF interface. Starts the generation of the S/PDIF output with
*  channel status, but the audio data is set to all 0's. This allows the S/PDIF
*  receiver to lock on to the component's clock.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPDIF_Enable(void) 
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    SPDIF_BCNT_AUX_CTL_REG   |= SPDIF_BCNT_EN;    /* Bit counter enabling */
    SPDIF_STATUS_AUX_CTL_REG |= SPDIF_INT_EN;     /* Interrupt generation enabling */
    CyExitCriticalSection(enableInterrupts);

    #if(0u != SPDIF_MANAGED_DMA)
        /* Enable channel status ISRs */
        CyIntEnable(SPDIF_CST_0_ISR_NUMBER);
        CyIntEnable(SPDIF_CST_1_ISR_NUMBER);

        /* Prepare and enable channel status DMA transfer */
        SPDIF_CstDmaInit();

        while(0u != (SPDIF_STATUS_REG & SPDIF_CHST_FIFOS_NOT_FULL))
        {
            ; /* Wait for DMA fills status FIFOs to proceed */
        }
    #endif /* 0u != SPDIF_MANAGED_DMA */

    SPDIF_CONTROL_REG |= SPDIF_ENBL;
}


#if (0u != SPDIF_MANAGED_DMA)
    /*******************************************************************************
    * Function Name: SPDIF_CstDmaInit
    ********************************************************************************
    *
    * Summary:
    *  Inits channel status DMA transfer.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  SPDIF_cst0Chan - DMA Channel to be used for Channel 0 Status
    *     DMA transfer.
    *  SPDIF_cst1Chan - DMA Channel to be used for Channel 1 Status
    *     DMA transfer.
    *  SPDIF_cst0Td[] - TD set to be used for Channel 0 Status DMA
    *     transfer.
    *  SPDIF_cst1Td[] - TD set to be used for Channel 1 Status DMA
    *     transfer.
    *  SPDIF_cstStream0[] - Channel 0 Status stream. Used as the source
    *     buffer for Channel 0 Status DMA. Modified when the data is copied for the
    *     first cycle.
    *  SPDIF_wrkCstStream0[] - Channel 0 Status intermediate buffer
    *     between API and DMA. This is required to allow changing of Channel Status
    *     at run time. Used when the data is copied for the first cycle.
    *  SPDIF_cstStream1[] - Channel 1 Status stream. Used as the source
    *     buffer for Channel 1 Status DMA. Modified when the data is copied for the
    *     first cycle.
    *  SPDIF_wrkCstStream1[] - Channel 1 Status intermediate buffer
    *     between API and DMA. This is required to allow changing of Channel Status
    *     at run time. Used when the data is copied for the first cycle.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    static void SPDIF_CstDmaInit(void) 
    {

        /* Copy channels' status values for the first cycle */
        (void) memcpy((void *) SPDIF_cstStream0,
                      (void *) SPDIF_wrkCstStream0, SPDIF_CST_LENGTH);

        (void) memcpy((void *) SPDIF_cstStream1,
                      (void *) SPDIF_wrkCstStream1, SPDIF_CST_LENGTH);

        SPDIF_cst0Td[0u] = CyDmaTdAllocate();
        SPDIF_cst0Td[1u] = CyDmaTdAllocate();

        SPDIF_cst1Td[0u] = CyDmaTdAllocate();
        SPDIF_cst1Td[1u] = CyDmaTdAllocate();

        (void) CyDmaTdSetConfiguration(
                SPDIF_cst0Td[0u],
                SPDIF_CST_HALF_LENGTH,
                SPDIF_cst0Td[1u],
                (CY_DMA_TD_INC_SRC_ADR | SPDIF_Cst0_DMA__TD_TERMOUT_EN));

        (void) CyDmaTdSetConfiguration(
                SPDIF_cst0Td[1u],
                SPDIF_CST_HALF_LENGTH,
                SPDIF_cst0Td[0u],
                (CY_DMA_TD_INC_SRC_ADR | SPDIF_Cst0_DMA__TD_TERMOUT_EN));

        (void) CyDmaTdSetConfiguration(
                SPDIF_cst1Td[0u],
                SPDIF_CST_HALF_LENGTH,
                SPDIF_cst1Td[1u],
                (CY_DMA_TD_INC_SRC_ADR | SPDIF_Cst1_DMA__TD_TERMOUT_EN));

        (void) CyDmaTdSetConfiguration(
                SPDIF_cst1Td[1u],
                SPDIF_CST_HALF_LENGTH,
                SPDIF_cst1Td[0u],
                (CY_DMA_TD_INC_SRC_ADR | SPDIF_Cst1_DMA__TD_TERMOUT_EN));

        (void) CyDmaTdSetAddress(
                SPDIF_cst0Td[0u],
                LO16((uint32)SPDIF_cstStream0),
                LO16((uint32)SPDIF_CST_FIFO_0_PTR));

        (void) CyDmaTdSetAddress(
                SPDIF_cst0Td[1u],
                LO16((uint32) (&SPDIF_cstStream0[SPDIF_CST_HALF_LENGTH])),
                LO16((uint32) SPDIF_CST_FIFO_0_PTR));

        (void) CyDmaTdSetAddress(
                SPDIF_cst1Td[0u],
                LO16((uint32) SPDIF_cstStream1),
                LO16((uint32) SPDIF_CST_FIFO_1_PTR));

        (void) CyDmaTdSetAddress(
                SPDIF_cst1Td[1u],
                LO16((uint32) (&SPDIF_cstStream1[SPDIF_CST_HALF_LENGTH])),
                LO16((uint32) SPDIF_CST_FIFO_1_PTR));

        (void) CyDmaChSetInitialTd(SPDIF_cst0Chan, SPDIF_cst0Td[0u]);
        (void) CyDmaChSetInitialTd(SPDIF_cst1Chan, SPDIF_cst1Td[0u]);

        (void) CyDmaChEnable(SPDIF_cst0Chan, 1u);
        (void) CyDmaChEnable(SPDIF_cst1Chan, 1u);
    }


    /*******************************************************************************
    * Function Name: SPDIF_CstDmaRelease
    ********************************************************************************
    *
    * Summary:
    *  Release allocated DMA channels and transfer descriptors.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  SPDIF_cst0Chan - DMA Channel to be used for Channel 0 Status
    *     DMA transfer.
    *  SPDIF_cst1Chan - DMA Channel to be used for Channel 1 Status
    *     DMA transfer.
    *  SPDIF_cst0Td[] - TD set to be used for Channel 0 Status DMA
    *     transfer.
    *  SPDIF_cst1Td[] - TD set to be used for Channel 1 Status DMA
    *     transfer.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    static void SPDIF_CstDmaRelease(void) 
    {
        /* Disable the managed channel status DMA */
        (void) CyDmaChDisable(SPDIF_cst0Chan);
        (void) CyDmaChDisable(SPDIF_cst1Chan);

        /* Clear any potential DMA requests and re-reset TD pointers */
        while(0u != (CY_DMA_CH_STRUCT_PTR[SPDIF_cst0Chan].basic_status[0] & CY_DMA_STATUS_TD_ACTIVE))
        {
            ; /* Wait for to be cleared */
        }

        (void) CyDmaChSetRequest(SPDIF_cst0Chan, CY_DMA_CPU_TERM_CHAIN);
        (void) CyDmaChEnable    (SPDIF_cst0Chan, 1u);

        while(0u != (CY_DMA_CH_STRUCT_PTR[SPDIF_cst0Chan].basic_cfg[0] & CY_DMA_STATUS_CHAIN_ACTIVE))
        {
            ; /* Wait for to be cleared */
        }


        while(0u != (CY_DMA_CH_STRUCT_PTR[SPDIF_cst1Chan].basic_status[0] & CY_DMA_STATUS_TD_ACTIVE))
        {
            ; /* Wait for to be cleared */
        }

        (void) CyDmaChSetRequest(SPDIF_cst1Chan, CY_DMA_CPU_TERM_CHAIN);
        (void) CyDmaChEnable    (SPDIF_cst1Chan, 1u);

        while(0u != (CY_DMA_CH_STRUCT_PTR[SPDIF_cst1Chan].basic_cfg[0] & CY_DMA_STATUS_CHAIN_ACTIVE))
        {
            ; /* Wait for to be cleared */
        }

        /* Release all allocated TDs and mark them as invalid */
        CyDmaTdFree(SPDIF_cst0Td[0u]);
        CyDmaTdFree(SPDIF_cst0Td[1u]);
        CyDmaTdFree(SPDIF_cst1Td[0u]);
        CyDmaTdFree(SPDIF_cst1Td[1u]);
        SPDIF_cst0Td[0u] = CY_DMA_INVALID_TD;
        SPDIF_cst0Td[1u] = CY_DMA_INVALID_TD;
        SPDIF_cst1Td[0u] = CY_DMA_INVALID_TD;
        SPDIF_cst1Td[1u] = CY_DMA_INVALID_TD;
    }
#endif /* 0u != SPDIF_MANAGED_DMA */


/*******************************************************************************
* Function Name: SPDIF_Init
********************************************************************************
*
* Summary:
*  Initializes the customizer settings for the component including channel
*  status.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  SPDIF_wrkCstStream0[] - Channel 0 Status internal buffer. Modified
*  when default S/PDIF configuration provided with customizer is initialized or
*  restored.
*  SPDIF_wrkCstStream1[] - Channel 1 Status internal buffer. Modified
*  when default S/PDIF configuration provided with customizer is initialized or
*  restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPDIF_Init(void) 
{
    #if(0u != SPDIF_MANAGED_DMA)
        /* Channel status set by user in the customizer. Used to initialize the
        *  settings in SPDIF_Init() API.
        */
        static const uint8 CYCODE SPDIF_initCstStream0[SPDIF_CST_LENGTH] = {
            0x00,0x00,0x00,0x00,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        };
        static const uint8 CYCODE SPDIF_initCstStream1[SPDIF_CST_LENGTH] = {
            0x00u, 0x00u, 0x11u, 0x00u, 0x0Bu, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u
        };
    #endif /* (0u != SPDIF_MANAGED_DMA) */
    
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    /* Set FIFOs in the Single Buffer Mode */
    SPDIF_FCNT_AUX_CTL_REG    |= SPDIF_FX_CLEAR;
    SPDIF_PREGEN_AUX_CTL_REG  |= SPDIF_FX_CLEAR;
    CyExitCriticalSection(enableInterrupts);

    /* Channel status ISR initialization  */
    #if(0u != SPDIF_MANAGED_DMA)
        CyIntDisable(SPDIF_CST_0_ISR_NUMBER);
        CyIntDisable(SPDIF_CST_1_ISR_NUMBER);

        /* Set the ISR to point to the Interrupt processing routines */
        (void) CyIntSetVector(SPDIF_CST_0_ISR_NUMBER, &SPDIF_Cst0Copy);
        (void) CyIntSetVector(SPDIF_CST_1_ISR_NUMBER, &SPDIF_Cst1Copy);

        /* Set the priority */
        CyIntSetPriority(SPDIF_CST_0_ISR_NUMBER, SPDIF_CST_0_ISR_PRIORITY);
        CyIntSetPriority(SPDIF_CST_1_ISR_NUMBER, SPDIF_CST_1_ISR_PRIORITY);
    #endif /* (0u != SPDIF_MANAGED_DMA) */

    /* Setup Frame and Block Intervals */
    /* Frame Period */
    SPDIF_FRAME_PERIOD_REG = SPDIF_FRAME_PERIOD;
    /* Preamble and Post Data Period */
    SPDIF_FCNT_PRE_POST_REG = SPDIF_PRE_POST_PERIOD;
    /* Audio Sample Word Length */
    SPDIF_FCNT_AUDIO_LENGTH_REG = SPDIF_AUDIO_DATA_PERIOD;
    /* Number of frames in block */
    SPDIF_FCNT_BLOCK_PERIOD_REG = SPDIF_BLOCK_PERIOD;

    /* Set Preamble Patterns */
    SPDIF_PREGEN_PREX_PTRN_REG = SPDIF_PREAMBLE_X_PATTERN;
    SPDIF_PREGEN_PREY_PTRN_REG = SPDIF_PREAMBLE_Y_PATTERN;
    SPDIF_PREGEN_PREZ_PTRN_REG = SPDIF_PREAMBLE_Z_PATTERN;

    /* Set Interrupt Mask. By default interrupt generation is allowed only for
    *  error conditions, including audio or channel status FIFOs underflow.
    */
    SPDIF_STATUS_MASK_REG = SPDIF_DEFAULT_INT_SRC;

    /* Channel Status DMA Config */
    #if(0u != SPDIF_MANAGED_DMA)
        /* Init channel status streams */
        (void) memcpy((void *) SPDIF_wrkCstStream0,
                      (void *) SPDIF_initCstStream0, SPDIF_CST_LENGTH);

        (void) memcpy((void *) SPDIF_wrkCstStream1,
                      (void *) SPDIF_initCstStream1, SPDIF_CST_LENGTH);

        /* Init DMA, 1 byte bursts, each burst requires a request */
        SPDIF_cst0Chan = SPDIF_Cst0_DMA_DmaInitialize(
            SPDIF_CST_DMA_BYTES_PER_BURST, SPDIF_CST_DMA_REQUEST_PER_BURST,
            HI16(SPDIF_CST_DMA_SRC_BASE), HI16(SPDIF_CST_DMA_DST_BASE));

        SPDIF_cst1Chan = SPDIF_Cst1_DMA_DmaInitialize(
            SPDIF_CST_DMA_BYTES_PER_BURST, SPDIF_CST_DMA_REQUEST_PER_BURST,
            HI16(SPDIF_CST_DMA_SRC_BASE), HI16(SPDIF_CST_DMA_DST_BASE));
    #endif /* (0u != SPDIF_MANAGED_DMA) */
}


/*******************************************************************************
* Function Name: SPDIF_Start
********************************************************************************
*
* Summary:
*  Starts the S/PDIF interface.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  SPDIF_initVar - used to check initial configuration, modified on
*  first function call.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPDIF_Start(void) 
{
    if(0u == SPDIF_initVar)
    {
        SPDIF_Init();
        SPDIF_initVar = 1u;
    }

    SPDIF_Enable();
}


/*******************************************************************************
* Function Name: SPDIF_Stop
********************************************************************************
*
* Summary:
*  Disables the S/PDIF interface. The audio data and channel data FIFOs are
*  cleared. If the component is configured to manage channel status DMA, then
*  the DMA channels and TDs are released to the system.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPDIF_Stop(void) 
{
    uint8 enableInterrupts;

    /* Disable audio data transmission */
    SPDIF_DisableTx();

    SPDIF_CONTROL_REG &= ((uint8) ~SPDIF_ENBL);

    enableInterrupts = CyEnterCriticalSection();
    SPDIF_STATUS_AUX_CTL_REG &= ((uint8) ~SPDIF_INT_EN);  /* Disable Interrupt generation */
    SPDIF_BCNT_AUX_CTL_REG   &= ((uint8) ~SPDIF_BCNT_EN); /* Disable Bit counter */
    CyExitCriticalSection(enableInterrupts);

    #if (0u != SPDIF_MANAGED_DMA)
        /* Disable channel status ISRs */
        CyIntDisable(SPDIF_CST_0_ISR_NUMBER);
        CyIntDisable(SPDIF_CST_1_ISR_NUMBER);

        CyIntClearPending(SPDIF_CST_0_ISR_NUMBER);
        CyIntClearPending(SPDIF_CST_1_ISR_NUMBER);

        /* Clear the buffer offset variables */
        SPDIF_cst0BufOffset = 0u;
        SPDIF_cst1BufOffset = 0u;
        SPDIF_CstDmaRelease();
    #endif /* 0u != SPDIF_MANAGED_DMA */

    SPDIF_ClearTxFIFO();
    SPDIF_ClearCstFIFO();
}


/*******************************************************************************
* Function Name: SPDIF_EnableTx
********************************************************************************
*
* Summary:
*  Enables the audio data output in the S/PDIF bit stream. Transmission will
*  begin at the next X or Z frame.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void SPDIF_EnableTx(void) 
{
    SPDIF_CONTROL_REG |= SPDIF_TX_EN;
}


/*******************************************************************************
* Function Name: SPDIF_DisableTx
********************************************************************************
*
* Summary:
*  Disables the Tx direction of the the audio output S/PDIF bit stream.
*  Transmission of data will stop at the next rising edge of the clock and a
*  constant 0 value will be transmitted.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void SPDIF_DisableTx(void) 
{
    SPDIF_CONTROL_REG &= ((uint8) ~SPDIF_TX_EN);
}


/*******************************************************************************
* Function Name: SPDIF_SetInterruptMode
********************************************************************************
*
* Summary:
*  Sets the interrupt source for the S/PDIF. Multiple sources may be ORed
*  together.
*
* Parameters:
*  Byte containing the constant for the selected interrupt sources.
*   SPDIF_AUDIO_FIFO_UNDERFLOW
*   SPDIF_AUDIO_0_FIFO_NOT_FULL
*   SPDIF_AUDIO_1_FIFO_NOT_FULL
*   SPDIF_CHST_FIFO_UNDERFLOW
*   SPDIF_CHST_0_FIFO_NOT_FULL
*   SPDIF_CHST_1_FIFO_NOT_FULL
*
* Return:
*  None.
*
*******************************************************************************/
void SPDIF_SetInterruptMode(uint8 interruptSource) 
{
    SPDIF_STATUS_MASK_REG = interruptSource;
}


/*******************************************************************************
* Function Name: SPDIF_ReadStatus
********************************************************************************
*
* Summary:
*  Returns state in the SPDIF status register.
*
* Parameters:
*  None.
*
* Return:
*  State of the SPDIF status register
*   SPDIF_AUDIO_FIFO_UNDERFLOW (Clear on Read)
*   SPDIF_AUDIO_0_FIFO_NOT_FULL
*   SPDIF_AUDIO_1_FIFO_NOT_FULL
*   SPDIF_CHST_FIFO_UNDERFLOW (Clear on Read)
*   SPDIF_CHST_0_FIFO_NOT_FULL
*   SPDIF_CHST_1_FIFO_NOT_FUL
*
* Side Effects:
*  Clears the bits of SPDIF status register that are Clear on Read.
*
*******************************************************************************/
uint8 SPDIF_ReadStatus(void) 
{
    return(SPDIF_STATUS_REG & SPDIF_INT_MASK);
}


/*******************************************************************************
* Function Name: SPDIF_WriteTxByte
********************************************************************************
*
* Summary:
*  Writes a single byte into the specified Audio FIFO.
*
* Parameters:
*  wrData: Byte containing the data to transmit.
*  channelSelect: Byte containing the constant for Channel to write.
*    SPDIF_CHANNEL_0 indicates to write to the Channel 0 and
*    SPDIF_CHANNEL_1 indicates to write to the Channel 1.
*  In the interleaved mode this parameter is ignored.
*
* Return:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPDIF_WriteTxByte(uint8 wrData, uint8 channelSelect) 
{
    #if(0u != SPDIF_DATA_INTERLEAVING)

        if(0u != channelSelect)
        {
            /* Suppress compiler warning */
        }

        SPDIF_TX_FIFO_0_REG = wrData;

    #else

        if(SPDIF_CHANNEL_0 == channelSelect)
        {
            SPDIF_TX_FIFO_0_REG = wrData;
        }
        else
        {
            SPDIF_TX_FIFO_1_REG = wrData;
        }

    #endif /* (0u != SPDIF_DATA_INTERLEAVING) */
}


/*******************************************************************************
* Function Name: SPDIF_ClearTxFIFO
********************************************************************************
*
* Summary:
*  Clears out the Tx FIFO. Any data present in the FIFO will not be sent. This
*  call should be made only when transmit is disabled. In the case of separated
*  audio mode, both audio FIFOs will be cleared.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void SPDIF_ClearTxFIFO(void) 
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    SPDIF_TX_AUX_CTL_REG |= ((uint8)  SPDIF_FX_CLEAR);
    SPDIF_TX_AUX_CTL_REG &= ((uint8) ~SPDIF_FX_CLEAR);
    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: SPDIF_WriteCstByte
********************************************************************************
*
* Summary:
*  Writes a single byte into the specified Channel Status FIFO.
*
* Parameters:
*  wrData: Byte containing the status data to transmit.
*  channelSelect: Byte containing the constant for Channel to write.
*    SPDIF_CHANNEL_0 indicates to write to the Channel 0 and
*    SPDIF_CHANNEL_1 indicates to write to the Channel 1.
*
* Return:
*  None.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void SPDIF_WriteCstByte(uint8 wrData, uint8 channelSelect) 
{
    if(SPDIF_CHANNEL_0 == channelSelect)
    {
        SPDIF_CST_FIFO_0_REG = wrData;
    }
    else
    {
        SPDIF_CST_FIFO_1_REG = wrData;
    }
}


/*******************************************************************************
* Function Name: SPDIF_ClearCstFIFO
********************************************************************************
*
* Summary:
*  Clears out the Channel Status FIFOs. Any data present in either FIFO will not
*  be sent. This call should be made only when the component is stopped.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void SPDIF_ClearCstFIFO(void) 
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    SPDIF_CST_AUX_CTL_REG |= ((uint8)  SPDIF_FX_CLEAR);
    SPDIF_CST_AUX_CTL_REG &= ((uint8) ~SPDIF_FX_CLEAR);
    CyExitCriticalSection(enableInterrupts);
}


#if(0u != SPDIF_MANAGED_DMA)
    /*******************************************************************************
    * Function Name: SPDIF_SetChannelStatus
    ********************************************************************************
    *
    * Summary:
    *  Sets the values of the channel status at run time. This API is only valid
    *  when the component is managing the DMA.
    *
    * Parameters:
    *  channel: Byte containing the constant for Channel to modify.
    *   SPDIF_CHANNEL_0 and SPDIF_CHANNEL_1 are used to
    *   specify Channel 0 and Channel 1 respectively.
    *  byte : Byte to modify. This argument should be in range from 0 to 23.
    *  mask : Mask on the byte.
    *  value: Value to set.
    *
    * Return:
    *  None.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void SPDIF_SetChannelStatus(uint8 channel, uint8 byte, uint8 mask, uint8 value) \
                                                                
    {
        if(SPDIF_CHANNEL_0 == channel)
        {
            /* Update of status stream needs to be atomic */
            CyIntDisable(SPDIF_CST_0_ISR_NUMBER);
            SPDIF_wrkCstStream0[byte] &= ((uint8) ~mask);    /* Clear the applicable bits */
            SPDIF_wrkCstStream0[byte] |= ((uint8) value);    /* Set the applicable bits   */
            CyIntEnable(SPDIF_CST_0_ISR_NUMBER);
        }
        else
        {
            /* Update of status stream needs to be atomic */
            CyIntDisable(SPDIF_CST_1_ISR_NUMBER);
            SPDIF_wrkCstStream1[byte] &= ((uint8) ~mask);    /* Clear the applicable bits */
            SPDIF_wrkCstStream1[byte] |= ((uint8) value);    /* Set the applicable bits   */
            CyIntEnable(SPDIF_CST_1_ISR_NUMBER);
        }
    }


    /*******************************************************************************
    * Function Name: SPDIF_SetFrequency
    ********************************************************************************
    *
    * Summary:
    *  Sets the values of the channel status for a specified frequency and returns
    *  1. This function only works if the component is stopped. If this is called
    *  while the component is started, a zero will be returned and the values will
    *  not be modified. This API is only valid when the component is managing the
    *  DMA.
    *
    * Parameters:
    *  Byte containing the constant for the specified frequency.
    *    SPDIF_SPS_UNKNOWN
    *    SPDIF_SPS_22KHZ
    *    SPDIF_SPS_24KHZ
    *    SPDIF_SPS_32KHZ
    *    SPDIF_SPS_44KHZ
    *    SPDIF_SPS_48KHZ
    *    SPDIF_SPS_64KHZ
    *    SPDIF_SPS_88KHZ
    *    SPDIF_SPS_96KHZ
    *    SPDIF_SPS_192KHZ
    *
    * Return:
    *  1 on success.
    *  0 on failure.
    *
    *******************************************************************************/
    uint8 SPDIF_SetFrequency(uint8 frequency) 
    {
        uint8 result;

        result = ((uint8) SPDIF_IS_DISABLED);

        /* The values of the channel status should not be modified if the component is started */
        if(0u != result)
        {
            /* Refer to sample frequency constansts: SPDIF_SF_freqKHZ  (3u) (0xCFu) (freq) */
            SPDIF_SetChannelStatus(SPDIF_CHANNEL_0, 3u, 0xCFu, frequency);
            SPDIF_SetChannelStatus(SPDIF_CHANNEL_1, 3u, 0xCFu, frequency);
        }

        return(result);
    }
#endif /* (0u != SPDIF_MANAGED_DMA) */


/* [] END OF FILE */
