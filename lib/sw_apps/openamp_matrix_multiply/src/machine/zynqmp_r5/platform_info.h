/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2017-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
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

/**
 * Convenience macros ML_ERR, ML_INFO, ML_DBG add source
 * function name and the line number before the message.
 * Inspired by pr_err, pr_info, etc. in the kernel's printk.h.
 * These should be moved to libmetal/lib/log.h and upstreamed.
 */
#define ML_ERR(fmt, args ...) metal_log(METAL_LOG_ERROR, "%s():%u "fmt, \
		__func__, __LINE__, ##args)
#define ML_INFO(fmt, args ...) metal_log(METAL_LOG_INFO, "%s():%u "fmt, \
		__func__, __LINE__, ##args)
#define ML_DBG(fmt, args ...) metal_log(METAL_LOG_DEBUG, "%s():%u "fmt, \
		__func__, __LINE__, ##args)

/* Interrupt vectors */
#ifdef VERSAL_NET
#define IPI_IRQ_VECT_ID     90
#define POLL_BASE_ADDR      0xEB340000
#define IPI_CHN_BITMASK     0x0000020

#elif defined(versal) /* Versal case */
#define IPI_IRQ_VECT_ID     63
#define POLL_BASE_ADDR       0xFF340000 /* IPI base address*/
#define IPI_CHN_BITMASK     0x0000020 /* IPI channel bit mask for IPI from/to
					   APU */
#else /* ZynqMP case */
#define IPI_IRQ_VECT_ID     XPAR_XIPIPSU_0_INT_ID
#define POLL_BASE_ADDR      XPAR_XIPIPSU_0_BASE_ADDRESS
#define IPI_CHN_BITMASK     0x01000000
#endif /* VERSAL_NET */

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
