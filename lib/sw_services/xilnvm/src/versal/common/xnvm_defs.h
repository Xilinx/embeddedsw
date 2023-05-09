/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/common/xnvm_defs.h
* @addtogroup xnvm_versal_api_ids XilNvm Versal API IDs
* @{
*
* @cond xnvm_internal
* This file contains the xilnvm API IDs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  07/05/21 Initial release
*       kal  07/25/21 Added eFUSE IPI API_IDs and common structures between
*                     client and server
*       kpt  08/27/21 Added client-server support for puf helper data efuse
*                     programming
* 1.1   kpt  11/29/21 Added macro XNvm_DCacheFlushRange
*       har  01/03/22 Renamed NumOfPufFuses as NumOfPufFusesRows
*       am   02/28/22 Fixed MISRA C violation rule 4.5
*       kpt  03/03/22 Fixed alignment issue in XNvm_EfusePufFuseAddr
*                     by rearranging the structure elements
* 3.1   skg  10/28/22 Added comments
*
* </pre>
* @note
*
* @endcond
******************************************************************************/

#ifndef XNVM_DEFS_H
#define XNVM_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xil_types.h"
#include "xnvm_common_defs.h"

/************************** Constant Definitions ****************************/
/**@cond xnvm_internal
 * @{
 */
/**< Enable client printfs by setting XNVM_DEBUG to 1 */
#define XNVM_DEBUG	(0U)

#if (XNVM_DEBUG)
#define XNVM_DEBUG_GENERAL (1U)
#else
#define XNVM_DEBUG_GENERAL (0U)
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define XNvm_Printf(DebugType, ...)	\
	if ((DebugType) == 1U) {xil_printf (__VA_ARGS__);}

#ifndef XNVM_CACHE_DISABLE
	#if defined(__microblaze__)
		#define XNvm_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((UINTPTR)SrcAddr, Len)
	#else
		#define XNvm_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((INTPTR)SrcAddr, Len)
	#endif
#else
	#define XNvm_DCacheFlushRange(SrcAddr, Len) {}
#endif /**< Cache Invalidate function */

/**< Macro to typecast XILSECURE API ID */
#define XNVM_API(ApiId)	((u32)ApiId)

#define XNVM_API_ID_MASK	(0xFFU)

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
#ifdef XNVM_ACCESS_PUF_USER_DATA
typedef struct {
	u64 PufFuseDataAddr;
	u32 StartPufFuseRow;
	u32 NumOfPufFusesRows;
	u8 EnvMonitorDis;
	u8 PrgmPufFuse;
} XNvm_EfusePufFuseAddr;
#else
/**< Puf helper data*/
typedef struct {
	XNvm_EfusePufSecCtrlBits PufSecCtrlBits;
	u8 PrgmPufHelperData;
	u8 PrgmPufSecCtrlBits;
	u8 EnvMonitorDis;
	u32 EfuseSynData[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS];
	u32 Chash;
	u32 Aux;
}XNvm_EfusePufHdAddr;
#endif

#ifdef XNVM_EN_ADD_PPKS
typedef struct {
	u8 PrgmPpk3Hash;
	u8 PrgmPpk4Hash;
	u32 Ppk3Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
	u32 Ppk4Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS];
} XNvm_EfuseAdditionalPpkHash;
#endif

/**< XilNVM API ids */
typedef enum {
	XNVM_API_FEATURES = 0,
	XNVM_API_ID_BBRAM_WRITE_AES_KEY,
	XNVM_API_ID_BBRAM_ZEROIZE,
	XNVM_API_ID_BBRAM_WRITE_USER_DATA,
	XNVM_API_ID_BBRAM_READ_USER_DATA,
	XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA,
	XNVM_API_ID_EFUSE_WRITE,
	XNVM_API_ID_EFUSE_WRITE_PUF,
	XNVM_API_ID_EFUSE_PUF_USER_FUSE_WRITE,
	XNVM_API_ID_EFUSE_READ_IV,
	XNVM_API_ID_EFUSE_READ_REVOCATION_ID,
	XNVM_API_ID_EFUSE_READ_OFFCHIP_REVOCATION_ID,
	XNVM_API_ID_EFUSE_READ_USER_FUSES,
	XNVM_API_ID_EFUSE_READ_MISC_CTRL_BITS,
	XNVM_API_ID_EFUSE_READ_SEC_CTRL_BITS,
	XNVM_API_ID_EFUSE_READ_SEC_MISC1_BITS,
	XNVM_API_ID_EFUSE_READ_BOOT_ENV_CTRL_BITS,
	XNVM_API_ID_EFUSE_READ_PUF_SEC_CTRL_BITS,
	XNVM_API_ID_EFUSE_READ_PPK_HASH,
	XNVM_API_ID_EFUSE_READ_DEC_EFUSE_ONLY,
	XNVM_API_ID_EFUSE_READ_DNA,
	XNVM_API_ID_EFUSE_READ_PUF_USER_FUSE,
	XNVM_API_ID_EFUSE_READ_PUF,
	XNVM_API_MAX,
} XNvm_ApiId;

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_DEFS_H */
