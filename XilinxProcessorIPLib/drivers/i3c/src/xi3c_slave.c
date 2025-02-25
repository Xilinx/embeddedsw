/******************************************************************************
* Copyright (C) 2025 Advanced Micro Devices, Inc. All Rights Reserved
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xi3c_slave.c
* @addtogroup Overview
* @{
*
* Handles slave mode transfers.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---  -------- ---------------------------------------------
* 1.2   gm  02/18/24 Added slave mode support
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_util.h"
#include "xi3c.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

static s32 XI3c_SlaveEvent(XI3c *InstancePtr, u16 ByteCount)
{
	u8 EventCCC;

	InstancePtr->RecvByteCount = ByteCount;
	XI3c_ReadRxFifo(InstancePtr);

	if (InstancePtr->DirectCCC)
		EventCCC = InstancePtr->DirectCCC;
	else
		EventCCC = InstancePtr->RecvBufferPtr[0];

	/*
	 * Validate the received CCC
	 */
	if ((EventCCC != (u8)XI3C_CCC_BRDCAST_ENEC) && (EventCCC != (u8)XI3C_CCC_BRDCAST_DISEC)
	    && (EventCCC != (u8)XI3C_CCC_ENEC) && (EventCCC != (u8)XI3C_CCC_DISEC)) {
		return XST_FAILURE;
	}

	/*
	 * Update EVT_INT value
	 */
	if (EventCCC == (u8)XI3C_CCC_BRDCAST_ENEC || EventCCC == (u8)XI3C_CCC_ENEC) {
		XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_EVENT,
                              (XI3c_ReadReg(InstancePtr->Config.BaseAddress,
				XI3C_EVENT)) | XI3C_1BIT_MASK);
	} else {
		XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_EVENT,
                              (XI3c_ReadReg(InstancePtr->Config.BaseAddress,
				XI3C_EVENT)) &~ XI3C_1BIT_MASK);
	}

	return XST_SUCCESS;
}

static s32 XI3c_SlaveSetMWL(XI3c *InstancePtr, u16 ByteCount)
{
	u32 MwlVal;
	u16 MwlMsbVal;
	u16 MwlLsbVal;
	u8 MwlCCC;

	InstancePtr->RecvByteCount = ByteCount;
	XI3c_ReadRxFifo(InstancePtr);

	if (InstancePtr->DirectCCC)
		MwlCCC = InstancePtr->DirectCCC;
	else
		MwlCCC = InstancePtr->RecvBufferPtr[0];

	/*
	 * Validate the received CCC
	 */
	if ((MwlCCC != (u8)XI3C_CCC_BRDCAST_SETMWL) && (MwlCCC != (u8)XI3C_CCC_SETMWL)) {
		return XST_FAILURE;
	}

	if (InstancePtr->DirectCCC) {
		MwlLsbVal = InstancePtr->RecvBufferPtr[0];
		MwlMsbVal = InstancePtr->RecvBufferPtr[1];
	} else {
		MwlLsbVal = InstancePtr->RecvBufferPtr[1];
		MwlMsbVal = InstancePtr->RecvBufferPtr[2];
	}

	MwlVal = ((MwlMsbVal << XI3C_MWL_MRL_MSB_SHIFT) & XI3C_MSB_8BITS_MASK) | MwlLsbVal;

	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_MWL_MRL,
		      (((XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_MWL_MRL))
			& (~XI3C_12BITS_MASK)) | (MwlVal & XI3C_12BITS_MASK)));

	return XST_SUCCESS;
}

static s32 XI3c_SlaveSetMRL(XI3c *InstancePtr, u16 ByteCount)
{
	u32 MrlVal;
	u16 MrlMsbVal;
	u16 MrlLsbVal;
	u8 MrlCCC;

	InstancePtr->RecvByteCount = ByteCount;
	XI3c_ReadRxFifo(InstancePtr);

	if (InstancePtr->DirectCCC)
		MrlCCC = InstancePtr->DirectCCC;
	else
		MrlCCC = InstancePtr->RecvBufferPtr[0];

	/*
	 * Validate the received CCC
	 */
	if ((MrlCCC != (u8)XI3C_CCC_BRDCAST_SETMRL) && (MrlCCC != (u8)XI3C_CCC_SETMRL)) {
		return XST_FAILURE;
	}

	if (InstancePtr->DirectCCC) {
		MrlLsbVal = InstancePtr->RecvBufferPtr[0];
		MrlMsbVal = InstancePtr->RecvBufferPtr[1];
	} else {
		MrlLsbVal = InstancePtr->RecvBufferPtr[1];
		MrlMsbVal = InstancePtr->RecvBufferPtr[2];
	}

	MrlVal = ((MrlMsbVal << XI3C_MWL_MRL_MSB_SHIFT) & XI3C_MSB_8BITS_MASK) | MrlLsbVal;

	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_MWL_MRL,
		      (((XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_MWL_MRL))
			&~ XI3C_MRL_MASK) | ((MrlVal & XI3C_12BITS_MASK) << XI3C_MRL_SHIFT)));

	return XST_SUCCESS;
}

static s32 XI3c_RstGrpAddr(XI3c *InstancePtr, u16 ByteCount)
{
	u8 RstGrpCCC;

	InstancePtr->RecvByteCount = ByteCount;
	XI3c_ReadRxFifo(InstancePtr);

	if (InstancePtr->DirectCCC)
		RstGrpCCC = InstancePtr->DirectCCC;
	else
		RstGrpCCC = InstancePtr->RecvBufferPtr[0];

	/*
	 * Validate the received CCC
	 */
	if ((RstGrpCCC != (u8)XI3C_CCC_BRDCAST_RSTGRPA) && (RstGrpCCC != (u8)XI3C_CCC_RSTGRPA)) {
		return XST_FAILURE;
	}

	XI3c_ClearGrpAddr(InstancePtr);

	return XST_SUCCESS;
}

static s32 XI3c_SetGrpAddr(XI3c *InstancePtr, u16 ByteCount)
{
	u16 GrpAddr;

	InstancePtr->RecvByteCount = ByteCount;
	XI3c_ReadRxFifo(InstancePtr);

	GrpAddr = InstancePtr->RecvBufferPtr[0];

	XI3c_WriteReg(InstancePtr->Config.BaseAddress, XI3C_ADDRESS_OFFSET,
					(((XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_ADDRESS_OFFSET))
					&~ XI3C_GRP_ADDR_MASK) | ((GrpAddr & XI3C_8BITS_MASK) << XI3C_GRP_ADDR_SHIFT)));

	return XST_SUCCESS;
}

static s32 XI3c_RstAction(XI3c *InstancePtr, u16 ByteCount)
{
	u8 RstActCCC;

	InstancePtr->RecvByteCount = ByteCount;
	XI3c_ReadRxFifo(InstancePtr);

	if (InstancePtr->DirectCCC)
		RstActCCC = InstancePtr->DirectCCC;
	else
		RstActCCC = InstancePtr->RecvBufferPtr[0];
	/*
	 * Validate the received CCC
	 */
	if ((RstActCCC != (u8)XI3C_CCC_BRDCAST_RSTACT) && (RstActCCC != (u8)XI3C_CCC_RSTACT)) {
		return XST_FAILURE;
	}

	XI3c_Reset(InstancePtr);
	XI3c_ClearGrpAddr(InstancePtr);

	return XST_SUCCESS;
}

static s32 XI3c_SlaveHandleCCC(XI3c *InstancePtr, u32 ResponseData)
{
	s32 Status = XST_FAILURE;
	u16 ByteCount;
	u8 CCCVal;

	ByteCount = (ResponseData & XI3C_RESP_BYTES_MASK) >> XI3C_RESP_BYTES_SHIFT;
	CCCVal = (ResponseData & XI3C_SLV_RESP_CCC_MASK) >> XI3C_SLV_RESP_CCC_SHIFT;

	if (CCCVal > XI3C_CCC_BRDCAST_MAX_VAL) {
		InstancePtr->DirectCCC = CCCVal;
		InstancePtr->RecvByteCount = ByteCount;
		XI3c_ReadRxFifo(InstancePtr);
		return XST_SUCCESS;
	}

	if (InstancePtr->DirectCCC) {
		CCCVal = InstancePtr->DirectCCC;
	}

	switch (CCCVal) {
		case XI3C_CCC_BRDCAST_ENEC:
		case XI3C_CCC_BRDCAST_DISEC:
		case XI3C_CCC_ENEC:
		case XI3C_CCC_DISEC:
			Status = XI3c_SlaveEvent(InstancePtr, ByteCount);
			break;
		case XI3C_CCC_BRDCAST_SETMWL:
		case XI3C_CCC_SETMWL:
			Status = XI3c_SlaveSetMWL(InstancePtr, ByteCount);
			break;
		case XI3C_CCC_BRDCAST_SETMRL:
		case XI3C_CCC_SETMRL:
			Status = XI3c_SlaveSetMRL(InstancePtr, ByteCount);
			break;
		case XI3C_CCC_BRDCAST_RSTGRPA:
		case XI3C_CCC_RSTGRPA:
			Status = XI3c_RstGrpAddr(InstancePtr, ByteCount);
			break;
		case XI3C_CCC_SETGRPA:
			Status = XI3c_SetGrpAddr(InstancePtr, ByteCount);
			break;
		case XI3C_CCC_BRDCAST_RSTACT:
		case XI3C_CCC_RSTACT:
			Status = XI3c_RstAction(InstancePtr, ByteCount);
			break;
		/*
		 * No action required for below CCC's
		 */
		case XI3C_CCC_GETMWL:
		case XI3C_CCC_GETMRL:
		case XI3C_CCC_GETPID:
		case XI3C_CCC_GETBCR:
		case XI3C_CCC_GETDCR:
		case XI3C_CCC_GETSTATUS:
		case XI3C_CCC_GETMXDS:
		case XI3C_CCC_GETCAPS:
			Status = XST_SUCCESS;
			break;
		default:
			Status = XST_FAILURE;
			break;
	}

	if (InstancePtr->DirectCCC)
		InstancePtr->DirectCCC = 0;

	return Status;
}

static s32 XI3c_SlaveHandleRecv(XI3c *InstancePtr, u16 ByteCount)
{
	InstancePtr->RecvByteCount = ByteCount;

	if (InstancePtr->RecvBufferPtr) {
		while (InstancePtr->RecvByteCount > 0)
			XI3c_ReadRxFifo(InstancePtr);
	}

	return XST_SUCCESS;
}

static s32 XI3c_SlaveTransferHandle(XI3c *InstancePtr)
{
	u32 ResponseData;
	u16 ByteCount;

	ResponseData = XI3c_ReadReg(InstancePtr->Config.BaseAddress,
				    XI3C_RESP_STATUS_FIFO_OFFSET);

	InstancePtr->Error = ((ResponseData & XI3C_RESP_CODE_MASK) >> XI3C_RESP_CODE_SHIFT);
	if(InstancePtr->Error)
		return InstancePtr->Error;

	ByteCount = (ResponseData & XI3C_RESP_BYTES_MASK) >> XI3C_RESP_BYTES_SHIFT;

	if (((ResponseData & XI3C_SLV_RESP_7E_FRAME_MASK) && ByteCount) || InstancePtr->DirectCCC) {
		/*
		 * CCC
		 */
		return XI3c_SlaveHandleCCC(InstancePtr, ResponseData);
	} else if (!(ResponseData & XI3C_RESP_RW_MASK)) {
		/*
		 * Recv
		 */
		return XI3c_SlaveHandleRecv(InstancePtr, ByteCount);
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* This function initiates a interrupt mode send in slave mode.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	MsgPtr is the pointer to the send buffer.
* @param	ByteCount is the number of bytes to be sent.
*
* @return	- XST_SUCCESS if everything went well.
*		- XST_NO_DATA if MsgPtr is NULL.
* 		- Error code if any error.
*
* @note		None.
*
****************************************************************************/
s32 XI3c_SlaveSend(XI3c *InstancePtr, u8 *MsgPtr, u16 ByteCount)
{
	u16 WrFifoSpace;
	u16 SpaceIndex;

	/*
         * Assert validates the input arguments.
         */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (MsgPtr == NULL)
		return XST_NO_DATA;

	if (ByteCount > (InstancePtr->Config.RwFifoDepth / 2) || ByteCount > XI3c_GetMRL(InstancePtr))
		return XST_SEND_ERROR;

	InstancePtr->SendBufferPtr = MsgPtr;
	InstancePtr->SendByteCount = ByteCount;

	WrFifoSpace = XI3c_WrFifoLevel(InstancePtr);

	for (SpaceIndex = 0; (SpaceIndex < WrFifoSpace) && (InstancePtr->SendByteCount > 0); SpaceIndex++) {
		/* Fill data */
		XI3c_WriteTxFifo(InstancePtr);
	}

	XI3c_FillSlaveSendCount(InstancePtr, ByteCount);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* This function initiates a interrupt mode receive in slave mode.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	MsgPtr is the pointer to the recv buffer.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_NO_DATA if MsgPtr is NULL.
*
* @note		None.
*
****************************************************************************/
s32 XI3c_SlaveRecv(XI3c *InstancePtr, u8 *MsgPtr)
{
	/*
         * Assert validates the input arguments.
         */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (MsgPtr == NULL)
		return XST_NO_DATA;

	InstancePtr->RecvBufferPtr = MsgPtr;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* This function initiates a polled mode send in slave mode.
*
* It sends data to the FIFO and the master to pick them up.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	MsgPtr is the pointer to the send buffer.
* @param	ByteCount is the number of bytes to be sent.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_SEND_ERROR if any error.
*		- XST_NO_DATA if MsgPtr is NULL.
*
* @note
* 		Caller need to wait for response after calling this function.
*
****************************************************************************/
s32 XI3c_SlaveSendPolled(XI3c *InstancePtr, u8 *MsgPtr, u16 ByteCount)
{
	u16 WrFifoSpace;
	u16 SpaceIndex;

	/*
         * Assert validates the input arguments.
         */
	Xil_AssertNonvoid(InstancePtr != NULL);
	if (MsgPtr == NULL)
		return XST_NO_DATA;

	if (ByteCount > (InstancePtr->Config.RwFifoDepth / 2) || ByteCount > XI3c_GetMRL(InstancePtr))
		return XST_SEND_ERROR;

	InstancePtr->SendBufferPtr = MsgPtr;
	InstancePtr->SendByteCount = ByteCount;

	WrFifoSpace = XI3c_WrFifoLevel(InstancePtr);

	for (SpaceIndex = 0; (SpaceIndex < WrFifoSpace) && (InstancePtr->SendByteCount > 0); SpaceIndex++) {
		/* Fill data */
		XI3c_WriteTxFifo(InstancePtr);
	}

	XI3c_FillSlaveSendCount(InstancePtr, ByteCount);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* @brief
* This function initiates a polled mode receive in slave mode.
*
* @param	InstancePtr is a pointer to the XI3c instance.
* @param	MsgPtr is the pointer to the send buffer.
*
* @return
*		- XST_SUCCESS if everything went well.
*		- XST_RECV_ERROR if any error.
*		- XST_NO_DATA if MsgPtr is NULL.
*
* @note		None.
*
****************************************************************************/
s32 XI3c_SlaveRecvPolled(XI3c *InstancePtr, u8 *MsgPtr)
{
	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (MsgPtr == NULL)
		return XST_NO_DATA;

	InstancePtr->RecvBufferPtr = MsgPtr;

	if (XI3c_SlaveTransferHandle(InstancePtr))
		return XST_RECV_ERROR;
	else
		return XST_SUCCESS;
}

/*****************************************************************************/
/**
* The interrupt handler for the slave mode. It does the slave side protocol
* handling for the interrupt-driven transfers.
*
* If the slave is receiving data then the data is read from the FIFO and
* the slave has to send data on master request data need to write to FIFO.
* If all the data has been transferred then a completion event
* is signalled to the upper layer by calling the callback handler.
*
* @param	InstancePtr is a pointer to the XI3c instance.
*
* @return	None.
*
* @note 	None.
*
****************************************************************************/
void XI3c_SlaveInterruptHandler(XI3c *InstancePtr)
{
	u32 IntrStatusReg;

	/*
	 * Assert validates the input arguments.
	 */
	Xil_AssertVoid(InstancePtr != NULL);

	/*
	 * Read the Raising edge and faling edge Interrupt status registers.
	 */
	IntrStatusReg = XI3c_ReadReg(InstancePtr->Config.BaseAddress, XI3C_INTR_STATUS_OFFSET);

	/*
	 * Write the status back to clear the interrupts so no events are
	 * missed while processing this interrupt.
	 */
	XI3c_WriteReg(InstancePtr->Config.BaseAddress, (u32)XI3C_INTR_STATUS_OFFSET, IntrStatusReg);

	/* Response */
	if (IntrStatusReg & XI3C_INTR_RESP_NOT_EMPTY_MASK) {
		XI3c_SlaveTransferHandle(InstancePtr);

		if (!InstancePtr->DirectCCC)
			InstancePtr->StatusHandler(InstancePtr->Error);
	}
}

/** @} */
