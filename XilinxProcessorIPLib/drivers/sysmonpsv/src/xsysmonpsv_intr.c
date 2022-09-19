/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsv_intr.c
* @addtogroup Overview
* @{
*
* Functions in this file are the minimum required functions for the XSysMonPsv
* driver. See xsysmonpsv.h for a detailed description of the driver.
*
* @note         None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date         Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   aad    11/20/18 First release.
* 1.3   aad    06/23/20 Fixed the register to read enabled interrupts.
* 2.0   aad    02/10/20 Added new Interrupt handler structure.
* 2.3   aad    04/30/21 Size optimization for PLM.
*       aad    07/26/21 Fixed doxygen comments.
*       aad    09/21/21 Fixed warning.
* 3.0   cog    03/25/21 Driver Restructure
* 3.1   cog    04/09/22 Remove GIC standalone related functionality for
*                       arch64 architecture
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_assert.h"
#include "xsysmonpsv.h"
#include "xsysmonpsv_lowlevel.h"
#include "xsysmonpsv_supplylist.h"

/************************** Constant Definitions ****************************/
#define XSYSMONPSV_INTR_OFFSET                                                 \
	0xCU /**< Interrupt register
                                                  offset */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

/************************** Variable Definitions ****************************/
/****************************************************************************/
/**
*
* This function enables the specified interrupts in the device.
*
* @param        InstancePtr is a pointer to the XSysMonPsv instance.
* @param        Mask is the 32 bit-mask of the interrupts to be enabled.
*               Bit positions of 1 will be enabled. Bit positions of 0 will
*               keep the previous setting. This mask is formed by OR'ing
*               XSYSMONPSV_IER_*  bits defined in xsysmonpsv_hw.h.
* @param        IntrNum is the interrupt enable register to be used
*
* @return       None.
*
* @note         None.
*
*****************************************************************************/
void XSysMonPsv_IntrEnable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum)
{
	u32 Offset;

	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(IntrNum <= 1U);

	/* Calculate the offset of the IER register to be written to */
	Offset = (XSYSMONPSV_IER0_OFFSET +
		  ((u32)IntrNum * XSYSMONPSV_INTR_OFFSET));

	/* Enable the specified interrupts in the AMS Interrupt Enable Register. */
	XSysMonPsv_WriteReg32(InstancePtr, Offset, Mask);
}

/****************************************************************************/
/**
*
* This function returns the enabled interrupts read from the Interrupt Mask
* Register (IMR). Use the XSYSMONPSV_IMR0_* and XSYSMONPSV_IMR1_* constants
* defined in xsysmonpsv_hw.h to interpret the returned value.
*
* @param        InstancePtr is a pointer to the XSysMonPsv instance.
* @param        IntrNum is the interrupt enable register to be used
*
* @return       A 32-bit value representing the contents of the Interrupt Mask
*               Registers.
*
* @note         None.
*
*****************************************************************************/
u32 XSysMonPsv_IntrGetEnabled(XSysMonPsv *InstancePtr, u8 IntrNum)
{
	u32 Interrupts;
	u32 Offset;

	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(IntrNum <= 1U);

	/* Calculate the offset of the IER register to be written to */
	Offset = (XSYSMONPSV_IMR0_OFFSET +
		  ((u32)IntrNum * XSYSMONPSV_INTR_OFFSET));
	/* Return the value read from the AMS Interrupt Mask Register. */
	XSysMonPsv_ReadReg32(InstancePtr, Offset, &Interrupts);

	return ~(Interrupts);
}

/****************************************************************************/
/**
*
* This function disables the specified interrupts in the device.
*
* @param        InstancePtr is a pointer to the XSysMonPsv instance.
* @param        Mask is the 32 bit-mask of the interrupts to be enabled.
*               Bit positions of 1 will be disabled. Bit positions of 0 will
*               keep the previous setting. This mask is formed by OR'ing
*               XSYSMONPSV_IDR_*  bits defined in xsysmonpsv_hw.h.
* @param        IntrNum is the interrupt disable register to be used
*
* @return       None.
*
* @note         None.
*
*****************************************************************************/
void XSysMonPsv_IntrDisable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum)
{
	u32 Offset;

	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(IntrNum <= 1U);

	/* Calculate the offset of the IDR register to be written to */
	Offset = (XSYSMONPSV_IDR0_OFFSET +
		  ((u32)IntrNum * XSYSMONPSV_INTR_OFFSET));

	/* Disable the specified interrupts in the AMS Interrupt Disable Register. */
	XSysMonPsv_WriteReg32(InstancePtr, Offset, Mask);
}

/****************************************************************************/
/**
*
* This function returns the interrupt status read from Interrupt Status
* Register(ISR). Use the XSYSMONPSV_ISR* constants defined in xsysmonpsv_hw.h
* to interpret the returned value.
*
* @param        InstancePtr is a pointer to the XSysMonPsv instance.
*
* @return       A 32-bit value representing the contents of the Interrupt Status
*               Register (ISR).
*
* @note         None.
*
*****************************************************************************/
u32 XSysMonPsv_IntrGetStatus(XSysMonPsv *InstancePtr)
{
	u32 IntrStatus;

	/* Assert the arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XSysMonPsv_ReadReg32(InstancePtr, XSYSMONPSV_ISR_OFFSET, &IntrStatus);

	return IntrStatus;
}

/****************************************************************************/
/**
*
* This function clears the specified interrupts in the Interrupt Status
* Register (ISR).
*
* @param        InstancePtr is a pointer to the XSysMonPsv instance.
* @param        Mask is the 32 bit-mask of the interrupts to be cleared.
*               Bit positions of 1 will be cleared. Bit positions of 0 will not
*               change the previous interrupt status.*
* @return       None.
*
* @note         None.
*
*****************************************************************************/
void XSysMonPsv_IntrClear(XSysMonPsv *InstancePtr, u32 Mask)
{
	/* Assert the arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Clear the specified interrupts in the Interrupt Status register. */
	XSysMonPsv_WriteReg32(InstancePtr, XSYSMONPSV_ISR_OFFSET, Mask);
}

/****************************************************************************/
/**
*
* This function sets a supply as a source new data interrupt.
*
* @param        InstancePtr is a pointer to the XSysMonPsv instance.
* @param        Supply is an enum from the XSysMonPsv_Supply
* @param        Mask is a 32 bit Mask for NEW_DATA_n fields in the interrupt
*               registers
* @return       None.
*
* @note         None.
*
*****************************************************************************/
void XSysMonPsv_SetNewDataIntSrc(XSysMonPsv *InstancePtr,
				 XSysMonPsv_Supply Supply, u32 Mask)
{
	u32 Reg, Val, Shift, Index;

	/* Assert the arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Mask & XSYSMONPSV_INTR_NEW_DATA_MASK) != 0U);

	XSysMonPsv_ReadReg32(InstancePtr, XSYSMONPSV_NEW_DATA_INT_SRC, &Reg);
	Val = (Mask & XSYSMONPSV_INTR_NEW_DATA_MASK) >>
	      XSYSMONPSV_INTR_NEW_DATA_SHIFT;

	for (Index = 0U; Index < 4U; Index++) {
		Val = Val >> 1U;

		if (Val == 0U) {
			break;
		}
	}

	Shift = XSYSMONPSV_NEW_DATA_INT_SRC_ADDR_ID1_SHIFT * Index;
	Val = InstancePtr->Config.Supply_List[Supply];

	if (Index < 4U) {
		Reg |= Val << Shift;
		XSysMonPsv_WriteReg32(InstancePtr, XSYSMONPSV_NEW_DATA_INT_SRC,
				    Reg);
	}
}

#if defined (ARMR5) || defined (__aarch64__)
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
 * @note        None.
 *
*******************************************************************************/
void XSysMonPsv_SetTempEventHandler(XSysMonPsv *InstancePtr,
				    XSysMonPsv_Handler CallbackFunc,
				    void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->TempEvent.Handler = CallbackFunc;
	InstancePtr->TempEvent.CallbackRef = CallbackRef;
	InstancePtr->TempEvent.IsCallbackSet = 1U;
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
 * @note        None.
 *
*******************************************************************************/
void XSysMonPsv_SetOTEventHandler(XSysMonPsv *InstancePtr,
				  XSysMonPsv_Handler CallbackFunc,
				  void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	InstancePtr->OTEvent.Handler = CallbackFunc;
	InstancePtr->OTEvent.CallbackRef = CallbackRef;
	InstancePtr->OTEvent.IsCallbackSet = 1U;
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
 * @note        None.
 *
*******************************************************************************/
void XSysMonPsv_SetSupplyEventHandler(XSysMonPsv *InstancePtr,
				      XSysMonPsv_Supply Supply,
				      XSysMonPsv_Handler CallbackFunc,
				      void *CallbackRef)
{
	u32 SupplyReg;
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	SupplyReg = InstancePtr->Config.Supply_List[Supply];
	InstancePtr->SupplyEvent[SupplyReg].Handler = CallbackFunc;
	InstancePtr->SupplyEvent[SupplyReg].CallbackRef = CallbackRef;
	InstancePtr->SupplyEvent[SupplyReg].Supply = Supply;
	InstancePtr->SupplyEvent[SupplyReg].IsCallbackSet = 1U;
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
 * @note        None.
 *
*******************************************************************************/
void XSysMonPsv_AlarmEventHandler(XSysMonPsv *InstancePtr)
{
	u32 DevTempDetected, OTTempDetected, SupplyAlarm;
	/* Upper 16 bits contain Min Temp, Lower 16 bits contain Max Temp */
	u32 IntrStatus, SupplyReg;
	u32 SupplyNum = 0U, ListLength = (u32)EndList;
	XSysMonPsv_EventHandler *EventHandler;
	XSysMonPsv_Supply Supply = (XSysMonPsv_Supply)0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Determine what kind of interrupt occured */
	IntrStatus = XSysMonPsv_IntrGetStatus(InstancePtr);

	/* Clear interrupt status register */
	XSysMonPsv_IntrClear(InstancePtr, IntrStatus);

	SupplyAlarm = IntrStatus &
		      (XSYSMONPSV_ISR_ALARM0_MASK | XSYSMONPSV_ISR_ALARM1_MASK |
		       XSYSMONPSV_ISR_ALARM2_MASK | XSYSMONPSV_ISR_ALARM3_MASK |
		       XSYSMONPSV_ISR_ALARM4_MASK);
	DevTempDetected = IntrStatus & XSYSMONPSV_ISR_TEMP_MASK;
	OTTempDetected = IntrStatus & XSYSMONPSV_ISR_OT_MASK;

	/* Handle OT Event */
	if ((OTTempDetected != 0U) &&
	    (InstancePtr->OTEvent.IsCallbackSet == 1U)) {
		InstancePtr->OTEvent.Handler(InstancePtr->OTEvent.CallbackRef);
	}

	/* Handle Dev Temp Event */
	if ((DevTempDetected != 0U) &&
	    (InstancePtr->TempEvent.IsCallbackSet == 1U)) {
		InstancePtr->TempEvent.Handler(
			InstancePtr->TempEvent.CallbackRef);
	}

	if (SupplyAlarm != 0U) {
		for (SupplyNum = 0U; SupplyNum < ListLength; SupplyNum++) {
			Supply = (XSysMonPsv_Supply)SupplyNum;
			if (XSysMonPsv_IsAlarmCondition(InstancePtr, Supply) ==
			    1U) {
				SupplyReg =
					InstancePtr->Config.Supply_List[Supply];
				EventHandler =
					&InstancePtr->SupplyEvent[SupplyReg];

				if (EventHandler->IsCallbackSet == 1U) {
					EventHandler->Handler(
						EventHandler->CallbackRef);
				}
			}
		}
	}
}
#endif
/** @} */
