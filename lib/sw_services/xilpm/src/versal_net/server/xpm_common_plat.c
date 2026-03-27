/******************************************************************************
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_io.h"
#include "xpm_regs.h"
#include "xpm_common.h"
#include "xpm_debug.h"
#include "xplmi_update.h"
#include "xpm_update_data.h"

#define STR2(x)	#x
#define STR(x)	STR2(x)

/* XILPM_BYTEBUFFER_SIZE_IN_KB is a configurable compiler option for XilPM */
#if defined(XILPM_BYTEBUFFER_SIZE_IN_KB) && XILPM_BYTEBUFFER_SIZE_IN_KB > 0
	#define MAX_BYTEBUFFER_SIZE	((XILPM_BYTEBUFFER_SIZE_IN_KB) * 1024U)
	#pragma message("XilPM ByteBuffer size is set to " STR(MAX_BYTEBUFFER_SIZE) " bytes")
#else
	#ifdef CPPUTEST
		#define MAX_BYTEBUFFER_SIZE	(52U * 1024U)
	#else
		#define MAX_BYTEBUFFER_SIZE	(79U * 1024U)
	#endif
	#pragma message("XILPM_BYTEBUFFER_SIZE_IN_KB is <= 0 or not defined, using default size " STR(MAX_BYTEBUFFER_SIZE) " bytes")
#endif

#ifdef CPPUTEST
	u8 ByteBuffer[MAX_BYTEBUFFER_SIZE];
#else
	static u8 ByteBuffer[MAX_BYTEBUFFER_SIZE];
#endif

/* Free Bytes Pointer */
static u8 *FreeBytes = ByteBuffer;

/* Saved ByteBuffer Address */
static u32 SavedBBAddr = 0;
/* Previous ByteBuffer Address */
static u32 PrevBBAddr = 0;

/* Handler for ByteBUffer during store and restore operation (PLM update) */
static int XPm_ByteBufferOps(u32 Op, u64 Addr, void *Data);
/* Handler for Address of ByteBUffer during store and restore operation (PLM update) */
static int XPm_PrevByteBufferOps(u32 Op, u64 Addr, void *Data);

/* Save but not restore whole ByteBuffer through special XPm_ByteBufferOps */
EXPORT_DS_W_HANDLER(ByteBuffer, \
	XPLMI_MODULE_XILPM_ID, XPM_BYTEBUFFER_DS_ID, \
	XPM_DATA_STRUCT_VERSION, XPM_DATA_STRUCT_LCVERSION, \
	sizeof(ByteBuffer), (u32)(UINTPTR)ByteBuffer , XPm_ByteBufferOps);

/* Save and restore whole PrevBBAddr */
EXPORT_DS_W_HANDLER(PrevBBAddr, \
	XPLMI_MODULE_XILPM_ID, XPM_BYTEBUFFER_ADDR_DS_ID, \
	XPM_DATA_STRUCT_VERSION, XPM_DATA_STRUCT_LCVERSION, \
	sizeof(PrevBBAddr), (u32)(UINTPTR)(&PrevBBAddr), XPm_PrevByteBufferOps);

void *XPm_AllocBytes(u32 SizeInBytes)
{
	void *Bytes = NULL;
	u32 BytesLeft = (u32)ByteBuffer + MAX_BYTEBUFFER_SIZE - (u32)FreeBytes;
	u32 i;
	u32 NumWords;
	u32 *Words;
	u32 Size = SizeInBytes;

	/* Round size to the next multiple of 4 */
	Size += 3U;
	Size &= ~0x3U;

	if (Size > BytesLeft) {
		goto done;
	}

	Bytes = FreeBytes;
	FreeBytes += Size;

	/* Zero the bytes */
	NumWords = Size / 4U;
	Words = (u32 *)Bytes;
	for (i = 0; i < NumWords; i++) {
		Words[i] = 0U;
	}

done:
	return Bytes;
}

void XPm_DumpMemUsage(void)
{
	xil_printf("Total buffer size = %u bytes\n\r", MAX_BYTEBUFFER_SIZE);
	xil_printf("Used = %u bytes\n\r", FreeBytes - ByteBuffer);
	xil_printf("Free = %u bytes\n\r", MAX_BYTEBUFFER_SIZE - (u32)(FreeBytes - ByteBuffer));
	xil_printf("\r\n");
}

/**
 * @brief Get the size of the ByteBuffer
 *
 * @return u32 Size of ByteBuffer in bytes
 */
inline u32 XPm_GetByteBufferSize(void) {
	return MAX_BYTEBUFFER_SIZE;
}

/**
 * @brief Get the address of ByteBuffer that is in DDR region
 *
 * @return u32 32-bit Address within DDR region
 */
inline u32 XPm_GetSavedByteBufferAddress(void) {
       return SavedBBAddr;
}

/**
 * @brief Validate that an address is within the saved ByteBuffer region
 *
 * Checks that the address is non-zero, word-aligned, and falls within
 * the valid DDR save region [SavedBBAddr, SavedBBAddr + MAX_BYTEBUFFER_SIZE).
 *
 * @param Addr Address to validate
 * @return 1U if valid, 0U if invalid
 */
u32 XPm_IsValidSavedAddress(u32 Addr)
{
	/* Check non-zero and word-aligned */
	if ((0U == Addr) || ((Addr & 0x3U) != 0U)) {
		return 0U;
	}

	/* Check within saved ByteBuffer bounds */
	if ((Addr < SavedBBAddr) || (Addr >= (SavedBBAddr + MAX_BYTEBUFFER_SIZE))) {
		return 0U;
	}

	return 1U;
}

/**
 * @brief Get the offset from the ByteBuffer that is in DDR region to
 *  the address of ByteBuffer that was in the runtime before PLM update occur
 *
 * @return u32 32-bit offset value.
 */
inline u32 XPm_GetByteBufferOffset(void) {
	return SavedBBAddr - PrevBBAddr;
}

/**
 * @brief Get the address of ByteBuffer that was previously in
 * the runtime region before PLM update occur
 *
 * @return u32 32-bit address of previous ByteBuffer.
 */
inline u32 XPm_GetPrevByteBufferAddress(void) {
       return PrevBBAddr;
}

/**
 * @brief Convert a given address in in previous ByteBuffer
 * to the address within the ByteBuffer that saved in DDR
 *
 * @param InputAddress address in the previous ByteBuffer
 * @return u32 32-bit address in the saved ByteBuffer. This address is in PLM storage area
 */

u32 XPm_ConvertToSavedAddress(u32 InputAddress){
	u32 OutputAddress = 0U;
	u32 Offset;

	if (0U == InputAddress) {
		goto done;
	}

	Offset = XPm_GetByteBufferOffset();

	/*
	 * Note: Offset calculation (SavedBBAddr - PrevBBAddr) wraps when
	 * SavedBBAddr (DDR, ~0x10000000) < PrevBBAddr (TCM, ~0xF0000000).
	 * The addition InputAddress + Offset is expected to wrap back to
	 * the DDR range. This is correct modular arithmetic - do NOT add
	 * an overflow check here. The bounds validation below ensures the
	 * result is within the valid saved ByteBuffer region.
	 */
	OutputAddress = InputAddress + Offset;

	/* Validate output is within saved ByteBuffer bounds */
	if (XPm_IsValidSavedAddress(OutputAddress) == 0U) {
		OutputAddress = 0U;
	}

done:
	return OutputAddress;
}

static int XPm_ByteBufferOps(u32 Op, u64 Addr, void *Data)
{
	XStatus Status = XST_FAILURE;
	if (Op == XPLMI_STORE_DATABASE) {
		/* Copy Data Structure to given address */
		Status = XPlmi_DsOps(Op, Addr, Data);
		if (XST_SUCCESS != Status) {
			goto END;
		}
	} else if (Op == XPLMI_RESTORE_DATABASE) {
		/** Only need to export the Saved Address of the ByteBuffer.
		 * Restoring ByteBuffer will be handle later post PMC cdo loaded.
		*/
		SavedBBAddr = (u32)(Addr + sizeof(XPlmi_DsHdr));
	}
	else {
		Status = XPLMI_ERR_PLM_UPDATE_INVALID_OP;
		goto END;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

static int XPm_PrevByteBufferOps(u32 Op, u64 Addr, void *Data)
{
	XStatus Status = XST_FAILURE;
	if (Op == XPLMI_STORE_DATABASE) {
		/* Assigned current ByteBuffer address*/
		PrevBBAddr = (u32)ByteBuffer;
		/* Then store it. */
		Status = XPlmi_DsOps(Op, Addr, Data);
		if (XST_SUCCESS != Status) {
			goto END;
		}
	} else if (Op == XPLMI_RESTORE_DATABASE) {
		/* Simply retstore it */
		Status = XPlmi_DsOps(Op, Addr, Data);
		if (XST_SUCCESS != Status) {
			goto END;
		}
	}
	else {
		Status = XPLMI_ERR_PLM_UPDATE_INVALID_OP;
		goto END;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}
