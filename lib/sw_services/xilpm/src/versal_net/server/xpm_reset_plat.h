/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_RESET_PLAT_H_
#define XPM_RESET_PLAT_H_

#include "xpm_node.h"
#include "xpm_common.h"
#include "xpm_subsystem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INVALID_RST_SUBCLASS(Subcls)	(((u32)XPM_NODETYPE_RESET_PERIPHERAL != (Subcls)) && \
					((u32)XPM_NODETYPE_RESET_POR != (Subcls)) && \
					((u32)XPM_NODETYPE_RESET_DBG != (Subcls)) && \
					((u32)XPM_NODETYPE_RESET_SRST != (Subcls)) && \
					((u32)XPM_NODETYPE_RESET_WARM != (Subcls)) && \
					((u32)XPM_NODETYPE_RESET_COLD != (Subcls)))

/************************** Function Prototypes ******************************/

maybe_unused static inline XStatus XPmReset_PlatSystemReset(void)
{
	return XST_SUCCESS;
}
maybe_unused static inline void *GetResetCustomOps(u32 ResetId)
{
	(void)ResetId;
	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif /* XPM_RESET_PLAT_H_ */
