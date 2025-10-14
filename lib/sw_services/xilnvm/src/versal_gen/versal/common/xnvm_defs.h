/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal/common/xnvm_defs.h
*
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
* 3.3	vss  02/23/24 Added IPI support for eFuse read and write
*	vss  05/20/24 Added IPI support for AES key write
*       ng   11/22/24 Fixed doxygen grouping
*
* </pre>
*
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

#define XNVM_DEBUG	(0U)	/**< Enable client printfs by setting XNVM_DEBUG to 1 */

#if (XNVM_DEBUG)
#define XNVM_DEBUG_GENERAL (1U)	/**< Enable debug messages */
#else
#define XNVM_DEBUG_GENERAL (0U)	/**< Disable debug messages */
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define XNvm_Printf(DebugType, ...)	\
	if ((DebugType) == 1U) {xil_printf (__VA_ARGS__);}	/**< Print debug messages */

#ifndef XNVM_CACHE_DISABLE
	#if defined(__microblaze__)
		#define XNvm_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((UINTPTR)SrcAddr, Len)
		/**< Flush the data cache for a specific range */
	#else
		#define XNvm_DCacheFlushRange(SrcAddr, Len) Xil_DCacheFlushRange((INTPTR)SrcAddr, Len)
		/**< Flush the data cache for a specific range */
	#endif
#else
	#define XNvm_DCacheFlushRange(SrcAddr, Len) {}
#endif /**< Cache Invalidate function */

#define XNVM_API(ApiId)	((u32)ApiId)	/**< Macro to typecast API ID */

#define XNVM_API_ID_MASK	(0xFFU)	/**< API ID Mask*/

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
#ifdef XNVM_ACCESS_PUF_USER_DATA
/**
 * @brief PUF user data
 */
typedef struct {
	u64 PufFuseDataAddr;	/**< PufUserFuseData Address */
	u32 StartPufFuseRow;	/**< Start Puf user eFuse row number */
	u32 NumOfPufFusesRows;	/**< Number of Puf user eFuses to be programmed */
	u8 EnvMonitorDis;	/**< Environmentol Monitor disable flag */
	u8 PrgmPufFuse;		/**< Program flag TRUE/FALSE */
} XNvm_EfusePufFuseAddr;
#else
/**
 * @brief PUF helper data
 */
typedef struct {
	XNvm_EfusePufSecCtrlBits PufSecCtrlBits;/**< PufSecCtrlBits Data */
	u8 PrgmPufHelperData;	/**< Program flag for PUF HD TRUE/FALSE */
	u8 PrgmPufSecCtrlBits;	/**< Program flag for PUF SecCtrlBits TRUE/FALSE */
	u8 EnvMonitorDis;	/**< Environmentol Monitor disable flag */
	u32 EfuseSynData[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS];	/**< Formatted PUF HD */
	u32 Chash;		/**< Chash value to be programmed to Chash eFuses */
	u32 Aux;		/**< Aux value to be programmed to AUX eFuses */
}XNvm_EfusePufHdAddr;
#endif

#ifdef XNVM_WRITE_SECURITY_CRITICAL_EFUSE
/**
 * @brief Efuse PUF Syndrome data
 */
typedef struct {
	u32 Chash;	/**< Chash value to be programmed to Chash eFuses */
	u32 Aux;	/**< Aux value to be programmed to AUX eFuses */
	u32 EfuseSynData[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS];/**< Formatted PUF HD */
} XNvm_EfusePufData;
#endif

#ifdef XNVM_EN_ADD_PPKS
/**
 * @brief Additional PPK Hash
 */
typedef struct {
	u8 PrgmPpk3Hash; /**< Program flag for PPK3 Hash TRUE/FALSE */
	u8 PrgmPpk4Hash; /**< Program flag for PPK4 Hash TRUE/FALSE */
	u32 Ppk3Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS]; /**< Data to be programmed to PPK3 Hash eFuses */
	u32 Ppk4Hash[XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS]; /**< Data to be programmed to PPK4 Hash eFuses */
} XNvm_EfuseAdditionalPpkHash;
#endif

/**
 * @brief API ids, IDs ranging from an enum value of 24 to 35 are used by IPI
 */
typedef enum {
	XNVM_API_FEATURES = 0,			/**< 0U */
	XNVM_API_ID_BBRAM_WRITE_AES_KEY,	/**< 1U */
	XNVM_API_ID_BBRAM_ZEROIZE,		/**< 2U */
	XNVM_API_ID_BBRAM_WRITE_USER_DATA,	/**< 3U */
	XNVM_API_ID_BBRAM_READ_USER_DATA,	/**< 4U */
	XNVM_API_ID_BBRAM_LOCK_WRITE_USER_DATA,	/**< 5U */
	XNVM_API_ID_EFUSE_WRITE,		/**< 6U */
	XNVM_API_ID_EFUSE_WRITE_PUF,		/**< 7U */
	XNVM_API_ID_EFUSE_PUF_USER_FUSE_WRITE,	/**< 8U */
	XNVM_API_ID_EFUSE_READ_IV,		/**< 9U */
	XNVM_API_ID_EFUSE_READ_REVOCATION_ID,	/**< 10U */
	XNVM_API_ID_EFUSE_READ_OFFCHIP_REVOCATION_ID,	/**< 11U */
	XNVM_API_ID_EFUSE_READ_USER_FUSES,	/**< 12U */
	XNVM_API_ID_EFUSE_READ_MISC_CTRL_BITS,	/**< 13U */
	XNVM_API_ID_EFUSE_READ_SEC_CTRL_BITS,	/**< 14U */
	XNVM_API_ID_EFUSE_READ_SEC_MISC1_BITS,	/**< 15U */
	XNVM_API_ID_EFUSE_READ_BOOT_ENV_CTRL_BITS,	/**< 16U */
	XNVM_API_ID_EFUSE_READ_PUF_SEC_CTRL_BITS,	/**< 17U */
	XNVM_API_ID_EFUSE_READ_PPK_HASH,	/**< 18U */
	XNVM_API_ID_EFUSE_READ_DEC_EFUSE_ONLY,	/**< 19U */
	XNVM_API_ID_EFUSE_READ_DNA,		/**< 20U */
	XNVM_API_ID_EFUSE_READ_PUF_USER_FUSE,	/**< 21U */
	XNVM_API_ID_EFUSE_READ_PUF,		/**< 22U */
	XNVM_API_ID_EFUSE_READ_CACHE,		/**< 23U */
	XNVM_API_ID_EFUSE_WRITE_IV,		/**< 24U */
	XNVM_API_ID_EFUSE_WRITE_SECURITY_MISC1,	/**< 25U */
	XNVM_API_ID_EFUSE_WRITE_PUF_DATA,	/**< 26U */
	XNVM_API_ID_EFUSE_WRITE_OFF_CHIP_ID,	/**< 27U */
	XNVM_API_ID_EFUSE_WRITE_USER_EFUSE,	/**< 28U */
	XNVM_API_ID_EFUSE_WRITE_REVOCATION_ID,	/**< 29U */
	XNVM_API_ID_EFUSE_WRITE_PPK_HASH,	/**< 30U */
	XNVM_API_ID_EFUSE_WRITE_ANLG_TRIM,	/**< 31U */
	XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL,	/**< 32U */
	XNVM_API_ID_EFUSE_WRITE_MISC_CTRL,	/**< 33U */
	XNVM_API_ID_EFUSE_WRITE_SECURITY_CTRL,	/**< 34U */
	XNVM_API_ID_EFUSE_WRITE_SECURITY_MISC0_CTRL,	/**< 35U */
	XNVM_API_ID_EFUSE_WRITE_AES_KEYS,	/**< 36U */
	XNVM_API_MAX,				/**< 37U */
} XNvm_ApiId;

#ifdef __cplusplus
}
#endif

#endif  /* XNVM_DEFS_H */
