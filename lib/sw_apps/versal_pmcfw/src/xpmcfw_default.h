/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/


#ifndef XPMCFW_DEFAULT_H_
#define XPMCFW_DEFAULT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* BSP Hdrs */
#include "xil_io.h"
#include "xil_types.h"
#include "mb_interface.h"
#include "xstatus.h"
#include "xparameters.h"

/* PMC FW Hdrs */
#include "xpmcfw_config.h"
#include "xpmcfw_hw.h"
#include "xpmcfw_hooks.h"
#include "xpmcfw_misc.h"
#include "xpmcfw_dma.h"

/* library headers */
#include "xpmcfw_util.h"
#include "xilcdo.h"
#include "xilpdi.h"

/* common headers  */
#include "xpmcfw_debug.h"
#include "xpmcfw_err.h"

/* Register Access Macros */
#define XPmcFw_Write32(Addr, Value)  Xil_Out32((Addr), (Value))
#define XPmcFw_Read32(Addr)  Xil_In32((Addr))
#define XPmcFw_RMW32  XPmcFw_UtilRMW

/* General defines */
#define XPMCFW_32BIT_MASK		(0xFFFFFFFFU)

/* Handler Table Structure */
typedef void (*VoidFunction_t)(void);
struct HandlerTable {
	u32 Mask;
	VoidFunction_t Handler;
};

#ifdef __cplusplus
}
#endif

#endif /* XPMCFW_DEFAULT_H_ */
