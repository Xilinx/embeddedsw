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
* @file xiomodule_extra.c
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
