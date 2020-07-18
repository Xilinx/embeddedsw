/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


#ifndef XPSMFW_API_H_
#define XPSMFW_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"
#include "xpsmfw_default.h"

#define NODE_CLASS_SHIFT	26U
#define NODE_SUBCLASS_SHIFT	20U
#define NODE_TYPE_SHIFT		14U
#define NODE_INDEX_SHIFT	0U
#define NODE_CLASS_MASK_BITS    0x3FU
#define NODE_SUBCLASS_MASK_BITS 0x3FU
#define NODE_TYPE_MASK_BITS     0x3FU
#define NODE_INDEX_MASK_BITS    0x3FFFU
#define NODE_CLASS_MASK         (NODE_CLASS_MASK_BITS << NODE_CLASS_SHIFT)
#define NODE_SUBCLASS_MASK      (NODE_SUBCLASS_MASK_BITS << NODE_SUBCLASS_SHIFT)
#define NODE_TYPE_MASK          (NODE_TYPE_MASK_BITS << NODE_TYPE_SHIFT)
#define NODE_INDEX_MASK         (NODE_INDEX_MASK_BITS << NODE_INDEX_SHIFT)

#define NODEID(CLASS, SUBCLASS, TYPE, INDEX)	\
	((((CLASS) & NODE_CLASS_MASK_BITS) << NODE_CLASS_SHIFT) | \
	(((SUBCLASS) & NODE_SUBCLASS_MASK_BITS) << NODE_SUBCLASS_SHIFT) | \
	(((TYPE) & NODE_TYPE_MASK_BITS) << NODE_TYPE_SHIFT) | \
	(((INDEX) & NODE_INDEX_MASK_BITS) << NODE_INDEX_SHIFT))

#define XPSMFW_NODECLASS_DEVICE		(6U)
#define XPSMFW_NODESUBCL_DEV_CORE	(1U)
#define XPSMFW_NODETYPE_DEV_CORE_APU	(3U)
#define XPSMFW_NODETYPE_DEV_CORE_RPU	(4U)
#define XPSMFW_NODEIDX_DEV_ACPU_0	(3U)
#define XPSMFW_NODEIDX_DEV_ACPU_1	(4U)
#define XPSMFW_NODEIDX_DEV_RPU0_0	(5U)
#define XPSMFW_NODEIDX_DEV_RPU0_1	(6U)

#define XPSMFW_DEV_ACPU_0	NODEID(XPSMFW_NODECLASS_DEVICE,		\
				       XPSMFW_NODESUBCL_DEV_CORE,	\
				       XPSMFW_NODETYPE_DEV_CORE_APU,	\
				       XPSMFW_NODEIDX_DEV_ACPU_0)

#define XPSMFW_DEV_ACPU_1	NODEID(XPSMFW_NODECLASS_DEVICE,		\
				       XPSMFW_NODESUBCL_DEV_CORE,	\
				       XPSMFW_NODETYPE_DEV_CORE_APU,	\
				       XPSMFW_NODEIDX_DEV_ACPU_1)

#define XPSMFW_DEV_RPU0_0	NODEID(XPSMFW_NODECLASS_DEVICE,		\
				       XPSMFW_NODESUBCL_DEV_CORE,	\
				       XPSMFW_NODETYPE_DEV_CORE_RPU,	\
				       XPSMFW_NODEIDX_DEV_RPU0_0)

#define XPSMFW_DEV_RPU0_1	NODEID(XPSMFW_NODECLASS_DEVICE,		\
				       XPSMFW_NODESUBCL_DEV_CORE,	\
				       XPSMFW_NODETYPE_DEV_CORE_RPU,	\
				       XPSMFW_NODEIDX_DEV_RPU0_1)

#define PM_PSM_TO_PLM_EVENT	(1U)

#define PSM_API_DIRECT_PWR_DWN	(1U)
#define PSM_API_DIRECT_PWR_UP	(2U)
#define PSM_API_FPD_HOUSECLEAN	(3U)
#define PSM_API_CCIX_EN		(4U)

/**
 *  PM init node functions
 */
enum XPmInitFunctions {
	FUNC_INIT_START,
	FUNC_INIT_FINISH,
	FUNC_SCAN_CLEAR,
	FUNC_BISR,
	FUNC_MBIST_LBIST,
	FUNC_ME_INITREG,
	FUNC_MBIST_CLEAR,
};

XStatus XPsmFw_NotifyPlmEvent(void);
XStatus XPsmFw_ProcessIpi(u32 *Payload);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_API_H_ */
