/*******************************************************************************
* File Name: AudioControl.c
*
* Version 4.0
*
*  Description: This file contains the Audio signal path configuration and 
*               processing code
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
#include <Codec.h>
#include <Configuration.h>
#include <Device.h>
#include <Interrupts.h>
#include <LCD.h>
#include <project.h>
#include <USBInterface.h>
#include <VolumeControl.h>

extern volatile uint8 USBFS_frequencyChanged;   /* USB audio class sampling frequency change flag */
extern volatile uint8 USBFS_currentSampleFrequency[][USBFS_SAMPLE_FREQ_LEN]; /* audio SR received from USB host */
extern volatile uint8 USBFS_interfaceSetting[]; /* current USB active interface settings */
extern volatile uint8 USBFS_transferState;      /* USB component state machine value */
extern volatile uint8 USBFS_device;             /* currently active USB device descriptor */

extern CYBIT audioClkConfigured;                 /* AudioClkGen configuration flag */
extern uint8 currentLCDVolume;                   /* volume display flag */
extern uint8 currentLCDMute;                     /* mute display flag */
extern CYDATA CodecRegister CodecWrite;    		 /* codec register read/write structure */

/* For iOS devices (i.e. USBiAP_device = 0 or 2), MIDI active flag is set when the Apple device polls the MIDI IN
 * endpoint (Endpoint 7 in this project). Windows XP doesn't poll the MIDI IN endpoint when just MIDI Out is enabled.
 * To support Windows XP (and similar OSs), MIDI active flag is set forever */
extern uint8 usbMidiActive;
extern CYDATA uint8 auxConfigured;               /* Aux audio stream mode flag */
extern CYPDATA uint8 systemAudioSource;          /* Active audio source in the system */
extern CYPDATA uint8 audioSource;                /* Apple device audio type (analog or digital) */

#ifdef AUX_DETECTION_ENABLE
extern uint16 auxTriggerafterSampleRateChangeTimer;
extern CYBIT inAuxMode;
#endif 
#if(USBFS_EP_MM == USBFS__EP_DMAAUTO) 
  #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
  extern uint8 outRam[OUT_AUDIOMAXPKT];          /* USB Auto DMA mode audio OUT EP buffer */
  #endif
  #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
  extern uint8 inRam[IN_AUDIOMAXPKT];            /* USB Auto DMA mode audio IN EP buffer */
  extern uint16 inCnt;                           /* USB audio IN EP actual count (varies based on audio sample rate */
  #endif
#endif

uint8 USBOutDmaChan;            /* DMA Channel: USB EP memory to OUT circular SRAM  */
CYPDATA uint8 txDmaChan;        /* DMA Channel: OUT circular buffer to ByteSwap Tx DMA */
CYPDATA uint8 I2STxDMAChan;     /* DMA Channel: ByteSwap Tx to I2S Tx DMA */
uint8 USBInDmaChan;             /* USB IN circular SRAM memory to IN EP DMA */
CYPDATA uint8 rxDmaChan;        /* ByteSwap Rx -> IN Circular buffer DMA */
CYPDATA uint8 I2SRxMDAChan;     /* I2S Rx -> ByteSwap Rx DMA */

CYPDATA uint8 txTd[NUM_TDS];    /* OUT circular buffer to ByteSwap Tx TDs */
CYPDATA uint8 rxTd[NUM_TDS];    /* ByteSwap Rx to circular IN buffer TDs */
uint8 I2STxTd;                  /* ByteSwap Tx to I2S Tx TD */
uint8 I2SRxTd;                  /* I2S Rx to ByteSwap Rx TD */
uint8 USBOutTd[2];              /* USB EP memory to OUT circular SRAM TDs */
uint8 USBInTd[2];               /* USB IN circular SRAM memory to IN EP TDs */

#ifdef SPDIF_ENABLED
extern uint8 SPDIF_initVar;
/* For the SPDIF Transmit */
uint8 SPDIFTxDmaChan;           /* SPDIF DMA channel */
uint8 SPDIFTxTd;                /* SPDIF DMA TD */
#endif

CYBIT  resetTx = 0;             /* audio OUT stream reset flag */
CYBIT  outPlaying = 0;          /* audio OUT stream (playback) active flag */
uint16 outLevel = 0;            /* audio OUT buffer level pointer */
uint16 outUsbCount = 0;         /* audio OUT USB interface count */
uint16 outUsbShadow = 0;        /* audio OUT temp count */
uint8  outBuffer[OUT_BUFSIZE];  /* audio OUT circular buffer */
uint16 outBufIndex = 0;         /* audio OUT USB DMA index */

#ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
CYBIT  resetRx = 0;             /* audio IN stream reset flag */
CYBIT  inPlaying = 0;           /* audio IN stream (recording) active flag */
uint16 inLevel = IN_BUFSIZE;    /* audio IN buffer level pointer */
uint16 inUsbCount = 0;          /* audio IN USB interface count */
uint16 inUsbShadow = 0;         /* audio IN temp count */
CYBIT  clearInBuffer = 0;       /* audio IN buffer reset flag */
uint8 inBuffer[IN_BUFSIZE];     /* audio IN circular buffer */
uint16 inBufIndex = 0;          /* audio IN USB DMA index */
#endif

CYDATA uint8 auxConfigured = 0;     /* Aux audio source active flag */
CYBIT lowPowerIdle = TRUE;          /* audio low power mode flag (low power = XTAL, Analog audio, PLL etc. shut down) */
CYDATA uint8 midiPowerIdle = TRUE;  /* MIDI interface active flag. PLL and XTAL will be ON if MIDI interface is active */
CYBIT auxInStatus = 0;              /* Aux audio selection button status */
CYPDATA uint8 systemAudioSource = AUDIO_SOURCE_DIGITAL;  /* accessories currently configured audio mode (Analog/Digital) */
CYPDATA uint8 setRate = FREQUENCY_NOT_SET; /* Audio sample rate configured or not in AudioClkGen component */
CYBIT rateChangedWhileInactive = FALSE;    /* Sample rate changed while the system was in low power mode flag */
CYPDATA uint8 clockSwitchTimer = 0;        /* Timer for audio clock shutdown when the streaming interface is inactive */
CYBIT updateLCD = FALSE;                   /* Update the LCD with currently configured sampling rate value */
CYBIT audioClkConfigured = FALSE;          /* AudioClkGen is configured with one or the other sample rate */
CYPDATA uint8 audioSource = AUDIO_SOURCE_DIGITAL; /* iAP transport layer indicator */
char freqStr[] = "    ";        /* LCD sample rate display string */
CYPDATA uint8 rate;             /* macro which holds the sample rate issues by the USB host */

/*******************************************************************************
* Function Name: ConfigureAudioPath
********************************************************************************
* Summary:
*       This function sets up the XTAL, DMA and starts iAP, USB, I2S and interrupts
*       to get the PSoC 3 device configured for audio streaming mode
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ConfigureAudioPath(void)
{
    uint8 index;
	
	CyPins_SetPin(PSOC_CODEC_PWR_0); 
        
    CyMasterClk_SetSource(CY_MASTER_SOURCE_IMO); /* Change system clock to IMO */
	CyDelayFreq(DELAY_FREQ_PARAM_VALUE_IMO);
    CyDelayUs(100);    
    CyXTAL_Stop();
    CyPLL_OUT_Stop();
        
    CY_SET_REG8((void CYXDATA *)(CYDEV_FASTCLK_PLL_CFG1), 0); /* Change the settings for the PLL filtering */

    AudioClkGen_Start();
    rate = RATE_44KHZ;
    CyPLL_OUT_SetSource(CY_PLL_SOURCE_DSI);
    
	/* Starting the MCLK always as required by some codecs */
	#ifdef MCLK_ALWAYS_ENABLED
	CyXTAL_Start(0);
	CyPLL_OUT_Start(0);
	AudioClkGen_SetAudioRate(RATE_48KHZ);
	#ifdef I2S_PIN_DRIVE_MODE_CHANGE_IN_IDLE_MODE
	    PSOC_I2S_MCLK_SetDriveMode(PSOC_I2S_MCLK_DM_STRONG);		
    #endif
	#endif 

    /* Audio sample SRAM to ByteSwap and vice versa DMA configuration*/
    /* __________________________________SRAM to ByteSwap DMA Config Start____________________________________________*/
    
    #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
    /* Set up the TX DMA, 1 byte bursts, each burst requires a request, upper addr of 0 */
    txDmaChan = TxDMA_DmaInitialize(DMA_BURSTCOUNT, RQST_PER_BURST, HI16(CYDEV_SRAM_BASE), HI16(CYDEV_PERIPH_BASE));
    #endif
    
    #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
    /* Rx DMA Config, 1 byte bursts, each burst requires a request, upper addr of 0 */
    rxDmaChan = RxDMA_DmaInitialize(DMA_BURSTCOUNT, RQST_PER_BURST, HI16(CYDEV_PERIPH_BASE), HI16(CYDEV_SRAM_BASE));
    #endif
    
    for (index=0; index < NUM_TDS; index++)
    {
        /* Request for a set of TDs from the pool */
        #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
        txTd[index] = CyDmaTdAllocate();
        #endif
        
        #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
        rxTd[index] = CyDmaTdAllocate();
        #endif
    }    
        
    for (index=0; index < NUM_TDS; index++)
    {
        #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
        /* Configure this Td chain as follows:
         * Loop through all of the TDs , Increment the source address, but not the destination address */
        CyDmaTdSetConfiguration(txTd[index], OUT_TRANS_SIZE, txTd[(index+1)%NUM_TDS], TD_INC_SRC_ADR | TxDMA__TD_TERMOUT_EN);
        #endif
        
        #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
        /* Configure this Td chain as follows:
         *  Loop through all of the TDs, Increment the destination address, but not the source address */
        CyDmaTdSetConfiguration(rxTd[index], IN_TRANS_SIZE, rxTd[(index+1)%NUM_TDS], TD_INC_DST_ADR | RxDMA__TD_TERMOUT_EN);
        #endif
        
        #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
        /* From the Audio Out SRAM buffer to ByteSwap component input FIFO */
        CyDmaTdSetAddress(txTd[index], (uint16)(outBuffer+(OUT_TRANS_SIZE*index)), (uint16)ByteSwap_Tx_dp_ByteSwap_u0__F0_REG);
        #endif
        
        #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
        /* From the ByteSwap component output FIFO to Audio In SRAM buffer */
        CyDmaTdSetAddress(rxTd[index], (uint16)ByteSwap_Rx_dp_ByteSwap_u0__F1_REG, (uint16)(inBuffer+(IN_TRANS_SIZE*index)));
        #endif
    }
    /* __________________________________SRAM to ByteSwap DMA Config End_____________________________________________ */
    
    
    /* __________________________________SRAM to SPDIF TX FIFO DMA Config Start_______________________________________*/
    #ifdef SPDIF_ENABLED
    SPDIFTxDmaChan = SPDIF_Tx_DMA_DmaInitialize(DMA_BURSTCOUNT, RQST_PER_BURST, HI16(CYDEV_SRAM_BASE), HI16(CYDEV_PERIPH_BASE));
    SPDIFTxTd = CyDmaTdAllocate();
    CyDmaTdSetConfiguration(SPDIFTxTd, OUT_BUFSIZE, SPDIFTxTd, TD_INC_SRC_ADR);
    CyDmaTdSetAddress(SPDIFTxTd, (uint16)(&outBuffer[0]), (uint16)SPDIF_TX_FIFO_0_PTR);
    #endif
    /* __________________________________SRAM to SPDIF TX FIFO DMA Config End_________________________________________*/
    
    
    #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
    /* __________________________________ByteSwap to I2S DMA Config Start_____________________________________________*/
    I2STxDMAChan = I2S_Tx_DMA_DmaInitialize(DMA_BURSTCOUNT, RQST_PER_BURST, HI16(CYDEV_PERIPH_BASE), HI16(CYDEV_PERIPH_BASE));
    I2STxTd = CyDmaTdAllocate();
    CyDmaTdSetConfiguration(I2STxTd, I2SDMA_TRANS_SIZE, I2STxTd, I2SDMA_CONFIG);
    CyDmaTdSetAddress(I2STxTd, (uint16)(ByteSwap_Tx_dp_ByteSwap_u0__F1_REG), (uint16)I2S_TX_FIFO_0_PTR );
    CyDmaChSetInitialTd(I2STxDMAChan, I2STxTd);
    /* __________________________________ByteSwap to I2S DMA Config End_______________________________________________*/
    #endif
    
    #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
    /* __________________________________I2S Rx ByteSwap to SRAM DMA Config Start_____________________________________*/
    I2SRxMDAChan = I2S_Rx_DMA_DmaInitialize(DMA_BURSTCOUNT, RQST_PER_BURST, HI16(CYDEV_PERIPH_BASE), HI16(CYDEV_PERIPH_BASE));
    I2SRxTd = CyDmaTdAllocate();
    CyDmaTdSetConfiguration(I2SRxTd, I2SDMA_TRANS_SIZE, I2SRxTd, I2SDMA_CONFIG);
    CyDmaTdSetAddress(I2SRxTd, (uint16)(I2S_RX_FIFO_0_PTR), (uint16)ByteSwap_Rx_dp_ByteSwap_u0__F0_REG );
    CyDmaChSetInitialTd(I2SRxMDAChan, I2SRxTd);
    /* __________________________________I2S Rx ByteSwap to SRAM DMA Config End_______________________________________*/
    #endif
	
    #ifdef AUX_DETECTION_ENABLE
	AuxDetection_Initialization();
	#endif
	
    #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
        /* USB Out DMA Config:  Entire chain is sent based on a single request from the CPU Td's are configured later */
        USBOutDmaChan = USBOutDMA_DmaInitialize(DMA_BURSTCOUNT, USBDMA_RQST_PER_BURST, HI16(CYDEV_SRAM_BASE), HI16(CYDEV_SRAM_BASE));  
        USBOutTd[0] = CyDmaTdAllocate();
        USBOutTd[1] = CyDmaTdAllocate();
    #endif
    
    #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
    /* USB In DMA Config: Entire chain is sent based on a single request from the CPU Td's are configured later    */
    USBInDmaChan = USBInDMA_DmaInitialize(DMA_BURSTCOUNT, USBDMA_RQST_PER_BURST, HI16(CYDEV_SRAM_BASE), HI16(CYDEV_SRAM_BASE));
    USBInTd[0] = CyDmaTdAllocate();
    USBInTd[1] = CyDmaTdAllocate();
    #endif
    
    #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
    isr_TxDMADone_StartEx(TxDMADone_Interrupt);
    #endif
    
    #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
    isr_RxDMADone_StartEx(RxDMADone_Interrupt);
    isr_InDMADone_StartEx(InDMADone_Interrupt);
    #endif
    
    #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
    /* set TX FIFO trigger to 2 bytes (half-empty) to increase timing margin */
    I2S_TX_AUX_CONTROL_REG = I2S_TX_AUX_CONTROL_REG | FIFO_HALF_EMPTY_MASK;
    #endif
    
    #ifdef ENABLE_VOLUME_CONTROL
    InitUSBVolumeLevel();
    #endif

    auxConfigured = FALSE;
    
    /* USER_CODE: [Audio Initialization] Placeholder for user code to initialize additional audio path components or 
     * external audio peripheral configuration */
}

/*******************************************************************************
* Function Name: ProcessAudioOut
********************************************************************************
* Summary:
*        Handle audio out data, setup USB DMA and trigger the DMA to transfer 
*        audio samples from SRAM USB endpoint memory to SRAM audio circular 
*        buffer. The API also starts the I2S transmit when USB audio out streaming 
*        is active
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
#ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
void ProcessAudioOut(void) 
{
	uint16 count;
    uint16 remain;
    
    if( TRUE == audioClkConfigured)
    {
        if (resetTx)         /* Check TX buffer reset condition and clear pointers if required */
        {
            outBufIndex = 0;
            outUsbCount = 0;
            resetTx = 0;
        }
        
        CyDmaChDisable(USBOutDmaChan);    /* Disable the USBOut DMA channel */
        
        count = USBFS_GetEPCount(AUDIO_OUT_ENDPOINT);
    
        /* Update of usbCount needs to be atomic */
        isr_TxDMADone_Disable();
        outUsbCount += count;
        isr_TxDMADone_Enable();
        
        if((outBufIndex + count) > sizeof(outBuffer))
        {
            remain = sizeof(outBuffer) - outBufIndex;  /* Set up TD to wrap around circular buffer */
            
            CyDmaTdSetConfiguration(USBOutTd[0], remain, USBOutTd[1], TD_INC_SRC_ADR | TD_INC_DST_ADR | TD_AUTO_EXEC_NEXT);
            CyDmaTdSetConfiguration(USBOutTd[1], count-remain, CY_DMA_DISABLE_TD, TD_INC_SRC_ADR | TD_INC_DST_ADR );
            CyDmaTdSetAddress(USBOutTd[0], (uint16) outRam, (uint16)(outBuffer+outBufIndex));
            CyDmaTdSetAddress(USBOutTd[1], (uint16) outRam + remain, (uint16)(outBuffer));

            CyDmaChSetInitialTd(USBOutDmaChan, USBOutTd[0]);
            outBufIndex = count-remain;
        }
        else 
        {
            /* Single contiguous TD */
            CyDmaTdSetConfiguration(USBOutTd[0], count, CY_DMA_DISABLE_TD, TD_INC_SRC_ADR | TD_INC_DST_ADR );
            CyDmaTdSetAddress(USBOutTd[0], (uint16) outRam, (uint16)(outBuffer+outBufIndex));
            CyDmaChSetInitialTd(USBOutDmaChan, USBOutTd[0]);
            outBufIndex += count;
            if (outBufIndex == sizeof(outBuffer)) outBufIndex = 0;
        }
        
        CyDmaChEnable(USBOutDmaChan, 1);           /* Enable the USB Out DMA, don't update the Td as it progresses */
        
        CyDmaChSetRequest(USBOutDmaChan, CPU_REQ); /* Start the DMA */
    
        /* Start playing audio only when transmit buffer is more than half full */
        if(!outPlaying && outUsbCount >= OUT_HALF)
        {
            outPlaying = TRUE;
            
            I2S_ClearTxFIFO(); /* Clear the I2S internal FIFO */
            
            /* clear any potential DMA requests and re-reset TD pointer */
            while((DMAC_CH[txDmaChan].basic_status[0] & 2));
    
            CyDmaChSetRequest(txDmaChan, CPU_TERM_CHAIN);
            CyDmaChEnable(txDmaChan, 1);
    
            while((DMAC_CH[txDmaChan].basic_cfg[0] & 1));

            /* Enable the Tx DMA, initialized to start of buffer */
            CyDmaChSetInitialTd(txDmaChan, txTd[0]);
            CyDmaChEnable(txDmaChan, 1);
            
            ByteSwap_Tx_Start();
            
            #ifdef SPDIF_ENABLED
            SPDIF_ClearTxFIFO(); /* Clear the SPDIF internal FIFO */
            
            /* clear any potential DMA requests and re-reset TD pointer for SPDIF*/
            while((DMAC_CH[SPDIFTxDmaChan].basic_status[0] & 2));
            CyDmaChSetRequest(SPDIFTxDmaChan, CPU_TERM_CHAIN);
            CyDmaChEnable(SPDIFTxDmaChan, 1);
            while((DMAC_CH[SPDIFTxDmaChan].basic_cfg[0] & 1));

            /* Enable the Tx DMA, initialized to start of buffer */
            CyDmaChSetInitialTd(SPDIFTxDmaChan, SPDIFTxTd);
            CyDmaChEnable(SPDIFTxDmaChan, 1);
            #endif
            
            CyDmaChEnable(I2STxDMAChan, 1);   /* enable byte swap to I2S DMA channel */

            I2S_EnableTx();                   /* Unmute the Tx output */
            
            #ifdef SPDIF_ENABLED
            SPDIF_EnableTx();                 /* Unmute the SPDIF Tx output */
            #endif
        }
        
        /* USER_CODE: [USB audio OUT endpoint ISR] Placeholder for user code to know when an USB audio OUT packet is 
         * received from the USB host. This routine is called from an ISR, do not add time consuming tasks inside
         * this routine so that other interrupts in the system can be serviced in a timely manner */
    }
}
#endif

/*******************************************************************************
* Function Name: ProcessAudioIn
********************************************************************************
* Summary:
*        Handle USB audio in data, setup USB DMA and trigger to transfer samples 
*        from SRAM audio circular buffer to SRAM USB endpoint memory.  Start I2S 
*		 receive when USB is active.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
#ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
void ProcessAudioIn(void)
{   
	uint16 remain;
    uint16 count;
    static uint8 rate44_count = 0;
    
    if( audioClkConfigured ==  TRUE)
    {
        if (resetRx)
        {
            inBufIndex = 0;
            inUsbCount = 0;
            resetRx = 0;
            rate44_count = 0;    
        }
        
        CyDmaChDisable(USBInDmaChan);
        
        switch(rate)
        {
            case RATE_48KHZ:                    
                    count = IN_AUDIOMAXPKT;        /* Count is number of bytes consumed from the buffer */
                break;
            case RATE_44KHZ:
                    if(9 == rate44_count) 
                    {                            
                        count = IN_AUDIOMAXPKT - (IN_AUDIOMAXPKT/16);  /* adjust the count to cater for 44.1kHz SR */
                        rate44_count = 0;
                    }
                    else 
                    {                            
                        count = IN_AUDIOMAXPKT - (IN_AUDIOMAXPKT/12); /* adjust the count to cater for 44.1kHz SR */
                        rate44_count++;
                    }
                break;
            case RATE_32KHZ:                
                    count = IN_AUDIOMAXPKT - (IN_AUDIOMAXPKT/3);        
                break;
            default:
                break;
        }
        
        inCnt = count;
         
        /* Update of inUsbCount needs to be atomic */
        isr_RxDMADone_Disable();
        inUsbCount += count;
        isr_RxDMADone_Enable();
                
        if ((inBufIndex + count) > sizeof(inBuffer)) 
        {
            /* Set up TD to wrap around circular buffer */
            remain = sizeof(inBuffer) - inBufIndex;

            CyDmaTdSetConfiguration(USBInTd[0], remain, USBInTd[1], TD_INC_SRC_ADR | TD_INC_DST_ADR | TD_AUTO_EXEC_NEXT);
            CyDmaTdSetConfiguration(USBInTd[1], count-remain, CY_DMA_DISABLE_TD, TD_INC_SRC_ADR | TD_INC_DST_ADR | USBInDMA__TD_TERMOUT_EN);
            CyDmaTdSetAddress(USBInTd[0], (uint16)(inBuffer+inBufIndex), (uint16) inRam);
            CyDmaTdSetAddress(USBInTd[1], (uint16)(inBuffer), (uint16) inRam + remain);
             
            CyDmaChSetInitialTd(USBInDmaChan, USBInTd[0]);
            inBufIndex = count-remain;
        }
        else 
        {
            /* Single contiguous TD */
            CyDmaTdSetConfiguration(USBInTd[0], count, CY_DMA_DISABLE_TD, TD_INC_SRC_ADR | TD_INC_DST_ADR | USBInDMA__TD_TERMOUT_EN);
            CyDmaTdSetAddress(USBInTd[0], (uint16)(inBuffer+inBufIndex), (uint16) inRam);
            
            CyDmaChSetInitialTd(USBInDmaChan, USBInTd[0]);
            inBufIndex += count;
            if (inBufIndex == sizeof(inBuffer)) 
			{
				inBufIndex = 0;
			}	
        }
    
        /* Enable the USB In DMA, don't update the Td as it progresses */
        CyDmaChEnable(USBInDmaChan, 1);
        CyDmaChSetRequest(USBInDmaChan, CPU_REQ); /* Start the DMA now */

        /* Sending of the data on the USB interface is enabled when the interrupt indicates that the buffer 
         * has been filled. */
        if (!inPlaying && inUsbCount >= IN_HALF) 
        {    
            inPlaying = 1;
            /* Clear the I2S internal FIFO */
            I2S_ClearRxFIFO();        
            
            ByteSwap_Rx_Start();
            
            /* enable byte swap to I2S DMA channel */
            CyDmaChEnable(I2SRxMDAChan, 1);
            
            /*Clear Any potential pending DMA requests before starting the DMA channel to transfer data */
            CyDmaChSetRequest(rxDmaChan, CPU_TERM_CHAIN);
            CyDmaChEnable(rxDmaChan, 1);
            
            /* Enable the Rx DMA, initialized back to the first TD */
            CyDmaChSetInitialTd(rxDmaChan, rxTd[0]);
            CyDmaChEnable(rxDmaChan, 1);
            
            I2S_EnableRx();  /* Unmute the Rx output */
        }
        
        /* USER_CODE: [USB audio IN endpoint ISR] Placeholder for user code to know when an USB audio IN packet is 
         * sent to the USB host. This routine is called from an ISR, do not add time consuming tasks inside this
         * routine so that other interrupts in the system can be serviced in a timely manner */
    }
}
#endif

/*******************************************************************************
* Function Name: Stop_I2S_Tx
********************************************************************************
* Summary:
*        This function stops the I2S data transmission by disabling the I2S and 
*        transmit DMA.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void Stop_I2S_Tx(void) CYREENTRANT
{
    #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
    if(outPlaying)
    {       
        I2S_DisableTx(); /* Stop I2S Transmit (Mute), I2S output clocks still active */
        
        CyDelayUs(20); /* Provide enough time for DMA to transfer the last audio samples completely to I2S TX FIFO */
        
        ByteSwap_Tx_Stop(); /* Terminate ByteSwap logic and hence DMA request line to TxDMA (SRAM to ByteSwap DMA) */

        #ifdef SPDIF_ENABLED
        SPDIF_DisableTx();
        #endif
    
        CyDmaChDisable(txDmaChan); /* Stop/Disable DMA - Needed to reset to start of chain */
        CyDmaChDisable(I2STxDMAChan); /* Disable ByteSwap to I2S data transfer DMA channel */
        
        #ifdef SPDIF_ENABLED
        CyDmaChDisable(SPDIFTxDmaChan);
        #endif
                
        resetTx = 1;
        outLevel = 0;
        outUsbShadow = 0;
        outPlaying = 0;
        
        /* USER_CODE: [USB audio playback stops] Placeholder for user code to shutdown any of the audio OUT path 
         * components. This routine is called when USB audio OUT endpoint stops streaming audio */
    }
    #endif
}

/*******************************************************************************
* Function Name: Stop_I2S_Rx
********************************************************************************
* Summary:
*        This function stops the I2S data reception by disabling the I2S and 
*        receive DMA.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
#ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
void Stop_I2S_Rx(void) CYREENTRANT
{
    if(inPlaying)
    {
        I2S_DisableRx();    /* Stop I2S Receive (Mute), I2S output clocks still active */
        
        /* Terminate TD chain & Stop/Disable DMA - Needed to reset to start of chain */
        CyDmaChSetRequest(rxDmaChan, CPU_TERM_CHAIN);
                
        resetRx = 1;
        
        clearInBuffer = 1;
        
        inLevel = IN_BUFSIZE;
        inUsbShadow = 0;    
        inPlaying = 0;
        
        /* USER_CODE: [USB audio recording stops] Placeholder for user code to shutdown any of the audio IN path 
         * components. This routine is called when USB audio IN endpoint stops requesting audio IN samples */
    }
}
#endif

/*******************************************************************************
* Function Name: SetClockRate
********************************************************************************
* Summary:
*        This function changes the audio clocking to generate clocks for a desired 
*        sample rate.
*
* Parameters:
*  newRate: audio sample rate from list in AudioClkGen component which is to be set
*
* Return:
*  void
*
*******************************************************************************/
void SetClockRate(uint8 newRate) CYREENTRANT
{

    CyPins_SetPin(PSOC_CODEC_PWR_0); 
        
    /* Stop I2S before changing PLL clock */
    #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
	Stop_I2S_Tx();
    #endif 
	
    #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
    Stop_I2S_Rx();
    #endif
    
    CyMasterClk_SetSource(CY_MASTER_SOURCE_IMO);
	CyDelayFreq(DELAY_FREQ_PARAM_VALUE_IMO);
    rate = newRate;

    AudioClkGen_SetAudioRate(newRate);
	
	/* Displaying sampling frequency on LCD */
	switch(newRate)
    {
        case AudioClkGen_RATE_32KHZ:
            strcpy(freqStr,"32.0");
        break;
        
        case AudioClkGen_RATE_48KHZ:
            strcpy(freqStr,"48.0");
        break;
        
        case AudioClkGen_RATE_44KHZ:
            strcpy(freqStr,"44.1");
        break;
    }
	
    CyMasterClk_SetSource(CY_MASTER_SOURCE_PLL);
    CyDelayFreq(DELAY_FREQ_PARAM_VALUE_PLL);
	  
    /* Switch IMO clock tree (including USB) to use XTAL, disable iAP out endpoint during clock switching */
    
    CyIMO_SetSource(CY_IMO_SOURCE_XTAL);
    
    
    #ifdef SPDIF_ENABLED
    /* Stop the SPDIF component to change the frequency in the headers */
    SPDIF_Stop();
    
    if(RATE_48KHZ == rate)
        SPDIF_SetFrequency(SPDIF_SPS_48KHZ);
    else if(RATE_44KHZ == rate)
        SPDIF_SetFrequency(SPDIF_SPS_44KHZ);
    else if(RATE_32KHZ == rate)
        SPDIF_SetFrequency(SPDIF_SPS_32KHZ);
        
    SPDIF_Start();
    #endif

    /* update LCD display with the new sampling frequency value */
    updateLCD = TRUE;
    
    /* flag to indicate audio clock active */
    audioClkConfigured = TRUE;
    
    /* CODEC is initialized when sampling frequency change request is received from the USB host (the sampling frequency
     * change event triggers the call to this routine), if the host is fast and requests for audio In samples immediately
     * after sampling frequency change request, then the CODEC interface wouldn't be ready for approximately 20ms
     * (20ms is specific to CS42L51) and the initial few milliseconds of audio IN data will be zeroes. The CODEC can be 
     * initialized at the start of the program in the case of self powered designs, but for Apple device powered case,
     * it is not possible to meet Apple accessory power policy idle current if CODEC is initialized at the start of the
     * program */
    InitCirrusCodec();
    
	/* USER_CODE:[CODEC] If different codec is used than onboard Cirrus codec (CS42L51), then
	 * Comment out the functions InitCirrusCodec().
	 * Add your code for codec initialization in place of InitCirrusCodec(). 
	 * Update the UpdateCodecVolume() and UpdateCodecAttenuation() functions as required by new codec. 
	 * Update the codec.h file with the I2C address of codec and the register addresses of the new codec. */
	
	
	#ifdef AUX_DETECTION_ENABLE
	auxTriggerafterSampleRateChangeTimer = CORRUPT_AUX_DATA_AFTER_RATE_CHANGE_TIMER;
	#endif
	
	/* USER_CODE: [Active mode begins]Add your code for initializing any external peripherals which requires the PSoC 3 
     * to be in active mode (where all the audio components and clocks are turned on). The system is operating at its 
     * maximum operating frequency at this time. This routine initializes the codec registers with proper configuration
     * when the system is in active mode for the first time. Refer to "Active Mode Clock 44.1/48KHz schematic pages
     * for system clock details. Firmware is in "Active Mode" at this point in code until asked to enter low power mode
     * by changing audio streaming interface or stopping MIDI polling. The sampling frequency to which the current 
     * system clock should be configured depends on the parameter to this API (newRate)*/
}
    
/*******************************************************************************
* Function Name:  ConfigureAuxDMA
********************************************************************************
* Summary:
*       This function configures the audio path for AUX mode.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ConfigureAuxDMA(void)
{
    #ifdef ENABLE_AUX_MODE   
    uint8 AuxSamplingRate;
    
    /* Audio clocks must be active for getting aux data over I2S interface */
    SetSystemAudioSource(AUDIO_SOURCE_ANALOG);
    StartAudioComponents();
    StartAnalogAudioComponents();
    AuxSamplingRate = RATE_48KHZ;
    
	#ifdef ENABLE_AUX_MODE
    if(auxInStatus)
	#endif
	#ifdef AUX_DETECTION_ENABLE 
	if(inAuxMode)
	#endif
    {
        #ifdef LCD_MODULE_ENABLED
        LCD2LineDisplay("Analog Loopback ",
                        "                ");
        
        #ifdef ENABLE_VOLUME_CONTROL
        currentLCDVolume--; /* Hack to update the volume and mute status display on LCD when switched to aux mode */
        currentLCDMute--;
        #endif
        #endif
    }
 
    /* System is configured in AUX or Analog audio mode now, select the appropriate digital line for I2S SDTO line */
    I2S_Out_Select_Write(I2S_IN_TO_I2S_OUT);
    
    #ifdef LCD_MODULE_ENABLED
    LCD_Position(1,0);
    LCD_PrintString("     ");
    LCD_Position(1,12);
    LCD_PrintString("48.0");
    #endif
      
    auxConfigured = TRUE;
        
    /* If Aux input is enabled, then configure the DMA accordingly */    
    #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
    /* Disable the RX DMA interrupt, which will otherwise overflow because there is no USB IN request */
    isr_RxDMADone_Disable(); 
    inPlaying = 1;
    Stop_I2S_Rx();
    I2S_ClearRxFIFO();
    ByteSwap_Rx_Stop();
    #endif
    
    #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
    /* Disable the TX DMA interrupt, which will otherwise overflow because there is no USB OUT request */
    isr_TxDMADone_Disable();
    outPlaying = 1;
    Stop_I2S_Tx();
    I2S_ClearTxFIFO();
    ByteSwap_Tx_Stop();
    #endif 
    
    SetClockRate(AuxSamplingRate); /* when in aux mode, audio sample rate is fixed at 48kHz */
    setRate = FREQUENCY_NOT_SET; /* clear the current audio sampling rate set value */
            
    CyPins_SetPin(PSOC_CODEC_RST_0); /* Turn on CODEC by releasing reset */
    
    outPlaying =1;  /* Enable playback through aux analog loopback or iPod analog output */
    #endif
}

/*******************************************************************************
* Function Name: ConfigureDigitalAudioDMA
********************************************************************************
* Summary:
*       This function resets the digital audio path from USB which was earlier
*		configured for AUX mode.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ConfigureDigitalAudioDMA(void)
{
    /* reset initialization to re-enumerate USB */
    USBDeviceState = USB_INIT_AFTER_ENUMERATION_COMPLETED;        
       
    /* System is configured in Digital audio mode now, select the appropriate digital line for I2S SDTO line */
    I2S_Out_Select_Write(I2S_OUT_TO_I2S_OUT);
    
	auxConfigured = FALSE;
    SetSystemAudioSource(AUDIO_SOURCE_DIGITAL);
}

/*******************************************************************************
* Function Name: StartAudioComponents
********************************************************************************
* Summary:
*       This function starts components for the digital audio signal path
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void StartAudioComponents(void)
{
	
	#ifndef MCLK_ALWAYS_ENABLED
    CyXTAL_Start(0);
    CyDelay(3);
    #endif
       
    I2S_Start();
    
    #ifdef SPDIF_ENABLED
    if((SPDIF_CONTROL_REG & SPDIF_ENBL) == 0)
    {
        SPDIF_Start();
    }
    #endif
    
    /* USER_CODE: Add your custom code to enable components which should be active when Audio streaming interface
     * is active or MIDI interface is being polled. XTAL is up and running at this point in code, but PLL is still
     * shutdown until SetClockRate API is called on receiving a sampling frequency set event from the USB host */
}

/*******************************************************************************
* Function Name: StopAudioComponents
********************************************************************************
* Summary:
*       This function stops components for the digital audio signal path,
*        including the crystal and PLL.  This puts them in low-power mode.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void StopAudioComponents(void)
{
	if(codecInit)
    {
        /* Mute the DAC output before changing the clocks */
        CodecWrite.address = CODEC_DAC_OUTPUT_CTRL_REGISTER;
        CodecWrite.value = 0x73;
        WriteToSlave(CODEC_I2C_ADDRESS, &CodecWrite.address, sizeof(CodecWrite));
        
        /* shutdown the power to the codec */
        CodecWrite.address = CODEC_POWER_CTRL_REGISTER;
        CodecWrite.value = 0x01; /* Power down the codec */
        WriteToSlave(CODEC_I2C_ADDRESS, &CodecWrite.address, sizeof(CodecWrite));
        
        CyDelay(5);
        CyPins_ClearPin(PSOC_CODEC_RST_0); /* Hold CODEC in reset */
		codecInit = FALSE;
	}

	/* Switch IMO clock tree (including USB) to use internal IMO, disable iAP out endpoint during clock switching */

    
    CyIMO_SetSource(CY_IMO_SOURCE_IMO);
	CyDelayFreq(DELAY_FREQ_PARAM_VALUE_IMO);
	

    /* Change system clock to IMO and then turn off PLL and XTAL */
    CyMasterClk_SetSource(CY_MASTER_SOURCE_IMO);
	CyDelayFreq(DELAY_FREQ_PARAM_VALUE_IMO);
    CyDelayUs(100);
    
	#ifndef MCLK_ALWAYS_ENABLED
    CyPLL_OUT_Stop(); 
    CyXTAL_Stop();
	#endif 
    
    #ifdef SPDIF_ENABLED
    /* If SPDIF component is running, turn it off */
    if(SPDIF_CONTROL_REG & SPDIF_ENBL)
    {
        SPDIF_Stop();
    }
    #endif
    
    I2S_Stop();
    
    #ifdef I2S_PIN_DRIVE_MODE_CHANGE_IN_IDLE_MODE
		#ifndef MCLK_ALWAYS_ENABLED
        PSOC_I2S_MCLK_SetDriveMode(PSOC_I2S_MCLK_DM_DIG_HIZ);
		#endif 
        PSOC_I2S_SDTO_SetDriveMode(PSOC_I2S_SDTO_DM_DIG_HIZ);
        PSOC_I2S_SCLK_SetDriveMode(PSOC_I2S_SCLK_DM_DIG_HIZ);
        PSOC_I2S_LRCLK_SetDriveMode(PSOC_I2S_LRCLK_DM_DIG_HIZ);
    #endif

	codecInit = FALSE;                 /* Reset CODEC initialization flag */
    audioClkConfigured = FALSE;        /* Audio clock reset flag */
    setRate = FREQUENCY_NOT_SET;       /* clear the current audio sampling rate set value */
    
    /* USER_CODE: Add your custom code to shutdown components or hardware blocks which need not be active when audio
     * or MIDI interface is inactive. All the audio components, DMAs and clocks are shutdown at this point in code
     * and the system will remain in this mode until audio or MIDI interface is made active or StartAudioComponents and
     * SetClockRate APIs are called by user code (See ConfigureAuxDMA routine for example). Refer to "Idle Mode Clock
     * Info" schematic page for more details on system clock in this mode */
}

/*******************************************************************************
* Function Name: StartAnalogAudioComponents
********************************************************************************
* Summary:
*       This function starts components for the analog audio signal path, 
*        including the opamps and PGA
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void StartAnalogAudioComponents(void)
{
    /* start analog audio bias and codec drive circuit */
    Opamp_R_Start();
    Opamp_L_Start();
    PGA_Vref_Start();
}

/*******************************************************************************
* Function Name: StopAnalogAudioComponents
********************************************************************************
* Summary:
*       This function stops components for the analog audio signal path, 
*        including the opamps and PGA.  This puts them in low-power mode.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void StopAnalogAudioComponents(void)
{
    /* shutdown analog audio bias and codec drive circuit */
    Opamp_R_Stop();
    Opamp_L_Stop();
    PGA_Vref_Stop();
}

/*******************************************************************************
* Function Name: HandleSamplingFrequencyChangeRequest
********************************************************************************
* Summary:
*       This function processes the sampling frequency change request from USB
*       host and updates the accessory playback sampling frequency accordingly
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void HandleSamplingFrequencyChangeRequest(void)
{
    /* USBiAP_frequencyChanged is set by the USB component when a setup token for Sampling frequency change is received
     * It takes a sometime for the host to actually send the Out token containing the updated sampling frequency.
     * Wait for USBiAP_transferState to be equal to USBiAP_TRANS_STATE_IDLE to make sure the updated sampling frequency 
     * is used for setting audio clocks */
     
    /* Handle USB Audio class sampling frequency change requests only when Aux audio is not active (to prevent pop noise)*/
    if(
	   #ifdef ENABLE_AUX_MODE
	   !auxInStatus &&
	   #endif
	   #ifdef AUX_DETECTION_ENABLE
	   !inAuxMode &&
	   #endif
	   USBFS_TRANS_STATE_IDLE == USBFS_transferState && USBFS_frequencyChanged)
    {
        #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
        #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
        if((!inPlaying && AUDIO_OUT_ENDPOINT == USBFS_frequencyChanged) || 
            (!outPlaying && AUDIO_IN_ENDPOINT == USBFS_frequencyChanged ))
        #endif            
        #endif 
        {
            uint32 newFrequency;
            uint8 newRate;
            
            newFrequency = (((uint32)USBFS_currentSampleFrequency[USBFS_frequencyChanged][2] << 16) |
                              ((uint32)USBFS_currentSampleFrequency[USBFS_frequencyChanged][1] << 8) |
                              ((uint32)USBFS_currentSampleFrequency[USBFS_frequencyChanged][0]));

			USBFS_frequencyChanged = 0;
			
            if(SAMPLING_RATE_48KHZ == newFrequency)
            {
                newRate = RATE_48KHZ;
            }
            else if(SAMPLING_RATE_44KHZ == newFrequency)
            {
                newRate = RATE_44KHZ;
            }
            else if(SAMPLING_RATE_32KHZ == newFrequency)
            {
                newRate = RATE_32KHZ;
            }
                
            if(setRate != newRate)
            {   
                setRate = newRate;
                
                if(lowPowerIdle)
                {
                    rateChangedWhileInactive = TRUE;
                }
                else
                {
                    SetClockRate(setRate);
                    rateChangedWhileInactive = FALSE;              
                }
            }
            
            clockSwitchTimer = FALSE;
        }
        #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
        #ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
        else
        {
            /* If another frequency change request is received when one of the audio stream is still active, clear the
             * frequency change request */
            USBFS_frequencyChanged = 0;
        }
        #endif
        #endif
    }
    
    #ifdef LCD_MODULE_ENABLED
    if (GetSystemAudioSource() == AUDIO_SOURCE_DIGITAL && updateLCD)
    {
        LCD_Position(1,12);
        LCD_PrintString(freqStr);
        updateLCD = FALSE;
    }
    #endif
}

/*******************************************************************************
* Function Name: HandleDigitalAudioLowPowerMode
********************************************************************************
* Summary:
*       This function switches between low and high power modes for digital audio 
*       depending on whether the audio stream from Apple device/PC/Mac is active 
*       or not.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void HandleDigitalAudioLowPowerMode(void)
{
    /* Handle STOP/START of audio stream */
    if(!auxConfigured
        && (FALSE == lowPowerIdle || FALSE == midiPowerIdle)
	  )
    {                        
        if(FALSE == lowPowerIdle && IS_USB_AUDIO_INTERFACE_NOT_ACTIVE())
        {
            if(0 == clockSwitchTimer)
            {
                clockSwitchTimer = CLOCK_SWITCHING_TIMEOUT;
            }
            /* Both Audio Interfaces are inactive for more than 500ms -> enter low power idle mode */
            if(CLOCK_SWITCH_TIMED_OUT == clockSwitchTimer)
            {
                lowPowerIdle = TRUE;
                    #ifdef MIDI_ENABLED
                    if(TRUE == midiPowerIdle)
                    {
                        StopAudioComponents();            /* Changes to 24 MHz IMO for USB */
                    }
                    setRate = FREQUENCY_NOT_SET;
                    #else
                        StopAudioComponents();            /* Changes to 24 MHz IMO for USB */
                    #endif
                StopAnalogAudioComponents();       /* Turn OFF Analog path for Audio-In/Apple device Analog */
                CyPins_ClearPin(PSOC_CODEC_RST_0); /* Hold CODEC in reset */
                codecInit = FALSE;
                
                    #ifdef SPDIF_ENABLED
                    CyPins_SetPin(PSOC_PERIPH_PWR_0); /* Turn off power to SPDIF Tx block */
                    #endif
            }
        }
        #ifdef MIDI_ENABLED
        if(FALSE == midiPowerIdle && IsUSBMidiActive() == FALSE)
        {
            if(TRUE == lowPowerIdle)
            {
                StopAudioComponents();        /* Changes to 24 MHz IMO for USB */
            }
            CyPins_SetPin(PSOC_MIDI_PWR_0);   /* Turn off the MIDI I/O hardware */
            midiPowerIdle = TRUE;             /* MIDI low power mode enabled */
        }
        #endif
        
                 
        /* USER_CODE: [High -> Low power transition] Placeholder for shutting down external hardware blocks and 
         * components before going into low power mode */
    }
    
    /* Start the Audio path components only in the following cases:
     1) Audio streaming interface is active and the system is in Audio low power mode 
     2) When MIDI stream is active and the system is in MIDI low power mode 
     3) iAP EA session is active and the system is in EA idle mode */
    if(TRUE == lowPowerIdle || TRUE == midiPowerIdle)
    {
        /* Audio streaming is active on atleast one interface */
        if(TRUE == lowPowerIdle && !(IS_USB_AUDIO_INTERFACE_NOT_ACTIVE()) )
        {
            lowPowerIdle = FALSE;
            
            #ifdef MIDI_ENABLED
            if(TRUE == midiPowerIdle)
            {
                /* USB Audio Started -> go higher power operating mode only if MIDI interface is inactive (otherwise,
                 * the audio components are already turned on)*/                    
                StartAudioComponents();        /* Turn on XTAL/PLL */
            }
            #else
            StartAudioComponents();            /* Turn on XTAL/PLL */
            #endif
            
			StartAnalogAudioComponents();      /* Turn on Analog path for Audio-In/Apple device Analog */
            CyPins_SetPin(PSOC_CODEC_RST_0);   /* Turn on CODEC by releasing reset */
            
            #ifdef SPDIF_ENABLED
            CyPins_ClearPin(PSOC_PERIPH_PWR_0); /* Turn on power to SPDIF Tx */
            #endif
            
            if(rateChangedWhileInactive)
            {
                SetClockRate(setRate);
                rateChangedWhileInactive = FALSE;
            }
        }
        #ifdef MIDI_ENABLED
        if(TRUE == midiPowerIdle && IsUSBMidiActive() == TRUE)
        {
            if(TRUE == lowPowerIdle)
            {
                StartAudioComponents();  /* Turn on XTAL/PLL */
                /* Set the clock rate only if aux is not configured (In aux mode the audio clocks are already active) */
                #ifdef ENABLE_AUX_MODE
				if(!auxInStatus)
				#endif
				#ifdef AUX_DETECTION_ENABLE
				if(!inAuxMode)
				#endif
                {
                    SetClockRate(RATE_48KHZ);   /* Call this routine to increase PSoC 3 master clock for MIDI */
                }
            }
            CyPins_ClearPin(PSOC_MIDI_PWR_0);   /* Turn On the MIDI I/O hardware */
            midiPowerIdle = FALSE;              /* This is MIDI active power mode, codec is not turned on */
        }
        #endif
		
        /* USER_CODE: [Low -> High power transition] Placeholder for turning on external hardware blocks and 
         * components before transitioning into high power mode */
    }
}

/*******************************************************************************
* Function Name: UpdateAudioStatusUI
********************************************************************************
* Summary:
*       Updates the Audio playback status on the LEDs
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void UpdateAudioStatusUI(void)
{
    /* show digital Audio Out and Audio In status on LEDs */
    #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
    inPlaying ? CyPins_SetPin(PSOC_RECORDING_LED_0): CyPins_ClearPin(PSOC_RECORDING_LED_0);
    #endif
    
	#ifndef ENABLE_DIGITAL_AUDIO_IN_ONLY
    outPlaying ? CyPins_SetPin(PSOC_PLAYBACK_LED_0): CyPins_ClearPin(PSOC_PLAYBACK_LED_0);
	#endif 
	
    #ifdef ENABLE_AUX_MODE
	auxInStatus ? CyPins_SetPin(PSOC_AUX_LED_0):     CyPins_ClearPin(PSOC_AUX_LED_0);
    #endif
	#ifdef AUX_DETECTION_ENABLE
	inAuxMode ? CyPins_SetPin(PSOC_AUX_LED_0):       CyPins_ClearPin(PSOC_AUX_LED_0);                  
	#endif
    
	if(outPlaying  
        #ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
        || inPlaying
        #endif
      )
    {
        if(!CyPins_ReadPin(PSOC_CODEC_RST_0))
        {
            CyPins_SetPin(PSOC_CODEC_RST_0);   /* Turn on CODEC by releasing reset */
        }
    }    
}

/*******************************************************************************
* Function Name: HandleAudioInBuffer
********************************************************************************
* Summary:
*       This routine clears the audio IN stream circular buffer and the audio
*       IN endpoint memory location based on the status of the audio IN stream.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
#ifndef ENABLE_DIGITAL_AUDIO_OUT_ONLY
void HandleAudioInBuffer(void)
{
    uint16 index;
    
    if(clearInBuffer)
    {
        /* Clear the IN circular buffer - This is slow and hence part of the main loop */
        for (index = 0; index < sizeof(inBuffer); index++)
        {
            if(index<IN_AUDIOMAXPKT)
            {
                inRam[index] = 0;
            }
            inBuffer[index] = 0;
        }
        clearInBuffer = 0;
    }
}
#endif

/* [] END OF FILE */
