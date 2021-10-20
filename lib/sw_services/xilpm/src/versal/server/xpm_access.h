/******************************************************************************
 * Copyright (c) 2021 - 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef XPM_ACCESS_H_
#define XPM_ACCESS_H_

#include "xpm_node.h"
#include "xpm_power.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Regnode class.
 */
typedef struct XPm_RegNode XPm_RegNode;

struct XPm_RegNode {
	u32 Id;				/**< Node Id for regnode */
	u32 BaseAddress;		/**< Base address of given node */
	XPm_Power *Power;		/**< Parent power node */
	XPm_RegNode *NextRegnode;	/**< Link to next regnode */
};

/************************** Function Prototypes ******************************/
void XPmAccess_RegnodeInit(XPm_RegNode *RegNode,
			   u32 NodeId, u32 BaseAddress, XPm_Power *Power);

#ifdef __cplusplus
}
#endif

#endif /* XPM_ACCESS_H_ */
