/******************************************************************************
* Copyright (C) 2002 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xwdttb_selftest.c
* @addtogroup wdttb Overview
* @{
*
* The xwdttb_selftest.c file contains diagnostic self-test functions for the
* XWdtTb component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00b jhl  02/06/02 First release
* 1.10b mta  03/23/07 Updated to new coding style
* 2.00a ktn  10/22/09 Updated to use the HAL processor APIs/macros.
* 4.0   sha  12/17/15 Adherence to coding and Doxygen guidelines.
*                     Changed int -> u32.
*                     Update self-test with window WDT feature.
*                     Removed included xil_types, xil_io and xil_assert header
*                     files.
*                     Adherence to MISRA-C guidelines.
* 4.0   sha  01/29/16 Added following macros for Window WDT feature:
*                     XWT_FW_COUNT, XWT_SW_COUNT.
* 4.3   srm  01/30/18 Added doxygen tags
* 4.4   sne  03/01/19 Fixed violations according to MISRAC-2012 standards
*                     modified the code for below violations,
*                     No brackets to then/else,
*                     Literal value requires a U suffix,Function return
*                     type inconsistent,Logical conjunctions need brackets,
*                     Declared the pointer param as Pointer to const,
*                     Procedure has more than one exit point.
* 4.4   sne  03/04/19 Added Support for Versal.
* 4.5	sne  09/27/19 Updated driver to support WWDT and AXI Timebase WDT.
* 5.0	sne  01/31/20 Removed compare value registers write while
*		      configuring Generic Watchdog window.
* 5.0	sne  02/27/20 Reorganize the driver source.
* 5.0	sne  03/09/20 Fixed MISRA-C violations.
* 5.7	sb   07/12/23 Added support for system device-tree flow.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xwdttb.h"

/************************** Constant Definitions *****************************/
#define XWT_MAX_SELFTEST_LOOP_COUNT	0x00010000U /**< Loopt count */
#define XWT_FW_COUNT			0x0U	    /**< Fist Window count */
#define XWT_SW_COUNT			0x10000U    /**< Second window count */
#define XWT_GWOR_COUNT			0x00001000U /**< Generic Watchdog
						     * Offset Regiser */
/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* @brief
*
* This function runs a self-test on the timebase or window if enabled. Timebase
* test verifies that the timebase is incrementing. The watchdog timer is not
* tested due to the time required to wait for the watchdog timer to expire. The
* time consumed by this test is dependent on the system clock and the
* configuration of the dividers in for the input clock of the timebase.
*
* Window test verifies that the windowing feature does not generate bad event
* after enabling window feature with first and second window count. It disables
* window feature immediately.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return
*		- XST_SUCCESS if self-test was successful.
*		- XST_WDTTB_TIMER_FAILED if the timebase is not incrementing.
*		- XST_FAILURE if self-test was failed.
*
* @note		None.
*
******************************************************************************/
s32 XWdtTb_SelfTest(const XWdtTb *InstancePtr)
{
	u32 LoopCount = 0;
	u32 TbrValue1;
	u32 TbrValue2;
	s32 Status;

	/*
	 * Assert to ensure the inputs are valid and the instance has been
	 * initialized
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Check whether Window WDT is enabled */
	if (InstancePtr->EnableWinMode == (u32)1) {
		/* Configure first window with zero count */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FWR_OFFSET,
				XWT_FW_COUNT);

		/* Configure second window count */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_SWR_OFFSET,
				XWT_SW_COUNT);

		/* Enable Window WDT */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET,
				XWT_ESR_WEN_MASK);
		TbrValue2 = (XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
					    XWT_ESR_OFFSET) & XWT_ESR_WEN_MASK);
		if (TbrValue2 == (u32)1U) {
			Status = (s32)XST_SUCCESS;
		} else {
			Status = (s32)XST_FAILURE;
			goto End;
		}

		/* Set writable mode */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_MWR_OFFSET,
				1);

		/* Read enable status register and update WEN bit */
		TbrValue1 = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
					   XWT_ESR_OFFSET) & (~XWT_ESR_WEN_MASK);

		/*
		 * Clear WSW bit. Otherwise writing 1 will generate restart
		 * kick. WSW is RW1C.
		 */
		TbrValue1 &= ~((u32)XWT_ESR_WSW_MASK);

		/* Disable Window WDT feature */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET,
				TbrValue1);
		/* Read enable status register and get last bad events */
		TbrValue2 = (XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
					    XWT_ESR_OFFSET) & XWT_ESR_LBE_MASK) >>
			    XWT_ESR_LBE_SHIFT;

		/* Compare last bad event */
		if (TbrValue2 == (u32)0) {
			Status = (s32)XST_SUCCESS;
		} else {
			Status = (s32)XST_FAILURE;
		}

	} else {
#ifndef SDT
		if (InstancePtr->Config.IsPl == (u32)0) {
#else
		if (!(strcmp(InstancePtr->Config.Name, "xlnx,versal-wwdt-1.0"))) {
#endif
			/* Write General Watchdog offset register for Generating interrupt */
			XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_GWOR_OFFSET, XWT_GWOR_COUNT);
			/*Enable GWEN bit for starting General Watchdog timer */
			XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_GWCSR_OFFSET, XWT_GWCSR_GWEN_MASK);
			TbrValue1 = (XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
						    XWT_GWCSR_OFFSET) & XWT_GWCSR_GWEN_MASK);
			if (TbrValue1 == (u32)1) {
				Status = (s32)XST_SUCCESS;
			} else {
				Status = (s32)XST_FAILURE;
				goto End;
			}
			/* Write General WDT Refresh register to restart the timer */
			XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_GWRR_OFFSET, 1U);
			/* Disable GWEN Register */
			XWdtTb_WriteReg(InstancePtr->Config.BaseAddr,
					XWT_GWCSR_OFFSET, (~(u32)XWT_GWCSR_GWEN_MASK));
			Status = (s32)XST_SUCCESS;
		} else {

			/*
			 * Read the timebase register twice to start the test
			 */
			TbrValue1 = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
						   XWT_TBR_OFFSET);
			TbrValue2 = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
						   XWT_TBR_OFFSET);

			/*
			 * Read the timebase register for a number of iterations or
			 * until it increments, which ever occurs first
			 */
			while ((LoopCount <= XWT_MAX_SELFTEST_LOOP_COUNT) &&
			       (TbrValue2 == TbrValue1)) {
				TbrValue2 =
					XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
						       XWT_TBR_OFFSET);
				LoopCount++;
			}

			/*
			 * If the timebase register changed the test is successful,
			 * otherwise it failed
			 */
			if (TbrValue2 != TbrValue1) {
				Status = (s32)XST_SUCCESS;
			} else {
				Status = (s32)XST_WDTTB_TIMER_FAILED;
			}
		}
	}
End:
	return Status;
}
/** @} */
