/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_services.c
*
* @brief Implements the services-layer functions for the XSysMonPsv driver.
*
* The xsysmonpsv_services.c file implements functions that provide services
* such as temperature and voltage event notification on a subscription basis.
*
* <pre>
*
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
* 3.1   cog    04/09/22 Remove GIC standalone related functionality for
*                       arch64 architecture
* 4.0   se     10/04/22 Update return value definitions
* 4.0   se     11/10/22 Secure and Non-Secure mode integration
* 5.2   se     08/24/25 Microblaze support added
* 5.3   dc     02/18/26 Correct spelling errors
*       se     03/12/26 Enhancements: NULL checks, range validation,
*                       standardize return values to XST_FAILURE
*       se     03/25/26 Fix Coverity/MISRA-C violations: U suffix,
*                       (void) cast, sign conversion, parentheses
* 5.4   vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup sysmonpsv_api SYSMONPSV APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xsysmonpsv_lowlevel.h"
#include "xsysmonpsv_common.h"
#include "xsysmonpsv_services.h"
#include "xsysmonpsv_hw.h"
#include "xstatus.h"

/******************************************************************************/
/**
 * @brief Enables the voltage event for the supply.
 *
 * @param[in]   InstancePtr Pointer to the driver instance.
 * @param[in]   Supply Enum from the XSysMonPsv_Supply.
 * @param[in]   IntrNum Interrupt offset.
 *
 * @return
 *              - XST_FAILURE if error.
 *              - XST_SUCCESS if successful.
 *
*******************************************************************************/
int XSysMonPsv_EnableVoltageEvents(XSysMonPsv *InstancePtr, u32 Supply,
				   u32 IntrNum)
{
	u32 AlarmRegOffset;
	u32 Event, Val, Bit;
	u32 SupplyReg, Ier;

	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}
	if ((Supply >= XSYSMONPSV_MAX_SUPPLIES) ||
	    (InstancePtr->Config.Supply_List[Supply] ==
	     XSYSMONPSV_INVALID_SUPPLY)) {
		return XST_FAILURE;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Bit = ALARM_SHIFT(SupplyReg);
	Event = ALARM_REG(SupplyReg);
	AlarmRegOffset = XSYSMONPSV_ALARM_REG0 + (Event * 4U);

	XSysMonPsv_ReadReg32(InstancePtr, AlarmRegOffset, &Val);
	Val = Val | (1U << Bit);
	Ier = GET_BIT(Event);
	(void)XSysMonPsv_InterruptEnable(InstancePtr, Ier, IntrNum);
	XSysMonPsv_WriteReg32(InstancePtr, AlarmRegOffset, Val);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * @brief Disables the voltage event for the supply.
 *
 * @param[in]   InstancePtr Pointer to the driver instance.
 * @param[in]   Supply Enum from the XSysMonPsv_Supply.
 *
 * @return
 *              - XST_FAILURE if error.
 *              - XST_SUCCESS if successful.
 *
*******************************************************************************/
int XSysMonPsv_DisableVoltageEvents(XSysMonPsv *InstancePtr, u32 Supply)
{
	u32 AlarmRegOffset;
	u32 Event, Val, Bit;
	u32 SupplyReg;

	if (InstancePtr == NULL) {
		return XST_FAILURE;
	}
	if ((Supply >= XSYSMONPSV_MAX_SUPPLIES) ||
	    (InstancePtr->Config.Supply_List[Supply] ==
	     XSYSMONPSV_INVALID_SUPPLY)) {
		return XST_FAILURE;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Bit = ALARM_SHIFT(SupplyReg);
	Event = ALARM_REG(SupplyReg);
	AlarmRegOffset = XSYSMONPSV_ALARM_REG0 + (Event * 4U);

	XSysMonPsv_ReadReg32(InstancePtr, AlarmRegOffset, &Val);
	Val = Val & (~(1U << Bit));

	XSysMonPsv_WriteReg32(InstancePtr, AlarmRegOffset, Val);

	return XST_SUCCESS;
}

#if defined (ARMR5) || defined (__aarch64__) || defined (PLATFORM_MB)
/******************************************************************************/
/**
 * @brief Installs a callback function for a device temperature interrupt.
 *
 * @param[in,out] InstancePtr Pointer to the XSysMonPsv instance.
 * @param[in]     CallbackFunc Address of the callback function.
 * @param[in]     CallbackRef User data item passed to the callback function
 *                when it is invoked.
 *
*******************************************************************************/
void XSysMonPsv_RegisterDevTempCallback(XSysMonPsv *InstancePtr,
					XSysMonPsv_Handler CallbackFunc,
					void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->TempEvent.Handler = CallbackFunc;
	InstancePtr->TempEvent.CallbackRef = CallbackRef;
	InstancePtr->TempEvent.IsCallbackSet = 1U;
}

/******************************************************************************/
/**
 * @brief Removes the callback function registered for a device temperature.
 *
 * @param[in,out] InstancePtr Pointer to the XSysMonPsv instance.
 *
*******************************************************************************/
void XSysMonPsv_UnregisterDevTempCallback(XSysMonPsv *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->TempEvent.Handler = NULL;
	InstancePtr->TempEvent.CallbackRef = NULL;
	InstancePtr->TempEvent.IsCallbackSet = 0U;
}

/******************************************************************************/
/**
 * @brief Installs a callback function for an OT temperature interrupt.
 *
 * @param[in,out] InstancePtr Pointer to the XSysMonPsv instance.
 * @param[in]     CallbackFunc Address of the callback function.
 * @param[in]     CallbackRef User data item passed to the callback function
 *                when it is invoked.
 *
*******************************************************************************/
void XSysMonPsv_RegisterOTCallback(XSysMonPsv *InstancePtr,
				   XSysMonPsv_Handler CallbackFunc,
				   void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->OTEvent.Handler = CallbackFunc;
	InstancePtr->OTEvent.CallbackRef = CallbackRef;
	InstancePtr->OTEvent.IsCallbackSet = 1U;
}

/******************************************************************************/
/**
 * @brief Removes the callback function registered for OT temperature.
 *
 * @param[in,out] InstancePtr Pointer to the XSysMonPsv instance.
 *
*******************************************************************************/
void XSysMonPsv_UnregisterOTCallback(XSysMonPsv *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->OTEvent.Handler = NULL;
	InstancePtr->OTEvent.CallbackRef = NULL;
	InstancePtr->OTEvent.IsCallbackSet = 0U;
}

/******************************************************************************/
/**
 * @brief Installs a callback function for a supply voltage alarm interrupt.
 *
 * @param[in,out] InstancePtr Pointer to the XSysMonPsv instance.
 * @param[in]     Supply Supply for which the alarm is set.
 * @param[in]     CallbackFunc Address of the callback function.
 * @param[in]     CallbackRef User data item passed to the callback function
 *                when it is invoked.
 *
*******************************************************************************/
void XSysMonPsv_RegisterSupplyCallback(XSysMonPsv *InstancePtr,
				       XSysMonPsv_Supply Supply,
				       XSysMonPsv_Handler CallbackFunc,
				       void *CallbackRef)
{
	u32 SupplyReg;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	InstancePtr->SupplyEvent[SupplyReg].Handler = CallbackFunc;
	InstancePtr->SupplyEvent[SupplyReg].CallbackRef = CallbackRef;
	InstancePtr->SupplyEvent[SupplyReg].Supply = Supply;
	InstancePtr->SupplyEvent[SupplyReg].IsCallbackSet = 1U;
}

/******************************************************************************/
/**
 * @brief Removes the callback function registered for a supply voltage alarm.
 *
 * @param[in,out] InstancePtr Pointer to the XSysMonPsv instance.
 * @param[in]     Supply Supply for which the alarm is set.
 *
*******************************************************************************/
void XSysMonPsv_UnregisterSupplyCallback(XSysMonPsv *InstancePtr, u32 Supply)
{
	u32 SupplyReg;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	InstancePtr->SupplyEvent[SupplyReg].Handler = NULL;
	InstancePtr->SupplyEvent[SupplyReg].CallbackRef = NULL;
	InstancePtr->SupplyEvent[SupplyReg].Supply = EndList;
	InstancePtr->SupplyEvent[SupplyReg].IsCallbackSet = 0U;
}

/******************************************************************************/
/**
 * @brief Handles interrupts for the XSysMonPsv driver.
 *
 * On an interrupt, this function first detects the kind of interrupt, then
 * invokes the corresponding callback function.
 *
 * @param[in]   InstancePtr Pointer to the XSysMonPsv instance.
 *
*******************************************************************************/
void XSysMonPsv_IntrHandler(XSysMonPsv *InstancePtr)
{
	u32 DevTempDetected, OTTempDetected, SupplyAlarm;

	/* Upper 16 bits contain Min Temp, Lower 16 bits contain Max Temp */
	u32 IntrStatus, SupplyReg, IntrMask;
	u32 SupplyNum = 0U, ListLength = (u32)EndList;
	XSysMonPsv_EventHandler *EventHandler;
	XSysMonPsv_Supply Supply = (XSysMonPsv_Supply)0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Determine what kind of interrupt occurred */
	(void)XSysMonPsv_InterruptGetStatus(InstancePtr, &IntrStatus);

	/* Clear interrupt status register */
	XSysMonPsv_InterruptClear(InstancePtr, IntrStatus);
	XSysMonPsv_ReadReg32(InstancePtr, XSYSMONPSV_IMR0_OFFSET, &IntrMask);

	IntrStatus &= ~IntrMask;
	SupplyAlarm = IntrStatus &
		      (XSYSMONPSV_ISR_ALARM0_MASK | XSYSMONPSV_ISR_ALARM1_MASK |
		       XSYSMONPSV_ISR_ALARM2_MASK | XSYSMONPSV_ISR_ALARM3_MASK |
		       XSYSMONPSV_ISR_ALARM4_MASK);

	DevTempDetected = IntrStatus & XSYSMONPSV_ISR_TEMP_MASK;
	OTTempDetected = IntrStatus & XSYSMONPSV_ISR_OT_MASK;

	/* Handle OT Event */
	if ((OTTempDetected != 0U) &&
	    (InstancePtr->OTEvent.IsCallbackSet == 1U)) {
		XSysMonPsv_WriteReg32(InstancePtr, XSYSMONPSV_IDR0_OFFSET,
				    GET_BIT(XSYSMONPSV_BIT_OT));
		InstancePtr->OTEvent.Handler(InstancePtr->OTEvent.CallbackRef);
	}

	/* Handle Dev Temp Event */
	if ((DevTempDetected != 0U) &&
	    (InstancePtr->TempEvent.IsCallbackSet == 1U)) {
		XSysMonPsv_WriteReg32(InstancePtr, XSYSMONPSV_IDR0_OFFSET,
				    GET_BIT(XSYSMONPSV_BIT_TEMP));
		InstancePtr->TempEvent.Handler(
			InstancePtr->TempEvent.CallbackRef);
	}

	if (SupplyAlarm != 0U) {
		for (SupplyNum = 0U; SupplyNum < ListLength; SupplyNum++) {
			Supply = (XSysMonPsv_Supply)SupplyNum;
			if (XSysMonPsv_IsAlarmPresent(InstancePtr, Supply) ==
			    1U) {
				SupplyReg =
					InstancePtr->Config.Supply_List[Supply];
				EventHandler =
					&InstancePtr->SupplyEvent[SupplyReg];
				(void)XSysMonPsv_DisableVoltageEvents(InstancePtr,
								Supply);
				(void)XSysMonPsv_ClearAlarm(InstancePtr, Supply);
				if (EventHandler->IsCallbackSet == 1U) {
					EventHandler->Handler(
						EventHandler->CallbackRef);
				}
			}
		}
	}
	XSysMonPsv_InterruptClear(InstancePtr, IntrStatus);
}
#endif
#if defined (ARMR5) || defined (__aarch64__)
/****************************************************************************/
/**
*
* @brief Registers the ISR for the driver instance.
*
* @param[in,out] IntcInstancePtr Pointer to the interrupt instance.
* @param[in]     InstancePtr Pointer to the driver instance.
* @param[in]     IntrId Interrupt unique ID.
*
* @return
*                - XST_FAILURE if error.
*                - XST_SUCCESS if successful.
*
***************************************************************************/
int XSysMonPsv_SetupInterrupts(XScuGic *IntcInstancePtr,
			       XSysMonPsv *InstancePtr, u16 IntrId)
{
	int Status;
	XScuGic_Config *IntcConfig;

	/* Initialize the interrupt controller driver */
	IntcConfig = XScuGic_LookupConfig(0);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the interrupt controller interrupt handler to the
	 * hardware interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(
		XIL_EXCEPTION_ID_INT,
		(Xil_ExceptionHandler)XScuGic_InterruptHandler,
		IntcInstancePtr);
	/*
	 * Connect a device driver handler that will be called when an
	 * interrupt for the device occurs, the device driver handler
	 * performs the specific interrupt processing for the device
	 */

	Status = XScuGic_Connect(IntcInstancePtr, IntrId,
				 (Xil_ExceptionHandler)XSysMonPsv_IntrHandler,
				 (void *)InstancePtr);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable the interrupt for the device */
	XScuGic_Enable(IntcInstancePtr, IntrId);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}
#endif

/** @} */
