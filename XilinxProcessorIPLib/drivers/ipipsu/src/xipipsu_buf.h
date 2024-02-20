
/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 * @file xipipsu_buf.h
 * @addtogroup ipipsu_api IPIPSU APIs
 * @{
 * @details
 *
 * The xipipsu_buf.h is the header file for implementation of IPIPSU driver
 * get buffer and calculate CRC functions.
 * Inter Processor Interrupt (IPI) is used for communication between
 * different processors.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver  Who Date     Changes
 * ---- --- -------- --------------------------------------------------
 * 2.6	sd	04/02/20	Restructured the code for more readability and modularity
 * 2.14	ht	06/13/23	Restructured the code for more modularity
 * 	ht	07/28/23	Fix MISRA-C warnings
 *
 * </pre>
 *
 *****************************************************************************/
/** @cond INTERNAL */
#ifndef XIPIPSU_BUF_H_
#define XIPIPSU_BUF_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xstatus.h"

/************************** Function Prototypes *****************************/

u32 *XIpiPsu_GetBufferAddress(XIpiPsu *InstancePtr, u32 SrcCpuMask,
			      u32 DestCpuMask, u32 BufferType);
u32 XIpiPsu_GetBufferIndex(const XIpiPsu *InstancePtr, u32 CpuMask);
#ifdef ENABLE_IPI_CRC
u32 XIpiPsu_CalculateCRC(u32 BufAddr, u32 BufSize);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XIPIPSU_BUF_H_ */
/** @endcond */
/** @} */
