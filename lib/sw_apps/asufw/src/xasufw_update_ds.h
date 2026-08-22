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
 *       vm   07/30/26 Export default database handler API
 *       vm   08/22/26 Added ASU data structure version metadata export and aligned the version
 *                     record field order with XPlmi_DsVer
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
#define XASUFW_DS_DSID_SHIFT		(8U)	/**< Data structure ID bit shift position */
#define XASUFW_DS_VERSION_SHIFT		(16U)	/**< Version bit shift position */
#define XASUFW_DS_LCVERSION_SHIFT	(24U)	/**< Lowest compatible version bit shift position */

/************************************** Type Definitions *****************************************/
/**
 * This typedef contains data structure version information. The field order matches XPlmi_DsVer
 * as the record is written into the ASU data structure information optional data of the PDI.
 */
typedef struct {
	u8 ModuleId;		/**< Module ID. */
	u8 DsId;		/**< Data Structure ID. */
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
#define XASUFW_PACK_DS_VER(ModuleId, DsId, Version, LcVersion) \
	(((u32)(ModuleId)) | ((u32)(DsId) << XASUFW_DS_DSID_SHIFT) | \
	 ((u32)(Version) << XASUFW_DS_VERSION_SHIFT) | \
	 ((u32)(LcVersion) << XASUFW_DS_LCVERSION_SHIFT))

/**
 * Section attribute for the data structure entries used by the store and restore flow.
 */
#define XASUFW_DSENTRY_ATTR	__attribute__((used)) \
				__attribute__((section(".struct_entries")))

/**
 * Section attribute for the data structure version records which are added to the ASU data
 * structure information optional data of the image header table while generating the PDI.
 */
#define XASUFW_DSVER_ATTR	__attribute__((used)) \
				__attribute__((section(".struct_versions")))

/**
 * Macro to export data structures for update functionality. It emits the data structure version
 * record and the data structure entry.
 */
#define EXPORT_ASUFW_DS(VarName, MId, DId, DsVer, LcVer, Size, VarAddr) \
	static XAsufw_DsVer XASUFW_DSVER_ATTR \
	VarName##_DsInfo = { \
		.ModuleId = (MId), \
		.DsId = (DId), \
		.Version = (DsVer), \
		.LcVersion = (LcVer) \
	}; \
	static XAsufw_DsEntry XASUFW_DSENTRY_ATTR \
	VarName##_DsEntry = { \
		.DsHdr = { \
			.Ver = { \
				.ModuleId = (MId), \
				.DsId = (DId), \
				.Version = (DsVer), \
				.LcVersion = (LcVer) \
			}, \
			.Len = (Size) \
		}, \
		.Addr = (VarAddr), \
		.Handler = XAsufw_DsOps \
	}

/************************************ Function Prototypes ****************************************/
s32 XAsufw_DsOps(u32 Op, u64 Addr, void *Data);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_UPDATE_DS_H_ */
/** @} */
