/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
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
 * 1.00  gnr  02/09/26 Initial release
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
/**
 * Structure to hold DDR Counters
 */
typedef struct {
		u32 DDRCounter0; /**< DDR Counter0 */
		u32 DDRCounter1; /**< DDR Counter1 */
		u32 DDRCounter2; /**< DDR Counter2 */
		u32 DDRCounter3; /**< DDR Counter3 */
} XLoader_DDRCounters;

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XLOADER_PDI_SRC_DDR			(0xF)	/**< Source of PDI is DDR */
#define XLOADER_PDI_SRC_IS			(0x10)	/**< Source of PDI is Image Store */
#define XLOADER_PDI_TYPE_FULL		        (0x1U)  /**< PDI type full */
#define XLOADER_PDI_TYPE_PARTIAL	        (0x2U)  /**< PDI type partial */

/************************************ Function Prototypes ****************************************/
int XLoader_ConfigureJtagState(XLoader_ClientInstance *InstancePtr, u32 Flag);
int XLoader_ReadDdrCryptoPerfCounters(XLoader_ClientInstance *InstancePtr, u32 NodeId,
		XLoader_DDRCounters *CryptoCounters);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_PLAT_CLIENT_H */
