/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xloader_plat_client.h
 *
 * This file Contains the client function prototypes, defines and macros.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *       har  02/16/24 Added XLoader_GetOptionalData API
 *       har  03/05/24 Fixed doxygen warnings
 *       kpt  10/04/24 Added support to validate partial and optimized authentication enabled PDI
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XLOADER_PLAT_CLIENT_H
#define XLOADER_PLAT_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

#include "xil_types.h"
#include "xloader_mailbox.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

typedef struct {
		u32 DDRCounter0; /**< DDR Counter0 */
		u32 DDRCounter1; /**< DDR Counter1 */
		u32 DDRCounter2; /**< DDR Counter2 */
		u32 DDRCounter3; /**< DDR Counter3 */
} XLoader_DDRCounters; /**< DDR Counters */

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XLOADER_PDI_SRC_DDR			(0xF)	/**< Source of PDI is DDR */
#define XLOADER_PDI_SRC_IS			(0x10)	/**< Source of PDI is Image Store */
#define XLOADER_PDI_TYPE_FULL		        (0x1U)  /**< PDI type full */
#define XLOADER_PDI_TYPE_PARTIAL	        (0x2U)  /**< PDI type partial */

/************************************ Function Prototypes ****************************************/
int XLoader_ConfigureJtagState(XLoader_ClientInstance *InstancePtr, u32 Flag);
int XLoader_ReadDdrCryptoPerfCounters(XLoader_ClientInstance *InstancePtr, u32 NodeId,
		XLoader_DDRCounters *CryptoCounters);
int XLoader_ValidatePdiAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr, const u32 PdiType);
int XLoader_GetOptionalData(XLoader_ClientInstance *InstancePtr,
	const XLoader_OptionalDataInfo* OptionalDataInfo, u64 DestAddr, u32 *DestSize);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_PLAT_CLIENT_H */
