/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpfw_default.h"
#include "xpfw_mod_common.h"
#include "xpfw_config.h"
#include "xpfw_core.h"
#include "xpfw_error_manager.h"
#include "xpfw_restart.h"
#include "xpfw_mod_wdt.h"

#if defined(ENABLE_SCHEDULER) && (defined(ENABLE_EM) || defined(ENABLE_WDT) || defined(ENABLE_SECURE))

static const XPfw_Module_t *CommonModPtr;

/****************************************************************************/
/**
 * @brief  This scheduler task checks for FSBL execution completion and
 *         performs other module's operation which are dependent on this.
 *
 * @param  None.
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
static void CheckFsblCompletion(void)
{
	s32 Status;
	u32 FsblCompletionStatus = XPfw_Read32(PMU_GLOBAL_GLOBAL_GEN_STORAGE5);

	if (FSBL_COMPLETION == (FsblCompletionStatus & FSBL_COMPLETION)) {

#ifdef ENABLE_EM
		/* Clear previous PLL lock errors if any */
		XPfw_Write32(PMU_GLOBAL_ERROR_STATUS_2, PMU_GLOBAL_ERROR_STATUS_2_PLL_LOCK_MASK);

		/* Set PS Error Out action for PLL lock errors */
		if (XST_SUCCESS !=
				XPfw_EmSetAction(EM_ERR_ID_PLL_LOCK, EM_ACTION_PSERR, NULL)) {
			XPfw_Printf(DEBUG_DETAILED,"Common: Set error action for "
					"PLL Lock errors failed\r\n");
		}

		/*
		 * Once FSBL execution is completed, PMU need to enable the LPD/FPD WDT
		 * error and set the error action as FSBL disables while exiting.
		*/
		if (FSBL_RUNNING_ON_A53 ==
				(FsblCompletionStatus & FSBL_STATE_PROC_INFO_MASK)) {

			if (XST_SUCCESS != XPfw_EmSetAction(EM_ERR_ID_FPD_SWDT,
					SWDT_EM_ACTION, ErrorTable[EM_ERR_ID_FPD_SWDT].Handler)) {
				XPfw_Printf(DEBUG_DETAILED,"Common: Set error action for "
						"FPD WDT error failed\r\n");
			}

			if (XST_SUCCESS != XPfw_EmSetAction(EM_ERR_ID_LPD_SWDT,
					EM_ACTION_SRST, ErrorTable[EM_ERR_ID_LPD_SWDT].Handler)) {
				XPfw_Printf(DEBUG_DETAILED,"Common: Set error action for "
						"LPD WDT error failed\r\n");
			}
		} else {

			if (XST_SUCCESS != XPfw_EmSetAction(EM_ERR_ID_FPD_SWDT,
					EM_ACTION_SRST, ErrorTable[EM_ERR_ID_FPD_SWDT].Handler)) {
				XPfw_Printf(DEBUG_DETAILED,"Common: Set error action for "
						"FPD WDT error failed\r\n");
			}

			if (XST_SUCCESS != XPfw_EmSetAction(EM_ERR_ID_LPD_SWDT,
					SWDT_EM_ACTION, ErrorTable[EM_ERR_ID_LPD_SWDT].Handler)) {
				XPfw_Printf(DEBUG_DETAILED,"Common: Set error action for "
						"LPD WDT error failed\r\n");
			}
		}

		/* If ENABLE_RECOVERY is defined, PMU need to call this function and
		 * set FPD/LPD WDT error action accordingly after FSBL execution
		 * is completed.
		 */
		if ((u32)XST_SUCCESS == XPfw_RecoveryInit()) {
			/* This is to enable FPD/LPD WDT and enable recovery mechanism when
			* ENABLE_RECOVERY flag is defined.
			*/
		}
#endif

#if defined(USE_DDR_FOR_APU_RESTART) && defined(ENABLE_SECURE)
		/*
		 * Store FSBL to reserved DDR memory location.
		 */
		Status = XPfw_StoreFsblToDDR();
		if (XST_SUCCESS != Status) {
			XPfw_Printf(DEBUG_ERROR,"%s: Error! Storing FSBL for "
					"APU-only restart failed. APU-only warm-restart "
					"may not work\r\n", __func__);
		}
#endif

#ifdef ENABLE_WDT
		/*
		 * Initialization of PMU WDT
		 */
		InitCsuPmuWdt();
#endif

		Status = XPfw_CoreRemoveTask(CommonModPtr, (u32)CHECK_FSBL_COMPLETION,
				CheckFsblCompletion);
		if (XST_FAILURE == Status) {
			XPfw_Printf(DEBUG_ERROR,"Common (MOD-%d):Removing Common config task "
					"failed.", CommonModPtr->ModId);
		}
	}
}

/****************************************************************************/
/**
 * @brief  Module init which schedules a task for checking FSBL completion.
 *
 * @param  ModPtr Module pointer
 *         CfgData Module config data
 *         Len Length of config data
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
static void CommonCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	s32 Status;

	/*
	 * Schedule a task to check for FSBL completion and do the following:
	 * EM Module:
	 * 	- Clear PLL lock errors occurred during FSBL initialization
	 * 	- Enable PLL lock errors and set error action
	 * 	- Set error action for FPD WDT and LPD WDT errors
	 * WDT Module:
	 * 	- Call PMU WDT initialize function
	 * Common functionality:
	 * 	- If ENABLE_SECURE is defined, store the FSBL image to reserved
	 * 	  DDR memory location
	 */
	Status = XPfw_CoreScheduleTask(ModPtr, CHECK_FSBL_COMPLETION, CheckFsblCompletion);
	if (XST_FAILURE == Status) {
		XPfw_Printf(DEBUG_ERROR,"Common (MOD-%d):Scheduling Common Cfg task failed.",
				ModPtr->ModId);
	}
}

/****************************************************************************/
/**
 * @brief  Module init which schedules a task for
 *
 * @param  None.
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
/*
 * Create a Mod and assign the Handlers. We will call this function
 * from XPfw_UserStartup()
 */
void ModCommonInit(void)
{
	CommonModPtr = XPfw_CoreCreateMod();

	if (XST_SUCCESS != XPfw_CoreSetCfgHandler(CommonModPtr, CommonCfgInit)) {
		XPfw_Printf(DEBUG_DETAILED,"Common: Set Cfg handler failed\r\n");
	}
}
#else
void ModCommonInit(void) { }
#endif /* ENABLE_SCHEDULER */
