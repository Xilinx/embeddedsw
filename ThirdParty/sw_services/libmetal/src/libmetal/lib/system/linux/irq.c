/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file	linux/irq.c
 * @brief	Linux libmetal irq operations
 */

#include <pthread.h>
#include <sched.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/sys.h>
#include <metal/mutex.h>
#include <metal/list.h>
#include <metal/utilities.h>
#include <metal/alloc.h>
#include <sys/time.h>
#include <sys/eventfd.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>

#define MAX_IRQS           FD_SETSIZE  /**< maximum number of irqs */
#define METAL_IRQ_STOP     0xFFFFFFFF  /**< stop interrupts handling thread */

#define METAL_LINUX_IRQ_DISABLED 0 /**< IRQ is disabled */
#define METAL_LINUX_IRQ_ENABLED  1 /**< IRQ is enabled */

/** IRQ descriptor structure */
struct metal_irq_desc {
	metal_irq_handler hd;     /**< irq handler */
	struct metal_device *dev; /**< metal device */
	void *drv_id;             /**< id to identify the driver
	                               of the irq handler*/
	bool state;           /**< IRQ enabling state */
};

struct metal_irqs_state {
	struct metal_irq_desc hds[MAX_IRQS]; /**< irqs handlers descriptor */
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
	int ret;

	if ((irq < 0) || (irq >= MAX_IRQS)) {
		metal_log(METAL_LOG_ERROR,
			  "%s: irq %d is larger than the max supported %d.\n",
			  __func__, irq, MAX_IRQS - 1);
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

	if (_irqs.hds[irq].hd != NULL && hd != NULL &&
	    _irqs.hds[irq].hd != hd) {
		metal_log(METAL_LOG_ERROR, "%s: irq %d already registered."
		          "Will not register again.\n", __func__, irq);
		metal_mutex_release(&_irqs.irq_lock);
		return -EINVAL;
	}

	_irqs.hds[irq].hd = hd;
	_irqs.hds[irq].dev = dev;
	_irqs.hds[irq].drv_id = drv_id;
	if (hd != NULL)
		_irqs.hds[irq].state = METAL_LINUX_IRQ_ENABLED;
	else
		_irqs.hds[irq].state = METAL_LINUX_IRQ_DISABLED;
	metal_mutex_release(&_irqs.irq_lock);

	ret = write(_irqs.irq_reg_fd, &val, sizeof(val));
	if (ret < 0) {
		metal_log(METAL_LOG_DEBUG, "%s: write failed IRQ %d\n",
			  __func__, irq);
	}

	metal_log(METAL_LOG_DEBUG, "%s: registered IRQ %d\n", __func__, irq);
	return 0;
}

unsigned int metal_irq_save_disable()
{
	/* This is to avoid deadlock if it is called in ISR */
	if (pthread_self() == _irqs.irq_pthread)
		return 0;
	metal_mutex_acquire(&_irqs.irq_lock);
	return 0;
}

void metal_irq_restore_enable(unsigned flags)
{
	(void)flags;
	if (pthread_self() != _irqs.irq_pthread)
		metal_mutex_release(&_irqs.irq_lock);
}

void metal_irq_enable(unsigned int vector)
{
	uint64_t val = 1;
	int ret;

	if (vector >= MAX_IRQS) {
		metal_log(METAL_LOG_ERROR,
			  "%s: irq %d is larger than the max supported %d.\n",
			  __func__, vector, MAX_IRQS - 1);
		return;
	}

	metal_mutex_acquire(&_irqs.irq_lock);
	if (_irqs.irq_state == METAL_IRQ_STOP) {
		metal_mutex_release(&_irqs.irq_lock);
		return;
	}
	_irqs.hds[vector].state = METAL_LINUX_IRQ_ENABLED;
	metal_mutex_release(&_irqs.irq_lock);

	ret = write(_irqs.irq_reg_fd, &val, sizeof(val));
	if (ret < 0) {
		metal_log(METAL_LOG_DEBUG, "%s: write failed IRQ %d\n",
			  __func__, vector);
	}
}

void metal_irq_disable(unsigned int vector)
{
	uint64_t val = 1;
	int ret;

	if (vector >= MAX_IRQS) {
		metal_log(METAL_LOG_ERROR,
			  "%s: irq %d is larger than the max supported %d.\n",
			  __func__, vector, MAX_IRQS - 1);
		return;
	}

	metal_mutex_acquire(&_irqs.irq_lock);
	if (_irqs.irq_state == METAL_IRQ_STOP) {
		metal_mutex_release(&_irqs.irq_lock);
		return;
	}
	_irqs.hds[vector].state = METAL_LINUX_IRQ_DISABLED;
	metal_mutex_release(&_irqs.irq_lock);

	ret = write(_irqs.irq_reg_fd, &val, sizeof(val));
	if (ret < 0) {
		metal_log(METAL_LOG_DEBUG, "%s: write failed IRQ %d\n",
			  __func__, vector);
	}
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
	struct pollfd *pfds;

	(void) args;

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
			if (_irqs.hds[i].hd != NULL &&
			    _irqs.hds[i].state == METAL_LINUX_IRQ_ENABLED) {
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
			break;
		}
		/* Waken up from interrupt */
		pfds_total = j;
		for (i = 0; i < pfds_total; i++) {
			if ( (pfds[i].fd == _irqs.irq_reg_fd) &&
			     (pfds[i].revents & (POLLIN | POLLRDNORM))) {
				/* IRQ registration change notification */
				if (read(pfds[i].fd, (void*)&val, sizeof(uint64_t)) < 0)
					metal_log(METAL_LOG_ERROR,
						  "%s, read irq fd %d failed.\n",
						  __func__, pfds[i].fd);
			} else if ((pfds[i].revents & (POLLIN | POLLRDNORM))) {
				struct metal_irq_desc *desc;
				struct metal_device *dev = NULL;
				int irq_handled = 0;

				metal_mutex_acquire(&_irqs.irq_lock);
				desc = &_irqs.hds[pfds[i].fd];
				dev = desc->dev;

				if (desc->hd(pfds[i].fd, desc->drv_id)
				    == METAL_IRQ_HANDLED)
					irq_handled = 1;
				if (irq_handled) {
					if (dev && dev->bus->ops.dev_irq_ack)
						dev->bus->ops.dev_irq_ack(dev->bus, dev, i);
				}
				metal_mutex_release(&_irqs.irq_lock);
			} else if (pfds[i].revents) {
				metal_log(METAL_LOG_DEBUG,
					  "%s: poll unexpected. fd %d: %d\n",
					  __func__, pfds[i].fd, pfds[i].revents);
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
