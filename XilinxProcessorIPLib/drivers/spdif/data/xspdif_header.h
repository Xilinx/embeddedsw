/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 * @file xspdif_header.h
 * @addtogroup spdif
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
#ifndef XSPDIF_HEADER_H       /* prevent circular inclusions */
#define XSPDIF_HEADER_H       /* by using protection macros  */

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

int SpdifSelfTestExample(u16 DeviceId);

#endif /* end of protection macro */
