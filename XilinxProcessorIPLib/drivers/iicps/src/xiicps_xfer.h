/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xiicps_xfer.h
* @addtogroup iicps_v3_12
* @{
*
* Contains implementation of required helper functions for the XIicPs driver.
* See xiicps.h for detailed description of the device and driver.
*
* <pre> MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- --------------------------------------------
* 3.11  rna     12/10/19 First release
* </pre>
*
******************************************************************************/
#ifndef XIICPS_XFER_H             /* prevent circular inclusions */
#define XIICPS_XFER_H             /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xiicps.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

static INLINE u32 XIicPs_RxDataValid(XIicPs *InstancePtr)
{
	return ((XIicPs_ReadReg(InstancePtr->Config.BaseAddress, XIICPS_SR_OFFSET))
				& XIICPS_SR_RXDV_MASK);
}

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
 * This function handles continuation of receiving data.
 */
s32 SlaveRecvData(XIicPs *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
