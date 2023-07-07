/******************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfx_asm.h
* @addtogroup dfxasm Overview
* @{
* @details
*
* The Xilinx DFX axi shutdown manager driver can be used to provide a
* boundary between the static logic and a Reconfigurable Partition during
* Partial Reconfiguration.
*
* The Xilinx DFX AXI Shutdonw manager supports below features
* Multiple Options for Status and Control:
* The DFX AXI Shutdown Manager core can be controlled and queried using
* single signals or an AXI4-Lite interface.
* Dynamic Function eXchange Controller Core Interoperability:
* The DFX AXI Shutdown Manager core connects directly to the Dynamic Function
* eXchange Controller core using the signal based control interface.
*
* <b> Initialization and Configuration </b>
*
*    - XDfxasm_LookupConfig(DeviceId) - Use the device identifier to find the
*      static configuration structure defined in xdfxasm_g.c. This is setup by
*      the tools. For some operating systems the config structure will be
*      initialized by the software and this call is not needed.
*
*    - XDfxasm_CfgInitialize() is used for initialisation. The user needs to first
*      call the XDfxasm_LookupConfig() which returns the Configuration structure
*      pointer which is passed as a parameter to the XDfxsm_CfgInitialize().
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on and it
* is recommended that users leave asserts on during development.
*
* <b> Building the driver </b>
*
* The XDfxsm driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date          Changes
* ----- -----  -----------   ---------------------------------------------
* 1.0   dp     07/14/20      First release
* 1.2   Nava   06/22/23      Added support for system device-tree flow.
* </pre>
*
******************************************************************************/

#ifndef XDFX_ASM_H_	/* prevent circular inclusions */
#define XDFX_ASM_H_	/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xdfxasm_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/* This typedef contains information about Shutdown manager Values */
typedef enum {
	XDFX_ASM_PASSTHROUGH_MODE, /* Passthrough mode */
	XDFX_ASM_SHUTDOWN_MODE, /* Shutdown mode */
} XDfxasm_State;

/* This typedef contains configuration information for a device */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID of the device */
#else
	char *Name;
#endif
	UINTPTR BaseAddress;	/**< Register Base Address */
} XDfxasm_Config;

/**
 * The XDfxasm instance data structure. A pointer to an instance data structure
 * is passed around by functions to refer to a specific instance.
 */
typedef struct {
	XDfxasm_Config Config;	/**< Device configuration */
	u32 IsReady;		/**< Device is initialized and ready */
} XDfxasm;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/* Functions in xdfxasm_sinit.c */
#ifndef SDT
XDfxasm_Config *XDfxasm_LookupConfig(u16 DeviceId);
#else
XDfxasm_Config *XDfxasm_LookupConfig(UINTPTR BaseAddress);
#endif

/* Functions in xdfxasm.c */
s32 XDfxasm_CfgInitialize(XDfxasm *InstancePtr, XDfxasm_Config *ConfigPtr,
			  UINTPTR EffectiveAddress);
void XDfxasm_SetState(XDfxasm *InstancePtr, XDfxasm_State ShutdownValue);
u32 XDfxasm_GetState(XDfxasm *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
