/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_runtime_alloc.h"
#include "xpm_common.h"
#include "xpm_memory_pools.h"

static u8 SubSystemPoolMemBuffer[MAX_SUBSYS_POOL_SIZE]; /** For Subsystem and requirement data */
static u8 ReqmPoolMemBuffer[MAX_REQM_POOL_SIZE]; /** For Subsystem and requirement data */
static u8 DevOpsPoolMemBuffer[MAX_DEVOPS_POOL_SIZE]; /** Other Pool Memory Buffer */
static u8 OtherPoolMemBuffer[MAX_OTHER_POOL_SIZE]; /** Other Pool Memory Buffer */

static XPm_AllocablePool_t SubSystemPoolMem = {
	.Size = MAX_SUBSYS_POOL_SIZE,
	.Pool = SubSystemPoolMemBuffer,
	.FreeMem = SubSystemPoolMemBuffer
};
static XPm_AllocablePool_t ReqmPoolMem = {
	.Size = MAX_REQM_POOL_SIZE,
	.Pool = ReqmPoolMemBuffer,
	.FreeMem = ReqmPoolMemBuffer
};
static XPm_AllocablePool_t DevOpsPoolMem = {
	.Size = MAX_DEVOPS_POOL_SIZE,
	.Pool = DevOpsPoolMemBuffer,
	.FreeMem = DevOpsPoolMemBuffer
};
static XPm_AllocablePool_t OtherPoolMem = {
	.Size = MAX_OTHER_POOL_SIZE,
	.Pool = OtherPoolMemBuffer,
	.FreeMem = OtherPoolMemBuffer
};

/**
 * @brief Allocates a block of memory from the SubSystemPoolMem.
 *
 * This function allocates a block of memory of the specified size from the SubSystemPoolMem.
 * If the allocation fails, an error message is printed and the memory usage is dumped.
 *
 * @param SizeInBytes The size of the memory block to allocate, in bytes.
 * @return A pointer to the allocated memory block, or NULL if the allocation fails.
 */
void *XPm_AllocBytesSubsys(u32 SizeInBytes)
{
	void* ret =  XPm_AllocPool(SizeInBytes, &SubSystemPoolMem);
	if (NULL == ret) {
		PmErr("Failed to allocate %u bytes from SubSystemPoolMem\n\r", SizeInBytes);
		XPm_DumpMemUsage();
	}
	return ret;
}

/**
 * @brief Allocates a block of memory from the ReqmPoolMem.
 *
 * This function allocates a block of memory of the specified size from the ReqmPoolMem.
 * If the allocation fails, an error message is printed and the memory usage is dumped.
 *
 * @param SizeInBytes The size of the memory block to allocate, in bytes.
 * @return A pointer to the allocated memory block, or NULL if the allocation fails.
 */
void *XPm_AllocBytesReqm(u32 SizeInBytes)
{
	void* ret =  XPm_AllocPool(SizeInBytes, &ReqmPoolMem);
	if (NULL == ret) {
		PmErr("Failed to allocate %u bytes from ReqmPoolMem\n\r", SizeInBytes);
		XPm_DumpMemUsage();
	}
	return ret;
}

/**
 * @brief Allocates a block of memory from the OtherPoolMem.
 *
 * This function allocates a block of memory of the specified size from the OtherPoolMem.
 * If the allocation fails, an error message is printed.
 *
 * @param SizeInBytes The size of the memory block to allocate, in bytes.
 * @return A pointer to the allocated memory block, or NULL if the allocation fails.
 */
void *XPm_AllocBytesOthers(u32 SizeInBytes)
{
	void* ret =  XPm_AllocPool(SizeInBytes, &OtherPoolMem);
	if (NULL == ret) {
		PmErr("Failed to allocate %u bytes from OtherPoolMem\n\r", SizeInBytes);
	}
	return ret;
}

void *XPm_AllocBytesDevOps(u32 SizeInBytes)
{
	void* ret =  XPm_AllocPool(SizeInBytes, &DevOpsPoolMem);
	if (NULL == ret) {
		PmErr("Failed to allocate %u bytes from DevOpsPoolMem\n\r", SizeInBytes);
	}
	return ret;
}

/**
 * @brief Dumps the memory usage of the SubSystemPoolMem, ReqmPoolMem, and OtherPoolMem.
 *
 * This function prints the memory usage of the SubSystemPoolMem, ReqmPoolMem, and OtherPoolMem
 * by calling the XPm_DumpMemUsage_Pool function for each pool.
 */
void  XPm_DumpMemUsage(void)
{
	(void)XPm_DumpTopologyMemUsage();
	PmInfo("SubSystem Pool:\n\r");
	(void) XPm_DumpMemUsage_Pool(SubSystemPoolMem);
	PmInfo("Requirement Pool:\n\r");
	(void) XPm_DumpMemUsage_Pool(ReqmPoolMem);
	PmInfo("DevOps Pool:\n\r");
	(void) XPm_DumpMemUsage_Pool(DevOpsPoolMem);
	PmInfo("Other Pool:\n\r");
	(void) XPm_DumpMemUsage_Pool(OtherPoolMem);
}
