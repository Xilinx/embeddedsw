/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcanfd.c
* @brief Implements the primary functions for the XCanFd driver.
*
* The XCanFd driver. Functions in this file are the minimum required functions
* for this driver. See xcanfd.h for a detailed description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   nsk  06/04/15 First release
* 1.0   nsk  15/05/15 Updated Correct AFRID and AFRMSK Registers.
*		      Modified DataSwaping when EDL is Zero.
*		      (CR 861772)
* 1.0	nsk  16/06/15 Updated XCanFd_Recv_Mailbox(), XCanFd_EnterMode(),
*		      XCanFd_GetMode() since RTL has changed.RTL Changes,Added
*		      new bits to MSR,SR,ISR,IER,ICR Registers and modified
*		      TS2 bits in BTR and F_SJW bits in F_BTR Registers.
* 1.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XCanFd_CfgInitialize API.
* 1.2   mi   09/22/16 Fixed compilation warnings.
* 2.1   ask  09/21/18 Added support for canfd 2.0 spec in PL canfd SoftIP.
*				  	  Added Api:XCanFd_Recv_Sequential
*								XCanFd_SeqRecv_logic
*								XCanFd_Recv_TXEvents_Sequential
*								XCanFd_GetNofMessages_Stored_TXE_FIFO
*								XCanFd_GetNofMessages_Stored_Rx_Fifo
*					  Fixed Message Queuing logic by modifying in functions
*					  XCanFd_Send_Queue, XCanFd_Addto_Queue, XCanFd_Send,
*					  and XCanFd_GetFreeBuffer. Added an static function
*					  XCanfd_TrrVal_Get_SetBit_Position.
*					  Added Macros regarding legacy API.
*		ask 09/27/18 Removed unnecessary register read from XCanFd_Send
*       ask  07/03/18 Fix for Sequential recv CR# 992606,CR# 1004222.
*       ask  08/27/18 Modified RecvSeq function to return XST_NO_DATA when the
*       		fifo fill levels are zero.
*		ask  08/08/18 Fixed Cppcheck warnings.
* 2.1   nsk  01/22/19 Pass correct fifo number to XCanFd_SeqRecv_logic()
*		      CR# 1018379
* 2.1	nsk  03/09/19 For CAN frames, DLC should always 8, even data bytes
*		      are greater than 8. CR# 1022045.
* 2.1	nsk  03/09/19 Fix for TrrMask to not to get written when using
		      XCanFd_Addto_Queue(), to send more than 32 buffers.
		      CR# 1022093
* 2.2   sn   06/11/19 Inactivating Mailbox RX buffers based on requirement.
* 2.3	sne  21/11/19 Used correct macro to access RX FIFO1 buffer.
* 2.3	sne  11/18/19 Fix for missing RX can packets on CANFD2.0.
* 2.3	sne  11/29/19 Fix for missing TX canfd packet while sending multiple packets
*		      by using multi buffer in loopback mode, CR# 1048366.
* 2.3  sne  12/18/19 Added Protocol Exception Event and BusOff event support.
* 2.3	sne  03/06/20 Fixed sending extra frames in XCanFd_Send_Queue API.
* 2.3	se   03/09/20 Initialize IsPl of config structure.
* 2.8	ht   06/19/23 Added support for system device-tree flow.
* 2.8	gm   06/22/23 Add support for request/release node.
* 2.10	ht   03/28/25 Remove Versal ES1 support as it is fixed in prod.
* 2.13  vn   07/30/26 Fix Doxygen warnings
* 2.13  vn   09/08/26 Align Doxygen comments with AMD standards
* 2.13  vn   09/30/26 Update Doxygen comments to AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup canfd Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xenv.h"
#include "xcanfd.h"
#if defined  (XPM_SUPPORT)
#include "pm_defs.h"
#include "pm_api_sys.h"
#include "pm_client.h"
#include "xstatus.h"
#include "xpm_init.h"
#endif
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

static void StubHandler(void);
static int XCanfd_TrrVal_Get_SetBit_Position(u32 u);
static u32 XCanFd_SeqRecv_logic(XCanFd *InstancePtr, u32 ReadIndex,
				u32 FsrVal, u32 *FramePtr, u8 fifo_no);

/************************** Global Variables ******************************/

/*****************************************************************************/
/**
*
* @brief Initializes a specific XCanFd instance/driver.
*
* This function should only be used when no Virtual Memory support is needed.
*
* This initialization entails:
* - Search for device configuration given the device ID.
* - Initialize Base Address field of the XCanFd structure using the device
*   address in the found device configuration.
* - Populate all other data fields in the XCanFd structure.
* - Reset the device.
*
* @param[out]   InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    ConfigPtr Pointer to XCanFd_Config instance.
* @param[in]    EffectiveAddr Base address of CANFD.
*
* @return	- XST_SUCCESS if initialization was successful.
* 		- XST_DEVICE_NOT_FOUND if device configuration information was
*		  not found for a device with the supplied device ID.
*
******************************************************************************/
int XCanFd_CfgInitialize(XCanFd *InstancePtr, XCanFd_Config *ConfigPtr,
			 UINTPTR EffectiveAddr)
{
	u32 FilterIndex;
#if defined  (XPM_SUPPORT)
	XStatus Status;
#endif

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

#if defined  (XPM_SUPPORT)
	Status = XPm_RequestNode(XpmGetNodeId((u64)EffectiveAddr), PM_CAP_ACCESS, MAX_QOS, REQUEST_ACK_BLOCKING);
	if (XST_SUCCESS != Status) {
		xil_printf("Canfd: XPm_RequestNode failed \r\n");
		return Status;
	}

	Status = XPm_ResetAssert(XpmGetResetId((u64)EffectiveAddr), XILPM_RESET_ACTION_RELEASE);
	if (XST_SUCCESS != Status) {
		xil_printf("Canfd: XPm_ResetAssert() ERROR=0x%x \r\n", Status);
		return Status;
	}
#endif

	/*
	 * Set some default values for instance data, don't indicate the device
	 * is ready to use until everything has been initialized successfully.
	 */
	InstancePtr->IsReady = 0;
	InstancePtr->CanFdConfig.BaseAddress = EffectiveAddr;
#ifdef SDT
	InstancePtr->CanFdConfig.Name = ConfigPtr->Name;
#else
	InstancePtr->CanFdConfig.DeviceId = ConfigPtr->DeviceId;
#endif
	InstancePtr->CanFdConfig.Rx_Mode = ConfigPtr->Rx_Mode;
	InstancePtr->CanFdConfig.NumofRxMbBuf = ConfigPtr->NumofRxMbBuf;
	InstancePtr->CanFdConfig.NumofTxBuf = ConfigPtr->NumofTxBuf;
#ifndef SDT
	InstancePtr->CanFdConfig.IsPl = ConfigPtr->IsPl;
#endif

	/*
	 * Set all handlers to stub values, let user configure this data later.
	 */
	InstancePtr->SendHandler = (XCanFd_SendRecvHandler) StubHandler;
	InstancePtr->RecvHandler = (XCanFd_SendRecvHandler) StubHandler;
	InstancePtr->ErrorHandler = (XCanFd_ErrorHandler) StubHandler;
	InstancePtr->EventHandler = (XCanFd_EventHandler) StubHandler;

	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	/* Reset the device to get it into its initial state. */
	XCanFd_Reset(InstancePtr);
	/* Update all AFID and AFMASK registers with Zero */
	for (FilterIndex = 1; FilterIndex <= MAX_FILTER_INDEX; FilterIndex++) {
		if (XCanFd_AcceptFilterSet(InstancePtr, FilterIndex,
					   (u32)0, (u32)0) != (s32)0) {
			return (s32)XST_FAILURE;
		}
	}

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Releases resources of the XCanFd instance/driver.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return	- XST_SUCCESS if node release was successful.
*		- XST_FAILURE or an error code or a reason code
*		  if node release failed.
*
******************************************************************************/
int XCanFd_stop(XCanFd *InstancePtr)
{
	int Status = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);

#if defined (XPM_SUPPORT)
	Status = XPm_ReleaseNode(XpmGetNodeId((u64)InstancePtr->CanFdConfig.BaseAddress));
	if(Status != XST_SUCCESS) {
		xil_printf("Canfd: XPm_ReleaseNode failed with reason 0x%x \r\n", Status);
	}
	return Status;
#else
	return Status;
#endif
}

/*****************************************************************************/
/**
*
* @brief Returns enabled acceptance filters.
*
* Use XCANFD_AFR_UAF*_MASK defined in xcanfd_hw.h to interpret the returned
* value. If no acceptance filters are enabled then all received frames are
* stored in the RX FIFO.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return	The value stored in Acceptance Filter Register.
*
* @note		Acceptance Filter Register is an optional register in Xilinx
*               CAN device. If it is NOT existing in the device, this function
*               should NOT be used. Calling this function in this case
*               causes an assertion failure.
*
******************************************************************************/
u32 XCanFd_AcceptFilterGetEnabled(XCanFd *InstancePtr)
{
	u32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_AFR_OFFSET);
	return Result;
}

/****************************************************************************/
/**
*
* @brief Returns the current operation mode of the CAN device.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return	- XCANFD_MODE_CONFIG if the device is in Configuration Mode.
* 		- XCANFD_MODE_SLEEP if the device is in Sleep Mode.
*		- XCANFD_MODE_NORMAL if the device is in Normal Mode.
*		- XCANFD_MODE_LOOPBACK if the device is in Loop Back Mode.
*		- XCANFD_MODE_SNOOP if the device is in Snoop Mode.
*		- XCANFD_MODE_BR if the device is in Bus-Off recovery Mode.
*		- XCANFD_MODE_PEE if the device is in Protocol Exception
*		  Event mode.
*
*****************************************************************************/
u8 XCanFd_GetMode(XCanFd *InstancePtr)
{
	u32 Value;
	u32 Mode;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Value = XCanFd_GetStatus(InstancePtr);

	if ((Value & XCANFD_SR_CONFIG_MASK) != (u32)0) {
		Mode = XCANFD_MODE_CONFIG;
	} else if ((Value & XCANFD_SR_SLEEP_MASK) != (u32)0) {
		Mode = XCANFD_MODE_SLEEP;
	} else if ((Value & XCANFD_SR_NORMAL_MASK) != (u32)0) {
		if ((Value & XCANFD_SR_SNOOP_MASK) != (u32)0) {
			Mode = XCANFD_MODE_SNOOP;
		} else {
			Mode = XCANFD_MODE_NORMAL;
		}
	}

	else {
		/* If this line is reached, the device is in Loop Back Mode. */
		Mode = XCANFD_MODE_LOOPBACK;
	}
	if ((Value & XCANFD_SR_PEE_CONFIG_MASK) != (u32)0) {
		Mode = Mode | (u32)XCANFD_MODE_PEE;
	}

	return (u8)Mode;
}

/*****************************************************************************/
/**
*
* @brief Enters the specified CAN operation mode.
*
* Allows the CAN device to enter one of the following operation modes:
*
* - Configuration Mode: Pass in parameter XCANFD_MODE_CONFIG
* - Sleep Mode: Pass in parameter XCANFD_MODE_SLEEP
* - Normal Mode: Pass in parameter XCANFD_MODE_NORMAL
* - Loop Back Mode: Pass in parameter XCANFD_MODE_LOOPBACK
* - Snoop Mode: Pass in Parameter XCANFD_MODE_SNOOP
* - Auto Bus-Off Recovery Mode: Pass in Parameter XCANFD_MODE_ABR
* - Start Bus-Off Recovery Mode: Pass in Parameter XCANFD_MODE_SBR
* - Protocol Exception Event Mode: Pass in Parameter XCANFD_MODE_PEE
* - Disable AutoRetransmission Mode: Pass in Parameter XCANFD_MODE_DAR
*
* Read xcanfd.h and device specification for detailed description of each
* operation mode.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    OperationMode Specifies which operation mode to enter. Valid
*		value is any of XCANFD_MODE_* defined in xcanfd.h. Note that no
*		multiple modes could be entered at the same time.
*
* @note		This function does NOT ensure CAN device enters the specified
*		operation mode before returns the control to the caller.
*		The caller is responsible for checking current operation mode
*		using XCanFd_GetMode().
*
******************************************************************************/
void XCanFd_EnterMode(XCanFd *InstancePtr, u8 OperationMode)
{
	u8 CurrentMode;
	u32 MsrReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid((OperationMode == (u32)XCANFD_MODE_CONFIG) ||
		       (OperationMode == (u32)XCANFD_MODE_SLEEP) ||
		       (OperationMode == (u32)XCANFD_MODE_NORMAL) ||
		       (OperationMode == (u32)XCANFD_MODE_LOOPBACK) ||
		       (OperationMode == (u32)XCANFD_MODE_SNOOP) ||
		       (OperationMode == (u32)XCANFD_MODE_PEE) ||
		       (OperationMode == (u32)XCANFD_MODE_ABR) ||
		       (OperationMode == (u32)XCANFD_MODE_DAR) ||
		       (OperationMode == (u32)XCANFD_MODE_SBR));

	CurrentMode = XCanFd_GetMode(InstancePtr);
	MsrReg = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_MSR_OFFSET) & XCANFD_MSR_CONFIG_MASK;
	if ((CurrentMode == (u8)XCANFD_MODE_NORMAL) &&
	    (OperationMode == (u8)XCANFD_MODE_SLEEP)) {

		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_MSR_OFFSET,
				(XCANFD_MSR_SLEEP_MASK | MsrReg));
		return;
	}

	else if ((CurrentMode == (u8)XCANFD_MODE_SLEEP) &&
		 (OperationMode == (u8)XCANFD_MODE_NORMAL)) {
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_MSR_OFFSET, MsrReg);
		return;
	}

	/*
	 * If the mode transition is not any of the two cases above, CAN must
	 * enter Configuration Mode before switching into the target operation
	 * mode.
	 */
	else {
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_SRR_OFFSET, 0);
	}

	/*
	 * Check if the device has entered Configuration Mode, if not, return
	 * to the caller.
	 */
	if (XCanFd_GetMode(InstancePtr) != (u8)XCANFD_MODE_CONFIG) {
		return;
	}

	switch (OperationMode) {
		case XCANFD_MODE_CONFIG:
			break;

		case XCANFD_MODE_SLEEP:

			/* Switch the device into Sleep Mode */
			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_MSR_OFFSET,
					(XCANFD_MSR_SLEEP_MASK | MsrReg));
			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_SRR_OFFSET, XCANFD_SRR_CEN_MASK);
			break;

		case XCANFD_MODE_NORMAL:

			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_MSR_OFFSET, MsrReg);
			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_SRR_OFFSET, XCANFD_SRR_CEN_MASK);
			break;

		case XCANFD_MODE_SNOOP:

			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_MSR_OFFSET, (XCANFD_MSR_SNOOP_MASK |
							    MsrReg));
			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_SRR_OFFSET, XCANFD_SRR_CEN_MASK);
			break;

		case XCANFD_MODE_ABR:

			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_MSR_OFFSET, (XCANFD_MSR_ABR_MASK |
							    MsrReg));
			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_SRR_OFFSET, XCANFD_SRR_CEN_MASK);

			break;

		case XCANFD_MODE_SBR:

			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_MSR_OFFSET, (XCANFD_MSR_SBR_MASK |
							    MsrReg));
			break;

		case XCANFD_MODE_PEE:

			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_MSR_OFFSET, (XCANFD_MSR_DPEE_MASK |
							    MsrReg));
			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_SRR_OFFSET, XCANFD_SRR_CEN_MASK);

			break;


		case XCANFD_MODE_DAR:

			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_MSR_OFFSET, (XCANFD_MSR_DAR_MASK |
							    MsrReg));
			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_SRR_OFFSET, XCANFD_SRR_CEN_MASK);

			break;

		default:
			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_MSR_OFFSET,
					(XCANFD_MSR_LBACK_MASK | MsrReg));
			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_SRR_OFFSET, XCANFD_SRR_CEN_MASK);
			break;
	}
}

/*****************************************************************************/
/**
*
* @brief Reads Receive and Transmit error counters.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[out]   RxErrorCount Contains Receive Error Counter value after
*		this function returns.
* @param[out]   TxErrorCount Contains Transmit Error Counter value after
*		this function returns.
*
******************************************************************************/
void XCanFd_GetBusErrorCounter(XCanFd *InstancePtr, u8 *RxErrorCount,
			       u8 *TxErrorCount)
{
	u32 Result;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_ECR_OFFSET);

	*RxErrorCount = (u8)((Result & XCANFD_ECR_REC_MASK) >> XCANFD_ECR_REC_SHIFT);
	*TxErrorCount = (u8)(Result & XCANFD_ECR_TEC_MASK);
}

/*****************************************************************************/
/**
*
* @brief Sends a CAN/CANFD Frame.
*
* Checks whether a free buffer is available. If a free buffer exists, the
* supplied data is written into it. Otherwise returns an error code
* immediately. Does
* not wait for the given frame to be sent to the CAN bus.
*
* @param[in,out] InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]     FramePtr Pointer to a 32-bit aligned buffer containing the
*                CAN frame to be sent.
* @param[out]    TxBufferNumber Buffer where the supplied data has been
*                written and it is updated by driver.
*
* @return        - XST_SUCCESS if TX FIFO was not full and the given frame was
*                  written into the FIFO.
*                - XST_FIFO_NO_ROOM if there is no room in the TX FIFO for the
*                  given frame.
*
*****************************************************************************/
int XCanFd_Send(XCanFd *InstancePtr, u32 *FramePtr, u32 *TxBufferNumber)
{
	u32 FreeTxBuffer;
	u32 TrrVal;
	u32 DwIndex = 0;
	u32 Value;
	u32 Dlc;
	u32 Len;
	u32 CanEDL;
	u32 OutValue;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Poll TRR to check pending transmission requests */

	if (InstancePtr->GlobalTrrMask == (u32)0) {
		InstancePtr->GlobalTrrMask = TRR_MASK_INIT_VAL;
	}
	TrrVal = XCanFd_GetFreeBuffer(InstancePtr);
	if (TrrVal == (u32)0) {
		return (s32)XST_FIFO_NO_ROOM;
	}

	TrrVal = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_TRR_OFFSET);
	Value = (~TrrVal) & InstancePtr->GlobalTrrMask;
	Value = XCanFD_Check_TrrVal_Set_Bit(Value);
	FreeTxBuffer =  (u32)XCanfd_TrrVal_Get_SetBit_Position(Value);

	/* Write ID to ID Register */
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_TXID_OFFSET(FreeTxBuffer), FramePtr[0]);

	/* Write DLC to DLC Register */
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_TXDLC_OFFSET(FreeTxBuffer), FramePtr[1]);
	CanEDL = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXDLC_OFFSET(FreeTxBuffer));

	if ((CanEDL & XCANFD_DLCR_EDL_MASK) != (u32)0) {

		/* CAN FD Frames. */
		Dlc = (u32)XCanFd_GetDlc2len(FramePtr[1] &
					     XCANFD_DLCR_DLC_MASK,
					     (CanEDL & XCANFD_DLCR_EDL_MASK));
		/* Write Data to Data Register */

		for (Len = (u32)0; Len < Dlc; Len += (u32)4) {
			OutValue = Xil_EndianSwap32(
					   FramePtr[(u32)2 + DwIndex]);
			XCanFd_WriteReg(
				InstancePtr->CanFdConfig.BaseAddress,
				(XCANFD_TXDW_OFFSET(FreeTxBuffer) +
				 (DwIndex * XCANFD_DW_BYTES)), OutValue);
			DwIndex++;
		}
	} else {

		/* Legacy CAN Frames */
		Dlc = (u32)XCanFd_GetDlc2len(FramePtr[1] &
					     XCANFD_DLCR_DLC_MASK,
					     (CanEDL & XCANFD_DLCR_EDL_MASK));
		for (Len = (u32)0; Len < Dlc; Len += (u32)4) {
			OutValue = Xil_EndianSwap32(
					   FramePtr[(u32)2 + DwIndex]);
			XCanFd_WriteReg(
				InstancePtr->CanFdConfig.BaseAddress,
				(XCANFD_TXDW_OFFSET(FreeTxBuffer) +
				 (Len)), OutValue);
			DwIndex++;
		}
	}

	Value = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			       XCANFD_TRR_OFFSET);
	Value |= ((u32)1 << FreeTxBuffer);
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_TRR_OFFSET, Value);

	/* Assign buffer number to user */
	*TxBufferNumber = FreeTxBuffer;

	/* Make That buffer as transmitted */

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Writes data into a specific TX buffer.
*
* There are 32 TX buffers. Data can be added to each buffer using this function.
* This function does not transmit the data; it only adds data to the buffers.
*
* @param[in,out] InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]     FramePtr Pointer to a 32-bit aligned buffer containing the
*                CAN frame to be sent.
* @param[out]    TxBufferNumber Buffer Number where the data has been written
*                and is given back to user.
*
* @return        - XST_SUCCESS if TX FIFO was not full and the given frame was
*                  written into the FIFO.
*                - XST_FIFO_NO_ROOM if there is no room in the TX FIFO for the
*                  given frame.
*
******************************************************************************/
int XCanFd_Addto_Queue(XCanFd *InstancePtr, u32 *FramePtr, u32 *TxBufferNumber)
{
	u32 FreeTxBuffer;
	u32 TrrVal;
	u32 MaskValue;
	u32 DwIndex = 0;
	u32 Len;
	u32 Dlc;
	u32 CanEDL;
	u32 OutValue;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);


	TrrVal = XCanFd_GetFreeBuffer(InstancePtr);
	if (TrrVal == (u32)0) {
		return (s32)XST_FIFO_NO_ROOM;
	}

	if (InstancePtr->GlobalTrrMask == (u32)0) {
		InstancePtr->GlobalTrrMask = TRR_MASK_INIT_VAL;
	}
	TrrVal = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_TRR_OFFSET);
	MaskValue = (~TrrVal) & InstancePtr->GlobalTrrMask ;
	InstancePtr->GlobalTrrValue = XCanFD_Check_TrrVal_Set_Bit(MaskValue);
	FreeTxBuffer =  (u32)XCanfd_TrrVal_Get_SetBit_Position(InstancePtr->GlobalTrrValue);
	InstancePtr->GlobalTrrMask ^= InstancePtr->GlobalTrrValue;


	InstancePtr->MultiBuffTrr = ~(InstancePtr->GlobalTrrMask);

	/* Write ID to ID Register*/
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_TXID_OFFSET(FreeTxBuffer), FramePtr[0]);

	/* Write DLC to DLC Register*/
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_TXDLC_OFFSET(FreeTxBuffer), FramePtr[1]);

	CanEDL = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXDLC_OFFSET(FreeTxBuffer));

	Dlc = (u32)XCanFd_GetDlc2len(FramePtr[1] & XCANFD_DLCR_DLC_MASK,
				     (CanEDL & XCANFD_DLCR_EDL_MASK));

	if ((CanEDL & XCANFD_DLCR_EDL_MASK) != (u32)0) {

		/* CAN FD Frames */
		for (Len = (u32)0; Len < Dlc; Len += (u32)4) {
			OutValue = Xil_EndianSwap32(
					   FramePtr[(u32)2 + DwIndex]);
			XCanFd_WriteReg(
				InstancePtr->CanFdConfig.BaseAddress,
				(XCANFD_TXDW_OFFSET(
					 FreeTxBuffer) + (DwIndex *
							  XCANFD_DW_BYTES)), OutValue);
			DwIndex++;
		}
	}

	else {

		/* Legacy CAN Frames */
		for (Len = (u32)0; Len < Dlc; Len += (u32)4) {
			OutValue = Xil_EndianSwap32(
					   FramePtr[(u32)2 + DwIndex]);
			XCanFd_WriteReg(
				InstancePtr->CanFdConfig.BaseAddress,
				(XCANFD_TXDW_OFFSET(
					 FreeTxBuffer) + (Len)),
				OutValue);
			DwIndex++;
		}
	}
	/* Assign  Buffer to user */
	*TxBufferNumber = FreeTxBuffer;


	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Receives a CAN/CAN FD Frame in sequential mode.
*
* Checks the FSR Register. The FL bits indicate the number of packets
* received. If FL is non-zero, reads the packet and stores it to the supplied
* buffer.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[out]   FramePtr Pointer to a 32-bit aligned buffer where the
*		CAN/CAN FD frame is to be written.
*
* @return	- XST_SUCCESS if RX FIFO was not empty and a frame was read from
*		  RX FIFO successfully and written into the given buffer.
*		- XST_NO_DATA if there is no frame to be received from the FIFO.
*
* @note         The CANFD has two design modes.
*               ->Sequential Mode - Core writes data sequentially to
*                 RxBuffers.
*               ->MailBox Mode - Core writes data to RxBuffers when a ID Match
*                 happened.
* 		This routine distinguishes receive checking as per the frame
*               availability from either Fifo 0 or Fifo 1.
*
******************************************************************************/
u32 XCanFd_Recv_Sequential(XCanFd *InstancePtr, u32 *FramePtr)
{
	u32 Result;
	u32 ReadIndex = 0;
	u32 Status = (u32)XST_NO_DATA;
	u8  FifoNo = 0xFF;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_FSR_OFFSET);

	if (((Result & XCANFD_FSR_FL_MASK) != (u32)0) ||
	    ((Result & XCANFD_FSR_FL_1_MASK) != (u32)0)) {

		/* Check for the Packet Available by reading FSR Register */
		if ((Result & XCANFD_FSR_FL_MASK) != (u32)0) {
			/*Fill the canfd frame for current RI value for Fifo 0 */

			ReadIndex = Result & XCANFD_FSR_RI_MASK;
			FifoNo = XCANFD_RX_FIFO_0;
		}

		if ((Result & XCANFD_FSR_FL_1_MASK) != (u32)0) {
			/*Fill the canfd frame for current RI value for Fifo 1 */

			ReadIndex = ((Result & XCANFD_FSR_RI_1_MASK)
				     >> XCANFD_FSR_RI_1_SHIFT);
			FifoNo = XCANFD_RX_FIFO_1;
		}

		Status = XCanFd_SeqRecv_logic(InstancePtr, ReadIndex, Result,
					      FramePtr, FifoNo);
	}
	return Status;

}

/*****************************************************************************/
/**
*
* @brief Receives CAN/CAN FD TX Events in sequential mode.
*
* Checks the FSR Register. The FL bits indicate the number of TX Event packets
* received. If FL is non-zero, reads the packet and stores it to the supplied
* buffer.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[out]   FramePtr Pointer to a 32-bit aligned buffer where the
*		CAN/CAN FD frame Event is to be written.
*
* @return       - XST_SUCCESS if TXE FIFO was not empty and a frame was read
*                 from TX FIFO successfully and written into the given buffer.
*               - XST_NO_DATA if there is no frame to be received from the TXE
*                 FIFO.
*
* @note		This CANFD has two design modes.
*		->Sequential Mode - Core writes data sequentially to RxBuffers.
*               ->MailBox Mode - Core writes data to RxBuffers when a ID Match
*                 happened.
*               This routine is useful for Both the Modes. This API is meant to
*                 be used with IP with CanFD 2.0 spec support only.
*
******************************************************************************/
u32 XCanFd_Recv_TXEvents_Sequential(XCanFd *InstancePtr, u32 *FramePtr)
{
	u32 ReadIndex;
	u32 Result;
	u32 CanEDL;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXE_FSR_OFFSET);

	/* Check for the Packet Availability by reading FSR Register */
	if ((Result & XCANFD_TXE_FL_MASK) != (u32)0) {
		ReadIndex = Result & XCANFD_TXE_RI_MASK;

		/* Read ID from ID Register*/
		FramePtr[0] = XCanFd_ReadReg(
				      InstancePtr->CanFdConfig.BaseAddress,
				      XCANFD_TXEID_OFFSET(ReadIndex));

		/* Read DLC from DLC Register*/
		CanEDL = XCanFd_ReadReg(
				 InstancePtr->CanFdConfig.BaseAddress,
				 XCANFD_TXEDLC_OFFSET(ReadIndex));
		FramePtr[1] = CanEDL;

		/* Set the IRI bit causes core to increment RI in FSR Register */
		Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_TXE_FSR_OFFSET);
		Result |= XCANFD_TXE_IRI_MASK;
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXE_FSR_OFFSET, Result);
		Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_TXE_FSR_OFFSET);

		return XST_SUCCESS;
	} else {
		return XST_NO_DATA;
	}
}

/*****************************************************************************/
/**
*
* @brief Receives a CAN Frame in Mailbox Mode.
*
* Reads the Rx Last Buffer Index from the ISR Register. This indicates which
* buffer contains data. Then reads and updates the data to the supplied buffer.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[out]   FramePtr Pointer to a 32-bit aligned buffer where the CAN
*		frame is to be received.
*
* @return	- XST_SUCCESS if RX FIFO was not empty and a frame was read from
*		  RX FIFO successfully and written into the given buffer.
*		- XST_NO_DATA if there is no frame to be received from the FIFO.
*
* @note		This CANFD has two design modes.
*		->Sequential Mode - Core writes data sequentially to RxBuffers.
*		->MailBox Mode	  - Core writes data to RxBuffers when a ID
*                 match happened.
*		This routine is useful for MailBox Mode.
*
******************************************************************************/
u32 XCanFd_Recv_Mailbox(XCanFd *InstancePtr, u32 *FramePtr)
{
	u32 DwIndex = 0;
	u32 Result;
	u32 CanEDL;
	u32 Dlc;
	u32 Len;
	u32 RcsRegNr = 0;
	u32 RxBufferIndex;
	u32 CoreStatusBit;
	u32 Mask;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RxBufferIndex = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				       XCANFD_ISR_OFFSET) &
			XCANFD_IXR_RXLRM_BI_MASK;
	RxBufferIndex >>= XCANFD_RXLRM_BI_SHIFT;
	CoreStatusBit = (RxBufferIndex % XCANFD_CSB_SHIFT) + XCANFD_CSB_SHIFT;
	RcsRegNr = RxBufferIndex / XCANFD_CSB_SHIFT;

	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RCS_OFFSET(RcsRegNr));
	Mask = Result & XCANFD_RCS_HCB_MASK;
	if ((Result & ((u32)1 << CoreStatusBit)) != (u32)0) {
		/* Read CanFd ID*/
		FramePtr[0] = XCanFd_ReadReg(InstancePtr->CanFdConfig.
					     BaseAddress, XCANFD_RXID_OFFSET(RxBufferIndex));

		/* Read CanFd DLC */
		CanEDL = XCanFd_ReadReg(InstancePtr->CanFdConfig.
					BaseAddress,
					XCANFD_RXDLC_OFFSET(RxBufferIndex));
		FramePtr[1] = CanEDL;

		Dlc = (u32)XCanFd_GetDlc2len(FramePtr[1] &
					     XCANFD_DLCR_DLC_MASK,
					     (CanEDL & XCANFD_DLCR_EDL_MASK));
		/* A CanFD Frame is received */

		if ((CanEDL & XCANFD_DLCR_EDL_MASK) != (u32)0) {

			/* Read all Bytes from DW Register */

			for (Len = (u32)0; Len < Dlc; Len += (u32)4) {
				FramePtr[(u32)2 + DwIndex] = Xil_EndianSwap32(
								     XCanFd_ReadReg(
										     InstancePtr->CanFdConfig.
										     BaseAddress,
										     (XCANFD_RXDW_OFFSET(
												     RxBufferIndex)
												     + (DwIndex * XCANFD_DW_BYTES))));
				DwIndex++;
			}
		} else {
			/* Legacy CAN Frame */
			for (Len = (u32)0; Len < Dlc; Len += (u32)4) {
				FramePtr[(u32)2 + DwIndex] = Xil_EndianSwap32(
								     XCanFd_ReadReg(
										     InstancePtr->CanFdConfig.
										     BaseAddress,
										     (XCANFD_RXDW_OFFSET(
												     RxBufferIndex) +
												     (Len))));
				DwIndex++;
			}
		}
		/* Clear CSB Bit of RCS Register */
		Result = Mask | ((u32)1 << CoreStatusBit);
		XCanFd_WriteReg(InstancePtr->CanFdConfig.
				BaseAddress, XCANFD_RCS_OFFSET(RcsRegNr), Result);
		return XST_SUCCESS;
	} else {
		return XST_NO_DATA;
	}
}

/*****************************************************************************/
/**
*
* @brief Sets an RxBuffer to Active State.
*
* In Mailbox Mode configuration each buffer can be set to receive with a
* specific Id and Mask. The buffer must first be activated before comparison.
* Maximum number of RxBuffers depends on Design. Range 48, 32, 16.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    RxBuffer Receive Buffer Number that defines which
*		Buffer to configure. Value ranges from 0 - 48.
*
* @return	- XST_SUCCESS if the values were set successfully.
*
******************************************************************************/
u32 XCanFd_RxBuff_MailBox_Active(XCanFd *InstancePtr, u32 RxBuffer)
{
	u32 Status = 0;
	u32 NoCtrlStatus;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(RxBuffer < InstancePtr->CanFdConfig.NumofRxMbBuf);

	if (RxBuffer <= DESIGN_RANGE_1) {
		NoCtrlStatus = CONTROL_STATUS_1;
	} else if (RxBuffer <= DESIGN_RANGE_2) {
		RxBuffer -= (DESIGN_RANGE_1 + (u32)1);
		NoCtrlStatus = CONTROL_STATUS_2;
	} else {
		RxBuffer -= (DESIGN_RANGE_2 + (u32)1);
		NoCtrlStatus = CONTROL_STATUS_3;
	}
	Status = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RCS_OFFSET(NoCtrlStatus));
	if ((Status & ((u32)1 << RxBuffer)) != (u32)0) {
		return 1;
	}
	Status |= ((u32)1 << RxBuffer);
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_RCS_OFFSET(NoCtrlStatus), Status);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Sets the Id and Mask for an RxBuffer for Id matching.
*
* If a packet is received with an id equal to the configured id, it is stored
* in the RxBuffer. Otherwise it is not stored.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    RxBuffer Receive Buffer Number that defines which
*		Buffer to configure. Value ranges from 0 - 48
*		(can get from NumofRxMbBuf).
* @param[in]    MaskValue Value to write into the RxBuffer Mask Register.
* @param[in]    IdValue Value to write into the RxBuffer Id register.
*
* @return	- XST_SUCCESS if the values were set successfully.
*
******************************************************************************/
u32 XCanFd_Set_MailBox_IdMask(XCanFd *InstancePtr, u32 RxBuffer,
			      u32 MaskValue, u32 IdValue)
{
	u32 Status = 0;
	u32 NoCtrlStatus;
	u32 BufferNr = RxBuffer;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(RxBuffer < InstancePtr->CanFdConfig.NumofRxMbBuf);

	if (RxBuffer <= DESIGN_RANGE_1) {
		NoCtrlStatus = CONTROL_STATUS_1;
	} else if (RxBuffer <= DESIGN_RANGE_2) {
		RxBuffer -= (DESIGN_RANGE_1 + (u32)1);
		NoCtrlStatus = CONTROL_STATUS_2;
	} else {
		RxBuffer -= (DESIGN_RANGE_2 + (u32)1);
		NoCtrlStatus = CONTROL_STATUS_3;
	}

	Status = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RCS_OFFSET(NoCtrlStatus));

	if ((Status & ((u32)1 << RxBuffer)) != (u32)0) {
		Status &= ~((u32)1 << RxBuffer);
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RCS_OFFSET(NoCtrlStatus), Status);
	}

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_MAILBOX_MASK_OFFSET(BufferNr), MaskValue);
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_MAILBOX_ID_OFFSET(BufferNr), IdValue);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Sets an RxBuffer to Inactive State.
*
* If a buffer is changed to Inactive state, Rx packets are not stored into
* that buffer, even if the Id is matched.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    RxBuffer Receive Buffer Number that defines which
*		Buffer to configure. Value ranges from 0 - 48.
*
* @return	- XST_SUCCESS if the values were set successfully.
*		- XST_FAILURE if the given filter was not disabled, or the CAN
*		  device was not ready to accept writes to AFMR and AFIR.
*
******************************************************************************/
u32 XCanFd_RxBuff_MailBox_DeActive(XCanFd *InstancePtr, u32 RxBuffer)
{
	u32 Status;
	u8 NoCtrlStatus;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(RxBuffer < InstancePtr->CanFdConfig.NumofRxMbBuf);

	if (RxBuffer <= DESIGN_RANGE_1) {
		NoCtrlStatus = CONTROL_STATUS_1;
	} else if (RxBuffer <= DESIGN_RANGE_2) {
		RxBuffer -= (DESIGN_RANGE_1 + (u32)1);
		NoCtrlStatus = CONTROL_STATUS_2;
	} else {
		RxBuffer -= (DESIGN_RANGE_2 + (u32)1);
		NoCtrlStatus = CONTROL_STATUS_3;
	}

	Status = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RCS_OFFSET(NoCtrlStatus));
	if ((Status & ((u32)1 << RxBuffer)) != (u32)0) {
		Status &= ((~((u32)1 << RxBuffer)) & (u32)XCANFD_MBRXBUF_MASK);
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RCS_OFFSET(NoCtrlStatus), Status);
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Cancels a CAN/CAN FD Frame transmission request.
*
* Checks the TRR Bit based on BufferNumber. If the TRR Bit is set, cancels
* the buffer transmission.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    BufferNumber Which Buffer to cancel out of 32 Buffers.
*
* @return	- XST_SUCCESS if cancellation was successful.
*		- XST_FAILURE if the buffer has no pending transmission.
*
* @note         This function first checks whether the TRR bit is set.
*               - If it is set, the function checks the corresponding TCR bit.
*               - If the TCR bit is set, the function waits until cancellation
*                 is performed.
*               - If the TCR bit is not set, the function sets it and waits
*                 until the core clears it.
*               - If the TRR bit is not set, the function does nothing.
*
******************************************************************************/
int XCanFd_TxBuffer_Cancel_Request(XCanFd *InstancePtr, u32 BufferNumber)
{
	u32 RegVal;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (BufferNumber < MAX_BUFFER_INDEX) {
		if ((XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				    XCANFD_TRR_OFFSET) & ((u32)1 << BufferNumber))
		    != (u32)0) {
			if (((XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					     XCANFD_TCR_OFFSET) & ((u32)1 << BufferNumber)) != (u32)0)) {

				/*
				 * Already Cancellation request is in process
				 * Host should wait until it core clears it.
				 */
				RegVal = XCanFd_ReadReg(
						 InstancePtr->CanFdConfig.BaseAddress,
						 XCANFD_TCR_OFFSET);
				while ((RegVal & ((u32)1 << BufferNumber)) != (u32)0) {
					RegVal = XCanFd_ReadReg(
							 InstancePtr->CanFdConfig.BaseAddress,
							 XCANFD_TCR_OFFSET);
				}

				return (s32)XST_SUCCESS;
			} else {

				/*
				 * buffer has pending transmission request but no pending
				 * cancellation request
				 */
				XCanFd_WriteReg(InstancePtr->CanFdConfig.
						BaseAddress, XCANFD_TCR_OFFSET, (u32)1 << BufferNumber);

				RegVal = XCanFd_ReadReg(
						 InstancePtr->CanFdConfig.BaseAddress,
						 XCANFD_TCR_OFFSET);
				while ((RegVal & ((u32)1 << BufferNumber)) != (u32)0) {
					RegVal = XCanFd_ReadReg(
							 InstancePtr->CanFdConfig.BaseAddress,
							 XCANFD_TCR_OFFSET);
				}

				return (s32)XST_SUCCESS;
			}
		}

		return (s32)XST_FAILURE;
	} else {
		return (s32)XST_FAILURE;
	}

}

/*****************************************************************************/
/**
* @brief Enables the acceptance filters. Up to 32 filters can be enabled.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    FilterIndexMask Specifies which filters to enable. Use
*               any XCANFD_AFR_UAF*_MASK to enable one filter, and combine
*               multiple XCANFD_AFR_UAF*_MASK values with a bitwise OR to
*               enable multiple filters. Any filter not specified in this
*               parameter keeps its previous enable or disable setting.
*
* @note         In Sequential Mode, enable these filters to receive data. To
*               filter by Id match, set the Id value in the AFR Id register.
*
******************************************************************************/
void XCanFd_AcceptFilterEnable(XCanFd *InstancePtr, u32 FilterIndexMask)
{
	u32 EnabledFilters;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	EnabledFilters =
		XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			       XCANFD_AFR_OFFSET);

	EnabledFilters |= FilterIndexMask;

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_AFR_OFFSET,
			EnabledFilters);

}

/*****************************************************************************/
/**
*
* @brief Disables the acceptance filters. 32 filters can be disabled.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    FilterIndexMask Specifies which filters to disable. Use
*               any XCANFD_AFR_UAF*_MASK to disable one filter, and combine
*               multiple XCANFD_AFR_UAF*_MASK values with a bitwise OR to
*               disable multiple filters. Any filter not specified in this
*               parameter keeps its previous enable or disable setting. If all
*               acceptance filters are disabled then all received frames are
*               stored in the RX FIFO.
*
******************************************************************************/
void XCanFd_AcceptFilterDisable(XCanFd *InstancePtr, u32 FilterIndexMask)
{
	u32 EnabledFilters;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	EnabledFilters =
		XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			       XCANFD_AFR_OFFSET);

	EnabledFilters &= (~FilterIndexMask);

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_AFR_OFFSET,
			EnabledFilters);

}

/*****************************************************************************/
/**
*
* @brief Sets values for the Acceptance Filter Mask and ID Registers.
*
* Sets values to the Acceptance Filter Mask Register (AFMR) and
* Acceptance Filter ID Register (AFIR) for the specified Acceptance Filter.
* Use XCANFD_IDR_* defined in xcanfd_hw.h to create the values to set the
* filter. Read xcanfd.h and device specification for details.
*
* This function should be called only after:
*   - The given filter is disabled by calling XCanFd_AcceptFilterDisable().
*   - And the CAN device is ready to accept writes to AFMR and AFIR, that is,
*     XCanFd_IsAcceptFilterBusy() returns FALSE.
*
* @param[in]   InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]   FilterIndex Defines which Acceptance Filter Mask and ID Register
*              to set. Use any single XCANFD_AFR_UAF*_MASK value. Ranges
*              from 1 - 32.
* @param[in]   MaskValue Value to write to the chosen Acceptance Filter
*              Mask Register.
* @param[in]   IdValue Value to write to the chosen Acceptance Filter
*              ID Register.
*
* @return      - XST_SUCCESS if the values were set successfully.
*              - XST_FAILURE if the given filter was not disabled, or the CAN
*                device was not ready to accept writes to AFMR and AFIR.
*
* @note        Acceptance Filter Mask and ID Registers are optional registers in
*              Xilinx XCanFd device. If they are not configured then the device
*              receives data with any ID.
*
******************************************************************************/
int XCanFd_AcceptFilterSet(XCanFd *InstancePtr, u32 FilterIndex,
			   u32 MaskValue, u32 IdValue)
{
	u32 EnabledFilters;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Xil_AssertNonvoid((FilterIndex > MIN_FILTER_INDEX) && (FilterIndex <= MAX_FILTER_INDEX));

	/*
	 * Check if the given filter is currently enabled. If yes, return error
	 * code.
	 */
	EnabledFilters = XCanFd_AcceptFilterGetEnabled(InstancePtr);

	if ((EnabledFilters & FilterIndex) == FilterIndex) {
		return (s32)XST_FAILURE;
	}
	FilterIndex--;
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_AFMR_OFFSET(FilterIndex), MaskValue);

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_AFIDR_OFFSET(FilterIndex), IdValue);


	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Reads the Acceptance Filter Mask and ID Register values.
*
* Reads the values of the Acceptance Filter Mask and ID Register for the
* specified Acceptance Filter. Use XCANFD_IDR_* defined in xcanfd_hw.h to
* interpret the values. Read xcanfd.h and device specification for details.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    FilterIndex Defines which Acceptance Filter Mask Register to get
*		Mask and ID from. Use any single XCANFD_FILTER_* value.
* @param[out]   MaskValue Mask value read from the chosen
*		Acceptance Filter Mask Register after this function returns.
* @param[out]   IdValue ID value read from the chosen Acceptance
*		Filter ID Register after this function returns.
*
* @note		Acceptance Filter Mask and ID Registers are optional registers
*		in Xilinx CAN device. If they are NOT existing in the device,
*		this function should NOT be used. Calling this function in this
*               case causes an assertion failure.
*
******************************************************************************/
void XCanFd_AcceptFilterGet(XCanFd *InstancePtr, u32 FilterIndex,
			    u32 *MaskValue, u32 *IdValue)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(FilterIndex < XCANFD_NOOF_AFR);
	Xil_AssertVoid((FilterIndex > MIN_FILTER_INDEX) && (FilterIndex <= MAX_FILTER_INDEX));

	FilterIndex = FilterIndex - (u32)1;
	*MaskValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				    XCANFD_AFMR_OFFSET(FilterIndex));

	*IdValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				  XCANFD_AFIDR_OFFSET(FilterIndex));

}
#ifndef SDT
/*****************************************************************************/
/**
*
* @brief Looks up the device configuration based on the device index.
*
* The table XCanFd_ConfigTable[] contains the configuration information for
* each device in the system.
*
* @param[in]    InstanceIndex 0-based integer indexing all CAN devices in
*		the system.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
******************************************************************************/
XCanFd_Config *XCanFd_GetConfig(unsigned int InstanceIndex)
{
	XCanFd_Config *CfgPtr;

	/* Check parameter */
	if (InstanceIndex >= XPAR_XCANFD_NUM_INSTANCES) {
		return NULL;
	}

	CfgPtr = &XCanFd_ConfigTable[InstanceIndex];

	return CfgPtr;
}
#endif
/******************************************************************************/
/**
*
* @brief Serves as a stub for asynchronous callbacks.
*
******************************************************************************/
static void StubHandler(void)
{
	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
*
* @brief Returns Data Length Code in bytes.
*
* @param[in]    Dlc DLC field value in Data Length Code Register.
* @param[in]    Edl EDL and FDF field in DLC register.
*
* @return	Total number of bytes stored in each buffer.
*
* @note		Refer CAN FD Spec about DLC.
*
******************************************************************************/
int XCanFd_GetDlc2len(u32 Dlc, u32 Edl)
{

	u32 NofBytes;

	if ((Edl == (u32)0U) && ((Dlc >> XCANFD_DLCR_DLC_SHIFT) > (u32)8)) {
		Dlc = XCANFD_DLC8;
	}

	switch (Dlc) {

		case  XCANFD_DLC1:
			NofBytes = 1;
			break;

		case XCANFD_DLC2:
			NofBytes = 2;
			break;

		case  XCANFD_DLC3:
			NofBytes = 3;
			break;

		case  XCANFD_DLC4:
			NofBytes = 4;
			break;

		case  XCANFD_DLC5:
			NofBytes = 5;
			break;

		case  XCANFD_DLC6:
			NofBytes = 6;
			break;

		case  XCANFD_DLC7:
			NofBytes = 7;
			break;

		case  XCANFD_DLC8:
			NofBytes = 8;
			break;

		case  XCANFD_DLC9:
			NofBytes = 12;
			break;

		case  XCANFD_DLC10:
			NofBytes = 16;
			break;

		case  XCANFD_DLC11:
			NofBytes = 20;
			break;

		case  XCANFD_DLC12:
			NofBytes = 24;
			break;

		case  XCANFD_DLC13:
			NofBytes = 32;
			break;

		case XCANFD_DLC14:
			NofBytes = 48;
			break;

		case XCANFD_DLC15:
			NofBytes = 64;
			break;
		default	:
			NofBytes = 0;
			break;
	}
	return (s32)NofBytes;
}

/*****************************************************************************/
/**
*
* @brief Returns the 4-bit Data Length Code for a given byte length.
*
* @param[in]    len Length in bytes.
*
* @return	The DLC value corresponding to the given byte length.
*
* @note		Refer CAN FD Spec about DLC.
*
******************************************************************************/
u8 XCanFd_GetLen2Dlc(int len)
{
	if (len <= 8) {
		return (u8)len;
	} else if (len <= 12) {
		return 9;
	} else if (len <= 16) {
		return 10;
	} else if (len <= 20) {
		return 11;
	} else if (len <= 24) {
		return 12;
	} else if (len <= 32) {
		return 13;
	} else if (len <= 48) {
		return 14;
	} else if (len <= 64) {
		return 15;
	} else {
		return (u8)XST_INVALID_DLC;
	}
}

/*****************************************************************************/
/**
* @brief Returns the free buffer count out of 32 Transmit Buffers.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return	Number of free buffers.
*
******************************************************************************/
u32 XCanFd_GetFreeBuffer(XCanFd *InstancePtr)
{

	u32 RegVal;
	u32 Index = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegVal = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_TRR_OFFSET);
	while ((RegVal & ((u32)1 << Index)) != (u32)0) {
		Index++;
		if (Index == MAX_BUFFER_INDEX) {
			break;
		}

	}

	return (MAX_BUFFER_INDEX - Index);
}

/*****************************************************************************/
/**
*
* @brief Sends queued buffers added via XCanFd_Addto_Queue().
*
* Triggers the TRR bits. Use this function to send multiple
* packets at a time.
*
* @param[in,out] InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return        - XST_SUCCESS.
*
******************************************************************************/
int XCanFd_Send_Queue(XCanFd *InstancePtr)
{

	u32 TrrVal;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Set the bits of Transmit Ready Request Register
	 * InstancePtr->MultiBuffTrr is updated by calling
	 * XCanFd_Addto_Queue()
	 */
	TrrVal = InstancePtr->MultiBuffTrr;

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TRR_OFFSET, TrrVal);

	InstancePtr->GlobalTrrValue = TRR_INIT_VAL;
	InstancePtr->GlobalTrrMask  = TRR_MASK_INIT_VAL;

	return (s32)XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* @brief Polls the TxBuffers for transmission completion.
*
* Call this function after sending multiple buffers using
* XCanFd_Addto_Queue() and XCanFd_Send_Queue().
*
* @param[in,out] InstancePtr Pointer to the XCanFd instance to be worked on.
*
******************************************************************************/
void XCanFd_PollQueue_Buffer(XCanFd *InstancePtr)
{
	u32 BufferNumber;
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->MultiBuffTrr != (u32)0) {
		for (BufferNumber = (u32)0; BufferNumber < MAX_BUFFER_VAL; BufferNumber++) {
			if ((InstancePtr->MultiBuffTrr & ((u32)TRR_POS_MASK << BufferNumber))
			    != (u32)0) {
				Value = (u32)XCanFd_IsBufferTransmitted(InstancePtr, BufferNumber);
				while (Value == (u32)FALSE) {
					Value = (u32)XCanFd_IsBufferTransmitted(
							InstancePtr, BufferNumber);
				}
			}
		}
		InstancePtr->MultiBuffTrr = (u32)0;
	}

}

/*****************************************************************************/
/**
*
* @brief Returns the number of messages stored in the Rx FIFO.
*
* The FSR Register has a field called FL that gives the number of packets
* received.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    fifo_no FIFO number.
*
* @return	The number of messages stored in FSR Register.
*
* @note		Selects either Rx Fifo 0 or Rx Fifo 1
*
******************************************************************************/
int XCanFd_GetNofMessages_Stored_Rx_Fifo(XCanFd *InstancePtr, u8 fifo_no)
{

	u32 FillLevel;
	u32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (fifo_no == (u8)XCANFD_RX_FIFO_0) {

		Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_FSR_OFFSET);

		FillLevel = Result & XCANFD_FSR_FL_MASK;
		FillLevel >>= XCANFD_FSR_FL_0_SHIFT;
	} else {

		Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_FSR_OFFSET);

		FillLevel = Result & XCANFD_FSR_FL_1_MASK;
		FillLevel >>= XCANFD_FSR_FL_1_SHIFT;
	}

	return (s32)FillLevel;

}

/*****************************************************************************/
/**
*
* @brief Returns the number of messages stored in TX Event FIFO.
*
* The FSR Register has a field called FL that gives the number of packets
* received.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
* @return	The number of messages stored in FSR Register.
*
* @note		This API is meant to be used with IP
*               with CanFD 2.0 spec support only.
*
******************************************************************************/
int XCanFd_GetNofMessages_Stored_TXE_FIFO(XCanFd *InstancePtr)
{

	u32 FillLevel;
	u32 Result;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXE_FSR_OFFSET);

	FillLevel = Result & XCANFD_TXE_FL_MASK;
	FillLevel >>= XCANFD_TXE_FL_SHIFT;

	return (s32)FillLevel;

}

/*****************************************************************************/
/**
*
* @brief Enables the Transceiver delay compensation.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
******************************************************************************/
void XCanFd_Enable_Tranceiver_Delay_Compensation(XCanFd *InstancePtr)
{
	u32 RegValue = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				  XCANFD_F_BRPR_OFFSET);
	RegValue |= XCANFD_F_BRPR_TDC_ENABLE_MASK;

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_F_BRPR_OFFSET, RegValue);
}

/*****************************************************************************/
/**
*
* @brief Sets the Transceiver delay compensation offset.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    TdcOffset Delay Compensation Offset.
*
******************************************************************************/
void XCanFd_Set_Tranceiver_Delay_Compensation(XCanFd *InstancePtr,
		u32 TdcOffset)
{

	u32 RegValue = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (TdcOffset <= TDC_MAX_OFFSET) {

		RegValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					  XCANFD_F_BRPR_OFFSET);

		TdcOffset <<= TDC_SHIFT;
		RegValue |= (TdcOffset & XCANFD_F_BRPR_TDCMASK);

		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_F_BRPR_OFFSET, RegValue);
	}

}

/*****************************************************************************/
/**
*
* @brief Disables the Transceiver delay compensation.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance.
*
******************************************************************************/
void XCanFd_Disable_Tranceiver_Delay_Compensation(XCanFd *InstancePtr)
{
	u32 RegValue = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				  XCANFD_F_BRPR_OFFSET);
	RegValue &= ~XCANFD_F_BRPR_TDC_ENABLE_MASK;
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_F_BRPR_OFFSET, RegValue);
}

/*****************************************************************************/
/**
*
* @brief Calculates the index position of the right most set bit.
*
* @param[in]    u GlobalTrrValue.
*
* @return       Index location of right most set bit.
*
* @note		log2 fast Math logic.
*
******************************************************************************/
static int XCanfd_TrrVal_Get_SetBit_Position(u32 u)
{

	u32 uCount;
	u32 lCount;

	u -= (u32)1;
	uCount = u - ((u >> SHIFT1) & FAST_MATH_MASK1) - ((u >> SHIFT2) & FAST_MATH_MASK2);
	lCount = ((uCount + (uCount >> SHIFT3)) & FAST_MATH_MASK3) % EXTRACTION_MASK;
	return (s32)lCount;

}

/*****************************************************************************/
/**
* @brief Receives a CAN/CAN FD Frame for the given Read Index.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
* @param[in]    ReadIndex Current RI (Read Index) maintained by CAN Core.
* @param[in]    FsrVal FIFO Status Register value.
* @param[out]   FramePtr Pointer to a 32-bit aligned buffer where the
*		CAN/CAN FD frame is to be written.
* @param[in]    fifo_no Target FIFO number.
*
* @return	- XST_SUCCESS after all operations.
*
* @note		This routine has generic logic for sequential receive mode
*
******************************************************************************/
static u32 XCanFd_SeqRecv_logic(XCanFd *InstancePtr, u32 ReadIndex, u32 FsrVal, u32 *FramePtr, u8 fifo_no)
{
	u32 DwIndex = 0;
	u32 CanEDL;
	u32 Dlc = 0;
	u32 Len;
	/* Read ID from ID Register */
	if (fifo_no == (u8)XCANFD_RX_FIFO_0) {
		FramePtr[0] = XCanFd_ReadReg(
				      InstancePtr->CanFdConfig.BaseAddress,
				      XCANFD_RXID_OFFSET(ReadIndex));
	} else {
		FramePtr[0] = XCanFd_ReadReg(
				      InstancePtr->CanFdConfig.BaseAddress,
				      XCANFD_FIFO_1_RXID_OFFSET(ReadIndex));
	}

	/* Read DLC from DLC Register*/
	if (fifo_no == (u8)XCANFD_RX_FIFO_0) {
		CanEDL = XCanFd_ReadReg(
				 InstancePtr->CanFdConfig.BaseAddress,
				 XCANFD_RXDLC_OFFSET(ReadIndex));
	} else {
		CanEDL = XCanFd_ReadReg(
				 InstancePtr->CanFdConfig.BaseAddress,
				 XCANFD_FIFO_1_RXDLC_OFFSET(ReadIndex));
	}
	FramePtr[1] = CanEDL;
	Dlc = (u32)XCanFd_GetDlc2len(FramePtr[1] & XCANFD_DLCR_DLC_MASK,
				     (CanEDL & XCANFD_DLCR_EDL_MASK));

	if ((CanEDL & XCANFD_DLCR_EDL_MASK) != (u32)0) {

		/* Can Fd frames */
		for (Len = (u32)0; Len < Dlc; Len += (u32)4) {
			if (fifo_no == (u8)XCANFD_RX_FIFO_0) {
				FramePtr[(u32)2 + DwIndex] = Xil_EndianSwap32(
								     XCanFd_ReadReg(
										     InstancePtr->CanFdConfig.BaseAddress,
										     (XCANFD_RXDW_OFFSET(ReadIndex) +
												     (DwIndex * XCANFD_DW_BYTES))));
			} else {
				FramePtr[(u32)2 + DwIndex] = Xil_EndianSwap32(
								     XCanFd_ReadReg(
										     InstancePtr->CanFdConfig.BaseAddress,
										     (XCANFD_FIFO_1_RXDW_OFFSET(ReadIndex) +
												     (DwIndex * XCANFD_DW_BYTES))));
			}
			DwIndex++;
		}
	} else {

		/* Legacy CAN Frame */
		for (Len = (u32)0; Len < Dlc; Len += (u32)4) {
			if (fifo_no == (u8)XCANFD_RX_FIFO_0) {
				FramePtr[(u32)2 + DwIndex] = Xil_EndianSwap32(
								     XCanFd_ReadReg(
										     InstancePtr->CanFdConfig.BaseAddress,
										     (XCANFD_RXDW_OFFSET(ReadIndex) + (Len))));
			} else {
				FramePtr[(u32)2 + DwIndex] = Xil_EndianSwap32(
								     XCanFd_ReadReg(
										     InstancePtr->CanFdConfig.BaseAddress,
										     (XCANFD_FIFO_1_RXDW_OFFSET(ReadIndex) + (Len))));
			}
			DwIndex++;
		}
	}

	/* Set the IRI bit causes core to increment RI in FSR Register */
	if (fifo_no == (u8)XCANFD_RX_FIFO_0) {
		FsrVal = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_FSR_OFFSET);
		FsrVal |= XCANFD_FSR_IRI_MASK;
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_FSR_OFFSET, FsrVal);
	} else {
		FsrVal = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_FSR_OFFSET);
		FsrVal |= XCANFD_FSR_IRI_1_MASK;
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_FSR_OFFSET, FsrVal);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief Recovers the CAN device from Protocol Exception and Bus-Off events.
*
* @param[in]    InstancePtr Pointer to the XCanFd instance to be worked on.
*
******************************************************************************/
void XCanFd_Pee_BusOff_Handler(XCanFd *InstancePtr)
{
	u32 RegValue;
	u32 Value;

	Xil_AssertVoid(InstancePtr != NULL);

	RegValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				  XCANFD_TRR_OFFSET);
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_TCR_OFFSET, RegValue);
	Value = (XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TRR_OFFSET));
	while (Value != (u32)0) {
		Value = (XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_TRR_OFFSET));
	}
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_TRR_OFFSET, RegValue);
}
/*****************************************************************************/
/** @} */
