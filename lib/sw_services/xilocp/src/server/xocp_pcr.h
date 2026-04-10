/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xocp_pcr.h
*
* This file contains the function prototypes, macro definitions and structure definitions
* for Platform Configuration Register (PCR) operations in the OCP library.
*
* The PCR module provides APIs for:
* - Hardware PCR extend and read operations
* - Software PCR configuration, extend and read operations
* - PCR event logging and retrieval
* - Secure state measurement and extension to PCRs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------------------------------------
* 1.7   rmv  01/30/26 Refactor OCP library
*       rmv  04/10/26 Added comprehensive API documentation
*
* </pre>
*
**************************************************************************************************/
#ifndef XOCP_PCR_H
#define XOCP_PCR_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_OCP
#include "xstatus.h"
#include "xocp_common.h"
#include "xocp_plat.h"
#include "xplmi_debug.h"
#include "xocp_generic.h"

/************************** Constant Definitions *****************************/
/** @cond xocp_internal
 * @{
 */
#define XOCP_SW_PCR_NUM_0				(0U) /**< SW PCR number 0 */
#define XOCP_SW_PCR_NUM_1				(1U) /**< SW PCR number 1 */
#define XOCP_SW_PCR_SEC_STATE_MEASUREMENT_IDX		(0U) /**< Measurement index for secure state extension */
#define XOCP_SW_PCR_PPK_CONFIG_MEASUREMENT_IDX		(1U) /**< Measurement index for eFuse ppk config extension */
#define XOCP_SW_PCR_SPK_REVOKE_CONFIG_MEASUREMENT_IDX	(2U) /**< Measurement index for eFuse spk revoke config extension */
#define XOCP_SW_PCR_REVOKE_OTHER_CONFIG_MEASUREMENT_IDX (3U) /**< Measurement index for eFuse revoke other config extension */
#define XOCP_SW_PCR_MISC_CONFIG_MEASUREMENT_IDX		(4U) /**< Measurement index for eFuse misc config extension */

/** @}
 * @endcond
 */

/**************************** Type Definitions *******************************/
/**
 * SW PCR Config
 */
typedef struct {
	u8 DigestsForPcr[XOCP_NUM_OF_SWPCRS];		/**< Max digests for each SW PCR */
	u8 PcrDigestsConfigured[XOCP_NUM_OF_SWPCRS];	/**< Configured no of digests */
	u8 PcrIdxInLog[XOCP_NUM_OF_SWPCRS];		/**< Each SW PCR index in log */
	u8 IsPcrConfigReceived;				/**< SW PCR config received */
	u8 Reserved[3U];                    /**< Reserved bytes */
} XOcp_SwPcrConfig;

/**
 * SW PCR data for a single extend event
 */
typedef struct {
	XOcp_PcrMeasurement Measurement;		/**< XOcp_PcrMeasurement structure */
	u32 IsReqExtended;				/**< Is request extended */
	u64 DataAddr;					/**< Address of the data buffer */
	u8 DataToExtend[XOCP_PCR_SIZE_BYTES];		/**< Data to extend */
} XOcp_SwPcrData;

/**
 * SW PCR log
 */
typedef struct {
	XOcp_SwPcrData Data[XOCP_MAX_NUM_OF_SWPCRS];	/**< SW PCR log store with max no of events */
	u8 CountPerPcr[XOCP_NUM_OF_SWPCRS];		/**< Number of digests extended for each SW PCR */
} XOcp_SwPcrStore;

/**
 * PPK (primary public key) eFuse configuration used for SW PCR measurement
 */
typedef struct {
	u32 Ppk0WrLk;		/**< PPK0 WR LK eFuse */
	u32 Ppk1WrLk;		/**< PPK1 WR LK eFuse */
	u32 Ppk2WrLk;		/**< PPK2 WR LK eFuse */
	u32 Ppk0Invld;		/**< PPK0 INVALID eFuse */
	u32 Ppk1Invld;		/**< PPK1 INVALID eFuse */
	u32 Ppk2Invld;		/**< PPK2 INVALID eFuse */
	u32 Ppk0Hash[XOCP_EFUSE_PPK_HASH_NO_OF_WORDS]; /**< PPK0 Hash eFuse */
	u32 Ppk1Hash[XOCP_EFUSE_PPK_HASH_NO_OF_WORDS]; /**< PPK1 Hash eFuse */
	u32 Ppk2Hash[XOCP_EFUSE_PPK_HASH_NO_OF_WORDS]; /**< PPK2 Hash eFuse */
} XOcp_PpkEfuseConfig;

/**
 * SPK revocation eFuse words (REVOCATION_ID)
 */
typedef struct {
	u32 RevocationId[XOCP_EFUSE_REVOCATION_NO_OF_WORDS]; /**< REVOCATION_ID eFuses */
} XOcp_RevocationSpkEfuseConfig;

/**
 * Other revocation-related eFuse fields (DME revoke, off-chip revocation ID)
 */
typedef struct {
	u32 DmeRevoke0;	/**< DME_REVOKE_0 eFuse */
	u32 DmeRevoke1; /**< DME_REVOKE_1 eFuse */
	u32 DmeRevoke2; /**< DME_REVOKE_2 eFuse */
	u32 DmeRevoke3; /**< DME_REVOKE_3 eFuse */
	u32 OffChipRevocationId[XOCP_EFUSE_REVOCATION_NO_OF_WORDS]; /**< OFFCHIP Revocation ID eFuses */
} XOcp_RevocationOtherEfuseConfig;

/**
 * Miscellaneous security-related eFuse configuration (UDS lock, test disable, PUF, DME mode, etc.)
 */
typedef struct {
	u32 UdsWrLk; 			/**< UDS_WR_LK eFuse */
	u32 HwTstBitsDis; 		/**< HWTST_DIS eFuse */
	u32 PufDis;			/**< PUF_DIS eFuse */
	u32 PmcScEn;	 		/**< PMC_SC_EN eFuses */
	u32 SysmonTempMonEn; 		/**< SYSMON_TEMP_MON_EN eFuses */
	u32 DmeMode;			/**< DME_MODE eFuses */
} XOcp_MiscEfuseConfig;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
#ifdef PLM_HW_PCR
int XOcp_ExtendHwPcr(XOcp_HwPcr PcrNum, u64 ExtHashAddr, u32 DataSize);
int XOcp_GetHwPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize);
int XOcp_GetHwPcrLog(u64 HwPcrEventsAddr, u64 HwPcrLogInfoAddr, u32 NumOfLogEntries);
#endif
int XOcp_ExtendSwPcr(u32 PcrNum, u32 MeasurementIdx, u64 DataAddr, u32 DataSize, u32 OverWrite);
int XOcp_StoreSwPcrConfigAndExtendSwPcr_0_1(u32 *Pload, u32 Len);
int XOcp_GetSwPcrLog(u64 Addr);
int XOcp_GetSwPcrData(u64 Addr);
int XOcp_GetSwPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize);
int XOcp_CheckAndExtendSecureState(void);
int XOcp_MeasureSecureStateAndExtendSwPcr(void);

#endif /* PLM_OCP */

#ifdef __cplusplus
}
#endif

#endif  /* XOCP_PCR_H */
