/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved..
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 * @file xi2srx_header.h
 * @addtogroup i2srx Overview
 * @{
 *
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    01/25/18 First release.
 * </pre>
 *
 *****************************************************************************/
#ifndef XI2SRX_HEADER_H       /* prevent circular inclusions */
#define XI2SRX_HEADER_H       /* by using protection macros  */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int I2srx_SelfTest_Example(u16 DeviceId);

#endif /* end of protection macro */
