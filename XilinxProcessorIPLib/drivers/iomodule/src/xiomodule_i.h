/******************************************************************************
* Copyright (C) 2011 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiomodule_i.h
* @addtogroup iomodule_v2_13
* @{
*
* This file contains data which is shared between files and internal to the
* XIOModule component. It is intended for internal use only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a sa   07/15/11 First release
* 1.02a sa   07/25/12 Added UART prototypes
* 2.11  mus  05/07/21  Fixed warnings reported by doxygen tool. It fixes
*                      CR#1088640.
* 2.12  mus  07/19/21 Fixed compilation warnings CR#1105405.
* </pre>
*
******************************************************************************/
/**
 *@cond nocomments
 */
#ifndef XIOMODULE_I_H		/* prevent circular inclusions */
#define XIOMODULE_I_H		/* by using protection macros */
/**
 *@endcond
 */
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xiomodule.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
*
* Update the statistics of the instance.
*
* @param	InstancePtr is a pointer to the XIOMOdule instance.
* @param	StatusRegister contains the contents of the UART status
*		register to update the statistics with.
*
* @return	None.
*
* @note
*
* Signature: void XIOModule_UpdateStats(XIOModule *InstancePtr,
*						u32 StatusRegister)
*
*****************************************************************************/
#define XIOModule_UpdateStats(InstancePtr, StatusRegister)		\
{									\
	if ((StatusRegister) & XUL_SR_OVERRUN_ERROR)			\
	{								\
		(InstancePtr)->Uart_Stats.ReceiveOverrunErrors++;	\
	}								\
	if ((StatusRegister) & XUL_SR_PARITY_ERROR)			\
	{								\
		(InstancePtr)->Uart_Stats.ReceiveParityErrors++;	\
	}								\
	if ((StatusRegister) & XUL_SR_FRAMING_ERROR)			\
	{								\
		(InstancePtr)->Uart_Stats.ReceiveFramingErrors++;	\
	}								\
}

/************************** Function Prototypes ******************************/

unsigned int XIOModule_SendBuffer(XIOModule *InstancePtr);
unsigned int XIOModule_ReceiveBuffer(XIOModule *InstancePtr);


/************************** Variable Definitions *****************************/

extern u32 XIOModule_BitPosMask[];

extern XIOModule_Config XIOModule_ConfigTable[];

#ifdef __cplusplus
}
#endif

#endif
/** @} */
