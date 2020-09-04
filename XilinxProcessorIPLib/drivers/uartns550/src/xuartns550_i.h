/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartns550_i.h
* @addtogroup uartns550_v3_7
* @{
*
* This header file contains internal identifiers, which are those shared
* between the files of the driver. It is intended for internal use only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/16/01 First release
* 1.00b jhl  03/11/02 Repartitioned driver for smaller files.
* 1.11a sv   03/20/07 Updated to use the new coding guidelines.
* 2.00a ktn  10/20/09 Converted all register accesses to 32 bit access.
*		      Updated to use HAL Processor APIs. _m is removed from the
*		      name of all the macro definitions. XUartNs550_mClearStats
*		      macro is removed, XUartNs550_ClearStats function should be
*		      used in its place.
* </pre>
*
******************************************************************************/

#ifndef XUARTNS550_I_H /* prevent circular inclusions */
#define XUARTNS550_I_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xuartns550.h"

/************************** Constant Definitions *****************************/


/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/****************************************************************************
*
* This macro updates the status based upon a specified line status register
* value. The stats that are updated are based upon bits in this register. It
* also keeps the last errors instance variable updated. The purpose of this
* macro is to allow common processing between the modules of the component
* with less overhead than a function in the required module.
*
* @param	InstancePtr is a pointer to the XUartNs550 instance .
* @param	CurrentLsr contains the Line Status Register value to
*		be used for the update.
*
* @return 	None.
*
* @note 	C-Style signature:
*
* void XUartNs550_UpdateStats(XUartNs550 *InstancePtr, u8 CurrentLsr)
*
*****************************************************************************/
#define XUartNs550_UpdateStats(InstancePtr, CurrentLsr)	\
{								\
	InstancePtr->LastErrors |= CurrentLsr;			\
								\
	if (CurrentLsr & XUN_LSR_OVERRUN_ERROR) {		\
		InstancePtr->Stats.ReceiveOverrunErrors++;	\
	}							\
	if (CurrentLsr & XUN_LSR_PARITY_ERROR) {		\
		InstancePtr->Stats.ReceiveParityErrors++;	\
	}							\
	if (CurrentLsr & XUN_LSR_FRAMING_ERROR) {		\
		InstancePtr->Stats.ReceiveFramingErrors++;	\
	}							\
	if (CurrentLsr & XUN_LSR_BREAK_INT) {			\
		InstancePtr->Stats.ReceiveBreakDetected++;	\
	}							\
}

/************************** Function Prototypes ******************************/

int XUartNs550_SetBaudRate(XUartNs550 *InstancePtr, u32 BaudRate);

unsigned int XUartNs550_SendBuffer(XUartNs550 *InstancePtr);

unsigned int XUartNs550_ReceiveBuffer(XUartNs550 *InstancePtr);

/************************** Variable Definitions ****************************/

extern XUartNs550_Config XUartNs550_ConfigTable[];

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
