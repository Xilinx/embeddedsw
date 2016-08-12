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

/*
 * @file	generic/irq.c
 * @brief	generic libmetal irq definitions.
 */

#include <errno.h>
#include "metal/irq.h"
#include "metal/sys.h"
#include "metal/log.h"
#include "metal/mutex.h"


#define MAX_HDS            10           /**< maximum number of
                                          handlers per IRQ */
/** IRQ handler descriptor structure */
struct metal_irq_hddesc {
        metal_irq_handler hd; /**< irq handler */
        void *drv_id;         /**< id to identify the driver
                                 of the irq handler*/
};

struct metal_irqs_state {
        struct metal_irq_hddesc hds[MAX_IRQS][MAX_HDS]; /**< irqs
                                                           handlers
                                                           descriptors */
        unsigned int intr_enable;
        metal_mutex_t irq_lock;
};

static struct metal_irqs_state _irqs;


int metal_irq_register(int irq,
                       metal_irq_handler hd,
                       struct metal_device *dev,
                       void *drv_id)
{
	unsigned int irq_flags_save;
	struct metal_irq_hddesc *hd_desc;
	int j,i = 0;

	(void)dev;

	if (irq < 0) {
		metal_log(LOG_ERROR, "%s: irq %d is less than 0.\n", __func__, irq);
		return -EINVAL;
	}
	if (irq >= MAX_IRQS) {
		metal_log(LOG_ERROR,
				"%s: irq %d is larger than the max supported %d.\n", __func__,
				irq, MAX_IRQS);
		return -EINVAL;
	}

	if (hd && !drv_id) {
		metal_log(LOG_ERROR, "%s: irq %d drv id cannot be 0.\n", __func__, irq);
		return -EINVAL;
	}

	metal_mutex_acquire(&_irqs.irq_lock);
	while (i < MAX_HDS) {
		hd_desc = &_irqs.hds[irq][i];
		if ((hd_desc->drv_id == drv_id) || (!drv_id && !hd)) {
			if (hd) {
				metal_log(LOG_ERROR, "%s: irq %d has already registered."
						"Will not register again.\n", __func__, irq);
				metal_mutex_release(&_irqs.irq_lock);
				return -EINVAL;
			} else {
				/* we are at end of registered hds */
				if (!hd_desc->drv_id)
					break;

				/* Make sure there are no hole in the hds */
				irq_flags_save=metal_irq_save_disable();
				for (j = i+1; j < MAX_HDS && 0 != _irqs.hds[irq][j].drv_id; j++) {
					_irqs.hds[irq][j-1] = _irqs.hds[irq][j];
				}
				hd_desc = &_irqs.hds[irq][j-1];
				hd_desc->hd = 0;
				hd_desc->drv_id = 0;
				metal_irq_restore_enable(irq_flags_save);

				/* If drv_id and hd are null, it will deregister
				 * all the handlers of the specified IRQ.
				 * Otherwise only the matching drv_id will deregister.
				 */
				if (drv_id)
					break;

				continue;
			}
		} else if (!hd_desc->drv_id && hd) {
			irq_flags_save=metal_irq_save_disable();
			hd_desc->drv_id = drv_id;
			hd_desc->hd = hd;
			metal_irq_restore_enable(irq_flags_save);
			break;
		} else if (!hd_desc->drv_id) {
			metal_log(LOG_ERROR, "%s: irq %d drv id not found.\n", __func__, irq);
			metal_mutex_release(&_irqs.irq_lock);
			return -EINVAL;
		}
		i++;
	}
	metal_mutex_release(&_irqs.irq_lock);

	if (i >= MAX_HDS) {
		metal_log(LOG_ERROR, "%s: exceed maximum handlers per IRQ.\n",
				__func__);
		return -EINVAL;
	}

	metal_log(LOG_DEBUG, "%s: %sregistered IRQ %d\n", __func__,
			  hd ? "" : "de",	irq);

	return 0;
}

unsigned int metal_irq_save_disable(void)
{
	if (_irqs.intr_enable == 1) {
		sys_irq_save_disable();
		_irqs.intr_enable = 0;
	}

	return 0;
}

void metal_irq_restore_enable(unsigned int flags)
{
	(void)flags;

	if (_irqs.intr_enable == 0) {
		sys_irq_restore_enable();
		_irqs.intr_enable = 1;
	}
}

void metal_irq_enable(unsigned int vector)
{
        sys_irq_enable(vector);
}

void metal_irq_disable(unsigned int vector)
{
        sys_irq_disable(vector);
}

/**
 * @brief default handler
 */
void metal_irq_isr(unsigned int vector)
{
        metal_irq_handler  hd; /**< irq handler */
        int j;

        for(j = 0; j < MAX_HDS; j++) {
		hd = _irqs.hds[vector][j].hd;
		/* there are no hole in the hds */
		if (!hd) break;
		hd(vector, _irqs.hds[vector][j].drv_id);
        }
}

int metal_irq_init(void)
{
       /* memset(&_irqs, 0, sizeof(_irqs)); */

       metal_mutex_init(&_irqs.irq_lock);
       return 0;
}

void metal_irq_deinit(void)
{
       metal_mutex_deinit(&_irqs.irq_lock);
}
