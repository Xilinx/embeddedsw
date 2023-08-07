/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
* @file xsem_ipi.h
*  This file contains IPI configurations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date         Changes
* ====  ==== ==========   ====================================================
* 0.1   gm   03/19/2021   Initial creation
* 0.2   ga   05/19/2023   Fixed IPI instance for versal net
* 0.3  rama  08/03/2023   Added support for system device-tree flow
* 0.4  gam   08/07/2023   Corrected XSEM_SSIT_MAX_SLR_CNT macro definition
*
* </pre>
*
* @note
*
*****************************************************************************/
#ifndef XSEM_IPI_H
#define XSEM_IPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <xstatus.h>
#include <xipipsu_hw.h>
#include <xipipsu.h>
#include "xsem_gic_setup.h"

#ifdef VERSAL_NET
#define SRC_IPI_MASK	(XPAR_XIPIPS_TARGET_PSX_PMC_0_CH0_MASK)
#else
#define SRC_IPI_MASK	(XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK)
#endif

#ifdef SDT
	#define XSEM_SSIT_MAX_SLR_CNT	NUMBER_OF_SLRS
#endif

typedef void (*IpiCallback)(XIpiPsu *const InstancePtr);
XStatus IpiRegisterCallback(XIpiPsu *const IpiInst, const u32 SrcMask,
		IpiCallback Callback);
XStatus IpiInit(XIpiPsu *const InstancePtr, XScuGic *const GicInst);

/* IPI Callback for SEM events */
void XSem_IpiCallback(XIpiPsu *const InstancePtr);

#endif /* XSEM_IPI_H */
