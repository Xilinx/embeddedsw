/******************************************************************************
* Copyright (c) 2020 - 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2025, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xsecure_init.h
 *
 * Header file for xsecure_init.c
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   rpo  07/01/2020 Initial release
 * 4.3   rpo  07/01/2020 Updated file version to sync with library version
 *       am   09/24/2020 Resolved MISRA C violations
 *       har  10/12/2020 Addressed security review comments
 * 4.5   bm   05/13/2021 Add common crypto instances
 * 4.6   ma   07/08/2022 Added support for secure lockdown
 * 5.4   kal  07/24/2024 Code refactoring for versal_2ve_2vm
 *       pre  03/02/2025 Modified prototype of XSecure_Init function
 *       sd   04/30/2025 Make XSecure_AesShaInit as non static function
 *
 * </pre>
 *
 ******************************************************************************/
/**
* @addtogroup xsecure_generic_server_apis XilSecure Generic Server APIs
* @{
*/
#ifndef XSECURE_INIT_H_
#define XSECURE_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_aes.h"
#ifndef PLM_RSA_EXCLUDE
#include "xsecure_rsa_core.h"
#endif
#include "xsecure_sha.h"
#ifdef VERSAL_PLM
#include "xplmi_config.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct
{
	u32 PartialPdiEventSts;
	int (*TriggerPartialPdiEvent)(void);
	u32 IpiMask;
	u32 PdiSrc;
	u64 PdiAddr;
} XSecure_PartialPdiEventParams;

/************************** Function Prototypes ******************************/
int XSecure_Init(XSecure_PartialPdiEventParams *PpdiEventParamsPtr);
int XSecure_AesShaInit(void);
XSecure_Sha *XSecure_GetSha3Instance(u32 DeviceId);
XSecure_Sha *XSecure_GetSha2Instance(u32 DeviceId);
XSecure_Aes *XSecure_GetAesInstance(void);
XSecure_Sha *XSecure_GetShaInstance(u32 DeviceId);
#ifndef PLM_RSA_EXCLUDE
XSecure_Rsa *XSecure_GetRsaInstance(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_INIT_H_ */
/** @} */
