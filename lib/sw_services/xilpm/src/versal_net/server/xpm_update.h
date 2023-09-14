/******************************************************************************
 * Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#ifndef XPM_UPDATE_H_
#define XPM_UPDATE_H_
#include "xpm_debug.h"
#include "xpm_common.h"
#ifdef __cplusplus
extern "C" {
#endif
#define XPM_SAVED_DATA_ID_OFFSET	(0U)	/* Stored Node ID*/
#define XPM_SAVED_DATA_INFO_OFFSET	(1U)	/* Stored Version and Size*/
#define XPM_SAVED_DATA_RETORE_OFFSET	(2U)	/* Stored other saved data to be restored*/
#define XPM_SAVED_DATA_SIZE_MASK (0xFFFFU)
#define XPM_SAVED_DATA_VERSION_MASK (0xFFFF0000U)
#define XPM_SAVED_DATA_VERSION_OFFSET (16U)
#define SAVED_DATA_ID(SavedData) (SavedData[XPM_SAVED_DATA_ID_OFFSET])
#define SAVED_DATA_INFO(SavedData) (SavedData[XPM_SAVED_DATA_INFO_OFFSET])
#define SAVED_DATA_GET_SIZE(SavedData) ((SAVED_DATA_INFO(SavedData)) & (XPM_SAVED_DATA_SIZE_MASK))
#define SAVED_DATA_GET_VERSION(SavedData) ((SAVED_DATA_INFO(SavedData)  >> XPM_SAVED_DATA_VERSION_OFFSET) & (0xFFFFU))

#define SAVED_DATA_SET_VERSION(SavedData, Version) \
	SAVED_DATA_INFO(SavedData) = (Version << (XPM_SAVED_DATA_VERSION_OFFSET)) | (SAVED_DATA_GET_SIZE(SavedData))

#define SAVED_DATA_SET_ID(SavedData, ID) SAVED_DATA_ID(SavedData) = ID
#define SAVED_DATA_SET_SIZE(SavedData, Size) \
	SAVED_DATA_INFO(SavedData) = (SAVED_DATA_GET_VERSION(SavedData)  << (XPM_SAVED_DATA_VERSION_OFFSET)) | (Size & XPM_SAVED_DATA_SIZE_MASK)


typedef u32* DynByteBufferIter_t;
DynByteBufferIter_t DynByteBuffer_Begin(void);
DynByteBufferIter_t DynByteBuffer_End(void);
DynByteBufferIter_t DynByteBuffer_Next(DynByteBufferIter_t i);
void* XPmUpdate_GetSavedDataById(u32 NodeId);

#define RestoreStruct(addr, m) { \
	m = *((typeof(m)*)(addr)); \
	addr = (u32*)((u32)(addr)+ (sizeof(m) + 3U)); \
	addr = (u32*)((u32)(addr) & (~(0x3U))); \
}

#define SaveStruct(STATUS, ERROR_LABEL, m) \
({ \
	typeof(m)* tmp = (typeof(m)*) XPmUpdate_DynAllocBytes(sizeof(m)); \
	if (NULL == tmp){ \
		STATUS = XPM_UPDATE_BUFFER_TOO_SMALL; \
		goto ERROR_LABEL; \
	} \
	*(tmp) = m; \
})

typedef XStatus (*SaveRestoreHandler_t)(u32* SavedData, u32*ThisData, u32 Op);

void *XPmUpdate_DynAllocBytes(u32 SizeInBytes);
void *XPmUpdate_GetDynFreeBytes(void);
void *XPmUpdate_GetDynBytesBuffer(void);
void XPm_DumpDynamicMemUsage(void);
XStatus XPmUpdate_SaveAllNodes(void);
XStatus XPmUpdate_RestoreAllNodes(void);

#define BEGIN_SAVE_STRUCT(SavedData, SaveNodeFunc, NodePtr) { \
	Status = SaveNodeFunc(NodePtr, &SavedData);\
	if (XST_SUCCESS != Status)\
	{\
		goto done; \
	}\
	u32* StartDataAddr = (u32*)XPmUpdate_GetDynFreeBytes();


#define END_SAVE_STRUCT(SavedData) \
	u32 size = SAVED_DATA_GET_SIZE(SavedData) + ((((u32)XPmUpdate_GetDynFreeBytes()) - (u32)StartDataAddr)>>2);\
	SAVED_DATA_SET_SIZE(SavedData, size);\
}

#define XPM_UPDATE_THROW_IF_ERROR(Status, NodeData) { \
	if (XST_SUCCESS != Status) { \
		PmErr("Error:[%s %d] NodeId=0x%x Status = 0x%x\n\r", __func__, __LINE__,NodeData?((XPm_Node*)NodeData)->Id:0, Status); \
	} \
}
#ifdef __cplusplus
}
#endif

#endif /* XPM_UPDATE_H_ */