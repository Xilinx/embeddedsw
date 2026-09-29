/******************************************************************************
* Copyright (C) 2007 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef MBOX_HEADER_H
#define MBOX_HEADER_H

/*****************************************************************************/
/**
*
* @file mbox_header.h
*
* @brief Declares prototypes for the Mailbox driver examples.
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xstatus.h"

/** @cond INTERNAL */
#ifndef SDT
int MailboxExample(u16 MailboxDeviceID);
#else
int MailboxExample(XMbox *MboxInstancePtr,UINTPTR BaseAddress);
#endif
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif

