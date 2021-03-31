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
	xPortInstallInterruptHandler(IPI_IRQ_VECT_ID,
				     (Xil_ExceptionHandler)metal_xlnx_irq_isr,
				     (void *)IPI_IRQ_VECT_ID);
	/*
	 * Enable interrupt for IPI_IRQ_VECT_ID.
	 * This calls XScuGic_InterruptMaptoCpu() via XScuGic_Enable()
	 */
	vPortEnableInterrupt(IPI_IRQ_VECT_ID);

	return 0;
}

static void system_metal_logger(enum metal_log_level level,
			   const char *format, ...)
{
	(void)level;
	(void)format;
}


/* Main hw machinery initialization entry point, called from main()*/
/* return 0 on success */
int init_system(void)
{
	int ret;
	struct metal_init_params metal_param = {
		.log_handler = system_metal_logger,
		.log_level = METAL_LOG_INFO,
	};

	/* Low level abstraction layer for openamp initialization */
	metal_init(&metal_param);

	/* configure the global interrupt controller */
	app_gic_initialize();

	/* Initialize metal Xilinx IRQ controller */
	ret = metal_xlnx_irq_init();
	if (ret) {
		xil_printf("%s: Xilinx metal IRQ controller init failed.\n",
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
