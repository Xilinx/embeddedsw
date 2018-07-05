/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_manager_i.h
* @addtogroup tmr_manager_v1_2
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
* </pre>
*
*****************************************************************************/

#ifndef XTMR_MANAGER_I_H /* prevent circular inclusions */
#define XTMR_MANAGER_I_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xtmr_manager.h"
#include "xtmr_manager_l.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/****************************************************************************
*
* Update the statistics of the instance.
*
* @param	InstancePtr is a pointer to the XTMR_Manager instance.
* @param	StatusRegister contains the contents of the core status
*		register to update the statistics with.
*
* @return	None.
*
* @note
*
* Signature: void XTMR_Manager_UpdateStats(XTMR_Manager *InstancePtr,
*						u32 StatusRegister)
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
