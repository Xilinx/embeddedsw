/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xtmr_manager_i.h
* @addtogroup tmr_manager_v1_0
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
