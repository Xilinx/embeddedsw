/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xtmr_manager_i.h
* @addtogroup Overview
* @{
*
* Contains data which is shared between the files of the XTMR_Manager component.
* It is intended for internal use only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
* 1.10  ml   25/09/26 Fixed doxygen 1.15.0 and compliance checker warnings.
* </pre>
*
*****************************************************************************/

#ifndef XTMR_MANAGER_I_H /* prevent circular inclusions */
/** @brief Include guard to prevent multiple inclusions. */
#define XTMR_MANAGER_I_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xtmr_manager.h"
#include "xtmr_manager_l.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/****************************************************************************/
/**
* @brief Update the statistics of the instance.
*
* @param[in] InstancePtr is a pointer to the XTMR_Manager instance.
* @param[in] FirstFailingRegister contains the contents of the first
*		failing register to update the statistics with.
*
* @note
*
* Signature: void XTMR_Manager_UpdateStats(XTMR_Manager *InstancePtr,
*						u32 FirstFailingRegister)
*
*****************************************************************************/
#define XTMR_Manager_UpdateStats(InstancePtr, FirstFailingRegister)	\
{									\
	if ((FirstFailingRegister) & XTM_FFR_REC)			\
	{								\
		(InstancePtr)->Stats.RecoveryCount++;			\
	}								\
}

/************************** Variable Definitions ****************************/

/* the configuration table */
extern XTMR_Manager_Config XTMR_Manager_ConfigTable[];

/************************** Function Prototypes *****************************/

#ifdef __cplusplus
}
#endif

#endif		/* end of protection macro */

/** @} */
