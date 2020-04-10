/******************************************************************************
* Copyright (C) 2002 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartlite_i.h
* @addtogroup uartlite_v3_4
* @{
*
* Contains data which is shared between the files of the XUartLite component.
* It is intended for internal use only.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/31/01 First release
* 1.00b jhl  02/21/02 Reparitioned the driver for smaller files
* 1.00b rpm  04/24/02 Moved register definitions to xuartlite_l.h and
*                     updated macro naming convention
* 2.00a ktn  10/20/09 The macros have been renamed to remove _m from
*		      the name. XUartLite_mClearStats macro is removed and
*		      XUartLite_ClearStats function should be used in its place.

* </pre>
*
*****************************************************************************/

#ifndef XUARTLITE_I_H /* prevent circular inclusions */
#define XUARTLITE_I_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xuartlite.h"
#include "xuartlite_l.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/****************************************************************************
*
* Update the statistics of the instance.
*
* @param	InstancePtr is a pointer to the XUartLite instance.
* @param	StatusRegister contains the contents of the UART status
*		register to update the statistics with.
*
* @return	None.
*
* @note
*
* Signature: void XUartLite_UpdateStats(XUartLite *InstancePtr,
*						u32 StatusRegister)
*
*****************************************************************************/
#define XUartLite_UpdateStats(InstancePtr, StatusRegister)	\
{								\
	if ((StatusRegister) & XUL_SR_OVERRUN_ERROR)		\
	{							\
		(InstancePtr)->Stats.ReceiveOverrunErrors++;	\
	}							\
	if ((StatusRegister) & XUL_SR_PARITY_ERROR)		\
	{							\
		(InstancePtr)->Stats.ReceiveParityErrors++;	\
	}							\
	if ((StatusRegister) & XUL_SR_FRAMING_ERROR)		\
	{							\
		(InstancePtr)->Stats.ReceiveFramingErrors++;	\
	}							\
}

/************************** Variable Definitions ****************************/

/* the configuration table */
extern XUartLite_Config XUartLite_ConfigTable[];

/************************** Function Prototypes *****************************/

unsigned int XUartLite_SendBuffer(XUartLite *InstancePtr);
unsigned int XUartLite_ReceiveBuffer(XUartLite *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif		/* end of protection macro */

/** @} */
