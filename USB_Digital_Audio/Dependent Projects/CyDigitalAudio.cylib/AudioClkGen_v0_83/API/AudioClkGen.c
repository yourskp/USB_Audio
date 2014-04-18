/*******************************************************************************
* File Name: `$INSTANCE_NAME`.c  
* 
* Version 0.83 
*
* Description:
*  This file contains API to enable firmware control of divider settings and 
*  audio clock generation.
*
*  Note: Only includes support for digital audio.
*
*******************************************************************************
* Copyright 2008-2013, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "cytypes.h"
#include "cyfitter.h"
#include "CyLib.h"

#include "`$INSTANCE_NAME`.h"
#include "`$INSTANCE_NAME`_Clock_I2S.h"
#include "`$INSTANCE_NAME`_Clock_SCK.h"

#define `$INSTANCE_NAME`_HIGH_PERIOD		(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_div_Div_u0__D0_REG)
#define `$INSTANCE_NAME`_LOW_PERIOD 		(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_div_Div_u0__D1_REG)

#define `$INSTANCE_NAME`_C0_0				(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_sync_SofCounter_u0__D0_REG)
#define `$INSTANCE_NAME`_C0_1				(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_sync_SofCounter_u1__D0_REG)
#define `$INSTANCE_NAME`_N					(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_sync_Counter_u0__D0_REG)
#define `$INSTANCE_NAME`_SHIFT_CNT			(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_sync_Counter_u0__D1_REG)

#define `$INSTANCE_NAME`_NC0_INIT_0			(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_shaper_Div_u0__A1_REG)
#define `$INSTANCE_NAME`_NC0_INIT_1			(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_shaper_Div_u1__A1_REG)

#define `$INSTANCE_NAME`_THRESHOLD_0		(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_shaper_Div_u0__D0_REG)
#define `$INSTANCE_NAME`_THRESHOLD_1		(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_shaper_Div_u1__D0_REG)

#define `$INSTANCE_NAME`_M_0				(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_shaper_Div_u0__D1_REG)
#define `$INSTANCE_NAME`_M_1				(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_shaper_Div_u1__D1_REG)

#define `$INSTANCE_NAME`_NC0_0				(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_sync_SofCounter_u0__D1_REG)
#define `$INSTANCE_NAME`_NC0_1				(*(reg8 *) `$INSTANCE_NAME`_UDB_ACG_sync_SofCounter_u1__D1_REG)

/*******************************************************************************
* Function Name: `$INSTANCE_NAME`_Start
********************************************************************************
* Summary:
*  The start function initializes the fractional divider with the default values.
*
* Parameters:  
*  void  
*
* Return: 
*  void
*
*******************************************************************************/
void `$INSTANCE_NAME`_Start(void)
{
	/* Divide by 23 plus a fraction
	 * When the waveform is 24 cycles long the extra cycle is added to the high period of the clock output */
	`$INSTANCE_NAME`_HIGH_PERIOD = (11-2);	/* High period is 11 or 12 cycles */
	`$INSTANCE_NAME`_LOW_PERIOD = (23-2);	/* Overall period is 23 or 24 cycles */
	
	/* Settings that aren't audio rate dependent */
	`$INSTANCE_NAME`_C0_0 = (uint8)((24000 - 74) & 0xFF);
	`$INSTANCE_NAME`_C0_1 = (uint8)(((24000 - 74) >> 8) & 0xFF);
	`$INSTANCE_NAME`_N = (55-2);
	`$INSTANCE_NAME`_SHIFT_CNT = (16-1);	/* Shift counter */	
}

/*******************************************************************************
* Function Name: `$INSTANCE_NAME`_Stop
********************************************************************************
* Summary:
*  The stop function halts the fractional divider (currently a placeholder).
*
* Parameters:  
*  void  
*
* Return: 
*  void
*
*******************************************************************************/
void `$INSTANCE_NAME`_Stop(void)
{
	;
}

/*******************************************************************************
* Function Name: `$INSTANCE_NAME`_SetDividerAudioRate
********************************************************************************
* Summary:
*  This function is used to setup all the required internal values for 
*  to generate common audio sample rate clocks from a 24 MHz input
*
* Parameters:  
*  rate: Desired audio sample rate to be fed to PLL.  
*  		 Must be one of the following:
*		`$INSTANCE_NAME`_RATE_8KHZ
*		`$INSTANCE_NAME`_RATE_11KHZ
*		`$INSTANCE_NAME`_RATE_16KHZ
*		`$INSTANCE_NAME`_RATE_22KHZ
*		`$INSTANCE_NAME`_RATE_32KHZ
*		`$INSTANCE_NAME`_RATE_44KHZ
* 		`$INSTANCE_NAME`_RATE_48KHZ
* 		`$INSTANCE_NAME`_RATE_96KHZ
*
* Return: 
*  char*: string with current audio sample frequency
*
*******************************************************************************/
void `$INSTANCE_NAME`_SetAudioRate(uint8 rate) `=ReentrantKeil("`$INSTANCE_NAME`_SetAudioRate")`
{
	/* Turn off the PLL */
	//CyPLL_OUT_Stop();	    
	
	switch (rate) {	
		case `$INSTANCE_NAME`_RATE_8KHZ:				
		case `$INSTANCE_NAME`_RATE_16KHZ:				
		case `$INSTANCE_NAME`_RATE_32KHZ:				
		case `$INSTANCE_NAME`_RATE_48KHZ:
		case `$INSTANCE_NAME`_RATE_96KHZ:		
				CY_SET_XTND_REG16((void CYFAR *)(CYDEV_FASTCLK_PLL_P), 48);
				`$INSTANCE_NAME`_NC0_INIT_0 = (uint8)(24640 & 0xFF);
				`$INSTANCE_NAME`_NC0_INIT_1 = (uint8)((24640 >> 8) & 0xFF);
				`$INSTANCE_NAME`_THRESHOLD_0 = (uint8)(36288 & 0xFF);
				`$INSTANCE_NAME`_THRESHOLD_1 = (uint8)((36288 >> 8) & 0xFF);
				`$INSTANCE_NAME`_M_0 = (uint8)(56320 & 0xFF);
				`$INSTANCE_NAME`_M_1 = (uint8)((56320 >> 8) & 0xFF);										
				`$INSTANCE_NAME`_NC0_0 = (uint8)(24640 & 0xFF);
				`$INSTANCE_NAME`_NC0_1 = (uint8)((24640 >> 8) & 0xFF);
			break;
		case `$INSTANCE_NAME`_RATE_11KHZ:		
		case `$INSTANCE_NAME`_RATE_22KHZ:		
		case `$INSTANCE_NAME`_RATE_44KHZ:
				CY_SET_XTND_REG16((void CYFAR *)(CYDEV_FASTCLK_PLL_P), 44);
				`$INSTANCE_NAME`_NC0_INIT_0 = (uint8)(21696 & 0xFF);
				`$INSTANCE_NAME`_NC0_INIT_1 = (uint8)((21696 >> 8) & 0xFF);
				`$INSTANCE_NAME`_THRESHOLD_0 = (uint8)(39296 & 0xFF);
				`$INSTANCE_NAME`_THRESHOLD_1 = (uint8)((39296 >> 8) & 0xFF);
				`$INSTANCE_NAME`_M_0 = (uint8)(56448 & 0xFF);
				`$INSTANCE_NAME`_M_1 = (uint8)((56448 >> 8) & 0xFF);										
				`$INSTANCE_NAME`_NC0_0 = (uint8)(21696 & 0xFF);
				`$INSTANCE_NAME`_NC0_1 = (uint8)((21696 >> 8) & 0xFF);									
			break;				
		default:
			break;
	}
	
	/* Turn on the PLL */
	CyPLL_OUT_Start(0);
    
	/* Wait to allow the PLL clock to stabilize */
	CyDelayUs(250);
	
	switch (rate) {	
		case `$INSTANCE_NAME`_RATE_8KHZ:				
			#if (`$INSTANCE_NAME`_Enable_Mclk)
			`$INSTANCE_NAME`_Clock_SCK_SetDividerRegister(23,0);	/* divide by 24 */
			#endif			
			#if (`$INSTANCE_NAME`_Enable_I2Sclk)
			`$INSTANCE_NAME`_Clock_I2S_SetDividerRegister(47,0);	/* divide by 48 */
			#endif
			break;
		case `$INSTANCE_NAME`_RATE_16KHZ:				
			#if (`$INSTANCE_NAME`_Enable_Mclk)
			`$INSTANCE_NAME`_Clock_SCK_SetDividerRegister(11,0);	/* divide by 12 */
			#endif
			#if (`$INSTANCE_NAME`_Enable_I2Sclk)
			`$INSTANCE_NAME`_Clock_I2S_SetDividerRegister(23,0);	/* divide by 24 */
			#endif
			break;
		case `$INSTANCE_NAME`_RATE_32KHZ:				
			#if (`$INSTANCE_NAME`_Enable_Mclk)
			`$INSTANCE_NAME`_Clock_SCK_SetDividerRegister(5,0);	/* divide by 6 */
			#endif
			#if (`$INSTANCE_NAME`_Enable_I2Sclk)
			`$INSTANCE_NAME`_Clock_I2S_SetDividerRegister(11,0);	/* divide by 12 */
			#endif
			break;
		case `$INSTANCE_NAME`_RATE_48KHZ:		
			#if (`$INSTANCE_NAME`_Enable_Mclk)
			`$INSTANCE_NAME`_Clock_SCK_SetDividerRegister(3,0);	/* divide by 4 */
			#endif
			#if (`$INSTANCE_NAME`_Enable_I2Sclk)
			`$INSTANCE_NAME`_Clock_I2S_SetDividerRegister(7,0);	/* divide by 8 */
			#endif
			break;
		case `$INSTANCE_NAME`_RATE_11KHZ:		
			#if (`$INSTANCE_NAME`_Enable_Mclk)
			`$INSTANCE_NAME`_Clock_SCK_SetDividerRegister(15,0);	/* divide by 16 */
			#endif
			#if (`$INSTANCE_NAME`_Enable_I2Sclk)
			`$INSTANCE_NAME`_Clock_I2S_SetDividerRegister(31,0);	/* divide by 32 */
			#endif
			break;
		case `$INSTANCE_NAME`_RATE_22KHZ:		
			#if (`$INSTANCE_NAME`_Enable_Mclk)
			`$INSTANCE_NAME`_Clock_SCK_SetDividerRegister(7,0);	/* divide by 8 */
			#endif
			#if (`$INSTANCE_NAME`_Enable_I2Sclk)
			`$INSTANCE_NAME`_Clock_I2S_SetDividerRegister(15,0);	/* divide by 16 */
			#endif
			break;
		case `$INSTANCE_NAME`_RATE_44KHZ:
			#if (`$INSTANCE_NAME`_Enable_Mclk)
			`$INSTANCE_NAME`_Clock_SCK_SetDividerRegister(3,0);	/* divide by 4 */
			#endif
			#if (`$INSTANCE_NAME`_Enable_I2Sclk)
			`$INSTANCE_NAME`_Clock_I2S_SetDividerRegister(7,0);	/* divide by 8 */
			#endif
			break;
		case `$INSTANCE_NAME`_RATE_96KHZ: 
			#if (`$INSTANCE_NAME`_Enable_Mclk) 
			`$INSTANCE_NAME`_Clock_SCK_SetDividerRegister(1,0); //divide by 2 
			#endif 
			#if (`$INSTANCE_NAME`_Enable_I2Sclk) 
			`$INSTANCE_NAME`_Clock_I2S_SetDividerRegister(3,0); //divide by 4 
			#endif 
			break;
		default:
			break;
	}
}

/* [] END OF FILE */
