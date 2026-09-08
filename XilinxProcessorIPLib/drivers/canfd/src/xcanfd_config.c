/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanfd_config.c
* @addtogroup canfd Overview
* @{
* @brief CAN Configuration Register access related functions.
*
* Functions in this file are CAN Configuration Register access related.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   nsk  06/04/15 First release
* 2.1   ask  06/25/18 Added support for canfd 2.0 spec sequential mode CR# 992606,
*			 CR# 1004222. Added Api: XCanFd_SetRxIntrWatermarkFifo1
*								 XCanFd_SetTxEventIntrWatermark
*								 XCanFd_SetRxFilterPartition
*					  Modified Api: XCanFd_SetBitTiming
*							XCanFd_SetFBitTiming
* 2.1   nsk  01/22/19 Fixed XCanFd_SetFBaudPrescalar(), which is not setting
*		      prescalar value properly CR# 1016013
* 2.13  vn   07/30/26 Fix Doxygen warnings
* 2.13  vn   09/08/26 Align Doxygen comments with AMD standards
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xcanfd.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


/*****************************************************************************/
/**
* @brief Sets Baud Rate Prescaler value in Arbitration Phase.
*
* The system clock for the CAN controller is divided by (Prescaler + 1)
* to generate the quantum clock needed for sampling and synchronization.
* Read the device specification for details.
*
* Baud Rate Prescaler can be set only after CAN device enters Configuration
* Mode. Call XCanFd_EnterMode() to enter Configuration Mode before
* using this function.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param[in]	Prescaler is the value to set. Valid values are from 0 to 255.
*
* @return	- XST_SUCCESS if the Baud Rate Prescaler value is set
*		successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
*
******************************************************************************/
int XCanFd_SetBaudRatePrescaler(XCanFd *InstancePtr, u8 Prescaler)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Return error code if the device currently is NOT in Configuration
	 * Mode
	 */
	if (XCanFd_GetMode(InstancePtr) != (u8)XCANFD_MODE_CONFIG) {
		return (s32)XST_FAILURE;
	}

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_BRPR_OFFSET,
			   (u32) Prescaler);

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Retrieves the Baud Rate Prescaler value.
*
* The system clock for the CAN controller is divided by (Prescaler + 1)
* to generate the quantum clock needed for sampling and synchronization.
* Read the device specification for details.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	Current used Baud Rate Prescaler value. The value's range is
*		from 0 to 255.
*
******************************************************************************/
u8 XCanFd_GetBaudRatePrescaler(XCanFd *InstancePtr)
{
	u8 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = (u8) XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_BRPR_OFFSET);

	return Result;
}

/*****************************************************************************/
/**
* @brief Sets Bit time for Arbitration Phase.
*
* Time segment 1, Time segment 2 and Synchronization Jump Width are set
* in this function. Device specification requires the values passed into
* this function be one less than the actual values of these fields. Read
* the device specification for details.
*
* Bit time can be set only after CAN device enters Configuration Mode.
* Call XCanFd_EnterMode() to enter Configuration Mode before using this
* function.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param[in]	SyncJumpWidth is the Synchronization Jump Width value to set.
*		Valid values are from 0 to 3.
* @param[in]	TimeSegment2 is the Time Segment 2 value to set. Valid values
*		are from 0 to 7.
* @param[in]	TimeSegment1 is the Time Segment 1 value to set. Valid values
*		are from 0 to 15.
*
* @return	- XST_SUCCESS if the Bit time is set successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
*		- XST_INVALID_PARAM if any value of SyncJumpWidth, TimeSegment2
*		and TimeSegment1 is invalid.
*
******************************************************************************/
int XCanFd_SetBitTiming(XCanFd *InstancePtr, u8 SyncJumpWidth,
			  u8 TimeSegment2, u16 TimeSegment1)
{
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if ((SyncJumpWidth > (u8)XCANFD_MAX_SJW_VALUE) ||
	    (TimeSegment2 > (u8)XCANFD_MAX_TS2_VALUE) ||
	    (TimeSegment1 > (u16)XCANFD_MAX_TS1_VALUE)) {
		return (s32)XST_INVALID_PARAM;
	}

	/* Return error code if the device is NOT in Configuration Mode */
	if (XCanFd_GetMode(InstancePtr) != (u8)XCANFD_MODE_CONFIG) {
		return (s32)XST_FAILURE;
	}

	Value = ((u32) TimeSegment1) & XCANFD_BTR_TS1_MASK;
	Value |= (((u32) TimeSegment2) << XCANFD_BTR_TS2_SHIFT) &
		XCANFD_BTR_TS2_MASK;
	Value |= (((u32) SyncJumpWidth) << XCANFD_BTR_SJW_SHIFT) &
		XCANFD_BTR_SJW_MASK;

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_BTR_OFFSET, Value);

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Retrieves Bit time for Arbitration Phase.
*
* Time segment 1, Time segment 2 and Synchronization Jump Width values
* are read in this function. According to device specification, the actual
* value of each of these fields is one more than the value read. Read the
* device specification for details.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param[in]	SyncJumpWidth [out] stores the Synchronization Jump Width value.
*		Its value ranges from 0 to 3.
* @param[in]	TimeSegment2 [out] stores the Time Segment 2 value.
*		Its value ranges from 0 to 7.
* @param[in]	TimeSegment1 [out] stores the Time Segment 1 value.
*		Its value ranges from 0 to 15.
*
******************************************************************************/
void XCanFd_GetBitTiming(XCanFd *InstancePtr, u8 *SyncJumpWidth,
			   u8 *TimeSegment2, u8 *TimeSegment1)
{
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Value = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_BTR_OFFSET);

	*TimeSegment1 = (u8) (Value & XCANFD_BTR_TS1_MASK);
	*TimeSegment2 =
		(u8) ((Value & XCANFD_BTR_TS2_MASK) >> XCANFD_BTR_TS2_SHIFT);
	*SyncJumpWidth =
		(u8) ((Value & XCANFD_BTR_SJW_MASK) >> XCANFD_BTR_SJW_SHIFT);
}

/*****************************************************************************/
/**
* @brief Sets Baud Rate Prescaler value in Data Phase.
*
* The system clock for the CAN controller is divided by (Prescaler + 1)
* to generate the quantum clock needed for sampling and synchronization.
* Read the device specification for details.
*
* Baud Rate Prescaler can be set only after CAN device enters Configuration
* Mode. Call XCanFd_EnterMode() to enter Configuration Mode before
* using this function.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param[in]	Prescaler is the value to set. Valid values are from 1 to 256.
*
* @return	- XST_SUCCESS if the Baud Rate Prescaler value is set
*		successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
*
******************************************************************************/
int XCanFd_SetFBaudRatePrescaler(XCanFd *InstancePtr, u8 Prescaler)
{
	u32 RegValue;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Return error code if the device currently is NOT in Configuration
	 * Mode
	 */
	if (XCanFd_GetMode(InstancePtr) != (u8)XCANFD_MODE_CONFIG) {
		return (s32)XST_FAILURE;
	}
	RegValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_F_BRPR_OFFSET);
	RegValue = RegValue & (~ XCANFD_BRPR_BRP_MASK);
	RegValue |= ((u32) Prescaler & XCANFD_BRPR_BRP_MASK);
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
		XCANFD_F_BRPR_OFFSET,RegValue);

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Retrieves the Baud Rate Prescaler value in Data Phase.
*
* The system clock for the CAN controller is divided by (Prescaler + 1)
* to generate the quantum clock needed for sampling and synchronization.
* Read the device specification for details.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	Current used Baud Rate Prescaler value. The value's range is
*		from 1 to 256.
*
******************************************************************************/
u8 XCanFd_GetFBaudRatePrescaler(XCanFd *InstancePtr)
{
	u32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = (u8) XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_F_BRPR_OFFSET);

	return (u8)(Result & XCANFD_BRPR_BRP_MASK);
}

/*****************************************************************************/
/**
* @brief Sets Bit time in Data Phase.
*
* Time segment 1, Time segment 2 and Synchronization Jump Width are set
* in this function. Device specification requires the values passed into
* this function be one less than the actual values of these fields. Read
* the device specification for details.
*
* Bit time can be set only after CAN device enters Configuration Mode.
* Call XCanFd_EnterMode() to enter Configuration Mode before using this
* function.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param[in]	SyncJumpWidth is the Synchronization Jump Width value to set.
*		Valid values are from 0 to 3.
* @param[in]	TimeSegment2 is the Time Segment 2 value to set. Valid values
*		are from 0 to 7.
* @param[in]	TimeSegment1 is the Time Segment 1 value to set. Valid values
*		are from 0 to 15.
*
* @return	- XST_SUCCESS if the Bit time is set successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
*		- XST_INVALID_PARAM if any value of SyncJumpWidth, TimeSegment2
*		and TimeSegment1 is invalid.
*
******************************************************************************/
int XCanFd_SetFBitTiming(XCanFd *InstancePtr, u8 SyncJumpWidth,
			  u8 TimeSegment2, u8 TimeSegment1)
{
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	if ((SyncJumpWidth > (u8)XCANFD_MAX_F_SJW_VALUE) ||
	    (TimeSegment2 > (u8)XCANFD_MAX_F_TS2_VALUE) ||
	    (TimeSegment1 > (u8)XCANFD_MAX_F_TS1_VALUE)) {
		return (s32)XST_INVALID_PARAM;
	}

	/* Return error code if the device is NOT in Configuration Mode */
	if (XCanFd_GetMode(InstancePtr) != (u8)XCANFD_MODE_CONFIG) {
		return (s32)XST_FAILURE;
	}

	Value = ((u32) TimeSegment1) & XCANFD_F_BTR_TS1_MASK;
	Value |= (((u32) TimeSegment2) << XCANFD_F_BTR_TS2_SHIFT) &
		XCANFD_F_BTR_TS2_MASK;
	Value |= (((u32) SyncJumpWidth) << XCANFD_F_BTR_SJW_SHIFT) &
		XCANFD_F_BTR_SJW_MASK;

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_F_BTR_OFFSET,Value);

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief Retrieves Bit time in Data Phase.
*
* Time segment 1, Time segment 2 and Synchronization Jump Width values
* are read in this function. According to device specification, the actual
* value of each of these fields is one more than the value read. Read the
* device specification for details.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param[in]	SyncJumpWidth [out] stores the Synchronization Jump Width value.
*		Its value ranges from 0 to 3.
* @param[in]	TimeSegment2 [out] stores the Time Segment 2 value.
*		Its value ranges from 0 to 7.
* @param[in]	TimeSegment1 [out] stores the Time Segment 1 value.
*		Its value ranges from 0 to 15.
*
******************************************************************************/
void XCanFd_GetFBitTiming(XCanFd *InstancePtr, u8 *SyncJumpWidth,
			   u8 *TimeSegment2, u8 *TimeSegment1)
{
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Value = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_F_BTR_OFFSET);

	*TimeSegment1 = (u8) (Value & XCANFD_F_BTR_TS1_MASK);
	*TimeSegment2 =
		(u8) ((Value & XCANFD_F_BTR_TS2_MASK) >>
			 XCANFD_F_BTR_TS2_SHIFT);
	*SyncJumpWidth =
		(u8) ((Value & XCANFD_F_BTR_SJW_MASK) >>
			XCANFD_F_BTR_SJW_SHIFT);
}

/*****************************************************************************/
/**
* @brief Sets the Bit Rate Switch with nominal bit rate.
*
* If the BRSD bit is set in Mode Select Register, the CAN Controller
* transmits CAN FD Frames with Nominal Bit Rate.
* Read the device specification for details.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
******************************************************************************/
void XCanFd_SetBitRateSwitch_EnableNominal(XCanFd *InstancePtr)
{
	u32 Result;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_MSR_OFFSET);
	if ((Result & XCANFD_SRR_CEN_MASK) == (u32)0) {
		Result = Result | XCANFD_MSR_BRSD_MASK;
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_MSR_OFFSET,Result);
	}

}

/*****************************************************************************/
/**
* @brief Disables the BRSD bit for Bit Rate Switch.
*
* Read the device specification for details.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @note		if we set BRSD bit in Mode Select Register then CAN Controller
*		transmits CAN FD Frames with Nominal Bit Rate. else with
*		configured bit rate(As specified in Data phase BRPR and BTR
*		Registers.
*
******************************************************************************/
void XCanFd_SetBitRateSwitch_DisableNominal(XCanFd *InstancePtr)
{
	u32 Result;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_MSR_OFFSET);
	if ((Result & XCANFD_SRR_CEN_MASK) == (u32)0) {
		Result = Result & (~XCANFD_MSR_BRSD_MASK);
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_MSR_OFFSET,Result);
	}
}

/****************************************************************************/
/**
* @brief Sets the Rx Full threshold in the Watermark Interrupt Register.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance.
* @param[in]	Threshold is the threshold to be set. The valid values are
*		from 1 to 63.
*
* @return	- XST_FAILURE if the CAN device is not in Configuration Mode.
*		- XST_SUCCESS if the Rx Full threshold is set in Watermark
*		Interrupt Register.
*
* @note		The threshold can only be set when the CAN device is in the
*		configuration mode.
*
*****************************************************************************/
u32 XCanFd_SetRxIntrWatermark(XCanFd *InstancePtr, s8 Threshold)
{

	u32 Status;
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Threshold >= (s8)0) && (Threshold <= (s8)XCANFD_WM_FIFO0_THRESHOLD));

	if (XCanFd_GetMode(InstancePtr) != (u8)XCANFD_MODE_CONFIG) {
		Status = (u32)XST_FAILURE;
	}
	else {
		Value = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_WIR_OFFSET);
		Value &= (~XCANFD_WIR_MASK);
		Value |= (((u32)Threshold) &
				(u32)XCANFD_WIR_MASK);
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_WIR_OFFSET,Value);
		Status = (u32)XST_SUCCESS;
	}
	return Status;

}

/****************************************************************************/
/**
* @brief Sets the Rx Full threshold for FIFO 1 in the Watermark
*		Interrupt Register.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance.
* @param[in]	Threshold is the threshold to be set. The valid values are
*		from 1 to 63.
*
* @return	- XST_FAILURE if the CAN device is not in Configuration Mode.
*		- XST_SUCCESS if the Rx Full threshold is set in Watermark
*		Interrupt Register.
*
* @note		The threshold can only be set when the CAN device is in the
*			configuration mode.This API is meant to be used with IP with
*			CanFD 2.0 spec support only.
*
*****************************************************************************/
u32 XCanFd_SetRxIntrWatermarkFifo1(XCanFd *InstancePtr, s8 Threshold)
{

	u32 Status;
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Threshold >=(s8)0) && (Threshold <= (s8)63));

	if (XCanFd_GetMode(InstancePtr) != (u8)XCANFD_MODE_CONFIG) {
		Status = (u32)XST_FAILURE;
	}
	else {
		Value = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_WIR_OFFSET);
		Value &= (~XCANFD_WMR_RXFWM_1_MASK);
		Value |= (((u32)Threshold << (u32)XCANFD_WMR_RXFWM_1_SHIFT) &
		                XCANFD_WMR_RXFWM_1_MASK);
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_WIR_OFFSET,Value);
		Status = (u32)XST_SUCCESS;
	}
	return Status;
}

/****************************************************************************/
/**
* @brief Sets the TX Events Full threshold in the Watermark Interrupt
*		Register.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance.
* @param[in]	Threshold is the threshold to be set. The valid values are
*		from 1 to 31.
*
* @return	- XST_FAILURE if the CAN device is not in Configuration Mode.
*		- XST_SUCCESS if the Rx Full threshold is set in Watermark
*		Interrupt Register.
*
* @note		The threshold can only be set when the CAN device is in the
*			configuration mode, This API is meant to be used with IP with
*			CanFD 2.0 spec support only.
*
*****************************************************************************/
u32 XCanFd_SetTxEventIntrWatermark(XCanFd *InstancePtr, u8 Threshold)
{

	u32 Status;
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((Threshold > (u8)0) || (Threshold <= (u8)31));

	if (XCanFd_GetMode(InstancePtr) != (u8)XCANFD_MODE_CONFIG) {
		Status = (u32)XST_FAILURE;
	}
	else {
		Value = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXEVENT_WIR_OFFSET);
		Value &= (~XCANFD_TXEVENT_WIR_MASK);
		Value |= ((u32)Threshold  & XCANFD_TXEVENT_WIR_MASK);
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXEVENT_WIR_OFFSET,Value);
		Status = XST_SUCCESS;
	}
	return Status;
}

/****************************************************************************/
/**
* @brief Sets the Receive filter partition in the Watermark Interrupt
*		Register.
*
* @param[in]	InstancePtr is a pointer to the XCanFd instance.
* @param[in]	FilterPartition is Filter Mask number, valid values are 0 to 31.
*
* @return	- XST_FAILURE if the CAN device is not in Configuration Mode.
*		- XST_SUCCESS if the Rx Full threshold is set in Watermark
*		Interrupt Register.
*
* @note		The RX filter partition can only be set when the CAN device is
*			in the configuration mode, This API is meant to be used with IP
*			with CanFD 2.0 spec support only.
*
*****************************************************************************/
u32 XCanFd_SetRxFilterPartition(XCanFd *InstancePtr, u8 FilterPartition)
{

	u32 Status;
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid((FilterPartition > (u8)0) || (FilterPartition <= (u8)31));

	if (XCanFd_GetMode(InstancePtr) != (u8)XCANFD_MODE_CONFIG) {
		Status = (u32)XST_FAILURE;
	}
	else {

        Value = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				      XCANFD_WIR_OFFSET);
		Value &= (~XCANFD_WMR_RXFP_MASK);
		Value |= (((u32)FilterPartition << XCANFD_WMR_RXFP_SHIFT) &
		                XCANFD_WMR_RXFP_MASK );
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				   XCANFD_WIR_OFFSET,Value);

	    Status = (u32)XST_SUCCESS;
	}
	return Status;
}

/** @} */
