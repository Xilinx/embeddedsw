/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp.h
* @addtogroup xilocp_server_apis XilOcp Server APIs
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   vns  06/26/2022 Initial release
* 1.1   am   01/10/2023 Modified function argument type to u64 in
*                       XOcp_GenerateDmeResponse().
* 1.2   kal  05/28/2023 Added SW PCR extend and logging functions
* 1.3   am   01/31/2024 Fixed wrong XOcp_ExtendSwPcr() prototype parameter
* 1.3   kpt  01/22/2024 Added support to extend secure state into SWPCR
*       kpt  02/21/2024 Add support for DME CSR extension
* 1.4   har  06/10/2024 Add data structure IDs for OCP module
*       vss  10/24/2024 Added xppu disabled value macro for dynamic reconfiguration
* 1.5   sk   03/05/2025 Added define for OCP PCR Measurement index position
*       rmv  07/17/2025 Move declaration of XOcp_ValidateDiceCdi() and XOcp_KeyGenDevAkSeed() APIs
*			from xocp_keymgmt.h file to xocp.h file as exported function.
*       tvp  08/23/2025 Enable hardware PCR functionality only if PLM_HW_PCR is defined
* 1.7   rmv  01/30/2026 Refactor xilocp library
*
* </pre>
*
* @note
*
* @endcond
******************************************************************************/
#ifndef XOCP_H
#define XOCP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP
#include "xocp_common.h"
#include "xocp_hw.h"
#include "xocp_init.h"
#include "xocp_generic.h"
#include "xocp_pcr.h"
#include "xocp_dme.h"
#ifdef PLM_OCP_ASUFW_KEY_MGMT
#include "xocp_asufw.h"
#endif
#ifdef PLM_OCP_KEY_MGMT
#include "xocp_keymgmt_common.h"
#ifdef PLM_OCP_NATIVE_KEY_MGMT
#include "xocp_keymgmt_native.h"
#endif
#endif
#include "xocp_plat.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif
#endif /* PLM_OCP */
#endif  /* XOCP_H */
/** @} */
