/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc.  All rights reserved.
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

/**************************** Type Definitions *******************************/
/*
 * SW PCR Config
 */
typedef struct {
	u8 DigestsForPcr[XOCP_NUM_OF_SWPCRS];		/**< Max digests for each SW PCR */
	u8 PcrDigestsConfigured[XOCP_NUM_OF_SWPCRS];	/**< Configured no of digests */
	u8 PcrIdxInLog[XOCP_NUM_OF_SWPCRS];		/**< Each SW PCR index in log */
	u8 IsPcrConfigReceived;				/**< SW PCR config received */
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

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
int XOcp_ExtendHwPcr(XOcp_HwPcr PcrNum, u64 ExtHashAddr, u32 DataSize);
int XOcp_GetHwPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize);
int XOcp_GetHwPcrLog(u64 HwPcrEventsAddr, u64 HwPcrLogInfoAddr, u32 NumOfLogEntries);
int XOcp_GenerateDmeResponse(u64 NonceAddr, u64 DmeStructResAddr);
int XOcp_ExtendSwPcr(u32 PcrNum, u32 MeasurementIdx, u64 DataAddr, u32 DataSize, u32 PdiType);
int XOcp_StoreSwPcrConfig(u32 *Pload, u32 Len);
int XOcp_GetSwPcrLog(u64 Addr);
int XOcp_GetSwPcrData(u64 Addr);
int XOcp_GetSwPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize);

#ifdef __cplusplus
}
#endif
#endif  /* XOCP_H */
