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

#define INVALID_RST_SUBCLASS(Subcls)	(((u32)XPM_NODETYPE_RESET_PERIPHERAL != Subcls) && \
					((u32)XPM_NODETYPE_RESET_POR != Subcls) && \
					((u32)XPM_NODETYPE_RESET_DBG != Subcls) && \
					((u32)XPM_NODETYPE_RESET_SOFT != Subcls) && \
					((u32)XPM_NODETYPE_RESET_WARM != Subcls) && \
					((u32)XPM_NODETYPE_RESET_COLD != Subcls))

#define PM_RST_PMC_POR			PM_RST_PS_PMC_POR
#define PM_RST_PMC			PM_RST_PS_PMC_SRST
#define PM_RST_SYS_RST_1		PM_RST_NONPS_SYS_1
#define PM_RST_SYS_RST_2		PM_RST_NONPS_SYS_2
#define PM_RST_SYS_RST_3		PM_RST_NONPS_SYS_3
#define PM_RST_PL_POR			PM_RST_PS_PL_POR
#define PM_RST_NOC_POR			PM_RST_NONPS_NOC_POR
#define PM_RST_PL_SRST			PM_RST_PS_PL_SRST
#define PM_RST_NOC			PM_RST_NONPS_NOC
#define PM_RST_NPI			PM_RST_NONPS_NPI

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
