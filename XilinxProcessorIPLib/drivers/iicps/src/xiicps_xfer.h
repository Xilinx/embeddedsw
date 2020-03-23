/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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
* @file xiicps_xfer.h
* @addtogroup iicps_v3_11
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
