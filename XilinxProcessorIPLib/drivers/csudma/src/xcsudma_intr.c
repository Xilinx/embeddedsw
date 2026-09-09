/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcsudma_intr.c
* @addtogroup csuma_api CSUDMA APIs
* @{
*
* The xcsudma_intr.c file contains interrupt related functions of Xilinx CSU_DMA core.
* Refer to the header file xcsudma.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ---------------------------------------------------
* 1.0   vnsld  22/10/14  First release
* 1.11	sk     03/03/22  Replace driver version in addtogroup with Overview.
* 1.11	sk     03/03/22  Update Overview section based on review comments.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcsudma.h"

/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* Reads interrupt status read from Interrupt Status Register.
* Use the XCSUDMA_IXR_*_MASK constants defined in xcsudma_hw.h to interpret the
* returned value.
*
* @param[in] InstancePtr is a pointer to XCsuDma instance to be worked on.
* @param[in] Channel Channel to operate on (XCSUDMA_SRC_CHANNEL or XCSUDMA_DST_CHANNEL).
*
* @return	The pending interrupts of the CSU_DMA. Use th following masks
*		to interpret the returned value.
*		XCSUDMA_IXR_SRC_MASK   - For Source channel
*		XCSUDMA_IXR_DST_MASK   - For Destination channel
*
*
******************************************************************************/
u32 XCsuDma_IntrGetStatus(XCsuDma *InstancePtr, XCsuDma_Channel Channel)
{
	u32 Data;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
				(Channel == (XCSUDMA_DST_CHANNEL)));

	Data = XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
				(u32)(XCSUDMA_I_STS_OFFSET) +
			((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF)));

	/* Return interrupt status */
	return Data;

}

/*****************************************************************************/
/**
*
* Clears interrupts. Every bit set in Interrupt Status
* Register indicates that a specific type of interrupt is occurring, and this
* function clears one or more interrupts by writing a bit mask to Interrupt
* Clear Register.
*
* @param[in] InstancePtr is a pointer to XCsuDma instance to be worked on.
* @param[in] Channel Channel to operate on (XCSUDMA_SRC_CHANNEL or XCSUDMA_DST_CHANNEL).
* @param[in] Mask Bit mask of interrupts to clear, formed by OR'ing
*		XCSUDMA_IXR_* bits defined in xcsudma_hw.h.
*
*
******************************************************************************/
void XCsuDma_IntrClear(XCsuDma *InstancePtr, XCsuDma_Channel Channel, u32 Mask)
{


	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
				(Channel == (XCSUDMA_DST_CHANNEL)));
	/* Clears Interrupt in SRC/DST channels */
	if (Channel == (XCSUDMA_SRC_CHANNEL)) {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
			(u32)(XCSUDMA_I_STS_OFFSET),
				(Mask & (u32)(XCSUDMA_IXR_SRC_MASK)));
	}
	else {
		XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
			((u32)(XCSUDMA_I_STS_OFFSET) +
			((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))),
				(Mask & (u32)(XCSUDMA_IXR_DST_MASK)));
	}
}

/*****************************************************************************/
/**
*
* Enables interrupts. Use the XCSUDMA_IXR_*_MASK constants
* defined in xcsudma_hw.h to create the bit-mask to enable interrupts.
*
* @param[in] InstancePtr is a pointer to XCsuDma instance to be worked on.
* @param[in] Channel Channel to operate on (XCSUDMA_SRC_CHANNEL or XCSUDMA_DST_CHANNEL).
* @param[in] Mask Bit mask of interrupts to enable, formed by OR'ing
*		XCSUDMA_IXR_*_MASK bits defined in xcsudma_hw.h.
*
*
*
******************************************************************************/
void XCsuDma_EnableIntr(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
							u32 Mask)
{
	u32 Data;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
				(Channel == (XCSUDMA_DST_CHANNEL)));

	if (Channel == (XCSUDMA_SRC_CHANNEL)) {
		Data = Mask & (u32)(XCSUDMA_IXR_SRC_MASK);
	}
	else {
		Data = Mask & (u32)(XCSUDMA_IXR_DST_MASK);
	}
	/*
	 * Write the mask to the IER Register
	 */
	XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
		((u32)(XCSUDMA_I_EN_OFFSET) +
		((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))), Data);

}

/*****************************************************************************/
/**
*
* Disables interrupts. Use the XCSUDMA_IXR_*_MASK constants
* defined in xcsudma_hw.h to create the bit-mask to disable interrupts.
*
* @param[in] InstancePtr is a pointer to XCsuDma instance to be worked on.
* @param[in] Channel Channel to operate on (XCSUDMA_SRC_CHANNEL or XCSUDMA_DST_CHANNEL).
* @param[in] Mask Bit mask of interrupts to disable, formed by OR'ing
*		XCSUDMA_IXR_*_MASK bits defined in xcsudma_hw.h.
*
*
*
******************************************************************************/
void XCsuDma_DisableIntr(XCsuDma *InstancePtr, XCsuDma_Channel Channel,
								u32 Mask)
{
	u32 Data;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
			(Channel == (XCSUDMA_DST_CHANNEL)));

	if (Channel == XCSUDMA_SRC_CHANNEL) {
		Data = (Mask) & (u32)(XCSUDMA_IXR_SRC_MASK);
	}
	else {
		Data = (Mask) & (u32)(XCSUDMA_IXR_DST_MASK);
	}

	/*
	 * Write the mask to the IDR Register
	 */
	XCsuDma_WriteReg(InstancePtr->Config.BaseAddress,
		((u32)(XCSUDMA_I_DIS_OFFSET) +
		((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF))), Data);

}

/*****************************************************************************/
/**
*
* Reads the interrupt mask to know which interrupts are
* enabled and which of them were disabled.
*
* @param[in] InstancePtr is a pointer to XCsuDma instance to be worked on.
* @param[in] Channel Channel to operate on (XCSUDMA_SRC_CHANNEL or XCSUDMA_DST_CHANNEL).
*
* @return	The current interrupt mask.
*       The mask indicates which interrupts are enabled/disabled.
*		- 0 bit represents that the corresponding interrupt is enabled.
*		- 1 bit represents that the Corresponding interrupt is disabled.
*		To interpret returned mask use
*		- XCSUDMA_IXR_SRC_MASK that the For source channel
*		- XCSUDMA_IXR_DST_MASK that the For destination channel
*
*
******************************************************************************/
u32 XCsuDma_GetIntrMask(XCsuDma *InstancePtr, XCsuDma_Channel Channel)
{

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid((Channel == (XCSUDMA_SRC_CHANNEL)) ||
					(Channel == (XCSUDMA_DST_CHANNEL)));

	/*
	 * Read the Interrupt Mask register
	 */
	return (XCsuDma_ReadReg(InstancePtr->Config.BaseAddress,
			((u32)(XCSUDMA_I_MASK_OFFSET) +
			((u32)Channel * (u32)(XCSUDMA_OFFSET_DIFF)))));
}
/** @} */
