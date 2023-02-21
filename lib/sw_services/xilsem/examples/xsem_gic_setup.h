/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
* @file xsem_gic_setup.h
*  This file contains global variables and Macro definitions for GIC setup
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ====  ==== ========== ======================================================
* 0.1   gm   03/19/2021 Initial version
*
* </pre>
*
* @note
*
*****************************************************************************/
#ifndef XSEM_GIC_SETUP_H
#define XSEM_GIC_SETUP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <xscugic.h>
#include <xscugic_hw.h>

#define INTC_DEVICE_ID	(XPAR_SCUGIC_SINGLE_DEVICE_ID)

s32 GicSetupInterruptSystem(XScuGic *GicInst);

#ifdef __cplusplus
}
#endif

#endif /* XSEM_GIC_SETUP_H */
