/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef _XSYSMONPSV_SERVICES_H_
#define _XSYSMONPSV_SERVICES_H_

/*****************************************************************************/
/**
* @file xsysmonpsv_services.h
*
* @brief Declares the services-layer APIs for the XSysMonPsv driver.
*
* The xsysmonpsv_services.h file declares services-layer APIs such as
* temperature and voltage alarms.
*
* <pre>
*
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
* 3.1   cog    04/09/22 Remove GIC standalone related functionality for
*                       arch64 architecture
* 4.0   se     11/10/22 Secure and Non-Secure mode integration
* 5.2   se     08/24/25 Microblaze support added
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
#include "xil_assert.h"
#if defined (ARMR5) || defined (__aarch64__)
#include "xscugic.h"
#endif
#if defined (PLATFORM_MB) && defined (XIL_INTERRUPT) && defined (SDT)
#include "xinterrupt_wrap.h"
#endif

int XSysMonPsv_EnableVoltageEvents(XSysMonPsv *InstancePtr, u32 Supply,
				   u32 IntrNum);
int XSysMonPsv_DisableVoltageEvents(XSysMonPsv *InstancePtr, u32 Supply);

#if defined (ARMR5) || defined (__aarch64__) || defined (PLATFORM_MB)
void XSysMonPsv_RegisterDevTempCallback(XSysMonPsv *InstancePtr,
					XSysMonPsv_Handler CallbackFunc,
					void *CallbackRef);
void XSysMonPsv_UnregisterDevTempCallback(XSysMonPsv *InstancePtr);
void XSysMonPsv_RegisterOTCallback(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Handler CallbackFunc,
				   void *CallbackRef);
void XSysMonPsv_UnregisterOTCallback(XSysMonPsv *InstancePtr);
void XSysMonPsv_RegisterSupplyCallback(XSysMonPsv *InstancePtr,
				       XSysMonPsv_Supply Supply,
				       XSysMonPsv_Handler CallbackFunc,
				       void *CallbackRef);
void XSysMonPsv_UnregisterSupplyCallback(XSysMonPsv *InstancePtr, u32 Supply);
void XSysMonPsv_IntrHandler(XSysMonPsv *InstancePtr);
#endif

#if defined (ARMR5) || defined (__aarch64__)
int XSysMonPsv_SetupInterrupts(XScuGic *IntcInstancePtr,
			       XSysMonPsv *InstancePtr, u16 IntrId);
#endif

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* _XSYSMONPSV_SERVICES_H_ */
