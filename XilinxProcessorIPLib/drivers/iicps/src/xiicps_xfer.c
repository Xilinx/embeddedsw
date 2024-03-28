/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiicps_xfer.c
* @addtogroup iicps_api IICPS APIs
* @{
*
* The xiicps_xfer.c file contains implementation of required helper functions
* for the XIicPs driver.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------
* 3.11  rna     12/10/19 First release
*		02/18/20 Modified latest code for MISRA-C:2012 Compliance.
* 3.18  gm      08/25/23 Added XIicPs_MasterPolledRead, XIicPs_MasterIntrSend
*			 and XIicPs_MasterIntrRead functions.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xiicps.h"
#include "xiicps_hw.h"
#include "xiicps_xfer.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
* This function prepares a device to transfers as a master.
*
* @param        InstancePtr Pointer to the XIicPs instance.
*
* @param        Role Specifies whether the device is sending or receiving.
*
* @return
*               - XST_SUCCESS if everything went well.
*               - XST_FAILURE if bus is busy.
*
* @note         Interrupts are always disabled. The device which needs to use
*               interrupts must setup interrupts after this call.
*
****************************************************************************/
s32 XIicPs_SetupMaster(XIicPs *InstancePtr, s32 Role)
{
	u32 ControlReg;
	UINTPTR BaseAddr;

	BaseAddr = InstancePtr->Config.BaseAddress;
	ControlReg = XIicPs_ReadReg(BaseAddr, XIICPS_CR_OFFSET);


	/*
	 * Only check if bus is busy when repeated start option is not set.
	 */
	if ((ControlReg & XIICPS_CR_HOLD_MASK) == 0U) {
		if (XIicPs_BusIsBusy(InstancePtr) == (s32)1) {
			return (s32)XST_FAILURE;
		}
	}

	/*
	 * Set up master, AckEn, nea and also clear fifo.
	 */
	ControlReg |= (u32)XIICPS_CR_ACKEN_MASK | (u32)XIICPS_CR_CLR_FIFO_MASK |
		(u32)XIICPS_CR_NEA_MASK | (u32)XIICPS_CR_MS_MASK;

	if (Role == RECVING_ROLE) {
		ControlReg |= (u32)XIICPS_CR_RD_WR_MASK;
	}else {
		ControlReg &= ~((u32)XIICPS_CR_RD_WR_MASK);
	}

	XIicPs_WriteReg(BaseAddr, XIICPS_CR_OFFSET, ControlReg);

	XIicPs_DisableAllInterrupts(BaseAddr);

	return (s32)XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function handles continuation of sending data. It is invoked
* from interrupt handler.
*
* @param        InstancePtr Pointer to the XIicPs instance.
*
* @return       None.
*
*
****************************************************************************/
void MasterSendData(XIicPs *InstancePtr)
{
	(void)TransmitFifoFill(InstancePtr);

	/*
	 * Clear hold bit if done, so stop can be sent out.
	 */
	if (InstancePtr->SendByteCount == 0) {

		/*
		 * If user has enabled repeated start as an option,
		 * do not disable it.
		 */
		if (InstancePtr->IsRepeatedStart == 0) {

			XIicPs_WriteReg(InstancePtr->Config.BaseAddress,
					(u32)XIICPS_CR_OFFSET,
					XIicPs_ReadReg(InstancePtr->Config.BaseAddress,
						(u32)XIICPS_CR_OFFSET) & ~((u32)XIICPS_CR_HOLD_MASK));
		}
	}

	return;
}

/*****************************************************************************/
/**
* @brief
* Handles polled mode receive in master mode.
*
* @param        InstancePtr Pointer to the XIicPs instance.
* @param        IsHold Hold status.
* @param        ByteCountVar ByteCount.
*
* @return       None.
*
*
 ****************************************************************************/

void XIicPs_MasterPolledRead(XIicPs *InstancePtr, s32 IsHold, s32 ByteCountVar)
{
	u32 Platform;
	u16 SlaveAddr;
	UINTPTR BaseAddr;
	u32 IntrStatusReg;

	Platform = XGetPlatform_Info();
	BaseAddr = InstancePtr->Config.BaseAddress;
	SlaveAddr = (u16)XIicPs_ReadReg(BaseAddr, (u32)XIICPS_ADDR_OFFSET);
	IntrStatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_ISR_OFFSET);

	while ((InstancePtr->RecvByteCount > 0) &&
			(((IntrStatusReg) & ((u32)XIICPS_IXR_ARB_LOST_MASK |
					     (u32)XIICPS_IXR_RX_OVR_MASK |
					     (u32)XIICPS_IXR_RX_UNF_MASK |
					     (u32)XIICPS_IXR_NACK_MASK)) == 0U)) {

		XIicPs_MasterRead(InstancePtr, IsHold, &ByteCountVar);

		if (Platform == (u32)XPLAT_ZYNQ) {
			if ((InstancePtr->UpdateTxSize != 0) &&
				(ByteCountVar == (XIICPS_FIFO_DEPTH + 1))) {
			    /*  wait while fifo is full */
			while (XIicPs_RxFIFOFull(InstancePtr,
						 ByteCountVar) != 0U) { ;
				}
				if ((InstancePtr->RecvByteCount - XIICPS_FIFO_DEPTH) >
					(s32)XIICPS_MAX_TRANSFER_SIZE) {

					XIicPs_WriteReg(BaseAddr,
						XIICPS_TRANS_SIZE_OFFSET,
						XIICPS_MAX_TRANSFER_SIZE);
				    ByteCountVar = (s32)XIICPS_MAX_TRANSFER_SIZE +
							XIICPS_FIFO_DEPTH;
				} else {
					XIicPs_WriteReg(BaseAddr,
						XIICPS_TRANS_SIZE_OFFSET,
						InstancePtr->RecvByteCount -
						XIICPS_FIFO_DEPTH);
					InstancePtr->UpdateTxSize = 0;
					ByteCountVar = InstancePtr->RecvByteCount;
				}
			}
		} else {
		    if ((InstancePtr->RecvByteCount > 0) && (ByteCountVar == 0)) {
				/*
				 * Clear the interrupt status register before use it to
				 * monitor.
				 */
				IntrStatusReg = XIicPs_ReadReg(BaseAddr,
								   XIICPS_ISR_OFFSET);
				XIicPs_WriteReg(BaseAddr, XIICPS_ISR_OFFSET,
						IntrStatusReg);

				XIicPs_WriteReg(BaseAddr, XIICPS_ADDR_OFFSET,
						SlaveAddr);

				if ((InstancePtr->RecvByteCount) >
					(s32)XIICPS_MAX_TRANSFER_SIZE) {

					XIicPs_WriteReg(BaseAddr,
							XIICPS_TRANS_SIZE_OFFSET,
							XIICPS_MAX_TRANSFER_SIZE);
				    ByteCountVar = (s32)XIICPS_MAX_TRANSFER_SIZE;
				} else {
					XIicPs_WriteReg(BaseAddr,
							XIICPS_TRANS_SIZE_OFFSET,
							InstancePtr->RecvByteCount);
					InstancePtr->UpdateTxSize = 0;
					ByteCountVar = InstancePtr->RecvByteCount;
				}
			}
		}

		IntrStatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_ISR_OFFSET);
	}
}

/*****************************************************************************/
/**
* @brief
* Handles interrupt-driven send in master mode.
*
* @param        InstancePtr Pointer to the XIicPs instance.
* @param        IntrStatusReg Value of interrupt status register.
* @param        StatusEventPtr Pointer to the StatusEvent.
*
* @return       None.
*
 ****************************************************************************/

void XIicPs_MasterIntrSend(XIicPs *InstancePtr, u32 IntrStatusReg,
			    u32 *StatusEventPtr)
{
	if (((InstancePtr->IsSend) != 0) &&
		((u32)0U != (IntrStatusReg & (u32)XIICPS_IXR_COMP_MASK))) {
		if (InstancePtr->SendByteCount > 0) {
			MasterSendData(InstancePtr);
		} else {
			*StatusEventPtr |= XIICPS_EVENT_COMPLETE_SEND;
		}
	}
}

/*****************************************************************************/
/**
* @brief
* Handles interrupt-driven receive in master mode.
*
* @param        InstancePtr Pointer to the XIicPs instance.
* @param        IntrStatusRegPtr Pointer to the interrupt status register.
* @param        IsHold Hold status.
*
* @return       None.
*
*
 ****************************************************************************/

void XIicPs_MasterIntrRead(XIicPs *InstancePtr, u32 *IntrStatusRegPtr,
					      s32 IsHold)
{
	s32 ByteCnt;
	u32 Platform;
	u16 SlaveAddr;
	UINTPTR BaseAddr;

	Platform = XGetPlatform_Info();
	ByteCnt = InstancePtr->CurrByteCount;
	BaseAddr = InstancePtr->Config.BaseAddress;

	if ((InstancePtr->IsSend == 0) &&
		((0U != (*IntrStatusRegPtr & (u32)XIICPS_IXR_DATA_MASK)) ||
		 (0U != (*IntrStatusRegPtr & (u32)XIICPS_IXR_COMP_MASK)))){

		XIicPs_MasterRead(InstancePtr, IsHold, &ByteCnt);

		if (Platform == (u32)XPLAT_ZYNQ) {
			if ((InstancePtr->UpdateTxSize != 0) &&
				(ByteCnt == (XIICPS_FIFO_DEPTH + 1))) {

				/* wait while fifo is full */
				while (XIicPs_RxFIFOFull(InstancePtr,
							 ByteCnt) != 0U) { ;
				}

				if ((InstancePtr->RecvByteCount - XIICPS_FIFO_DEPTH) >
					(s32)XIICPS_MAX_TRANSFER_SIZE) {

					XIicPs_WriteReg(BaseAddr,
						XIICPS_TRANS_SIZE_OFFSET,
						XIICPS_MAX_TRANSFER_SIZE);
					ByteCnt = (s32)XIICPS_MAX_TRANSFER_SIZE +
							XIICPS_FIFO_DEPTH;
				} else {
					XIicPs_WriteReg(BaseAddr,
						XIICPS_TRANS_SIZE_OFFSET,
						InstancePtr->RecvByteCount -
						XIICPS_FIFO_DEPTH);
					InstancePtr->UpdateTxSize = 0;
					ByteCnt = InstancePtr->RecvByteCount;
				}
			}
		} else {
			if ((InstancePtr->RecvByteCount > 0) && (ByteCnt == 0)) {
				/*
				 * Clear the interrupt status register before use it to
				 * monitor.
				 */
				*IntrStatusRegPtr = XIicPs_ReadReg(BaseAddr,
								   XIICPS_ISR_OFFSET);
				XIicPs_WriteReg(BaseAddr, XIICPS_ISR_OFFSET,
						*IntrStatusRegPtr);

				SlaveAddr = (u16)XIicPs_ReadReg(BaseAddr,
								(u32)XIICPS_ADDR_OFFSET);
				XIicPs_WriteReg(BaseAddr, XIICPS_ADDR_OFFSET, SlaveAddr);

				if ((InstancePtr->RecvByteCount) >
					(s32)XIICPS_MAX_TRANSFER_SIZE) {

					XIicPs_WriteReg(BaseAddr,
						XIICPS_TRANS_SIZE_OFFSET,
						XIICPS_MAX_TRANSFER_SIZE);
					ByteCnt = (s32)XIICPS_MAX_TRANSFER_SIZE;
				} else {
					XIicPs_WriteReg(BaseAddr,
						XIICPS_TRANS_SIZE_OFFSET,
						InstancePtr->RecvByteCount);
					InstancePtr->UpdateTxSize = 0;
					ByteCnt = InstancePtr->RecvByteCount;
				}
				XIicPs_EnableInterrupts(BaseAddr,
					(u32)XIICPS_IXR_NACK_MASK |
					(u32)XIICPS_IXR_DATA_MASK |
					(u32)XIICPS_IXR_RX_OVR_MASK |
					(u32)XIICPS_IXR_COMP_MASK |
					(u32)XIICPS_IXR_ARB_LOST_MASK);
			}
		}
		InstancePtr->CurrByteCount = ByteCnt;
	}
}

/*****************************************************************************/
/**
*
* Handles the continuation of receiving data. It is invoked
* from interrupt handler.
*
* @param        InstancePtr Pointer to the XIicPs instance.
*
* @return       Number of bytes still expected by the instance.
*
* @note         None.
*
****************************************************************************/
s32 SlaveRecvData(XIicPs *InstancePtr)
{
	u32 StatusReg;
	UINTPTR BaseAddr;
	u8 *Data;
	u8 Value;

	Xil_AssertNonvoid(InstancePtr != NULL);

	BaseAddr = InstancePtr->Config.BaseAddress;

	StatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_SR_OFFSET);

	while ((StatusReg & XIICPS_SR_RXDV_MASK)!=0x0U) {
		Value = (u8)(XIicPs_In32((InstancePtr)->Config.BaseAddress
				  + (u32)XIICPS_DATA_OFFSET));
		Data = &Value;
		*(InstancePtr)->RecvBufferPtr = *Data;
		(InstancePtr)->RecvBufferPtr += 1;
		(InstancePtr)->RecvByteCount ++;

		StatusReg = XIicPs_ReadReg(BaseAddr, XIICPS_SR_OFFSET);
	}

	return InstancePtr->RecvByteCount;
}
/** @} */
