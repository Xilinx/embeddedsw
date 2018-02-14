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

/* We need to find the internal MAX_IRQS limit */
/* Could be retrieved from platform specific files in the future */
#define METAL_INTERNAL

#include <metal/irq.h>
#include <metal/log.h>
#include <metal/sys.h>
#include <metal/list.h>
#include <metal/utilities.h>
#include "metal-test-internal.h"

int irq_handler(int irq, void *priv)
{
	(void)irq;
	(void)priv;

	return 0;
}

int irq_handler_1(int irq, void *priv)
{
	(void)irq;
	(void)priv;

	return 0;
}

static int irq(void)
{
	int rc = 0, flags_1, flags_2;
	char *err_msg="";
	enum metal_log_level mll= metal_get_log_level();

	/* Do not show LOG_ERROR or LOG_DEBUG for expected fail case */
	metal_set_log_level(METAL_LOG_CRITICAL);

	rc = metal_irq_register(1, irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 1 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(2, irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 2 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(2, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 2 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_register(3, irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 3 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(4, irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "register irq 4 fail drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(4, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 4 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_register(1, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 1 fail drv_id 2\n";
		goto out;
	}

	rc = metal_irq_unregister(1, 0, 0, (void *)0);
	if (rc) {
		err_msg = "unregister irq 1 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(1, 0, 0, (void *)0);
	if (!rc) {
		err_msg = "unregister irq 1 fail expected\n";
		goto out;
	}

	rc = metal_irq_unregister(2, 0, 0, (void *)2);
	if (rc) {
		err_msg = "unregister irq 2 drv_id 2 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(2, 0, 0, (void *)2);
	if (!rc) {
		err_msg = "unregister irq 2 drv_id 2 fail expected\n";
		goto out;
	}
	rc = metal_irq_register(2, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 2 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_unregister(2, 0, 0, (void *)1);
	if (rc) {
		err_msg = "unregister irq 2 drv_id 1 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(2, 0, 0, (void *)2);
	if (rc) {
		err_msg = "unregister irq 2 drv_id 2 failed \n";
		goto out;
	}

	rc = metal_irq_register(3, irq_handler, 0, (void *)1);
	if (!rc) {
		err_msg = "register irq 3 drv_id 1 overwrite fail expected\n";
		goto out;
	}
	rc = metal_irq_register(3, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "register irq 3 fail drv_id 2\n";
		goto out;
	}
	rc = metal_irq_unregister(3, irq_handler_1, 0, (void *)0);
	if (!rc) {
		err_msg = "unregister irq 3 match handler fail expected\n";
		goto out;
	}
	rc = metal_irq_unregister(3, irq_handler, 0, (void *)0);
	if (rc) {
		err_msg = "unregister irq 3 match handler failed \n";
		goto out;
	}

	rc = metal_irq_unregister(4, irq_handler, 0, (void *)2);
	if (rc) {
		err_msg = "unregister irq 4 match handler and drv_id 2 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(4, irq_handler, 0, (void *)1);
	if (rc) {
		err_msg = "unregister irq 4 match handler and drv_id 1 failed \n";
		goto out;
	}

	rc = metal_irq_register(5, irq_handler, (void *)10, (void *)1);
	if (rc) {
		err_msg = "register irq 5 fail dev 10 drv_id 1\n";
		goto out;
	}
	rc = metal_irq_register(5, irq_handler, (void *)20, (void *)2);
	if (rc) {
		err_msg = "register irq 5 fail dev 20 drv_id 2\n";
		goto out;
	}
	rc = metal_irq_register(5, irq_handler, (void *)10, (void *)3);
	if (rc) {
		err_msg = "register irq 5 fail dev 10 drv_id 3\n";
		goto out;
	}
	rc = metal_irq_register(5, irq_handler, 0, (void *)4);
	if (rc) {
		err_msg = "register irq 5 fail drv_id 4\n";
		goto out;
	}
	rc = metal_irq_register(5, irq_handler, (void *)10, (void *)5);
	if (rc) {
		err_msg = "register irq 5 fail dev 10 drv_id 5\n";
		goto out;
	}

	rc = metal_irq_unregister(5, irq_handler, (void *)10, (void *)3);
	if (rc) {
		err_msg = "unregister irq 5 match handle, dev 10 and drv_id 3 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(5, 0, 0, (void *)4);
	if (rc) {
		err_msg = "unregister irq 5 drv_id 4 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(5, 0, (void *)10, 0);
	if (rc) {
		err_msg = "unregister irq 5 dev 10 failed \n";
		goto out;
	}
	rc = metal_irq_unregister(5, 0, (void *)20, (void *)2);
	if (rc) {
		err_msg = "unregister irq 5 match dev 20 and drv_id 2 failed \n";
		goto out;
	}

	rc = metal_irq_register(-1, irq_handler, 0, (void *)1);
	if (!rc) {
		err_msg = "register irq -1 should have failed\n";
		goto out;
	}

	/* global interrupt disable/enable normal behavior */
	flags_1=metal_irq_save_disable();
	metal_irq_restore_enable(flags_1);

	/* global interrupt less common */
	flags_1=metal_irq_save_disable();
	flags_2=metal_irq_save_disable();
	metal_irq_restore_enable(flags_2);
	metal_irq_restore_enable(flags_1);

	rc = 0;

out:
	metal_set_log_level(mll);
	if ((err_msg[0] != '\0') && (!rc))
		rc = -EINVAL;
	if (rc) metal_log(METAL_LOG_ERROR, "%s", err_msg);
	return rc;
}

METAL_ADD_TEST(irq);
