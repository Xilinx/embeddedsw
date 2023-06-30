/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xwdtps.h
* @addtogroup wdtps Overview
* @{
* @details
*
* The Xilinx watchdog timer driver supports the Xilinx watchdog timer hardware.
*
* The Xilinx watchdog timer (WDT) driver supports the following features:
*   - Both Interrupt driven and Polled mode
*   - enabling and disabling the watchdog timer
*   - restarting the watchdog.
*   - initializing the most significant digit of the counter restart value.
*   - multiple individually enabling/disabling outputs
*
* It is the responsibility of the application to provide an interrupt handler
* for the watchdog timer and connect it to the interrupt system if interrupt
* driven mode is desired.
*
* If interrupt is enabled, the watchdog timer device generates an interrupt
* when the counter reaches zero.
*
* If the hardware interrupt signal is not connected/enabled, polled mode is the
* only option (using IsWdtExpired) for the watchdog.
*
* The outputs from the WDT are individually enabled/disabled using
* _EnableOutput()/_DisableOutput(). The clock divisor ratio and initial restart
* value of the count is configurable using _SetControlValues().
*
* The reset condition of the hardware has the maximum initial count in the
* Counter Reset Value (CRV) and the WDT is disabled with the reset enable
* enabled and the reset length set to 32 clocks. i.e.
* <pre>
*     register ZMR = 0x1C2
*     register CCR = 0x3FC
* </pre>
*
* This driver is intended to be RTOS and processor independent. It works with
* physical addresses only.  Any needs for dynamic memory management, threads
* or thread mutual exclusion, virtual memory, or cache control must be
* satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00a ecm/jz 01/15/10 First release
* 1.01a asa    02/15/12 Added tcl file to generate xparameters
* 1.02a  sg    07/15/12 Removed code/APIs related to  External Signal
*						Length functionality for CR 658287
*						Removed APIs XWdtPs_SetExternalSignalLength,
*						XWdtPs_GetExternalSignalLength
*						Modified the Self Test to use the Reset Length mask
*						for CR 658287
* 3.0	pkp	   12/09/14 Added support for Zynq Ultrascale Mp.Also
*			modified code for MISRA-C:2012 compliance.
*       ms     03/17/17 Added readme.txt file in examples folder for doxygen
*                       generation.
* 3.1   sg     08/17/18 Updated interrupt example to fix interrupt ID conflict
*			issue
* 3.2	sne    08/05/19 Fixed coverity warnings.
* 3.4	sne    08/28/20 Modify Makefile to support parallel make execution.
* 3.6	sb     06/27/23 Added support for system device-tree flow.
*
* </pre>
*
******************************************************************************/
#ifndef XWDTPS_H		/* prevent circular inclusions */
#define XWDTPS_H		/**< by using protection macros */

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xwdtps_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/

/*
 * Choices for output selections for the device, used in
 * XWdtPs_EnableOutput()/XWdtPs_DisableOutput() functions
 */
#define XWDTPS_RESET_SIGNAL	0x01U   /**< Reset signal request */
#define XWDTPS_IRQ_SIGNAL	0x02U   /**< IRQ signal request */

/*
 * Control value setting flags, used in
 * XWdtPs_SetControlValues()/XWdtPs_GetControlValues() functions
 */
#define XWDTPS_CLK_PRESCALE		0x01U   /**< Clock Prescale request */
#define XWDTPS_COUNTER_RESET	0x02U   /**< Counter Reset request */

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID of device */
#else
	char *Name;
#endif
	UINTPTR BaseAddress;	/**< Base address of the device */
#ifdef SDT
	u32 InputClockHz;	/**< Input clock frequency */
	u16 IntrId; /**< Bits[11:0] Interrupt-id Bits[15:12] trigger type and level flags */
	UINTPTR IntrParent; /**< Bit[0] Interrupt parent type Bit[64/32:1] Parent base address */
#endif
} XWdtPs_Config;


/**
 * The XWdtPs driver instance data. The user is required to allocate a
 * variable of this type for every watchdog/timer device in the system.
 * A pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	XWdtPs_Config Config;   /**< Hardware Configuration */
	u32 IsReady;		/**< Device is initialized and ready */
	u32 IsStarted;		/**< Device watchdog timer is running */
} XWdtPs;

/************************** Variable Definitions *****************************/
extern XWdtPs_Config XWdtPs_ConfigTable[];      /**< Configuration table */

/***************** Macros (Inline Functions) Definitions *********************/
/****************************************************************************/
/**
*
* Check if the watchdog timer has expired. This function is used for polled
* mode and it is also used to check if the last reset was caused by the
* watchdog timer.
*
* @param	InstancePtr is a pointer to the XWdtPs instance.
*
* @return
*		- TRUE if the watchdog has expired.
*		- FALSE if the watchdog has not expired.
*
* @note		C-style signature:
*		int XWdtPs_IsWdtExpired(XWdtPs *InstancePtr)
*
******************************************************************************/
#define XWdtPs_IsWdtExpired(InstancePtr)				  \
	((XWdtPs_ReadReg((InstancePtr)->Config.BaseAddress, XWDTPS_SR_OFFSET) & \
	  XWDTPS_SR_WDZ_MASK) == XWDTPS_SR_WDZ_MASK)


/****************************************************************************/
/**
*
* Restart the watchdog timer. An application needs to call this function
* periodically to keep the timer from asserting the enabled output.
*
* @param	InstancePtr is a pointer to the XWdtPs instance.
*
* @return	None.
*
* @note		C-style signature:
*		void XWdtPs_RestartWdt(XWdtPs *InstancePtr)
*
******************************************************************************/
#define XWdtPs_RestartWdt(InstancePtr)					\
	XWdtPs_WriteReg((InstancePtr)->Config.BaseAddress,		\
			XWDTPS_RESTART_OFFSET, XWDTPS_RESTART_KEY_VAL)

/************************** Function Prototypes ******************************/

/*
 * Lookup configuration in xwdtps_sinit.c.
 */
#ifndef SDT
XWdtPs_Config *XWdtPs_LookupConfig(u16 DeviceId);
#else
XWdtPs_Config *XWdtPs_LookupConfig(u32 BaseAddress);
#endif

/*
 * Interface functions in xwdtps.c
 */
s32 XWdtPs_CfgInitialize(XWdtPs *InstancePtr,
			 XWdtPs_Config *ConfigPtr, UINTPTR EffectiveAddress);

void XWdtPs_Start(XWdtPs *InstancePtr);

void XWdtPs_Stop(XWdtPs *InstancePtr);

void XWdtPs_EnableOutput(XWdtPs *InstancePtr, u8 Signal);

void XWdtPs_DisableOutput(XWdtPs *InstancePtr, u8 Signal);

u32 XWdtPs_GetControlValue(XWdtPs *InstancePtr, u8 Control);

void XWdtPs_SetControlValue(XWdtPs *InstancePtr, u8 Control, u32 Value);

/*
 * Self-test function in xwdttb_selftest.c.
 */
s32 XWdtPs_SelfTest(XWdtPs *InstancePtr);


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
