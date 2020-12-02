/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpfw_default.h"
#include "xpfw_config.h"
#include "xpfw_module.h"
#include "xpfw_core.h"
#include "xpfw_mod_rpu.h"

#ifdef ENABLE_RPU_RUN_MODE
const XPfw_Module_t *RpuModPtr;

/****************************************************************************/
/**
 * @brief  This scheduler task monitors RPU Run mode.
 *
 * @param  None.
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
static void XPfw_RpuRunModeChk(void)
{
	u32 RpuPwrSts;
	u32 RpuRunSts;
	u32 RpuCfgSts;

	/* Read RPU_0 Power state, RUN mode and RPU halt state */
	RpuPwrSts = Xil_In32(PMU_GLOBAL_PWR_STATE);
	RpuRunSts = Xil_In32(RPU_0_STATUS_REG);
	RpuCfgSts = Xil_In32(RPU_0_CFG_REG);

	if(((RpuPwrSts & RPU_POWER_UP_MASK) != RPU_POWER_UP_MASK) ||
			((RpuRunSts & RUN_MODE_MASK) != RUN_MODE_MASK) ||
			((RpuCfgSts & RPU_HALT_MASK) != RPU_HALT_MASK)) {

		/* RPU is not in RUN mode, Trigger Error Manager action */
		XPfw_Printf(DEBUG_ERROR,"RPU not in Run mode, failed. \n");

		/* Write error occurrence to PERS register and trigger FW Error1 */
		XPfw_RMW32(PMU_GLOBAL_PERS_GLOB_GEN_STORAGE5, RPU_RUN_MODE_ERROR,
											RPU_RUN_MODE_ERROR);
		XPfw_RMW32(PMU_LOCAL_PMU_SERV_ERR, PMU_LOCAL_PMU_SERV_ERR_FWERR1_MASK,
									PMU_LOCAL_PMU_SERV_ERR_FWERR1_MASK);
		XPfw_RMW32(PMU_LOCAL_PMU_SERV_ERR, PMU_LOCAL_PMU_SERV_ERR_FWERR1_MASK,
																	0x0U);
	}
}

/****************************************************************************/
/**
 * @brief  This scheduler task checks for STL triggered from RPU. If yes, adds
 * 		   RPU monitoring task to the scheduler
 *
 * @param  None.
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
static void CheckStlConfig(void)
{
	s32 Status;
	u32 StlStatus = XPfw_Read32(PMU_GLOBAL_GLOBAL_GEN_STORAGE5);

	/* If Bit 29 in PMU_GLOBAL_GEN_STORAGE5 is set
	 * Start RPU run mode check
	 */
	if ((StlStatus & STL_STARTED) == STL_STARTED) {
		Status = XPfw_CoreRemoveTask(RpuModPtr, CHECK_STL_STARTED,
											CheckStlConfig);
		if(XST_FAILURE == Status) {
			XPfw_Printf(DEBUG_ERROR,"RPU (MOD-%d):Removing Check STL cfg failed.",
							RpuModPtr->ModId);
		}

		/* Received STL command, start RPU Run mode task */
		Status = XPfw_CoreScheduleTask(RpuModPtr, XPFW_RPU_RUNMODE_TIME,
													XPfw_RpuRunModeChk);
		if (XST_FAILURE == Status) {
			XPfw_Printf(DEBUG_ERROR,"RPU (MOD-%d):Scheduling RPU run mode check failed.",
								RpuModPtr->ModId);
		}
	}
}

/****************************************************************************/
/**
 * @brief  This function schedules STL start check task.
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
static void RpuRunCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	s32 Status;

	Status = XPfw_CoreScheduleTask(ModPtr, CHECK_STL_STARTED, CheckStlConfig);
	if (XST_FAILURE == Status) {
		XPfw_Printf(DEBUG_ERROR,"RPU_(MOD-%d):Scheduling Check STL cfg failed.",
				ModPtr->ModId);
	}
}

/*
 * Create a Mod and assign the Handlers. We will call this function
 * from XPfw_UserStartup()
 */
void ModRpuInit(void)
{
	s32 Status;
	RpuModPtr = XPfw_CoreCreateMod();

	Status = XPfw_CoreSetCfgHandler(RpuModPtr, RpuRunCfgInit);
	if(XST_FAILURE == Status) {
		XPfw_Printf(DEBUG_ERROR,"RPU_(MOD-%d):Init failed.",
								RpuModPtr->ModId);
	}
}

#else /* ENABLE_RPU_RUN_MODE */
void ModRpuInit(void) { }
#endif /* ENABLE_RPU_RUN_MODE */
