/**************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xloader_plat_client.h
 *
 * This file Contains the client function prototypes, defines and macros.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  gnr  02/09/26 Initial release
 * 2.40  sri  03/26/26 Added client API to validate authenticated PDI
 * 2.5   ng   08/17/26 Removed XLOADER_PDI_SRC_DDR and XLOADER_PDI_SRC_IS macros
 *       ng   08/23/26 Added XLoader_ConfigureGlitchDetector client API
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XLOADER_PLAT_CLIENT_H
#define XLOADER_PLAT_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

#include "xil_types.h"
#include "xloader_mailbox.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/
/**
 * Structure to hold DDR Counters
 */
typedef struct {
	u32 DDRCounter0; /**< DDR Counter0 */
	u32 DDRCounter1; /**< DDR Counter1 */
	u32 DDRCounter2; /**< DDR Counter2 */
	u32 DDRCounter3; /**< DDR Counter3 */
} XLoader_DDRCounters;

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XLOADER_PDI_TYPE_FULL		        (0x1U)  /**< PDI type full */
#define XLOADER_PDI_TYPE_PARTIAL	        (0x2U)  /**< PDI type partial */

/* Discrete glitch detector config word counts */
#define XLOADER_GD_CFG_LEN_1			(1U) /**< 1 GD config word */
#define XLOADER_GD_CFG_LEN_2			(2U) /**< 2 GD config words */
#define XLOADER_GD_CFG_LEN_3			(3U) /**< 3 GD config words */
#define XLOADER_GD_CFG_LEN_4			(4U) /**< 4 GD config words */
#define XLOADER_GD_CFG_LEN_5			(5U) /**< 5 GD config words */

/*************************************************************************************************/
/**
 * @brief	Pack one glitch-detector config word for XLOADER_CMD_ID_CONFIG_GLITCH_DETECTOR.
 *
 * @param	GdType		Detector type (XLOADER_GD_TYPE_*)
 * @param	Depth		Detection depth
 * @param	Width		Detection width
 * @param	RefVoltage	Reference voltage select
 *
 * @return	Packed payload word (LSB-first u8 fields)
 *
 *************************************************************************************************/
 static inline u32 XLoader_PackGlitchDetectorCfg(u8 GdType, u8 Depth, u8 Width, u8 RefVoltage)
 {
	 return (((u32)GdType & XLOADER_GD_FIELD_MASK) << XLOADER_GD_TYPE_SHIFT) |
		(((u32)Depth & XLOADER_GD_FIELD_MASK) << XLOADER_GD_DEPTH_SHIFT) |
		(((u32)Width & XLOADER_GD_FIELD_MASK) << XLOADER_GD_WIDTH_SHIFT) |
		(((u32)RefVoltage & XLOADER_GD_FIELD_MASK) << XLOADER_GD_REFVOL_SHIFT);
 }

/************************************ Function Prototypes ****************************************/
int XLoader_ConfigureJtagState(XLoader_ClientInstance *InstancePtr, u32 Flag);
int XLoader_ReadDdrCryptoPerfCounters(XLoader_ClientInstance *InstancePtr, u32 NodeId,
		XLoader_DDRCounters *CryptoCounters);
int XLoader_ValidatePdiAuth(XLoader_ClientInstance *InstancePtr, const u64 PdiAddr, const u32 PdiType);
int XLoader_ConfigureGlitchDetector(XLoader_ClientInstance *InstancePtr, const u32 *GdCfgWords,
				    u32 NumWords);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_PLAT_CLIENT_H */
