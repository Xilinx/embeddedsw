/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xipipsu_buf.c
* @addtogroup ipipsu_v2_10
* @{
*
* This file contains the implementation of the buffer access functions for XIpiPsu
* driver. Refer to the header file xipipsu_buf.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date	Changes
* ----- ------ -------- ----------------------------------------------
* 2.6	sd	04/02/20	Restructured the code for more readability and modularity
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xipipsu.h"
#include "xipipsu_hw.h"
#include "xipipsu_buf.h"

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
 * @brief	Get the Buffer Index for a CPU specified by Mask
 *
 * @param	InstancePtr is the pointer to current IPI instance
 * @param	CpuMask is the Mask of the CPU form which Index is required
 *
 * @return	Buffer Index value if CPU Mask is valid
 * 			XIPIPSU_MAX_BUFF_INDEX+1 if not valid
 *
 */
static u32 XIpiPsu_GetBufferIndex(const XIpiPsu *InstancePtr, u32 CpuMask)
{
	u32 BufferIndex;
	u32 Index;
	/* Init Index with an invalid value */
	BufferIndex = XIPIPSU_MAX_BUFF_INDEX + 1U;

	/*Search for CPU in the List */
	for (Index = 0U; Index < InstancePtr->Config.TargetCount; Index++) {
		/*If we find the CPU , then set the Index and break the loop*/
		if (InstancePtr->Config.TargetList[Index].Mask == CpuMask) {
			BufferIndex = InstancePtr->Config.TargetList[Index].BufferIndex;
			break;
		}
	}

	/* Return the Index */
	return BufferIndex;
}
/**
 * @brief	Get the Buffer Address for a given pair of CPUs
 *
 * @param	InstancePtr is the pointer to current IPI instance
 * @param	SrcCpuMask is the Mask for Source CPU
 * @param	DestCpuMask is the Mask for Destination CPU
 * @param	BufferType is either XIPIPSU_BUF_TYPE_MSG or XIPIPSU_BUF_TYPE_RESP
 *
 * @return	Valid Buffer Address if no error
 * 			NULL if an error occurred in calculating Address
 *
 */

u32* XIpiPsu_GetBufferAddress(XIpiPsu *InstancePtr, u32 SrcCpuMask,
		u32 DestCpuMask, u32 BufferType)
{
#ifdef __aarch64__
	u64 BufferAddr;
#else
	u32 BufferAddr;
#endif
	u32 SrcIndex;
	u32 DestIndex;
	/* Get the buffer indices */
	SrcIndex = XIpiPsu_GetBufferIndex(InstancePtr, SrcCpuMask);
	DestIndex = XIpiPsu_GetBufferIndex(InstancePtr, DestCpuMask);

	/* If we got an invalid buffer index, then return NULL pointer, else valid address */
	if ((SrcIndex > XIPIPSU_MAX_BUFF_INDEX)
			|| (DestIndex > XIPIPSU_MAX_BUFF_INDEX)) {
		BufferAddr = 0U;
	} else {

		if (XIPIPSU_BUF_TYPE_MSG == BufferType) {
			BufferAddr = XIPIPSU_MSG_RAM_BASE
					+ (SrcIndex * XIPIPSU_BUFFER_OFFSET_GROUP)
					+ (DestIndex * XIPIPSU_BUFFER_OFFSET_TARGET);
		} else if (XIPIPSU_BUF_TYPE_RESP == BufferType) {
			BufferAddr = XIPIPSU_MSG_RAM_BASE
					+ (DestIndex * XIPIPSU_BUFFER_OFFSET_GROUP)
					+ (SrcIndex * XIPIPSU_BUFFER_OFFSET_TARGET)
					+ (XIPIPSU_BUFFER_OFFSET_RESPONSE);
		} else {
			BufferAddr = 0U;
		}

	}
	return (u32 *) BufferAddr;
}
/** @} */
