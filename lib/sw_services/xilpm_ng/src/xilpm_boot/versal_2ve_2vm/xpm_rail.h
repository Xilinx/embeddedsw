/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RAIL_H_
#define XPM_RAIL_H_

#include "xstatus.h"
#include "xpm_vid.h"
#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_1_DEVICE_ID) || \
    defined (XPAR_XIICPS_2_DEVICE_ID)
#include "xiicps.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/************************** Function Prototypes ******************************/
XStatus XPmRail_Control(XPm_Rail *Rail, u8 State, u8 Mode);
XStatus XPmRail_Init(XPm_Rail *Rail, u32 RailId, const u32 *Args, u32 NumArgs);
#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_1_DEVICE_ID) || \
    defined (XPAR_XIICPS_2_DEVICE_ID)
XIicPs *XPmRail_GetIicInstance(void);
XStatus I2CInitialize(XIicPs *Iic, const u32 ControllerID);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XPM_RAIL_H_ */
