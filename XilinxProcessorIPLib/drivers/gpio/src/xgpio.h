/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xgpio.h
* @addtogroup gpio_api GPIO APIs
* @{
* @details
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  03/13/02 First release
* 2.00a jhl  11/26/03 Added support for dual channels and interrupts
* 2.01a jvb  12/14/05 I separated dependency on the static config table and
*                     xparameters.h from the driver initialization by moving
*                     _Initialize and _LookupConfig to _sinit.c. I also added
*                     the new _CfgInitialize routine.
* 2.11a mta  03/21/07 Updated to new coding style, added GetDataDirection
* 2.12a sv   11/21/07 Updated driver to support access through DCR bus
* 2.12a sv   06/05/08 Updated driver to fix the XGpio_InterruptDisable function
*		      to properly update the Interrupt Enable register
* 2.13a sdm  08/22/08 Removed support for static interrupt handlers from the MDD
*		      file
* 3.00a sv   11/21/09 Updated to use HAL Processor APIs.
*		      Renamed the macros XGpio_mWriteReg to XGpio_WriteReg and
*		      XGpio_mReadReg to XGpio_ReadReg. Removed the macros
*		      XGpio_mSetDataDirection, XGpio_mGetDataReg and
*		      XGpio_mSetDataReg. Users should use XGpio_WriteReg and
*		      XGpio_ReadReg to achieve the same functionality.
* 3.01a bss  04/18/13 Updated driver tcl to generate Canonical params in
*		      xparameters.h. CR#698589
* 4.0   adk  19/12/13 Updated as per the New Tcl API's
* 4.1   lks  11/18/15 Updated to use canonical xparameters in examples and
*		      clean up of the comments, removed support for DCR bridge
*		      and removed xgpio_intr_example for CR 900381
* 4.2   sk   08/16/16 Used UINTPTR instead of u32 for Baseaddress as part of
*                     adding 64 bit support. CR# 867425.
*                     Changed the prototype of XGpio_CfgInitialize API.
* 4.3   sk   09/29/16 Modified the example to make it work when LED_bits are
*                     configured as an output. CR# 958644
*       ms   01/23/17 Added xil_printf statement in main function for all
*                     examples to ensure that "Successfully ran" and "Failed"
*                     strings are available in all examples. This is a fix
*                     for CR-965028.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
* 4.4   sne  04/25/19 Updated Makefile for IAR compier (CR-1029421).
* 4.5   sne  06/14/19 Fixed IAR compiler warnings on example files.
* 4.6	sne  08/11/19 Fixed compilation error of armcc compiler.
* 4.7   sne  08/28/20 Modify Makefile to support parallel make execution.
* 4.8	sne  02/10/21 Fixed doxygen warnings.
* 4.10  gm   07/11/23 Added SDT support.
* 4.10  gm   08/28/23 Added Width member to XGpio_Config in SDT flow.
*
* </pre>
*****************************************************************************/

#ifndef XGPIO_H			/* prevent circular inclusions */
#define XGPIO_H			/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xgpio_l.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID  of device */
#else
	char *Name;
#endif
	UINTPTR BaseAddress;	/**< Device base address */
	int InterruptPresent;	/**< Are interrupts supported in h/w */
	int IsDual;		/**< Are 2 channels supported in h/w */
#ifdef SDT
	u16 IntrId; /** Bits[11:0] Interrupt-id Bits[15:12] trigger type and level flags */
	UINTPTR IntrParent; /** Bit[0] Interrupt parent type Bit[64/32:1] Parent base address */
	u16 Width;  /** Gpio width */
#endif
} XGpio_Config;

/**
 * The XGpio driver instance data. The user is required to allocate a
 * variable of this type for every GPIO device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	UINTPTR BaseAddress;	/**< Device base address */
	u32 IsReady;		/**< Device is initialized and ready */
	int InterruptPresent;	/**< Are interrupts supported in h/w */
	int IsDual;		/**< Are 2 channels supported in h/w */
} XGpio;

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/

/*
 * Initialization functions in xgpio_sinit.c
 */
#ifndef SDT
int XGpio_Initialize(XGpio *InstancePtr, u16 DeviceId);
XGpio_Config *XGpio_LookupConfig(u16 DeviceId);
#else
int XGpio_Initialize(XGpio *InstancePtr, UINTPTR BaseAddress);
XGpio_Config *XGpio_LookupConfig(UINTPTR BaseAddress);
#endif

/*
 * API Basic functions implemented in xgpio.c
 */
int XGpio_CfgInitialize(XGpio *InstancePtr, XGpio_Config * Config,
			UINTPTR EffectiveAddr);
void XGpio_SetDataDirection(XGpio *InstancePtr, unsigned Channel,
			    u32 DirectionMask);
u32 XGpio_GetDataDirection(XGpio *InstancePtr, unsigned Channel);
u32 XGpio_DiscreteRead(XGpio *InstancePtr, unsigned Channel);
void XGpio_DiscreteWrite(XGpio *InstancePtr, unsigned Channel, u32 Mask);


/*
 * API Functions implemented in xgpio_extra.c
 */
void XGpio_DiscreteSet(XGpio *InstancePtr, unsigned Channel, u32 Mask);
void XGpio_DiscreteClear(XGpio *InstancePtr, unsigned Channel, u32 Mask);

/*
 * API Functions implemented in xgpio_selftest.c
 */
int XGpio_SelfTest(XGpio *InstancePtr);

/*
 * API Functions implemented in xgpio_intr.c
 */
void XGpio_InterruptGlobalEnable(XGpio *InstancePtr);
void XGpio_InterruptGlobalDisable(XGpio *InstancePtr);
void XGpio_InterruptEnable(XGpio *InstancePtr, u32 Mask);
void XGpio_InterruptDisable(XGpio *InstancePtr, u32 Mask);
void XGpio_InterruptClear(XGpio *InstancePtr, u32 Mask);
u32 XGpio_InterruptGetEnabled(XGpio *InstancePtr);
u32 XGpio_InterruptGetStatus(XGpio *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
