/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
******************************************************************************/

#ifndef XPFW_MOD_RPU_H_
#define XPFW_MOD_RPU_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Macros for RPU_0 Status, Cfg and standby mode masks */
#define RPU_0_CFG_REG			0xFF9A0100U
#define RPU_0_STATUS_REG		0xFF9A0104U
#define RUN_MODE_MASK			0x6U
#define RPU_HALT_MASK			0x1U

/* Mask to know RPU_0 is powered down */
#define RPU_POWER_UP_MASK 		0x400U

/* Macros to indicate STL task started on PMU */
#define STL_STARTED		0x20000000U
#define CHECK_STL_STARTED 		100U
#define XPFW_RPU_RUNMODE_TIME 	100U

void ModRpuInit(void);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_MOD_RPU_H_ */
