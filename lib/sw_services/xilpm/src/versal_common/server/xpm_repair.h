/******************************************************************************
 * Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
 * Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

#ifndef XPM_REPAIR_H_
#define XPM_REPAIR_H_

#include "xpm_bisr.h"
#include "xpm_regs.h"
#include "xpm_powerdomain.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Function Prototypes ******************************/
XStatus XPmRepair_Vdu(u32 EfuseTagAddr, u32 TagSize, u32 TagOptional, u32 *TagDataAddr);

#ifdef __cplusplus
}
#endif

#endif /* XPM_REPAIR_H_ */
