/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprd.h
* @addtogroup prd_v2_0
* @{
* @details
*
* The Xilinx Partial Reconfiguration Decoupler driver can be used to provide a
* boundary between the static logic and a Reconfigurable Partition during
* Partial Reconfiguration.
*
* The PR Decoupler supports the following features:
* - All Interface types registered in the Vivado Design Suite are supported,
*   including custom interfaces.
* - Non-Vivado Design Suite interfaces are supported.
* - The decoupling behaviour can be configured for each interface.
* - Each interface can have Clock Domain Crossing support.
* - Optional AXI4-Stream based control.
* - Optional AXI4-Stream based Status.
* - Optional AXI4-Lite based status and control.
*
* <b> Initialization and Configuration </b>
*
*    - XPrd_LookupConfig(DeviceId) - Use the device identifier to find the
*      static configuration structure defined in xprd_g.c. This is setup by
*      the tools. For some operating systems the config structure will be
*      initialized by the software and this call is not needed.
*
*    - XPrd_CfgInitialize() is used for initialisation. The user needs to first
*      call the XPrd_LookupConfig() which returns the Configuration structure
*      pointer which is passed as a parameter to the XPrd_CfgInitialize().
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
* The XPrd driver is composed of several source files. This allows the user
* to build and link only those parts of the driver that are necessary.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date          Changes
* ----- -----  -----------   ---------------------------------------------
* 1.0   ms     07/14/16      First release
* 1.1   ms     01/16/17      Updated the parameter naming from
*                            XPAR_PR_DECOUPLER_NUM_INSTANCES to
*                            XPAR_XPRD_NUM_INSTANCES to avoid compilation
*                            failure for XPAR_PR_DECOUPLER_NUM_INSTANCES as the
*                            tools are generating XPAR_XPRD_NUM_INSTANCES
*                            in the generated xprd_g.c for fixing MISRA-C
*                            files. This is a fix for CR-966099 based on the
*                            update in the tools.
*       ms     03/17/17      Added readme.txt file in examples folder for
*                            doxygen generation.
*       ms     04/05/2017    Modified comment lines notation in functions
*                            of prd examples to avoid unnecessary description
*                            which was displayed while generating doxygen.
*
* </pre>
*
******************************************************************************/

#ifndef XPRD_H_	/* prevent circular inclusions */
#define XPRD_H_	/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xprd_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/* This typedef contains information about Decoupler Values */
typedef enum {
	XPRD_DECOUPLER_OFF,	/**< Decoupler Off */
	XPRD_DECOUPLER_ON,	/**< Decoupler On */
} XPrd_State;

/* This typedef contains configuration information for a device */
typedef struct {
	u16 DeviceId;		/**< Unique ID of the device */
	u32 BaseAddress;	/**< Register Base Address */
} XPrd_Config;

/**
 * The XPrd instance data structure. A pointer to an instance data structure
 * is passed around by functions to refer to a specific instance.
 */
typedef struct {
	XPrd_Config Config;	/**< Device configuration */
	u32 IsReady;		/**< Device is initialized and ready */
} XPrd;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/* Functions in xprd_sinit.c */
XPrd_Config *XPrd_LookupConfig(u16 DeviceId);

/* Functions in xprd.c */
s32 XPrd_CfgInitialize(XPrd *InstancePtr, XPrd_Config *ConfigPtr,
				u32 EffectiveAddress);
void XPrd_SetDecouplerState(XPrd *InstancePtr, XPrd_State DecouplerValue);
XPrd_State XPrd_GetDecouplerState(XPrd *InstancePtr);

/* Functions in xprd_selftest.c */
s32 XPrd_SelfTest(XPrd *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
