/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file amd_xspi_options.c
* @addtogroup amd_xspi Overview
* @{
*
* This file contains the functions that configure the AXI XSPI controller,
* such as the clock mode and the target selection.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   akm  07/29/26 Initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "amd_xspi.h"
#include "amd_xspi_control.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* Set the SPI clock mode (CPOL/CPHA).
*
* The clock is only programmable in the Standard, Dual and Quad modes. A core
* built for one of the xSPI profiles runs mode 0 and nothing else, so that is
* the only value it accepts.
*
* @param[in] InstancePtr is a pointer to the AmdXspi instance.
* @param[in] ClkMode is the clock mode (0-3):
*		- 0: CPOL=0, CPHA=0
*		- 1: CPOL=0, CPHA=1
*		- 2: CPOL=1, CPHA=0
*		- 3: CPOL=1, CPHA=1
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if the mode is out of range, or is not
*		  mode 0 on a core built for an xSPI profile.
*
******************************************************************************/
s32 AmdXspi_SetClkMode(AmdXspi *InstancePtr, u8 ClkMode)
{
	u32 ControlReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (ClkMode > AMD_XSPI_CLK_MODE_3) {
		return XST_INVALID_PARAM;
	}

	if ((InstancePtr->Config.SpiMode >= AMD_XSPI_MODE_XSPI_PROFILE_1_0) &&
	    (ClkMode != AMD_XSPI_CLK_MODE_0)) {
		return XST_INVALID_PARAM;
	}

	/*
	 * CPOL and CPHA define the idle level of SCK and the edge data is
	 * sampled on, so changing them under an enabled core would move the
	 * clock mid-transfer. The core is quiesced across the change and
	 * enabled again afterwards, which requires the caller to be idle.
	 */
	AmdXspi_Disable(InstancePtr);

	ControlReg = AmdXspi_ReadReg(InstancePtr->Config.BaseAddress,
				      AMD_XSPI_CONTROL_OFFSET);

	ControlReg &= ~(AMD_XSPI_CR_CPOL_MASK | AMD_XSPI_CR_CPHA_MASK);

	if ((ClkMode == AMD_XSPI_CLK_MODE_2) || (ClkMode == AMD_XSPI_CLK_MODE_3)) {
		ControlReg |= AMD_XSPI_CR_CPOL_MASK;
	}
	if ((ClkMode == AMD_XSPI_CLK_MODE_1) || (ClkMode == AMD_XSPI_CLK_MODE_3)) {
		ControlReg |= AMD_XSPI_CR_CPHA_MASK;
	}

	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_CONTROL_OFFSET, ControlReg);

	AmdXspi_Enable(InstancePtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Select a slave device.
*
* @param[in] InstancePtr is a pointer to the AmdXspi instance.
* @param[in] SlaveNum is the slave number (0-7).
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_INVALID_PARAM if slave number is invalid.
*
******************************************************************************/
s32 AmdXspi_SetSlaveSelect(AmdXspi *InstancePtr, u8 SlaveNum)
{
	u8 SlaveSelectValue;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (SlaveNum >= InstancePtr->Config.NumSlaves) {
		return XST_INVALID_PARAM;
	}

	/*
	 * TARGET_SELECT is active-high one-hot: a 1 in the target's bit
	 * position selects that target (the engine drives the active-low
	 * cs_o pin internally). Slave 0 -> 0x01. This matches the value
	 * observed in known-good hardware transactions.
	 */
	SlaveSelectValue = (u8)((1U << SlaveNum) & 0xFFU);

	AmdXspi_WriteReg(InstancePtr->Config.BaseAddress,
			  AMD_XSPI_TARGET_SELECT_OFFSET,
			  (u32)SlaveSelectValue);

	return XST_SUCCESS;
}

/** @} */
