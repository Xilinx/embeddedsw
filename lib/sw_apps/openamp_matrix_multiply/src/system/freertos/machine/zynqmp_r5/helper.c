/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (c) 2021 Xilinx, Inc. All rights reserved.
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdarg.h>
#include "xparameters.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include "FreeRTOS.h"
#include "xil_cache.h"
#include <metal/sys.h>
#include <metal/irq.h>
#include "platform_info.h"

/*
 * app_gic_initialize - Interrupt Controller setup
 *
 * Interrupt IPI_IRQ_VECT_ID can also be enabled via call to
 * vPortEnableInterrupt() which will call XScuGic_Enable().
 *
 * vPortEnableInterrupt() is not needed for this demo as the
 * interrupt is enabled via call to metal_irq_enable() in
 * zynqmp_r5_a53_rproc.c::zynqmp_r5_a53_proc_init().
 *
 * return 0.
 */
static int app_gic_initialize(void)
{
	/*
	 * Register the ISR with the interrupt controller instance
	 * initialized by porting layer.
	 */
	xPortInstallInterruptHandler(IPI_IRQ_VECT_ID,
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

	/* prefix "ts cnt L6 ": timestamp in ticks, count and log level */
	len = sprintf(msg, "%u %u L%u ", xTaskGetTickCount(),
			circ.c_cnt, level);
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

	ML_DBG("buf,len,configTICK_RATE_HZ = %p,%u,%u \n",
		circ.c_buf, circ.c_len,
		configTICK_RATE_HZ);
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
