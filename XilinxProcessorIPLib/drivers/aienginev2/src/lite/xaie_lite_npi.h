/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_npi.h
* @{
*
* This header file defines a lightweight version of AIE NPI registers offsets
* and operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   09/06/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_NPI_H
#define XAIE_LITE_NPI_H

/************************** Constant Definitions *****************************/
#ifndef XAIE_NPI_BASEADDR
#define XAIE_NPI_BASEADDR		0xF70A0000
#endif

#define XAIE_NPI_TIMEOUT_US		40U

#define XAIE_NPI_PCSR_MASK_REG			0x00000000U
#define XAIE_NPI_PCSR_MASK_SHIM_RESET_MSK		0x08000000U
#define XAIE_NPI_PCSR_MASK_SHIM_RESET_LSB		27U

#define XAIE_NPI_PCSR_CONTROL_REG			0X00000004U
#define XAIE_NPI_PCSR_CONTROL_SHIM_RESET_MSK		0x08000000U
#define XAIE_NPI_PCSR_CONTROL_SHIM_RESET_LSB		27U

#define XAIE_NPI_PCSR_LOCK_REG				0X0000000CU
#define XAIE_NPI_PCSR_LOCK_STATE_UNLOCK_CODE		0xF9E8D7C6U

#define XAIE_NPI_PROT_REG_CNTR_REG			0x00000200U
#define XAIE_NPI_PROT_REG_CNTR_EN_MSK			0x00000001U
#define XAIE_NPI_PROT_REG_CNTR_EN_LSB			0U
#define XAIE_NPI_PROT_REG_CNTR_FIRSTCOL_MSK		0x000000FEU
#define XAIE_NPI_PROT_REG_CNTR_FIRSTCOL_LSB		1U
#define XAIE_NPI_PROT_REG_CNTR_LASTCOL_MSK		0x00007F00U
#define XAIE_NPI_PROT_REG_CNTR_LASTCOL_LSB		8U

#define XAIE_NPI_IRQ0_ENABLE_REG			0x00000038U
#define XAIE_NPI_PER_IRQ_REGOFF				0x10U

/***************************** Include Files *********************************/
#include "xaie_lite_io.h"

/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/

/*****************************************************************************/
/**
*
* This is function to set NPI lock
*
* @param	Lock : XAIE_ENABLE to lock, XAIE_DISABLE to unlock
*
* @note		This function is internal.
*******************************************************************************/
static inline void _XAie_LNpiSetLock(u8 Lock)
{
	u32 LockVal;

	if (Lock == XAIE_DISABLE) {
		LockVal = XAIE_NPI_PCSR_LOCK_STATE_UNLOCK_CODE;
	} else {
		LockVal = 0;
	}

	_XAie_LNpiWriteCheck32(XAIE_NPI_PCSR_LOCK_REG, LockVal);
}


/*****************************************************************************/
/**
*
* This is function to mask write to PCSR register
*
* @param	RegVal : Value to write to PCSR register
* @param	Mask : Mask to write to PCSR register
*
* @note		Sequence to write PCSR control register is as follows:
*		* unlock the PCSR register
*		* enable PCSR mask from mask register
*		* set the value to PCSR control register
*		* disable PCSR mask from mask register
*		* lock the PCSR register
*		This function is internal.
*******************************************************************************/
static inline void _XAie_LNpiWritePcsr(u32 RegVal, u32 Mask)
{
	_XAie_LNpiSetLock(XAIE_DISABLE);

	_XAie_LNpiWriteCheck32(XAIE_NPI_PCSR_MASK_REG, (RegVal & Mask));
	_XAie_LNpiWriteCheck32(XAIE_NPI_PCSR_CONTROL_REG, (RegVal & Mask));
	_XAie_LNpiWriteCheck32(XAIE_NPI_PCSR_MASK_REG, 0);

	_XAie_LNpiSetLock(XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This is the NPI function to set the SHIM set assert
*
* @param	DevInst : AI engine device pointer
* @param	RstEnable : XAIE_ENABLE to assert reset, and XAIE_DISABLE to
*			    deassert reset.
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		This function is internal.
*
*******************************************************************************/
static inline void _XAie_LNpiSetShimReset(u8 RstEnable)
{
	u32 RegVal;

	RegVal = _XAie_LSetRegField(RstEnable,
				XAIE_NPI_PCSR_CONTROL_SHIM_RESET_LSB,
				XAIE_NPI_PCSR_CONTROL_SHIM_RESET_MSK);

	_XAie_LNpiWritePcsr(RegVal, XAIE_NPI_PCSR_CONTROL_SHIM_RESET_MSK);
}

/*****************************************************************************/
/**
*
* This is the NPI function to set the SHIM set assert
*
* @param	NpiIrqId: NPI IRQ ID.
* @param	AieIrqId: AIE IRQ ID.
*
* @note		This function is internal.
*
*******************************************************************************/
static inline void _XAie_LNpiIrqEnable(u8 NpiIrqId, u8 AieIrqId)
{
	u64 RegOff;

	RegOff = XAIE_NPI_IRQ0_ENABLE_REG + NpiIrqId * XAIE_NPI_PER_IRQ_REGOFF;
	_XAie_LNpiSetLock(XAIE_DISABLE);

	_XAie_LNpiWrite32(RegOff, (1 << AieIrqId));
	_XAie_LNpiSetLock(XAIE_ENABLE);
}

#endif		/* end of protection macro */
/** @} */
