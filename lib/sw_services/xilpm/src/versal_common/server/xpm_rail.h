/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_RAIL_H_
#define XPM_RAIL_H_

#include "xstatus.h"
#include "xpm_rail_plat.h"
#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_1_DEVICE_ID) || \
    defined (XPAR_XIICPS_2_DEVICE_ID) || defined (XPAR_XIICPS_0_BASEADDR)  || \
    defined (XPAR_XIICPS_1_BASEADDR)  || defined (XPAR_XIICPS_2_BASEADDR)
#include "xiicps.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/************************** Function Prototypes ******************************/
XStatus XPmRail_Control(XPm_Rail *Rail, u8 State, u8 Mode);
XStatus XPmRail_Init(XPm_Rail *Rail, u32 RailId, const u32 *Args, u32 NumArgs);
#if defined (XPAR_XIICPS_0_DEVICE_ID) || defined (XPAR_XIICPS_1_DEVICE_ID) || \
    defined (XPAR_XIICPS_2_DEVICE_ID) || defined (XPAR_XIICPS_0_BASEADDR)  || \
    defined (XPAR_XIICPS_1_BASEADDR)  || defined (XPAR_XIICPS_2_BASEADDR)
XIicPs *XPmRail_GetIicInstance(void);
XStatus I2CInitialize(XIicPs *Iic, const u32 ControllerID);
#if defined (RAIL_CONTROL)
XStatus I2CIdleBusWait(XIicPs *Iic);
XStatus I2CWrite(XIicPs *Iic, u16 SlaveAddr, u8 *Buffer, s32 ByteCount);
#endif /* RAIL_CONTROL */
#endif

#ifdef __cplusplus
}
#endif

#endif /* XPM_RAIL_H_ */
