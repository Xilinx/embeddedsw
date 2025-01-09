/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xemacps_pcs.c
* @addtogroup emacps Overview
* @{
*
* This file contains the implementation of the ethernet PCS block api's
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 3.22  vineeth  11/15/24 First Release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdlib.h>
#include "xemacps_pcs.h"
#include "xil_sutil.h"
#define POLL_TIMEOUT	2000000

/*****************************************************************************/
/**
* Set up PCS as follows for every given speed:
*   - Configure HS_MAC register for given speed
*   - Configure serdes rate and speed in USX_CONTROL register
*   - Assert Rx reset signal for GT reset
*   - Wait for PCS_BLOCK_LOCK
*
* @param InstancePtr is a pointer to the instance to be worked on.
*
* @return N/A
*
******************************************************************************/
void XEmacPs_SetupPCS(XEmacPs *EmacPsInstancePtr)
{

	u32 BaseAddr = EmacPsInstancePtr->Config.BaseAddress;
	u32 UsxCntrl, Status;

	Xil_AssertVoid(EmacPsInstancePtr != NULL);
	Xil_AssertVoid(EmacPsInstancePtr->Config.BaseAddress != 0);

	XEmacPs_WriteReg(BaseAddr, XEMACPS_REG(HS_MAC_CONFIG), SPEED_10G);
	/* Configure USX Control Register */
	UsxCntrl = XEMACPS_BFINS(SERDES_RATE, SERDES_RATE_10G, 0);
	UsxCntrl = XEMACPS_BFINS(USX_SPEED, SPEED_10G, UsxCntrl);
	UsxCntrl |= XEMACPS_BIT(RX_SYNC_RESET);
	UsxCntrl &= ~(XEMACPS_BIT(TX_SCR_BYPASS) | XEMACPS_BIT(RX_SCR_BYPASS));
	XEmacPs_WriteReg(BaseAddr, XEMACPS_REG(USX_CONTROL), UsxCntrl);

	UsxCntrl |= (XEMACPS_BIT(SIGNAL_OK) | XEMACPS_BIT(TX_EN));
	UsxCntrl &= ~XEMACPS_BIT(RX_SYNC_RESET);
	XEmacPs_WriteReg(BaseAddr, XEMACPS_REG(USX_CONTROL), UsxCntrl);

	if (Xil_WaitForEvent(BaseAddr + XEMACPS_REG(NWSR), ~0, XEMACPS_NWSR_MDIOIDLE_MASK,
			     POLL_TIMEOUT) != (u32)XST_SUCCESS) {
		Status = (s32)XST_FAILURE;
		xil_printf("ERROR: TIMEOUT: PCS Link not Up..\n");
		exit(Status);
	}
	return ;
}

/*****************************************************************************/
/**
* Get USX synchronisation status
*
* @param BaseAddr is the base address of the device
*
* @return
* - true if block synchronization achieved
* - false if block is out of sync
*
******************************************************************************/
bool XEmacPs_USXPCSGetState(UINTPTR BaseAddr)
{
	u32 Reg;

	Xil_AssertNonvoid(BaseAddr != 0);
	Reg = XEmacPs_ReadReg(BaseAddr, XEMACPS_REG(USX_STATUS));
	return !!(Reg & XEMACPS_BIT(USX_BLOCK_LOCK));
}

/*****************************************************************************/
/**
* Detect if the device supports High Speeds ( 5G, 10G )
*
* @param BaseAddr is the base address of the device
*
* @return
* - true if the device support High Speed
* - false if the device doesn't support High Speed
*
******************************************************************************/
bool XEmacPs_IsHighSpeedPCS(UINTPTR BaseAddr)
{
	u32 Reg_cfg1;
	u32 Reg_cfg12;

	Xil_AssertNonvoid(BaseAddr != 0);
	Reg_cfg1 = XEmacPs_ReadReg(BaseAddr, XEMACPS_REG(DCFG1));
	Reg_cfg12 = XEmacPs_ReadReg(BaseAddr, XEMACPS_REG(DCFG12));

	return !!(!XEMACPS_BFEXT(NO_PCS, Reg_cfg1) &&
	          XEMACPS_BFEXT(HIGH_SPEED, Reg_cfg12));
}
/** @} */
