/******************************************************************************
 * Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#ifndef XPM_UPDATE_H_
#define XPM_UPDATE_H_
#include "xpm_debug.h"
#include "xpm_common.h"
#include "xpm_node.h"
#ifdef __cplusplus
extern "C" {
#endif

#define RESTORE_REGION(SavedNode, Node) \
		XPmUpdate_RegionRestore((u32*)(&((SavedNode).save)), sizeof((SavedNode).save), (u32*)(&((Node).save)), sizeof((Node).save));

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
	Status = RESTORE_REGION(*SavedNode, *Node);\
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

#ifdef __cplusplus
}
#endif

#endif /* XPM_UPDATE_H_ */