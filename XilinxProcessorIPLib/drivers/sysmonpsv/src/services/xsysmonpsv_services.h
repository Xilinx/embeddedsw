/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xsysmonpsv_services.h
* @addtogroup Overview
*
* Services layers provide APIs to which a user can subscribe to like Temperature,
* voltage alarms.
*
* <pre>
*
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
* 3.1   cog    04/09/22 Remove GIC standalone related functionality for
*                       arch64 architecture
*
* </pre>
*
******************************************************************************/

#ifndef _XSYSMONPSV_SERVICES_H_
#define _XSYSMONPSV_SERVICES_H_
#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xsysmonpsv_lowlevel.h"
#if defined (ARMR5) || defined (__aarch64__)
#include "xscugic.h"
#endif

int XSysMonPsv_EnableVoltageEvents(XSysMonPsv *InstancePtr, u32 Supply,
				   u32 IntrNum);
int XSysMonPsv_DisableVoltageEvents(XSysMonPsv *InstancePtr, u32 Supply);

#if defined (ARMR5) || defined (__aarch64__)
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
int XSysMonPsv_SetupInterrupts(XScuGic *IntcInstancePtr,
			       XSysMonPsv *InstancePtr, u16 IntrId);
#endif

#ifdef __cplusplus
}
#endif
#endif /* _XSYSMONPSV_SERVICES_H_ */
