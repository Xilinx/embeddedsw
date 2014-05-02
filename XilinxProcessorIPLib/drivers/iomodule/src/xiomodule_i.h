/******************************************************************************
*
* (c) Copyright 2011-2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xiomodule_i.h
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
* </pre>
*
******************************************************************************/

#ifndef XIOMODULE_I_H		/* prevent circular inclusions */
#define XIOMODULE_I_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xiomodule.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************
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
