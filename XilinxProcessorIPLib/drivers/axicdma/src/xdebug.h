/**************************************************************
 * (c) Copyright 2010-2011 Xilinx, Inc. All rights reserved.
 *
 * This file contains confidential and proprietary information
 * of Xilinx, Inc. and is protected under U.S. and
 * international copyright and other intellectual property
 * laws.
 *
 * DISCLAIMER
 * This disclaimer is not a license and does not grant any
 * rights to the materials distributed herewith. Except as
 * otherwise provided in a valid license issued to you by
 * Xilinx, and to the maximum extent permitted by applicable
 * law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
 * WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
 * AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
 * BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
 * INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
 * (2) Xilinx shall not be liable (whether in contract or tort,
 * including negligence, or under any other theory of
 * liability) for any loss or damage of any kind or nature
 * related to, arising under or in connection with these
 * materials, including for any direct, or any indirect,
 * special, incidental, or consequential loss or damage
 * (including loss of data, profits, goodwill, or any type of
 * loss or damage suffered as a result of any action brought
 * by a third party) even if such damage or loss was
 * reasonably foreseeable or Xilinx had been advised of the
 * possibility of the same.
 *
 * CRITICAL APPLICATIONS
 * Xilinx products are not designed or intended to be fail-
 * safe, or for use in any application requiring fail-safe
 * performance, such as life-support or safety devices or
 * systems, Class III medical devices, nuclear facilities,
 * applications related to the deployment of airbags, or any
 * other applications that could lead to death, personal
 * injury, or severe property or environmental damage
 * (individually and collectively, "Critical
 * Applications"). Customer assumes the sole risk and
 * liability of any use of Xilinx products in Critical
 * Applications, subject only to applicable laws and
 * regulations governing limitations on product liability.
 *
 * THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
 * PART OF THIS FILE AT ALL TIMES.
 *************************************************************/
#ifndef _XDEBUG_H
#define _XDEBUG_H

#if defined(DEBUG) && !defined(NDEBUG)

#include <stdio.h>

#ifndef XDEBUG_WARNING
#define XDEBUG_WARNING
#warning DEBUG is enabled
#endif

#define XDBG_DEBUG_ERROR             0x00000001    /* error  condition messages */
#define XDBG_DEBUG_GENERAL           0x00000002    /* general debug  messages */
#define XDBG_DEBUG_ALL               0xFFFFFFFF    /* all debugging data */

#define XDBG_DEBUG_FIFO_REG          0x00000100    /* display register reads/writes */
#define XDBG_DEBUG_FIFO_RX           0x00000101    /* receive debug messages */
#define XDBG_DEBUG_FIFO_TX           0x00000102    /* transmit debug messages */
#define XDBG_DEBUG_FIFO_ALL          0x0000010F    /* all fifo debug messages */

#define XDBG_DEBUG_TEMAC_REG         0x00000400    /* display register reads/writes */
#define XDBG_DEBUG_TEMAC_RX          0x00000401    /* receive debug messages */
#define XDBG_DEBUG_TEMAC_TX          0x00000402    /* transmit debug messages */
#define XDBG_DEBUG_TEMAC_ALL         0x0000040F    /* all temac  debug messages */

#define XDBG_DEBUG_TEMAC_ADPT_RX     0x00000800    /* receive debug messages */
#define XDBG_DEBUG_TEMAC_ADPT_TX     0x00000801    /* transmit debug messages */
#define XDBG_DEBUG_TEMAC_ADPT_IOCTL  0x00000802    /* ioctl debug messages */
#define XDBG_DEBUG_TEMAC_ADPT_MISC   0x00000803    /* debug msg for other routines */
#define XDBG_DEBUG_TEMAC_ADPT_ALL    0x0000080F    /* all temac adapter debug messages */

//#define xdbg_current_types (XDBG_DEBUG_ERROR | XDBG_DEBUG_GENERAL)
#define xdbg_current_types (XDBG_DEBUG_ERROR)

#define xdbg_stmnt(x)  x

/* In VxWorks, if _WRS_GNU_VAR_MACROS is defined, special syntax is needed for
 * macros that accept variable number of arguments
 */
#if defined(XENV_VXWORKS) && defined(_WRS_GNU_VAR_MACROS)
#define xdbg_printf(type, args...) (((type) & xdbg_current_types) ? printf (## args) : 0)

#else /* ANSI Syntax */

#define xdbg_printf(type, ...) (((type) & xdbg_current_types) ? printf (__VA_ARGS__) : 0)

#endif

#else /* defined(DEBUG) && !defined(NDEBUG) */

#define xdbg_stmnt(x)

/* See VxWorks comments above */
#if defined(XENV_VXWORKS) && defined(_WRS_GNU_VAR_MACROS)
#define xdbg_printf(type, args...)
#else /* ANSI Syntax */
#define xdbg_printf(...)
#endif

#endif /* defined(DEBUG) && !defined(NDEBUG) */

#endif /* _XDEBUG_H */
