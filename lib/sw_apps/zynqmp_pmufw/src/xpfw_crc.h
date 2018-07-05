/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_CRC_H_
#define XPFW_CRC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpfw_default.h"

#ifdef ENABLE_IPI_CRC

u32 XPfw_CalculateCRC(u32 BufAddr, u32 BufSize);

#endif /* ENABLE_IPI_CRC */

#ifdef __cplusplus
}
#endif

#endif /* XPFW_CRC_H_ */
