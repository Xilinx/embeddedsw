/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdmi_menu.h
*
* This is the main header file for the Xilinx Menu implementation as used
* in the HDMI example design.
*
* <b>Software Initialization & Configuration</b>
*
*
* <b>Interrupts </b>
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- --------------------------------------------------
* X.X   ..   DD-MM-YYYY ..
* 1.00   GM     05/03/18 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XHDMI_INTR_H_
#define XHDMI_INTR_H_  /**< Prevent circular inclusions
*  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

	/***************************** Include Files *********************************/
#include "platform.h"
#include "xparameters.h"
#if defined (ARMR5) || (__aarch64__) || (__arm__)
#include "xscugic.h"
#else
#include "xintc.h"
#endif

/************************** Variable Definitions *****************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef struct {
	XInterruptHandler Handler;
	void *CallBackRef;
} XHdmi_Interrupt_VectorTableEntry;

struct XHdmi_InterruptFlag_t {
	void *CallBackRef;
	struct XHdmi_InterruptFlag_t *Next;
};

/************************** Function Prototypes ******************************/
void XHdmi_InterruptInitialize(void);
u32 XHdmi_InterruptConnect(void *Intc, u8 Id, XInterruptHandler Handler,
		void *CallBackRef);
void XHdmi_InterruptHandler(void *CallBackRef);
void XHdmi_InterruptService(void);
u32 XHdmi_InterruptDevIdLookUp(void *CallBackRef);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
