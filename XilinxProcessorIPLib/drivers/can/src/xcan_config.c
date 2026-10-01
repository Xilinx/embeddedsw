/******************************************************************************
* Copyright (C) 2005 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcan_config.c
* @brief Implements CAN Configuration Register access functions.
*
* Functions in this file are CAN Configuration Register access related.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a xd   04/12/05 First release
* 1.10a mta  05/13/07 Updated to new coding style
* 2.00a ktn  10/22/09 Updated to use the HAL APIs/macros.
*		      The macros have been renamed to remove _m from the name.
* 3.13  vn   07/30/26 Fix Doxygen warnings
* 3.13  vn   09/08/26 Align Doxygen comments with AMD standards
* 3.13  vn   09/30/26 Update Doxygen comments to AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup can Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xcan.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/


/*****************************************************************************/
/**
*
* @brief Sets the Baud Rate Prescaler value.
*
* The system clock for the CAN
* controller is divided by (Prescaler + 1) to generate the quantum clock
* needed for sampling and synchronization. Read the device specification
* for details.
*
* Set the Baud Rate Prescaler only after the CAN device enters Configuration
* Mode. Call XCan_EnterMode() to enter Configuration Mode before using this
* function.
*
* @param[in]    InstancePtr Pointer to the XCan instance to be worked on.
* @param[in]    Prescaler Value to set. Valid values are from 0 to 255.
*
* @return
*		- XST_SUCCESS if the Baud Rate Prescaler value is set
*                 successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
*
******************************************************************************/
int XCan_SetBaudRatePrescaler(XCan *InstancePtr, u8 Prescaler)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Return error code if the device currently is NOT in Configuration
	 * Mode
	 */
	if (XCan_GetMode(InstancePtr) != XCAN_MODE_CONFIG) {
		return XST_FAILURE;
	}

	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_BRPR_OFFSET,
			   (u32) Prescaler);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Returns the Baud Rate Prescaler value.
*
* The system clock for the CAN
* controller is divided by (Prescaler + 1) to generate the quantum clock
* needed for sampling and synchronization. Read the device specification
* for details.
*
* @param[in]    InstancePtr Pointer to the XCan instance to be worked on.
*
* @return	Current used Baud Rate Prescaler value. The value's range is
*		from 0 to 255.
*
******************************************************************************/
u8 XCan_GetBaudRatePrescaler(XCan *InstancePtr)
{
	u8 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = (u8) XCan_ReadReg(InstancePtr->BaseAddress, XCAN_BRPR_OFFSET);

	return Result;
}

/*****************************************************************************/
/**
*
* @brief Sets the Bit Timing Register values.
*
* Time segment 1, Time segment 2 and
* Synchronization Jump Width are set in this function. Device specification
* requires the values passed into this function be one less than the actual
* values of these fields. Read the device specification for details.
*
* Set the Bit time only after the CAN device enters Configuration Mode.
* Call XCan_EnterMode() to enter Configuration Mode before using this
* function.
*
* @param[in]    InstancePtr Pointer to the XCan instance to be worked on.
* @param[in]    SyncJumpWidth Synchronization Jump Width value to set.
*		Valid values are from 0 to 3.
* @param[in]    TimeSegment2 Time Segment 2 value to set. Valid values
*		are from 0 to 7.
* @param[in]    TimeSegment1 Time Segment 1 value to set. Valid values
*		are from 0 to 15.
*
* @return
*		- XST_SUCCESS if the Bit time is set successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
* 		- XST_INVALID_PARAM if any value of SyncJumpWidth, TimeSegment2
*                 and TimeSegment1 is invalid.
*
******************************************************************************/
int XCan_SetBitTiming(XCan *InstancePtr, u8 SyncJumpWidth,
			  u8 TimeSegment2, u8 TimeSegment1)
{
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	if (SyncJumpWidth > 3 || TimeSegment2 > 7 || TimeSegment1 > 15) {
		return XST_INVALID_PARAM;
	}

	/* Return error code if the device is NOT in Configuration Mode */
	if (XCan_GetMode(InstancePtr) != XCAN_MODE_CONFIG) {
		return XST_FAILURE;
	}

	Value = ((u32) TimeSegment1) & XCAN_BTR_TS1_MASK;
	Value |= (((u32) TimeSegment2) << XCAN_BTR_TS2_SHIFT) &
		XCAN_BTR_TS2_MASK;
	Value |= (((u32) SyncJumpWidth) << XCAN_BTR_SJW_SHIFT) &
		XCAN_BTR_SJW_MASK;

	XCan_WriteReg(InstancePtr->BaseAddress, XCAN_BTR_OFFSET, Value);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Reads the Bit Timing Register values.
*
* Time segment 1, Time segment 2 and
* Synchronization Jump Width values are read in this function. According to
* device specification, the actual value of each of these fields is one
* more than the value read. Read the device specification for details.
*
* @param[in]    InstancePtr Pointer to the XCan instance to be worked on.
* @param[out]   SyncJumpWidth Synchronization Jump Width value
*		after this function returns. Its value ranges from 0 to 3.
* @param[out]   TimeSegment2 Time Segment 2 value after this
*		function returns. Its value ranges from 0 to 7.
* @param[out]   TimeSegment1 Time Segment 1 value after this
*		function returns. Its value ranges from 0 to 15.
*
******************************************************************************/
void XCan_GetBitTiming(XCan *InstancePtr, u8 *SyncJumpWidth,
			   u8 *TimeSegment2, u8 *TimeSegment1)
{
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Value = XCan_ReadReg(InstancePtr->BaseAddress, XCAN_BTR_OFFSET);

	*TimeSegment1 = (u8) (Value & XCAN_BTR_TS1_MASK);
	*TimeSegment2 =
		(u8) ((Value & XCAN_BTR_TS2_MASK) >> XCAN_BTR_TS2_SHIFT);
	*SyncJumpWidth =
		(u8) ((Value & XCAN_BTR_SJW_MASK) >> XCAN_BTR_SJW_SHIFT);
}


/** @} */
