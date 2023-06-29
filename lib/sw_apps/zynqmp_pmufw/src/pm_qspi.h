/*****************************************************************************
* Copyright (c) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef PM_QSPI_H_
#define PM_QSPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_common.h"
#include "xpfw_default.h"

#if defined(XPAR_XQSPIPSU_0_DEVICE_ID) || defined(XPAR_XQSPIPSU_0_BASEADDR)
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define QSPIPSU_DEVICE_ID		XPAR_XQSPIPSU_0_DEVICE_ID
#else
#define QSPIPSU_DEVICE_ID		XPAR_XQSPIPSU_0_BASEADDR
#endif
#define QSPIDMA_DST_CTRL		(XPAR_XQSPIPSU_0_BASEADDR + 0x80CU)
#endif

s32 PmQspiInit(void);
s32 PmQspiWrite(u8 *WriteBufrPtr, u32 ByteCount);
s32 PmQspiRead(u32 ByteCount, u8 *ReadBfrPtr);
s32 PmQspiHWInit(void);

#ifdef __cplusplus
}
#endif

#endif /* PM_QSPI_H_ */
