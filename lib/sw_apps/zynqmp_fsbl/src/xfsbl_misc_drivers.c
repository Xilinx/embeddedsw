/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xfsbl_misc_drivers.c
*
* This is the header file which contains definitions for wrapper functions for
* WDT, CSUDMA drivers
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xfsbl_misc_drivers.h"

/**
 * Include WDT only if WDT is present
 */
#ifdef XFSBL_WDT_PRESENT
#include "xwdtps.h"
#endif
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifdef XFSBL_WDT_PRESENT
static XWdtPs Watchdog;		/* Instance of WatchDog Timer	*/
#endif
/*****************************************************************************/


/**
 * Include WDT code only if WDT is present
 */
#ifdef XFSBL_WDT_PRESENT
/**
 * WDT driver wrapper functions
 */

/*****************************************************************************/
/**
 * This function is used to initialize the system
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
u32 XFsbl_InitWdt()
{
	u32 Status = XFSBL_SUCCESS;
	XWdtPs_Config *ConfigPtr; 	/* Config structure of the WatchDog Timer */
	u32 CounterValue = 1;
	u32 RegValue;


	/**
	 * Initialize the WDT timer
	 */
	ConfigPtr = XWdtPs_LookupConfig(XFSBL_WDT_DEVICE_ID);
	Status = XWdtPs_CfgInitialize(&Watchdog,
			ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		XFsbl_Printf(DEBUG_INFO, "XFSBL_WDT_INIT_FAILED\n\r");
		Status = XFSBL_WDT_INIT_FAILED;
		goto END;
	}

	/**
	 * Setting the divider value
	 */
	XWdtPs_SetControlValue(&Watchdog,
			XWDTPS_CLK_PRESCALE,
			XWDTPS_CCR_PSCALE_4096);
	/**
	 * Convert time to  Watchdog counter reset value
	 */
	CounterValue = XFsbl_ConvertTime_WdtCounter(XFSBL_WDT_EXPIRE_TIME);

	/**
	 * Set the Watchdog counter reset value
	 */
	XWdtPs_SetControlValue(&Watchdog,
			XWDTPS_COUNTER_RESET,
			CounterValue);
	/**
	 * enable reset output, as we are only using this as a basic counter
	 */
	XWdtPs_EnableOutput(&Watchdog, XWDTPS_RESET_SIGNAL);

	/* Enable generation of system reset by PMU due to LPD SWDT */
	RegValue = XFsbl_In32(PMU_GLOBAL_ERROR_SRST_EN_1);
	RegValue |= PMU_GLOBAL_ERROR_SRST_EN_1_LPD_SWDT_MASK;
	XFsbl_Out32(PMU_GLOBAL_ERROR_SRST_EN_1, RegValue);

	/* Enable LPD System Watchdog Timer Error */
	RegValue = XFsbl_In32(PMU_GLOBAL_ERROR_EN_1);
	RegValue |= PMU_GLOBAL_ERROR_EN_1_LPD_SWDT_MASK;
	XFsbl_Out32(PMU_GLOBAL_ERROR_EN_1, RegValue);

	/**
	 * Start the Watchdog timer
	 */
	XWdtPs_Start(&Watchdog);

	XWdtPs_RestartWdt(&Watchdog);

END:
	return Status;
}


/******************************************************************************
*
* This function converts time into Watchdog counter value
*
* @param	watchdog expire time in seconds
*
* @return
*			Counter value for Watchdog
*
* @note		None
*
*******************************************************************************/
u32 XFsbl_ConvertTime_WdtCounter(u32 seconds)
{
	double time = 0.0;
	double CounterValue;
	u32 Crv = 0;
	u32 Prescaler=0;
	u32 PrescalerValue=0;

	Prescaler = XWdtPs_GetControlValue(&Watchdog, XWDTPS_CLK_PRESCALE);

	if (Prescaler == XWDTPS_CCR_PSCALE_0008) {
		PrescalerValue = 8;
	} if (Prescaler == XWDTPS_CCR_PSCALE_0064) {
		PrescalerValue = 64;
	} if (Prescaler == XWDTPS_CCR_PSCALE_4096) {
		PrescalerValue = 4096;
	}

	time = (double)(PrescalerValue) / (double)XPAR_XWDTPS_0_WDT_CLK_FREQ_HZ;

	CounterValue = seconds / time;

	Crv = (u32)CounterValue;
	Crv >>= XFSBL_WDT_CRV_SHIFT;

	return Crv;
}

/******************************************************************************
*
* This function is used to restart Watchdog
*
* @param	None
*
* @return   None
*
* @note		None
*
*******************************************************************************/
void XFsbl_RestartWdt()
{
	XWdtPs_RestartWdt(&Watchdog);
}

/******************************************************************************
*
* This function is used to stop Watchdog
*
* @param	None
*
* @return   None
*
* @note		None
*
*******************************************************************************/
void XFsbl_StopWdt()
{
	u32 RegValue;

	XWdtPs_Stop(&Watchdog);

	/* Disable LPD System Watchdog Timer Error */
	RegValue = XFsbl_In32(PMU_GLOBAL_ERROR_EN_1);
	RegValue &= ~(PMU_GLOBAL_ERROR_EN_1_LPD_SWDT_MASK);
	XFsbl_Out32(PMU_GLOBAL_ERROR_EN_1, RegValue);

	/* Disable generation of system reset by PMU due to LPD SWDT */
	RegValue = XFsbl_In32(PMU_GLOBAL_ERROR_SRST_DIS_1);
	RegValue |= PMU_GLOBAL_ERROR_SRST_DIS_1_LPD_SWDT_MASK;
	XFsbl_Out32(PMU_GLOBAL_ERROR_SRST_DIS_1, RegValue);
}

#endif /** end of WDT wrapper code */
