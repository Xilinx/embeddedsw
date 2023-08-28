/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_startup.c
*
* This file contains the startup tasks related code for PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/28/2018 Initial release
* 1.01  ma   08/01/2019 Removed LPD module init related code from PLM app
*       rm   09/08/2019 Adding xilsem library in place of source code
* 1.02  kc   02/26/2020 Added XPLM_SEM macro to include/disable SEM
*                       functionality
*       kc   03/23/2020 Minor code cleanup
* 1.03  bm   01/08/2021 Updated PmcCdo hook function name
*       rb   02/02/2021 Added XPLM_SEM macro to SEM header file
*       bm   02/08/2021 Renamed PlmCdo to PmcCdo
*       rb   03/09/2021 Updated Sem Scan Init API call
*       skd  03/16/2021 Added XPlm_CreateKeepAliveTask to task list
*                       for psm is alive feature
*       bm   04/03/2021 Updated StartupTaskList to be in line with the new
*                       TaskNode structure
* 1.04  td   07/08/2021 Fix doxygen warnings
*       ma   07/12/2021 Minor updates to StartupTaskList as per the new
*                       XPlmi_TaskNode structure
*       bsv  08/09/2021 Code clean up to reduce elf size
*       bsv  08/13/2021 Removed unwanted header files from xplm_startup.h
*       bm   09/07/2021 Merged pre-boot startup tasks into a single task
* 1.05  am   11/24/2021 Fixed doxygen warnings
*       ma   01/17/2022 Enable SLVERR for PLM related components
*       is   03/22/2022 Updated PMC XMPU/XPPUs IEN macros
* 1.06  bm   07/06/2022 Refactor versal and versal_net code
*       ma   07/13/2022 Fix bug in enabling SLVERR for RTC
*       ma   07/29/2022 Replaced XPAR_XIPIPSU_0_DEVICE_ID macro with
*                       XPLMI_IPI_DEVICE_ID
* 1.07  ng   11/11/2022 Updated doxygen comments
*       sk   01/11/2023 Added Image Store Config init function
*       dd   03/16/2023 Misra-C violation Rule 17.7 fixed
*       sk   08/28/2023 Added EAM error's check after processing
*                       of PMC cdo
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_startup.h"
#include "xplm_pm.h"
#include "xplm_module.h"
#include "xplm_loader.h"
#include "xplm_hooks.h"
#include "xplmi_task.h"
#include "xplm_plat.h"
#include "xplmi_err_common.h"
#ifdef XPLM_SEM
#include "xplm_sem_init.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef int (*TaskHandler)(void * PrivData);
typedef struct {
	TaskHandler Handler; /**< Task handler */
	void * PrivData; /**< Private data */
} StartupTaskHandler;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XPlm_PreBootTasks(void *Arg);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief This function call all the init functions of all the different
 * modules. As a part of init functions, modules can register the
 * command handlers, interrupt handlers with the interface layer.
 *
 * @return	Status as defined in xplmi_status.h
 *
 *****************************************************************************/
int XPlm_AddStartUpTasks(void)
{
	int Status = XST_FAILURE;
	u32 Index;
	XPlmi_TaskNode *Task;
#ifdef XPLMI_IPI_DEVICE_ID
	static u32 MilliSeconds = XPLM_DEFAULT_FTTI_TIME;
#endif /* XPLMI_IPI_DEVICE_ID */

	/**
	 * Add the following tasks to startup list.
     *  - PreBoot
     *  - Load Boot PDI
     *  - Hook after boot PDI
     *  - Keep alive task
     *  - SEM scan init
	 */
	const StartupTaskHandler StartUpTaskList[] = {
		{XPlm_PreBootTasks, NULL},
		{XPlm_LoadBootPdi, NULL},
		{XPlm_HookAfterBootPdi, NULL},
#ifdef XPLMI_IPI_DEVICE_ID
		{XPlm_CreateKeepAliveTask, (void *)&MilliSeconds},
#endif /* XPLMI_IPI_DEVICE_ID */
#ifdef XPLM_SEM
		{XPlm_SemScanInit, NULL},
#endif
	};

	/**
	 * Create the Tasks of the PLM from the Task List
	 */
	for (Index = 0U; Index < XPLMI_ARRAY_SIZE(StartUpTaskList); Index++) {
		Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_0,
			StartUpTaskList[Index].Handler,
			StartUpTaskList[Index].PrivData);
		if (Task == NULL) {
			Status = XPlmi_UpdateStatus(XPLM_ERR_TASK_CREATE, 0);
			goto END;
		}
		microblaze_disable_interrupts();
		XPlmi_TaskTriggerNow(Task);
		microblaze_enable_interrupts();
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function enables SLVERR for PMC related modules
*
* @return	None
*
*****************************************************************************/
static void XPlm_EnableSlaveErrors(void)
{
	/** Enable SLVERR for PMC_IOU_SLCR registers */
	XPlmi_Out32(PMC_IOU_SLCR_CTRL, XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/** Enable SLVERR for PMC_GLOBAL registers */
	XPlmi_UtilRMW(PMC_GLOBAL_BASEADDR,
			PMC_GLOBAL_GLOBAL_CNTRL_SLVERR_ENABLE_MASK,
			PMC_GLOBAL_GLOBAL_CNTRL_SLVERR_ENABLE_MASK);
	/** Enable SLVERR for PMC_IOU_SECURE_SLCR registers */
	XPlmi_Out32(PMC_IOU_SECURE_SLCR_CTRL, XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/** Enable SLVERR for PMC_RAM_CFG registers */
	XPlmi_UtilRMW(PMC_RAM_CFG_BASEADDR, XPLMI_SLAVE_ERROR_ENABLE_MASK,
			XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/** Enable SLVERR for PMC_ANALOG registers */
	XPlmi_Out32(PMC_ANALOG_SLVERR_CTRL, XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/** Enable SLVERR for PMC_TAP registers */
	XPlmi_Out32(PMC_TAP_SLVERR_CTRL, XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/** Enable SLVERR for AES registers */
	XPlmi_Out32(AES_SLV_ERR_CTRL, XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/** Enable SLVERR for BBRAM registers */
	XPlmi_Out32(BBRAM_CTRL_SLV_ERR_CTRL, XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/** Enable SLVERR for ECDSA_RSA registers */
	XPlmi_Out32(ECDSA_RSA_APB_SLV_ERR_CTRL, XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/** Enable SLVERR for SHA3 registers */
	XPlmi_Out32(SHA3_SHA_SLV_ERR_CTRL, XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/**
	 * Enable SLVERR for EFUSE registers. EFUSE registers need to be unlocked
	 * to enable writes to these registers
	 */
	XPlmi_Out32(EFUSE_CTRL_WR_LOCK, XPLMI_EFUSE_CTRL_UNLOCK_VAL);
	XPlmi_UtilRMW(EFUSE_CTRL_CFG, EFUSE_CTRL_CFG_SLVERR_ENABLE_MASK,
			EFUSE_CTRL_CFG_SLVERR_ENABLE_MASK);
	XPlmi_Out32(EFUSE_CTRL_WR_LOCK, XPLMI_EFUSE_CTRL_LOCK_VAL);
	/** Enable SLVERR for CRP registers */
	XPlmi_Out32(CRP_BASEADDR, XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/**
	 * Enable SLVERR for RTC registers
	 * Bit 31 in RTC_CONTROL is to disable the battery which is write-only.
	 * When writing to this register, bit 31 should also be set to 1.
	 */
	XPlmi_UtilRMW(RTC_CONTROL, RTC_CONTROL_SLVERR_EN_MASK,
			RTC_CONTROL_SLVERR_EN_MASK);
	/** Enable SLVERR for PMC_XMPU registers */
	XPlmi_Out32((PMC_XMPU_BASEADDR + XMPU_IEN), XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/** Enable SLVERR for PMC_XPPU_NPI registers */
	XPlmi_Out32((PMC_XPPU_NPI_BASEADDR + XPPU_IEN), XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/** Enable SLVERR for PMC_XPPU registers */
	XPlmi_Out32((PMC_XPPU_BASEADDR + XPPU_IEN), XPLMI_SLAVE_ERROR_ENABLE_MASK);
	/** Enable Platform specific Slave Errors */
	XPlm_EnablePlatformSlaveErrors();
}

/*****************************************************************************/
/**
* @brief	This function executes pre boot tasks
*
* @param	Arg is the argument passed to pre boot tasks
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
static int XPlm_PreBootTasks(void* Arg)
{
	int Status = XST_FAILURE;

	Status = XPlm_ModuleInit(Arg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Enable SLVERR's */
	XPlm_EnableSlaveErrors();

	Status = XPlm_HookBeforePmcCdo(Arg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlm_ProcessPmcCdo(Arg);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Check for any EAM errors after Processing PMC CDO*/
	(void)XPlmi_ErrorTaskHandler(NULL);

	if (XPlmi_IsPlmUpdateDone() == (u8)FALSE) {
		(void) XLoader_ReadImageStoreCfg();
	}

	Status = XPlm_HookAfterPmcCdo(Arg);

END:
	return Status;
}
