
/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the <ORGANIZATION> nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xil_cache.h"
#include "metal/sys.h"
#include "metal/irq.h"
#include "metal/system/generic/irq.h"


#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
/** IPI IRQ ID */
#define IPI_IRQ_VECT_ID         65

extern int platform_register_metal_device(void);

static XScuGic InterruptController;


static int zynqmp_r5_gic_initialize(void)
{
	u32 Status;
	XScuGic_Config *IntcConfig;	/* The configuration parameters of the interrupt controller */

	Xil_ExceptionDisable();

	/*
	 * Initialize the interrupt controller driver
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Register the interrupt handler to the hardware interrupt handling
	 * logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler)XScuGic_InterruptHandler,
			&InterruptController);

	Xil_ExceptionEnable();

	/* Connect Interrupt ID with ISR */
	XScuGic_Connect(&InterruptController, IPI_IRQ_VECT_ID,
			   (Xil_ExceptionHandler)metal_irq_isr,
			   (void *)IPI_IRQ_VECT_ID);

	return 0;
}

void init_system(void)
{
	struct metal_init_params metal_param = METAL_INIT_DEFAULTS;

	metal_init(&metal_param);
	zynqmp_r5_gic_initialize();
	platform_register_metal_device();
}

void cleanup_system()
{
	metal_finish();

	Xil_DCacheDisable();
	Xil_ICacheDisable();
	Xil_DCacheInvalidate();
	Xil_ICacheInvalidate();
}
