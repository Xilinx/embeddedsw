/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file versal_2vp/xsecure_plat_kat_ipihandler.h
*
* This file contains the Xilsecure KAT platform IPI handler declaration.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 5.7   tvp  11/14/25 Initial release
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_kat_server_apis XilSecure KAT Server APIs
* @{
*/
#ifndef XSECURE_PLAT_KAT_IPIHANDLER_H_
#define XSECURE_PLAT_KAT_IPIHANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/****************************************** Include Files *****************************************/
#include "xplmi_cmd.h"
#include "xsecure_plat_kat.h"

/*************************************** Constant Definitions *************************************/

/****************************************** Type Definitions **************************************/

/**************************************** Function Prototypes *************************************/
int XSecure_KatPlatIpiHandler(XPlmi_Cmd *Cmd);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_PLAT_KAT_IPIHANDLER_H_ */
/** @} */
