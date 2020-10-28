/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_sem_init.c
*
* This file contains the startup tasks related code for PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rm   09/22/2019 Initial release
* 1.01  kc   02/10/2020 Updated scheduler to add/remove tasks
*       kc   02/17/2020 Added configurable priority for scheduler tasks
*       kc   02/26/2020 Added XPLM_SEM macro to include/disable SEM
*                       functionality
*       kc   03/23/2020 Minor code cleanup
*       hb   10/29/2020 Updated OwnerId for NPI scan scheduler
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_sem_init.h"
#include "xplmi_scheduler.h"
#ifdef XPLM_SEM
#include "xilsem.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief This function call all the init functions of all xilsem
 * modules. As a part of init functions, modules can initiate scan on both cfi
 * npi, scan will be decided based on the CIPS params in xparameters.h.
 *
 * @param	Arg is not used
 *
 * @return	Status as defined in XilSem library
 *
 *****************************************************************************/
int XPlm_SemInit(void *Arg)
{
	int Status = XST_FAILURE;

	(void)Arg;
#ifdef XSEM_CFRSCAN_EN
	Status = XSem_CfrInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}
#endif

#ifdef XSEM_NPISCAN_EN
	Status = XSem_NpiInit();
	if (Status != XST_SUCCESS) {
		goto END;
	} else {
		Status = XPlmi_SchedulerAddTask(XPLMI_MODULE_SEM_ID,
				XSem_NpiRunScan, 100U, XPLM_TASK_PRIORITY_0);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}
#endif
	Status = XST_SUCCESS;

#if defined(XSEM_CFRSCAN_EN) || defined(XSEM_NPISCAN_EN)
END:
#endif
	return Status;
}
#endif
