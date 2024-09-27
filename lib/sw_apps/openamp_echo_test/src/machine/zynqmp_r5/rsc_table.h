/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (c) 2021-2022 Xilinx, Inc. and Contributors. All rights reserved.
 * Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * This file populates resource table for BM remote
 * for use by the Linux host
 */

#ifndef RSC_TABLE_H_
#define RSC_TABLE_H_

#include <stddef.h>
#include <openamp/open_amp.h>

#if defined __cplusplus
extern "C" {
#endif

#define NO_RESOURCE_ENTRIES         8

/* Resource table for the given remote */
struct remote_resource_table {
	uint32_t version;
	uint32_t num;
	uint32_t reserved[2];
	uint32_t offset[NO_RESOURCE_ENTRIES];
	/* rpmsg vdev entry */
	struct fw_rsc_vdev rpmsg_vdev;
	struct fw_rsc_vdev_vring rpmsg_vring0;
	struct fw_rsc_vdev_vring rpmsg_vring1;
	struct fw_rsc_trace rsc_trace;
}__attribute__((packed, aligned(0x100)));

void *get_resource_table (uint32_t rsc_id, uint32_t *len);

#if defined __cplusplus
}
#endif

#endif /* RSC_TABLE_H_ */
