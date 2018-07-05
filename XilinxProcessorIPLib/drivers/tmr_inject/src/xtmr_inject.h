/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_inject.h
* @addtogroup tmr_inject_v1_2
* @{
* @details
*
* This component contains the implementation of the XTMR_Inject component which is
* the driver for the Xilinx TMR Inject device.
*
* <b>Initialization & Configuration</b>
*
* The XTMR_Inject_Config structure is used by the driver to configure itself. This
* configuration structure is typically created by the tool-chain based on HW
* build properties.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized in one
* of the following ways:
*
*   - XTMR_Inject_Initialize(InstancePtr, DeviceId) - The driver looks up its own
*     configuration structure created by the tool-chain based on an ID provided
*     by the tool-chain.
*
*   - XTMR_Inject_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddr) - Uses a
*     configuration structure provided by the caller. If running in a system
*     with address translation, the provided virtual memory base address
*     replaces the physical address present in the configuration structure.
*
* <b>RTOS Independence</b>
*
* This driver is intended to be RTOS and processor independent.  It works
* with physical addresses only.  Any needs for dynamic memory management,
* threads or thread mutual exclusion, virtual memory, or cache control must
* be satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.0   sa   04/05/17 First release
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
* 1.1   mus  10/25/18 Added new member "LMBAddrWidth" to config structure.
*                     It contains value of C_INJECT_LMB_AWIDTH parameter.
* </pre>
*
*****************************************************************************/

#ifndef XTMR_INJECT_H /* prevent circular inclusions */
#define XTMR_INJECT_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions ****************************/
#define XTI_STANDARD_LMB_WIDTH		32U
/**************************** Type Definitions ******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID  of device */
	UINTPTR RegBaseAddr;	/**< Register base address */
	u8  MagicByte;		/**< Magic Byte parameter */
	u8  CpuId;		/**< CPU to Inject parameter */
	u8  LMBAddrWidth;		/**< LMB address width */
} XTMR_Inject_Config;

/**
 * The XTMR_Inject driver instance data. The user is required to allocate a
 * variable of this type for every TMR Inject device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	UINTPTR RegBaseAddress;		/* Base address of registers */
	u32 IsReady;			/* Device is initialized and ready */
	u8  MagicByte;			/**< Magic byte parameter */

	u32 Address;			/**< Address value (AIR) */
	u32 Instruction;		/**< Instruction value (IIR) */
	u8  Magic;			/**< Magic byte value (in CR) */
	u8  CpuId;			/**< CPU ID value (in CR) */
	u8  Inject;			/**< Inject bit value (in CR) */
} XTMR_Inject;


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/

/*
 * Initialization functions in xtmr_inject_sinit.c
 */
int XTMR_Inject_Initialize(XTMR_Inject *InstancePtr, u16 DeviceId);
XTMR_Inject_Config *XTMR_Inject_LookupConfig(u16 DeviceId);

/*
 * Required functions, in file xtmr_inject.c
 */
int XTMR_Inject_CfgInitialize(XTMR_Inject *InstancePtr,
				XTMR_Inject_Config *Config,
				UINTPTR EffectiveAddr);

void XTMR_Inject_Enable(XTMR_Inject *InstancePtr, u8 CpuId);

void XTMR_Inject_Disable(XTMR_Inject *InstancePtr);

u32 XTMR_Inject_InjectBit(XTMR_Inject *InstancePtr, u32 Value, u8 Bit);

u32 XTMR_Inject_InjectMask(XTMR_Inject *InstancePtr, u32 Value, u32 Mask);

/*
 * Functions for self-test, in file xtmr_inject_selftest.c
 */
int XTMR_Inject_SelfTest(XTMR_Inject *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif			/* end of protection macro */

/** @} */
