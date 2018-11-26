/*
 * Copyright (c) 2016 - 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	freertos/irq.c
 * @brief	FreeRTOS libmetal irq definitions.
 */

#include <metal/irq.h>
#include <metal/sys.h>

unsigned int metal_irq_save_disable(void)
{
	return sys_irq_save_disable();
}

void metal_irq_restore_enable(unsigned int flags)
{
	sys_irq_restore_enable(flags);
}
