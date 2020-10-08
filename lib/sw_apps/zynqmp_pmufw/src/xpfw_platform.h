/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_PLATFORM_H_
#define XPFW_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"

#define XPFW_PLATFORM_PS_V1    0x0U
#define	PBR_VERSION_REG			0xFFD07FCCU

u8 XPfw_PlatformGetPsVersion(void);
void XPfw_PrintPBRVersion(u32 xpbr_version);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_PLATFORM_H_ */
