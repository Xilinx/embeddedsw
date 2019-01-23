/******************************************************************************
 *
 * Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/

#ifndef XPSMFW_API_H_
#define XPSMFW_API_H_

#include "xparameters.h"
#include "xpsmfw_default.h"

#define NODE_CLASS_SHIFT	24U
#define NODE_SUBCLASS_SHIFT	16U
#define NODE_TYPE_SHIFT		8U
#define NODE_INDEX_SHIFT	0U
#define NODE_CLASS_MASK		0xFF000000U
#define NODE_SUBCLASS_MASK	0x00FF0000U
#define NODE_TYPE_MASK		0x0000FF00U
#define NODE_INDEX_MASK		0x000000FFU

#define NODEID(CLASS, SUBCLASS, TYPE, INDEX)		\
	((((CLASS) & 0xFF) << NODE_CLASS_SHIFT) |	\
	(((SUBCLASS) & 0xFF) << NODE_SUBCLASS_SHIFT) |	\
	(((TYPE) & 0xFF) << NODE_TYPE_SHIFT) |		\
	(((INDEX) & 0xFF) << NODE_INDEX_SHIFT))

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

#define PM_PWR_DWN_EVENT	(1U)

#define PSM_API_DIRECT_PWR_DWN	(1U)
#define PSM_API_DIRECT_PWR_UP	(2U)

XStatus XPsmFw_PowerDownEvent(u32 DevId);
XStatus XPsmFw_ProcessIpi(u32 *Payload);

#endif /* XPSMFW_API_H_ */
