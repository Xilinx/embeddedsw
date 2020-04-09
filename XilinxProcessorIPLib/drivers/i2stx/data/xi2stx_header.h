/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
 * @file xi2stx_header.h
 * @addtogroup i2stx_v1_0
 * @{
 *
 * <pre>
 *
 * MODIFICATION HISTORY:i
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    01/25/18 First release.
 * </pre>
 *
 *****************************************************************************/
#ifndef XI2STX_HEADER_H       /* prevent circular inclusions */
#define XI2STX_HEADER_H       /* by using protection macros  */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int I2sSelfTestExample(u16 DeviceId);

#endif /* end of protection macro */
