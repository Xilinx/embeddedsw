/******************************************************************************
 * Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#ifndef XPM_UPDATE_H_
#define XPM_UPDATE_H_
#include "xpm_debug.h"
#include "xpm_common.h"
#include "xpm_node.h"
#define member_size(type, member) sizeof(((type *)0)->member)

#ifdef __cplusplus
extern "C" {
#endif
/** PSM update related definition*/
#define PSM_UPDATE_REG_STATE (0xF20142C0U) /** Register to store state of PSM update*/
/** States of PSM update */
#define PSM_UPDATE_STATE_INIT		0x0
#define PSM_UPDATE_STATE_SHUTDOWN_START	0x1U
#define PSM_UPDATE_STATE_SHUTDOWN_DONE	0x2U
#define PSM_UPDATE_STATE_LOAD_ELF_DONE	0x3U
#define PSM_UPDATE_STATE_FINISHED	0x4U
/*******************************************/

/* This is the List of	of type that we use to save and restore*/
#define LIST_OF_XPM_TYPE\
	X(XPm_Node)\
	X(XPm_Subsystem)\
	X(XPm_Power)\
	X(XPm_ClockNode)\
	X(XPm_ResetNode)\
	X(XPm_PinNode)\
	X(XPm_Device)\
	X(XPm_Iso)\
	X(XPm_PowerDomain)\
	X(XPm_PmcDomain)\
	X(XPm_PsFpDomain)\
	X(XPm_PsLpDomain)\
	X(XPm_NpDomain)\
	X(XPm_CpmDomain)\
	X(XPm_PlDomain)\
	X(XPm_HnicxDomain)\
	X(XPm_Rail)\
	X(XPm_Regulator)\
	X(XPm_PllClockNode)\
	X(XPm_OutClockNode)\
	X(XPm_PlDevice)\
	X(XPm_MemCtrlrDevice)\
	X(XPm_MemRegnDevice)\
	X(XPm_Core)\
	X(XPm_Pmc)\
	X(XPm_Psm)\
	X(XPm_ApuCore)\
	X(XPm_RpuCore)\
	X(XPm_MemDevice)\
	X(XPm_Periph)\
	X(XPm_Requirement)
#define INDEX(T) Index_##T
#define X(T) INDEX(T),
enum {
	INDEX(XPm_Invalid),
	LIST_OF_XPM_TYPE
	INDEX(XPm_Max)
};
#undef X

#define RoundToWordAddr(Addr) (((Addr)+3)&(~0x03U))

typedef struct XPm_SaveRegionInfo {
	u32 Offset;
	u32 Size;
} XPm_SaveRegionInfo;

#define RESTORE_REGION(SavedNode, SrInfo, Node) \
		XPmUpdate_RegionRestore(\
			(u8*)((u32)SavedNode + SrInfo.Offset), SrInfo.Size,\
			(u8*)(&((Node)->save)), sizeof((Node)->save));

void XPmUpdate_AllNodes_Add(XPm_Node* Node);
XStatus XPmUpdate_RestoreAllNodes(void);

#define XPM_UPDATE_THROW_IF_ERROR(Status, NodeData) { \
	if (XST_SUCCESS != Status) { \
		PmErr("Error:[%s %d] NodeId=0x%x Status = 0x%x\n\r", __func__, __LINE__,NodeData?((XPm_Node*)NodeData)->Id:0, Status); \
	} \
}
/* Calling restore of type on 2 params : saved and current objects*/
#define RESTORE(TYPE, SAVED_OBJ, OBJ) TYPE##_Restore((TYPE*)SAVED_OBJ,(TYPE*)OBJ);
#define GENERIC_RESTORE(TYPE, SAVED_OBJ, OBJ) TYPE##_Generic_Restore((TYPE*)SAVED_OBJ,(TYPE*)OBJ);

#define MAKE_GENERIC_RESTORE_FUNC(Type, BaseType, BaseName) \
static XStatus Type##_Generic_Restore(Type *SavedNode, Type *Node) {\
	XStatus Status = XST_FAILURE; \
	Status = BaseType## _Restore(&(SavedNode->BaseName), &(Node->BaseName));\
	if (XST_SUCCESS != Status) {\
		    goto done;\
	}\
	XPm_SaveRegionInfo SrInfo = AllSaveRegionsInfo[Index_##Type];\
	Status = RESTORE_REGION(SavedNode, SrInfo, Node);\
	if (XST_SUCCESS != Status) {\
		goto done;\
	}\
	Status = XST_SUCCESS;\
done:\
	return Status;\
}

#define MAKE_RESTORE_FUNC(Type, BaseType, BaseName) \
MAKE_GENERIC_RESTORE_FUNC(Type, BaseType, BaseName) \
static XStatus Type##_Restore(Type *SavedNode, Type *Node) {\
	XStatus Status = XST_FAILURE; \
	Status = Type##_Generic_Restore(SavedNode, Node);\
	if (XST_SUCCESS != Status) {\
		goto done;\
	}\
	Status = XST_SUCCESS;\
done:\
	return Status;\
}
XStatus XPmUpdate_ShutdownPsm(void);
XStatus XPmUpdate_ResetPsm(void);
void XPmUpdate_PsmUpdateSetState(u32 State);
u32 XPmUpdate_PsmUpdateGetState(void);
XStatus XPmUpdate_LoadPsmElf(void);
int XPmUpdate_ShutdownHandler(XPlmi_ModuleOp Op);
#ifdef __cplusplus
}
#endif

#endif /* XPM_UPDATE_H_ */
