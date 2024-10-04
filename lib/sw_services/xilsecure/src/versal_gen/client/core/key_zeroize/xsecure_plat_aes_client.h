/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file xsecure_plat_aes_client.h
* This file contains the function prototypes for the AES client APIs for
* Versal Net.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 5.3   har  02/05/24 Initial release
* 5.4   yog  04/29/24 Fixed doxygen grouping
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_aes_client_apis XilSecure AES Client APIs
* @{
*/
#ifndef XSECURE_PLAT_AES_CLIENT_H
#define XSECURE_PLAT_AES_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_mailbox.h"
#include "xil_types.h"
#include "xsecure_defs.h"

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/
int XSecure_AesPerformOperationAndZeroizeKey(XSecure_ClientInstance *InstancePtr, u64 KeyAddr, const XSecure_AesDataBlockParams *AesDataParams);

#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_PLAT_AES_CLIENT_H */
/** @} */
