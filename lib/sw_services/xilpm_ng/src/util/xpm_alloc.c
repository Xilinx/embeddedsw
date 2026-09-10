/******************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_alloc.h"
#include "xpm_common.h"
#include "xpm_memory_pools.h"
#include "xpm_update.h"

static u8 TopoPoolMemBuffer[MAX_TOPO_POOL_SIZE] XPM_PLACE_IN(".TopoPoolMemBuffer") ; /** For Topology data  */
static XPm_AllocablePool_t TopoPoolMem XPM_INIT_DATA(TopoPoolMem) =  {
	.Size = MAX_TOPO_POOL_SIZE,
	.Pool = TopoPoolMemBuffer,
	.FreeMem = TopoPoolMemBuffer
};

/* Local helpers for temporal checks (no include changes) */
/** Check that an unsigned value is nonzero. */
static inline XStatus XPm_CheckPositiveU32(u32 v)
{
	return (v > 0U) ? XST_SUCCESS : XST_FAILURE;
}

/** Check that one unsigned value does not exceed another. */
static inline XStatus XPm_CheckSizeLE(u32 a, u32 b)
{
	return (a <= b) ? XST_SUCCESS : XST_FAILURE;
}

/****************************************************************************/
/**
 * @brief Allocate zero-initialized storage from a monotonic memory pool.
 *
 * Rounds @p SizeInBytes up to a four-byte boundary, advances the pool's free
 * pointer by the rounded size, and securely zeroes the allocated region. The
 * pool has no corresponding free operation, and a zeroization failure does not
 * roll back the free pointer.
 *
 * @param SizeInBytes Number of bytes requested.
 * @param PoolMem Pool descriptor whose free pointer is advanced on success.
 *
 * @return Pointer to the allocated region if allocation reaches the zeroization
 *         step, including when secure zeroization fails (in which case the
 *         returned memory may not be zeroed); NULL if the pool has no bytes
 *         left or the rounded request exceeds the available space.
 *
 ****************************************************************************/
void *XPm_AllocPool(u32 SizeInBytes, XPm_AllocablePool_t* PoolMem)
{
	void *Bytes = NULL;
	s32 BytesLeft = (u8*)PoolMem->Pool + PoolMem->Size - (u8*)PoolMem->FreeMem;
	u32 Size = SizeInBytes;
	volatile XStatus Status = XST_FAILURE;

	/* Reject an exhausted pool before rounding the request */
	XSECURE_TEMPORAL_CHECK(done, Status, XPm_CheckPositiveU32, (u32)BytesLeft);

	/* Round size up to a 4-byte boundary (ceil) */
	Size += 3U;
	Size &= ~0x3U;

	/* Ensure the aligned allocation fits in the remaining pool */
	XSECURE_TEMPORAL_CHECK(done, Status, XPm_CheckSizeLE, Size, (u32)BytesLeft);

	/* Carve the allocation from the current monotonic free pointer */
	Bytes = PoolMem->FreeMem;
	PoolMem->FreeMem = (u8*)PoolMem->FreeMem + Size;

	/* Zeroize after advancing the pool; failure does not roll back the allocation */
	XSECURE_TEMPORAL_CHECK(done, Status, Xil_SecureZeroize, (u8*)Bytes, Size);

done:
	return Bytes;
}

/****************************************************************************/
/**
 * @brief Allocate zero-initialized storage from the topology pool.
 *
 * On allocation failure, logs the requested size and dumps all available pool
 * usage information.
 *
 * @param SizeInBytes Number of bytes requested.
 *
 * @return Pointer to the allocated region on success; NULL on failure.
 *
 ****************************************************************************/
void *XPm_AllocBytes(u32 SizeInBytes)
{
	void* ret =  XPm_AllocPool(SizeInBytes, &TopoPoolMem);

	if (NULL == ret) {
		PmErr("Failed to allocate %u bytes from TopoPoolMem\n\r", SizeInBytes);
		XPm_DumpMemUsage();
	}
	return ret;
}

/****************************************************************************/
/**
 * @brief Print the total, used, and free byte counts for a memory pool.
 *
 * Derives the used and free counts from the pool base, size, and current free
 * pointer without modifying the pool.
 *
 * @param PoolMem Pool descriptor to report, passed by value.
 *
 * @return None.
 *
 ****************************************************************************/
void  XPm_DumpMemUsage_Pool(XPm_AllocablePool_t PoolMem)
{
	PmInfo("Total buffer size = %u bytes\n\r", PoolMem.Size);
	/* FreeMem marks the first unallocated byte, so its offset is the used size. */
	PmInfo("Used = %d bytes\n\r", (u8*)PoolMem.FreeMem - (u8*)PoolMem.Pool);
	PmInfo("Free = %d bytes\n\r", PoolMem.Size + (u8*)PoolMem.Pool -  (u8*)PoolMem.FreeMem);
}

/**
 * @brief Dumps the memory usage of the topology pool.
 *
 * This function prints the memory usage of the topology pool.
 * It calls the XPm_DumpMemUsage_Pool() function to perform the actual dumping.
 *
 * @note This function is specific to the XPm_DumpTopologyMemUsage() function.
 */
void XPm_DumpTopologyMemUsage(void){
	PmInfo("Topology Pool:\n\r");
	(void) XPm_DumpMemUsage_Pool(TopoPoolMem);
}

/**
 * @brief Weak implementation of XPm_DumpMemUsage().
 *
 * This function is a weak implementation of XPm_DumpMemUsage().
 * It calls the XPm_DumpTopologyMemUsage() function to perform the actual dumping.
 *
 * @note This function can be overridden by a stronger implementation.
 */
void __attribute__((weak, noinline)) XPm_DumpMemUsage(void)
{
	/* xpm_runtime_alloc.c replaces this weak boot default in runtime builds. */
	(void)XPm_DumpTopologyMemUsage();
}

static u8 BoardPoolMemBuffer[MAX_BOARD_POOL_SIZE] XPM_PLACE_IN(".BoardPoolMemBuffer");	/** Board Pool Memory Buffer */
/* coverity[misra_c_2025_rule_8_9_violation] IPU save/restore persistent state; must retain file scope for the persistence/update mechanism. Excluded from static analysis per design. */
static XPm_AllocablePool_t BoardPoolMem XPM_INIT_DATA(BoardPoolMem)=  {
	.Size = MAX_BOARD_POOL_SIZE,
	.Pool = BoardPoolMemBuffer,
	.FreeMem = BoardPoolMemBuffer
};

/****************************************************************************/
/**
 * @brief Allocate zero-initialized storage from the board-data pool.
 *
 * On allocation failure, logs the requested size and dumps all available pool
 * usage information.
 *
 * @param SizeInBytes Number of bytes requested.
 *
 * @return Pointer to the allocated region on success; NULL on failure.
 *
 ****************************************************************************/
void *XPm_AllocBytesBoard(u32 SizeInBytes)
{
	void* ret =  XPm_AllocPool(SizeInBytes, &BoardPoolMem);
	if (NULL == ret) {
		PmErr("Failed to allocate %u bytes from BoardPoolMem\n\r", SizeInBytes);
		/* Use the overridable hook so runtime builds report every configured pool. */
		XPm_DumpMemUsage();
	}
	return ret;
}
