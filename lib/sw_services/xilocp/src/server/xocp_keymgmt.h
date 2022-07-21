/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_keymgmt.h
* @addtogroup xil_ocpapis DeviceKeysMgmt APIs
* @{
*
* @cond xocp_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date         Changes
* ----- ---- ----------   -------------------------------------------------------
* 1.0   dc  07/08/2022   Initial release
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XOCPKEYMGMT_SERVER_H
#define XOCPKEYMGMT_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xocp.h"

/************************** Constant Definitions *****************************/
#define XOCP_EFUSE_DEVICE_DNA_CACHE			(0xF1250020U)
#define XOCP_EFUSE_DEVICE_DNA_SIZE_WORDS		(4U)
#define XOCP_EFUSE_DEVICE_DNA_SIZE_BYTES		(12U)
#define XOCP_CDI_SIZE_IN_BYTES				(48U)
#define XOCP_CDI_SIZE_IN_WORDS				(12U)
#define XOCP_DEVAK_GEN_TRNG_SEED_SIZE_IN_BYTES		(48U)
#define XOCP_TIMEOUT_MAX				(0x1FFFFU)

#define XOCP_PMC_GLOBAL_ZEROIZE_CTRL_ZEROIZE_MASK	(0x00000001U)
#define XOCP_PMC_GLOBAL_ZEROIZE_STATUS_PASS_MASK	(0x00000002U)
#define XOCP_PMC_GLOBAL_ZEROIZE_STATUS_DONE_MASK	(0x00000001U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
int XOcp_KeyInit(void);

#ifdef __cplusplus
}
#endif

#endif  /* XOCPKEYMGMT_SERVER_H */
