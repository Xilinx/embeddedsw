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

/**
 * @file	linux/irq.c
 * @brief	Linux libmetal irq definitions.
 */

#include <pthread.h>
#include <sched.h>
#include "metal/device.h"
#include "metal/irq.h"
#include "metal/sys.h"
#include "metal/mutex.h"
#include <sys/time.h>
#include <sys/eventfd.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#define MAX_IRQS           FD_SETSIZE  /**< maximum number of irqs */
#define MAX_HDS            20          /**< maximum number of
				          handlers per IRQ */
#define METAL_IRQ_STOP     0xFFFFFFFF  /**< stop interrupts handling thread */

/** IRQ handler descriptor structure */
struct metal_irq_hddesc {
	metal_irq_handler hd; /**< irq handler */
	struct metal_device *dev; /**< metal device */
	void *drv_id;         /**< id to identify the driver
	                         of the irq handler*/
};

struct metal_irqs_state {
	struct metal_irq_hddesc hds[MAX_IRQS][MAX_HDS]; /**< irqs
	                                                   handlers
	                                                   descriptors */
	signed char irq_reg_stat[MAX_IRQS]; /**< irqs registration statistics.
	                                It restore how many handlers have
	                                been registered for each IRQ. */

	int   irq_reg_fd; /**< irqs registration notification file
	                    descriptor */

	metal_mutex_t irq_lock; /**< irq handling lock */

	unsigned int irq_state; /**< global irq handling state */

	pthread_t    irq_pthread; /**< irq handling thread id */
};

struct metal_irqs_state _irqs;

int metal_irq_register(int irq,
		       metal_irq_handler hd,
		       struct metal_device *dev,
		       void *drv_id)
{
	uint64_t val = 1;
	struct metal_irq_hddesc *hd_desc;
	int i, ret;

	if (irq < 0) {
		metal_log(METAL_LOG_ERROR,
			  "%s: irq fd %d is less than 0.\n",
			  __func__, irq);
		return -EINVAL;
	}
	if (irq >= MAX_IRQS) {
		metal_log(METAL_LOG_ERROR,
			  "%s: irq fd %d is larger than the max supported %d.\n",
			  __func__, irq, MAX_IRQS);
		return -EINVAL;
	}
	metal_mutex_acquire(&_irqs.irq_lock);
	if (_irqs.irq_state == METAL_IRQ_STOP) {
		metal_log(METAL_LOG_ERROR,
			  "%s: failed. metal IRQ handling has stopped.\n",
			  __func__);
		metal_mutex_release(&_irqs.irq_lock);
		return -EINVAL;
	}

	if (!hd && !drv_id && !dev) {
		memset(&_irqs.hds[irq][0], 0,
			sizeof(struct metal_irq_hddesc)*MAX_HDS);
		_irqs.irq_reg_stat[irq] = 0;
		goto out;
	}
	for(i = 0; i < MAX_HDS; i++) {
		hd_desc = &_irqs.hds[irq][i];
		if ((hd_desc->drv_id == drv_id) &&
		    (hd_desc->dev == dev)) {
			if (hd) {
				metal_log(METAL_LOG_ERROR, "%s: irq %d has already registered."
				         "Will not register again.\n",
				         __func__, irq);
				metal_mutex_release(&_irqs.irq_lock);
				return -EINVAL;
			} else {
				if (_irqs.irq_reg_stat[irq] > 0)
					_irqs.irq_reg_stat[irq]--;
				i++;
				for (; i < (MAX_HDS - 1); i++) {
					hd_desc->hd = _irqs.hds[irq][i].hd;
					hd_desc->dev =
						_irqs.hds[irq][i].dev;
					hd_desc->drv_id =
						_irqs.hds[irq][i].drv_id;
					hd_desc++;
				}
				break;
			}
		} else if (!hd_desc->drv_id && !hd_desc->dev) {
			if (hd) {
				hd_desc->dev = dev;
				hd_desc->drv_id = drv_id;
				hd_desc->hd = hd;
				_irqs.irq_reg_stat[irq]++;
				break;
			}
		}
	}
	if (i == MAX_HDS) {
		metal_log(METAL_LOG_ERROR, "%s: exceed maximum handlers per IRQ.\n",
			  __func__);
		metal_mutex_release(&_irqs.irq_lock);
		return -EINVAL;
	}
out:
	metal_mutex_release(&_irqs.irq_lock);
	ret = write(_irqs.irq_reg_fd, &val, sizeof(val));
	if (ret < 0) {
		metal_log(METAL_LOG_DEBUG, "%s: write failed IRQ %d\n", __func__, irq);
	}
	if (hd)
		metal_log(METAL_LOG_DEBUG, "%s: registered IRQ %d\n", __func__, irq);
	else
		metal_log(METAL_LOG_DEBUG, "%s: deregistered IRQ %d\n", __func__, irq);
	return 0;
}

unsigned int metal_irq_save_disable()
{
	metal_mutex_acquire(&_irqs.irq_lock);
	return 0;
}

void metal_irq_restore_enable(unsigned flags)
{
	(void)flags;
	metal_mutex_release(&_irqs.irq_lock);
}

void metal_irq_enable(unsigned int vector)
{
	(void)vector;
}

void metal_irq_disable(unsigned int vector)
{
	(void)vector;
}

/**
  * @brief       IRQ handler
  * @param[in]   args  not used. required for pthread.
  */
static void *metal_linux_irq_handling(void *args)
{
	struct sched_param param;
	uint64_t val;
	int ret;
	int i, j, pfds_total;
	(void) args;

	struct metal_irq_hddesc *hddec; /**< irq handler descriptro */
	metal_irq_handler  hd; /**< irq handler */
	struct metal_device *dev; /**< metal device which a IRQ belongs to */
	int irq_handled; /**< A flag to indicate if irq is handled */
	struct pollfd *pfds;

	pfds = (struct pollfd *)malloc(MAX_IRQS * sizeof(struct pollfd));
	if (!pfds) {
		metal_log(METAL_LOG_ERROR, "%s: failed to allocate irq fds mem.\n",
			  __func__);
		return NULL;
	}

	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	/* Ignore the set scheduler error */
	ret = sched_setscheduler(0, SCHED_FIFO, &param);
	if (ret) {
		metal_log(METAL_LOG_WARNING, "%s: Failed to set scheduler: %d.\n",
			  __func__, ret);
	}

	while (1) {
		metal_mutex_acquire(&_irqs.irq_lock);
		if (_irqs.irq_state == METAL_IRQ_STOP) {
			/* Killing this IRQ handling thread */
			metal_mutex_release(&_irqs.irq_lock);
			break;
		}

		/* Get the fdset */
		memset(pfds, 0, MAX_IRQS * sizeof(struct pollfd));
		pfds[0].fd = _irqs.irq_reg_fd;
		pfds[0].events = POLLIN;
		for(i = 0, j = 1; i < MAX_IRQS && j < MAX_IRQS; i++) {
			if (_irqs.irq_reg_stat[i] > 0) {
				pfds[j].fd = i;
				pfds[j].events = POLLIN;
				j++;
			}
		}
		metal_mutex_release(&_irqs.irq_lock);
		/* Wait for interrupt */
		ret = poll(pfds, j, -1);
		if (ret < 0) {
			metal_log(METAL_LOG_ERROR, "%s: poll() failed: %s.\n",
				  __func__, strerror(errno));
			free(pfds);
			return NULL;
		}
		/* Waken up from interrupt */
		pfds_total = j;
		for (i = 0; i < pfds_total; i++) {
			if ( (pfds[i].fd == _irqs.irq_reg_fd) &&
			     (pfds[i].revents & (POLLIN | POLLERR))) {
				/* IRQ registration change notification */
				if (read(pfds[i].fd, (void*)&val,
					sizeof(uint64_t)) < 0)
					metal_log(METAL_LOG_ERROR,
					"%s, read irq fd %d failed.\n",
					__func__, pfds[i].fd);
			} else if ((pfds[i].revents & (POLLIN | POLLERR))) {
				irq_handled = 0;
				dev = NULL;
				for(j = 0, hddec = &_irqs.hds[pfds[i].fd][0];
					j < MAX_HDS; j++, hddec++) {
					metal_mutex_acquire(&_irqs.irq_lock);
					if (!hddec->hd) {
						metal_mutex_release(&_irqs.irq_lock);
						break;
					}
					hd = hddec->hd;
					if (!dev)
						dev = hddec->dev;
					metal_mutex_release(&_irqs.irq_lock);

					if (hd(pfds[i].fd, hddec->drv_id) ==
						METAL_IRQ_HANDLED)
						irq_handled = 1;
				}
				if (irq_handled) {
					if (dev && dev->bus->ops.dev_irq_ack)
						dev->bus->ops.dev_irq_ack(
							dev->bus, dev, i);
				}
			} else if (pfds[i].revents) {
				metal_log(METAL_LOG_DEBUG,
					  "%s: poll unexpected. fd %d: %d\n",
					  __func__, pfds[i].fd,pfds[i].revents);
			}
		}
	}
	free(pfds);
	return NULL;
}

/**
  * @brief irq handling initialization
  * @return 0 on sucess, non-zero on failure
  */
int metal_linux_irq_init()
{
	int ret;
	memset(&_irqs, 0, sizeof(_irqs));
	_irqs.irq_reg_fd = eventfd(0,0);
	if (_irqs.irq_reg_fd < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create eventfd for IRQ handling.\n");
		return  -EAGAIN;
	}

	metal_mutex_init(&_irqs.irq_lock);
	ret = pthread_create(&_irqs.irq_pthread, NULL,
				metal_linux_irq_handling, NULL);
	if (ret != 0) {
		metal_log(METAL_LOG_ERROR, "Failed to create IRQ thread: %d.\n", ret);
		return -EAGAIN;
	}
	return 0;
}

/**
  * @brief irq handling shutdown
  */
void metal_linux_irq_shutdown()
{
	int ret;
	uint64_t val = 1;

	metal_log(METAL_LOG_DEBUG, "%s\n", __func__);
	metal_mutex_acquire(&_irqs.irq_lock);
	_irqs.irq_state = METAL_IRQ_STOP;
	metal_mutex_release(&_irqs.irq_lock);
	ret = write (_irqs.irq_reg_fd, &val, sizeof(val));
	if (ret < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to write.\n");
	}
	ret = pthread_join(_irqs.irq_pthread, NULL);
	if (ret) {
		metal_log(METAL_LOG_ERROR, "Failed to join IRQ thread: %d.\n", ret);
	}
	close(_irqs.irq_reg_fd);
	metal_mutex_deinit(&_irqs.irq_lock);
}
