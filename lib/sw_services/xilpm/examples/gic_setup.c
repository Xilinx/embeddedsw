/******************************************************************************
* Copyright (c) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*
 * CONTENT
 * Generic functions for gic initialization and interrupt enabling
 */

#include "gic_setup.h"

XScuGic GicInst;
#if defined(versal)
typedef struct {
	void *CallBackRef;
	u8 Enabled;
} GicIrqEntry;

static GicIrqEntry GicIrqTable[XSCUGIC_MAX_NUM_INTR_INPUTS];
#endif /* versal */

/**
 * GicInit() - Initialize gic
 *
 * @return	Status of operation success (XST_* from xstatus.h)
 */
s32 GicInit(void)
{
	s32 Status;
	XScuGic_Config *GicCfgPtr = XScuGic_LookupConfig(INTC_DEVICE_ID);

	if (NULL == GicCfgPtr)
		return XST_FAILURE;

	Status = XScuGic_CfgInitialize(&GicInst, GicCfgPtr, GicCfgPtr->CpuBaseAddress);
	if (XST_SUCCESS != Status)
		return Status;

	/*
	 * Connect the interrupt controller interrupt Handler to the
	 * hardware interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				    (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				    &GicInst);

	return XST_SUCCESS;
}

/**
 * GicSetupInterruptSystem() - configure the system to receive peripheral interrupt
 * @IntId	interrupt id of the timer
 * @PeriphInstPtr	peripheral
 * @Handler	interrupt Handler
 *
 * Does everything that is needed for enabling interrupts (gic setup, Handler connecting,
 * interrupt enabling on processor and gic level)
 *
 * @return:	status of operation success (XST_* from xstatus.h)
 */
s32 GicSetupInterruptSystem(u32 IntId,
		void *PeriphInstPtr, Xil_ExceptionHandler Handler)
{
	s32 Status;

	Status = GicInit();
	if(XST_SUCCESS != Status)
		return Status;

	Status = XScuGic_Connect(&GicInst, IntId, Handler, PeriphInstPtr);
	if(XST_SUCCESS != Status)
		return Status;

	XScuGic_Enable(&GicInst, IntId);

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

#if defined(versal)
int GicResume()
{
	int Status = XST_FAILURE;
	u32 i;
	XScuGic_Config *GicCfgPtr;

	GicInst.IsReady = 0U;

	GicCfgPtr = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == GicCfgPtr) {
		xil_printf("XScuGic_LookupConfig() failed\r\n");
		goto done;
	}

	Status = XScuGic_CfgInitialize(&GicInst, GicCfgPtr, GicCfgPtr->CpuBaseAddress);
	if (XST_SUCCESS != Status) {
		xil_printf("XScuGic_CfgInitialize() failed with error: %d\r\n", Status);
		goto done;
	}

#if defined(GICv3)
	XScuGic_MarkCoreAwake(&GicInst);
#endif /* GICv3 */

	/* Restore handler pointers and enable interrupt if it was enabled */
	for (i = 0U; i < XSCUGIC_MAX_NUM_INTR_INPUTS; i++) {
		GicInst.Config->HandlerTable[i].CallBackRef = GicIrqTable[i].CallBackRef;

		if (GicIrqTable[i].Enabled) {
			XScuGic_Enable(&GicInst, i);
		}
	}

	Xil_ExceptionEnable();

done:
	return Status;

}

void GicSuspend()
{
	u32 i;
	u32 Mask, Reg;

	for (i = 0U; i < XSCUGIC_MAX_NUM_INTR_INPUTS; i++) {
		GicIrqTable[i].CallBackRef = GicInst.Config->HandlerTable[i].CallBackRef;

		Mask = 0x00000001U << (i % 32U);
		Reg = XScuGic_DistReadReg(&GicInst, XSCUGIC_ENABLE_SET_OFFSET +
					  ((i / 32U) * 4U));
		if (Mask & Reg) {
			GicIrqTable[i].Enabled = 1U;
		} else {
			GicIrqTable[i].Enabled = 0U;
		}
	}

#if defined(GICv3)
	XScuGic_MarkCoreAsleep(&GicInst);
#endif /* GICv3 */
}
#endif /* versal */
