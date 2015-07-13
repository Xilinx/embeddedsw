/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xcanfd_config.c
* @addtogroup canfd_v1_0
* @{
*
* Functions in this file are CAN Configuration Register access related.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   nsk  06/04/15 First release
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
*
* This routine sets Baud Rate Prescaler value in Arbitration Phse.
* The system clock for the CAN controller is divided by (Prescaler + 1)
* to generate the quantum clock needed for sampling and synchronization.
*  Read the device specification for details.
*
* Baud Rate Prescaler could be set only after CAN device entered Configuration
* Mode. So please call XCanFd_EnterMode() to enter Configuration Mode before
* using this function.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Prescaler is the value to set. Valid values are from 0 to 255.
*
* @return	- XST_SUCCESS if the Baud Rate Prescaler value is set
*		successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
*
* @note		None.
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
	if (XCanFd_GetMode(InstancePtr) != XCANFD_MODE_CONFIG) {
		return XST_FAILURE;
	}

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_BRPR_OFFSET,
			   (u32) Prescaler);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This routine gets Baud Rate Prescaler value. The system clock for the CAN
* controller is divided by (Prescaler + 1) to generate the quantum clock
* needed for sampling and synchronization. Read the device specification for
* details.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	Current used Baud Rate Prescaler value. The value's range is
*		from 0 to 255.
*
* @note		None.
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
*
* This routine sets Bit time. Time segment 1, Time segment 2 and
* Synchronization Jump Width are set in this function. Device specification
* requires the values passed into this function be one less than the actual
* values of these fields. Read the device specification for details.
*
* Bit time could be set only after CAN device entered Configuration Mode.
* Please call XCanFd_EnterMode() to enter Configuration Mode before using this
* function.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	SyncJumpWidth is the Synchronization Jump Width value to set.
*		Valid values are from 0 to 3.
* @param	TimeSegment2 is the Time Segment 2 value to set. Valid values
*		are from 0 to 7.
* @param	TimeSegment1 is the Time Segment 1 value to set. Valid values
*		are from 0 to 15.
*
* @return	- XST_SUCCESS if the Bit time is set successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
*		- XST_INVALID_PARAM if any value of SyncJumpWidth, TimeSegment2
*		and TimeSegment1 is invalid.
*
* @note		None.
*
******************************************************************************/
int XCanFd_SetBitTiming(XCanFd *InstancePtr, u8 SyncJumpWidth,
			  u8 TimeSegment2, u8 TimeSegment1)
{
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (SyncJumpWidth > 16 || TimeSegment2 > 32 || TimeSegment1 > 64) {
		return XST_INVALID_PARAM;
	}

	/* Return error code if the device is NOT in Configuration Mode */
	if (XCanFd_GetMode(InstancePtr) != XCANFD_MODE_CONFIG) {
		return XST_FAILURE;
	}

	Value = ((u32) TimeSegment1) & XCANFD_BTR_TS1_MASK;
	Value |= (((u32) TimeSegment2) << XCANFD_BTR_TS2_SHIFT) &
		XCANFD_BTR_TS2_MASK;
	Value |= (((u32) SyncJumpWidth) << XCANFD_BTR_SJW_SHIFT) &
		XCANFD_BTR_SJW_MASK;

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_BTR_OFFSET, Value);
	Value = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_BTR_OFFSET);


	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This routine gets Bit time. Time segment 1, Time segment 2 and
* Synchronization Jump Width values are read in this function. According to
* device specification, the actual value of each of these fields is one
* more than the value read. Read the device specification for details.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	SyncJumpWidth will store the Synchronization Jump Width value
*		after this function returns. Its value ranges from 0 to 3.
* @param	TimeSegment2 will store the Time Segment 2 value after this
*		function returns. Its value ranges from 0 to 7.
* @param	TimeSegment1 will store the Time Segment 1 value after this
*		function returns. Its value ranges from 0 to 15.
*
* @return	None.
*
* @note		None.
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
*
* This routine sets Baud Rate Prescaler value in Data Phase.
*  The system clock for the CAN controller is divided by (Prescaler + 1)
*  to generate the quantum clock  needed for sampling and synchronization.
*  Read the device specification  for details.
*
* Baud Rate Prescaler could be set only after CAN device entered Configuration
* Mode. So please call XCanFd_EnterMode() to enter Configuration Mode before
* using this function.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Prescaler is the value to set. Valid values are from 1 to 256.
*
* @return	- XST_SUCCESS if the Baud Rate Prescaler value is set
*		successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
*
* @note		None.
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
	if (XCanFd_GetMode(InstancePtr) != XCANFD_MODE_CONFIG) {
		return XST_FAILURE;
	}
	RegValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_F_BRPR_OFFSET);
	RegValue |= ((u32) Prescaler & XCANFD_BRPR_BRP_MASK);
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
		XCANFD_F_BRPR_OFFSET,RegValue);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This routine gets Baud Rate Prescaler value in Data Phase. The system clock
*  for the CAN controller is divided by (Prescaler + 1) to generate the quantum
*  clock needed for sampling and synchronization. Read the device specification
*  for details.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	Current used Baud Rate Prescaler value. The value's range is
*		from 1 to 256.
*
* @note		None.
*
******************************************************************************/
u8 XCanFd_GetFBaudRatePrescaler(XCanFd *InstancePtr)
{
	u32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = (u8) XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_F_BRPR_OFFSET);

	return (Result&XCANFD_BRPR_BRP_MASK);
}

/*****************************************************************************/
/**
*
* This routine sets Bit time in Data Phase. Time segment 1, Time segment 2 and
* Synchronization Jump Width are set in this function. Device specification
* requires the values passed into this function be one less than the actual
* values of these fields. Read the device specification for details.
*
* Bit time could be set only after CAN device entered Configuration Mode.
* Please call XCanFd_EnterMode() to enter Configuration Mode before using this
* function.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	SyncJumpWidth is the Synchronization Jump Width value to set.
*		Valid values are from 0 to 3.
* @param	TimeSegment2 is the Time Segment 2 value to set. Valid values
*		are from 0 to 7.
* @param	TimeSegment1 is the Time Segment 1 value to set. Valid values
*		are from 0 to 15.
*
* @return	- XST_SUCCESS if the Bit time is set successfully.
*		- XST_FAILURE if CAN device is not in Configuration Mode.
* 		- XST_INVALID_PARAM if any value of SyncJumpWidth, TimeSegment2
*		and TimeSegment1 is invalid.
*
* @note		None.
*
******************************************************************************/
int XCanFd_SetFBitTiming(XCanFd *InstancePtr, u8 SyncJumpWidth,
			  u8 TimeSegment2, u8 TimeSegment1)
{
	u32 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	if (SyncJumpWidth > 3 || TimeSegment2 > 7 || TimeSegment1 > 15) {
		return XST_INVALID_PARAM;
	}

	/* Return error code if the device is NOT in Configuration Mode */
	if (XCanFd_GetMode(InstancePtr) != XCANFD_MODE_CONFIG) {
		return XST_FAILURE;
	}

	Value = ((u32) TimeSegment1) & XCANFD_F_BTR_TS1_MASK;
	Value |= (((u32) TimeSegment2) << XCANFD_F_BTR_TS2_SHIFT) &
		XCANFD_F_BTR_TS2_MASK;
	Value |= (((u32) SyncJumpWidth) << XCANFD_F_BTR_SJW_SHIFT) &
		XCANFD_F_BTR_SJW_MASK;

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_F_BTR_OFFSET,Value);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This routine gets Bit time in Data Phase. Time segment 1, Time segment 2 and
* Synchronization Jump Width values are read in this function. According to
* device specification, the actual value of each of these fields is one
* more than the value read. Read the device specification for details.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	SyncJumpWidth will store the Synchronization Jump Width value
*		after this function returns. Its value ranges from 0 to 3.
* @param	TimeSegment2 will store the Time Segment 2 value after this
*		function returns. Its value ranges from 0 to 7.
* @param	TimeSegment1 will store the Time Segment 1 value after this
*		function returns. Its value ranges from 0 to 15.
*
* @return	None.
*
* @note		None.
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
*
* This routine sets the Bit Rate Switch with nominal bit rate.
* if we set BRSD bit in Mode Select Register then CAN Controller transmits
* CAN FD Frames with Nominal Bit Rate.
* Read the device specification for details.
*
*  @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
*  @return	None.
*
*  @note	None.
*
* ****************************************************************************/
void XCanFd_SetBitRateSwitch_EnableNominal(XCanFd *InstancePtr)
{
	u32 Result;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_MSR_OFFSET);
	if (!(Result & XCANFD_SRR_CEN_MASK)) {
		Result = Result | XCANFD_MSR_BRSD_MASK;
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_MSR_OFFSET,Result);
	}
	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_MSR_OFFSET);
}

/*****************************************************************************/
/**
*
* This routine Disables the BRSD bit, so that Bit Rate Switch can be happen
* with Nominal or configured rate.
* Read the device specification for details.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	None.
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
	if (!(Result & XCANFD_SRR_CEN_MASK)) {
		Result = Result & (~XCANFD_MSR_BRSD_MASK);
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_MSR_OFFSET,Result);
	}
	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_MSR_OFFSET);
}
/** @} */
