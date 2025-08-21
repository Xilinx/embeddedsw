/******************************************************************************
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_alloc.h"
#include "xpm_common.h"
#include "xpm_memory_pools.h"

/** List of memory pool list:*/
static u8 TopoPoolMemBuffer[MAX_TOPO_POOL_SIZE]; /** For Topology data  */
static XPm_AllocablePool_t TopoPoolMem = {
	.Size = MAX_TOPO_POOL_SIZE,
	.Pool = TopoPoolMemBuffer,
	.FreeMem = TopoPoolMemBuffer
};
/**
 * @brief Allocates a pool of memory of the specified size.
 *
 * This function allocates a pool of memory of the specified size in bytes.
 *
 * @param SizeInBytes The size of the memory pool to be allocated in bytes.
 * @param PoolMem Pointer to the structure representing the allocable memory pool.
 *
 * @return Pointer to the allocated memory pool, or NULL if allocation fails.
 */
void *XPm_AllocPool(u32 SizeInBytes, XPm_AllocablePool_t* PoolMem)
{
	void *Bytes = NULL;
	int BytesLeft = (u8*)PoolMem->Pool + PoolMem->Size - (u8*)PoolMem->FreeMem;
	if (BytesLeft <= 0) {
		goto done;
	}
	u32 i;
	u32 NumWords;
	u32 *Words;
	u32 Size = SizeInBytes;

	/* Round size to the next multiple of 4 */
	Size += 3U;
	Size &= ~0x3U;

	if (Size >(u32)BytesLeft) {
		goto done;
	}

	Bytes = PoolMem->FreeMem;
	PoolMem->FreeMem = (u8*)PoolMem->FreeMem + Size;

	/* Zero the bytes */
	NumWords = Size / 4U;
	Words = (u32 *)Bytes;
	for (i = 0; i < NumWords; i++) {
		Words[i] = 0U;
	}

done:
	return Bytes;
}

/**
 * @brief Allocates a block of memory of the specified size in bytes.
 *
 * This function is used to allocate a block of memory of the specified size
 * in bytes. The allocated memory block can be used to store any type of data.
 *
 * @param SizeInBytes The size of the memory block to be allocated, in bytes.
 *
 * @return A pointer to the allocated memory block, or NULL if the allocation
 *         failed.
 */
void *XPm_AllocBytes(u32 SizeInBytes)
{
	void* ret =  XPm_AllocPool(SizeInBytes, &TopoPoolMem);

	if (NULL == ret) {
		PmErr("Failed to allocate %u bytes from TopoPoolMem\n\r", SizeInBytes);
		XPm_DumpMemUsage();
	}
	return ret;
}

/**
 * @brief Dumps the memory usage of a specific pool.
 *
 * This function is used to dump the memory usage of a specific pool.
 *
 * @param PoolMem The pool for which the memory usage needs to be dumped.
 */
void  XPm_DumpMemUsage_Pool(XPm_AllocablePool_t PoolMem)
{
	PmInfo("Total buffer size = %u bytes\n\r", PoolMem.Size);
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
	(void)XPm_DumpTopologyMemUsage();
}

static u8 BoardPoolMemBuffer[MAX_BOARD_POOL_SIZE];	/** Board Pool Memory Buffer */

static XPm_AllocablePool_t BoardPoolMem = {
	.Size = MAX_BOARD_POOL_SIZE,
	.Pool = BoardPoolMemBuffer,
	.FreeMem = BoardPoolMemBuffer
};

/**
 * @brief Allocates a block of memory from the BoardPoolMem.
 *
 * This function allocates a block of memory of the specified size from the BoardPoolMem.
 * If the allocation fails, an error message is printed and the memory usage is dumped.
 *
 * @param SizeInBytes The size of the memory block to allocate, in bytes.
 * @return A pointer to the allocated memory block, or NULL if the allocation fails.
 */
void *XPm_AllocBytesBoard(u32 SizeInBytes)
{
	void* ret =  XPm_AllocPool(SizeInBytes, &BoardPoolMem);
	if (NULL == ret) {
		PmErr("Failed to allocate %u bytes from BoardPoolMem\n\r", SizeInBytes);
		XPm_DumpMemUsage();
	}
	return ret;
}
