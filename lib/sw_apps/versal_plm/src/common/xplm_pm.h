/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_pm.h
*
* This file contains the header functions of wrapper xilpm
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/20/2018 Initial release
* 1.01  rp   08/08/2019 Added code to send PM notify callback through IPI
* 1.02  kc   03/23/2020 Minor code cleanup
* 1.03  bm   02/08/2021 Renamed PlmCdo to PmcCdo
*       skd  03/16/2021 Added code to monitor if psm is alive or not
*       rama 03/22/2021 Added hook for STL periodic execution and
*                       FTTI configuration support for keep alive task
* 1.04  td   07/08/2021 Fix doxygen warnings
*       bsv  08/13/2021 Removed unwanted header file
*       ma   07/29/2022 Replaced XPAR_XIPIPSU_0_DEVICE_ID macro with
*                       XPLMI_IPI_DEVICE_ID
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLM_PM_H
#define XPLM_PM_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/
#ifdef XPLMI_IPI_DEVICE_ID
#define XPLM_PSM_HEALTH_CHK				(0xCU)
#define XPLM_PSM_ALIVE_COUNTER_ADDR		(0xF20140C8U)
#define XPLM_PSM_API_KEEP_ALIVE			(5U)
#define XPLM_MIN_FTTI_TIME				(10U)
#define XPLM_DEFAULT_FTTI_TIME			(100U)
#define XPLM_PSM_ALIVE_NOT_STARTED		(0U)
#define XPLM_PSM_ALIVE_STARTED			(1U)
#define XPLM_PSM_ALIVE_ERR				(2U)
#define XPLM_PSM_ALIVE_REMOVE_TASK_ERR	(3U)
#define XPLM_PSM_ALIVE_RETURN           (4U)
#define XPLM_PSM_COUNTER_CLEAR			(0U)
#define XPLM_PSM_COUNTER_INCREMENT		(1U)
#define XPLM_PSM_COUNTER_RETURN			(2U)
#endif /* XPLMI_IPI_DEVICE_ID */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

int XPlm_PmInit(void);
int XPlm_ProcessPmcCdo(void *Arg);
#ifdef XPLMI_IPI_DEVICE_ID
int XPlm_CreateKeepAliveTask(void *PtrMilliSeconds);
int XPlm_RemoveKeepAliveTask(void);
#endif /* XPLMI_IPI_DEVICE_ID */

#ifdef __cplusplus
}
#endif

#endif  /* XPLM_PM_H */
