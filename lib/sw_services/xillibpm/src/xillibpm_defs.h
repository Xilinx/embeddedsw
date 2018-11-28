/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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

#ifndef XILLIBPM_DEFS_H_
#define XILLIBPM_DEFS_H_

/* PM API ids */
#define PM_GET_API_VERSION              1U
#define PM_SET_CONFIGURATION            2U
#define PM_GET_DEVICE_STATUS            3U
#define PM_GET_OP_CHARACTERISTIC        4U
#define PM_REGISTER_NOTIFIER            5U

#define PM_REQUEST_SUSPEND              6U
#define PM_SELF_SUSPEND                 7U
#define PM_FORCE_POWERDOWN              8U
#define PM_ABORT_SUSPEND                9U
#define PM_REQUEST_WAKEUP               10U
#define PM_SET_WAKEUP_SOURCE            11U
#define PM_SYSTEM_SHUTDOWN              12U

#define PM_REQUEST_DEVICE                13U
#define PM_RELEASE_DEVICE                14U
#define PM_SET_REQUIREMENT              15U
#define PM_SET_MAX_LATENCY              16U

#define PM_RESET_ASSERT                 17U
#define PM_RESET_GET_STATUS             18U
#define PM_MMIO_WRITE                   19U
#define PM_MMIO_READ                    20U

#define PM_INIT_FINALIZE                21U

#define PM_FPGA_LOAD                    22U
#define PM_FPGA_GET_STATUS              23U

#define PM_GET_CHIPID                   24U

#define PM_SECURE_RSA_AES               25U
#define PM_SECURE_SHA                   26U
#define PM_SECURE_RSA                   27U

#define PM_PINCTRL_REQUEST              28U
#define PM_PINCTRL_RELEASE              29U
#define PM_PINCTRL_GET_FUNCTION         30U
#define PM_PINCTRL_SET_FUNCTION         31U
#define PM_PINCTRL_CONFIG_PARAM_GET     32U
#define PM_PINCTRL_CONFIG_PARAM_SET     33U

#define PM_IOCTL                        34U

#define PM_QUERY_DATA                   35U

#define PM_CLOCK_ENABLE                 36U
#define PM_CLOCK_DISABLE                37U
#define PM_CLOCK_GETSTATE               38U
#define PM_CLOCK_SETDIVIDER             39U
#define PM_CLOCK_GETDIVIDER             40U
#define PM_CLOCK_SETRATE                41U
#define PM_CLOCK_GETRATE                42U
#define PM_CLOCK_SETPARENT              43U
#define PM_CLOCK_GETPARENT              44U
#define PM_SECURE_IMAGE                 45U
#define PM_FPGA_READ                    46U

#define PM_CREATE_SUBSYSTEM             47U
#define PM_DESTROY_SUBSYSTEM             48U

#define PM_DESCRIBE_NODES				49U
#define	PM_ADD_NODE						50U
#define	PM_ADD_NODE_PARENT				51U
#define	PM_ADD_NODE_NAME				52U

#define PM_PLL_SET_PARAMETER			53U
#define PM_PLL_GET_PARAMETER			54U
#define PM_PLL_SET_MODE					55U
#define PM_PLL_GET_MODE					56U

#define PM_API_MIN      PM_GET_API_VERSION
#define PM_API_MAX      PM_PLL_GET_MODE


#endif /* XILLIBPM_DEFS_H_ */
