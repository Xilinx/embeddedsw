/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2022 Xilinx, Inc.
 * Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**************************************************************************
 * FILE NAME
 *
 *       platform_info.c
 *
 * DESCRIPTION
 *
 *       This file define platform specific data and implements APIs to set
 *       platform specific information for OpenAMP.
 *
 **************************************************************************/

#include <metal/atomic.h>
#include <metal/assert.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/utilities.h>
#include <openamp/rpmsg_virtio.h>
#include <errno.h>
#include "platform_info.h"
#include "rsc_table.h"

#ifndef RPMSG_NO_IPI
#define _rproc_wait() __asm volatile("wfi")
#endif /* !RPMSG_NO_IPI */

#ifdef USE_FREERTOS
extern TaskHandle_t rpmsg_task;
#endif /* USE_FREERTOS */

/* Polling information used by remoteproc operations.
 */
static metal_phys_addr_t poll_phys_addr = POLL_BASE_ADDR;
struct metal_device kick_device = {
	.name = "poll_dev",
	.bus = NULL,
	.num_regions = 1,
	.regions = {
		{
			.virt = (void *)POLL_BASE_ADDR,
			.physmap = &poll_phys_addr,
			.size = 0x1000,
			.page_shift = -1UL,
			.page_mask = -1UL,
			.mem_flags = DEVICE_NONSHARED | PRIV_RW_USER_RW,
			.ops = {NULL},
		}
	},
	.node = {NULL},
#ifndef RPMSG_NO_IPI
	.irq_num = 1,
	.irq_info = (void *)IPI_IRQ_VECT_ID,
#endif /* !RPMSG_NO_IPI */
};

static struct remoteproc_priv rproc_priv = {
	.kick_dev_name = KICK_DEV_NAME,
	.kick_dev_bus_name = KICK_BUS_NAME,
#ifndef RPMSG_NO_IPI
	.ipi_chn_mask = IPI_CHN_BITMASK,
#endif /* !RPMSG_NO_IPI */
};

static struct remoteproc rproc_inst;

/* External functions */
extern int init_system(void);
extern void cleanup_system(void);

/* processor operations from r5 to a53. It defines
 * notification operation and remote processor managementi operations. */
extern const struct remoteproc_ops zynqmp_r5_a53_proc_ops;

/* RPMsg virtio shared buffer pool */
static struct rpmsg_virtio_shm_pool shpool;

static struct remoteproc *
platform_create_proc(int proc_index, int rsc_index)
{
	void *rsc_table;
	int rsc_size;
	int ret;
	metal_phys_addr_t pa;

	(void) proc_index;
	rsc_table = get_resource_table(rsc_index, &rsc_size);
	ML_INFO("rsc_table, rsc_size = %#x, %#x\r\n", rsc_table, rsc_size);

	/* Register IPI device */
	if (metal_register_generic_device(&kick_device))
		return NULL;

	/* Initialize remoteproc instance */
	if (!remoteproc_init(&rproc_inst, &zynqmp_r5_a53_proc_ops, &rproc_priv))
		return NULL;

	ML_DBG("poll{name,bus,chn_mask} = %s,%s,%#x\r\n",
		rproc_priv.kick_dev_name,
		rproc_priv.kick_dev_bus_name,
		IPI_CHN_BITMASK);
	/*
	 * Mmap shared memories
	 * Or shall we constraint that they will be set as carved out
	 * in the resource table?
	 */
	/* mmap resource table */
	pa = (metal_phys_addr_t)rsc_table;
	(void *)remoteproc_mmap(&rproc_inst, &pa,
				NULL, rsc_size,
				NORM_NSHARED_NCACHE|PRIV_RW_USER_RW,
				&rproc_inst.rsc_io);
	/* mmap shared memory */
	pa = SHARED_MEM_PA;
	(void *)remoteproc_mmap(&rproc_inst, &pa,
				NULL, SHARED_MEM_SIZE,
				NORM_NSHARED_NCACHE|PRIV_RW_USER_RW,
				NULL);

	/* parse resource table to remoteproc */
	ret = remoteproc_set_rsc_table(&rproc_inst, rsc_table, rsc_size);
	if (ret) {
		ML_ERR("Failed to initialize remoteproc\r\n");
		remoteproc_remove(&rproc_inst);
		return NULL;
	}
	ML_INFO("Initialize remoteproc successfully.\r\n");

	return &rproc_inst;
}

int platform_init(int argc, char *argv[], void **platform)
{
	unsigned long proc_id = 0;
	unsigned long rsc_id = 0;
	struct remoteproc *rproc;

	/* metal_log setup is in init_system */
	if (!platform) {
		xil_printf("Failed to initialize platform,"
			   "NULL pointer to store platform data.\r\n");
		return -EINVAL;
	}
	/* Initialize HW system components */
	init_system();

	if (argc >= 2) {
		proc_id = strtoul(argv[1], NULL, 0);
	}

	if (argc >= 3) {
		rsc_id = strtoul(argv[2], NULL, 0);
	}

	ML_INFO("platform_create_proc()\r\n");
	rproc = platform_create_proc(proc_id, rsc_id);
	if (!rproc) {
		ML_ERR("Failed to create remoteproc device.\r\n");
		return -EINVAL;
	}
	*platform = rproc;
	return 0;
}

struct  rpmsg_device *
platform_create_rpmsg_vdev(void *platform, unsigned int vdev_index,
			   unsigned int role,
			   void (*rst_cb)(struct virtio_device *vdev),
			   rpmsg_ns_bind_cb ns_bind_cb)
{
	struct remoteproc *rproc = platform;
	struct rpmsg_virtio_device *rpmsg_vdev;
	struct virtio_device *vdev;
	void *shbuf;
	struct metal_io_region *shbuf_io;
	int ret;

	rpmsg_vdev = metal_allocate_memory(sizeof(*rpmsg_vdev));
	if (!rpmsg_vdev)
		return NULL;
	shbuf_io = remoteproc_get_io_with_pa(rproc, SHARED_MEM_PA);
	if (!shbuf_io)
		goto err1;
	shbuf = metal_io_phys_to_virt(shbuf_io,
				      SHARED_MEM_PA + SHARED_BUF_OFFSET);

	ML_INFO("creating remoteproc virtio rproc %p\r\n", rproc);
	/* TODO: can we have a wrapper for the following two functions? */
	vdev = remoteproc_create_virtio(rproc, vdev_index, role, rst_cb);
	if (!vdev) {
		ML_ERR("failed remoteproc_create_virtio\r\n");
		goto err1;
	}

	ML_INFO("initializing rpmsg shared buffer pool\r\n");
	/* Only RPMsg virtio master needs to initialize the shared buffers pool */
	rpmsg_virtio_init_shm_pool(&shpool, shbuf,
				   (SHARED_MEM_SIZE - SHARED_BUF_OFFSET));

	ML_INFO("initializing rpmsg vdev\r\n");
	/* RPMsg virtio device can set shared buffers pool argument to NULL */
	ret =  rpmsg_init_vdev(rpmsg_vdev, vdev, ns_bind_cb,
			       shbuf_io,
			       &shpool);
	if (ret) {
		ML_ERR("failed rpmsg_init_vdev\r\n");
		goto err2;
	}
	return rpmsg_virtio_get_rpmsg_device(rpmsg_vdev);
err2:
	remoteproc_remove_virtio(rproc, vdev);
err1:
	metal_free_memory(rpmsg_vdev);
	return NULL;
}

int platform_poll_for_rpc(void *arg)
{
	struct rproc_plat_info *data = arg;
	struct rpmsg_device *rpdev = data->rpdev;
	struct remoteproc *rproc = data->rproc;
	struct rpmsg_virtio_device *rvdev;
	struct remoteproc_priv *prproc;
	unsigned int flags;
	int ret;

	if (!rproc || !rpdev)
		return -EINVAL;

	prproc = rproc->priv;
	if (!prproc)
		return -EINVAL;

	rvdev = metal_container_of(rpdev, struct rpmsg_virtio_device, rdev);

	/**
	 * Check virtio status after every interrupt. In case of stop or
	 * detach, virtio device status will be reset by remote
	 * processor. In that case, break loop and destroy rvdev
	 */
	while (rpmsg_virtio_get_status(rvdev) & VIRTIO_CONFIG_STATUS_DRIVER_OK) {
#ifdef RPMSG_NO_IPI
		(void)flags;
		if (metal_io_read32(prproc->kick_io, 0))
			remoteproc_get_notification(rproc, RSC_NOTIFY_ID_ANY);
#else /* !RPMSG_NO_IPI */
		flags = metal_irq_save_disable();
		if (!(atomic_flag_test_and_set(&prproc->ipi_nokick))) {
			metal_irq_restore_enable(flags);
			ret = remoteproc_get_notification(rproc, RSC_NOTIFY_ID_ANY);
			if (ret)
				return ret;
			break;
		}
#ifdef USE_FREERTOS
		vTaskSuspend(rpmsg_task);
#else
		_rproc_wait();
#endif /* USE_FREERTOS */
		metal_irq_restore_enable(flags);
#endif /* RPMSG_NO_IPI */
	}
	return 0;
}

int platform_poll(void *priv)
{
	struct remoteproc *rproc = priv;
	struct remoteproc_priv *prproc;
	unsigned int flags;
	int ret;

	prproc = rproc->priv;
	while(1) {
#ifdef RPMSG_NO_IPI
		if (metal_io_read32(prproc->kick_io, 0)) {
			ret = remoteproc_get_notification(rproc,
							  RSC_NOTIFY_ID_ANY);
			if (ret)
				return ret;
			break;
		}
		(void)flags;
#else /* !RPMSG_NO_IPI */
		flags = metal_irq_save_disable();
		if (!(atomic_flag_test_and_set(&prproc->ipi_nokick))) {
			metal_irq_restore_enable(flags);
			ret = remoteproc_get_notification(rproc,
							  RSC_NOTIFY_ID_ANY);
			if (ret)
				return ret;
			break;
		}
		_rproc_wait();
		metal_irq_restore_enable(flags);
#endif /* RPMSG_NO_IPI */
	}
	return 0;
}

void platform_release_rpmsg_vdev(struct rpmsg_device *rpdev, void *platform)
{
	struct rpmsg_virtio_device *rpvdev;
	struct remoteproc *rproc;

	rpvdev = metal_container_of(rpdev, struct rpmsg_virtio_device, rdev);
	rproc = platform;

	rpmsg_deinit_vdev(rpvdev);
	remoteproc_remove_virtio(rproc, rpvdev->vdev);
}

void platform_cleanup(void *platform)
{
	struct remoteproc *rproc = platform;

	if (rproc)
		remoteproc_remove(rproc);
	cleanup_system();
}
