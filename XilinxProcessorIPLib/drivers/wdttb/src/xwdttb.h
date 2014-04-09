/* $Id: xwdttb.h,v 1.1.2.1 2009/12/04 05:52:37 svemula Exp $ */
/******************************************************************************
*
* (c) Copyright 2001-2009 Xilinx, Inc. All rights reserved.
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
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
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
/**
*
* @file xwdttb.h
*
* The Xilinx watchdog timer/timebase component supports the Xilinx watchdog
* timer/timebase hardware. More detailed description of the driver operation
* for each function can be found in the xwdttb.c file.
*
* The Xilinx watchdog timer/timebase driver supports the following features:
*   - Polled mode
*   - enabling and disabling (if allowed by the hardware) the watchdog timer
*   - restarting the watchdog.
*   - reading the timebase.
*
* It is the responsibility of the application to provide an interrupt handler
* for the timebase and the watchdog and connect them to the interrupt
* system if interrupt driven mode is desired.
*
* The watchdog timer/timebase component ALWAYS generates an interrupt output
* when:
*   - the watchdog expires the first time
*   - the timebase rolls over
*
* and ALWAYS generates a reset output when the watchdog timer expires a second
* time. This is not configurable in any way from the software driver's
* perspective.
*
* The Timebase is reset to 0 when the Watchdog Timer is enabled.
*
* If the hardware interrupt signal is not connected, polled mode is the only
* option (using IsWdtExpired) for the watchdog. Reset output will occur for the
* second watchdog timeout regardless. Polled mode for the timebase rollover is
* just reading the contents of the register and seeing if the MSB has
* transitioned from 1 to 0.
*
* The IsWdtExpired function is used for polling the watchdog timer and it is
* also used to check if the watchdog was the cause of the last reset. In this
* situation, call Initialize then call WdtIsExpired. If the result is true
* watchdog timeout caused the last system reset. It is then acceptable to
* further initialize the component which will reset this bit.
*
* This driver is intended to be RTOS and processor independent. It works with
* physical addresses only.  Any needs for dynamic memory management, threads
* or thread mutual exclusion, virtual memory, or cache control must be
* satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/16/01 First release
* 1.00b jhl  02/21/02 Repartitioned driver for smaller files
* 1.00b rpm  04/26/02 Made LookupConfig public and added XWdtTb_Config
* 1.10b mta  03/23/07 Updated to new coding style
* 1.11a sdm  08/22/08 Removed support for static interrupt handlers from the MDD
*		      file
* 2.00a ktn  22/10/09 The driver is updated to use HAL processor APIs/macros.
*		      The following macros defined in xwdttb_l.h file have been
*		      removed - XWdtTb_mEnableWdt, XWdtTb_mDisbleWdt,
*		      XWdtTb_mRestartWdt, XWdtTb_mGetTimebaseReg and
*		      XWdtTb_mHasReset.
*		      Added the XWdtTb_ReadReg and XWdtTb_WriteReg
*		      macros. User should XWdtTb_ReadReg/XWdtTb_WriteReg to
*		      acheive the desired functioanality of the macros that
*		      were removed.
* 3.0   adk  19/12/13 Updated as per the New Tcl API's
* </pre>
*
******************************************************************************/

#ifndef XWDTTB_H		/* prevent circular inclusions */
#define XWDTTB_H		/* by using protection macros */

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xwdttb_l.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;	 /**< Unique ID of device */
	u32 BaseAddr;	 /**< Base address of the device */
} XWdtTb_Config;


/**
 * The XWdtTb driver instance data. The user is required to allocate a
 * variable of this type for every watchdog/timer device in the system.
 * A pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	u32 RegBaseAddress;	/* Base address of registers */
	u32 IsReady;		/* Device is initialized and ready */
	u32 IsStarted;		/* Device watchdog timer is running */
} XWdtTb;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/*
 * Required functions in xwdttb.c
 */
int XWdtTb_Initialize(XWdtTb *InstancePtr, u16 DeviceId);

void XWdtTb_Start(XWdtTb *InstancePtr);

int XWdtTb_Stop(XWdtTb *InstancePtr);

int XWdtTb_IsWdtExpired(XWdtTb *InstancePtr);

void XWdtTb_RestartWdt(XWdtTb *InstancePtr);

u32 XWdtTb_GetTbValue(XWdtTb *InstancePtr);

XWdtTb_Config *XWdtTb_LookupConfig(u16 DeviceId);

/*
 * Self-test functions in xwdttb_selftest.c
 */
int XWdtTb_SelfTest(XWdtTb *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
