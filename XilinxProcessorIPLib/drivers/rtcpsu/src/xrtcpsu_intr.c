/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrtcpsu_intr.c
* @brief Implements the interrupt handling functions of the XRtcPsu driver.
*
* This file contains functions related to RTC interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date	Changes
* ----- -----  -------- -----------------------------------------------
* 1.00  kvn    04/21/15 First release
* 1.3   vak    04/25/16 Changed the XRtcPsu_InterruptHandler() for updating RTC
*                       read and write time logic(cr#948833).
* 1.6   aru    08/17/18 Resolved MISRA-C mandatory violations.(CR#1007752)
* 1.7   sne    03/01/19 Added Versal support.
* 1.7   sne    03/01/19 Fixed violations according to MISRAC-2012 standards
*                       modified the code such as
*                       No brackets to loop body,Declared the pointer param
*                       as Pointer to const,No brackets to then/else,
*                       Literal value requires a U suffix,Casting operation to a pointer
*                       Array has no bounds specified,Logical conjunctions need brackets.
* 1.17  vlt    01/30/26 Fixed codespell errors.
* 1.18  vn     07/30/26 Fix Doxygen warnings
* 1.18  vn     09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup rtcpsu Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xrtcpsu.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions ****************************/

/****************************************************************************/
/**
*
* @brief Sets the interrupt mask.
*
* @param[in]	InstancePtr Pointer to the XRtcPsu instance.
* @param[in]	Mask Interrupts to be enabled.
*		A '1' enables an interrupt, and a '0' disables.
*
*****************************************************************************/
void XRtcPsu_SetInterruptMask(XRtcPsu *InstancePtr, u32 Mask)
{
	/*
	 * Clear the Status register to be sure of no pending interrupts.
	 * Writing mask values to interrupt bits as it is a WTC register.
	 */
	XRtcPsu_WriteReg(InstancePtr->RtcConfig.BaseAddr + XRTC_INT_STS_OFFSET,
			((u32)XRTC_INT_STS_ALRM_MASK | (u32)XRTC_INT_STS_SECS_MASK));

	/*
	 * XRTC_INT_MSK_RSTVAL contains the valid interrupts
	 * for the RTC device. The AND operation on Mask makes sure one
	 * of the valid bits are only set.
	 */

	/* Write the mask to the IER Register */
	XRtcPsu_WriteReg(InstancePtr->RtcConfig.BaseAddr+XRTC_INT_EN_OFFSET,
					(Mask & (u32)XRTC_INT_MSK_RSTVAL));

}

/****************************************************************************/
/**
*
* @brief Clears the interrupt mask.
*
* @param[in]	InstancePtr Pointer to the XRtcPsu instance.
* @param[in]	Mask Interrupts to be disabled.
*		A '1' enables an interrupt, and a '0' disables.
*
*****************************************************************************/
void XRtcPsu_ClearInterruptMask(XRtcPsu *InstancePtr, u32 Mask)
{
	/*
	 * XRTC_INT_MSK_RSTVAL contains the valid interrupts
	 * for the RTC device. The AND operation on mask makes sure one
	 * of the valid bits are only cleared.
	 */

	/* Write the Mask to the IDR register */
	XRtcPsu_WriteReg(InstancePtr->RtcConfig.BaseAddr+XRTC_INT_DIS_OFFSET,
					(Mask & (u32)XRTC_INT_MSK_RSTVAL));
}

/****************************************************************************/
/**
*
* @brief Sets the handler that the driver calls when an event occurs.
*
* The driver calls the handler when an event (interrupt) occurs that needs
* the application's attention.
*
* @param[in,out] InstancePtr Pointer to the XRtcPsu instance.
* @param[in]	 FunctionPtr Pointer to the callback function.
* @param[in]	 CallBackRef Upper layer callback reference passed back when
*		 the callback function is invoked.
*
* @note
* There is no assert on the CallBackRef since the driver does not know what it
* is (nor should it).
*
*****************************************************************************/
void XRtcPsu_SetHandler(XRtcPsu *InstancePtr, XRtcPsu_Handler FunctionPtr,
		 void *CallBackRef)
{
	/*
	 * Asserts validate the input arguments
	 * CallBackRef not checked, no way to know what is valid
	 */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FunctionPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Handler = FunctionPtr;
	InstancePtr->CallBackRef = CallBackRef;
}

/****************************************************************************/
/**
*
* @brief Handles interrupts for the driver.
*
* The application must connect it to an interrupt system so that it
* can be called when an interrupt occurs.
*
* @param[in,out] InstancePtr Pointer to the driver instance.
*
******************************************************************************/
void XRtcPsu_InterruptHandler(XRtcPsu *InstancePtr)
{
	u32 IsrStatus;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the interrupt ID register to determine which
	 * interrupt is active.
	 */
	IsrStatus = ~(XRtcPsu_ReadReg(InstancePtr->RtcConfig.BaseAddr +
			XRTC_INT_MSK_OFFSET));

	IsrStatus &= XRtcPsu_ReadReg(InstancePtr->RtcConfig.BaseAddr +
			XRTC_INT_STS_OFFSET);

	/*
	 * Clear the interrupt status to allow future
	 * interrupts before this generated interrupt is serviced.
	 */
	XRtcPsu_WriteReg(InstancePtr->RtcConfig.BaseAddr +
			XRTC_INT_STS_OFFSET, IsrStatus);

	/* Handle the generated interrupts appropriately. */

	/* Alarm interrupt */
	if((IsrStatus & XRTC_INT_STS_ALRM_MASK) != (u32)0) {

		if(InstancePtr->IsPeriodicAlarm != 0U) {
			XRtcPsu_SetAlarm(InstancePtr,
					(XRtcPsu_GetCurrentTime(InstancePtr)+InstancePtr->PeriodicAlarmTime),1U);
		}

		/*
		 * Call the application handler to indicate that there is an
		 * alarm interrupt. If the application cares about this alarm,
		 * it will act accordingly through its own handler.
		 */
		InstancePtr->Handler(InstancePtr->CallBackRef,
					XRTCPSU_EVENT_ALARM_GEN);
	}

	/* Seconds interrupt */
	if((IsrStatus & XRTC_INT_STS_SECS_MASK) != (u32)0) {
		/* Set the CurrTimeUpdated flag to 1 */
		InstancePtr->CurrTimeUpdated = 1;

		if(InstancePtr->TimeUpdated == (u32)1) {
			/* Clear the TimeUpdated */
			InstancePtr->TimeUpdated = (u32)0;
		}

		/*
		 * Call the application handler to indicate that there is an
		 * seconds interrupt. If the application cares about this seconds
		 * interrupt, it will act accordingly through its own handler.
		 */
		InstancePtr->Handler(InstancePtr->CallBackRef,
					XRTCPSU_EVENT_SECS_GEN);
	}

}
/** @} */
