/*******************************************************************************
* File Name: I2C.c
*
* Version 4.0
*
* Description: This file contains routines for I2C bulk read/write from PSoC 3 to
*              an external I2C slave (EEPROM, CODEC, LCD, Coprocessor on the 
*              current design)
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
#include <project.h>
#include <LCD.h>

/* USER_CODE: [I2C] CY8CKIT-033A application layer code uses I2C APIs (WriteToSlave and iAP2_ReadFromSlave) exposed 
 * by iAP2 component's iAP2_I2C.c file. If iAP2 component is taken out of the design, then change "#if 0" to "#if 1" so
 * that the WriteToSlave API required for I2C communication with CODEC and LCD is restored */
#if 1
/*******************************************************************************
* Function Name: WriteToSlave
********************************************************************************
* Summary:
*        Writes data to the I2C slave device
*
* Parameters:
*  address - address of the I2C slave being addressed
*  writeBuffer - buffer which contains the data to be written
*  length - length of the data to be written
* 
* Return:
*  None
*
* Note:
*  Exits the function if the write is successful, otherwise keeps trying 
*  (blocking call). This routine should only be enabled when iAP2 component is 
*  not used in the design (Mac/PC only designs for example). If iAP2 component
*  is present in the design, iAP2 internal I2C read/write routines are used.
*
*******************************************************************************/
void WriteToSlave(uint8 address, uint8* writeBuffer, uint8 length)
{
    CYDATA uint8 count;
    uint8 status;
    
    /* Wait for the current transfer to complete */
    while (I2C_Master_MasterStatus() & I2C_Master_MSTAT_XFER_INP);
    
    do
    {
        do
        {
            /* clear any previous status */
            I2C_Master_MasterClearStatus(); 
            
            /* write data to the slave */
            status = I2C_Master_MasterWriteBuf(address, writeBuffer, length, I2C_Master_MODE_COMPLETE_XFER);
            
        } while(status != I2C_Master_MSTR_NO_ERROR);
        
        count = 0;
        
        /* wait till write operation is complete or an error flag is set */
        while ((I2C_Master_MasterStatus() & (I2C_Master_MSTAT_WR_CMPLT | I2C_Master_MSTAT_ERR_MASK)) == 0)
        {
            if(I2C_DELAY_COUNT == count)
            {
                break;
            }
            
            CyDelayUs(length);
            count++;
        }
    
    }while(I2C_Master_MasterStatus()!= I2C_Master_MSTAT_WR_CMPLT);
    
    /* clear any previous status */
    I2C_Master_MasterClearStatus(); 
}
#endif

/*******************************************************************************
* Function Name: SetMaxI2CBusSpeed
********************************************************************************
* Summary:
*      This function dynamically changes the clock speed for the fixed function
*      I2C component by changing the clock divider value for I2C block. The I2C
*      clock is set to I2C_MAX_BUS_SPEED (defined in Application.h) which can be
*      set to one of the following by users depending on the application 
*      requirements
*      1. I2C_BUS_SPEED_50KHZ
*      2. I2C_BUS_SPEED_100KHZ
*      3. I2C_BUS_SPEED_400KHZ 
*
* Parameters:
*  voids
*
* Return:
*  void
*
*******************************************************************************/
void SetMaxI2CBusSpeed(void)
{
//	#if (I2C_MAX_BUS_SPEED == I2C_BUS_SPEED_50KHZ)
//        iAP2_I2C_Master_CLKDIV1_REG = I2C_DIVIDER_50KHZ_OPERATION;
//    #elif (I2C_MAX_BUS_SPEED == I2C_BUS_SPEED_100KHZ)
//        iAP2_I2C_Master_CLKDIV1_REG = I2C_DIVIDER_100KHZ_OPERATION;
//    #elif (I2C_MAX_BUS_SPEED == I2C_BUS_SPEED_400KHZ)
        I2C_Master_CLKDIV1_REG = I2C_DIVIDER_400KHZ_OPERATION;
//    #endif
}

/*******************************************************************************
* Function Name: SetMinI2CBusSpeed
********************************************************************************
* Summary:
*      This function dynamically changes the clock speed for the fixed function
*      I2C component by changing the clock divider value for I2C block. The I2C
*      clock is set to I2C_MIN_BUS_SPEED (defined in Application.h) which can be
*      set to one of the following by users depending on the application 
*      requirements
*      1. I2C_BUS_SPEED_50KHZ
*      2. I2C_BUS_SPEED_100KHZ
*      3. I2C_BUS_SPEED_400KHZ 
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void SetMinI2CBusSpeed(void)
{
//	#if (I2C_MIN_BUS_SPEED == I2C_BUS_SPEED_50KHZ)
//        iAP2_I2C_Master_CLKDIV1_REG = I2C_DIVIDER_50KHZ_OPERATION;
//    #elif (I2C_MIN_BUS_SPEED == I2C_BUS_SPEED_100KHZ)
//        iAP2_I2C_Master_CLKDIV1_REG = I2C_DIVIDER_100KHZ_OPERATION;
//    #elif (I2C_MIN_BUS_SPEED == I2C_BUS_SPEED_400KHZ)
        I2C_Master_CLKDIV1_REG = I2C_DIVIDER_400KHZ_OPERATION;
//    #endif
}

/* [] END OF FILE */
