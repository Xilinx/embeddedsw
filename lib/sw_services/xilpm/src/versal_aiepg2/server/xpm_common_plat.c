/******************************************************************************
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_io.h"
#include "xpm_regs.h"
#include "xpm_common.h"
#include "xpm_debug.h"
#include "xplmi_update.h"
#ifdef CPPUTEST
#define MAX_BYTEBUFFER_SIZE	(67U * 1024U)
#else
#define MAX_BYTEBUFFER_SIZE	(67U * 1024U)
#endif

#ifdef CPPUTEST
u8 ByteBuffer[MAX_BYTEBUFFER_SIZE];
#else
static u8 ByteBuffer[MAX_BYTEBUFFER_SIZE];
#endif
static u8 *FreeBytes = ByteBuffer;

/* Saved ByteBuffer Address */
static u32 SavedBBAddr = 0;
/* Previous ByteBuffer Address */
static u32 PrevBBAddr = 0;

/* Handler for ByteBUffer during store and restore operation (PLM update) */
static int XPm_ByteBufferOps(u32 Op, u64 Addr, void *Data);
/* Handler for Address of ByteBUffer during store and restore operation (PLM update) */
static int XPm_PrevByteBufferOps(u32 Op, u64 Addr, void *Data);


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
 * @brief Get the address of ByteBuffer that is in DDR region
 *
 * @return u32 32-bit Address within DDR region
 */
inline u32 XPm_GetSavedByteBufferAddress(void) {
       return SavedBBAddr;
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
 * @return u32 32-bit address in the saved ByteBuffer.
 */
u32 XPm_ConvertToSavedAddress(u32 InputAddress){
	return InputAddress + XPm_GetByteBufferOffset();
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

/*****************************************************************************/
/**
 *  This function is used to set/clear bits in any NPI PCSR
 *
 *  @param BaseAddress  BaseAddress of device
 *  @param Mask                 Mask to be written into PCSR_MASK register
 *  @param Value                Value to be written into PCSR_CONTROL register
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *****************************************************************************/
XStatus XPm_PcsrWrite(u32 BaseAddress, u32 Mask, u32 Value)
{
        XStatus Status = XST_FAILURE;
        u16 DbgErr = XPM_INT_ERR_UNDEFINED;

        XPm_Out32((BaseAddress + NPI_PCSR_MASK_OFFSET), Mask);
        /* Blind write check */
        PmChkRegOut32((BaseAddress + NPI_PCSR_MASK_OFFSET), Mask, Status);
        if (XPM_REG_WRITE_FAILED == Status) {
                DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_MASK;
                goto done;
        }

        XPm_Out32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Value);
        /* Blind write check */
        PmChkRegMask32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Mask, Value, Status);
        if (XPM_REG_WRITE_FAILED == Status) {
                DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_CONTROL;
                goto done;
        }

        Status = XST_SUCCESS;

done:
        XPm_PrintDbgErr(Status, DbgErr);
        return Status;
}
