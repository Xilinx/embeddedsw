/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef _GIC_SETUP_H_
#define _GIC_SETUP_H_

#include <xscugic.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID

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
