/* SPDX-License-Identifier: GPL-2.0 */
/*
 * The header for UIO driver
 *
 * Copyright (C) 2019 Xilinx, Inc.
 *
 * Author: Hyun Woo Kwon <hyun.kwon@xilinx.com>
 */

#ifndef _UAPI_UIO_UIO_H_
#define _UAPI_UIO_UIO_H_

#include <linux/ioctl.h>
#include <linux/types.h>

#define UIO_DMABUF_DIR_BIDIR	1
#define UIO_DMABUF_DIR_TO_DEV	2
#define UIO_DMABUF_DIR_FROM_DEV	3
#define UIO_DMABUF_DIR_NONE	4

struct uio_dmabuf_args {
	__s32	dbuf_fd;
	__u64	dma_addr;
	__u64	size;
	__u32	dir;
};

#define UIO_IOC_BASE		'U'

#define	UIO_IOC_MAP_DMABUF	_IOWR(UIO_IOC_BASE, 0x1, struct uio_dmabuf_args)
#define	UIO_IOC_UNMAP_DMABUF	_IOWR(UIO_IOC_BASE, 0x2, struct uio_dmabuf_args)

#endif
