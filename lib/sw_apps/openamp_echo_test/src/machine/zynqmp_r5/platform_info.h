/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2017-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_INFO_H_
#define PLATFORM_INFO_H_

#include <openamp/remoteproc.h>
#include <openamp/virtio.h>
#include <openamp/rpmsg.h>
#include <metal/log.h>
#include "xreg_cortexr5.h"

#if defined __cplusplus
extern "C" {
#endif

/* Interrupt vectors */
#ifdef VERSAL_NET

#ifndef IPI_IRQ_VECT_ID
#define IPI_IRQ_VECT_ID     90
#endif /* IPI_IRQ_VECT_ID */

#ifndef POLL_BASE_ADDR
#define POLL_BASE_ADDR      0xEB340000
#endif /* POLL_BASE_ADDR */

#ifndef IPI_CHN_BITMASK
#define IPI_CHN_BITMASK     0x0000020
#endif /* IPI_CHN_BITMASK */

#elif defined(versal) /* Versal case */

#ifndef IPI_IRQ_VECT_ID
#define IPI_IRQ_VECT_ID     63
#endif /* IPI_IRQ_VECT_ID */

#ifndef POLL_BASE_ADDR
/* IPI base address */
#define POLL_BASE_ADDR       0xFF340000
#endif /* POLL_BASE_ADDR */

#ifndef IPI_CHN_BITMASK
#define IPI_CHN_BITMASK     0x0000020
#endif /* IPI_CHN_BITMASK */

#else /* ZynqMP case */

#ifndef IPI_IRQ_VECT_ID
#define IPI_IRQ_VECT_ID     XPAR_XIPIPSU_0_INT_ID
#endif /* IPI_IRQ_VECT_ID */

#ifndef POLL_BASE_ADDR
#define POLL_BASE_ADDR      XPAR_XIPIPSU_0_BASE_ADDRESS
#endif /* POLL_BASE_ADDR */

#ifndef IPI_CHN_BITMASK
#define IPI_CHN_BITMASK     0x01000000
#endif /* IPI_CHN_BITMASK */

#endif /* VERSAL_NET */

#if XPAR_CPU_ID == 0

#ifndef SHARED_MEM_PA
#define SHARED_MEM_PA  0x3ED40000UL
#endif /* !SHARED_MEM_PA */

#else

#ifndef SHARED_MEM_PA
#define SHARED_MEM_PA  0x3EF40000UL
#endif /* !SHARED_MEM_PA */

#endif /* XPAR_CPU_ID */

#define KICK_DEV_NAME         "poll_dev"
#define KICK_BUS_NAME         "generic"

#ifndef SHARED_MEM_SIZE
#define SHARED_MEM_SIZE 0x100000UL
#endif /* !SHARED_MEM_SIZE */

#ifndef SHARED_BUF_OFFSET
#define SHARED_BUF_OFFSET 0x8000UL
#endif /* !SHARED_BUF_OFFSET */

#ifdef RPMSG_NO_IPI
#undef POLL_BASE_ADDR
#define POLL_BASE_ADDR 0x3EE40000
#define POLL_STOP 0x1U
#endif /* RPMSG_NO_IPI */

struct remoteproc_priv {
	const char *kick_dev_name;
	const char *kick_dev_bus_name;
	struct metal_device *kick_dev;
	struct metal_io_region *kick_io;
#ifndef RPMSG_NO_IPI
	unsigned int ipi_chn_mask; /**< IPI channel mask */
	atomic_int ipi_nokick;
#endif /* !RPMSG_NO_IPI */
};

/**
 * platform_init - initialize the platform
 *
 * It will initialize the platform.
 *
 * @argc: number of arguments
 * @argv: array of the input arguments
 * @platform: pointer to store the platform data pointer
 *
 * return 0 for success or negative value for failure
 */
int platform_init(int argc, char *argv[], void **platform);

/**
 * platform_create_rpmsg_vdev - create rpmsg vdev
 *
 * It will create rpmsg virtio device, and returns the rpmsg virtio
 * device pointer.
 *
 * @platform: pointer to the private data
 * @vdev_index: index of the virtio device, there can more than one vdev
 *              on the platform.
 * @role: virtio driver or virtio device of the vdev
 * @rst_cb: virtio device reset callback
 * @ns_bind_cb: rpmsg name service bind callback
 *
 * return pointer to the rpmsg virtio device
 */
struct rpmsg_device *
platform_create_rpmsg_vdev(void *platform, unsigned int vdev_index,
			   unsigned int role,
			   void (*rst_cb)(struct virtio_device *vdev),
			   rpmsg_ns_bind_cb ns_bind_cb);

/**
 * platform_poll - platform poll function
 *
 * @platform: pointer to the platform
 *
 * return negative value for errors, otherwise 0.
 */
int platform_poll(void *platform);

struct rproc_plat_info {
	struct rpmsg_device *rpdev;
	struct remoteproc *rproc;
};

int platform_poll_on_vdev_reset(void *arg);


/**
 * platform_release_rpmsg_vdev - release rpmsg virtio device
 *
 * @rpdev: pointer to the rpmsg device
 */
void platform_release_rpmsg_vdev(struct rpmsg_device *rpdev, void *platform);

/**
 * platform_cleanup - clean up the platform resource
 *
 * @platform: pointer to the platform
 */
void platform_cleanup(void *platform);

#if defined __cplusplus
}
#endif

#endif /* PLATFORM_INFO_H_ */
