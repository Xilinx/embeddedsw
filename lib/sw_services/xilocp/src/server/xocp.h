/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp.h
* @addtogroup xil_ocpapis APIs
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
#include "xstatus.h"
#include "xocp_common.h"
#include "xplmi_debug.h"

/************************** Constant Definitions *****************************/
#define XOCP_XPPU_MASTER_ID0_PPU0_CONFIG_VAL		(0x03FF0246U)
							/**< PPU0 SMID */
#define XOCP_XPPU_MASTER_ID1_PPU1_CONFIG_VAL		(0x03FF0247U)
							/**< PPU1 SMID */
#define XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL		(0x10000001U)
							/**< PPU0 configuration value */
#define XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL	(0x00000003U)
							/**< PPU1 configuration value */
#define XOCP_XPPU_DYNAMIC_RECONFIG_APER_SET_VALUE	(0x31U)
							  /**< Dynamic reconfiguration set value */
#define XOCP_PMC_XPPU_CTRL_ENABLE_VAL			(0x1U) /**< XPPU enable value */
#define XOCP_PMC_XPPU_CTRL_DISABLE_VAL			(0x0U) /**< XPPU disable value */
#define XOcp_MemCopy								XPlmi_DmaXfr
							/**< Data transfer through DMA */
#define XOcp_Printf								XPlmi_Printf
							/**< XILOCP print function */
#define XOCP_WORD_LEN					(0x4U) /**< Word length */
#define XOCP_PCR_NUMBER_MASK 				(0x0000FFFFU) /**< Number mask value */
#define XOCP_PCR_MEASUREMENT_INDEX_MASK 		(0xFFFF0000U)
							 /**< Measurement index mask value */
#define XOCP_PCR_INVALID_VALUE 				(0xFFFFFFFFU) /**< PCR invalid value */
#define XOCP_PCR_HASH_SIZE_IN_BYTES			(48U) /**< Hash size in bytes */

#define XOCP_EFUSE_CACHE_BOOT_ENV_CTRL			(0xF1250094U) /**< Boot environmental register address */
#define XOCP_PMC_LOCAL_BOOT_MODE_DIS			(0xF00441D0U) /**< Boot mode register address */
#define XOCP_EFUSE_CACHE_MISC_CTRL				(0xF12500A0U) /**< MISC control register address */
#define XOCP_EFUSE_CACHE_ANLG_TRIM_3			(0xF1250010U) /**< Analog Trim3 registe address */
#define XOCP_EFUSE_CACHE_IP_DISABLE_0			(0xF1250018U) /**< IP disable 0 register address */
#define XOCP_EFUSE_CACHE_IP_DISABLE_1			(0xF125001CU) /**< IP disable 1 register address */
#define XOCP_EFUSE_CACHE_CAHER_1				(0xF12500F0U) /**< Caher1 register address */
#define XOCP_EFUSE_CACHE_SECURITY_MISC_0		(0xF12500E4U) /**< MISC 0 control register address */
#define XOCP_EFUSE_CACHE_SECURITY_CONTROL		(0xF12500ACU) /**< security control register address */
#define XOCP_EFUSE_CACHE_SECURITY_MISC_1		(0xF12500E8U) /**< security misc 1 control register address */
#define XOCP_EFUSE_CACHE_DME_FIPS				(0xF1250234U) /**< DME FIPS register address */
#define XOCP_EFUSE_CACHE_ROM_RSVD				(0xF1250090U) /**< ROM reseved register address */
#define XOCP_EFUSE_CACHE_RO_SWAP_EN				(0xF12500D0U) /**< RO SWAP enable address */

#define XOCP_CAHER_1_MEASURED_MASK				(0x00000F00U) /**< HNIC DIS| DDR XTS export
																	| HNIC DDR export | DDR XTS GCM DIS */
#define XOCP_DEC_ONLY_MEASURED_MASK				(0x0000FFFFU) /**< Decrypt only mask */
#define XOCP_SEC_CTRL_MEASURED_MASK				(0x03FF001FU) /**< Security control measured mask */
#define XOCP_PMC_LOCAL_BOOT_MODE_DIS_FULLMASK	(0x0000FFFFU) /**< PMC local disable mask */
#define XOCP_MISC_CTRL_MEASURED_MASK			(0xE018C100U) /**< MISC control mask */
#define XOCP_DME_FIPS_MEASURED_MASK				(0xFF00000FU) /**< DME FIPS mask */
#define XOCP_IP_DISABLE0_MEASURED_MASK			(0xF0000F04U) /**< IP disable mask */
#define XOCP_ROM_RSVD_MEASURED_MASK				(0x000007C0U) /**< ROM reserved mask */
#define XOCP_PMC_TAP_DAP_CFG_OFFSET				(0xF11B0008U) /**< DAP CFG register address */
#define XOCP_PMC_TAP_INST_MASK_0_OFFSET			(0xF11B0000U) /**< Instruction Mask 0 register address */
#define XOCP_PMC_TAP_INST_MASK_1_OFFSET			(0xF11B0004U) /**< Instruction Mask 1 register address */
#define XOCP_PMC_TAP_DAP_SECURITY_OFFSET		(0xF11B000CU) /**< DAP security register address */

#define XOCP_PMC_PLM_HASH_ADDR					(0xF1110750U) /**< PLM hash address */
#define XOCP_PMC_ROM_HASH_ADDR					(0xF1110704U) /**< ROM hash address */

#define XOCP_SW_PCR_NUM_0  (0U)    /**< SW PCR number 0 */
#define XOCP_SW_PCR_NUM_1  (1U)    /**< SW PCR number 1 */
#define XOCP_SW_PCR_SEC_STATE_MEASUREMENT_IDX  (0U) /**< Measurement index for secure state extension */

/**************************** Type Definitions *******************************/
/*
 * SW PCR Config
 */
typedef struct {
	u8 DigestsForPcr[XOCP_NUM_OF_SWPCRS];		/**< Max digests for each SW PCR */
	u8 PcrDigestsConfigured[XOCP_NUM_OF_SWPCRS];	/**< Configured no of digests */
	u8 PcrIdxInLog[XOCP_NUM_OF_SWPCRS];		/**< Each SW PCR index in log */
	u8 IsPcrConfigReceived;				/**< SW PCR config received */
	u8 Reserved[3U];                    /**< Reserved bytes */
} XOcp_SwPcrConfig;

/*
 * SW PCR Data per event
 */
typedef struct {
	XOcp_PcrMeasurement Measurement;		/**< XOcp_PcrMeasurement structure */
	u32 IsReqExtended;				/**< Is request extended */
	u64 DataAddr;					/**< Address of the data buffer */
	u8 DataToExtend[XOCP_PCR_SIZE_BYTES];		/**< Data to extend */
} XOcp_SwPcrData;

/*
 * SW PCR log
 */
typedef struct {
	XOcp_SwPcrData Data[XOCP_MAX_NUM_OF_SWPCRS];	/**< SW PCR log store with max no of events */
	u8 CountPerPcr[XOCP_NUM_OF_SWPCRS];		/**< Number of digests extended for each SW PCR */
} XOcp_SwPcrStore;

/*
 * Dme XPPU config
 */
typedef struct {
	u32 XppuAperAddr; /**< XPPU MASTER IDs and Aperture address */
	u32 XppuAperWriteCfgVal; /**< Required configurations */
	u32 XppuAperReadCfgVal; /**< Initial values of Apertures and Master IDs */
	u32 IsModified; /**< Is XPPU Configuration modified */
}XOcp_DmeXppuCfg;

typedef struct {
	u32 BootmodeDis;	/* BOOT_MODE_DIS_15_0 */
	u32 MiscCtrl;		/* MISC_CTRL */
	u32 AnlgTrim3; 		/* ANLG_TRIM_3 */
	u32 BootEnvCtrl;	/* BOOT_ENV_CTRL considering reserved bits */
	u32 IpDisable1;		/* IP_DISABLE_1 */
	u32 Caher1;			/* Caher_1 */
	u32 DecOnly;		/* DEC_ONLY */
	u32 SecCtrl;		/* Secure control */
	u32 SecMisc1;		/* SEC_MISC 1 */
	u32 DmeFips;		/* DME FIPS */
	u32 IPDisable0;		/* IP_DISABLE_0 */
	u32 RomRsvd;		/* ROM RSVD */
	u32 RoSwapEn;		/* RO_SWAP_EN */
} XOcp_SecureConfig;

typedef struct {
	u32 DapCfg;			/* Dap configuration */
	u32 InstMask0;		/* Inst mask 0 */
	u32 InstMask1;		/* Inst mask 1 */
	u32 DapSecurity;	/* DAP security */
	u32 BootDevice;		/* Boot Device */
} XOcp_SecureTapConfig;

typedef struct {
	u8 RomHash[XOCP_PCR_HASH_SIZE_IN_BYTES]; /* Rom hash */
	u8 PlmHash[XOCP_PCR_HASH_SIZE_IN_BYTES]; /* Plm hash */
	u8 SecureConfigHash[XOCP_PCR_HASH_SIZE_IN_BYTES]; /* Secure config hash */
	u8 TapConfigHash[XOCP_PCR_HASH_SIZE_IN_BYTES]; /* Tap config hash */
} XOcp_SecureStateHash;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
int XOcp_ExtendHwPcr(XOcp_HwPcr PcrNum, u64 ExtHashAddr, u32 DataSize);
int XOcp_GetHwPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize);
int XOcp_GetHwPcrLog(u64 HwPcrEventsAddr, u64 HwPcrLogInfoAddr, u32 NumOfLogEntries);
int XOcp_GenerateDmeResponse(u64 NonceAddr, u64 DmeStructResAddr);
int XOcp_ExtendSwPcr(u32 PcrNum, u32 MeasurementIdx, u64 DataAddr, u32 DataSize, u32 OverWrite);
int XOcp_StoreSwPcrConfigAndExtendSwPcr_0_1(u32 *Pload, u32 Len);
int XOcp_GetSwPcrLog(u64 Addr);
int XOcp_GetSwPcrData(u64 Addr);
int XOcp_GetSwPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize);
int XOcp_CheckAndExtendSecureState(void);
int XOcp_MeasureSecureStateAndExtendSwPcr(void);
XOcp_DmeResponse* XOcp_GetDmeResponse(void);
u32 XOcp_IsDmeChlAvail(void);

#ifdef __cplusplus
}
#endif
#endif /* PLM_OCP */
#endif  /* XOCP_H */
