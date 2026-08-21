/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_memvalidation.h
 *
 * This file contains the definitions, structures and API prototype for validating a client
 * provided address range.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   rmv  07/20/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_MEMVALIDATION_H_
#define XASUFW_MEMVALIDATION_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_util.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/
/* Memory region access table related macros */
#define XASUFW_MEM_REGN_MAX_ENTRIES		(64U)		/**< Maximum number of regions the
								table can hold */

/************************************** Type Definitions *****************************************/
/**
 * This structure contains the attributes, size, and start address of a memory region.
 */
typedef struct {
	u32 Attr;	/**< State [31:28] | MemFlags [27:24] | Reserved [23:16] | AccessPerm [15:0] */
	u32 Size;	/**< Region size in 4 KB units (ActualBytes = Size << 12) */
	u32 AddrHigh;	/**< Region start address bits [63:32] */
	u32 AddrLow;	/**< Region start address bits [31:0] */
} XAsufw_MemRegnEntry;

/**
 * This structure contains the header and the region entries of the memory region access table.
 */
typedef struct {
	u32 Header;		/**< Header of the memory region access table */
	u32 UpdateCounter;	/**< Seqlock counter maintained by PLM (odd while updating) */
	XAsufw_MemRegnEntry Entries[XASUFW_MEM_REGN_MAX_ENTRIES];	/**< Region entries */
} XAsufw_MemRegnTable;

/*************************** Macros (Inline Functions) Definitions *******************************/
/**
 * Macro to validate that the client provided buffer of the given size lies entirely within a
 * memory region which the requesting subsystem is permitted to access.
 */
#define XASUFW_VALIDATE_MEM_ACCESS(Label, Status, SubsystemId, Addr, Size) \
	do { \
		XFih_Var XFihMemRegnVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE); \
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE); \
		XFIH_CALL_GOTO(XAsufw_ValidateMemRegnAccess, XFihMemRegnVar, Status, Label, \
			       (SubsystemId), (u64)(Addr), (u64)(Size)); \
	} while (0)

/************************************ Function Prototypes ****************************************/
s32 XAsufw_ValidateMemRegnAccess(u32 SubsystemId, u64 Address, u64 Size);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_MEMVALIDATION_H_ */
/** @} */
