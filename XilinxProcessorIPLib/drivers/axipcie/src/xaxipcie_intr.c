/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************
**
* @file xaxipcie_intr.c
* @addtogroup axipcie_v3_1
* @{
*
* This file implements interrupt functions for the XAxiPcie IP
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.00a rkv  03/03/11  Original code.
* 2.02a nm   08/01/12  Updated for removing compilation errors with C++,
*		       changed XCOMPONENT_IS_READY to XIL_COMPONENT_IS_READY
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/
#include "xaxipcie.h"

/*************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

/*************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
* Enable the Global Interrupt.
*
* @param 	InstancePtr is the XAxiPcie instance to operate on.
*
* @return 	None
*
* @note 	This bit is in the Bridge Status and Control Register.
*
******************************************************************************/
void XAxiPcie_EnableGlobalInterrupt(XAxiPcie *InstancePtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XAXIPCIE_BSC_OFFSET);

	Data |= (XAXIPCIE_BSC_GI_MASK >> XAXIPCIE_BSC_GI_SHIFT);
	XAxiPcie_WriteReg(InstancePtr->Config.BaseAddress,
						XAXIPCIE_BSC_OFFSET, Data);

}

/*****************************************************************************/
/**
* Disable the Global Interrupt.
*
* @param 	InstancePtr is the XAxiPcie instance to operate on.
*
* @return 	None
*
* @note 	This bit is in the Bridge Status and Control Register.
*
******************************************************************************/
void XAxiPcie_DisableGlobalInterrupt(XAxiPcie *InstancePtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XAXIPCIE_BSC_OFFSET);

	Data &= ~(XAXIPCIE_BSC_GI_MASK >> XAXIPCIE_BSC_GI_SHIFT);

	XAxiPcie_WriteReg(InstancePtr->Config.BaseAddress,
						XAXIPCIE_BSC_OFFSET, Data);

}

/*****************************************************************************/
/**
* Enable the IP interrupt bits passed into "EnableMask".
*
* @param 	InstancePtr is the XAxiPcie instance to operate on.
* @param 	EnableMask is the bit pattern for interrupts wanted to be
*		enabled.
*
* @return 	None
*
* @note 	If an interrupt is already enabled before calling this
*		function, it will stay enabled regardless of the value of
*		"EnableMask" passed from the caller.
*
******************************************************************************/
void XAxiPcie_EnableInterrupts(XAxiPcie *InstancePtr, u32 EnableMask)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
						XAXIPCIE_IM_OFFSET);

	XAxiPcie_WriteReg(InstancePtr->Config.BaseAddress,
				XAXIPCIE_IM_OFFSET, (Data | EnableMask));

}

/*****************************************************************************/
/**
* Disable the IP interrupt bits passed into "DisableMask".
*
* @param 	InstancePtr is the XAxiPcie instance to operate on.
* @param 	DisableMask is the bit pattern for interrupts wanted to be
*		disabled.
*
* @return 	None
*
* @note 	If an interrupt is already disabled before calling this
*		function, it will stay disabled regardless of the value of
*		"DisableMask" passed from the caller.
*
******************************************************************************/
void XAxiPcie_DisableInterrupts(XAxiPcie *InstancePtr, u32 DisableMask)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XAXIPCIE_IM_OFFSET);

	XAxiPcie_WriteReg(InstancePtr->Config.BaseAddress,
				XAXIPCIE_IM_OFFSET, (Data & (~DisableMask)));

}

/*****************************************************************************/
/**
* Get the currently enabled interrupt bits of the IP and pass them back
* to the caller into "EnabledMask".
*
* @param 	InstancePtr is the XAxiPcie instance to operate on.
* @param 	EnabledMaskPtr is a pointer to a variable where the driver will
*        	pass back the enabled interrupt bits after reading them from IP.
*
* @return 	None.
*
* @note 	None.
*
******************************************************************************/
void XAxiPcie_GetEnabledInterrupts(XAxiPcie *InstancePtr, u32 *EnabledMaskPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(EnabledMaskPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	*EnabledMaskPtr = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XAXIPCIE_IM_OFFSET);

}

/*****************************************************************************/
/**
* Get the currently pending interrupt bits of the IP and pass them back
* to the caller into "PendingMask".
*
* @param 	InstancePtr is the XAxiPcie instance to operate on.
* @param 	PendingMaskPtr is a pointer to a variable where the driver will
* 		pass back the pending interrupt bits after reading them from IP.
*
* @return 	None.
*
* @note 	None.
*
******************************************************************************/
void XAxiPcie_GetPendingInterrupts(XAxiPcie *InstancePtr, u32 *PendingMaskPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(PendingMaskPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	*PendingMaskPtr = XAxiPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XAXIPCIE_ID_OFFSET);

}

/*****************************************************************************/
/**
* Clear the currently pending interrupt bits of the IP passed from
* the caller into "ClearMask".
*
* @param 	InstancePtr is the XAxiPcie instance to operate on.
* @param 	ClearMask is the bit pattern for pending interrupts wanted to
* 		be cleared.
*
* @return 	None.
*
* @note 	None.
*
******************************************************************************/
void XAxiPcie_ClearPendingInterrupts(XAxiPcie *InstancePtr, u32 ClearMask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	XAxiPcie_WriteReg((InstancePtr->Config.BaseAddress), XAXIPCIE_ID_OFFSET,
			XAxiPcie_ReadReg((InstancePtr->Config.BaseAddress),
				XAXIPCIE_ID_OFFSET) & (ClearMask));
}
/** @} */
