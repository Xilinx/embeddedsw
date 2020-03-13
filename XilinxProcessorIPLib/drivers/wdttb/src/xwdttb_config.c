/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc. All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xwdttb_config.c
* @addtogroup wdttb_v5_0
* @{
*
* Contains the required functions of the XWdtTb driver. See xwdttb.h for a
* description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 5.0	sne  11/19/19 First release
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xwdttb.h"
#include "xwdttb_config.h"
/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/**
*
* This function disables Window Watchdog Timer feature.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return
*		- XST_SUCESS, if Window WDT feature is disabled.
*		- XST_FAILURE, if Window WDT feature is not disabled. Refer
*		note section for the reasons.
*
* @note
*		- Disabling watchdog in first window duration is considered as
*		bade event. It can only be disabled in the second window
*		duration.
*		- If fail counter is enabled, watchdog can be disabled only
*		when fail counter is zero.
*
*****************************************************************************/
s32 XWdtTb_DisableWinWdt(XWdtTb *InstancePtr)
{
	s32 Status;
	u32 FailCounterVal;
	u32 SecWindow;
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->EnableWinMode == (u32)TRUE);

	/* Read enable status register and get second window value */
	SecWindow = (XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_ESR_OFFSET) & XWT_ESR_WSW_MASK) >>
			XWT_ESR_WSW_SHIFT;

	/* Check whether FC is enabled */
	if (InstancePtr->EnableFailCounter == (u32)XWT_ONE) {
		/* Read enable status register and get FC value */
		FailCounterVal = (XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
					XWT_ESR_OFFSET) & XWT_ESR_FCV_MASK) >>
					XWT_ESR_FCV_SHIFT;
		/* Check whether FC is zero and WDT is in second window */
		if ((FailCounterVal == (u32)XWT_ZERO) &&
				(SecWindow == (u32)XWT_ONE)) {
			/* Read enable status register and update WEN bit */
			RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
						  XWT_ESR_OFFSET) &
						  (~XWT_ESR_WEN_MASK);

			/* Set WSW bit to zero. It is RW1C bit */
			RegValue &= ~((u32)XWT_ESR_WSW_MASK);

			/*
			 * Write enable status register with updated WEN and
			 * WSW value
			 */
			XWdtTb_WriteReg(InstancePtr->Config.BaseAddr,
					XWT_ESR_OFFSET, RegValue);

			InstancePtr->IsStarted = (u32)0U;
			Status = (s32)XST_SUCCESS;
		} else {
			Status = (s32)XST_FAILURE;
		}
	}
	/* Check whether watchdog in second window */
	else if (SecWindow == (u32)XWT_ONE) {
		/* Read enable status register and update WEN bit */
		RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
					  XWT_ESR_OFFSET) &
					  (~XWT_ESR_WEN_MASK);

		/* Set WSW bit to zero. It is RW1C bit */
		RegValue &= ~((u32)XWT_ESR_WSW_MASK);

		/*
		 * Write enable status register with updated WEN and WSW
		 * value
		 */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET,
				RegValue);

		InstancePtr->IsStarted = (u32)0U;
		Status = (s32)XST_SUCCESS;
	} else {
		Status = (s32)XST_FAILURE;
	}

	return Status;
}
/*****************************************************************************/
/**
*
* This function enables Timebase Watchdog Timer feature.
*
* @param        InstancePtr is a pointer to the XWdtTb instance to be
*               worked on.
*
* @return       None.
*
* @note         This will Start the Timebase Watchdog timer.
*
*****************************************************************************/
void XWdtTb_EnableTimebaseWdt(XWdtTb *InstancePtr)
{
	u32 ControlStatusRegister0;
	/*
	 * Read the current contents of TCSR0 so that subsequent writes
	 * to the register won't destroy any other bits
	 */
	ControlStatusRegister0 =
		XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			       XWT_TWCSR0_OFFSET);
	/*
	 * Clear the bit that indicates the reason for the last
	 * system reset, WRS and the WDS bit, if set, by writing
	 * 1's to TCSR0
	 */
	ControlStatusRegister0 |= ((u32)XWT_CSR0_WRS_MASK |
				   (u32)XWT_CSR0_WDS_MASK);
	/* Indicate that the device is started before we enable it */
	InstancePtr->IsStarted = XIL_COMPONENT_IS_STARTED;
	/*
	 * Set the registers to enable the watchdog timer, both enable
	 * bits in TCSR0 and TCSR1 need to be set to enable it
	 */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr,
			XWT_TWCSR0_OFFSET, (ControlStatusRegister0 |
			(u32)XWT_CSR0_EWDT1_MASK));
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr,
			XWT_TWCSR1_OFFSET, XWT_CSRX_EWDT2_MASK);
}
/*****************************************************************************/
/**
*
* This function Disable Timebase Watchdog Timer feature.
*
* @param        InstancePtr is a pointer to the XWdtTb instance to be
*               worked on.
*
* @return
*               - XST_SUCESS, if  Timebase  WDT feature is disabled.
*               - XST_FAILURE, if Timebase  WDT feature is not disabled.
*
* @note         This will Disable Timebase Watchdog Timer.
*
*****************************************************************************/
s32 XWdtTb_DisableTimebaseWdt(XWdtTb *InstancePtr)
{
	u32 ControlStatusRegister0;
	s32 Status;
	/*
	 * Check if the disable of the watchdog timer is possible by
	 * writing a 0 to TCSR1 to clear the 2nd enable. If the Enable
	 * does not clear in TCSR0, the watchdog cannot be disabled.
	 * Return a NO_FEATURE to indicate this.
	 */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_TWCSR1_OFFSET,
			(u32)0U);
	/*
	 * Read the contents of TCSR0 so that the writes to the
	 * register that follow are not destructive to other bits and
	 * to check if the second enable was set to zero.
	 */
	ControlStatusRegister0 =
		XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			       XWT_TWCSR0_OFFSET);
	/*
	 * If the second enable was not set to zero, the feature is not
	 * allowed in the hardware. Return with NO_FEATURE status
	 */
	if ((ControlStatusRegister0 & XWT_CSRX_EWDT2_MASK) != (u32)XWT_ZERO) {
		Status = (s32)XST_NO_FEATURE;
	} else {
		/*
		 * Disable the watchdog timer by performing 2 writes, 1st to
		 * TCSR0 to clear the enable 1 and then to TCSR1 to clear the
		 * 2nd enable.
		 */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr,
				XWT_TWCSR0_OFFSET, (ControlStatusRegister0 &
				~((u32)XWT_CSR0_EWDT1_MASK)));
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr,
				XWT_TWCSR1_OFFSET, 0U);
		InstancePtr->IsStarted = (u32)0U;
		Status = (s32)XST_SUCCESS;
	}
	return Status;
}
/** @} */
