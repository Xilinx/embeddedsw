
/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XIPIPSU_BUF_H_
#define XIPIPSU_BUF_H_

/*****************************************************************************/
/**
 * @file xipipsu_buf.h
 * @brief Declares the buffer-address and CRC helper functions of the driver.
 *
 * The Inter-Processor Interrupt (IPI) provides communication between
 * processors.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver  Who Date     Changes
 * ---- --- -------- --------------------------------------------------
 * 2.6	sd	04/02/20	Restructured the code for more readability and modularity
 * 2.14	ht	06/13/23	Restructured the code for more modularity
 * 	ht	07/28/23	Fix MISRA-C warnings
 * 2.20 vn      07/30/26        Fix Doxygen warnings
 * 2.20 vn      09/10/26        Align Doxygen comments with AMD standards
 *
 * </pre>
 *
 *****************************************************************************/

/**
 * @addtogroup ipipsu_api IPIPSU APIs
 * @{
 */

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
/** @} */
