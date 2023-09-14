
/******************************************************************************
 * Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#include "xplmi_update.h"
#include "xpm_common.h"
#include "xpm_node.h"
#include "xpm_subsystem.h"
#include "xpm_pldevice.h"

int XPm_DsOps(u32 Op, u64 Addr, void *Data);
#define XPM_DYN_BYTEBUFFER_ID 0x1
#define XPM_DYN_BYTEBUFFER_VERSION   1U
#define XPM_DYN_BYTEBUFFER_LCVERSION 1U
#define XPM_DYN_BUFFER_END_ID 0x2
#define MAX_DYN_BYTEBUFFER_SIZE	(10U * 1024U)
static u8 Dyn_ByteBuffer[MAX_DYN_BYTEBUFFER_SIZE] = {0};

EXPORT_DS(Dyn_ByteBuffer, \
	XPLMI_MODULE_XILPM_ID, \
	XPM_DYN_BYTEBUFFER_ID,
	XPM_DYN_BYTEBUFFER_VERSION,\
	XPM_DYN_BYTEBUFFER_LCVERSION, \
	sizeof(Dyn_ByteBuffer), \
	(u32)(UINTPTR)Dyn_ByteBuffer \
	);

static u8 *Dyn_FreeBytes = Dyn_ByteBuffer;
static u32 Dyn_BufferEnd = (u32)Dyn_ByteBuffer;

EXPORT_DS(Dyn_BufferEnd, \
	XPLMI_MODULE_XILPM_ID, \
	XPM_DYN_BUFFER_END_ID, \
	XPM_DYN_BYTEBUFFER_VERSION,\
	XPM_DYN_BYTEBUFFER_LCVERSION, \
	sizeof(u32), \
	(u32)(UINTPTR)(&Dyn_BufferEnd) \
	);
inline DynByteBufferIter_t DynByteBuffer_Begin(void){
	return (u32*) Dyn_ByteBuffer;
}
inline DynByteBufferIter_t DynByteBuffer_End(void){
	return (u32*) Dyn_BufferEnd;
}
DynByteBufferIter_t DynByteBuffer_Next(DynByteBufferIter_t i){
	if (NULL == i) {
		return NULL;
	}
	u16 size = SAVED_DATA_GET_SIZE(i);
	return (i + size + 2);
}



static void* AllocBytes(u32 SizeInBytes, u8 Buffer[], u32 BufferSize, u32* BytesFree )
{
	void *Bytes = NULL;
	u32 BytesLeft = (u32)Buffer + BufferSize - *BytesFree;
	u32 Size = SizeInBytes;

	/* Round Size to the next multiple of 4 */
	Size += 3U;
	Size &= ~0x3U;

	if (Size > BytesLeft) {
		goto done;
	}

	Bytes = (void*)(*BytesFree);
	*BytesFree += Size;

	/* Zero the bytes */
	u32 NumWords = Size / 4U;
	u32* Words = (u32 *)Bytes;
	for (u32 i = 0; i < NumWords; i++) {
		Words[i] = 0U;
	}
done:
	Dyn_BufferEnd = (u32)(*BytesFree);
	return Bytes;
}

/**
 * @brief Print Usage of the Buffer
 */
void XPm_DumpDynamicMemUsage(void)
{
	xil_printf(" --- Total dynamic buffer Size = %u bytes\n\r", MAX_DYN_BYTEBUFFER_SIZE);
	xil_printf("Used = %u bytes\n\r", ((u32)Dyn_FreeBytes) - ((u32)Dyn_ByteBuffer));
	xil_printf("Free = %u bytes\n\r", MAX_DYN_BYTEBUFFER_SIZE - (((u32)Dyn_FreeBytes) - ((u32)Dyn_ByteBuffer)));
	xil_printf("\r\n");
}

/**
 * @brief Allocating memory within Dynamic ByteBuffer.
 * Return address always 4-Byte aligned
 */
void *XPmUpdate_DynAllocBytes(u32 SizeInBytes)
{

	return AllocBytes(SizeInBytes, Dyn_ByteBuffer, MAX_DYN_BYTEBUFFER_SIZE, (u32*)(&Dyn_FreeBytes));
}

/**
 * @brief Get the current unallocated memory address of Dynamic Byte Buffer
 * Note: This function simply return the address without checking any boundary.
 *
 */
inline void *XPmUpdate_GetDynFreeBytes(void)
{
	return (void*)Dyn_FreeBytes;
}

/**
 * @brief Searching through Dynamic Byte Buffer to look for saved node with
 * the same ID
 * Return the address of the saved node.
 */
void* XPmUpdate_GetSavedDataById(u32 NodeId)
{
	for (DynByteBufferIter_t i = DynByteBuffer_Begin(); i !=DynByteBuffer_End(); i = DynByteBuffer_Next(i)){
		if (SAVED_DATA_ID(i) == NodeId) return i;
	}
	return NULL;
}

/**
 * @brief Get address of DynByteBuffer
 */
void *XPmUpdate_GetDynBytesBuffer()
{
	return (void*)Dyn_ByteBuffer;
}
/**
 * @brief Search all Nodes type in Byte Buffer and save dynamic part of each node
 */
XStatus XPmUpdate_SaveAllNodes(void)
{
	XStatus Status = XST_FAILURE;
	XPm_Node* FailedNode = NULL;

	/* Iterating through all subsystems */
	for (u32 i = 0 ; i < XPM_NODEIDX_SUBSYS_MAX; i++){
		XPm_Subsystem *SubSystem = XPmSubsystem_GetByIndex(i);
		if (NULL == SubSystem) {
			/* Empty Subsystem */
			continue;
		}
		Status = SubSystem->SaveRestoreHandler(NULL, (u32*)SubSystem, XPLMI_STORE_DATABASE);
		if (XST_SUCCESS != Status){
			/* Note: This follow casting is OK; because XPm_Node and XPm_Subsystem have the same
			 * "header" words which is only required for logging error
			 */
			FailedNode = (XPm_Node*)SubSystem;
			goto done;
		}
	}

	/* Iterating through all Nodes */
	u32 NumNodes = XPmNode_GetNumNodes();
	for (u32 i =0; i < NumNodes; i++){
		XPm_Node* node = XPmNode_GetNodeAt(i);
		if (NULL == node){
			/* Empty Node */
			continue;
		}
		Status = node->SaveRestoreHandler(NULL, (u32*)node, XPLMI_STORE_DATABASE);
		if (XST_SUCCESS != Status)
		{
			FailedNode = node;
			goto done;
		}
	}
	Status = XST_SUCCESS;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, FailedNode);
	return Status;
}

static XStatus AddMissingPlDevices(void) {
	XStatus Status = XST_FAILURE;
	for (DynByteBufferIter_t i = DynByteBuffer_Begin(); i !=DynByteBuffer_End(); i = DynByteBuffer_Next(i))
	{
		u32 NodeId = SAVED_DATA_ID(i);
		if (NODECLASS(NodeId) == XPM_NODECLASS_DEVICE \
			&&  NODESUBCLASS(NodeId) == XPM_NODESUBCL_DEV_PL) {
			if (NULL == XPmDevice_GetById(NodeId)) {
				u32 Args[1] = {NodeId};
				Status = XPm_AddNode(Args, 1);
				if (XST_SUCCESS != Status) {
					goto done;
				}
			}
		}
	}
	Status = XST_SUCCESS;
done:
	return Status;
}

/**
 * @brief Search all Nodes type in Byte Buffer and restore dynamic part of each node
 */
XStatus XPmUpdate_RestoreAllNodes(void)
{
	XStatus Status = XST_FAILURE;
	XPm_Node* FailedNode = NULL;

	/* Iterating through all subsystems */
	for (u32 i = 0 ; i < XPM_NODEIDX_SUBSYS_MAX; i++){
		XPm_Subsystem *SubSystem = XPmSubsystem_GetByIndex(i);
		if (NULL == SubSystem) {
			/* Empty Subsystem */
			continue;
		}
		u32* SavedData = (u32*) XPmUpdate_GetSavedDataById(SubSystem->Id);
		if (NULL == SavedData){
			/* Not found in Saved Data */
			continue;
		}
		Status = SubSystem->SaveRestoreHandler(SavedData, (u32*)SubSystem, XPLMI_RESTORE_DATABASE );
		if (XST_SUCCESS != Status){
			/* Note: This follow casting is OK; because XPm_Node and Subsystem have same
			 * "header" words which is only required for logging error
			 */
			FailedNode = (XPm_Node*)SubSystem;
			goto done;
		}
	}

	Status = AddMissingPlDevices();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Iterating through all Nodes */
	u32 NumNodes = XPmNode_GetNumNodes();
	for (u32 i =0; i < NumNodes; i++){
		XPm_Node* node = XPmNode_GetNodeAt(i);

		if (NULL == node){
			/* Empty Node */
			continue;
		}

		u32* SavedData = (u32*) XPmUpdate_GetSavedDataById(node->Id);
		if (NULL == SavedData) {
			/* Not found in Saved Data */
			continue;
		}

		Status = node->SaveRestoreHandler(SavedData, (u32*)node, XPLMI_RESTORE_DATABASE);
		if (XST_SUCCESS != Status)
		{
			FailedNode = node;
			goto done;
		}
	}
	Status = XST_SUCCESS;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, FailedNode);
	return Status;
}