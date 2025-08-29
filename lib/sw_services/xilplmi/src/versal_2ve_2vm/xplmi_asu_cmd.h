/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_asu_cmd.c
*
* This file contains asu commands code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================
* 1.00  am   02/19/25 Initial release
*       rmv  08/26/25 Update function prototype of XPlmi_AsuModuleInit
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xilplmi_server_apis XilPlmi server APIs
 * @{
 */

/***************************** Include Files *********************************/
#ifndef XPLMI_ASU_CMD_H
#define XPLMI_ASU_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xparameters.h"

/************************** Function Prototypes ******************************/
void XPlmi_AsuModuleInit(int (* const GeneratePufKEK)(u8* PufRegenStatusFlag),
	int (* const InitiateKeyXfer)(void),
	int (* const GetAsuCdiSeed)(u32 CdiAddr),
	int (* const GetSubsysDigest)(u32 SubsystemId, u32 SubsysHashAddrPtr),
	u32 (* const GetKEKIvAddr)(void));

/************************** Variable Definitions *****************************/

/**
 * @}
 * @endcond
 */


#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_ASU_CMD_H */
