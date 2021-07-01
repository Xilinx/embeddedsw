/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2021 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This file populates resource table for BM remote
 * for use by the Linux Master */

#include <openamp/open_amp.h>
#include "rsc_table.h"

/* Place resource table in special ELF section */
#define __section_t(S)          __attribute__((__section__(#S)))
#define __resource              __section_t(.resource_table)

#define RPMSG_IPU_C0_FEATURES        1

/* VirtIO rpmsg device id */
#define VIRTIO_ID_RPMSG_             7

/* Remote supports Name Service announcement */
#define VIRTIO_RPMSG_F_NS           0

#define NUM_VRINGS                  0x02
#define VRING_ALIGN                 0x1000
#define RING_TX                     FW_RSC_U32_ADDR_ANY
#define RING_RX                     FW_RSC_U32_ADDR_ANY
#define VRING_SIZE                  256

#define NUM_TABLE_ENTRIES           2
/* Trace buffer for the rsc_trace entry */
#if !defined(RSC_TRACE_SZ)
#define RSC_TRACE_SZ (4*1024)
#endif /* RSC_TRACE_SZ */
static char rsc_trace_buf[RSC_TRACE_SZ];

struct remote_resource_table __resource resources = {
	.version = 1,
	.num = NUM_TABLE_ENTRIES,
	.reserved = {0, 0},
	.offset[0] = offsetof(struct remote_resource_table, rpmsg_vdev),
	.offset[1] = offsetof(struct remote_resource_table, rsc_trace),
	/* Virtio device entry */
	.rpmsg_vdev = {
		.type =		RSC_VDEV,
		.id =		VIRTIO_ID_RPMSG_,
		.notifyid =	0,
		.dfeatures =	RPMSG_IPU_C0_FEATURES,
		.gfeatures =	0,
		.config_len =	0,
		.status =	0,
		.num_of_vrings = NUM_VRINGS,
		.reserved =	{0, 0},
	},
	/* Vring rsc entry - part of vdev rsc entry */
	.rpmsg_vring0 = {RING_TX, VRING_ALIGN, VRING_SIZE, 1, 0},
	.rpmsg_vring1 = {RING_RX, VRING_ALIGN, VRING_SIZE, 2, 0},
	/* trace buffer for logs, accessible via debugfs */
	.rsc_trace = {
		.type =		RSC_TRACE,
		.da =		(unsigned int)rsc_trace_buf,
		.len =		sizeof(rsc_trace_buf),
		.reserved =	0,
		.name =		"r5_trace",
	},
};

char *get_rsc_trace_info(unsigned int *len)
{
	*len = sizeof(rsc_trace_buf);
	return rsc_trace_buf;
}

void *get_resource_table (int rsc_id, int *len)
{
	(void) rsc_id;
	*len = sizeof(resources);
	return &resources;
}
