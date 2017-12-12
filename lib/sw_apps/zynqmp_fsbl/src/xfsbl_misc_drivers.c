/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
* 2.0   bv   12/02/16 Made compliance to MISRAC 2012 guidelines
*       jr   01/24/17 Updated XFsbl_PmInit function, to process only
*                     SYSCFG is enabled and sending PM_SET_CONFIGURATION API
*                     to the PMU
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

/**
 * Include IPI driver only if IPI device is present
 */
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
#include "xipipsu.h"
#endif

#ifdef XPAR_XILPM_ENABLED
#include "pm_defs.h"
#include "pm_cfg_obj.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
#define IPI_DEVICE_ID			XPAR_XIPIPSU_0_DEVICE_ID
#define IPI_PMU_PM_INT_MASK		XPAR_XIPIPS_TARGET_PSU_PMU_0_CH0_MASK
#define PM_INIT				21U
#define PM_IPI_TIMEOUT			(~0)
#endif
#define PM_INIT_COMPLETED_KEY		0x5A5A5A5AU
/************************** Function Prototypes ******************************/
static u32 XFsbl_ConvertTime_WdtCounter(u32 seconds);
/************************** Variable Definitions *****************************/
#ifdef XFSBL_WDT_PRESENT
static XWdtPs Watchdog={0};		/* Instance of WatchDog Timer	*/
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
u32 XFsbl_InitWdt(void)
{
	s32 Status;
	u32 UStatus;
	XWdtPs_Config *ConfigPtr; 	/* Config structure of the WatchDog Timer */
	u32 CounterValue;
	u32 RegValue;


	/**
	 * Initialize the WDT timer
	 */
	ConfigPtr = XWdtPs_LookupConfig(XFSBL_WDT_DEVICE_ID);

	if(ConfigPtr==NULL) {
		UStatus = XFSBL_WDT_INIT_FAILED;
		goto END;
	}

	Status = XWdtPs_CfgInitialize(&Watchdog,
			ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != XFSBL_SUCCESS) {
		XFsbl_Printf(DEBUG_INFO, "XFSBL_WDT_INIT_FAILED\n\r");
		UStatus = XFSBL_WDT_INIT_FAILED;
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

	UStatus = XFSBL_SUCCESS;
END:
	return UStatus;
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
static u32 XFsbl_ConvertTime_WdtCounter(u32 seconds)
{
	double time;
	double CounterValue;
	u32 Crv;
	u32 Prescaler;
	u32 PrescalerValue;

	Prescaler = XWdtPs_GetControlValue(&Watchdog, XWDTPS_CLK_PRESCALE);

	if (Prescaler == XWDTPS_CCR_PSCALE_0008) {
		PrescalerValue = 8;
	} else if (Prescaler == XWDTPS_CCR_PSCALE_0064) {
		PrescalerValue = 64;
	} else if (Prescaler == XWDTPS_CCR_PSCALE_4096) {
		PrescalerValue = 4096;
	}
	else
	{
		PrescalerValue = 0U;
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
void XFsbl_RestartWdt(void)
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
void XFsbl_StopWdt(void)
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

/******************************************************************************
*
* This function is used to notify PMU firmware (if present) that initialization
* of all PM related register is completed
*
* @param	None
*
* @return	Success or XFSBL_ERROR_PM_INIT in case of any error
*
* @note		None
*
*******************************************************************************/
u32 XFsbl_PmInit(void)
{
	u32 UStatus;
/* Proceed only if SYSCFG is enabled */
#ifdef XPAR_XILPM_ENABLED
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
	s32 Status ;
	XIpiPsu IpiInstance;
	XIpiPsu_Config *Config;
	u32 Response = 0U;

	#ifdef __aarch64__
	u32 CfgCmd[2U] = {PM_SET_CONFIGURATION, (u32)((u64)&XPm_ConfigObject[0])};
	#else
	u32 CfgCmd[2U] = {PM_SET_CONFIGURATION, (u32)&XPm_ConfigObject[0]};
	#endif
#endif

	/**
	 * Mark to the PMU that FSBL has completed with system initialization
	 * This is needed for the JTAG boot mode
	 */
	Xil_Out32(PMU_GLOBAL_PERS_GLOB_GEN_STORAGE5, PM_INIT_COMPLETED_KEY);

	/**
	 * Check if PMU FW is present
	 * If PMU FW is present, but IPI device does not exist, report an error
	 * If IPI device exists, but PMU FW is not present, do not issue IPI
	 */
	if ((XFsbl_In32(PMU_GLOBAL_GLOBAL_CNTRL) &
			PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) !=
				PMU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK) {
		XFsbl_Printf(DEBUG_PRINT_ALWAYS,"PMU-FW is not running, certain applications may not be supported.\n\r");
		UStatus = XFSBL_SUCCESS;
		goto END;
	}
#ifndef XPAR_XIPIPSU_0_DEVICE_ID
	else {
		UStatus = XFSBL_ERROR_PM_INIT;
		XFsbl_Printf(DEBUG_GENERAL,
			"PMU firmware is present, but IPI is disabled\r\n");
		goto END;
	}
#endif

#ifdef XPAR_XIPIPSU_0_DEVICE_ID
	/* Initialize IPI peripheral */
	Config = XIpiPsu_LookupConfig(IPI_DEVICE_ID);
	if (Config == NULL) {
		UStatus = XFSBL_ERROR_PM_INIT;
		goto END;
	}

	Status = XIpiPsu_CfgInitialize(&IpiInstance, Config,
			Config->BaseAddress);
	if (XFSBL_SUCCESS != Status) {
		UStatus = XFSBL_ERROR_PM_INIT;
		goto END;
	}
	/* Send PM_SET_CONFIGURATION API to the PMU */
	Status = XIpiPsu_WriteMessage(&IpiInstance, IPI_PMU_PM_INT_MASK,
					&CfgCmd[0], 2U, XIPIPSU_BUF_TYPE_MSG);
	if (XFSBL_SUCCESS != Status) {
		UStatus = XFSBL_ERROR_PM_INIT;
		goto END;
	}

	Status = XIpiPsu_TriggerIpi(&IpiInstance, IPI_PMU_PM_INT_MASK);
	if (XFSBL_SUCCESS != Status) {
		UStatus = XFSBL_ERROR_PM_INIT;
		goto END;
	}


	/* This is a blocking call, wait until IPI is handled by the PMU */
	Status = XIpiPsu_PollForAck(&IpiInstance, IPI_PMU_PM_INT_MASK,
				  PM_IPI_TIMEOUT);
	if (XFSBL_SUCCESS != Status) {
		UStatus = XFSBL_ERROR_PM_INIT;
		goto END;
	}

	Status = XIpiPsu_ReadMessage(&IpiInstance, IPI_PMU_PM_INT_MASK,
					&Response, 1, XIPIPSU_BUF_TYPE_RESP);
	if ((Status != XFSBL_SUCCESS) || (Response != XFSBL_SUCCESS)) {
		UStatus = XFSBL_ERROR_PM_INIT;
		goto END;
	}
#endif /** end of IPI related code */
#endif /* End of XPAR_XILPM_ENABLED */
	XFsbl_Printf(DEBUG_DETAILED,"PM Init Success\r\n");
	UStatus = XFSBL_SUCCESS;
END:
	return UStatus;
}
