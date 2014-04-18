/*******************************************************************************
* File Name: CapSense_PM.c
* Version 3.30
*
* Description:
*  This file provides Sleep APIs for CapSense CSD Component.
*
* Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "CapSense.h"

CapSense_BACKUP_STRUCT CapSense_backup =
{   
    0x00u, /* enableState; */
    /* Set ScanSpeed */
    #if (CY_PSOC5A)
        CapSense_SCANSPEED_VALUE,  /* scan speed value */
    #endif  /* (CY_PSOC5A) */
};


/*******************************************************************************
* Function Name: CapSense_SaveConfig
********************************************************************************
*
* Summary:
*  Saves customer configuration of CapSense none-retention registers. Resets 
*  all sensors to an inactive state.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global Variables:
*  CapSense_backup - used to save component state before enter sleep 
*  mode and none-retention registers.
*
* Reentrant:
*  No - for PSoC5 ES1 silicon, Yes - for PSoC3 ES3.
*
*******************************************************************************/
void CapSense_SaveConfig(void) 
{    
    /* Set ScanSpeed */
    #if (CY_PSOC5A)
        CapSense_backup.scanspeed = CapSense_SCANSPEED_PERIOD_REG;
    #endif  /* (CY_PSOC5A) */

    /* Set CONTROL_REG */
    CapSense_backup.ctrlreg = CapSense_CONTROL_REG;

    /* Clear all sensors */
    CapSense_ClearSensors();
    
    /* The pins disable is customer concern: Cmod and Rb */
}


/*******************************************************************************
* Function Name: CapSense_Sleep
********************************************************************************
*
* Summary:
*  Disables Active mode power template bits for number of component used within 
*  CapSense. Calls CapSense_SaveConfig() function to save customer 
*  configuration of CapSense none-retention registers and resets all sensors 
*  to an inactive state.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global Variables:
*  CapSense_backup - used to save component state before enter sleep 
*  mode and none-retention registers.
*
* Reentrant:
*  No
*
*******************************************************************************/
void CapSense_Sleep(void) 
{
    /* Check and save enable state */
    if(CapSense_IS_CAPSENSE_ENABLE(CapSense_CONTROL_REG))
    {
        CapSense_backup.enableState = 1u;
        CapSense_Stop();
    }
    else
    {
        CapSense_backup.enableState = 0u;
    }
    
    CapSense_SaveConfig();
}


/*******************************************************************************
* Function Name: CapSense_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores CapSense configuration and non-retention register values.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  Must be called only after CapSense_SaveConfig() routine. Otherwise 
*  the component configuration will be overwritten with its initial setting.  
*
* Global Variables:
*  CapSense_backup - used to save component state before enter sleep 
*  mode and none-retention registers.
*
*******************************************************************************/
void CapSense_RestoreConfig(void) 
{   
    #if ( ((CapSense_PRS_OPTIONS) || \
           (CapSense_IMPLEMENTATION_CH0 == CapSense_MEASURE_IMPLEMENTATION_UDB) || \
           ( (CapSense_DESIGN_TYPE == CapSense_TWO_CHANNELS_DESIGN) && \
             (CapSense_IMPLEMENTATION_CH1 == CapSense_MEASURE_IMPLEMENTATION_UDB))) && \
          (CY_PSOC5A) )
        
        uint8 enableInterrupts;
    #endif /* ( ((CapSense_PRS_OPTIONS) || \
           *     (CapSense_IMPLEMENTATION_CH0 == CapSense_MEASURE_IMPLEMENTATION_UDB) || \
           *   ( (CapSense_DESIGN_TYPE == CapSense_TWO_CHANNELS_DESIGN) && \
           *     (CapSense_IMPLEMENTATION_CH1 == CapSense_MEASURE_IMPLEMENTATION_UDB))) && \
           *     (CY_PSOC5A) )
           */
    
    /* Set PRS */
    #if (CapSense_PRS_OPTIONS == CapSense_PRS_8BITS)
        #if (CY_PSOC5A)
            /* Aux control set FIFO as REG */ 
            enableInterrupts = CyEnterCriticalSection();
            CapSense_AUX_CONTROL_A_REG |= CapSense_AUXCTRL_FIFO_SINGLE_REG;
            CyExitCriticalSection(enableInterrupts);
            
            /* Write polynomial */
            CapSense_POLYNOM_REG   = CapSense_PRS8_DEFAULT_POLYNOM;             /* D0 register */
        #endif  /* (CY_PSOC5A) */
        
        /* Write FIFO with seed */
        CapSense_SEED_COPY_REG = CapSense_MEASURE_FULL_RANGE_LOW;               /* F0 register */
    
    #elif (CapSense_PRS_OPTIONS == CapSense_PRS_16BITS)
        #if (CY_PSOC5A)
            /* Aux control set FIFO as REG */
            enableInterrupts = CyEnterCriticalSection();
            CapSense_AUX_CONTROL_A_REG |= CapSense_AUXCTRL_FIFO_SINGLE_REG;
            CapSense_AUX_CONTROL_B_REG |= CapSense_AUXCTRL_FIFO_SINGLE_REG;
            CyExitCriticalSection(enableInterrupts);
            
            /* Write polynomial */
            CY_SET_REG16(CapSense_POLYNOM_PTR, CapSense_PRS16_DEFAULT_POLYNOM); /* D0 register */
        #endif  /* (CY_PSOC5A) */
        
        /* Write FIFO with seed */
        CY_SET_REG16(CapSense_SEED_COPY_PTR, CapSense_MEASURE_FULL_RANGE);      /* F0 register */
                
    #elif (CapSense_PRS_OPTIONS == CapSense_PRS_16BITS_4X)
        #if (CY_PSOC5A)
            /* Aux control set FIFO as REG */
            enableInterrupts = CyEnterCriticalSection();
            CapSense_AUX_CONTROL_A_REG  |= CapSense_AUXCTRL_FIFO_SINGLE_REG;
            CyExitCriticalSection(enableInterrupts);
            
            /* Write polynomial */
            CapSense_POLYNOM_A__D1_REG   = HI8(CapSense_PRS16_DEFAULT_POLYNOM); /* D0 register */
            CapSense_POLYNOM_A__D0_REG   = LO8(CapSense_PRS16_DEFAULT_POLYNOM); /* D1 register */
        #endif  /* (CY_PSOC5A) */
        
        /* Write FIFO with seed */
        CapSense_SEED_COPY_A__F1_REG = CapSense_MEASURE_FULL_RANGE_LOW;         /* F0 register */
        CapSense_SEED_COPY_A__F0_REG =CapSense_MEASURE_FULL_RANGE_LOW;          /* F1 register */
        
    #else
        /* Do nothing = config without PRS */
    #endif  /* (CapSense_PRS_OPTIONS == CapSense_PRS_8BITS) */
    
    #if (CY_PSOC5A)
        /* Set ScanSpeed */
        CapSense_SCANSPEED_PERIOD_REG = CapSense_backup.scanspeed;       /* Counter7_PERIOD register */
    #endif  /* (CY_PSOC5A) */
    
    /* Set the Measure */
    #if (CapSense_IMPLEMENTATION_CH0 == CapSense_MEASURE_IMPLEMENTATION_FF)
        /* Window PWM  - FF Timer register are retention */
        /* Raw Counter - FF Timer register are retention */
    #else
        /* Window PWM and Raw Counter AUX and D0 set */ 
        #if (CY_PSOC5A)
            enableInterrupts = CyEnterCriticalSection();
            CapSense_PWM_CH0_AUX_CONTROL_REG |= CapSense_AUXCTRL_FIFO_SINGLE_REG;   /* AUX register */
            CapSense_RAW_CH0_AUX_CONTROL_REG |= CapSense_AUXCTRL_FIFO_SINGLE_REG;   /* AUX register */
            CyExitCriticalSection(enableInterrupts);
            
            CapSense_PWM_CH0_ADD_VALUE_REG    = CapSense_MEASURE_FULL_RANGE_LOW;    /* D0 register */
            CapSense_RAW_CH0_ADD_VALUE_REG    = CapSense_MEASURE_FULL_RANGE_LOW;    /* D0 register */
            
        #endif  /* (CY_PSOC5A) */
        
        /* Window PWM */
        CapSense_PWM_CH0_PERIOD_LO_REG    = CapSense_MEASURE_FULL_RANGE_LOW;        /* F0 register */
        
        /* Raw Counter */
        CapSense_RAW_CH0_PERIOD_HI_REG    = CapSense_MEASURE_FULL_RANGE_LOW;        /* F1 register */
        CapSense_RAW_CH0_PERIOD_LO_REG    = CapSense_MEASURE_FULL_RANGE_LOW;        /* F0 register */
    
    #endif  /* (CapSense_IMPLEMENTATION_CH0 == CapSense_MEASURE_IMPLEMENTATION_FF) */ 
    
    #if (CapSense_DESIGN_TYPE == CapSense_TWO_CHANNELS_DESIGN)
        #if (CapSense_IMPLEMENTATION_CH1 == CapSense_MEASURE_IMPLEMENTATION_FF)
            /* Window PWM  - FF Timer register are retention */
            /* Raw Counter - FF Timer register are retention */
        #else
            /* Window PWM and Raw Counter AUX and D0 set */ 
            #if (CY_PSOC5A)
                enableInterrupts = CyEnterCriticalSection();
                CapSense_PWM_CH1_AUX_CONTROL_REG |= CapSense_AUXCTRL_FIFO_SINGLE_REG; /* AUX register */
                CapSense_RAW_CH1_AUX_CONTROL_REG |= CapSense_AUXCTRL_FIFO_SINGLE_REG; /* AUX register */
                CyExitCriticalSection(enableInterrupts);
                
                CapSense_RAW_CH1_ADD_VALUE_REG    = CapSense_MEASURE_FULL_RANGE_LOW;   /* D0 register */
                CapSense_PWM_CH1_ADD_VALUE_REG    = CapSense_MEASURE_FULL_RANGE_LOW;   /* D0 register */
            #endif  /* (CY_PSOC5A) */
            
            /* Window PWM */
            CapSense_PWM_CH1_PERIOD_LO_REG    = CapSense_MEASURE_FULL_RANGE_LOW;       /* F0 register */
            
            /* Raw Counter */
            CapSense_RAW_CH1_PERIOD_HI_REG    = CapSense_MEASURE_FULL_RANGE_LOW;       /* F1 register */
            CapSense_RAW_CH1_PERIOD_LO_REG    = CapSense_MEASURE_FULL_RANGE_LOW;       /* F0 register */
            
        #endif  /* (CapSense_IMPLEMENTATION_CH1 == CapSense_MEASURE_IMPLEMENTATION_FF) */
    
    #endif  /* (CapSense_DESIGN_TYPE == TWO_CHANNELS_DESIGN)*/

    /* Set CONTROL_REG */
    CapSense_CONTROL_REG = CapSense_backup.ctrlreg;

    /* Enable window generation */
    CapSense_CONTROL_REG |= CapSense_CTRL_WINDOW_EN__CH0;
    #if (CapSense_DESIGN_TYPE == CapSense_TWO_CHANNELS_DESIGN) 
        CapSense_CONTROL_REG |= CapSense_CTRL_WINDOW_EN__CH1; 
    #endif  /* CapSense_DESIGN_TYPE */
 
    /* The pins enable are customer concern: Cmod and Rb */
 }
 

/*******************************************************************************
* Function Name: CapSense_Wakeup
********************************************************************************
*
* Summary:
*  Restores CapSense configuration and non-retention register values. 
*  Restores enabled state of component by setting Active mode power template 
*  bits for number of component used within CapSense.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global Variables:
*  CapSense_backup - used to save component state before enter sleep 
*  mode and none-retention registers.
*
*******************************************************************************/
void CapSense_Wakeup(void) 
{
    CapSense_RestoreConfig();
    
    /* Restore CapSense Enable state */
    if (CapSense_backup.enableState != 0u)
    {
        CapSense_Enable();
    }
}


/* [] END OF FILE */
