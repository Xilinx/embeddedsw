/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_main.c
*
* This is the main file which contains code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/12/2018 Initial release
* 1.01  kc   04/08/2019 Added code to request UART if debug prints
*                       are enabled
*       kc   05/09/2019 Addeed code to disable CFRAME isolation
*                       as soon as we boot in PLM
*       ma   08/01/2019 Removed LPD module init related code from PLM app
* 1.02  kc   02/19/2020 Moved PLM banner print to XilPlmi
*       kc   03/23/2020 Minor code cleanup
*       td   10/19/2020 MISRA C Fixes
* 1.03  skd  03/16/2021 Warnings Fixed
*       ma   03/24/2021 Store DebugLog structure to RTCA
* 1.04  td   07/08/2021 Fix doxygen warnings
*       bsv  07/18/2021 Print PLM banner at the beginning of PLM execution
*       kc   07/22/2021 Issue internal POR for VP1802 ES1 devices
*       bsv  07/24/2021 Clear RTC area at the beginning of PLM
*       rb   07/28/2021 Check Efuse DNA_57 bit before issuing internal POR
*       bsv  08/13/2021 Code clean up to reduce size
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_proc.h"
#include "xplm_startup.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xplm_loader.h"
#include "xplmi_err.h"
#include "xplmi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define PLM_VP1802_POR_SETTLE_TIME	(25000U)

/************************** Function Prototypes ******************************/
static int XPlm_Init(void);
static void XPlm_PerformInternalPOR(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief This is PLM main function
 *
 * @return	Ideally should not return, in case if it reaches end,
 *		error is returned
 *
 *****************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	/** Initialize the processor, tasks lists */
	Status = XPlm_Init();
	if (Status != XST_SUCCESS)
	{
		XPlmi_ErrMgr(Status);
	}

	/** Timestamps are enabled now */
	/** Print PLM banner */
	XPlmi_PrintPlmBanner();

	/** Initialize the start up events */
	Status = XPlm_AddStartUpTasks();
	if (Status != XST_SUCCESS)
	{
		XPlmi_ErrMgr(Status);
	}

	/** Run the handlers in task loop based on the priority */
	XPlmi_TaskDispatchLoop();

	/** Should never reach here */
	while (TRUE) {
		;
	}
	Status = XST_FAILURE;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function check conditions and perform internal POR
 * 		for VP1802 and VP1502 device if required.
 *
 * @return	None.
 *
 *****************************************************************************/
static void XPlm_PerformInternalPOR(void)
{
	u32 IdCode = XPlmi_In32(PMC_TAP_IDCODE) &
			PMC_TAP_IDCODE_SIREV_DVCD_MASK;
	u32 ResetReason = XPlmi_In32(CRP_RESET_REASON);
	u8 SlrType = (u8)(XPlmi_In32(PMC_TAP_SLR_TYPE) &
		PMC_TAP_SLR_TYPE_VAL_MASK);
	u32 DnaBit = XPlmi_In32(EFUSE_CACHE_DNA_1) &
			EFUSE_CACHE_DNA_1_BIT25_MASK;
	PdiSrc_t BootMode = XLoader_GetBootMode();

	if ((IdCode != PMC_TAP_IDCODE_ES1_VP1802) &&
		(IdCode != PMC_TAP_IDCODE_ES1_VP1502)) {
		/* Not a VP1802 Or VP1502 device */
		goto END;
	}

	if (SlrType != XLOADER_SSIT_MASTER_SLR) {
		/* Not a Master SLR */
		goto END;
	}

	if ((BootMode == XLOADER_PDI_SRC_JTAG) ||
		(BootMode == XLOADER_PDI_SRC_SMAP)) {
		/* Bootmode check failed for IPOR of VP1502/VP1802 device */
		goto END;
	}

	if (DnaBit == 0x00U) {
		/* Efuse DNA_57 bit should be non-zero for IPOR */
		goto END;
	}

	/* All the pre-conditions are met to do IPOR of VP1502/VP1802 device */
	if (ResetReason == CRP_RESET_REASON_EXT_POR_MASK) {
		usleep(PLM_VP1802_POR_SETTLE_TIME);
		XPlmi_PORHandler();
	}

END:
	return;
}

/*****************************************************************************/
/**
 * @brief This function initializes DMA, Run Time Config area, the processor
 * 		and task list structures.
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
static int XPlm_Init(void)
{
	int Status = XST_FAILURE;
	u8 PmcVersion = (u8)(XPlmi_In32(PMC_TAP_VERSION) &
			PMC_TAP_VERSION_PMC_VERSION_MASK);

	/**
	 * Disable CFRAME isolation for VCCRAM for ES1 Silicon
	 */
	if (PmcVersion == XPLMI_SILICON_ES1_VAL) {
		XPlmi_UtilRMW(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
		 PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_PL_CFRAME_MASK, 0U);
	}

	/**
	 * Reset the wakeup signal set by ROM
	 * Otherwise MB will always wakeup, irrespective of the sleep state
	 */
	XPlmi_PpuWakeUpDis();

	/* Initializes the DMA pointers */
	Status = XPlmi_DmaInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_RunTimeConfigInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Initialize debug log structure */
	XPlmi_InitDebugLogBuffer();

#ifdef DEBUG_UART_MDM
	/** If MDM UART, banner can be printed before any initialization */
	XPlmi_InitUart();
#endif

	/** Initialize the processor, enable exceptions */
	Status = XPlm_InitProc();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Do Internal POR if any specific case */
	XPlm_PerformInternalPOR();

	/** Initialize the tasks lists */
	XPlmi_TaskInit();

END:
	return Status;
}
