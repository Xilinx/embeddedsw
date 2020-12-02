/******************************************************************************
* Copyright (C) 2011 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xiomodule_extra.c
* @addtogroup iomodule_v2_9
* @{
*
* The implementation of the XIOModule component's advanced discrete
* functions. See xiomodule.h for more information about the component.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sa   07/15/11 First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xiomodule.h"
#include "xiomodule_i.h"
#include "xil_types.h"
#include "xil_assert.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/


/****************************************************************************/
/**
* Set output discrete(s) to logic 1 for the specified GPO channel.
*
* @param	InstancePtr is a pointer to an XIOModule instance to be
*               worked on.
* @param	Channel contains the channel of the GPIO (1, 2, 3 or 4) to
*               operate on.
* @param	Mask is the set of bits that will be set to 1 in the discrete
*		data register. All other bits in the data register are
*		unaffected.
*
* @return	None.
*
*****************************************************************************/
void XIOModule_DiscreteSet(XIOModule * InstancePtr, unsigned Channel, u32 Mask)
{
	u32 Current;
	unsigned DataOffset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Channel >= 1) && (Channel <= XGPO_DEVICE_COUNT));

	/*
	 * Calculate the offset to the data register of the GPO
	 */
	DataOffset = ((Channel - 1) * XGPO_CHAN_OFFSET) + XGPO_DATA_OFFSET;

	/*
	 * Read the contents from the instance, merge in Mask and write
	 * back results
	 */
	Current = InstancePtr->GpoValue[Channel - 1];
	Current |= Mask;
	XIOModule_WriteReg(InstancePtr->BaseAddress, DataOffset, Current);
	InstancePtr->GpoValue[Channel - 1] = Current;
}


/****************************************************************************/
/**
* Set output discrete(s) to logic 0 for the specified GPO channel.
*
* @param	InstancePtr is a pointer to an XIOModule instance to be
*               worked on.
* @param	Channel contains the channel of the GPIO (1, 2, 3 or 4) to
*               operate on.
* @param	Mask is the set of bits that will be set to 0 in the discrete
*		data register. All other bits in the data register are
*		unaffected.
*
* @return	None.
*
*****************************************************************************/
void XIOModule_DiscreteClear(XIOModule * InstancePtr,
			     unsigned Channel,
			     u32 Mask)
{
	u32 Current;
	unsigned DataOffset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((Channel >= 1) && (Channel <= XGPO_DEVICE_COUNT));

	/*
	 * Calculate the offset to the data register of the GPO
	 */
	DataOffset = ((Channel - 1) * XGPO_CHAN_OFFSET) + XGPO_DATA_OFFSET;

	/*
	 * Read the contents from the instance, merge in Mask and write
	 * back results
	 */
	Current = InstancePtr->GpoValue[Channel - 1];
	Current &= ~Mask;
	XIOModule_WriteReg(InstancePtr->BaseAddress, DataOffset, Current);
	InstancePtr->GpoValue[Channel - 1] = Current;
}
/** @} */
