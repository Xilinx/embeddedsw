/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_memvalidation.c
 *
 * This file contains the address range validation logic for ASUFW.
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
/*************************************** Include Files *******************************************/
#include "xasufw_memvalidation.h"
#include "xasufw_memory.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "sleep.h"

/************************************ Constant Definitions ***************************************/

/*
 * Memory region table header fields:
 *   Bits [7:0]   Version    - table format version
 *   Bits [11:8]  HeaderSize - header size in 32-bit words
 *   Bits [15:12] EntrySize  - size of each region entry in 32-bit words
 *   Bits [31:16] NumEntries - number of region entries in the table
 */
#define XASUFW_MEM_REGN_TABLE_VERSION		(0x01U)	/**< Supported table format version */
#define XASUFW_MEM_REGN_TABLE_HEADER_SIZE	(0x02U)	/**< Expected header size in words */
#define XASUFW_MEM_REGN_ENTRY_SIZE		(0x04U)	/**< Expected size of each entry in words */
#define XASUFW_MEM_REGN_VERSION_MASK		(0xFFU)	/**< Version field mask */
#define XASUFW_MEM_REGN_HEADER_SIZE_SHIFT	(8U)	/**< Header size field shift */
#define XASUFW_MEM_REGN_HEADER_SIZE_MASK	(0x0FU)	/**< Header size field mask */
#define XASUFW_MEM_REGN_ENTRY_SIZE_SHIFT	(12U)	/**< Entry size field shift */
#define XASUFW_MEM_REGN_ENTRY_SIZE_MASK		(0x0FU)	/**< Entry size field mask */
#define XASUFW_MEM_REGN_NUM_ENTRIES_SHIFT	(16U)	/**< Number of entries field shift */
#define XASUFW_MEM_REGN_NUM_ENTRIES_MASK	(0xFFFFU)	/**< Number of entries field mask */

/*
 * Memory region attribute fields:
 *   Bits [31:28] State    - state of the region
 *   Bits [27:24] MemFlags - memory flags
 *   Bits [23:16] Reserved - reserved bits
 *   Bits [15:0]  AccessPerm - access permissions
 */
#define XASUFW_MEM_REGN_STATE_SHIFT		(28U)		/**< State field shift */
#define XASUFW_MEM_REGN_STATE_MASK		(0x0FU)		/**< State field mask */
#define XASUFW_MEM_REGN_STATE_ACTIVE		(0x01U)		/**< State value indicating the
								* region is physically reachable */

#define XASUFW_MEM_REGN_SUBSYSTEM_BIT_MASK	(0x3FFFU) /**< Mask to derive the AccessPerm bit
							index from a subsystem ID */
#define XASUFW_MEM_REGN_MAX_SUBSYSTEM_BIT	(15U)	/**< Highest AccessPerm bit index that can
							be represented in the 16-bit field */
#define XASUFW_MEM_REGN_SIZE_TO_BYTES_SHIFT	(12U)	/**< Region size is stored in 4 KB units
							(ActualBytes = Size << 12) */

#define XASUFW_MEM_REGN_TABLE_MAX_RETRIES	(100U)	/**< Maximum seqlock retries while obtaining
							a consistent table snapshot */
#define XASUFW_MEM_REGN_RETRY_DELAY_US		(5U)	/**< Wait before each retry, so the retry
							limit covers a table update lasting up to
							500 us (100 retries of 5 us) */

/*
 * The table is read in place from a PMC RAM region that PLM also writes, so the number of entries
 * this view can address must stay within the reservation.
 */
_Static_assert(sizeof(XAsufw_MemRegnTable) <= XASUFW_MEM_REGN_TABLE_SIZE_BYTES,
	       "XAsufw_MemRegnTable must fit the PMC RAM region reserved for the table");

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_CheckMemRegnAccess(u32 SubsystemId, u64 Address, u64 Size);
static s32 XAsufw_GetMemRegnNumEntries(const volatile XAsufw_MemRegnTable *Table, u32 *NumEntries);
static s32 XAsufw_SearchMemRegnTable(const volatile XAsufw_MemRegnTable *Table, u32 SubsystemId,
				     u64 Address, u64 Size, u32 NumEntries);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function validates the access to the given address range.
 *
 * @param	SubsystemId	Subsystem ID.
 * @param	Address		Start address of the client buffer to be validated.
 * @param	Size		Size of the client buffer in bytes.
 *
 * @return
 *	- XASUFW_SUCCESS, if size is zero or subsystem ID is the default and the range is valid.
 *	- XASUFW_FAILURE, in case of failure.
 *
 *************************************************************************************************/
s32 XAsufw_ValidateMemRegnAccess(u32 SubsystemId, u64 Address, u64 Size)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);

	/**
	 * A zero length buffer is never accessed, so there is no range to validate. For default
	 * subsystem, there is no access control boundary so no need to validate client buffer.
	 */
	if ((Size != 0U) && (SubsystemId != XASUFW_DEFAULT_SUBSYSTEM_ID)) {
		Status = XAsufw_CheckMemRegnAccess(SubsystemId, Address, Size);
	} else {
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function checks the given address range against the memory region access table.
 *
 * @param	SubsystemId	Subsystem ID.
 * @param	Address		Start address of the client buffer to be validated.
 * @param	Size		Size of the client buffer in bytes, which is never zero.
 *
 * @return
 *	- XASUFW_SUCCESS, if the entire range is inside an active region that grants access to the
 *	  requesting subsystem.
 *	- XASUFW_MEM_REGN_INVALID_PARAM, if the range overflows the 64-bit address space.
 *	- XASUFW_MEM_REGN_TABLE_INVALID, if the table header is invalid or the table is absent.
 *	- XASUFW_SEARCH_MEM_REGN_FAILED, if the search for the matching region failed.
 *	- XASUFW_MEM_REGN_TABLE_SYNC_FAIL, if a consistent snapshot could not be obtained.
 *
 *************************************************************************************************/
static s32 XAsufw_CheckMemRegnAccess(u32 SubsystemId, u64 Address, u64 Size)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	const volatile XAsufw_MemRegnTable *Table =
		(const volatile XAsufw_MemRegnTable *)(UINTPTR)XASUFW_MEM_REGN_TABLE_BASE_ADDR;
	u32 CounterBefore = 0U;
	u32 CounterAfter = 0U;
	u32 NumEntries = 0U;
	u32 Idx;
	u32 IsStable = XASU_FALSE;

	/** Reject a range that wraps past the end of the 64-bit address space. */
	if ((Size - 1U) > (XASUFW_MAX_64BIT_ADDRESS - Address)) {
		Status = XASUFW_MEM_REGN_INVALID_PARAM;
		goto END;
	}

	/**
	 * PLM maintains UpdateCounter as a seqlock: it is made odd before the table is modified and
	 * even again afterwards. A snapshot is trusted only when the counter is even and unchanged
	 * across the whole read. Retry a bounded number of times to ride out an in-progress update,
	 * then fail closed if no consistent snapshot is obtained.
	 */
	for (Idx = 0U; Idx < XASUFW_MEM_REGN_TABLE_MAX_RETRIES; Idx++) {
		/** Get update counter before validation; retry if PLM is mid-update. */
		CounterBefore = Table->UpdateCounter;
		if (XASUFW_IS_ODD(CounterBefore) != 0U) {
			/** Wait while PLM is updating the table. */
			usleep(XASUFW_MEM_REGN_RETRY_DELAY_US);
			continue;
		}

		/** Get the number of region entries. */
		Status = XAsufw_GetMemRegnNumEntries(Table, &NumEntries);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_MEM_REGN_TABLE_INVALID);
			goto END;
		}

		/** Search the access table for the matching region. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
		Status = XAsufw_SearchMemRegnTable(Table, SubsystemId, Address, Size, NumEntries);
		if (Status != XASUFW_SUCCESS) {
			Status = XAsufw_UpdateErrorStatus(Status, XASUFW_SEARCH_MEM_REGN_FAILED);
		}

		/**
		 * Accept the result only if the table did not change during the read. Otherwise the
		 * snapshot may be updated, so discard it and retry.
		 */
		CounterAfter = Table->UpdateCounter;
		if (CounterBefore == CounterAfter) {
			IsStable = XASU_TRUE;
			break;
		}

		/** Assign failure status and retry the validation. */
		ASSIGN_VOLATILE(Status, XASUFW_FAILURE);

		/** Wait for the table to be updated. */
		usleep(XASUFW_MEM_REGN_RETRY_DELAY_US);
	}

	/** Fail closed if no consistent snapshot was obtained within the retry limit. */
	if (IsStable == XASU_FALSE) {
		Status = XASUFW_MEM_REGN_TABLE_SYNC_FAIL;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function validates and gets the number of region entries from the access
 *		table header.
 *
 * @param	Table		Pointer to the access table.
 * @param	NumEntries	Pointer to store the number of region entries.
 *
 * @return
 *	- XASUFW_SUCCESS, if the header is valid.
 *	- XASUFW_FAILURE, if the header is invalid.
 *
 *************************************************************************************************/
static s32 XAsufw_GetMemRegnNumEntries(const volatile XAsufw_MemRegnTable *Table, u32 *NumEntries)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 HeaderWord = Table->Header;
	u32 Version = HeaderWord & XASUFW_MEM_REGN_VERSION_MASK;
	u32 HeaderSize = (HeaderWord >> XASUFW_MEM_REGN_HEADER_SIZE_SHIFT) &
				XASUFW_MEM_REGN_HEADER_SIZE_MASK;
	u32 EntrySize = (HeaderWord >> XASUFW_MEM_REGN_ENTRY_SIZE_SHIFT) &
				XASUFW_MEM_REGN_ENTRY_SIZE_MASK;
	u32 Entries = (HeaderWord >> XASUFW_MEM_REGN_NUM_ENTRIES_SHIFT) &
				XASUFW_MEM_REGN_NUM_ENTRIES_MASK;

	/** Validate the table header. */
	if ((Version == XASUFW_MEM_REGN_TABLE_VERSION) &&
	    (HeaderSize == XASUFW_MEM_REGN_TABLE_HEADER_SIZE) &&
	    (EntrySize == XASUFW_MEM_REGN_ENTRY_SIZE) &&
	    (Entries != 0U) && (Entries <= XASUFW_MEM_REGN_MAX_ENTRIES)) {
		/** Get the number of region entries. */
		*NumEntries = Entries;
		Status = XASUFW_SUCCESS;
	}

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function locates the region that contains the requested range and determines
 *		whether the range is permitted.
 *
 * @param	Table		Pointer to the access table.
 * @param	SubsystemId	Subsystem ID.
 * @param	Address		Start address of the client buffer.
 * @param	Size		Size of the client buffer in bytes.
 * @param	NumEntries	Number of region entries in the table.
 *
 * @note	The region entries must be sorted in ascending order of start address and do not
 *		overlap, so at most one region can contain the requested range. The requested range
 *		must lie entirely within that single region; a buffer that spans into a
 *		neighbouring region is refused even when both regions grant access.
 *
 * @return
 *	- XASUFW_SUCCESS, if a matching active region grants access to the subsystem.
 *	- XASUFW_MEM_REGN_REGION_INACTIVE, if the matching region is inactive.
 *	- XASUFW_MEM_REGN_INVALID_ACCESS_BIT, if the access bit index is invalid.
 *	- XASUFW_MEM_REGN_ACCESS_DENIED, if no matching region grants access to the subsystem.
 *
 *************************************************************************************************/
static s32 XAsufw_SearchMemRegnTable(const volatile XAsufw_MemRegnTable *Table, u32 SubsystemId,
				     u64 Address, u64 Size, u32 NumEntries)
{
	CREATE_VOLATILE(Status, XASUFW_MEM_REGN_ACCESS_DENIED);
	u32 AccessBit = SubsystemId & XASUFW_MEM_REGN_SUBSYSTEM_BIT_MASK;
	u64 ReqEnd = (Address + Size) - 1U;
	u32 Low = 0U;
	u32 High = NumEntries;
	u32 Mid;
	u32 CandIndex = NumEntries;
	u64 RegionStart = 0U;
	u64 RegionBytes = 0U;
	u64 RegionEnd = 0U;
	u32 Attr = 0U;
	XFih_Var XFihAttr = XFih_VolatileAssignU32(0U);

	/** Validate access bit index. */
	if (AccessBit > XASUFW_MEM_REGN_MAX_SUBSYSTEM_BIT) {
		Status = XASUFW_MEM_REGN_INVALID_ACCESS_BIT;
		goto END;
	}

	/**
	 * Get the region entry that contains the requested start address.
	 */
	while (Low < High) {
		Mid = Low + ((High - Low) / XASUFW_VALUE_TWO);
		RegionStart = ((u64)Table->Entries[Mid].AddrHigh << XASUFW_BITS_IN_WORD) |
				(u64)Table->Entries[Mid].AddrLow;
		if (RegionStart <= Address) {
			CandIndex = Mid;
			Low = Mid + 1U;
		} else {
			High = Mid;
		}
	}

	/** A candidate region exists only when some region starts at or below the address. */
	if (CandIndex < NumEntries) {
		RegionStart = ((u64)Table->Entries[CandIndex].AddrHigh << XASUFW_BITS_IN_WORD) |
				(u64)Table->Entries[CandIndex].AddrLow;
		RegionBytes = (u64)Table->Entries[CandIndex].Size <<
				XASUFW_MEM_REGN_SIZE_TO_BYTES_SHIFT;

		/**
		 * Ignore a zero size region or one whose inclusive end overflows the 64-bit space,
		 * then validate the requested range against the region boundaries.
		 */
		if ((RegionBytes != 0U) &&
		    ((RegionBytes - 1U) <= (XASUFW_MAX_64BIT_ADDRESS - RegionStart))) {
			RegionEnd = (RegionStart + RegionBytes) - 1U;

			if (ReqEnd <= RegionEnd) {
				/** Read the region attribute word. */
				Attr = Table->Entries[CandIndex].Attr;
				XFihAttr = XFih_VolatileAssignU32(Attr);

				/** Deny the request unless the region is active. */
				ASSIGN_VOLATILE(Status, XASUFW_MEM_REGN_REGION_INACTIVE);
				XFIH_IF_FAILOUT_WITH_MASK(XFihAttr,
					(XASUFW_MEM_REGN_STATE_MASK << XASUFW_MEM_REGN_STATE_SHIFT), ==,
					(XASUFW_MEM_REGN_STATE_ACTIVE << XASUFW_MEM_REGN_STATE_SHIFT)) {
					/** Grant access only when the subsystem's permission bit is set. */
					ASSIGN_VOLATILE(Status, XASUFW_MEM_REGN_ACCESS_DENIED);
					XFIH_IF_FAILOUT_WITH_MASK(XFihAttr, ((u32)1U << AccessBit), !=, 0U) {
						Status = XASUFW_SUCCESS;
					}
				}
			}
		}
	}

END:
	return Status;
}
/** @} */
