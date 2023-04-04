/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/xplmi_wdt.c
*
* This file contains the PLMI WDT functionality related code for versal.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/28/2020 Initial release
*       bm   10/14/2020 Code clean up
* 1.01  td   07/08/2021 Fix doxygen warnings
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  08/13/2021 Code clean up
* 1.02  bm   07/06/2022 Refactor versal and versal_net code
* 1.03  ng   11/11/2022 Fixed doxygen file name error
*       bm   01/14/2023 Remove bypassing of PLM Set Alive during boot
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_wdt.h"
#include "xplmi_hw.h"
#include "xplmi_status.h"
#include "xplmi_debug.h"
#include "xplmi.h"

/**@cond xplmi_internal
 * @{
 */
/************************** Constant Definitions *****************************/
#define XPLMI_WDT_PERIODICITY		(100U)
#define XPLMI_SCHEDULER_PERIOD		(10U)

#define XPLMI_WDT_PERIODICITY_MIN	(15U)
#define XPLMI_WDT_PERIODICITY_MAX	(1000U)

#define XPLMI_MIO_NUM_PER_BANK		(26U)

#define XPLMI_PM_STMIC_LMIO_0		(0x14104001U)
#define XPLMI_PM_STMIC_LMIO_25		(0x1410401aU)
#define XPLMI_PM_STMIC_PMIO_0		(0x1410801bU)
#define XPLMI_PM_STMIC_PMIO_51		(0x1410804eU)

/**************************** Type Definitions *******************************/
typedef struct {
	u8 PlmLiveStatus; /**< PLM sets this bit to indicate it is alive */
	u8 IsEnabled; /**< Used to indicate if WDT is enabled or not */
	u32 Periodicity; /**< WDT period at which PLM should set the
			   live status */
	u32 GpioAddr; /**< GPIO address corresponding to MIO used for WDT */
	u32 GpioMask; /**< GPIO Mask corresponding to MIO used for WDT */
} XPlmi_Wdt;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XPlmi_Wdt WdtInstance = {
	.PlmLiveStatus = (u8)FALSE,
	.IsEnabled = (u8)FALSE,
	.Periodicity = XPLMI_WDT_PERIODICITY,
	.GpioAddr = 0U,
	.GpioMask = 0U
};

/**
 * @}
 * @endcond
 */

/*****************************************************************************/
/**
 * @brief	This function enables the WDT and sets NodeId and periodicity.
 *			It also verifies the parameters.
 *
 * @param	NodeId NodeId is the MIO node to be used by PLM for toggling.
 * @param	Periodicity at which MIO value should be toggled.
 *
 * @return
 * 			- XST_SUCCESS if success.
 * 			- XPLMI_ERR_WDT_PERIODICITY on invalid Periodicity.
 * 			- XPLMI_ERR_WDT_LPD_NOT_INITIALIZED if LPD is not initialized.
 * 			- XPLMI_ERR_WDT_NODE_ID on invalid node ID.
 *
 *****************************************************************************/
int XPlmi_EnableWdt(u32 NodeId, u32 Periodicity)
{
	int Status = XST_FAILURE;
	u32 MioNum;

	if ((Periodicity < XPLMI_WDT_PERIODICITY_MIN) ||
	    (Periodicity > XPLMI_WDT_PERIODICITY_MAX)) {
		Status = (int)XPLMI_ERR_WDT_PERIODICITY;
		goto END;
	}

	/** - Check for Valid Node ID */
	if ((NodeId >= XPLMI_PM_STMIC_LMIO_0) &&
	    (NodeId <= XPLMI_PM_STMIC_LMIO_25)) {
		/* LPD MIO is used */
		if (XPlmi_IsLpdInitialized() != (u8)TRUE) {
			Status = (int)XPLMI_ERR_WDT_LPD_NOT_INITIALIZED;
			goto END;
		}
		XPlmi_SetLpdInitialized(LPD_WDT_INITIALIZED);
		MioNum = NodeId - XPLMI_PM_STMIC_LMIO_0;
		WdtInstance.GpioAddr = PS_GPIO_DATA_0_OFFSET;
		WdtInstance.GpioMask = (u32)(1U) << MioNum;
	}
	else if ((NodeId >= XPLMI_PM_STMIC_PMIO_0) &&
		 (NodeId <= XPLMI_PM_STMIC_PMIO_51)) {
		/* PMC MIO used */
		MioNum = NodeId - XPLMI_PM_STMIC_PMIO_0;
		if (MioNum < XPLMI_MIO_NUM_PER_BANK) {
			WdtInstance.GpioAddr = PMC_GPIO_DATA_0_OFFSET;
		}
		else {
			WdtInstance.GpioAddr = PMC_GPIO_DATA_1_OFFSET;
		}
		WdtInstance.GpioMask = (u32)(1U) <<
					(MioNum % XPLMI_MIO_NUM_PER_BANK);
	}
	else {
		Status = (int)XPLMI_ERR_WDT_NODE_ID;
		goto END;
	}

	WdtInstance.Periodicity = Periodicity;
	WdtInstance.IsEnabled = (u8)TRUE;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function disables the WDT. This is required when LPD is
 * 			powered down and if LPD MIO is used. Also required when	debugging.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_DisableWdt(u32 NodeId)
{
	(void)NodeId;
	WdtInstance.IsEnabled = (u8)FALSE;
}

/*****************************************************************************/
/**
 * @brief	This function Sets the PLM Status.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_SetPlmLiveStatus(void)
{
	WdtInstance.PlmLiveStatus = (u8)TRUE;
}

/*****************************************************************************/
/**
 * @brief	This function clears the PLM status.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_ClearPlmLiveStatus(void)
{
	WdtInstance.PlmLiveStatus = (u8)FALSE;
}

/*****************************************************************************/
/**
 * @brief	This function is handler for WDT. Scheduler calls this function
 * 			periodically to check the PLM Live status and to toggle the MIO.
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_WdtHandler(void)
{
	static u32 WdtLastResetPeriod = 0U;

	/** - if WDT is not enabled, just return */
	if (WdtInstance.IsEnabled == (u8)FALSE) {
		goto END;
	}

	WdtLastResetPeriod += XPLMI_SCHEDULER_PERIOD;

	/** - Toggle MIO only when last reset period exceeds periodicity */
	if (WdtLastResetPeriod >
	    (u32)(WdtInstance.Periodicity - XPLMI_WDT_PERIODICITY_MIN)) {
		if (WdtInstance.PlmLiveStatus == (u8)TRUE) {
			XPlmi_Out32(WdtInstance.GpioAddr,
			XPlmi_In32(WdtInstance.GpioAddr) ^ WdtInstance.GpioMask);
			WdtInstance.PlmLiveStatus = (u8)FALSE;
		}
		WdtLastResetPeriod = 0U;
	}

END:
	return;
}
