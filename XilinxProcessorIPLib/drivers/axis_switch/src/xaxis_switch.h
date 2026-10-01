/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef XAXIS_SWITCH_H_
#define XAXIS_SWITCH_H_

/*****************************************************************************/
/**
*
* @file xaxis_switch.h
* @addtogroup axis_switch Overview
* @{
* @brief Defines the AXI4-Stream Switch Control Router driver API.
*
* @details
*
* Provides the main header for the AXI4-Stream Switch Control Router
* core. Routes streams where masters in the system do not know the
* final destination address.
*
* <b>Core Features </b>
*
* For a full description of AXI4-Stream Switch Control Router, see the
* hardware specification.
*
* <b>Software Initialization & Configuration</b>
*
* The application needs to do the following steps to prepare the
* AXI4-Stream Switch Control Router core.
*
* - Call XAxisScr_LookupConfig using a device ID to find the core
*   configuration.
* - Call XAxisScr_CfgInitialize to initialize the device and the driver
*   instance associated with it.
*
* <b>Interrupts </b>
*
* This driver does not have interrupt mechanism.
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
* is recommended to leave asserts on during development.
*
* <b> Building the driver </b>
*
* The XAXI4-Stream Switch driver is composed of several source files. This
* allows building and linking only those parts of the driver that are
* necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- --------------------------------------------------
* 1.00  sha 01/28/15 Initial release.
* 1.1   sk  08/16/16 Used UINTPTR instead of u32 for Baseaddress as part of
*                    adding 64 bit support. CR# 867425.
*                    Changed the prototype of XAxisScr_CfgInitialize API.
* 1.2   ms  02/20/17 Fixed compilation warning in _sinit.c file. This is a
*                    fix for CR-969126.
*       ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                    generation.
* 1.4   sd   02/09/20 Updated makefile for parallel execution.
* 1.8  vlt  12/12/25 Update Doxygen comments to include SDT flow details.
* 1.9  vn   09/08/26 Align Doxygen comments with AMD standards
* 1.9  vn   09/30/26 Update Doxygen comments to AMD standards
* </pre>
*
******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xaxis_switch_hw.h"
#include "xil_assert.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/**
* @brief Holds configuration information for the AXI4-Stream Switch core.
*
* Each AXI4-Stream Switch device should have a configuration structure
* associated.
*/
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID of the AXI4-Stream Switch
				 * core. */
#else
	char *Name;		/**< Name of the device. */
#endif
	UINTPTR BaseAddress;	/**< Physical base address of the core
				 * registers. */
	u8 MaxNumSI;		/**< Maximum number of slave interfaces. */
	u8 MaxNumMI;		/**< Maximum number of master interfaces. */
} XAxis_Switch_Config;

/**
* @brief Stores the AXI4-Stream Switch driver instance data.
*
* An instance must be allocated for each core in use.
*/
typedef struct {
	XAxis_Switch_Config Config;	/**< Hardware configuration. */
	u32 IsReady;			/**< Core and the driver instance are
					 * initialized. */
} XAxis_Switch;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* @brief Enables register updates.
*
* @param[in]    InstancePtr Pointer to the XAxis_Switch core instance.
*
* @note		C-style signature:
*		void XAxisScr_RegUpdateEnable(XAxis_Switch *InstancePtr)
*
******************************************************************************/
#define XAxisScr_RegUpdateEnable(InstancePtr) \
	XAxisScr_WriteReg((InstancePtr)->Config.BaseAddress, \
		XAXIS_SCR_CTRL_OFFSET, \
		XAxisScr_ReadReg((InstancePtr)->Config.BaseAddress, \
			XAXIS_SCR_CTRL_OFFSET) | \
				XAXIS_SCR_CTRL_REG_UPDATE_MASK)

/*****************************************************************************/
/**
* @brief Disables register updates.
*
* @param[in]    InstancePtr Pointer to the XAxis_Switch core instance.
*
* @note		C-style signature:
*		void XAxisScr_RegUpdateDisable(XAxis_Switch *InstancePtr)
*
******************************************************************************/
#define XAxisScr_RegUpdateDisable(InstancePtr) \
	XAxisScr_WriteReg((InstancePtr)->Config.BaseAddress, \
		XAXIS_SCR_CTRL_OFFSET, \
		XAxisScr_ReadReg((InstancePtr)->Config.BaseAddress, \
			XAXIS_SCR_CTRL_OFFSET) & \
				(~XAXIS_SCR_CTRL_REG_UPDATE_MASK))

/************************** Function Prototypes ******************************/

/* Initialization function in xaxis_switch_sinit.c */
#ifndef SDT
XAxis_Switch_Config *XAxisScr_LookupConfig(u16 DeviceId);
#else
XAxis_Switch_Config *XAxisScr_LookupConfig(UINTPTR BaseAddress);
#endif

/* Initialization and control functions in xaxis_switch.c */
s32 XAxisScr_CfgInitialize(XAxis_Switch *InstancePtr,
			XAxis_Switch_Config *CfgPtr, UINTPTR EffectiveAddr);

void XAxisScr_MiPortEnable(XAxis_Switch *InstancePtr, u8 MiIndex,
				u8 SiIndex);
void XAxisScr_MiPortDisable(XAxis_Switch *InstancePtr, u8 MiIndex);
s32 XAxisScr_IsMiPortEnabled(XAxis_Switch *InstancePtr, u8 MiIndex,
				u8 SiIndex);
s32 XAxisScr_IsMiPortDisabled(XAxis_Switch *InstancePtr, u8 MiIndex);
void XAxisScr_MiPortDisableAll(XAxis_Switch *InstancePtr);

/* Self test function in xaxis_switch_selftest.c */
s32 XAxisScr_SelfTest(XAxis_Switch *InstancePtr);

/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

/** @} */
#endif /* End of protection macro */
