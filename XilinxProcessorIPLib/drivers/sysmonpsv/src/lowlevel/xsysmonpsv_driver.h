/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef _XSYSMONPSV_DRIVER_H_
#define _XSYSMONPSV_DRIVER_H_

/*****************************************************************************/
/**
* @file xsysmonpsv_driver.h
*
* @brief Declares the driver data structures for the AMD Versal System Monitor.
*
* The SysMon driver supports the AMD System Monitor device on Versal.
*
* The System Monitor device has the following features:
*		- Measures and monitors up to 160 voltages across the chip.
*		- Provides automatic alarms based on user-defined limits for
*		  the on-chip temperature.
*		- Provides optional interrupt request generation.
*
* Refer to the hardware device specification for detailed information about
* the device.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
* 3.1   cog    04/09/22 Remove GIC standalone related functionality for
*                       arch64 architecture
* 4.0   se     10/04/22 Update return value definitions
* 4.0   se     11/10/22 Secure and Non-Secure mode integration
* 4.1   cog    07/18/23 Add support for SDT flow
* 5.1   se     03/03/25 Compiler warnings fixed
* 5.2   se     08/24/25 Microblaze support added
* 5.3   se     03/13/26 Fix secure mode and PCSR re-lock in SDT flow
* 5.4   vn     07/30/26 Fix Doxygen warnings
* 5.4   vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup sysmonpsv_api SYSMONPSV APIs
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#ifndef SDT
#include "xparameters.h"
#endif
#include "xil_assert.h"
#include "xsysmonpsv_supplylist.h"

#define XSYSMONPSV_MAX_SUPPLIES 160U /**< Max Supplies */

/******************************************************************************/
/**
 * @brief Defines an application handler for the interrupt system.
 *
 * An application defines this handler to communicate with the interrupt
 * system and retrieve application state information.
 *
 * @param[in]   CallbackRef Callback reference passed in by the upper layer
 *              when setting the handler, and passed back to the upper layer
 *              when the handler is called. It is used to find the device
 *              driver instance.
 *
 ******************************************************************************/
typedef void (*XSysMonPsv_Handler)(void *CallbackRef);

/**
 * @brief Holds the configuration information for a device.
 */
typedef struct {
#ifdef SDT
	char *Name; /**< Device name. */
#endif
	UINTPTR BaseAddress; /**< Register base address. */
#ifdef SDT
	u16 IntrId; /**< Interrupt ID. */
	UINTPTR IntrParent; /**< Interrupt parent. */
#endif
	u8 Supply_List[XSYSMONPSV_MAX_SUPPLIES]; /**< Maps voltage supplies in
                                              *   use to the supply
                                              *   registers. */
} XSysMonPsv_Config;

/**
 * @brief Stores the interrupt callbacks and their callback references.
 */
typedef struct {
	XSysMonPsv_Handler Handler; /**< Event handler. */
	void *CallbackRef; /**< Callback reference for the event handler. */
	XSysMonPsv_Supply Supply; /**< Supply for which the event is set. */
	u32 IsCallbackSet; /**< Flag to check if a callback has been set. */
} XSysMonPsv_EventHandler;

/**
 * @brief Holds the XSysMonPsv driver instance data.
 *
 * Allocate a variable of this type for the SYSMON device in the system. A
 * pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	XSysMonPsv_Config Config; /**< Device configuration. */
#if defined (ARMR5) || defined (__aarch64__) || defined (PLATFORM_MB)
	XSysMonPsv_EventHandler
		SupplyEvent[XSYSMONPSV_MAX_SUPPLIES]; /**< Holds callbacks for
                                               *   events supported by
                                               *   SysMon. */
	XSysMonPsv_EventHandler TempEvent; /**< Device temperature event
                                        *   handler information. */
	XSysMonPsv_EventHandler OTEvent; /**< OT event handler information. */
#endif
	u32 IsReady; /**< Is the device ready. */
#if defined (XSYSMONPSV_SECURE_MODE)
	u32 IpiIntrId; /**< Secure mode IPI Interrupt ID*/
#ifndef SDT
	u32 IpiDeviceId; /**< Secure mode IPI Device ID*/
#else
	UINTPTR IpiBaseAddress; /**< Secure mode IPI Base Address (SDT flow) */
#endif
#endif
} XSysMonPsv;

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* _XSYSMONPSV_DRIVER_H_ */
