/*******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xmmidp_phy.c
 * @addtogroup mmi_dppsu14 Overview
 * @{
 *
 * @note        None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   ck   03/14/25	Initial release
 * </pre>
 *
*******************************************************************************/
/******************************* Include Files ********************************/
#include <stdlib.h>
#include <xstatus.h>
#include <sleep.h>

#include "xmmidp.h"

#define XMMIDP_AUX_MAX_DATA_WRITE_SIZE		0x4
#define XMMIDP_AUX_MAX_TIMEOUT_COUNT		50
#define XMMIDP_AUX_MAX_DEFER_COUNT		50
#define XMMIDP_AUX_MAX_WAIT			20000
#define XMMIDP_IS_CONNECTED_MAX_TIMEOUT_COUNT	50
#define XMMIDP_AUX_REPLY_TIMEOUT_TIMER		20
#define XMMIDP_AUX_REQUEST_TIMEOUT_TIMER	3200
#define XMMIDP_AUX_MAX_DATA_BYTES		16

#define XMMIDP_EDID_BLOCK_SIZE			128
#define XMMIDP_EDID_ADDR			0x50
#define XMMIDP_SEGPTR_ADDR			0x30
#define XMMIDP_MAX_SEGMENT_SIZE			256

/**
 * This typedef describes an AUX transaction.
 */
typedef struct {
	u32 Address;
	u8 *Data;
	u16 Cmd;
	u8 NumBytes;
} XMmiDp_AuxTransaction;

/******************************************************************************/
/**
 * This function is the delay/sleep function for the XMmiDp driver. *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       MicroSeconds is the number of microseconds to delay/sleep for.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_WaitUs(XMmiDp *InstancePtr, u32 MicroSeconds)
{
	Xil_AssertVoid(InstancePtr != NULL);

	if (!MicroSeconds) {
		return;
	}
	/* Wait the requested amount of time. */
	usleep(MicroSeconds);
}

/******************************************************************************/
/**
 * This function clears the AUX_DATA registers by setting it to 0.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 *
 * @note        None.
 *
*******************************************************************************/
static void XMmiDp_ClearAuxData(XMmiDp *InstancePtr)
{
	u32 Index;

	for (Index = 0; Index < 4; Index++) {
		XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
				XMMIDP_AUX_DATA0_0 + (Index * 4), 0);

	}

}

/******************************************************************************/
/**
 * This function programs the Aux data to the AUX_DATA registers
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Request is a pointer to an initialized XMmiDp_AuxTransaction
 *              structure containing the required information for issuing an AUX
 *              command.
 *
 * @note        None.
 *
*******************************************************************************/
static void XMmiDp_SetAuxData(XMmiDp *InstancePtr, XMmiDp_AuxTransaction *Request)
{

	u32 Index;
	u32 data[4];

	memset(data, 0, sizeof(u32) * 4);

	for (Index = 0; Index < Request->NumBytes; Index++) {
		data[Index / 4] |= (Request->Data[Index] << ((Index % 4) * 8));
	}

	for (Index = 0; Index < 4; Index++) {
		XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
				XMMIDP_AUX_DATA0_0 + (Index * 4), data[Index]);
	}

}

/******************************************************************************/
/**
 * This function reads the Aux data from AUX_DATA registers
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Request is a pointer to an initialized XMmiDp_AuxTransaction
 *              structure containing the required information for issuing an AUX
 *              command.
 *
 * @note        None.
 *
*******************************************************************************/
static u32 XMmiDp_GetAuxData(XMmiDp *InstancePtr, XMmiDp_AuxTransaction *Request)
{

	u32 Index;
	u32 data[4];

	u32 TimeoutCount = 0;
	u32 Status;
	u32 BytesRead = 0;

	memset(data, 0, sizeof(u32) * 4);

	/* Wait until all data has been received */
	do {
		Status = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
					XMMIDP_AUX_STATUS0);

		XMmiDp_WaitUs(InstancePtr, 100);
		TimeoutCount++;
		if (TimeoutCount >= XMMIDP_AUX_MAX_TIMEOUT_COUNT) {
			return XST_ERROR_COUNT_MAX;
		}

		BytesRead = (Status & XMMIDP_AUX_BYTES_READ_MASK) >>
			    XMMIDP_AUX_BYTES_READ_SHIFT;
		BytesRead -= 1;
	} while (BytesRead != Request->NumBytes);

	/* Read AUX_DATA_0 Registers */

	for (Index = 0; Index < 4; Index++) {
		data[Index] = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
					     XMMIDP_AUX_DATA0_0 + (Index * 4));
	}

	for (Index = 0; Index < Request->NumBytes; Index++) {
		Request->Data[Index] = (data[Index / 4] >> ((Index % 4) * 8)) & 0xFF;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function waits until another request is no longer in progress.
 * Check if GENERAL_INTERRUPT_AUX_REPLY_EVENT bit is cleared. If not then wait.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return
 *              - XST_SUCCESS if the the RX device is no longer busy.
 *              - XST_ERROR_COUNT_MAX otherwise, if a timeout has occurred.
 *
 * @note        None.
 *
*******************************************************************************/
static u32 XMmiDp_AuxWaitReady(XMmiDp *InstancePtr)
{
	u32 Status;
	u32 Timeout = 100;

	/* wait for GENERAL_INTERRUPT.AUX_REPLY_EVENT bit to be cleared */
	do {
		Status = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
					XMMIDP_GEN_INT0);

		if (!Timeout --) {
			return XST_ERROR_COUNT_MAX;
		}

		XMmiDp_WaitUs(InstancePtr, XMMIDP_AUX_REPLY_TIMEOUT_TIMER);

	} while (Status & XMMIDP_GEN_INT0_AUX_REPLY_EVENT_MASK);

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function waits for a reply indicating that the most recent AUX request
 * has been received by the RX device.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return
 *              - XST_SUCCESS if a reply was sent from the RX device.
 *              - XST_ERROR_COUNT_MAX otherwise, if a timeout has occurred.
 *
 * @note        None.
 *
*******************************************************************************/
static u32 XMmiDp_AuxWaitReply(XMmiDp *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	u32 Timeout = 100;
	u32 Status;
	u32 ReplyReceived = 0;
	u32 ReplyError = 0;

	while (0 < Timeout) {
		Status = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
					XMMIDP_AUX_STATUS0);

		ReplyError = (Status & XMMIDP_AUX_REPLY_ERR_MASK) >>
			     XMMIDP_AUX_REPLY_ERR_SHIFT;
		/* Check for error. */
		if (ReplyError) {
			return XST_ERROR_COUNT_MAX;
		}

		ReplyReceived = (Status & XMMIDP_AUX_REPLY_RECEIVED_MASK) >>
				XMMIDP_AUX_REPLY_RECEIVED_SHIFT;

		/* Check for a reply. */
		/* This bit resets to 0 when AUX transfer completes */
		if (ReplyReceived == XMMIDP_AUX_REPLY_RECEIVED) {
			return XST_SUCCESS;
		}

		(Timeout--);
		XMmiDp_WaitUs(InstancePtr, XMMIDP_AUX_REPLY_TIMEOUT_TIMER);
	}

	return XST_ERROR_COUNT_MAX;
}

/******************************************************************************/
/**
 * This function submits the supplied AUX request to the RX device over the AUX
 * channel by writing the command, the destination address, (the write buffer
 * for write commands), and the data size to the DisplayPort TX core.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Request is a pointer to an initialized XMmiDp_AuxTransaction
 *              structure containing the required information for issuing an AUX
 *              command.
 *
 * @return
 *              - XST_SUCCESS if the request was acknowledged.
 *              - XST_ERROR_COUNT_MAX if waiting for a reply timed out.
 *              - XST_SEND_ERROR if the request was deferred.
 *              - XST_FAILURE otherwise, if the request was NACK'ed.
 *
 * @note        None.
 *
*******************************************************************************/
static u32 XMmiDp_SetAuxRequest(XMmiDp *InstancePtr, XMmiDp_AuxTransaction *Request)
{

	u32 Status;
	u32 RegVal = 0;
	u32 TimeoutCount = 0;
	u32 AuxState = 0;

	/* Check if AUX Channel is currently ACTIVE/BUSY */
	do {
		Status = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_AUX_STATUS0);

		XMmiDp_WaitUs(InstancePtr, XMMIDP_AUX_REPLY_TIMEOUT_TIMER);
		TimeoutCount++;
		if (TimeoutCount >= XMMIDP_AUX_MAX_TIMEOUT_COUNT) {
			return XST_ERROR_COUNT_MAX;
		}

		AuxState = (Status & XMMIDP_AUX_REPLY_RECEIVED_MASK) >>
			   XMMIDP_AUX_REPLY_RECEIVED_SHIFT ;

	} while (AuxState == XMMIDP_AUX_REPLY_ACTIVE_TRANSFER);

	XMmiDp_ClearAuxData(InstancePtr);

	/* Program AUX_DATA 0/1/2/3 */
	if ((Request->Cmd == XMMIDP_AUX_CMD_NATIVE_WRITE) ||
	    (Request->Cmd == XMMIDP_AUX_CMD_I2C_WRITE) ||
	    (Request->Cmd == XMMIDP_AUX_CMD_I2C_MOT_WRITE)) {
		XMmiDp_SetAuxData(InstancePtr, Request);
	}

	/* Program AUX_CMD_TYPE, AUX_ADDR, AUX_LEN */
	RegVal = Request->NumBytes - 1;
	RegVal |= Request->Cmd << XMMIDP_AUX_CMD_AUX_CMD_TYPE_SHIFT;
	RegVal |= Request->Address << XMMIDP_AUX_CMD_AUX_ADDR_SHIFT;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_AUX_CMD0, RegVal);

	/* Wait for Aux Reply */
	Status = XMmiDp_AuxWaitReply(InstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Handle Aux Reply Status */
	Status = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr, XMMIDP_AUX_STATUS0);
	Status = Status & XMMIDP_AUX_STATUS_MASK;

	/* Request NACK'd, return error */
	if ((Status == XMMIDP_AUX_STATUS_AUX_NACK) ||
	    (Status == XMMIDP_AUX_STATUS_I2C_NACK)) {
		return XST_FAILURE;
	}

	/* Request defered, re-try again */
	if ((Status == XMMIDP_AUX_STATUS_AUX_DEFER) ||
	    (Status == XMMIDP_AUX_STATUS_I2C_DEFER)) {
		return XST_SEND_ERROR;
	}

	/* Request ACK'd, Read Reply */
	if ((Request->Cmd == XMMIDP_AUX_CMD_NATIVE_READ) ||
	    (Request->Cmd == XMMIDP_AUX_CMD_I2C_READ) ||
	    (Request->Cmd == XMMIDP_AUX_CMD_I2C_MOT_READ)) {
		Status = XMmiDp_GetAuxData(InstancePtr, Request);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	/* Clear Aux Reply Event bit */
	Status = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				XMMIDP_GEN_INT0);

	RegVal =  Status & (XMMIDP_GEN_INT0_AUX_REPLY_EVENT_MASK);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_GEN_INT0, RegVal);

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function submits the supplied AUX request to the RX device over the AUX
 * channel. If waiting for a reply times out, or if the DisplayPort TX core
 * indicates that the request was deferred, the request is sent again (up to a
 * maximum specified by XMMIDP_AUX_MAX_DEFER_COUNT|DPPSU14_AUX_MAX_TIMEOUT_COUNT).
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Request is a pointer to an initialized XMmiDp_AuxTransaction
 *              structure containing the required information for issuing an
 *              AUX command, as well as a write buffer used for write commands,
 *              and a read buffer for read commands.
 *
 * @return
 *              - XST_SUCCESS if the request was acknowledged.
 *              - XST_ERROR_COUNT_MAX if resending the request exceeded the
 *                maximum for deferral and timeout.
 *              - XST_FAILURE otherwise (if the DisplayPort TX core sees a NACK
 *                reply code or if the AUX transaction failed).
 *
 * @note        None.
 *
*******************************************************************************/
static u32 XMmiDp_AuxRequest(XMmiDp *InstancePtr, XMmiDp_AuxTransaction *Request)
{
	u32 Status;
	u32 DeferCount = 0;
	u32 TimeoutCount = 0;

	do {
		/*if Aux Request in progress */
		Status = XMmiDp_AuxWaitReady(InstancePtr);
		if (Status != XST_SUCCESS) {
			TimeoutCount++;
			continue;
		}

		Status = XMmiDp_SetAuxRequest(InstancePtr, Request);
		if (Status == XST_SEND_ERROR) {
			DeferCount++;
		} else if (Status == XST_ERROR_COUNT_MAX) {
			TimeoutCount++;
		} else {
			return Status;
		}

		XMmiDp_WaitUs(InstancePtr, XMMIDP_AUX_REQUEST_TIMEOUT_TIMER);

	} while ((DeferCount <  XMMIDP_AUX_MAX_DEFER_COUNT) &&
		 (TimeoutCount < XMMIDP_AUX_MAX_TIMEOUT_COUNT));

	return XST_ERROR_COUNT_MAX;
}

/******************************************************************************/
/**
 * This Api splits the Aux commands into multiple requests, each
 * acting on a maximum of 16 bytes.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       CmdType is the type of AUX command
 * @param       Address is the starting address that the AUX transaction will
 *              read/write from/to the RX device.
 * @param       NumBytes is the number of bytes to read/write from/to the RX
 *              device.
 * @param       Data is a pointer to the data buffer that contains the data
 *              to be read/written from/to the RX device.
 *
 * @return
 *              - XST_SUCCESS if the AUX transaction request was acknowledged.
 *              - XST_ERROR_COUNT_MAX if the AUX request timed out.
 *              - XST_FAILURE otherwise (if the DisplayPort TX core sees a NACK
 *                reply code or if the AUX transaction failed).
 *              - XST_DEVICE_NOT_FOUND if the sink is diconnected from the TX.
 *
 * @note        None.
 *
*******************************************************************************/
static u32 XMmiDp_AuxChanCommand(XMmiDp *InstancePtr, u32 AuxCmdType, u32
				 Address, u32 Bytes, u8 *Data)
{
	u32 Status;
	u32 BytesLeft;
	XMmiDp_AuxTransaction Request;

	Request.Address = Address;

	BytesLeft = Bytes;

	while (BytesLeft > 0) {
		Request.Cmd = AuxCmdType;

		if ((Request.Cmd == XMMIDP_AUX_CMD_NATIVE_WRITE) ||
		    (Request.Cmd == XMMIDP_AUX_CMD_NATIVE_READ)) {
			Request.Address = Address + (Bytes - BytesLeft);
		}

		Request.Data = &Data[Bytes - BytesLeft];

		if (BytesLeft > XMMIDP_AUX_MAX_DATA_BYTES) {
			Request.NumBytes = XMMIDP_AUX_MAX_DATA_BYTES;
		} else {
			Request.NumBytes = BytesLeft;
		}

		BytesLeft -= Request.NumBytes;

		if ((Request.Cmd == XMMIDP_AUX_CMD_I2C_WRITE) && (BytesLeft > 0)) {
			Request.Cmd == XMMIDP_AUX_CMD_I2C_MOT_WRITE;
		}

		if ((Request.Cmd == XMMIDP_AUX_CMD_I2C_READ) && (BytesLeft > 0)) {
			Request.Cmd == XMMIDP_AUX_CMD_I2C_MOT_READ;
		}

		XMmiDp_WaitUs(InstancePtr, XMMIDP_AUX_REQUEST_TIMEOUT_TIMER);

		Status = XMmiDp_AuxRequest(InstancePtr, &Request);
		if (Status != XST_SUCCESS) {
			return Status;
		}
	}

	return XST_SUCCESS;

}

/******************************************************************************/
/**
 * This function issues a write request over the AUX channel that will write to
 * the RX device's DisplayPort Configuration Data (DPCD) address space. The
 * write message will be divided into multiple transactions which write a
 * maximum of 16 bytes each.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       DpcdAddr is the starting address to write to the RX device.
 * @param       Bytes is the number of bytes to write to the RX device.
 * @param       Data is a pointer to the data buffer that contains the data
 *              to be written to the RX device.
 *
 * @return
 *              - XST_SUCCESS if the AUX write request was successfully
 *                acknowledged.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *              - XST_ERROR_COUNT_MAX if the AUX request timed out.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_AuxWrite(XMmiDp *InstancePtr, u32 DpcdAddr, u32 Bytes, void *Data)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DpcdAddr <= 0xFFFFF);
	Xil_AssertNonvoid(Bytes <= 0xFFFFF);
	Xil_AssertNonvoid(Data != NULL);

	return XMmiDp_AuxChanCommand(InstancePtr, XMMIDP_AUX_CMD_NATIVE_WRITE,
				     DpcdAddr, Bytes, (u8 *)Data);

}

/******************************************************************************/
/**
 * This function issues a read request over the AUX channel that will read from
 * the RX device's DisplayPort Configuration Data (DPCD) address space. The read
 * message will be divided into multiple transactions which read a maximum of 16
 * bytes each.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       DpcdAddress is the starting address to read from the RX device.
 * @param       Bytes is the number of bytes to read from the RX device.
 * @param       Data is a pointer to the data buffer that will be filled
 *              with read data.
 *
 * @return
 *              - XST_SUCCESS if the AUX read request was successfully
 *                acknowledged.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *              - XST_ERROR_COUNT_MAX if the AUX request timed out.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_AuxRead(XMmiDp *InstancePtr, u32 DpcdAddr, u32 Bytes, void *Data)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(DpcdAddr <= 0xFFFFF);
	Xil_AssertNonvoid(Bytes <= 0xFFFFF);
	Xil_AssertNonvoid(Data != NULL);

	return XMmiDp_AuxChanCommand(InstancePtr, XMMIDP_AUX_CMD_NATIVE_READ,
				     DpcdAddr, Bytes, (u8 *)Data);
}

/******************************************************************************/
/**
 * This function performs an I2C write over the AUX channel.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       IicAddress is the address on the I2C bus of the target device.
 * @param       BytesToWrite is the number of bytes to write.
 * @param       WriteData is a pointer to a buffer which will be used as the
 *              data source for the write.
 *
 * @return
 *              - XST_SUCCESS if the I2C write has successfully completed with
 *                no errors.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *              - XST_ERROR_COUNT_MAX if the AUX request timed out.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_I2cWrite(XMmiDp *InstancePtr, u32 I2cAddr, u32 Bytes, void *Data)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Bytes <= 0xFFFFF);
	Xil_AssertNonvoid(Data != NULL);

	return XMmiDp_AuxChanCommand(InstancePtr, XMMIDP_AUX_CMD_I2C_WRITE,
				     I2cAddr, Bytes, (u8 *)Data);
}

/******************************************************************************/
/**
 * This function performs an I2C read over the AUX channel. The read message
 *  will be divided into multiple transactions if the requested data spans
 *  multiple segmetns. The segment pointer is automatically incremented and
 *  the offset is calibrated as needed. E.g. For an overall offset of:
 *	- 128, an I2C read is done on segptr=0; offset=128.
 *	- 256, an I2C read is done on segptr=1; offset=0.
 *	- 384, an I2C read is done on segptr=1; offset=128.
 *	- 512, an I2C read is done on segptr=2; offset=0.
 *	- etc.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       IicAddress is the address on the I2C bus of the target device.
 * @param       Offset is the offset at specified address of targeted I2C device
		 that the read will start from.
 * @param	Bytes is the number of bytes to read.
 * @param	ReadData is a pointer to a buffer that will be filled with the
 *		I2C read data.
 *
 * @return
 *              - XST_SUCCESS if the I2C write has successfully completed with
 *                no errors.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *              - XST_ERROR_COUNT_MAX if the AUX request timed out.
 *              - XST_FAILURE otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_I2cRead(XMmiDp *InstancePtr, u32 I2cAddr, u16 Offset, u32 Bytes, void *Data)
{
	u32 Status;
	u16 BytesLeft;
	u8 Offset8;
	u8 SegPtr;
	u16 NumBytesLeftInSeg;
	u8 CurrBytesToRead;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Bytes <= 0xFFFFF);
	Xil_AssertNonvoid(Data != NULL);

	if (!XMmiDp_IsConnected(InstancePtr)) {
		return XST_DEVICE_NOT_FOUND;
	}

	BytesLeft = Bytes;

	SegPtr = 0;
	if ( Offset > (XMMIDP_MAX_SEGMENT_SIZE - 1)) {
		SegPtr += Offset / XMMIDP_MAX_SEGMENT_SIZE;
		Offset %= XMMIDP_MAX_SEGMENT_SIZE;
	}

	Offset8 = Offset;
	NumBytesLeftInSeg = XMMIDP_MAX_SEGMENT_SIZE - Offset8;

	XMmiDp_AuxChanCommand(InstancePtr, XMMIDP_AUX_CMD_I2C_MOT_WRITE, XMMIDP_SEGPTR_ADDR, 1, &SegPtr);

	while ( BytesLeft > 0 ) {

		/* Read the remaining number of bytes as requested. */
		if (NumBytesLeftInSeg >= BytesLeft) {
			CurrBytesToRead = BytesLeft;
		} else {
			/* Read the remaining data in the current segment boundary. */
			CurrBytesToRead = NumBytesLeftInSeg;
		}

		/* Setup the I2C-over-AUX read transaction with the offset. */
		Status = XMmiDp_AuxChanCommand(InstancePtr,
					       XMMIDP_AUX_CMD_I2C_MOT_WRITE, I2cAddr, 1,
					       &Offset8);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		Status = XMmiDp_AuxChanCommand(InstancePtr, XMMIDP_AUX_CMD_I2C_READ,
					       I2cAddr, CurrBytesToRead, (u8 *)Data);
		if (Status != XST_SUCCESS) {
			return Status;
		}

		/* Previous I2C read was done on the remaining data in the
		 * current segment; prepare for next read. */
		if (BytesLeft > CurrBytesToRead) {
			BytesLeft -= CurrBytesToRead;
			Offset += CurrBytesToRead;
			Data += CurrBytesToRead;

			/* Increment the segment pointer to access more I2C
			 * address space, if required. */
			if (BytesLeft > 0) {
				NumBytesLeftInSeg = XMMIDP_MAX_SEGMENT_SIZE;
				Offset %= XMMIDP_MAX_SEGMENT_SIZE;
				SegPtr++;

				XMmiDp_AuxChanCommand(InstancePtr,
						      XMMIDP_AUX_CMD_I2C_MOT_WRITE, XMMIDP_SEGPTR_ADDR, 1,
						      &SegPtr);

			}
			Offset8 = Offset;
		} else {
			/* Last I2C read. */
			BytesLeft = 0;
		}
	}

	return Status;
}

/******************************************************************************/
/**
 * This helper api reads value from register, computes the new value and writes
 * to the register
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param	Register offset to read/write values
 * @param	Mask value
 * @param	Shift value
 * @param	Val to be written
 *
 * @return
 *              - None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_RegReadModifyWrite(XMmiDp *InstancePtr, u32 RegOffset,
			       u32 Mask, u32 Shift, u32 Val)
{
	u32 RegVal;

	RegVal = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				RegOffset);
	RegVal &= ~Mask;
	RegVal |= Val << Shift;

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, RegOffset, RegVal);
}

/******************************************************************************/
/**
 * This helper api reads value from DPCD, computes the new value and writes
 * to the DPCD register
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param	Register offset to read/write values
 * @param	Mask value
 * @param	Shift value
 * @param	Val to be written
 *
 * @return
 *              - None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_DpcdReadModifyWrite(XMmiDp *InstancePtr, u32 DpcdReg,
				u32 Mask, u32 Shift, u32 Val)
{
	u32 DpcdVal;

	/* Read Rx DPCD register val */
	XMmiDp_AuxRead(InstancePtr, DpcdReg, 0x1, &DpcdVal);

	DpcdVal &= ~Mask;
	DpcdVal |= Val << Shift;

	/* Write the new lane count to the RX device. */
	XMmiDp_AuxWrite(InstancePtr, DpcdReg, 0x1, &DpcdVal);

}

u8 XMmiDp_GetLaneCount(XMmiDp *InstancePtr, u8 NumLanes)
{
	if (NumLanes == 4) {
		return XMMIDP_PHY_LANES_4;
	} else if (NumLanes == 2) {
		return XMMIDP_PHY_LANES_2;
	} else {
		return XMMIDP_PHY_LANES_1;
	}
}

u8 XMmiDp_GetNumLanes(XMmiDp *InstancePtr, u8 LaneCount)
{

	if (LaneCount == XMMIDP_PHY_LANES_1) {
		return 0x1;
	} else if (LaneCount == XMMIDP_PHY_LANES_2) {
		return 0x2;
	} else if (LaneCount == XMMIDP_PHY_LANES_4) {
		return 0x4;
	} else {
		return 0;
	}
}

/******************************************************************************/
/**
 * This function sets the number of lanes to be used by the main link for both
 * the DisplayPort TX core and the RX device.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       LaneCount is the number of lanes to be used over the main link.
 *
 * @return
 *              - None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetDpcdLaneCount(XMmiDp *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_DpcdReadModifyWrite(InstancePtr, XMMIDP_DPCD_LANE_COUNT_SET,
				   XMMIDP_DPCD_LANE_COUNT_SET_MASK, 0, InstancePtr->PhyConfig.NumLanes);

}

/******************************************************************************/
/**
 * This function sets the number of lanes to be used by the main link for both
 * the DisplayPort TX core and the RX device.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       LaneCount is the number of lanes to be used over the main link.
 *
 * @return
 *              - None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetPhyLaneCount(XMmiDp *InstancePtr, XMmiDp_PhyLanes LaneCount)
{
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->PhyConfig.LaneCount = LaneCount;

	/* Get Number of Lanes from LaneCount option */
	InstancePtr->PhyConfig.NumLanes = XMmiDp_GetNumLanes(InstancePtr, LaneCount);

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_PHYIF_CTRL_0,
				  XMMIDP_PHYIF_CTRL_0_PHY_LANES_MASK,
				  XMMIDP_PHYIF_CTRL_0_PHY_LANES_SHIFT,
				  InstancePtr->PhyConfig.LaneCount);

}

u16 XMmiDp_GetLinkRate(XMmiDp *InstancePtr, u8 LinkBW)
{
	u16 link_rate;
	if (LinkBW == XMMIDP_DPCD_LINK_BW_SET_270GBPS) {
		link_rate = XMMIDP_HBR_LINK_RATE;
	} else if (LinkBW == XMMIDP_DPCD_LINK_BW_SET_540GBPS) {
		link_rate = XMMIDP_HBR2_LINK_RATE;
	} else if (LinkBW == XMMIDP_DPCD_LINK_BW_SET_810GBPS) {
		link_rate = XMMIDP_HBR3_LINK_RATE;
	} else {
		link_rate = XMMIDP_RBR_LINK_RATE;
	}
	return link_rate;
}

u8 XMmiDp_GetLinkBW(XMmiDp *InstancePtr, u8 LinkRate)
{
	if (LinkRate == XMMIDP_PHY_RATE_RBR_162GBPS) {
		return XMMIDP_DPCD_LINK_BW_SET_162GBPS;
	} else if (LinkRate == XMMIDP_PHY_RATE_HBR_270GBPS) {
		return XMMIDP_DPCD_LINK_BW_SET_270GBPS;
	} else if (LinkRate == XMMIDP_PHY_RATE_HBR2_540GBPS) {
		return XMMIDP_DPCD_LINK_BW_SET_540GBPS;
	} else if (LinkRate == XMMIDP_PHY_RATE_HBR3_810GBPS) {
		return XMMIDP_DPCD_LINK_BW_SET_810GBPS;
	} else {
		return 0;
	}
}

/******************************************************************************/
/**
 * This function sets the dpcd link bw to be used by the main link for both the
 * DisplayPort TX core and the RX device.
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       LinkRate is the link rate to be used over the main link based on
 *              one of the following selects:
 *              - XMMIDP_LINK_BW_SET_162GBPS = 0x06 (for a 1.62 Gbps data rate)
 *              - XMMIDP_LINK_BW_SET_270GBPS = 0x0A (for a 2.70 Gbps data rate)
 *              - XMMIDP_LINK_BW_SET_540GBPS = 0x14 (for a 5.40 Gbps data rate)
 *              - XMMIDP_LINK_BW_SET_810GBPS = 0x1E (for a 8.10 Gbps data rate)
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetDpcdLinkRate(XMmiDp *InstancePtr)
{
	u32 DpcdVal = 0;

	Xil_AssertVoid(InstancePtr != NULL);

	DpcdVal = InstancePtr->PhyConfig.LinkBW;

	/* Write the new lane count to the RX device. */
	XMmiDp_AuxWrite(InstancePtr, XMMIDP_DPCD_LINK_BW_SET, 0x1,
			&DpcdVal);
}

/******************************************************************************/
/**
 * This function sets the data rate to be used by the main link for both the
 * DisplayPort TX core and the RX device.
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       LinkRate is the link rate to be used over the main link based on
 *              one of the following selects:
 *              - XMMIDP_LINK_BW_SET_162GBPS = 0x06 (for a 1.62 Gbps data rate)
 *              - XMMIDP_LINK_BW_SET_270GBPS = 0x0A (for a 2.70 Gbps data rate)
 *              - XMMIDP_LINK_BW_SET_540GBPS = 0x14 (for a 5.40 Gbps data rate)
 *              - XMMIDP_LINK_BW_SET_810GBPS = 0x1E (for a 8.10 Gbps data rate)
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetPhyLinkRate(XMmiDp *InstancePtr, XMmiDp_PhyRate LinkRate)
{
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->PhyConfig.LinkRate = LinkRate;
	InstancePtr->PhyConfig.LinkBW = XMmiDp_GetLinkBW(InstancePtr, LinkRate);

	/* Set 20-bit PHY width */
	XMmiDp_SetPhyWidth(InstancePtr, XMMIDP_PHY_20BIT);

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_PHYIF_CTRL_0,
				  XMMIDP_PHYIF_CTRL_0_PHY_RATE_MASK,
				  XMMIDP_PHYIF_CTRL_0_PHY_RATE_SHIFT,
				  InstancePtr->PhyConfig.LinkRate);

}

/******************************************************************************/
/**
 * This function sets the DPCD Link qual_0 training pattern to be used by the main link for both the
 * DisplayPort TX core and the RX device.
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Pattern select to be used over the main link based on
 *              one of the following selects:
 *		 XMMIDP_PHY_NO_TRAIN = 0x0,
 *		 XMMIDP_PHY_TPS1 = 0x1,
 *		 XMMIDP_PHY_TPS2 = 0x2,
 *		 XMMIDP_PHY_TPS3 = 0x3,
 *		 XMMIDP_PHY_TPS4 = 0x4,
 *		 XMMIDP_PHY_SYMBOL_ERR_RATE = 0x5,
 *		 XMMIDP_PHY_PRBS7 = 0x6,
 *		 XMMIDP_PHY_CUSTOMPAT = 0x7,
 *		 XMMIDP_PHY_CP2520_PAT_1 = 0x8,
 *		 XMMIDP_PHY_CP2520_PAT_2 = 0x9,
 *
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetDpcdLinkQualPattern(XMmiDp *InstancePtr,
				   XMmiDp_PhyTrainingPattern Pattern)
{
	u32 Val = 0x0;

	u32 DpcdReg = XMMIDP_DPCD_LINK_QUAL_LANE0_SET;
	u32 Shift = XMMIDP_DPCD_LINK_QUAL_PATTERN_SET_SHIFT;
	u32 Mask = XMMIDP_DPCD_LINK_QUAL_PATTERN_SET_MASK;

	if (Pattern == XMMIDP_PHY_SYMBOL_ERR_RATE) {
		Val = 0x2;
	} else if (Pattern == XMMIDP_PHY_PRBS7) {
		Val = 0x3;
	} else if (Pattern == XMMIDP_PHY_CUSTOMPAT) {
		Val = 0x4;
	} else if (Pattern == XMMIDP_PHY_CP2520_PAT_1) {
		Val = 0x5;
	} else if (Pattern == XMMIDP_PHY_CP2520_PAT_2) {
		Val = 0x6;
	} else {
		Val = 0x0;
	}

	for (int Index = 0; Index < InstancePtr->PhyConfig.NumLanes; Index++) {
		DpcdReg += Index;
		XMmiDp_DpcdReadModifyWrite(InstancePtr, DpcdReg,
					   Mask, Shift, Val);
		XMmiDp_WaitUs(InstancePtr, 1000);
	}

}

/******************************************************************************/
/**
 * This function sets the DPCD training pattern to be used by the main link for both the
 * DisplayPort TX core and the RX device.
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Pattern select to be used over the main link based on
 *              one of the following selects:
 *		 XMMIDP_PHY_NO_TRAIN = 0x0,
 *		 XMMIDP_PHY_TPS1 = 0x1,
 *		 XMMIDP_PHY_TPS2 = 0x2,
 *		 XMMIDP_PHY_TPS3 = 0x3,
 *		 XMMIDP_PHY_TPS4 = 0x4,
 *		 XMMIDP_PHY_SYMBOL_ERR_RATE = 0x5,
 *		 XMMIDP_PHY_PRBS7 = 0x6,
 *		 XMMIDP_PHY_CUSTOMPAT = 0x7,
 *		 XMMIDP_PHY_CP2520_PAT_1 = 0x8,
 *		 XMMIDP_PHY_CP2520_PAT_2 = 0x9,
 *
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetDpcdTrainingPattern(XMmiDp *InstancePtr,
				   XMmiDp_PhyTrainingPattern Pattern)
{
	u32 Val = 0x0;
	u32 DpcdVal = 0x0;
	u32 ScrambleEn = 0x1;

	u32 DpcdReg = XMMIDP_DPCD_TRAINING_PATTERN_SET;
	u32 Shift = XMMIDP_DPCD_TRAINING_PATTERN_SELECT_SHIFT;
	u32 Mask = XMMIDP_DPCD_TRAINING_PATTERN_SELECT_MASK;

	Val = Pattern;

	if (Pattern == XMMIDP_PHY_NO_TRAIN) {
		ScrambleEn = 0x0;
	}

	if (Pattern == XMMIDP_PHY_TPS4) {
		Val = 0x7;
	}

	/* Read from Rx DPCDP */
	XMmiDp_AuxRead(InstancePtr, DpcdReg, 0x1, &DpcdVal);
	DpcdVal &= ~Mask;
	DpcdVal |= Val << Shift;

	DpcdVal |= (ScrambleEn) << XMMIDP_DPCD_SCRAMBLING_DISABLE_SHIFT;

	/* Write VsLevel to  RX device. */
	XMmiDp_AuxWrite(InstancePtr, DpcdReg, 0x1, &DpcdVal);

	XMmiDp_WaitUs(InstancePtr, 1000);

}

/******************************************************************************/
/**
 * This function sets the training pattern to be used by the main link for both the
 * DisplayPort TX core and the RX device.
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Pattern select to be used over the main link based on
 *              one of the following selects:
 *		 XMMIDP_PHY_NO_TRAIN = 0x0,
 *		 XMMIDP_PHY_TPS1 = 0x1,
 *		 XMMIDP_PHY_TPS2 = 0x2,
 *		 XMMIDP_PHY_TPS3 = 0x3,
 *		 XMMIDP_PHY_TPS4 = 0x4,
 *		 XMMIDP_PHY_SYMBOL_ERR_RATE = 0x5,
 *		 XMMIDP_PHY_PRBS7 = 0x6,
 *		 XMMIDP_PHY_CUSTOMPAT = 0x7,
 *		 XMMIDP_PHY_CP2520_PAT_1 = 0x8,
 *		 XMMIDP_PHY_CP2520_PAT_2 = 0x9,
 *
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetPhyTrainingPattern(XMmiDp *InstancePtr,
				  XMmiDp_PhyTrainingPattern Pattern)
{
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->PhyConfig.TrainingPattern = Pattern;

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_PHYIF_CTRL_0,
				  XMMIDP_PHYIF_CTRL_0_TPS_SEL_MASK,
				  XMMIDP_PHYIF_CTRL_0_TPS_SEL_SHIFT,
				  Pattern);
}

static void XMmiDp_GetVoltageSwingMask(u32 Lane, u32 *VsShift, u32 *VsMask)
{

	if (Lane == 0) {
		*VsShift = XMMIDP_PHY_TX_EQ_LANE0_TX_VSWING_SHIFT;
		*VsMask = XMMIDP_PHY_TX_EQ_LANE0_TX_VSWING_MASK;
	}

	if (Lane == 1) {
		*VsShift = XMMIDP_PHY_TX_EQ_LANE1_TX_VSWING_SHIFT;
		*VsMask = XMMIDP_PHY_TX_EQ_LANE1_TX_VSWING_MASK;
	}

	if (Lane == 2) {
		*VsShift = XMMIDP_PHY_TX_EQ_LANE2_TX_VSWING_SHIFT;
		*VsMask = XMMIDP_PHY_TX_EQ_LANE2_TX_VSWING_MASK;
	}

	if (Lane == 3) {
		*VsShift = XMMIDP_PHY_TX_EQ_LANE3_TX_VSWING_SHIFT;
		*VsMask = XMMIDP_PHY_TX_EQ_LANE3_TX_VSWING_MASK;
	}

}

/**
 * This function sets the DPCD VoltageSwing levels int the
 * DisplayPort TX core
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       VoltageSwing for each lane to be used over the main link based on
 *              one of the following selects:
 *		- XMMIDP_PHY_VSWING_LEVEL0
 *		- XMMIDP_PHY_VSWING_LEVEL1
 *		- XMMIDP_PHY_VSWING_LEVEL2
 *		- XMMIDP_PHY_VSWING_LEVEL3
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetDpcdVoltageSwing(XMmiDp *InstancePtr, XMmiDp_PhyVSwing *VsLevel)
{

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(VsLevel != NULL);

	for (u32 Index = 0; Index < InstancePtr->PhyConfig.NumLanes; Index++) {
		XMmiDp_DpcdReadModifyWrite(InstancePtr,
					   XMMIDP_DPCD_TRAINING_LANE0_SET + Index,
					   XMMIDP_DPCD_VOLTAGE_SWING_SET_MASK,
					   XMMIDP_DPCD_VOLTAGE_SWING_SET_SHIFT,
					   (u32)VsLevel[Index]);
	}
}

/**
 * This function sets the VoltageSwing levels in the
 * DisplayPort TX core
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       VoltageSwing for each lane to be used over the main link based on
 *              one of the following selects:
 *		- XMMIDP_PHY_VSWING_LEVEL0
 *		- XMMIDP_PHY_VSWING_LEVEL1
 *		- XMMIDP_PHY_VSWING_LEVEL2
 *		- XMMIDP_PHY_VSWING_LEVEL3
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetPhyVoltageSwing(XMmiDp *InstancePtr, u8 *VsLevel)
{
	u32 Index;
	u32 VsShift = 0;
	u32 VsMask = 0;
	u32 Val = 0;

	for (Index = 0; Index < InstancePtr->PhyConfig.NumLanes; Index++) {
		XMmiDp_GetVoltageSwingMask(Index, &VsShift, &VsMask);

		InstancePtr->PhyConfig.VsLevel[Index] = VsLevel[Index];

		Val = VsLevel[Index];
		XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_PHY_TX_EQ,
					  VsMask, VsShift, Val);
	}
}

static void XMmiDp_GetPreEmphasisMask(u32 Lane, u32 *PeShift, u32 *PeMask)
{
	if (Lane == 0) {
		*PeShift = XMMIDP_PHY_TX_EQ_LANE0_TX_PREEMP_SHIFT;
		*PeMask = XMMIDP_PHY_TX_EQ_LANE0_TX_PREEMP_MASK;
	}

	if (Lane == 1) {
		*PeShift = XMMIDP_PHY_TX_EQ_LANE1_TX_PREEMP_SHIFT;
		*PeMask = XMMIDP_PHY_TX_EQ_LANE1_TX_PREEMP_MASK;
	}

	if (Lane == 2) {
		*PeShift = XMMIDP_PHY_TX_EQ_LANE2_TX_PREEMP_SHIFT;
		*PeMask = XMMIDP_PHY_TX_EQ_LANE2_TX_PREEMP_MASK;
	}

	if (Lane == 3) {
		*PeShift = XMMIDP_PHY_TX_EQ_LANE3_TX_PREEMP_SHIFT;
		*PeMask = XMMIDP_PHY_TX_EQ_LANE3_TX_PREEMP_MASK;
	}

}

/**
 * This function sets the DPCD VoltagePreEmphasis levels for the
 * DisplayPort TX PHY
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       VoltageSwing for each lane to be used over the main link based on
 *              one of the following selects:
 *		- PHYPREEMP_LEVEL0
 *		- PHYPREEMP_LEVEL1
 *		- PHYPREEMP_LEVEL2
 *		- PHYPREEMP_LEVEL3
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetDpcdPreEmphasis(XMmiDp *InstancePtr, XMmiDp_PhyPreEmp *PeLevel)
{
	u32 Index;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(PeLevel != NULL);

	for (Index = 0; Index < InstancePtr->PhyConfig.NumLanes; Index++) {

		InstancePtr->PhyConfig.PeLevel[Index] = PeLevel[Index];

		XMmiDp_DpcdReadModifyWrite(InstancePtr,
					   XMMIDP_DPCD_TRAINING_LANE0_SET + Index,
					   XMMIDP_DPCD_PREEMPHASIS_SET_MASK,
					   XMMIDP_DPCD_PREEMPHASIS_SET_SHIFT,
					   PeLevel[Index]);
	}

}

/**
 * This function sets the VoltagePreEmphasis levels for the
 * DisplayPort TX PHY
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       VoltageSwing for each lane to be used over the main link based on
 *              one of the following selects:
 *		- XMMIDP_PHY_PREEMP_LEVEL0
 *		- XMMIDP_PHY_PREEMP_LEVEL1
 *		- XMMIDP_PHY_PREEMP_LEVEL2
 *		- XMMIDP_PHY_PREEMP_LEVEL3
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetPhyPreEmphasis(XMmiDp *InstancePtr, u8 *PeLevel)
{
	u32 Index;
	u32 PeShift = 0;
	u32 PeMask = 0;
	u32 Val = 0;

	for (Index = 0; Index < InstancePtr->PhyConfig.NumLanes; Index++) {
		XMmiDp_GetPreEmphasisMask(Index, &PeShift, &PeMask);

		InstancePtr->PhyConfig.PeLevel[Index] = PeLevel[Index];
		Val = PeLevel[Index];

		XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_PHY_TX_EQ,
					  PeMask, PeShift, Val);

	}

}

/******************************************************************************/
/**
 * This function enables phy transfer for lanes
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetPhyXmitEnable(XMmiDp *InstancePtr)
{
	XMmiDp_PhyXmitEn XmitEn = 0;

	Xil_AssertVoid(InstancePtr != NULL);

	switch ( InstancePtr->PhyConfig.LaneCount ) {
		case 2:
			XmitEn |= XMMIDP_PHY_XMIT_EN_LANE3 |
				  XMMIDP_PHY_XMIT_EN_LANE2;
		/* fall through */
		case 1:
			XmitEn |= XMMIDP_PHY_XMIT_EN_LANE1;
		/* fall through */
		case 0:
			XmitEn |= XMMIDP_PHY_XMIT_EN_LANE0;
			break;
	}

	InstancePtr->PhyConfig.XmitEn = XmitEn;

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_PHYIF_CTRL_0,
				  XMMIDP_PHYIF_CTRL_0_XMIT_ENABLE_MASK,
				  XMMIDP_PHYIF_CTRL_0_XMIT_ENABLE_SHIFT,
				  XmitEn);
}

/******************************************************************************/
/**
 * This function disables phy transfer for lanes
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetPhyXmitDisable(XMmiDp *InstancePtr)
{

	Xil_AssertVoid(InstancePtr != NULL);

	/* Current Phy XmitEn */
	XMmiDp_PhyXmitEn XmitEn = InstancePtr->PhyConfig.XmitEn;

	switch ( InstancePtr->PhyConfig.LaneCount ) {
		case 2:
			XmitEn &= (~XMMIDP_PHY_XMIT_EN_LANE3) &
				  (~XMMIDP_PHY_XMIT_EN_LANE2);
		/* fall through */
		case 1:
			XmitEn &= ~XMMIDP_PHY_XMIT_EN_LANE1;
		/* fall through */
		case 0:
			XmitEn &= ~XMMIDP_PHY_XMIT_EN_LANE0;
			break;
	}

	/* Save new Phy XmitEn */
	InstancePtr->PhyConfig.XmitEn = XmitEn;

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_PHYIF_CTRL_0,
				  XMMIDP_PHYIF_CTRL_0_XMIT_ENABLE_MASK,
				  XMMIDP_PHYIF_CTRL_0_XMIT_ENABLE_SHIFT, XmitEn);

}

/******************************************************************************/
/**
 * This function enables phy scrambler over the main link through CCTL register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_PhyScrambleEnable(XMmiDp *InstancePtr)
{
	InstancePtr->PhyConfig.ScrambleEn = 0;

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_CCTL0,
				  XMMIDP_CCTL0_SCRAMBLE_DIS_MASK,
				  XMMIDP_CCTL0_SCRAMBLE_DIS_SHIFT,
				  InstancePtr->PhyConfig.ScrambleEn);

}

/******************************************************************************/
/**
 * This function disable phy scrambler
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_PhyScrambleDisable(XMmiDp *InstancePtr)
{
	InstancePtr->PhyConfig.ScrambleEn = 1;

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_CCTL0,
				  XMMIDP_CCTL0_SCRAMBLE_DIS_MASK,
				  XMMIDP_CCTL0_SCRAMBLE_DIS_SHIFT,
				  InstancePtr->PhyConfig.ScrambleEn);

}

/******************************************************************************/
/**
 * This function checks if there is a connected RX device.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return
 *              - 1 if there is a connection.
 *              - 0 if there is no connection.
 *
*******************************************************************************/
u32 XMmiDp_IsConnected(XMmiDp *InstancePtr)
{
	u32 Status;
	u8 Retries = 0;
	u32 RegVal = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	do {
		RegVal = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
					XMMIDP_HPD_STATUS0);

		Status = (RegVal & XMMIDP_HPD_STATUS0_STATUS_MASK) >>
			 XMMIDP_HPD_STATUS0_STATUS_SHIFT;

		if (Retries > XMMIDP_IS_CONNECTED_MAX_TIMEOUT_COUNT) {
			return 0;
		}

		Retries++;
		XMmiDp_WaitUs(InstancePtr, 1000);

	} while (Status == XMMIDP_HPD_STATUS0_NOT_CON);

	return 1;
}

/******************************************************************************/
/**
 * This function intializes the configuration for the XMmiDp Instance.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       BaseAddr sets the base address of the DP instance
 *
 * @return      None.
 *
 *
*******************************************************************************/
void XMmiDp_CfgInitialize(XMmiDp *InstancePtr, u32 BaseAddr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	InstancePtr->Config.BaseAddr = BaseAddr;
}

/******************************************************************************/
/**
 * This function waits for the DisplayPort PHY to come out of reset.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 *
 * @return
 *              - XST_ERROR_COUNT_MAX if the PHY failed to be ready.
 *              - XST_SUCCESS otherwise.
 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_PhyWaitReady(XMmiDp *InstancePtr)
{
	u32 Timeout = 100;
	u32 BusyLanes = 0;
	u32 RegVal = 0;

	switch ( InstancePtr->PhyConfig.LaneCount ) {
		case 2:
			BusyLanes |= XMMIDP_PHY_BUSY_LANE3 |
				     XMMIDP_PHY_BUSY_LANE2;
		case 1:
			BusyLanes |= XMMIDP_PHY_BUSY_LANE1;
		case 0:
			BusyLanes |= XMMIDP_PHY_BUSY_LANE0;
			break;
	}

	/* Wait until the PHY is ready. */
	do {
		XMmiDp_WaitUs(InstancePtr, 2000);
		RegVal = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
					XMMIDP_PHYIF_CTRL_0);

		RegVal &= XMMIDP_PHYIF_CTRL_0_PHY_BUSY_MASK;
		RegVal |= RegVal >> XMMIDP_PHYIF_CTRL_0_PHY_BUSY_SHIFT;

		/* Protect against an infinite loop. */
		if (!Timeout--) {
			return XST_ERROR_COUNT_MAX;
		}

	} while ((RegVal & BusyLanes) != 0x0);

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function does a PHY Soft reset.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Reset is the type of reset to assert.
 *
 * @return      None.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_PhySoftReset(XMmiDp *InstancePtr)
{
	u32 PhyVal;
	u32 RegVal;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Preserve the current PHY settings. */
	PhyVal = XMmiDp_ReadReg(InstancePtr->Config.BaseAddr,
				XMMIDP_SFT_RST_CTRL0);

	/* Apply reset. */
	RegVal = PhyVal | XMMIDP_SFT_RST_CTRL0_AUX_RST_MASK;
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_SFT_RST_CTRL0, RegVal);

	XMmiDp_WaitUs(InstancePtr, 10);

	/* Remove the reset. */
	RegVal = PhyVal & (~XMMIDP_SFT_RST_CTRL0_AUX_RST_MASK);
	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr,
			XMMIDP_SFT_RST_CTRL0, RegVal);

	/* Wait for the PHY to be ready. */
	XMmiDp_PhyWaitReady(InstancePtr);

}

/******************************************************************************/
/**
 * This function retrieves connected RX devices Extended Display Identification
 * given the block number. A block num of 0 represents the base EDID.
 *
 * @param       InstancePtr is a pointer to the XMmiDp instance.
 * @param       Data is a pointer to the data buffer to save the block data to
 * @param       BlockNum is the EDID block number to retrieve.
 *
 * @return
 *              - XST_SUCCESS if the block read has successfully completed with
 *                no errors.
 *              - XST_ERROR_COUNT_MAX if a time out occurred while attempting to
 *                read the requested block.
 *              - XST_DEVICE_NOT_FOUND if no RX device is connected.
 *              - XST_FAILURE otherwise.

 *
 * @note        None.
 *
*******************************************************************************/
u32 XMmiDp_GetEdidBlock(XMmiDp *InstancePtr, u8 *Data, u8 BlockNum)
{
	u32 Status;
	u16 Offset;

	/* Calculate the I2C offset for the specified EDID block. */
	Offset = BlockNum * XMMIDP_EDID_BLOCK_SIZE;

	/* Issue the I2C read for the specified EDID block. */
	Status = XMmiDp_I2cRead(InstancePtr, XMMIDP_EDID_ADDR, Offset,
				XMMIDP_EDID_BLOCK_SIZE, Data);

	return Status;

}

/******************************************************************************/
/**
 * This function enables SSC on the PHY
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_PhySSCEnable(XMmiDp *InstancePtr)
{
	InstancePtr->PhyConfig.SSCEn = 0;

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_PHYIF_CTRL_0,
				  XMMIDP_PHYIF_CTRL_0_SSC_DIS_MASK,
				  XMMIDP_PHYIF_CTRL_0_SSC_DIS_SHIFT,
				  InstancePtr->PhyConfig.SSCEn);

}

/******************************************************************************/
/**
 * This function disables SSC on the PHY
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_PhySSCDisable(XMmiDp *InstancePtr)
{
	InstancePtr->PhyConfig.SSCEn = 1;

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_PHYIF_CTRL_0,
				  XMMIDP_PHYIF_CTRL_0_SSC_DIS_MASK,
				  XMMIDP_PHYIF_CTRL_0_SSC_DIS_SHIFT,
				  InstancePtr->PhyConfig.SSCEn);

}

/******************************************************************************/
/**
 * This function Initializes the XMmiDp Instance structure
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_Initialize(XMmiDp *InstancePtr)
{

	memset(&InstancePtr->PhyConfig, 0, sizeof(XMmiDp_PhyConfig));
	memset(&InstancePtr->RxConfig, 0, sizeof(XMmiDp_RxConfig));
	memset(&InstancePtr->LinkConfig, 0, sizeof(XMmiDp_LinkConfig));

	memset(&InstancePtr->MsaConfig[0], 0, sizeof(XMmiDp_MainStreamAttributes));
	memset(&InstancePtr->MsaConfig[1], 0, sizeof(XMmiDp_MainStreamAttributes));
	memset(&InstancePtr->MsaConfig[2], 0, sizeof(XMmiDp_MainStreamAttributes));
	memset(&InstancePtr->MsaConfig[3], 0, sizeof(XMmiDp_MainStreamAttributes));

	memset(&InstancePtr->VideoConfig[0], 0, sizeof(XMmiDp_VideoConfig));
	memset(&InstancePtr->VideoConfig[1], 0, sizeof(XMmiDp_VideoConfig));
	memset(&InstancePtr->VideoConfig[2], 0, sizeof(XMmiDp_VideoConfig));
	memset(&InstancePtr->VideoConfig[3], 0, sizeof(XMmiDp_VideoConfig));

	memset(&InstancePtr->VSampleCtrl[0], 0, sizeof(XMmiDp_VSampleCtrl));
	memset(&InstancePtr->VSampleCtrl[1], 0, sizeof(XMmiDp_VSampleCtrl));
	memset(&InstancePtr->VSampleCtrl[2], 0, sizeof(XMmiDp_VSampleCtrl));
	memset(&InstancePtr->VSampleCtrl[3], 0, sizeof(XMmiDp_VSampleCtrl));

	memset(&InstancePtr->CtrlConfig[0], 0, sizeof(XMmiDp_Controller));
	memset(&InstancePtr->CtrlConfig[1], 0, sizeof(XMmiDp_Controller));
	memset(&InstancePtr->CtrlConfig[2], 0, sizeof(XMmiDp_Controller));
	memset(&InstancePtr->CtrlConfig[3], 0, sizeof(XMmiDp_Controller));

}

/******************************************************************************/
/**
 * This function sets the phy powerdown control val
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Powerdown val is the link rate to be used over the main link based on
 *              one of the following selects:
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetPhyPowerdown(XMmiDp *InstancePtr, XMmiDp_PhyPwrDown Control)
{
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->PhyConfig.PwrDown = Control;

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_PHYIF_CTRL_0,
				  XMMIDP_PHYIF_CTRL_0_PHY_POWERDOWN_MASK,
				  XMMIDP_PHYIF_CTRL_0_PHY_POWERDOWN_SHIFT,
				  InstancePtr->PhyConfig.PwrDown);

}

/******************************************************************************/
/**
 * This function sets the phy width control val
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Phy width val to be used over the main link based on
 *              one of the following selects:
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetPhyWidth(XMmiDp *InstancePtr, XMmiDp_PhyWidth Width)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_RegReadModifyWrite(InstancePtr, XMMIDP_PHYIF_CTRL_0,
				  XMMIDP_PHYIF_CTRL_0_PHY_WIDTH_MASK,
				  XMMIDP_PHYIF_CTRL_0_PHY_WIDTH_SHIFT,
				  Width);

}

/******************************************************************************/
/**
 * This function sets the AUX_250US_CNT_LIMIT
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Aux 250Us count limit
 *
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetAux_250Us_Cnt_Limit(XMmiDp *InstancePtr, u16 Limit)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_AUX_250US_CNT_LIMIT_0, Limit);

}

/******************************************************************************/
/**
 * This function sets the AUX_2000US_CNT_LIMIT
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Aux 2000Us count limit
 *
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetAux_2000Us_Cnt_Limit(XMmiDp *InstancePtr, u16 Limit)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_AUX_2000US_CNT_LIMIT_0, Limit);

}

/******************************************************************************/
/**
 * This function sets the AUX_100000US_CNT_LIMIT
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       Aux 100000Us count limit
 *
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetAux_100000Us_Cnt_Limit(XMmiDp *InstancePtr, u32 Limit)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_AUX_100000US_CNT_LIMIT_0, Limit);
}

/******************************************************************************/
/**
 * This function sets PM_COFIG1 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       RegVal
 *
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetPmConfig1(XMmiDp *InstancePtr, u32 Val)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_PM_CONFIG1, Val);

}

/******************************************************************************/
/**
 * This function sets PM_COFIG2 register
 *
 * @param       InstancePtr is a pointer to the XDpPsu instance.
 * @param       RegVal
 *
 * @return
 *              - none.
 *
 * @note        None.
 *
*******************************************************************************/
void XMmiDp_SetPmConfig2(XMmiDp *InstancePtr, u32 Val)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XMmiDp_WriteReg(InstancePtr->Config.BaseAddr, XMMIDP_PM_CONFIG2, Val);

}
/** @} */
