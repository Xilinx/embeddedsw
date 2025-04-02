/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xedid_print_example.h
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  YB    07/01/15 Initial release.
* </pre>
*
******************************************************************************/

#ifndef XEDID_PRINT_EXAMPLE_H_
#define XEDID_PRINT_EXAMPLE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "xdp.h"
#include "xvidc_edid.h"

u32 Edid_PrintDecodeBase(u8 *EdidRaw);
void Edid_Print_Supported_VideoModeTable(u8 *EdidRaw);
void XDptx_DbgPrintEdid(XDp *InstancePtr);
#ifdef __cplusplus
}
#endif
#endif /* XEDID_PRINT_EXAMPLE_H_ */
