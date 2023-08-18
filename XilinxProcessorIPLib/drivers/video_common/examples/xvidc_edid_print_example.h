/*******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xvidc_edid_print_example.h
 *
 * Contains an example that, given a supplied base Extended Display
 * Identification Data (EDID) structure, will parse, decode, and print its
 * contents.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  11/09/14 Initial release.
 * </pre>
 *
*******************************************************************************/

#ifndef XVIDC_EDID_PRINT_H_
/* Prevent circular inclusions by using protection macros. */
#define XVIDC_EDID_PRINT_H_

/******************************* Include Files ********************************/

#include "xdp.h"
#include "xvidc_edid.h"

/**************************** Function Prototypes *****************************/

u32 Edid_PrintDecodeBase(u8 *EdidRaw);
void Edid_PrintSuppVmTable(u8 *EdidRaw);

#endif /* XVIDC_EDID_PRINT_H_ */
