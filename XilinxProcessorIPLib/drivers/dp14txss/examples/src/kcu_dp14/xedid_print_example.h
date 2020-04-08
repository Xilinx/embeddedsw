/*******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

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

#include "xdp.h"
#include "xvidc_edid.h"

u32 Edid_PrintDecodeBase(u8 *EdidRaw);
void Edid_Print_Supported_VideoModeTable(u8 *EdidRaw);
void XDptx_DbgPrintEdid(XDp *InstancePtr);

#endif /* XEDID_PRINT_EXAMPLE_H_ */
