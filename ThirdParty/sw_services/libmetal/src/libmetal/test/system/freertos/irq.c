/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Xilinx nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
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

#include <stdlib.h>
#include <errno.h>

#include "metal-test.h"
#include "metal/irq.h"
#include "metal/log.h"
#include "metal/sys.h"

#define MAX_HDS  10


int irq_handler(int irq, void *priv)
{
	(void)irq;
	(void)priv;

	return 0;
}


static int irq(void)
{
	int i, j, rc = 0, flags_1, flags_2;
	enum metal_log_level mll= metal_get_log_level();


	metal_set_log_level(LOG_CRITICAL);

	/* go over the max to force an error */
	for (i=-1; i <=  MAX_IRQS+3; i++) {
		for (j=0; j <= MAX_HDS+3; j++) {
			rc = metal_irq_register(i, irq_handler, 0, (void *)j);

			/* check boundaries and drv_id ==0 while handler is not null */
			if ((rc &&
					((i >= MAX_IRQS) || (j >= (MAX_HDS+1))  ||
					 (i < 0 ) || (j <= 0))
				) ||
				(!rc &&
					(i < MAX_IRQS) && (j < (MAX_HDS+1)) &&
					(i >= 0) && (j > 0)
				))
				continue;

			metal_set_log_level(mll);
			metal_log(LOG_DEBUG, "register irq %d fail hd %d\n", i, j);
			return rc ? rc : -EINVAL;
		}
	}

	/* delete all handlers for IRQ #1 */
	rc = metal_irq_register(1, 0, 0, (void *)0);
	if (rc) {
		metal_set_log_level(mll);
		metal_log(LOG_DEBUG, "deregister irq 1 all handlers\n");
		return rc;
	}

	/* delete only one handler IRQ */
	rc = metal_irq_register(2, 0, 0, (void *)3);
	if (rc) {
		metal_set_log_level(mll);
		metal_log(LOG_DEBUG, "deregister irq 2 hd %d\n", 3);
		return rc;
	}

	/* global interrupt disable/enable normal behavior */
	flags_1=metal_irq_save_disable();
	metal_irq_restore_enable(flags_1);

	/* global interrupt less common */
	flags_1=metal_irq_save_disable();
	flags_2=metal_irq_save_disable();
	metal_irq_restore_enable(flags_2);
	metal_irq_restore_enable(flags_1);

	metal_set_log_level(mll);
	return rc;
}

METAL_ADD_TEST(irq);
