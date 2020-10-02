/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaiegbl.c
* @{
*
* This file contains the global initialization functions for the Tile.
* This is applicable for both the AIE tiles and Shim tiles.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   10/22/2019  Enable AIE initilization
* 1.2   Tejus   06/09/2020  Call IO init api from XAie_CfgInitialize
* 1.3   Tejus   06/10/2020  Add api to change backend at runtime.
* 1.4   Dishita 07/28/2020  Add api to turn ECC On and Off.
* 1.5   Nishad  09/15/2020  Add check to validate XAie_MemCacheProp value in
*			    XAie_MemAllocate().
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>

#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_regdef.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/

/************************** Variable Definitions *****************************/
extern XAie_TileMod AieMod[XAIEGBL_TILE_TYPE_MAX];

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the global initialization function for all the tiles of the AIE array
* The function sets up the Device Instance pointer with the appropriate values
* from the ConfigPtr.
*
* @param	InstPtr - Global AIE instance structure.
* @param	ConfigPtr - Global AIE configuration pointer.
*
* @return	XAIE_OK on success and error code on failure
*
* @note		This function needs to be called before calling any other AI
*		engine functions. After this function, as all tiles are gated
*		after system boots, XAie_PmRequestTiles() needs to be called
*		before calling other functions, otherwise, other functions
*		may access gated tiles.
*
******************************************************************************/
AieRC XAie_CfgInitialize(XAie_DevInst *InstPtr, XAie_Config *ConfigPtr)
{
	AieRC RC;

	if((InstPtr == XAIE_NULL) || (ConfigPtr == XAIE_NULL)) {
		XAIE_ERROR("Invalid input arguments\n",
				XAIE_INVALID_ARGS);
		return XAIE_INVALID_ARGS;
	}

	if(InstPtr->IsReady)
		return XAIE_OK;

	/* Initialize device property according to Device Type */
	if(ConfigPtr->AieGen == XAIE_DEV_GEN_AIE) {
		InstPtr->DevProp.DevMod = AieMod;
		InstPtr->DevProp.DevGen = XAIE_DEV_GEN_AIE;
	} else {
		XAIE_ERROR("Invalid device\n",
				XAIE_INVALID_DEVICE);
		return XAIE_INVALID_DEVICE;
	}

	InstPtr->IsReady = XAIE_COMPONENT_IS_READY;
	InstPtr->DevProp.RowShift = ConfigPtr->RowShift;
	InstPtr->DevProp.ColShift = ConfigPtr->ColShift;
	InstPtr->BaseAddr = ConfigPtr->BaseAddr;
	InstPtr->NumRows = ConfigPtr->NumRows;
	InstPtr->NumCols = ConfigPtr->NumCols;
	InstPtr->ShimRow = ConfigPtr->ShimRowNum;
	InstPtr->ReservedRowStart = ConfigPtr->ReservedRowStart;
	InstPtr->ReservedNumRows = ConfigPtr->ReservedNumRows;
	InstPtr->AieTileRowStart = ConfigPtr->AieTileRowStart;
	InstPtr->AieTileNumRows = ConfigPtr->AieTileNumRows;
	InstPtr->EccStatus = XAIE_ENABLE;

	memcpy(&InstPtr->PartProp, &ConfigPtr->PartProp,
		sizeof(ConfigPtr->PartProp));

	RC = XAie_IOInit(InstPtr);
	if(RC != XAIE_OK) {
		return RC;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the api to finish the AI enigne partition. It will release
* the occupied AI engine resource
*
* @param	DevInst - Global AIE device instance pointer.
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_Finish(XAie_DevInst *DevInst)
{
	const XAie_Backend *CurrBackend;
	AieRC RC;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	CurrBackend = DevInst->Backend;
	RC = CurrBackend->Ops.Finish(DevInst->IOInst);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to close backend instance.\n");
		return RC;
	}

	DevInst->IsReady = 0;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the api to set the IO backend of the driver at runtime.
*
* @param	DevInst - Global AIE device instance pointer.
* @param	Backend - Backend IO type to switch to.
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_SetIOBackend(XAie_DevInst *DevInst, XAie_BackendType Backend)
{
	AieRC RC;
	const XAie_Backend *CurrBackend, *NewBackend;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Backend >= XAIE_IO_BACKEND_MAX) {
		XAIE_ERROR("Invalid backend request \n");
		return XAIE_INVALID_ARGS;
	}

	/* Release resources for current backend */
	CurrBackend = DevInst->Backend;
	RC = CurrBackend->Ops.Finish((void *)(DevInst->IOInst));
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to close backend instance."
				"Falling back to backend %d\n",
				CurrBackend->Type);
		return RC;
	}

	/* Get new backend and initialize the backend */
	NewBackend = _XAie_GetBackendPtr(Backend);
	RC = NewBackend->Ops.Init(DevInst);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to initialize backend %d\n",
				Backend);
		return RC;
	}

	XAIE_DBG("Switching backend to %d\n", Backend);
	DevInst->Backend = NewBackend;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to allocate a memory
*
* @param	DevInst: Device Instance
* @param	Size: Size of the memory
* @param	Cache: Buffer to be cacheable or not
*
* @return	Pointer to the allocated memory instance.
*
* @note		None.
*
*******************************************************************************/
XAie_MemInst* XAie_MemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	const XAie_Backend *Backend;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return NULL;
	}

	if(Cache > XAIE_MEM_NONCACHEABLE) {
		XAIE_ERROR("Invalid cache property\n");
		return NULL;
	}

	Backend = DevInst->Backend;

	return Backend->Ops.MemAllocate(DevInst, Size, Cache);
}

/*****************************************************************************/
/**
*
* This is the memory function to free the memory
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_MemFree(XAie_MemInst *MemInst)
{
	const XAie_Backend *Backend;

	if(MemInst == XAIE_NULL) {
		XAIE_ERROR("Invalid memory instance\n");
		return XAIE_ERR;
	}

	Backend = MemInst->DevInst->Backend;

	return Backend->Ops.MemFree(MemInst);
}

/*****************************************************************************/
/**
*
* This is the memory function to sync the memory for CPU
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_MemSyncForCPU(XAie_MemInst *MemInst)
{
	const XAie_Backend *Backend;

	if(MemInst == XAIE_NULL) {
		XAIE_ERROR("Invalid memory instance\n");
		return XAIE_ERR;
	}

	Backend = MemInst->DevInst->Backend;

	return Backend->Ops.MemSyncForCPU(MemInst);
}

/*****************************************************************************/
/**
*
* This is the memory function to sync the memory for device
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_MemSyncForDev(XAie_MemInst *MemInst)
{
	const XAie_Backend *Backend;

	if(MemInst == XAIE_NULL) {
		XAIE_ERROR("Invalid memory instance\n");
		return XAIE_ERR;
	}

	Backend = MemInst->DevInst->Backend;

	return Backend->Ops.MemSyncForDev(MemInst);
}

/*****************************************************************************/
/**
*
* This is the memory function to return the virtual address of the memory
* instance
*
* @param	MemInst: Memory instance pointer.
*
* @return	Mapped virtual address of the memory instance.
*
* @note		None.
*
*******************************************************************************/
void* XAie_MemGetVAddr(XAie_MemInst *MemInst)
{
	if(MemInst == XAIE_NULL) {
		XAIE_ERROR("Invalid memory instance\n");
		return NULL;
	}

	return MemInst->VAddr;
}

/*****************************************************************************/
/**
*
* This is the memory function to return the physical address of the memory
* instance
*
* @param	MemInst: Memory instance pointer.
*
* @return	Physical address of the memory instance.
*
* @note		None.
*
*******************************************************************************/
u64 XAie_MemGetDevAddr(XAie_MemInst *MemInst)
{
	if(MemInst == XAIE_NULL) {
		XAIE_ERROR("Invalid memory instance\n");
		return XAIE_ERR;
	}

	return MemInst->DevAddr;
}

/*****************************************************************************/
/**
*
* This is the memory function to attach user allocated memory to the AI engine
* partition device instance.
*
* @param	DevInst: Device Instance
* @param	MemInst: pointer to memory instance which will be filled with
*			 attached AI engine memory instance information by
*			 this function.
* @param	DevAddr: Device address of the allocated memory. It is usually
*			 the physical address of the memory. For Linux dmabuf
*			 memory, it is the offset to the start of the dmabuf.
* @param	VAddr: Virtual address of the allocated memory. For Linux
*		       dmabuf memory, it is not required, it can be NULL.
*		Cache: Buffer is cacheable or not.
*		MemHandle: Handle of the allocated memory. It is ignored for
*			   other backends except Linux backend. For Linux
*			   backend, it is the file descriptor of a dmabuf.
*
* @return	XAIE_OK for success, or error code for failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_MemAttach(XAie_DevInst *DevInst, XAie_MemInst *MemInst, u64 DevAddr,
		u64 VAddr, u64 Size, XAie_MemCacheProp Cache, u64 MemHandle)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY) ||
		(DevInst->Backend == XAIE_NULL)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(MemInst == XAIE_NULL) {
		XAIE_ERROR("Invalid memory instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(Cache > XAIE_MEM_NONCACHEABLE) {
		XAIE_ERROR("Invalid cache property\n");
		return XAIE_INVALID_ARGS;
	}

	MemInst->DevInst = DevInst;
	MemInst->VAddr = (void *)(uintptr_t)VAddr;
	MemInst->DevAddr = DevAddr;
	MemInst->Size = Size;
	MemInst->Cache = Cache;

	return DevInst->Backend->Ops.MemAttach(MemInst, MemHandle);
}

/*****************************************************************************/
/**
*
* This is the memory function to dettach user allocated memory from the AI engine
* partition device instance.
*
* @param	MemInst: Memory Instance
*
* @return	XAIE_OK for success, and error code for failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_MemDetach(XAie_MemInst *MemInst)
{
	XAie_DevInst *DevInst;

	if(MemInst == XAIE_NULL) {
		XAIE_ERROR("Invalid memory instance\n");
		return XAIE_INVALID_ARGS;
	}

	DevInst = MemInst->DevInst;
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY) ||
		(DevInst->Backend == XAIE_NULL)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	return DevInst->Backend->Ops.MemDetach(MemInst);
}

/*****************************************************************************/
/*
* This API disables the ECC flag in the Device Instance of the partition. It
* should be called before calling elf loader to disable ECC. ECC configuration
* is done from elf loader.
*
* @param        DevInst: Device Instance
*
* @return       XAIE_OK on success
* @note         None
*
*******************************************************************************/
AieRC XAie_TurnEccOff(XAie_DevInst *DevInst)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	DevInst->EccStatus = XAIE_DISABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/*
* This API enables the ECC flag in the Device Instance of the partition. ECC
* configuration is done from elf loader.
*
* @param        DevInst: Device Instance
*
* @return       XAIE_OK on success
* @note         None
*
*******************************************************************************/
AieRC XAie_TurnEccOn(XAie_DevInst *DevInst)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	DevInst->EccStatus = XAIE_ENABLE;

	return XAIE_OK;
}
/** @} */
