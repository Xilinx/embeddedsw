/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_INTERRUPTS_H_
#define XPFW_INTERRUPTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xpfw_default.h"

void XPfw_InterruptHandler(void) __attribute__ ((interrupt_handler));
void XPfw_InterruptDisable(u32 Mask);
void XPfw_InterruptEnable(u32 Mask);
void XPfw_InterruptStart(void);
void XPfw_InterruptInit(void);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_INTERRUPTS_H_ */
