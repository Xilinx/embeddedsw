/***************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
 *
 * @file versal_2vp_p/client/xloader_plat_client.h
 *
 * This file contains the client function prototypes, defines and macros.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 2.5   tvp  08/03/26 Initial release
 *
 * </pre>
 *
 **************************************************************************************************/

#ifndef XLOADER_PLAT_CLIENT_H
#define XLOADER_PLAT_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xloader_mailbox.h"

/************************************* Constant Definitions ***************************************/

/*************************************** Type Definitions *****************************************/

/**************************** Macros (Inline Functions) Definitions *******************************/

/************************************* Function Prototypes ****************************************/
int XLoader_ConfigureJtagState(XLoader_ClientInstance *InstancePtr, u32 Flag);

/************************************* Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_PLAT_CLIENT_H */
