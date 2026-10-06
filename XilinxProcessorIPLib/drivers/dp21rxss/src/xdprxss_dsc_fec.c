/******************************************************************************
* Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xdprxss_dsc_fec.c
 * @brief DP RX Subsystem DSC/FEC Support Implementation
 *
 * Programs FEC_CAPS and DSC_SUPPORT so upstream TX can discover capability
 * via DPCD (AUX). Per RAL: these AXI-Lite registers map to DPCD.
 */

/***************************** Include Files *********************************/
#include "xdprxss_dsc_fec.h"

#ifdef XDPRXSS_USE_DSC

#include "xdprxss.h"
#include "xdp21rxss_hw.h"
#include "xdp21_dsc_fec_vbid_pps_extraction.h"

/************************** Function Definitions *****************************/

/**
 * @brief Initialize DSC/FEC support for RX Subsystem
 *
 * Exposes DSC/FEC capability so an upstream source sees both advertised in
 * DPCD as soon as the link comes up.
 */
int XDpRxSs_DscFecInit(XDpRxSs *InstancePtr)
{
	if (InstancePtr == NULL || InstancePtr->DpPtr == NULL) {
		return XST_FAILURE;
	}

	XDp21_ExposeDscFecCapability(InstancePtr->DpPtr->Config.BaseAddr);

	return XST_SUCCESS;
}

/**
 * @brief Expose DSC/FEC capability to upstream TX (call when RX link is up)
 */
int XDpRxSs_ExposeDscFecCapability(XDpRxSs *InstancePtr)
{
	if (InstancePtr == NULL || InstancePtr->DpPtr == NULL) {
		return XST_FAILURE;
	}

	XDp21_ExposeDscFecCapability(InstancePtr->DpPtr->Config.BaseAddr);

	return XST_SUCCESS;
}

/**
 * @brief Enable/Disable FEC on RX
 *
 * The receive FEC decoder follows the capability advertised by
 * XDpRxSs_ExposeDscFecCapability() and has no separate enable in the register
 * map, so this validates the instance and reports success. Enable is accepted
 * to keep the call site stable for when a control register is defined.
 */
int XDpRxSs_SetFecEnable(XDpRxSs *InstancePtr, u8 Enable)
{
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	(void)Enable;

	return XST_SUCCESS;
}

/**
 * @brief Enable DSC pass-through mode
 *
 * Pass-through needs no receive side programming, because the subsystem
 * forwards the compressed stream whenever the upstream source chooses a DSC
 * encoding. Enable is accepted to keep the call site stable for when a control
 * register is defined.
 */
int XDpRxSs_EnableDscPassthrough(XDpRxSs *InstancePtr, u8 Enable)
{
	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}

	(void)Enable;

	return XST_SUCCESS;
}

#endif /* XDPRXSS_USE_DSC */
