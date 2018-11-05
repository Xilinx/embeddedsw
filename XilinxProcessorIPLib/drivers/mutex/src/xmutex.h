/******************************************************************************
*
* Copyright (C) 2007 - 2017 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xmutex.h
* @addtogroup mutex_v4_3
* @{
* @details
*
* The Xilinx Mutex driver. This driver supports the Xilinx Mutex Core. More
* detailed description of the driver operation can be found in the xmutex.c
* file.
*
* <b>Features</b>
*
* The Xilinx Mutex supports the following features:
*   - Provide for synchronization between multiple processors in the
*     system.
*   - Write to lock scheme with CPU ID encoded.
*   - Multiple Mutex locks within a single instance of the device.
*   - An optional user field within each Mutex that can be read or
*     written to by software.
*
* This driver is intended to be RTOS and processor independent. Any needs for
* dynamic memory management, threads or thread mutual exclusion, virtual memory,
* or cache control must be satisfied by the layer above this driver.
* The effective address provided to the XMutex_CfgInitialize() function can be
* either the real, physical address or the remapped virtual address. The
* remapping of this address occurs above this driver, no remapping occurs within
* the driver itself.
*
* <b>Initialization & Configuration</b>
*
* The XMutex_Config structure is used by the driver to configure itself. This
* configuration structure is typically created by the tool-chain based on HW
* build properties.
*
* To support multiple runtime loading and initialization strategies employed by
* various operating systems, the driver instance can be initialized in the
* following way:
*
*   - XMutex_LookupConfig (DeviceId) - Use the device identifier to find the
*     static configuration structure defined in XMutex_g.c. This is setup by
*     the tools. For some operating systems the config structure will be
*     initialized by the software and this call is not needed. This function
*     returns the CfgPtr argument used by the CfgInitialize function described
*     below.
*
*   - XMutex_CfgInitialize (InstancePtr, ConfigPtr, EffectiveAddress) - Uses a
*     configuration structure provided by the caller. If running in a system
*     with address translation, the provided virtual memory base address
*     replaces the physical address present in the configuration structure.
*     The EffectiveAddress argument is required regardless of operating system
*     environment, i.e. in standalone, ConfigPtr->BaseAddress is recommended and
*     not the xparameters definition..
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a va            First release
* 1.00a ecm  06/01/07 Cleanup, new coding standard, check into XCS
* 1.00b ecm  06/01/07 fixed tcl error for number of mutex's, CR502416
* 2.00a hm   04/14/09 Fixed CR 466322, removed extra definitions
*			Also fixed canonical definitions treating an interface
*			as an device instance.
* 3.00a hbm  10/15/09 Migrated to HAL phase 1 to use xil_io, xil_types,
*			and xil_assert.
* 3.01a sdm  05/06/10 New driver to support AXI version of the core and
*		      cleaned up for coding guidelines.
* 3.02a bss  01/31/13 Updated driver tcl to fix CR #679127
* 4.0   adk  19/12/13 Updated as per the New Tcl API's
* 4.00a bss  03/05/14 Modified XMutex_CfgInitialize to fix CR# 770096
* 4.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XMutex_CfgInitialize API.
* 4.2   mi   09/22/16 Fixed compilation warnings.
*       ms   01/23/17 Modified xil_printf statement in main function for all
*                     examples to ensure that "Successfully ran" and "Failed"
*                     strings are available in all examples. This is a fix
*                     for CR-965028.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
* 4.3   ms   04/18/17 Modified tcl file to add suffix U for all macros
*                     definitions of mutex in xparameters.h
*       ms   08/07/17 Fixed compilation warnings in xmutex_sinit.c
* </pre>
*
******************************************************************************/

#ifndef XMUTEX_H		/* prevent circular inclusions */
#define XMUTEX_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xmutex_hw.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;	/**< Unique ID of device */
	UINTPTR BaseAddress;/**< Register base address */
	u32 NumMutex;	/**< Number of Mutexes in this device */
	 u8 UserReg;	/**< User Register, access not controlled by Mutex */
} XMutex_Config;


/**
 * The XMutex driver instance data. The user is required to allocate a
 * variable of this type for every Mutex device in the system. A
 * pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	XMutex_Config Config; /**< Configuration data, includes base address */
	u32 IsReady;	      /**< Device is initialized and ready */
} XMutex;


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/*
 * Required functions, in file xmutex.c
 */
int XMutex_CfgInitialize(XMutex *InstancePtr, XMutex_Config *ConfigPtr,
			 UINTPTR EffectiveAddress);
void XMutex_Lock(XMutex *InstancePtr, u8 MutexNumber);
int XMutex_Trylock(XMutex *InstancePtr, u8 MutexNumber);
int XMutex_Unlock(XMutex *InstancePtr, u8 MutexNumber);
int XMutex_IsLocked(XMutex *InstancePtr, u8 MutexNumber);
void XMutex_GetStatus(XMutex *InstancePtr, u8 MutexNumber, u32 *Locked,
			u32 *Owner);
int XMutex_GetUser(XMutex *InstancePtr, u8 MutexNumber, u32 *User);
int XMutex_SetUser(XMutex *InstancePtr, u8 MutexNumber, u32 User);

/*
 * Static Initialization function, in file xmutex_sinit.c
 */
XMutex_Config *XMutex_LookupConfig(u16 DeviceId);

/*
 * Functions for self-test, in file xmutex_selftest.c
 */
int XMutex_SelfTest(XMutex *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
