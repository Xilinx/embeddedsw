/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_REGNODE_H_
#define XPM_REGNODE_H_

#include "xil_types.h"
#include "xpm_power.h"
typedef struct XPm_RegNode XPm_RegNode;
#ifdef __cplusplus
extern "C" {
#endif
/**
 * Regnode class.
 */
struct XPm_RegNode {
	u32 Id;				/**< Node Id for regnode */
	u32 BaseAddress;		/**< Base address of given node */
	u32 Requirements;		/**< Requirements from different subsystems */
	XPm_Power *Power;		/**< Parent power node */
	XPm_RegNode *NextRegnode;	/**< Link to next regnode */
};

XPm_RegNode* XPmRegNode_GetNodes(void);
void XPmRegNode_Init(XPm_RegNode *RegNode, u32 NodeId, u32 BaseAddress, XPm_Power *Power);

#ifdef __cplusplus
}
#endif
#endif /* XPM_REGNODE_H_ */