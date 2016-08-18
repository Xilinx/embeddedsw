/*
 * Zynq Remote Processor Messaging Framework driver
 *
 * Copyright (C) 2014 Mentor Graphics Corporation
 *
 * Based on Zynq Remote Processor driver
 *
 * Copyright (C) 2012 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2012 PetaLogix
 *
 * Based on origin OMAP Remote Processor driver
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 * Copyright (C) 2011 Google, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define ZYNQ_RPMSG_NUM_VRINGS		2
struct zynq_rpmsg_vring {

	void *va;
	dma_addr_t dma;
	int len;
	u32 da;
	u32 align;
	struct virtqueue *vq;
};

struct zynq_rpmsg_instance {

	u32 vring0;
	u32 vring1;
	u32 mem_start;
	u32 mem_end;
	u32 num_descs;
	u32 dev_feature;
	u32 gen_feature;
	u32 num_vrings;
	u32 align;
	u32 virtioid;
	u32 ringtx;
	u32 ringrx;

	struct virtio_device virtio_dev;

	struct zynq_rpmsg_vring vrings[ZYNQ_RPMSG_NUM_VRINGS];

	struct device mid_dev;
};

#ifndef CONFIG_SMP
extern int set_ipi_handler(int ipinr, void *handler, char *desc);

extern void clear_ipi_handler(int ipinr);

extern void gic_raise_softirq_unicore(unsigned long cpu, unsigned int irq);
#endif
