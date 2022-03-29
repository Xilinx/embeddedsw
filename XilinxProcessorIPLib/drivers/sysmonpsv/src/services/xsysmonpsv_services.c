/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file sysmon_services.c
* @addtogroup sysmonpsv_v3_0
*
* Functions in this file provides services like temperature and voltage
* event notification if subscribed.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------
* 3.0   cog    03/25/21 Driver Restructure
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xsysmonpsv_lowlevel.h"
#include "xsysmonpsv_common.h"
#include "xsysmonpsv_services.h"
#include "xsysmonpsv_hw.h"

/******************************************************************************/
/**
 * This function enables voltage event for the supply.
 *
 * @param	InstancePtr is a pointer to the driver instance.
 * @param	Supply is an enum from the XSysMonPsv_Supply.
  * @param	IntrNum is interrupt Offset.
 *
 * @return	- -XSYSMONPSV_EINVAL if error.
 *		- XSYSMONPSV_SUCCESS if successful.
 *
*******************************************************************************/
int XSysMonPsv_EnableVoltageEvents(XSysMonPsv *InstancePtr, u32 Supply,
				   u32 IntrNum)
{
	u32 AlarmRegOffset;
	u32 Event, Val, Bit;
	u32 SupplyReg, Ier;

	if (InstancePtr == NULL) {
		return -XSYSMONPSV_EINVAL;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Bit = ALARM_SHIFT(SupplyReg);
	Event = ALARM_REG(SupplyReg);
	AlarmRegOffset = XSYSMONPSV_ALARM_REG0 + (Event * 4U);

	XSysMonPsv_ReadReg32(InstancePtr, AlarmRegOffset, &Val);
	Val = Val | (1U << Bit);
	Ier = GET_BIT(Event);
	XSysMonPsv_InterruptEnable(InstancePtr, Ier, IntrNum);
	XSysMonPsv_WriteReg32(InstancePtr, AlarmRegOffset, Val);

	return XSYSMONPSV_SUCCESS;
}

/******************************************************************************/
/**
 * This function disables voltage event for the supply.
 *
 * @param	InstancePtr is a pointer to the driver instance.
 * @param	Supply is an enum from the XSysMonPsv_Supply.
 *
 * @return	- -XSYSMONPSV_EINVAL if error
 *		- XSYSMONPSV_SUCCESS if successful.
 *
*******************************************************************************/
int XSysMonPsv_DisableVoltageEvents(XSysMonPsv *InstancePtr, u32 Supply)
{
	u32 AlarmRegOffset;
	u32 Event, Val, Bit;
	u32 SupplyReg;

	if (InstancePtr == NULL) {
		return -XSYSMONPSV_EINVAL;
	}

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	Bit = ALARM_SHIFT(SupplyReg);
	Event = ALARM_REG(SupplyReg);
	AlarmRegOffset = XSYSMONPSV_ALARM_REG0 + (Event * 4U);

	XSysMonPsv_ReadReg32(InstancePtr, AlarmRegOffset, &Val);
	Val = Val & (~(1U << Bit));

	XSysMonPsv_WriteReg32(InstancePtr, AlarmRegOffset, Val);

	return XSYSMONPSV_SUCCESS;
}

#if defined (ARMR5) || defined (__arch64__) || defined (__aarch64__)
/******************************************************************************/
/**
 * This function installs a callback function for when a Device Temperature
 * interrupt occurs
 *
 * @param       InstancePtr is a pointer to the XSysMonPsv instance.
 * @param       CallbackFunc is the address to the callback function.
 * @param       CallbackRef is the user data item that will be passed to the
 *              callback function when it is invoked.
 *
 * @return      None.
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
 * This function removes the callback function registered for a Device Temperature.
 *
 * @param       InstancePtr is a pointer to the XSysMonPsv instance.
 *
 * @return      None.
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
 * This function installs a callback function for when a OT Temperature
 * interrupt occurs
 *
 * @param       InstancePtr is a pointer to the XSysMonPsv instance.
 * @param       CallbackFunc is the address to the callback function.
 * @param       CallbackRef is the user data item that will be passed to the
 *              callback function when it is invoked.
 *
 * @return      None.
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
 * This function removes callback function registered for OT Temperature.
 *
 * @param       InstancePtr is a pointer to the XSysMonPsv instance.
 *
 * @return      None.
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
 * This function installs a callback function for when a Supply Voltage alarm
 * interrupt occurs
 *
 * @param       InstancePtr is a pointer to the XSysMonPsv instance.
 * @param       Supply is the supply for which the alarm is to be set.
 * @param       CallbackFunc is the address to the callback function.
 * @param       CallbackRef is the user data item that will be passed to the
 *              callback function when it is invoked.
 *
 * @return      None.
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
 * This function removes the registered a callback function for Supply Voltage alarm
 *
 *
 * @param       InstancePtr is a pointer to the XSysMonPsv instance.
 * @param       Supply is the supply for which the alarm is to be set.
 *
 * @return      None.
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
 * This function is the interrupt handler for the XSysMonPsv driver.
 *
 * When an interrupt happens, it first detects what kind of interrupt happened,
 * then decides which callback function to invoke.
 *
 * @param       InstancePtr is a pointer to the XSysMonPsv instance.
 *
 * @return      None.
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

	/* Determine what kind of interrupt occured */
	XSysMonPsv_InterruptGetStatus(InstancePtr, &IntrStatus);

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
				XSysMonPsv_DisableVoltageEvents(InstancePtr,
								Supply);
				XSysMonPsv_ClearAlarm(InstancePtr, Supply);
				if (EventHandler->IsCallbackSet == 1U) {
					EventHandler->Handler(
						EventHandler->CallbackRef);
				}
			}
		}
	}
	XSysMonPsv_InterruptClear(InstancePtr, IntrStatus);
}

/****************************************************************************/
/**
*
* This function registers ISR for driver instance
*
* @param	IntcInstancePtr is pointer to Interrupt instance.
* @param	InstancePtr is a pointer to the driver instance.
* @param	IntrId is Interrupt unique ID.
*
* @return	None.
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
		return XSYSMONPSV_EINVAL;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XSYSMONPSV_SUCCESS) {
		return XSYSMONPSV_EINVAL;
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

	if (Status != XSYSMONPSV_SUCCESS) {
		return XSYSMONPSV_EINVAL;
	}

	/* Enable the interrupt for the device */
	XScuGic_Enable(IntcInstancePtr, IntrId);

	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XSYSMONPSV_SUCCESS;
}
#endif
