/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_generic.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.7   rmv  01/30/26 Refactor OCP library
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XOCP_GENERIC_H
#define XOCP_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xil_types.h"
#ifdef PLM_OCP
#include "xplmi_dma.h"

/************************** Constant Definitions *****************************/
/** @cond xocp_internal
 * @{
 */
#define XOcp_MemCopy					XPlmi_DmaXfr
							/**< Data transfer through DMA */
#define XOcp_Printf					XPlmi_Printf
							/**< XILOCP print function */
#define XOCP_WORD_LEN					(0x4U) /**< Word length */
#define XOCP_PCR_NUMBER_MASK				(0x0000FFFFU) /**< Number mask value */
#define XOCP_PCR_MEASUREMENT_INDEX_MASK			(0xFFFF0000U)
#define XOCP_PCR_MEASUREMENT_INDEX_SHIFT		(16U)
							 /**< Measurement index mask value */
#define XOCP_PCR_INVALID_VALUE				(0xFFFFFFFFU) /**< PCR invalid value */
#define XOCP_PCR_HASH_SIZE_IN_BYTES			(48U) /**< Hash size in bytes */

/** XilOcp Module Data Structure Ids*/
#define XOCP_DEVAK_SUBSYS_HASH_DS_ID		(1U)	/**< DevAk Subsystem Hash data structure ID */
#define XOCP_SWPCR_CONFIG_DS_ID			(2U)	/**< SW PCR config data structure ID */
#define XOCP_SWPCR_STORE_DS_ID			(3U)	/**< SW PCR store data structure ID */
#define XOCP_HWPCR_LOG_DS_ID			(4U)	/**< HW PCR log data structure ID */
#define XOCP_ACTIVE_SUBSYS_MASK_DS_ID		(5U)	/**< Active OCP subsystem mask data
							structure ID */
#define XOCP_OCP_SUBSYSTEM_INFO_DS_ID		(6U)	/**< OCP subsystem info data structure ID */

#define XOCP_CDI_SIZE_IN_BYTES			(48U)	/**< CDI size in bytes */
#define XOCP_CDI_SIZE_IN_WORDS			(12U)	/**< CDI size in words */
/** @}
 * @endcond
 */

/**************************** Type Definitions *******************************/
/**
 * Secure TAP (DAP) and boot mode configuration read from hardware
 */
typedef struct {
	u32 DapCfg; /**< Dap configuration */
	u32 InstMask0; /**< Inst mask 0 */
	u32 InstMask1; /**< Inst mask 1 */
	u32 DapSecurity; /**< DAP security */
	u32 BootDevice; /**< Boot Device */
} XOcp_SecureTapConfig;

/**
 * Digests for ROM, PLM, secure configuration, and TAP used in secure-state measurement
 */
typedef struct {
	u8 RomHash[XOCP_PCR_HASH_SIZE_IN_BYTES]; /**< Rom hash */
	u8 PlmHash[XOCP_PCR_HASH_SIZE_IN_BYTES]; /**< Plm hash */
	u8 SecureConfigHash[XOCP_PCR_HASH_SIZE_IN_BYTES]; /**< Secure config hash */
	u8 TapConfigHash[XOCP_PCR_HASH_SIZE_IN_BYTES]; /**< Tap config hash */
} XOcp_SecureStateHash;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XOcp_ReadTapConfig(XOcp_SecureTapConfig* TapConfig);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif
#endif /* PLM_OCP */
#endif  /* XOCP_GENERIC_H */
