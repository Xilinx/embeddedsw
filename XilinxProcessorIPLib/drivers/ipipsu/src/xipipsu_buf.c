/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xipipsu_buf.c
* @addtogroup ipipsu_api IPIPSU APIs
* @{
*
* The xipipsu_buf.c file contains the implementation of the buffer access
* functions for XIpiPsu driver.
* Refer to the header file xipipsu_buf.h for more detailed information.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date	Changes
* ----- ------ -------- ----------------------------------------------
* 2.6	sd	04/02/20	Restructured the code for more readability and modularity
* 2.14	ht	06/13/23	Restructured the code for more modularity
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xipipsu.h"
#include "xipipsu_hw.h"
#include "xipipsu_buf.h"

/************************** Variable Definitions *****************************/


/**
 * @brief	Gets the Buffer Address for a given pair of CPUs
 *
 * @param	InstancePtr Pointer to current IPI instance
 * @param	SrcCpuMask Mask for Source CPU
 * @param	DestCpuMask Mask for Destination CPU
 * @param	BufferType Type of buffer either XIPIPSU_BUF_TYPE_MSG or XIPIPSU_BUF_TYPE_RESP
 *
 * @return	Valid Buffer Address if no error
 * 			NULL if an error occurs in calculating address
 *
 */

u32 *XIpiPsu_GetBufferAddress(XIpiPsu *InstancePtr, u32 SrcCpuMask,
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
