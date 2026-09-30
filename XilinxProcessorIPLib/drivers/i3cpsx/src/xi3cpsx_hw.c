
/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xi3cpsx_hw.c
* @brief Implements I3C reset and FIFO reset helpers.
*
* This file contains low level access functions using the base address
* directly without an instance.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.00  sd  06/10/22 First release
* 1.8   vn  07/30/26 Fix Doxygen warnings
* 1.8   tb  09/22/26 Align Doxygen comments with AMD documentation standards
*
* </pre>
*
******************************************************************************/

/**
* @addtogroup i3cpsx_api I3CPSX APIs
* @{
*/

#include "xi3cpsx.h"
#include "sleep.h"
/*****************************************************************************/
/**
*
* @brief Resets the I3C controller.
*
* Reset must only be called after the driver has been initialized. The
* configuration of the device after reset is the same as its configuration
* after initialization. Any data transfer that is in progress is aborted.
*
* The upper layer software is responsible for re-configuring (if necessary)
* and reenabling interrupts for the I3C device after the reset.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
*
******************************************************************************/
void XI3cPsx_Reset(XI3cPsx *InstancePtr)
{
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_RESET_CTRL,
			 XI3CPSX_RESET_CTRL_SOFT_RST_VAL);

	(void)Xil_WaitForEvent(((InstancePtr->Config.BaseAddress) + XI3CPSX_RESET_CTRL),
			       XI3CPSX_RESET_CTRL_SOFT_RST_VAL, 0, XI3CPSX_TIMEOUT_COUNTER);
}
/*****************************************************************************/
/**
*
* @brief Resets the I3C FIFOs.
*
* Reset must only be called after the driver has been initialized. The
* configuration of the device after reset is the same as its configuration
* after initialization. Any data transfer that is in progress is aborted.
*
* The upper layer software is responsible for re-configuring (if necessary)
* and reenabling interrupts for the I3C device after the reset.
*
* @param[in] InstancePtr Pointer to the XI3cPsx instance.
*
******************************************************************************/
void XI3cPsx_ResetFifos(XI3cPsx *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	/*
	 * Reset any values so the software state matches the hardware device.
	 */
	XI3cPsx_WriteReg(InstancePtr->Config.BaseAddress, XI3CPSX_RESET_CTRL,
			 XI3CPSX_RESET_CTRL_ALL_FIFOS_RST_VAL);

	(void)Xil_WaitForEvent(((InstancePtr->Config.BaseAddress) + XI3CPSX_RESET_CTRL),
			       XI3CPSX_RESET_CTRL_ALL_FIFOS_RST_VAL, 0, XI3CPSX_TIMEOUT_COUNTER);
}

/*****************************************************************************/
/**
*
* @brief Brings the controller back to a usable state after a transfer error.
*
* An error response such as a receive buffer overflow leaves the core halted,
* and it stays halted until RESUME is written. The FIFOs are cleared after
* that because a transfer aborted part way through can leave untransmitted
* data behind in the TX FIFO.
*
* This does not restore a target that has latched an error into its
* CCC_DEVICE_STATUS; that register is read only and needs the controller to
* read it with GETSTATUS.
*
* @param[in,out] InstancePtr Pointer to the XI3cPsx instance.
*
* @note         RESUME must be written before the FIFOs are reset. Resetting
*               the FIFOs of a halted core leaves it halted.
*
******************************************************************************/
void XI3cPsx_RecoverFromError(XI3cPsx *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == (u32)XIL_COMPONENT_IS_READY);

	XI3cPsx_Resume(InstancePtr);
	XI3cPsx_ResetFifos(InstancePtr);

	InstancePtr->SendByteCount = 0;
	InstancePtr->RecvByteCount = 0;
}

/** @} */
