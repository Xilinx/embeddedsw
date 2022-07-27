/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_io.h"

#include "xpm_common.h"
#include "xpm_regs.h"
#include "xpm_pmc.h"
#include "xpm_psfpdomain.h"
#include "xpm_pslpdomain.h"
#include "xpm_debug.h"
#include <stdarg.h>

/*****************************************************************************/
/**
 * @brief This function unlocks the NPI PCSR registers.
 *
 * @param BaseAddr		Base address of the device
 *
 *****************************************************************************/
XStatus XPm_UnlockPcsr(u32 BaseAddr)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	XPm_Out32(BaseAddr + NPI_PCSR_LOCK_OFFSET, PCSR_UNLOCK_VAL);
	/* Blind write check */
	PmChkRegOut32((BaseAddr + NPI_PCSR_LOCK_OFFSET), PCSR_UNLOCK_VAL, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_UNLOCK;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function locks the NPI PCSR registers.
 *
 * @param BaseAddr      Base address of the device
 *
 *****************************************************************************/
XStatus XPm_LockPcsr(u32 BaseAddr)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	XPm_Out32(BaseAddr + NPI_PCSR_LOCK_OFFSET, PCSR_LOCK_VAL);
	/* Blind write check */
	PmChkRegOut32((BaseAddr + NPI_PCSR_LOCK_OFFSET), PCSR_LOCK_VAL, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_LOCK;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
 *  This function is used to set/clear bits in any NPI PCSR
 *
 *  @param BaseAddress	BaseAddress of device
 *  @param Mask			Mask to be written into PCSR_MASK register
 *  @param Value		Value to be written into PCSR_CONTROL register
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *****************************************************************************/
XStatus XPm_PcsrWrite(u32 BaseAddress, u32 Mask, u32 Value)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	XPm_Out32((BaseAddress + NPI_PCSR_MASK_OFFSET), Mask);
	/* Blind write check */
	PmChkRegOut32((BaseAddress + NPI_PCSR_MASK_OFFSET), Mask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_MASK;
		goto done;
	}

	XPm_Out32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Value);
	/* Blind write check */
	PmChkRegMask32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Mask, Value, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_CONTROL;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
