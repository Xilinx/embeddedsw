/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_update_ds.h
 *
 * This file contains data structure definitions for ASUFW update functionality.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vm   03/16/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_UPDATE_DS_H_
#define XASUFW_UPDATE_DS_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/
/* Bit shift positions for packing data structure version fields */
#define XASUFW_DS_MODULEID_SHIFT	(8U)	/**< Module ID bit shift position */
#define XASUFW_DS_VERSION_SHIFT		(16U)	/**< Version bit shift position */
#define XASUFW_DS_LCVERSION_SHIFT	(24U)	/**< Lowest compatible version bit shift position */

/************************************** Type Definitions *****************************************/
/**
 * This typedef contains data structure version information.
 */
typedef struct {
	u8 DsId;		/**< Data Structure ID. */
	u8 ModuleId;		/**< Module ID. */
	u8 Version;		/**< Version. */
	u8 LcVersion;		/**< Lowest compatible version. */
} XAsufw_DsVer;

/**
 * This typedef contains data structure header information.
 */
typedef struct {
	XAsufw_DsVer Ver;		/**< Version information. */
	u32 Len;			/**< Length of data structure. */
} XAsufw_DsHdr;

/**
 * This typedef contains database header information.
 */
typedef struct {
	u8 HdrVersion;			/**< Header version. */
	u8 HdrSize;			/**< Header size. */
	u16 Reserved;			/**< Reserved field. */
	u32 DbSize;			/**< Database size in words. */
} XAsufw_DbHdr;

/**
 * This typedef contains data structure entry information.
 */
typedef struct {
	XAsufw_DsHdr DsHdr;		/**< Data structure header. */
	u32 Addr;			/**< Address of data structure. */
	s32 (*Handler)(u32 Op, u64 Addr, void *Data);	/**< Handler function. */
} XAsufw_DsEntry;

/*************************** Macros (Inline Functions) Definitions *******************************/
/**
 * Macro to pack XAsufw_DsVer fields into a u32 value.
 */
#define XASUFW_PACK_DS_VER(DsId, ModuleId, Version, LcVersion) \
	(((u32)(DsId)) | ((u32)(ModuleId) << XASUFW_DS_MODULEID_SHIFT) | \
	 ((u32)(Version) << XASUFW_DS_VERSION_SHIFT) | \
	 ((u32)(LcVersion) << XASUFW_DS_LCVERSION_SHIFT))

/**
 * Macro to export data structures for update functionality.
 */
#define EXPORT_ASUFW_DS(VarName, MId, DId, DsVer, LcVer, Size, VarAddr) \
	static XAsufw_DsEntry __attribute__((used)) __attribute__((section(".struct_entries")))\
	VarName##_DsEntry = { \
		.DsHdr = { \
			.Ver = { \
				.DsId = DId, \
				.ModuleId = MId, \
				.Version = DsVer, \
				.LcVersion = LcVer \
			}, \
			.Len = Size \
		}, \
		.Addr = VarAddr, \
		.Handler = XAsufw_DsOps \
	}

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_UPDATE_DS_H_ */
/** @} */
