
/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xi3cpsx_hw.c
* @addtogroup Overview
* @{
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
*
* </pre>
*
******************************************************************************/

#include "xi3cpsx.h"
#include "sleep.h"
/*****************************************************************************/
/**
*
* @brief
* Resets the IIC device. Reset must only be called after the driver has been
* initialized. The configuration of the device after reset is the same as its
* configuration after initialization.  Any data transfer that is in progress is
* aborted.
*
* The upper layer software is responsible for re-configuring (if necessary)
* and reenabling interrupts for the IIC device after the reset.
*
* @param        InstancePtr is a pointer to the XIicPs instance.
*
* @return       None.
*
* @note         None.
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
* @brief
* Resets the IIC device. Reset must only be called after the driver has been
* initialized. The configuration of the device after reset is the same as its
* configuration after initialization.  Any data transfer that is in progress is
* aborted.
*
* The upper layer software is responsible for re-configuring (if necessary)
* and reenabling interrupts for the IIC device after the reset.
*
* @param        InstancePtr is a pointer to the XIicPs instance.
*
* @return       None.
*
* @note         None.
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

/** @} */
