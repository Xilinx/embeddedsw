/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (c) 2021 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdarg.h>
#include "xparameters.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "xscugic.h"
#include "xil_cache.h"
#include <metal/sys.h>
#include <metal/irq.h>
#include "platform_info.h"

#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID

static XScuGic xInterruptController;

/* Interrupt Controller setup */
static int app_gic_initialize(void)
{
	uint32_t status;
	XScuGic_Config *int_ctrl_config; /* interrupt controller configuration params */
	uint32_t int_id;
	uint32_t mask_cpu_id = ((u32)0x1 << XPAR_CPU_ID);
	uint32_t target_cpu;

	mask_cpu_id |= mask_cpu_id << 8U;
	mask_cpu_id |= mask_cpu_id << 16U;

	Xil_ExceptionDisable();

	/*
	 * Initialize the interrupt controller driver
	 */
	int_ctrl_config = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == int_ctrl_config) {
		return XST_FAILURE;
	}

	status = XScuGic_CfgInitialize(&xInterruptController, int_ctrl_config,
				       int_ctrl_config->CpuBaseAddress);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Only associate interrupt needed to this CPU */
	for (int_id = 32U; int_id<XSCUGIC_MAX_NUM_INTR_INPUTS;int_id=int_id+4U) {
		target_cpu = XScuGic_DistReadReg(&xInterruptController,
						XSCUGIC_SPI_TARGET_OFFSET_CALC(int_id));
		/* Remove current CPU from interrupt target register */
		target_cpu &= ~mask_cpu_id;
		XScuGic_DistWriteReg(&xInterruptController,
					XSCUGIC_SPI_TARGET_OFFSET_CALC(int_id), target_cpu);
	}
	XScuGic_InterruptMaptoCpu(&xInterruptController, XPAR_CPU_ID, IPI_IRQ_VECT_ID);

	/*
	 * Register the interrupt handler to the hardware interrupt handling
	 * logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler)XScuGic_InterruptHandler,
			&xInterruptController);

	/* Disable the interrupt before enabling exception to avoid interrupts
	 * received before exception is enabled.
	 */
	XScuGic_Disable(&xInterruptController, IPI_IRQ_VECT_ID);

	Xil_ExceptionEnable();

	/* Connect Interrupt ID with ISR */
	XScuGic_Connect(&xInterruptController, IPI_IRQ_VECT_ID,
			(Xil_ExceptionHandler)metal_xlnx_irq_isr,
			(void *)IPI_IRQ_VECT_ID);

	return 0;
}

/*
 * A circular buffer for libmetal log. Need locks if ported to MT world.
 * c_buf - pointer to the buffer referenced in the resource table
 * c_len - size of the buffer
 * c_pos - next rext record position
 * c_cnt - free running count of records to help sorting in case of overrun
 */
extern char *get_rsc_trace_info(unsigned int *);
static struct {
	char * c_buf;
	unsigned int c_len;
	unsigned int c_pos;
	unsigned int c_cnt;
} circ;

static void rsc_trace_putchar(char c)
{
	if (circ.c_pos >= circ.c_len)
		circ.c_pos = 0;
	circ.c_buf[circ.c_pos++] = c;
}

static void rsc_trace_logger(enum metal_log_level level,
			   const char *format, ...)
{
	char msg[128];
	char *p;
	int len;
	va_list args;

	/* prefix "cnt L6 ": record count and log level */
	len = sprintf(msg, "%u L%u ", circ.c_cnt, level);
	if (len < 0 || len >= sizeof(msg))
		len = 0;
	circ.c_cnt++;

	va_start(args, format);
	vsnprintf(msg + len, sizeof(msg) - len, format, args);
	va_end(args);

	/* copy at most sizeof(msg) to the circular buffer */
	for (len = 0, p = msg; *p && len < sizeof(msg); ++len, ++p)
		rsc_trace_putchar(*p);
	/* Remove this xil_printf to stop printing to console */
	xil_printf("%s", msg);
}

/* Main hw machinery initialization entry point, called from main()*/
/* return 0 on success */
int init_system(void)
{
	int ret;
	struct metal_init_params metal_param = METAL_INIT_DEFAULTS;

	circ.c_buf = get_rsc_trace_info(&circ.c_len);
	if (circ.c_buf && circ.c_len){
		metal_param.log_handler = rsc_trace_logger;
		metal_param.log_level = METAL_LOG_DEBUG;
		circ.c_pos = circ.c_cnt = 0;
	};

	/* Low level abstraction layer for openamp initialization */
	metal_init(&metal_param);

	/* configure the global interrupt controller */
	app_gic_initialize();

	/* Initialize metal Xilinx IRQ controller */
	ret = metal_xlnx_irq_init();
	if (ret) {
		ML_ERR("metal_xlnx_irq_init failed.\r\n");
	}

	ML_DBG("c_buf,c_len = %p,%u\r\n", circ.c_buf, circ.c_len);
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
