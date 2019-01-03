/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (c) 2021 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "xparameters.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "FreeRTOS.h"
#include "xil_cache.h"
#include <metal/sys.h>
#include <metal/irq.h>
#include "platform_info.h"

/* Interrupt Controller setup */
static int app_gic_initialize(void)
{
	/*
	 * Register the ISR with the interrupt controller instance
	 * initialized by porting layer.
	 */
	xPortInstallInterruptHandler(SGI_NOTIFICATION,
				     (Xil_ExceptionHandler)metal_xlnx_irq_isr,
				     (void *)SGI_NOTIFICATION);

	/*
	 * Enable interrupt for SGI_NOTIFICATION.
	 * This calls XScuGic_InterruptMaptoCpu() via XScuGic_Enable()
	 */
	vPortEnableInterrupt(SGI_NOTIFICATION);

	return 0;
}

/* Main hw machinery initialization entry point, called from main()*/
/* return 0 on success */
int init_system(void)
{
	int ret;
	struct metal_init_params metal_param = METAL_INIT_DEFAULTS;

	/* Low level abstraction layer for openamp initialization */
	metal_init(&metal_param);

	/* configure the global interrupt controller */
	app_gic_initialize();

	/* Initialize metal Xilinx IRQ controller */
	ret = metal_xlnx_irq_init();
	if (ret) {
		xil_printf("%s: Xilinx metal IRQ controller init failed.\r\n",
			__func__);
	}

	return ret;
}

void cleanup_system()
{
	metal_finish();

	Xil_DCacheDisable();
	Xil_ICacheDisable();
	Xil_DCacheInvalidate();
	Xil_ICacheInvalidate();
}
