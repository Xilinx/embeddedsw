/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_SUBSYSTEM_H_
#define XPM_SUBSYSTEM_H_

#include "xpm_defs.h"
#include "xstatus.h"

#ifdef __cplusplus
extern "C" {
#endif

#define	INVALID_SUBSYSID			(0xFFFFFFFFU)
/************************** Function Prototypes ******************************/
XStatus XPmSubsystem_Configure(u32 SubsystemId);
u32 XPmSubsystem_GetSubSysIdByIpiMask(u32 IpiMask);
#ifdef __cplusplus
}
#endif

#endif /* XPM_SUBSYSTEM_H_ */
