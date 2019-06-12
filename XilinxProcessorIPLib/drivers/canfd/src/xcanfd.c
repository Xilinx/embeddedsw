/******************************************************************************
*
* Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xcanfd.c
* @addtogroup canfd_v2_2
* @{
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
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xenv.h"
#include "xcanfd.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

extern XCanFd_Config XCanFd_ConfigTable[];

/************************** Function Prototypes ******************************/

static void StubHandler(void);
static int XCanfd_TrrVal_Get_SetBit_Position(u32 u);
static u32 XCanFd_SeqRecv_logic(XCanFd *InstancePtr, u32 ReadIndex,
	   u32 FsrVal, u32 *FramePtr, u8 fifo_no);

/************************** Global Variables ******************************/

/*****************************************************************************/
/**
*
* This routine initializes a specific XCanFd instance/driver. This function
* should only be used when no Virtual Memory support is needed.
*
* This initialization entails:
* - Search for device configuration given the device ID.
* - Initialize Base Address field of the XCanFd structure using the device
* - address in the found device configuration.
* - Populate all other data fields in the XCanFd structure
* - Reset the device.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @param	ConfigPtr is the pointer to XCanFd_Config instance
*
* @param	EffectiveAddr is the base address of CANFD
*
* @return	- XST_SUCCESS if initialization was successful
* 		- XST_DEVICE_NOT_FOUND if device configuration information was
		not found for a device with the supplied device ID.
*
* @note		None.
*
******************************************************************************/
int XCanFd_CfgInitialize(XCanFd *InstancePtr, XCanFd_Config *ConfigPtr,
				UINTPTR EffectiveAddr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	/*
	 * Set some default values for instance data, don't indicate the device
	 * is ready to use until everything has been initialized successfully.
	 */
	InstancePtr->IsReady = 0;
	InstancePtr->CanFdConfig.BaseAddress = EffectiveAddr;
	InstancePtr->CanFdConfig.DeviceId = ConfigPtr->DeviceId;
	InstancePtr->CanFdConfig.Rx_Mode = ConfigPtr->Rx_Mode;
	InstancePtr->CanFdConfig.NumofRxMbBuf = ConfigPtr->NumofRxMbBuf;
	InstancePtr->CanFdConfig.NumofTxBuf = ConfigPtr->NumofTxBuf;

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

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function returns enabled acceptance filters. Use XCANFD_AFR_UAF*_MASK
* defined in xcanfd_hw.h to interpret the returned value. If no acceptance
* filters are enabled then all received frames are stored in the RX FIFO.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	The value stored in Acceptance Filter Register.
*
* @note		Acceptance Filter Register is an optional register in Xilinx
*		CAN device If it is NOT existing in the device, this function
*		should NOT be used.Calling this function in this case will
*		cause an assertion failure.
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
* This routine returns current operation mode the CAN device is in.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	- XCANFD_MODE_CONFIG if the device is in Configuration Mode.
* 		- XCANFD_MODE_SLEEP if the device is in Sleep Mode.
*		- XCANFD_MODE_NORMAL if the device is in Normal Mode.
*		- XCANFD_MODE_LOOPBACK if the device is in Loop Back Mode.
*		- XCANFD_MODE_SNOOP if the device is in Snoop Mode.
*		- XCANFD_MODE_BR if the device is in Bus-Off recovery Mode.
*		- XCANFD_MODE_PEE if the device is in Protocol Exception
*		  Event mode.
* @note		None.
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
	}
	else if ((Value & XCANFD_SR_SLEEP_MASK) != (u32)0) {
		Mode = XCANFD_MODE_SLEEP;
	}
	else if ((Value & XCANFD_SR_NORMAL_MASK) != (u32)0) {
		if ((Value & XCANFD_SR_SNOOP_MASK) != (u32)0) {
			Mode = XCANFD_MODE_SNOOP;
		}
		else {
			Mode = XCANFD_MODE_NORMAL;
		}
	}

	else {
		/* If this line is reached, the device is in Loop Back Mode. */
		Mode = XCANFD_MODE_LOOPBACK;
	}
	if ((Value & XCANFD_SR_PEE_CONFIG_MASK) != (u32)0){
		Mode = Mode | XCANFD_MODE_PEE;
	}

	return (u8)Mode;
}

/*****************************************************************************/
/**
*
* This function allows the CAN device to enter one of the following operation
* modes:
*
* - Configuration Mode: Pass in parameter XCANFD_MODE_CONFIG
* - Sleep Mode: Pass in parameter XCANFD_MODE_SLEEP
* - Normal Mode: Pass in parameter XCANFD_MODE_NORMAL
* - Loop Back Mode: Pass in parameter XCANFD_MODE_LOOPBACK
* - Snoop Mode:	Pass in Parameter XCANFD_MODE_SNOOP
* - Auto Bus-Off Recovery Mode: Pass in Parameter XCANFD_MODE_ABR
* - Start Bus-Off Recovery Mode: Pass in Parameter XCANFD_MODE_SBR
* - Protocol Exception Event Mode: Pass in Parameter XCANFD_MODE_PEE
* - Disable AutoRetransmission Mode: Pass in Parameter XCANFD_MODE_DAR
*
* Read xcanfd.h and device specification for detailed description of each
* operation mode.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	OperationMode specify which operation mode to enter.Valid value
*		is any of XCANFD_MODE_* defined in xcanfd.h. Please note no
*		multiple modes could be entered at the same time.
*
* @return	None.
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
	Xil_AssertVoid((OperationMode == XCANFD_MODE_CONFIG) ||
			(OperationMode == XCANFD_MODE_SLEEP) ||
			(OperationMode == XCANFD_MODE_NORMAL) ||
			(OperationMode == XCANFD_MODE_LOOPBACK) ||
			(OperationMode == XCANFD_MODE_SNOOP) ||
			(OperationMode == XCANFD_MODE_PEE) ||
			(OperationMode == XCANFD_MODE_ABR) ||
			(OperationMode == XCANFD_MODE_DAR) ||
			(OperationMode == XCANFD_MODE_SBR));

	CurrentMode = XCanFd_GetMode(InstancePtr);
	MsrReg = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_MSR_OFFSET) & XCANFD_MSR_CONFIG_MASK;
	if ((CurrentMode == XCANFD_MODE_NORMAL) &&
		(OperationMode == XCANFD_MODE_SLEEP)) {

		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_MSR_OFFSET,
				(XCANFD_MSR_SLEEP_MASK | MsrReg));
		return;
	}

	else if ((CurrentMode == XCANFD_MODE_SLEEP) &&
		 (OperationMode == XCANFD_MODE_NORMAL)) {
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_MSR_OFFSET, MsrReg);
		return;
	}

	/*
	 * If the mode transition is not any of the two cases above, CAN must
	 * enter Configuration Mode before switching into the target operation
	 * mode.
	 */
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
		XCANFD_SRR_OFFSET, 0);

	/*
	 * Check if the device has entered Configuration Mode, if not, return
	 * to the caller.
	 */
	if (XCanFd_GetMode(InstancePtr) != XCANFD_MODE_CONFIG) {
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

		case XCANFD_MODE_LOOPBACK:

			XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_MSR_OFFSET,
					(XCANFD_MSR_LBACK_MASK | MsrReg));
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
	}
}

/*****************************************************************************/
/**
*
* This function reads Receive and Transmit error counters.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	RxErrorCount will contain Receive Error Counter value after
		this function returns.
* @param	TxErrorCount will contain Transmit Error Counter value after
*		this function returns.
*
* @return	None.
*
* @note		None.
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

	*RxErrorCount = (Result & XCANFD_ECR_REC_MASK) >> XCANFD_ECR_REC_SHIFT;
	*TxErrorCount = Result & XCANFD_ECR_TEC_MASK;
}

/*****************************************************************************/
/**
*
* This function sends a CAN/CANFD Frame. This function first checks whether
* free buffer is there or not.if free buffer is there the user data will be
* written into the free buffer.otherwise it returns error code immediately.
* This function does not wait for the given frame being sent to CAN bus.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	FramePtr is a pointer to a 32-bit aligned buffer containing the
*		CAN frame to be sent.
*
* @param	TxBufferNumber is the buffer where the user data has been
*		written and it is updated by driver.
*
* @return	- XST_SUCCESS if TX FIFO was not full and the given frame was
*		written into the FIFO;
*		- XST_FIFO_NO_ROOM if there is no room in the TX FIFO for the
*		given frame
*
* @note		None.
*
*****************************************************************************/
int XCanFd_Send(XCanFd *InstancePtr, u32 *FramePtr,u32 *TxBufferNumber)
{
	u32 FreeTxBuffer;
	u32 TrrVal;
	u32 DwIndex=0;
	u32 Value;
	u32 Dlc;
	u32 Len;
	u32 CanEDL;
	u32 OutValue;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Poll TRR to check pending transmission requests */

	if(InstancePtr->GlobalTrrMask == 0)
		InstancePtr->GlobalTrrMask = TRR_MASK_INIT_VAL;
	TrrVal = XCanFd_GetFreeBuffer(InstancePtr);
	Value = (~TrrVal) & InstancePtr->GlobalTrrMask;
	Value = XCanFD_Check_TrrVal_Set_Bit(Value);
	FreeTxBuffer =  XCanfd_TrrVal_Get_SetBit_Position(Value);

	if (FreeTxBuffer == XST_NOBUFFER){
		return XST_FIFO_NO_ROOM;
	}
	else {

		/* Write ID to ID Register */
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXID_OFFSET(FreeTxBuffer), FramePtr[0]);

		/* Write DLC to DLC Register */
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXDLC_OFFSET(FreeTxBuffer), FramePtr[1]);
		CanEDL = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXDLC_OFFSET(FreeTxBuffer));

		if (CanEDL & XCANFD_DLCR_EDL_MASK) {

			/* CAN FD Frames. */
			Dlc = XCanFd_GetDlc2len(FramePtr[1] &
				XCANFD_DLCR_DLC_MASK,
				(CanEDL & XCANFD_DLCR_EDL_MASK));
			/* Write Data to Data Register */

			for (Len = 0;Len < Dlc;Len += 4) {
				OutValue = Xil_EndianSwap32(
						FramePtr[2+DwIndex]);
				XCanFd_WriteReg(
					InstancePtr->CanFdConfig.BaseAddress,
					(XCANFD_TXDW_OFFSET(FreeTxBuffer)+
					(DwIndex*XCANFD_DW_BYTES)), OutValue);
				DwIndex++;
			}
		}
		else {

			/* Legacy CAN Frames */
			Dlc = XCanFd_GetDlc2len(FramePtr[1] &
				XCANFD_DLCR_DLC_MASK,
				(CanEDL & XCANFD_DLCR_EDL_MASK));
			for (Len = 0;Len < Dlc;Len += 4) {
				OutValue = Xil_EndianSwap32(
						FramePtr[2+DwIndex]);
				XCanFd_WriteReg(
					InstancePtr->CanFdConfig.BaseAddress,
					(XCANFD_TXDW_OFFSET(FreeTxBuffer)+
					(Len)), OutValue);
				DwIndex++;
			}
		}
	}

	Value = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_TRR_OFFSET);
	Value |= (1 << FreeTxBuffer);
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_TRR_OFFSET, Value);

	/* Assign buffer number to user */
	*TxBufferNumber = FreeTxBuffer;

	/* Make That buffer as transmitted */

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes the Data into specific Buffer.we have 32 TxBuffers
* we can Add data to each Buffer using this routine.This routine won't
* transmit the data. it only adds data to Buffers.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	FramePtr is a pointer to a 32-bit aligned buffer containing the
*		CAN frame to be sent.
* @param	TxBufferNumber is Buffer Number where the data has written
*		and is given back to user.
*
* @return	- XST_SUCCESS if TX FIFO was not full and the given frame was
*		written into the FIFO;
*		- XST_FIFO_NO_ROOM if there is no room in the TX FIFO for the
*		given frame
*
* @note		None.
*
******************************************************************************/
int XCanFd_Addto_Queue(XCanFd *InstancePtr, u32 *FramePtr,u32 *TxBufferNumber)
{
	u32 FreeTxBuffer;
	u32 TrrVal;
	u32 MaskValue;
	u32 DwIndex=0;
	u32 Len;
	u32 Dlc;
	u32 CanEDL;
	u32 OutValue;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Poll TRR to check pending transmission requests */
	XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_TRR_OFFSET);

	TrrVal = XCanFd_GetFreeBuffer(InstancePtr);
	if (InstancePtr->MultiBuffTrr == TRR_MASK_INIT_VAL) {
		return XST_FIFO_NO_ROOM;
	}

	if(InstancePtr->GlobalTrrMask == 0)
		InstancePtr->GlobalTrrMask = TRR_MASK_INIT_VAL;
	MaskValue = (~TrrVal) & InstancePtr->GlobalTrrMask ;
	InstancePtr->GlobalTrrValue = XCanFD_Check_TrrVal_Set_Bit(MaskValue);
	FreeTxBuffer =  XCanfd_TrrVal_Get_SetBit_Position(InstancePtr->GlobalTrrValue);
	InstancePtr->GlobalTrrMask ^= InstancePtr->GlobalTrrValue;

	if (FreeTxBuffer == XST_NOBUFFER){
		return XST_FIFO_NO_ROOM;
	}
	else {

		InstancePtr->MultiBuffTrr = ~(InstancePtr->GlobalTrrMask);

		/* Write ID to ID Register*/
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXID_OFFSET(FreeTxBuffer),FramePtr[0]);

		/* Write DLC to DLC Register*/
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXDLC_OFFSET(FreeTxBuffer),FramePtr[1]);

		CanEDL = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXDLC_OFFSET(FreeTxBuffer));

		Dlc = XCanFd_GetDlc2len(FramePtr[1] & XCANFD_DLCR_DLC_MASK,
			(CanEDL & XCANFD_DLCR_EDL_MASK));

		if (CanEDL & XCANFD_DLCR_EDL_MASK) {

			/* CAN FD Frames */
			for (Len = 0;Len < Dlc;Len += 4) {
				OutValue = Xil_EndianSwap32(
						FramePtr[2+DwIndex]);
				XCanFd_WriteReg(
					InstancePtr->CanFdConfig.BaseAddress,
						(XCANFD_TXDW_OFFSET(
						FreeTxBuffer)+(DwIndex*
						XCANFD_DW_BYTES)),OutValue);
				DwIndex++;
			}
		}

		else {

			/* Legacy CAN Frames */
			for (Len = 0;Len < Dlc;Len += 4) {
				OutValue = Xil_EndianSwap32(
						FramePtr[2+DwIndex]);
				XCanFd_WriteReg(
					InstancePtr->CanFdConfig.BaseAddress,
						(XCANFD_TXDW_OFFSET(
						FreeTxBuffer)+(Len)),
						OutValue);
				DwIndex++;
			}
		}
		/* Assign  Buffer to user */
		*TxBufferNumber = FreeTxBuffer;


		return XST_SUCCESS;
	}
}

/*****************************************************************************/
/**
*
* This function receives a CAN/CAN FD Frame. This function first checks FSR
* Register.The FL bits tells the Number of Packets received. if FL is non Zero
* then Read the Packet and store it to user Buffer.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	FramePtr is a pointer to a 32-bit aligned buffer where the
*		    CAN/CAN FD frame to be written.
*
* @return - XST_SUCCESS if RX FIFO was not empty and a frame was read from
*		    RX FIFO successfully and written into the given buffer;
*		    - XST_NO_DATA if there is no frame to be received from the FIFO
*
* @note	 The CANFD has two design modes.
*		 ->Sequential Mode - Core writes data sequentially to RxBuffers.
*		 ->MailBox Mode	  - Core writes data to RxBuffers when a ID
*					        Match happened.
* 		This routine distinguishes receive checking as per the frame
*       availability from either Fifo 0 or Fifo 1.
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

	if ((Result & XCANFD_FSR_FL_MASK) || (Result & XCANFD_FSR_FL_1_MASK)) {

		/* Check for the Packet Available by reading FSR Register */
		if (Result & XCANFD_FSR_FL_MASK) {
		/*Fill the canfd frame for current RI value for Fifo 0 */

				ReadIndex = Result & XCANFD_FSR_RI_MASK;
				FifoNo = XCANFD_RX_FIFO_0;
		}

		if (Result & XCANFD_FSR_FL_1_MASK) {
		/*Fill the canfd frame for current RI value for Fifo 1 */

				ReadIndex = ((Result & XCANFD_FSR_IRI_1_MASK)
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
* This function receives a CAN/CAN FD TX Events. This function first checks FSR
* Register.The FL bits tells the Number of TX Event packets received. if FL is
* non Zero then Read the Packet and store it to user Buffer.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	FramePtr is a pointer to a 32-bit aligned buffer where the
*		CAN/CAN FD frame Event to be written.
*
* @return	- XST_SUCCESS if TXE FIFO was not empty and a frame was read from
*		TX FIFO successfully and written into the given buffer;
*		- XST_NO_DATA if there is no frame to be received from the TXE FIFO
*
* @note		This CANFD has two design modes.
*		->Sequential Mode - Core writes data sequentially to RxBuffers.
*		->MailBox Mode	  - Core writes data to RxBuffers when a ID
*					Match happened.
* 		This routine is useful for Both the Modes.This API is meant to be used
*		with IP with CanFD 2.0 spec support only.
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
	if (Result & XCANFD_TXE_FL_MASK) {
		ReadIndex = Result & XCANFD_TXE_RI_MASK;

		/* Read ID from ID Register*/
		FramePtr[0] = XCanFd_ReadReg(
				InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXEID_OFFSET(ReadIndex));

		/* Read DLC from DLC Register*/
		FramePtr[1] = CanEDL = XCanFd_ReadReg(
				InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXEDLC_OFFSET(ReadIndex));

		/* Set the IRI bit causes core to increment RI in FSR Register */
		Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXE_FSR_OFFSET);
		Result |= XCANFD_TXE_IRI_MASK;
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXE_FSR_OFFSET,Result);
		Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TXE_FSR_OFFSET);

		return XST_SUCCESS;
	}
	else {
		return XST_NO_DATA;
	}
}

/*****************************************************************************/
/**
*
* This function receives a CAN Frame in MAIL BOX Mode. Read Rx Last Buffer
* Index from ISR Register. This tells which buffer is having data.then read
* and update the data to user buffer.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	FramePtr is a pointer to a 32-bit aligned buffer where the CAN
*		frame to be receive.
*
* @return	- XST_SUCCESS if RX FIFO was not empty and a frame was read from
*		RX FIFO successfully and written into the given buffer;
*		- XST_NO_DATA if there is no frame to be received from the FIFO
*
* @note		This CANFD has two design modes.
*		->Sequential Mode - Core writes data sequentially to RxBuffers.
*		->MailBox Mode	  - Core writes data to RxBuffers when a ID
*				    match happened.
*		This routine is useful for MailBox Mode.
*
******************************************************************************/
u32 XCanFd_Recv_Mailbox(XCanFd *InstancePtr, u32 *FramePtr)
{
	u32 DwIndex=0;
	u32 Result;
	u32 CanEDL;
	u32 Dlc;
	u32 Len;
	u32 RcsRegNr=0;
	u32 RxBufferIndex;
	u32 CoreStatusBit;
	u32 Mask;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RxBufferIndex = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_ISR_OFFSET) &
					XCANFD_IXR_RXLRM_BI_MASK;
	RxBufferIndex >>= XCANFD_RXLRM_BI_SHIFT;
	CoreStatusBit = (RxBufferIndex%XCANFD_CSB_SHIFT) + XCANFD_CSB_SHIFT;
	RcsRegNr = RxBufferIndex/XCANFD_CSB_SHIFT;

	Result = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
                                XCANFD_RCS_OFFSET(RcsRegNr));
	Mask = Result & XCANFD_RCS_HCB_MASK;
	if (Result & 1<<CoreStatusBit) {
		/* Read CanFd ID*/
		FramePtr[0] = XCanFd_ReadReg(InstancePtr->CanFdConfig.
				BaseAddress,XCANFD_RXID_OFFSET(RxBufferIndex));

		/* Read CanFd DLC */
		FramePtr[1] = CanEDL = XCanFd_ReadReg(InstancePtr->CanFdConfig.
					BaseAddress,
					XCANFD_RXDLC_OFFSET(RxBufferIndex));

		Dlc = XCanFd_GetDlc2len(FramePtr[1] &
			XCANFD_DLCR_DLC_MASK,
			(CanEDL & XCANFD_DLCR_EDL_MASK));
		/* A CanFD Frame is received */

		if (CanEDL & XCANFD_DLCR_EDL_MASK) {

			/* Read all Bytes from DW Register */

			for (Len = 0;Len < Dlc;Len += 4) {
				FramePtr[2+DwIndex] = Xil_EndianSwap32(
						      XCanFd_ReadReg(
						      InstancePtr->CanFdConfig.
						      BaseAddress,
						      (XCANFD_RXDW_OFFSET(
						      RxBufferIndex)
						      +(DwIndex*XCANFD_DW_BYTES))));
						      DwIndex++;
			}
			DwIndex=0;
		}
		else {
			/* Legacy CAN Frame */
			for (Len = 0;Len < Dlc;Len += 4) {
				FramePtr[2+DwIndex] = Xil_EndianSwap32(
						      XCanFd_ReadReg(
						      InstancePtr->CanFdConfig.
						      BaseAddress,
						      (XCANFD_RXDW_OFFSET(
						      RxBufferIndex)+
						      (Len))));
						      DwIndex++;
			}
		}
		/* Clear CSB Bit of RCS Register */
		Result = Mask | (1 << CoreStatusBit);
		XCanFd_WriteReg(InstancePtr->CanFdConfig.
			BaseAddress,XCANFD_RCS_OFFSET(RcsRegNr),Result);
		return XST_SUCCESS;
	} else {
		return XST_NO_DATA;
	}
}

/*****************************************************************************/
/**
*
* This function sets an RxBuffer to Active State.In Mailbox Mode configuration
* we can set each buffer to receive with specific Id and Mask.inorder compare
* we need to first Activate the Buffer.Maximum number of RxBuffers depends on
* Design.Range 48,32,16.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	RxBuffer Receive Buffer Number defines which Buffer to configure
*		Value ranges from  0 - 48
*
* @return	- XST_SUCCESS if the values were set successfully.
*
* @note		none
*
******************************************************************************/
u32 XCanFd_RxBuff_MailBox_Active(XCanFd *InstancePtr, u32 RxBuffer)
{
	u32 Status = 0;
	u32 NoCtrlStatus;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(RxBuffer < InstancePtr->CanFdConfig.NumofRxMbBuf);

	if (RxBuffer <=DESIGN_RANGE_1) {
		NoCtrlStatus = CONTROL_STATUS_1;
	}
	else if (RxBuffer <=DESIGN_RANGE_2) {
		RxBuffer -= (DESIGN_RANGE_1+1);
		NoCtrlStatus = CONTROL_STATUS_2;
	}
	else {
		RxBuffer -= (DESIGN_RANGE_2+1);
		NoCtrlStatus = CONTROL_STATUS_3;
	}
	Status = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RCS_OFFSET(NoCtrlStatus));
	if (Status & 1 << RxBuffer) {
		return 1;
	}
	Status |= (1<< RxBuffer);
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RCS_OFFSET(NoCtrlStatus),Status);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the Id and Mask for an RxBuffer to participate in
* Id match.if a packet is received with an id which is equal to id we
* configured, then it is stored in RxBuffer. otherwise it won't.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	RxBuffer Receive Buffer Number defines which Buffer to configure
*		Value ranges from  0 - 48(can get from NumofRxMbBuf)
* @param	MaskValue is the value to write into the RxBuffer Mask Register
* @param	IdValue is the value to write into the RxBuffer Id register
*
* @return	- XST_SUCCESS if the values were set successfully.
**
* @note		none
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

	if (RxBuffer <=DESIGN_RANGE_1) {
		NoCtrlStatus = CONTROL_STATUS_1;
	}
	else if (RxBuffer <=DESIGN_RANGE_2) {
		RxBuffer -= (DESIGN_RANGE_1+1);
		NoCtrlStatus = CONTROL_STATUS_2;
	}
	else {
		RxBuffer -= (DESIGN_RANGE_2+1);
		NoCtrlStatus = CONTROL_STATUS_3;
	}

	Status = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_RCS_OFFSET(NoCtrlStatus));

	if (Status & 1 << RxBuffer) {
		Status &= ~(1<< RxBuffer);
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RCS_OFFSET(NoCtrlStatus),Status);
	}

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_MAILBOX_MASK_OFFSET(BufferNr),MaskValue);
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_MAILBOX_ID_OFFSET(BufferNr),IdValue);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets an RxBuffer to InActive State.if we change a buffer to
* InActive state, then Rx Packet won't store into that buffer, even the Id
* is matched.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	RxBuffer Receive Buffer Number defines which Buffer to configure
*		Value ranges from  0 - 48
*
* @return	- XST_SUCCESS if the values were set successfully.
*		- XST_FAILURE if the given filter was not disabled, or the CAN
*		device was not ready to accept writes to AFMR and AFIR.
*
* @note		none
*
******************************************************************************/
u32 XCanFd_RxBuff_MailBox_DeActive(XCanFd *InstancePtr, u32 RxBuffer)
{
	u32 Status;
	u8 NoCtrlStatus;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(RxBuffer < InstancePtr->CanFdConfig.NumofRxMbBuf);

	if (RxBuffer <=DESIGN_RANGE_1) {
		NoCtrlStatus = CONTROL_STATUS_1;
	}
	else if (RxBuffer <=DESIGN_RANGE_2) {
		RxBuffer -= (DESIGN_RANGE_1+1);
		NoCtrlStatus = CONTROL_STATUS_2;
	}
	else {
		RxBuffer -= (DESIGN_RANGE_2+1);
		NoCtrlStatus = CONTROL_STATUS_3;
	}

	Status = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_RCS_OFFSET(NoCtrlStatus));
	if (Status & 1 << RxBuffer) {
		Status &= ((~(1<< RxBuffer)) & XCANFD_MBRXBUF_MASK);
		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RCS_OFFSET(NoCtrlStatus),Status);
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function Cancels a CAN/CAN FD Frame which was already initiated for
* transmission.This function first checks TRR Bit based on BufferNumber.
* if TRR Bit is set, then it cancels the Buffers.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	BufferNumber is which Buffer to cancel out of 32 Buffers.
*
* @return	- XST_SUCCESS if RX FIFO was not empty and a frame was read from
*		RX FIFO successfully and written into the given buffer;
*		- XST_NO_DATA if there is no frame to be received from the FIFO
*
* @note		This function first checks whether TRR bit is set or not
*		if Set, it then checks the corresponding TCR bit
*			->if Set, then wait until cancellation is performed.
*			->if Not set, then set the corresponding TCR bit and
*			wait until core clears it.
*		if Not set, Nothing to do.
*
******************************************************************************/
int XCanFd_TxBuffer_Cancel_Request(XCanFd *InstancePtr,u32 BufferNumber)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (BufferNumber <MAX_BUFFER_INDEX) {
		if (XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_TRR_OFFSET) & (1<<BufferNumber)) {
			if (XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_TCR_OFFSET)& (1<<BufferNumber)) {

				/*
				 * Already Cancellation request is in process
				 * Host should wait until it core clears it.
				 */
				while (XCanFd_ReadReg(
					InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_TCR_OFFSET)& (1<<BufferNumber));

				return XST_SUCCESS;
			}
			else {

				/*
				 * buffer has pending transmission request but no pending
				 * cancellation request
				 */
				XCanFd_WriteReg(InstancePtr->CanFdConfig.
				BaseAddress,XCANFD_TCR_OFFSET,1<<BufferNumber);

				while (XCanFd_ReadReg(
					InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_TCR_OFFSET)& (1<<BufferNumber));

				return XST_SUCCESS;
			}
		}

		return XST_FAILURE;
	}
	else
		return XST_FAILURE;

}

/*****************************************************************************/
/**
* This routine enables the acceptance filters. Up to 32 filters can
* be enabled.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	FilterIndexMask specifies which filter(s) to enable. Use
*		any XCANFD_AFR_UAF*_MASK to enable one filter, and "Or" multiple
*		XCANFD_AFR_UAF*_MASK values if multiple filters need to be
*		enabled. Any filter not specified in this parameter will keep
*		its previous enable/disable setting.
*
* @return	None.
*
* @note		In Sequential Mode, in order to receive data, we need to enable
*		these filters. if we want to make filtration i.e Id match
*		then we need to set the Id value in AFR Id register.
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
* This routine disables the acceptance filters. 32 filters can be disabled.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	FilterIndexMask specifies which filter(s) to disable. Use
*		any XCANFD_AFR_UAF*_MASK to disable one filter, and "Or" multiple
*		XCANFD_AFR_UAF*_MASK values if multiple filters need to be
*		disabled. Any filter not specified in this parameter will keep
*		its previous enable/disable setting. If all acceptance filters
*		are disabled then all received frames are stored in the RX FIFO.
*
* @return	None.
*
* @note		None
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
* This function sets values to the Acceptance Filter Mask Register (AFMR) and
* Acceptance Filter ID Register (AFIR) for the specified Acceptance Filter.
* Use XCANFD_IDR_* defined in xcanfd_hw.h to create the values to set the filter.
* Read xcanfd.h and device specification for details.
*
* This function should be called only after:
*   - The given filter is disabled by calling XCanFd_AcceptFilterDisable();
*   - And the CAN device is ready to accept writes to AFMR and AFIR, i.e.,
*	 XCanFd_IsAcceptFilterBusy() returns FALSE.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	FilterIndex defines which Acceptance Filter Mask and ID Register
*		to set. Use any single XCANFD_AFR_UAF*_MASK value.ranges from 1
*		- 32
* @param	MaskValue is the value to write to the chosen Acceptance Filter
*		Mask Register.
* @param	IdValue is the value to write to the chosen Acceptance Filter
*		ID Register.
*
* @return	- XST_SUCCESS if the values were set successfully.
*		- XST_FAILURE if the given filter was not disabled, or the CAN
*		device was not ready to accept writes to AFMR and AFIR.
*
* @note		Acceptance Filter Mask and ID Registers are optional registers in
*		Xilinx XCanFd device.if they are not configured then device will
*		receive data with any ID.
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
		return XST_FAILURE;
	}
	FilterIndex--;
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_AFMR_OFFSET(FilterIndex),MaskValue);

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_AFIDR_OFFSET(FilterIndex),IdValue);


	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function reads the values of the Acceptance Filter Mask and ID Register
* for the specified Acceptance Filter. Use XCANFD_IDR_* defined in xcanfd_hw.h to
* interpret the values. Read xcanfd.h and device specification for details.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	FilterIndex defines which Acceptance Filter Mask Register to get
*		Mask and ID from. Use any single XCANFD_FILTER_* value.
* @param	MaskValue will store the Mask value read from the chosen
*		Acceptance Filter Mask Register after this function returns.
* @param	IdValue will store the ID value read from the chosen Acceptance
*		Filter ID Register after this function returns.
*
* @return	None.
*
* @note		Acceptance Filter Mask and ID Registers are optional registers
*		in Xilinx CAN device. If they are NOT existing in the device,
*		this function should NOT be used. Calling this function in this
*		case will cause an assertion failure.
*
******************************************************************************/
void XCanFd_AcceptFilterGet(XCanFd *InstancePtr, u32 FilterIndex,
			  u32 *MaskValue, u32 *IdValue)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(FilterIndex < XCANFD_NOOF_AFR);
	Xil_AssertVoid((FilterIndex > MIN_FILTER_INDEX) && (FilterIndex <= MAX_FILTER_INDEX));

	FilterIndex--;
	*MaskValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_AFMR_OFFSET(FilterIndex));

	*IdValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_AFIDR_OFFSET(FilterIndex));

}

/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the device index.
* The table XCanFd_ConfigTable[] contains the configuration information for each
* device in the system.
*
* @param	InstanceIndex is a 0-based integer indexing all CAN devices in
*		the system.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XCanFd_Config *XCanFd_GetConfig(unsigned int InstanceIndex)
{
	XCanFd_Config *CfgPtr;

	/* Check parameter */
	if (InstanceIndex >= XPAR_XCANFD_NUM_INSTANCES)
		return NULL;

	CfgPtr = &XCanFd_ConfigTable[InstanceIndex];

	return CfgPtr;
}

/******************************************************************************/
/**
*
* This routine is a stub for the asynchronous callbacks. The stub is here in
* case the upper layer forgot to set the handler(s). On initialization, all
* handlers are set to this callback. It is considered an error for this handler
* to be invoked.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void StubHandler(void)
{
	Xil_AssertVoidAlways();
}

/*****************************************************************************/
/**
*
* This function returns Data Length Code(in Bytes),we need to pass
* DLC Field value in DLC Register.
*
* @param	Dlc Field in Data Length Code Register.
* @param	Edl/Fdf Field in DLC register.
*
*
* @return	Total Number of Bytes stored in each Buffer.
*
* @note		Refer CAN FD Spec about DLC.
*
******************************************************************************/
int XCanFd_GetDlc2len(u32 Dlc, u32 Edl)
{

	u32 NofBytes=0;

	if ((Edl == 0U) && ((Dlc >> XCANFD_DLCR_DLC_SHIFT) > 8)) {
		Dlc = XCANFD_DLC8;
	}

	switch(Dlc) {

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
				break;
	}
	return NofBytes;
}

/*****************************************************************************/
/**
*
* This function returns Data Length Code of 4bits,we need to pass
* length in bytes.
*
* @param	len is the length in bytes.
*
* @return	Total Number of Bytes stored in each Buffer.
*
* @note		Refer CAN FD Spec about DLC.
*
******************************************************************************/
u8 XCanFd_GetLen2Dlc(int len)
{
	if(len <= 8)
		return len;
	else if(len <= 12)
		return 9;
	else if(len <= 16)
		return 10;
	else if(len <= 20)
		return 11;
	else if(len <= 24)
		return 12;
	else if(len <= 32)
		return 13;
	else if(len <= 48)
		return 14;
	else if(len <= 64)
		return 15;
	else
		return XST_INVALID_DLC;
}

/*****************************************************************************/
/**
* This Routine returns the Free Buffer out of 32 Transmit Buffers.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
*
* @return	Returns Free buffer if any free buffer
*		other wise returns XST_NOBUFFER.
*
* @note		None.
*
******************************************************************************/
u32 XCanFd_GetFreeBuffer(XCanFd *InstancePtr)
{

	u32 RegVal;
	u32 Index = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegVal = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_TRR_OFFSET);
	while(RegVal & (1 << Index)) {
		Index++;
		if(Index == MAX_BUFFER_INDEX)
			break;

	}
	if(Index == MAX_BUFFER_INDEX)
		return XST_NOBUFFER;

	return RegVal;
}

/*****************************************************************************/
/**
*
* This routine sends queue of buffers,when added to queue using Addto_Queue()
* Basically this will trigger the TRR Bit(s).This routine can be used
* when user want to send multiple packets at a time.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	- XST_SUCCESS.
*
* @note		None.
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

	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress, XCANFD_TRR_OFFSET, TrrVal);
	InstancePtr->GlobalTrrValue = TRR_INIT_VAL;
	InstancePtr->GlobalTrrMask  = TRR_MASK_INIT_VAL;

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* This function Polls the TxBuffer(s) whether it is transmitted or not.
* This function can call when user sends multiple Buffers using Addto_Queue()
* and XCanFd_Send_Queue().
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	None.
*
*
* @note		None.
*
******************************************************************************/
void XCanFd_PollQueue_Buffer(XCanFd *InstancePtr)
{
	u32 BufferNumber;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->MultiBuffTrr!=0) {
		for (BufferNumber = 0;BufferNumber < MAX_BUFFER_VAL;BufferNumber++) {
			if (InstancePtr->MultiBuffTrr & ((u32)TRR_POS_MASK << BufferNumber))
				while (XCanFd_IsBufferTransmitted(InstancePtr,
					BufferNumber) == FALSE);
		}
		InstancePtr->MultiBuffTrr = 0;
	}

}

/*****************************************************************************/
/**
*
* This function returns Number of messages Stored.
* The FSR Register has Field called FL. this gives number of packets
* received.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	Value is the number of messages stored in FSR Register.
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

	if (fifo_no == XCANFD_RX_FIFO_0) {

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

	return FillLevel;

}

/*****************************************************************************/
/**
*
* This function returns Number of messages Stored in TX Event FIFO
* The FSR Register has Field called FL. this gives number of packets
* received.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	Value is the number of messages stored in FSR Register.
*
* @note		This API is meant to be used with IP
*			with CanFD 2.0 spec support only.
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

	return FillLevel;

}

/*****************************************************************************/
/**
*
* This function Enables the Transceiver delay compensation.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	None.
*
*
* @note		None.
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
		XCANFD_F_BRPR_OFFSET,RegValue);
}

/*****************************************************************************/
/**
*
* This function Sets the Transceiver delay compensation offset.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @param	TdcOffset is the Delay Compensation Offset.
*
* @return	None.
*
*
* @note		None.
*
******************************************************************************/
void XCanFd_Set_Tranceiver_Delay_Compensation(XCanFd *InstancePtr,
						u32 TdcOffset)
{

	u32 RegValue=0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (TdcOffset <= TDC_MAX_OFFSET) {

		RegValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
					XCANFD_F_BRPR_OFFSET);

		TdcOffset <<= TDC_SHIFT;
		RegValue |= (TdcOffset & XCANFD_F_BRPR_TDCMASK);

		XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_F_BRPR_OFFSET,RegValue);
	}

}

/*****************************************************************************/
/**
*
* This function Disables the Transceiver delay compensation.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XCanFd_Disable_Tranceiver_Delay_Compensation(XCanFd *InstancePtr)
{
	u32 RegValue=0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	RegValue = XCanFd_ReadReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_F_BRPR_OFFSET);
	RegValue &= ~XCANFD_F_BRPR_TDC_ENABLE_MASK;
	XCanFd_WriteReg(InstancePtr->CanFdConfig.BaseAddress,
			XCANFD_F_BRPR_OFFSET,RegValue);
}

/*****************************************************************************/
/**
*
* This function calculates the index position of the right most set bit.
*
* @param	GlobalTrrValue
*
* @return	Index location of right most set bit
*
* @note		log2 fast Math logic.
*
******************************************************************************/
static int XCanfd_TrrVal_Get_SetBit_Position(u32 u) {

	u32 uCount;
	u32 lCount;

	u -= 1;
	uCount = u - ((u >> SHIFT1) & FAST_MATH_MASK1) - ((u >> SHIFT2) & FAST_MATH_MASK2);
	lCount = ((uCount + (uCount >> SHIFT3)) & FAST_MATH_MASK3) % EXTRACTION_MASK;
	return lCount;

}

/*****************************************************************************/
/**
* This function receives a CAN/CAN FD Frame as per the input Can Read Index
* value.
*
* @param	InstancePtr is a pointer to the XCanFd instance to be worked on.
* @param	Current RI(Read Index) maintained by Can Core.
* @param    Fifo Status Registor value.
* @param	FramePtr is a pointer to a 32-bit aligned buffer where the
*		    CAN/CAN FD frame is to be written.
* @param    Target fifo number
*
* @return	- XST_SUCCESS after all operations
*
* @note		This routine has generic logic for sequential receive mode
*
******************************************************************************/
static u32 XCanFd_SeqRecv_logic(XCanFd *InstancePtr, u32 ReadIndex, u32 FsrVal, u32 *FramePtr, u8 fifo_no)
{
	u32 DwIndex=0;
	u32 CanEDL;
	u32 Dlc=0;
	u32 Len;
/* Read ID from ID Register*/
	if (fifo_no == XCANFD_RX_FIFO_0) {
		FramePtr[0] = XCanFd_ReadReg(
				InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RXID_OFFSET(ReadIndex));
	} else {
		FramePtr[0] = XCanFd_ReadReg(
				InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_FIFO_1_RXID_OFFSET(ReadIndex));
	}

/* Read DLC from DLC Register*/
	if (fifo_no == XCANFD_RX_FIFO_0) {
		FramePtr[1] = CanEDL = XCanFd_ReadReg(
				InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_RXDLC_OFFSET(ReadIndex));
	} else {
		FramePtr[1] = CanEDL = XCanFd_ReadReg(
				InstancePtr->CanFdConfig.BaseAddress,
				XCANFD_FIFO_1_RXDLC_OFFSET(ReadIndex));
	}
		Dlc = XCanFd_GetDlc2len(FramePtr[1] & XCANFD_DLCR_DLC_MASK,
				(CanEDL & XCANFD_DLCR_EDL_MASK));

		if (CanEDL & XCANFD_DLCR_EDL_MASK) {

			/* Can Fd frames */
			for (Len = 0;Len < Dlc;Len += 4) {
				if (fifo_no == XCANFD_RX_FIFO_0) {
					FramePtr[2+DwIndex] = Xil_EndianSwap32(
					XCanFd_ReadReg(
					InstancePtr->CanFdConfig.BaseAddress,
					(XCANFD_RXDW_OFFSET(ReadIndex)+
					(DwIndex*XCANFD_DW_BYTES))));
				} else {
					FramePtr[2+DwIndex] = Xil_EndianSwap32(
					XCanFd_ReadReg(
					InstancePtr->CanFdConfig.BaseAddress,
					(XCANFD_FIFO_1_RXDW_OFFSET(ReadIndex)+
					(DwIndex*XCANFD_DW_BYTES))));
				}
				DwIndex++;
			}
		}
		else {

			/* Legacy CAN Frame */
			for (Len = 0;Len < Dlc;Len += 4) {
				if (fifo_no == XCANFD_RX_FIFO_0) {
					FramePtr[2+DwIndex] = Xil_EndianSwap32(
					XCanFd_ReadReg(
					InstancePtr->CanFdConfig.BaseAddress,
					(XCANFD_RXDW_OFFSET(ReadIndex)+(Len))));
				} else {
					FramePtr[2+DwIndex] = Xil_EndianSwap32(
					XCanFd_ReadReg(
					InstancePtr->CanFdConfig.BaseAddress,
					(XCANFD_FIFO_1_RXDW_OFFSET(ReadIndex)+(Len))));
				}
				DwIndex++;
			}
		}

		/* Set the IRI bit causes core to increment RI in FSR Register */
		if (fifo_no == XCANFD_RX_FIFO_0) {
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
/** @} */
