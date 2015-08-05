/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
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
 */

/*********************************************************************
 * Definitions of commonly used enums that have to match definitions
 * that all software layer in the system use.
 *********************************************************************/

#ifndef PM_DEFS_H_
#define PM_DEFS_H_

/*********************************************************************
 * Macro definitions
 ********************************************************************/

/*
 * Version number is a 32bit value, like:
 * (PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR
 */
#define PM_VERSION_MAJOR    0U
#define PM_VERSION_MINOR    1U

#define PM_VERSION	((PM_VERSION_MAJOR << 16) | PM_VERSION_MINOR)

/*
 * Capabilities common for all slave nodes (common capabilities should take
 * lower 16 bits, specific capabilities of each slave take higher 16 bits)
 */
#define PM_CAP_ACCESS       0x1U
#define PM_CAP_CONTEXT      0x2U
#define PM_CAP_WAKEUP       0x4U

#define MAX_LATENCY (~0U)
#define MAX_QOS     100U

/* System shutdown macros */
#define PM_SHUTDOWN	0U
#define PM_RESTART	1U

/* PM API ids */
#define PM_GET_API_VERSION          1U
#define PM_SET_CONFIGURATION        2U
#define PM_GET_NODE_STATUS          3U
#define PM_GET_OP_CHARACTERISTIC    4U
#define PM_REGISTER_NOTIFIER        5U

#define PM_REQUEST_SUSPEND          6U
#define PM_SELF_SUSPEND             7U
#define PM_FORCE_POWERDOWN          8U
#define PM_ABORT_SUSPEND            9U
#define PM_REQUEST_WAKEUP           10U
#define PM_SET_WAKEUP_SOURCE        11U
#define PM_SYSTEM_SHUTDOWN          12U

#define PM_REQUEST_NODE             13U
#define PM_RELEASE_NODE             14U
#define PM_SET_REQUIREMENT          15U
#define PM_SET_MAX_LATENCY          16U

#define PM_RESET_ASSERT             17U
#define PM_RESET_GET_STATUS         18U
#define PM_MMIO_WRITE               19U
#define PM_MMIO_READ                20U

#define PM_API_MIN	PM_GET_API_VERSION
#define PM_API_MAX	PM_MMIO_READ

/* PM API callback ids */
#define PM_INIT_SUSPEND_CB      30U
#define PM_ACKNOWLEDGE_CB       31U
#define PM_NOTIFY_CB            32U

/* Nodes */
#define NODE_UNKNOWN    0U
#define NODE_APU        1U
#define NODE_APU_0      2U
#define NODE_APU_1      3U
#define NODE_APU_2      4U
#define NODE_APU_3      5U
#define NODE_RPU        6U
#define NODE_RPU_0      7U
#define NODE_RPU_1      8U
#define NODE_PL         9U
#define NODE_FPD        10U
#define NODE_OCM_BANK_0 11U
#define NODE_OCM_BANK_1 12U
#define NODE_OCM_BANK_2 13U
#define NODE_OCM_BANK_3 14U
#define NODE_TCM_0_A    15U
#define NODE_TCM_0_B    16U
#define NODE_TCM_1_A    17U
#define NODE_TCM_1_B    18U
#define NODE_L2         19U
#define NODE_GPU_PP_0   20U
#define NODE_GPU_PP_1   21U
#define NODE_USB_0      22U
#define NODE_USB_1      23U
#define NODE_TTC_0      24U
#define NODE_TTC_1      25U
#define NODE_TTC_2      26U
#define NODE_TTC_3      27U
#define NODE_SATA       28U
#define NODE_ETH_0      29U
#define NODE_ETH_1      30U
#define NODE_ETH_2      31U
#define NODE_ETH_3      32U
#define NODE_UART_0     33U
#define NODE_UART_1     34U
#define NODE_SPI_0      35U
#define NODE_SPI_1      36U
#define NODE_I2C_0      37U
#define NODE_I2C_1      38U
#define NODE_SD_0       39U
#define NODE_SD_1       40U
#define NODE_DP         41U
#define NODE_GDMA       42U
#define NODE_ADMA       43U
#define NODE_NAND       44U
#define NODE_QSPI       45U
#define NODE_GPIO       46U
#define NODE_CAN_0      47U
#define NODE_CAN_1      48U
#define NODE_AFI        49U
#define NODE_APLL       50U
#define NODE_VPLL       51U
#define NODE_DPLL       52U
#define NODE_RPLL       53U
#define NODE_IOPLL      54U

#define NODE_MIN        NODE_APU
#define NODE_MAX        NODE_IOPLL

/* Request acknowledge argument values */
#define REQUEST_ACK_NO          1U
#define REQUEST_ACK_BLOCKING    2U
#define REQUEST_ACK_CB_STANDARD 3U
#define REQUEST_ACK_MIN         REQUEST_ACK_NO
#define REQUEST_ACK_MAX         REQUEST_ACK_CB_STANDARD

/* Abort reason argument */
#define ABORT_REASON_WKUP_EVENT 100U
#define ABORT_REASON_PU_BUSY    101U
#define ABORT_REASON_NO_PWRDN   102U
#define ABORT_REASON_UNKNOWN    103U

#define ABORT_REASON_MIN    ABORT_REASON_WKUP_EVENT
#define ABORT_REASON_MAX    ABORT_REASON_UNKNOWN

/* Suspend reason argument */
#define SUSPEND_REASON_PU_REQ       201U
#define SUSPEND_REASON_ALERT        202U
#define SUSPEND_REASON_SYS_SHUTDOWN 203U

#define SUSPEND_REASON_MIN  SUSPEND_REASON_PU_REQ
#define SUSPEND_REASON_MAX  SUSPEND_REASON_SYS_SHUTDOWN

/* Operating characteristics type */
#define PM_OPCHAR_TYPE_POWER    1U
#define PM_OPCHAR_TYPE_ENERGY   2U
#define PM_OPCHAR_TYPE_TEMP     3U

/* Power management specific return error statuses */
#define XST_PM_INTERNAL		2000L
#define XST_PM_CONFLICT		2001L
#define XST_PM_NO_ACCESS	2002L
#define XST_PM_INVALID_NODE	2003L
#define XST_PM_DOUBLE_REQ	2004L
#define XST_PM_ABORT_SUSPEND	2005L

#endif
