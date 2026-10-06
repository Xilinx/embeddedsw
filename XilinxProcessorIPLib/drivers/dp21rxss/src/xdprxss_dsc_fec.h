/******************************************************************************
* Copyright 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
 * @file xdprxss_dsc_fec.h
 * @brief DP RX Subsystem DSC/FEC Support Header
 *
 * Declares the Display Stream Compression and Forward Error Correction entry
 * points of the DisplayPort 2.1 RX Subsystem. The whole interface is compiled
 * only for designs that build the DSC decoder in.
 */

#ifndef XDPRXSS_DSC_FEC_H
#define XDPRXSS_DSC_FEC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xdprxss.h"
#include "xil_types.h"
#include "xstatus.h"

#ifdef XDPRXSS_USE_DSC

/************************** Function Prototypes ******************************/

/**
 * @brief Initialize DSC/FEC support for RX Subsystem
 * @param InstancePtr Pointer to XDpRxSs instance
 * @return XST_SUCCESS if successful
 */
int XDpRxSs_DscFecInit(XDpRxSs *InstancePtr);

/**
 * @brief Enable/Disable FEC on RX
 * @param InstancePtr Pointer to XDpRxSs instance
 * @param Enable 1 to enable, 0 to disable
 * @return XST_SUCCESS if successful
 */
int XDpRxSs_SetFecEnable(XDpRxSs *InstancePtr, u8 Enable);

/**
 * @brief Enable DSC pass-through mode
 * @param InstancePtr Pointer to XDpRxSs instance
 * @param Enable 1 to enable, 0 to disable
 * @return XST_SUCCESS if successful
 */
int XDpRxSs_EnableDscPassthrough(XDpRxSs *InstancePtr, u8 Enable);

/**
 * @brief Expose DSC/FEC capability to upstream TX (call when RX link is up)
 * @param InstancePtr Pointer to XDpRxSs instance
 * @return XST_SUCCESS if successful
 */
int XDpRxSs_ExposeDscFecCapability(XDpRxSs *InstancePtr);

#endif /* XDPRXSS_USE_DSC */

#ifdef __cplusplus
}
#endif

#endif /* XDPRXSS_DSC_FEC_H */
