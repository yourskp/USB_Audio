/*******************************************************************************
* File Name: AudioClkGen.c  
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

#include "AudioClkGen.h"
#include "AudioClkGen_Clock_I2S.h"
#include "AudioClkGen_Clock_SCK.h"

#define AudioClkGen_HIGH_PERIOD		(*(reg8 *) AudioClkGen_UDB_ACG_div_Div_u0__D0_REG)
#define AudioClkGen_LOW_PERIOD 		(*(reg8 *) AudioClkGen_UDB_ACG_div_Div_u0__D1_REG)

#define AudioClkGen_C0_0				(*(reg8 *) AudioClkGen_UDB_ACG_sync_SofCounter_u0__D0_REG)
#define AudioClkGen_C0_1				(*(reg8 *) AudioClkGen_UDB_ACG_sync_SofCounter_u1__D0_REG)
#define AudioClkGen_N					(*(reg8 *) AudioClkGen_UDB_ACG_sync_Counter_u0__D0_REG)
#define AudioClkGen_SHIFT_CNT			(*(reg8 *) AudioClkGen_UDB_ACG_sync_Counter_u0__D1_REG)

#define AudioClkGen_NC0_INIT_0			(*(reg8 *) AudioClkGen_UDB_ACG_shaper_Div_u0__A1_REG)
#define AudioClkGen_NC0_INIT_1			(*(reg8 *) AudioClkGen_UDB_ACG_shaper_Div_u1__A1_REG)

#define AudioClkGen_THRESHOLD_0		(*(reg8 *) AudioClkGen_UDB_ACG_shaper_Div_u0__D0_REG)
#define AudioClkGen_THRESHOLD_1		(*(reg8 *) AudioClkGen_UDB_ACG_shaper_Div_u1__D0_REG)

#define AudioClkGen_M_0				(*(reg8 *) AudioClkGen_UDB_ACG_shaper_Div_u0__D1_REG)
#define AudioClkGen_M_1				(*(reg8 *) AudioClkGen_UDB_ACG_shaper_Div_u1__D1_REG)

#define AudioClkGen_NC0_0				(*(reg8 *) AudioClkGen_UDB_ACG_sync_SofCounter_u0__D1_REG)
#define AudioClkGen_NC0_1				(*(reg8 *) AudioClkGen_UDB_ACG_sync_SofCounter_u1__D1_REG)

/*******************************************************************************
* Function Name: AudioClkGen_Start
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
void AudioClkGen_Start(void)
{
	/* Divide by 23 plus a fraction
	 * When the waveform is 24 cycles long the extra cycle is added to the high period of the clock output */
	AudioClkGen_HIGH_PERIOD = (11-2);	/* High period is 11 or 12 cycles */
	AudioClkGen_LOW_PERIOD = (23-2);	/* Overall period is 23 or 24 cycles */
	
	/* Settings that aren't audio rate dependent */
	AudioClkGen_C0_0 = (uint8)((24000 - 74) & 0xFF);
	AudioClkGen_C0_1 = (uint8)(((24000 - 74) >> 8) & 0xFF);
	AudioClkGen_N = (55-2);
	AudioClkGen_SHIFT_CNT = (16-1);	/* Shift counter */	
}

/*******************************************************************************
* Function Name: AudioClkGen_Stop
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
void AudioClkGen_Stop(void)
{
	;
}

/*******************************************************************************
* Function Name: AudioClkGen_SetDividerAudioRate
********************************************************************************
* Summary:
*  This function is used to setup all the required internal values for 
*  to generate common audio sample rate clocks from a 24 MHz input
*
* Parameters:  
*  rate: Desired audio sample rate to be fed to PLL.  
*  		 Must be one of the following:
*		AudioClkGen_RATE_8KHZ
*		AudioClkGen_RATE_11KHZ
*		AudioClkGen_RATE_16KHZ
*		AudioClkGen_RATE_22KHZ
*		AudioClkGen_RATE_32KHZ
*		AudioClkGen_RATE_44KHZ
* 		AudioClkGen_RATE_48KHZ
* 		AudioClkGen_RATE_96KHZ
*
* Return: 
*  char*: string with current audio sample frequency
*
*******************************************************************************/
void AudioClkGen_SetAudioRate(uint8 rate) CYREENTRANT
{
	/* Turn off the PLL */
	//CyPLL_OUT_Stop();	    
	
	switch (rate) {	
		case AudioClkGen_RATE_8KHZ:				
		case AudioClkGen_RATE_16KHZ:				
		case AudioClkGen_RATE_32KHZ:				
		case AudioClkGen_RATE_48KHZ:
		case AudioClkGen_RATE_96KHZ:		
				CY_SET_XTND_REG16((void CYFAR *)(CYDEV_FASTCLK_PLL_P), 48);
				AudioClkGen_NC0_INIT_0 = (uint8)(24640 & 0xFF);
				AudioClkGen_NC0_INIT_1 = (uint8)((24640 >> 8) & 0xFF);
				AudioClkGen_THRESHOLD_0 = (uint8)(36288 & 0xFF);
				AudioClkGen_THRESHOLD_1 = (uint8)((36288 >> 8) & 0xFF);
				AudioClkGen_M_0 = (uint8)(56320 & 0xFF);
				AudioClkGen_M_1 = (uint8)((56320 >> 8) & 0xFF);										
				AudioClkGen_NC0_0 = (uint8)(24640 & 0xFF);
				AudioClkGen_NC0_1 = (uint8)((24640 >> 8) & 0xFF);
			break;
		case AudioClkGen_RATE_11KHZ:		
		case AudioClkGen_RATE_22KHZ:		
		case AudioClkGen_RATE_44KHZ:
				CY_SET_XTND_REG16((void CYFAR *)(CYDEV_FASTCLK_PLL_P), 44);
				AudioClkGen_NC0_INIT_0 = (uint8)(21696 & 0xFF);
				AudioClkGen_NC0_INIT_1 = (uint8)((21696 >> 8) & 0xFF);
				AudioClkGen_THRESHOLD_0 = (uint8)(39296 & 0xFF);
				AudioClkGen_THRESHOLD_1 = (uint8)((39296 >> 8) & 0xFF);
				AudioClkGen_M_0 = (uint8)(56448 & 0xFF);
				AudioClkGen_M_1 = (uint8)((56448 >> 8) & 0xFF);										
				AudioClkGen_NC0_0 = (uint8)(21696 & 0xFF);
				AudioClkGen_NC0_1 = (uint8)((21696 >> 8) & 0xFF);									
			break;				
		default:
			break;
	}
	
	/* Turn on the PLL */
	CyPLL_OUT_Start(0);
    
	/* Wait to allow the PLL clock to stabilize */
	CyDelayUs(250);
	
	switch (rate) {	
		case AudioClkGen_RATE_8KHZ:				
			#if (AudioClkGen_Enable_Mclk)
			AudioClkGen_Clock_SCK_SetDividerRegister(23,0);	/* divide by 24 */
			#endif			
			#if (AudioClkGen_Enable_I2Sclk)
			AudioClkGen_Clock_I2S_SetDividerRegister(47,0);	/* divide by 48 */
			#endif
			break;
		case AudioClkGen_RATE_16KHZ:				
			#if (AudioClkGen_Enable_Mclk)
			AudioClkGen_Clock_SCK_SetDividerRegister(11,0);	/* divide by 12 */
			#endif
			#if (AudioClkGen_Enable_I2Sclk)
			AudioClkGen_Clock_I2S_SetDividerRegister(23,0);	/* divide by 24 */
			#endif
			break;
		case AudioClkGen_RATE_32KHZ:				
			#if (AudioClkGen_Enable_Mclk)
			AudioClkGen_Clock_SCK_SetDividerRegister(5,0);	/* divide by 6 */
			#endif
			#if (AudioClkGen_Enable_I2Sclk)
			AudioClkGen_Clock_I2S_SetDividerRegister(11,0);	/* divide by 12 */
			#endif
			break;
		case AudioClkGen_RATE_48KHZ:		
			#if (AudioClkGen_Enable_Mclk)
			AudioClkGen_Clock_SCK_SetDividerRegister(3,0);	/* divide by 4 */
			#endif
			#if (AudioClkGen_Enable_I2Sclk)
			AudioClkGen_Clock_I2S_SetDividerRegister(7,0);	/* divide by 8 */
			#endif
			break;
		case AudioClkGen_RATE_11KHZ:		
			#if (AudioClkGen_Enable_Mclk)
			AudioClkGen_Clock_SCK_SetDividerRegister(15,0);	/* divide by 16 */
			#endif
			#if (AudioClkGen_Enable_I2Sclk)
			AudioClkGen_Clock_I2S_SetDividerRegister(31,0);	/* divide by 32 */
			#endif
			break;
		case AudioClkGen_RATE_22KHZ:		
			#if (AudioClkGen_Enable_Mclk)
			AudioClkGen_Clock_SCK_SetDividerRegister(7,0);	/* divide by 8 */
			#endif
			#if (AudioClkGen_Enable_I2Sclk)
			AudioClkGen_Clock_I2S_SetDividerRegister(15,0);	/* divide by 16 */
			#endif
			break;
		case AudioClkGen_RATE_44KHZ:
			#if (AudioClkGen_Enable_Mclk)
			AudioClkGen_Clock_SCK_SetDividerRegister(3,0);	/* divide by 4 */
			#endif
			#if (AudioClkGen_Enable_I2Sclk)
			AudioClkGen_Clock_I2S_SetDividerRegister(7,0);	/* divide by 8 */
			#endif
			break;
		case AudioClkGen_RATE_96KHZ: 
			#if (AudioClkGen_Enable_Mclk) 
			AudioClkGen_Clock_SCK_SetDividerRegister(1,0); //divide by 2 
			#endif 
			#if (AudioClkGen_Enable_I2Sclk) 
			AudioClkGen_Clock_I2S_SetDividerRegister(3,0); //divide by 4 
			#endif 
			break;
		default:
			break;
	}
}

/* [] END OF FILE */
