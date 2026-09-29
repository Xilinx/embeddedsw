/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XCORESIGHTPSDCC_H                /* prevent circular inclusions */
#define XCORESIGHTPSDCC_H

/*****************************************************************************/
/**
*
* @file xcoresightpsdcc.h
* @addtogroup coresightps_dcc Overview
* @{
* @brief Provides CoreSight Debug Communication Channel (DCC) byte I/O.
*
* CoreSight driver component.
*
* Coresight is part of the Debug Communication Channel (DCC) group.
* JTAG UART for ARM uses DCC. Each ARM core has its own DCC.
* Select an ARM target in the XSDB console before running
* the jtag terminal command.
* The CoreSight driver can direct the output stream to a log file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date		Changes
* ----- -----  -------- -----------------------------------------------
* 1.00  kvn    02/14/15 First release
* 1.1   kvn    06/12/15 Add support for Zynq Ultrascale+ MP.
*       kvn    08/18/15 Modified Makefile according to compiler changes.
* 1.3   asa    07/01/16 Made changes to ensure that the file does not compile
*                       for MB BSPs. Instead it throws up a warning. This
*                       fixes the CR#953056.
* 1.5   sne    01/19/19 Fixed MISRA-C Violations CR#1025101.
* 1.10  mus    10/06/23 Fix compilation error for Microblaze RISC-V processor.
* 1.14  tb     09/10/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#ifdef __cplusplus
extern "C" {
#endif

#if ! defined(__MICROBLAZE__) && ! defined(__riscv)
#include <xil_types.h>

void XCoresightPs_DccSendByte(u32 BaseAddress, u8 Data);

u8 XCoresightPs_DccRecvByte(u32 BaseAddress);
#endif

#ifdef __cplusplus
}
#endif

#endif
/** @} */
