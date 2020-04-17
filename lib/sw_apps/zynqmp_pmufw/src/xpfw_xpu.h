/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_XPU_H_
#define XPFW_XPU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"

void XPfw_XpuIntrInit(void);
void XPfw_XpuIntrAck(void);
void XPfw_XpuIntrHandler(u8 ErrorId);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_XPU_H_ */
