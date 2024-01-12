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

/************************************ Function Prototypes ****************************************/

int XLoader_ConfigureJtagState(XLoader_ClientInstance *InstancePtr, u32 Flag);
int XLoader_ReadDdrCryptoPerfCounters(XLoader_ClientInstance *InstancePtr, u32 Id,
		XLoader_DDRCounters *CryptoCounters);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_PLAT_CLIENT_H */