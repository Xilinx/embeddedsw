/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xsysmonpsv_driver.h
* @addtogroup Overview
*
* The SysMon driver supports the Xilinx System Monitor device on Versal
*
* The System Monitor device has the following features:
*		- Measure and monitor up to 160 voltages across the chip
*		- Automatic alarms based on user defined limis for the
*		  on-chip temperature.
*		- Optional interrupt request generation
*
*
* The user should refer to the hardware device specification for detailed
* information about the device.
*
*
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
* 3.1   cog    04/09/22 Remove GIC standalone related functionality for
*                       arch64 architecture
* 4.0   se     10/04/22 Update return value definitions
*		se	   11/10/22 Secure and Non-Secure mode integration
* </pre>
*
******************************************************************************/

#ifndef _XSYSMONPSV_DRIVER_H_
#define _XSYSMONPSV_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xparameters.h"
#include "xil_assert.h"
#include "xsysmonpsv_supplylist.h"

#define XSYSMONPSV_MAX_SUPPLIES 160U /**< Max Supplies */

/******************************************************************************/
/**
 * This data type defines a handler that an application defines to communicate
 * with interrupt system to retrieve state information about an application.
 *
 * @param       CallbackRef is a callback reference passed in by the upper layer
 *              when setting the handler, and is passed back to the upper layer
 *              when the handler is called. It is used to find the device driver
 *              instance.
 *
 ******************************************************************************/
typedef void (*XSysMonPsv_Handler)(void *CallbackRef);

/**
 * @brief This typedef contains configuration information for a device.
 * @{
 */
typedef struct {
	UINTPTR BaseAddress; /**< Register base address */
	u8 Supply_List[XSYSMONPSV_MAX_SUPPLIES]; /**< Maps voltage supplies in
                                                  use to the Supply registers */
} XSysMonPsv_Config;

/*@}*/

/**
 * @brief This is an interrupt callback structure where callbacks and the
 * callback reference is stored.
 * @{
 */
typedef struct {
	XSysMonPsv_Handler Handler; /**< Event handler */
	void *CallbackRef; /**< Callback reference for
                                          event handler */
	XSysMonPsv_Supply Supply; /**< Supply for which event is set */
	u32 IsCallbackSet; /**< Flag to check if a callback has
                                          been set */
} XSysMonPsv_EventHandler;

/*@}*/

/**
 * @brief The XSysmonPsv driver instance data. The user is required to allocate a
 * variable of this type for the SYSMON device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 *
 * @{
 */
typedef struct {
	XSysMonPsv_Config Config; /**< Device configuration */
#if defined (ARMR5) || defined (__aarch64__)
	XSysMonPsv_EventHandler
		SupplyEvent[XSYSMONPSV_MAX_SUPPLIES]; /**< EventList will
                                                          have callbacks for
                                                          events supported
                                                          by sysmon */
	XSysMonPsv_EventHandler TempEvent; /**< Device Temperature event
                                            handler information */
	XSysMonPsv_EventHandler OTEvent; /**< OT event handler information */
#endif
	u32 IsReady; /**< Is device ready */
#if defined (XSYSMONPSV_SECURE_MODE)
	u32 IpiIntrId; /**< Secure mode IPI Interrupt ID*/
	u32 IpiDeviceId; /**< Secure mode IPI Device ID*/
#endif
} XSysMonPsv;

#ifdef __cplusplus
}
#endif
#endif /* _XSYSMONPSV_DRIVER_H_ */
