/* $Id: xil_testlib.h,v 1.00 2009/2/19  */
/******************************************************************************
*
* (c) Copyright 2010-13 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/

/***************************** Include Files *********************************/
#include <xil_printf.h>
#include "intg.h"

/**
*
* @file xil_testlib.h
*
* This file contains Xilinx software exit messages.These messages are set to be
* matched with the scan strings for board test such that board test would know
* the result of a standalone test program
*
******************************************************************************/

#ifndef XIL_TESTLIB_H			/**< prevent circular inclusions */
#define XIL_TESTLIB_H			/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * XIL_PASS puts out a message to signal board test that a program has pass
 * the testing program without errors
 *
 * param	None.
 *
 * @return	None.
 *
 * @note	None.
 */
#define XIL_PASS() xil_printf("\r\nREGRESSION TEST PASS.\r\n")

/*
 * XIL_FAIL puts out a message to signal board test that a program has not pass
 * the testing program
 *
 * @param	number of errors.
 *
 * @return	None.
 *
 * @note	None.
 */
#define XIL_FAIL(numErrors) \
		xil_printf("\r\nREGRESSION TEST FAIL WITH %d ERRORS.\r\n", numErrors)


#ifdef __cplusplus
}
#endif

#endif /**< End of protection macro */
