/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef MBOX_HEADER_H		/* prevent circular inclusions */
#define MBOX_HEADER_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xstatus.h"

#ifndef SDT
int MailboxExample(u16 MailboxDeviceID);
#else
int MailboxExample(XMbox *MboxInstancePtr,UINTPTR BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif

