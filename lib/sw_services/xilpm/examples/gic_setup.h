/******************************************************************************
* Copyright (c) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef _GIC_SETUP_H_
#define _GIC_SETUP_H_

#include <xscugic.h>

#ifdef SDT
#include <xparameters.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDT
 #if defined(PLATFORM_ZYNQMP)
  #ifdef __aarch64__
   #define INTC_DEVICE_ID	XPAR_GIC_A53_BASEADDR		/* ZynqMP APU */
  #else
   #define INTC_DEVICE_ID	XPAR_GIC_R5_BASEADDR		/* ZynqMP RPU */
  #endif
 #elif defined(versal)
  #define INTC_DEVICE_ID	XPS_SCU_PERIPH_BASE		/* Versal APU And RPU */
 #endif
#else
 #define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID	/* Non SDT Flow */
#endif

extern XScuGic GicInst;

s32 GicSetupInterruptSystem(u32 IntId,
		void *PeriphInstPtr, Xil_ExceptionHandler Handler);

s32 GicInit();
#if defined(versal)
void GicSuspend();
int GicResume();
#endif /* versal */

#ifdef __cplusplus
}
#endif

#endif /* _GIC_SETUP_H_ */
