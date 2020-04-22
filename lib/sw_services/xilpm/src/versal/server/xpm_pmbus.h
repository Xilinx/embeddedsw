/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_PMBUS_H_
#define XPM_PMBUS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"

#ifdef XPAR_XIICPS_1_DEVICE_ID
#include "xiicps.h"

/****************************** PMBus Commands *******************************/
#define OPERATION				0x01
#define ON_OFF_CONFIG				0x02

/* Operation */
#define PM_OP_POWER_UP				0x80
#define PM_OP_POWER_DOWN			0x00
#define PM_OP_SOFT_OFF				0x40

/* On_off_config */
#define OP_POW_CTRL_CONFIG			0x1A

/************************** Function Declarations ***************************/
XStatus XPmBus_WriteByte(XIicPs *Iic, u16 SlaveAddr, u8 Command, u8 Byte);
XStatus XPmBus_WriteWord(XIicPs *Iic, u16 SlaveAddr, u8 Command, u16 Word);
XStatus XPmBus_ReadData(XIicPs *Iic, u8 *Buffer, u16 SlaveAddr,
						u8 Command, s32 ByteCount);

#ifdef __cplusplus
}
#endif

#endif /* XPAR_XIICPS_1_DEVICE_ID */
#endif /* XPM_PMBUS_H_ */
