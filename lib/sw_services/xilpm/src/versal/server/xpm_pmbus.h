/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*
*
******************************************************************************/

#ifndef XPM_PMBUS_H_
#define XPM_PMBUS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"

#ifdef XPAR_XIICPS_1_DEVICE_ID
#include "xiicps.h"

/************************** Variable Declarations ****************************/
XIicPs IicInstance;
#define BUFFER_SIZE					3 /**< I2C Buffer size */

/****************************** PMBus Commands *******************************/
#define OPERATION					0x01
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
