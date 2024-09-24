/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_common.h"
#include "xpm_pldomain.h"
#include "xpm_regs.h"
#include "xpm_debug.h"


static u32 SavedCfuDivider = 0U;

/*****************************************************************************/
/**
 * @brief This function reduces the CFU clock frequency by dividing by 2
 *
 * @param       None
 *
 * @return      XST_SUCCESS on successful operation.
 *
 * @note	This function checks the MSB of CFU divisor register value. If MSB is set,
 * 		(meaning left shifting would cause the overflow) then it does not reduce
 * 		the CFU frequency and return with XST_SUCCESS;
 *		otherwise, it reduces the clock frequency by 2 by left shifting one bit.
 *		Also, calling this function twice consecutively cannot cause the reducing the
 *		CFU clock frequency by 4. Only the first function call works.
 *
 *****************************************************************************/
XStatus ReduceCfuClkFreq(void)
{
	XStatus Status = XST_FAILURE;
	u32 CfuDivider;

	/* Get current CFU CLK divider value */
	CfuDivider = XPm_In32(CRP_BASEADDR + CRP_CFU_REF_CTRL_OFFSET) \
		& CRP_CFU_REF_CTRL_DIVISOR0_MASK;

	/*
	 * Check if CFU clk divider is already double of the saved value,
	 * meaning this function has been called twice.
	 */
	if (((SavedCfuDivider << 1U) == CfuDivider)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check MSB; if it is set, then left shifting would overflow */
	if (0U != (CfuDivider & CRP_CFU_REF_CTRL_DIVISOR0_MASK_MSB)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/*
	 * Stored the value of divider for restoring the clock frequency.
	 * This SavedCfuDivider should be 0 when clock frequency is restored.
	 */
	SavedCfuDivider = CfuDivider;

	/* Write clock freq divided by 2 */
	CfuDivider <<= 1U;

	/* Write clock freq */
	XSECURE_REDUNDANT_IMPL(XPm_RMW32, CRP_BASEADDR + CRP_CFU_REF_CTRL_OFFSET, \
		CRP_CFU_REF_CTRL_DIVISOR0_MASK, CfuDivider);
	Status = XST_SUCCESS;

done:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function restores the CFU clock frequency
 *
 * @param       None
 *
 * @return      XST_SUCCESS on successful operation
 * 		XST_FAILURE on incorrect clock divider states
 *
 * @note	This function, instead of right shifting one bit,
 *		utilizes a stored variable to restore the original clock frequency back.
 *		It also checks the exisitng CFU clk setting to make sure
 *		the frequency is as expected, and returns an error if not.
 *
 *****************************************************************************/
XStatus RestoreCfuClkFreq(void)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Get current CFU CLK divider value */
	u32 CfuDivider = (XPm_In32(CRP_BASEADDR + CRP_CFU_REF_CTRL_OFFSET) \
		& CRP_CFU_REF_CTRL_DIVISOR0_MASK);

	/* Check if CFU clk frequency is already restored,
	 * SavedCfuDivider can also be 0 due to the overflow in ReduceCfuClkFreq() */
	if (0U == SavedCfuDivider) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check if CFU clk divider is at expected (double) value */
	if ((SavedCfuDivider << 1U) != CfuDivider) {
		PmErr("[Error]Cfu clock divider value is unxpected. Expect: %d \
			Actual: %d.\n\r", SavedCfuDivider << 1U, CfuDivider);
		Status = XST_FAILURE;
		DbgErr = XPM_INT_ERR_CFU_CLK_DIVIDER;
		goto done;
	}

	/* Write saved CFU clock */
	XSECURE_REDUNDANT_IMPL(XPm_RMW32, CRP_BASEADDR + CRP_CFU_REF_CTRL_OFFSET, \
		CRP_CFU_REF_CTRL_DIVISOR0_MASK, SavedCfuDivider);

	/* Reset SavedCfuDivider back as 0 */
	SavedCfuDivider = 0U;
	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
