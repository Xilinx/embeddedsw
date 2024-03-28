/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiicps_xfer.h
* @addtogroup iicps_api IICPS APIs
* @{
*
* The xiicps_xfer.h file contains implementation of required helper functions
* for the XIicPs driver.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------
* 3.11  rna     12/10/19 First release
* 3.18  gm      08/25/23 Added function prototypes for XIicPs_MasterPolledRead,
* 			 XIicPs_MasterIntrSend, XIicPs_MasterIntrRead and
* 			 XIicPs_MasterRead.
* </pre>
*
******************************************************************************/
/** @cond INTERNAL */
#ifndef XIICPS_XFER_H             /* prevent circular inclusions */
#define XIICPS_XFER_H             /**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xiicps.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* Checks if Rx data is valid or not.
*
* @param        InstancePtr Pointer to the XIicPs instance.
*
* @return       The return value is '1' if Rx data is valid, '0' otherwise.
*
* @note         None.
*
******************************************************************************/
static INLINE u32 XIicPs_RxDataValid(XIicPs *InstancePtr)
{
	return ((XIicPs_ReadReg(InstancePtr->Config.BaseAddress, XIICPS_SR_OFFSET))
				& XIICPS_SR_RXDV_MASK);
}

/*****************************************************************************/
/**
*
* Checks if Rx FIFO is full or not.
*
* @param        InstancePtr Pointer to the XIicPs instance.
* @param        ByteCountVar Number of bytes to be received.
*
* @return       The return value is '0' if Rx FIFO is full, '1' otherwise.
*
* @note         None.
*
******************************************************************************/
static INLINE u32 XIicPs_RxFIFOFull(XIicPs *InstancePtr, s32 ByteCountVar)
{
	u32 Status = 0;

	Status = (u32)(XIicPs_ReadReg(InstancePtr->Config.BaseAddress,
				XIICPS_TRANS_SIZE_OFFSET) !=(u32)(ByteCountVar -
					(s32)XIICPS_FIFO_DEPTH));
	return Status;
}

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/
/*
 * This function prepares a device to transfer as a master.
 */
s32 XIicPs_SetupMaster(XIicPs *InstancePtr, s32 Role);
/*
 * This function handles continuation of sending data.
 */
void MasterSendData(XIicPs *InstancePtr);
/*
 * This function handles polled mode receive in master mode.
 */
void XIicPs_MasterPolledRead(XIicPs *InstancePtr, s32 IsHold, s32 ByteCountVar);

/*
 * This function handles interrupt-driven send in master mode.
 */
void XIicPs_MasterIntrSend(XIicPs *InstancePtr, u32 IntrStatusReg,
			    u32 *StatusEventPtr);

/*
 * This function handles interrupt-driven receive in master mode.
 */
void XIicPs_MasterIntrRead(XIicPs *InstancePtr, u32 *IntrStatusRegPtr,
					      s32 IsHold);
/*
 * This function handles interrupt-driven send in master mode.
 */
void XIicPs_MasterRead(XIicPs *InstancePtr, s32 IsHold, s32 *ByteCntPtr);

/*
 * This function handles continuation of receiving data.
 */
s32 SlaveRecvData(XIicPs *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @endcond */
/** @} */
