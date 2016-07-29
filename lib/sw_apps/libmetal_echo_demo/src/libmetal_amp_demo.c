/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * libmetal_echo_demo.c
 *
 * This application shows how to use IPI to trigger interrupt and how to
 * setup shared memory with libmetal API for communication between processors.
 *
 * This app does the following:
 * 1.  Initialize the platform hardware, Uart, GIC.
 * 2.  Connect the IPI
 *     interrupt.
 * 3.  Register IPI device, shared memory descriptor device and shared memory
*      device with libmetal in the intialization.
*  4.  In the main application, open the registered libmetal devices:
*      IPI device, shared memory descriptor device and shared memory device.
*  5.  Map the shared memory descriptor as non-cached memory.
*  6.  Map the shared memory as non-cached memory. If you do not map the
*      shared memory as non-cached memory, before you notify the other end,
*      make sure you flush the cache.
*  7.  Register the IPI interrupt handler with libmetal.
*  8.  Wait for the IPI interrupt from the other end.
*  9.  If IPI interrupt is received, echo the message back to the other end.
*  10. If "shutdown" message is received, cleanup the libmetal source.
 */

#include <stdio.h>
#include "sys_init.h"
#include "xil_printf.h"
#include "xil_exception.h"
#include "metal/sys.h"
#include "metal/irq.h"
#include "metal/io.h"
#include "metal/device.h"
#include "metal/atomic.h"

#define IPI_TRIG_OFFSET 0x0
#define IPI_OBS_OFFSET  0x4
#define IPI_ISR_OFFSET  0x10
#define IPI_IMR_OFFSET  0x14
#define IPI_IER_OFFSET  0x18
#define IPI_IDR_OFFSET  0x1C

#define IPI_MASK        0x1000000

#define IPI_DEV_NAME    "ff310000.ipi"
#define SHM_DESC_DEV_NAME    "3ed00000.shm_desc"
#define SHM_DEV_NAME    "3ed10000.shm"
#define BUS_NAME        "generic"
#define D0_SHM_OFFSET   0x0000
#define D1_SHM_OFFSET   0x8000

#define SHUTDOWN "shutdown"

#define LPRINTF(format, ...) \
	xil_printf("SERVER> " format, ##__VA_ARGS__)

struct shm_mg_s {
	uint64_t d0_pa;
	uint64_t d1_pa;
};

struct msg_hdr_s {
	uint32_t index;
	int32_t len;
};

struct channel_s {
	struct metal_device *ipi_dev;
	struct metal_io_region *ipi_io;
	unsigned int ipi_mask;
	struct metal_device *shm_desc_dev;
	struct metal_io_region *shm_desc_io;
	struct metal_device *shm_dev;
	struct metal_io_region *shm_io;
	atomic_int notified;
	unsigned long d0_start_offset;
	unsigned long d1_start_offset;
};

static struct channel_s ch0;

extern int run_comm_task(void *task, void *arg);
extern void metal_generic_default_poll(void);

int ipi_irq_isr (int vect_id, void *priv)
{
	struct channel_s *ch = (struct channel_s *)priv;
	uint64_t val = 1;

	if (!ch)
		return METAL_IRQ_NOT_HANDLED;
	val = metal_io_read32(ch->ipi_io, IPI_ISR_OFFSET);
	if (val & ch->ipi_mask) {
		metal_io_write32(ch->ipi_io, IPI_ISR_OFFSET, ch->ipi_mask);
		atomic_flag_clear(&ch->notified);
		return METAL_IRQ_HANDLED;
	}
	return METAL_IRQ_NOT_HANDLED;
}

static void *ipi_thread(void *arg)
{
	struct channel_s *ch = (struct channel_s *)arg;
	struct shm_mg_s *shm_mg;
	struct msg_hdr_s *msg_hdr;
	void *d0, *d1;
	metal_phys_addr_t d0_pa;
	unsigned int flags;

	shm_mg = (struct shm_mg_s *)metal_io_virt(ch->shm_desc_io, 0);
	d1 = metal_io_virt(ch->shm_io, ch->d1_start_offset);

	while (1) {
		do {
			flags = metal_irq_save_disable();
			if (!atomic_flag_test_and_set(&ch->notified)) {
				metal_irq_restore_enable(flags);
				break;
			}
			metal_generic_default_poll();
			metal_irq_restore_enable(flags);
		} while(1);
		atomic_thread_fence(memory_order_acq_rel);
		d0_pa = (metal_phys_addr_t)shm_mg->d0_pa;
		d0 = metal_io_phys_to_virt(ch->shm_io, d0_pa);
		if (!d0) {
			LPRINTF("ERROR: failed to get rx address: 0x%x.\n",
				d0_pa);
			return NULL;
		}
		msg_hdr = (struct msg_hdr_s *)d0;
		if (msg_hdr->len < 0) {
			LPRINTF("ERROR: wrong msg length: %d.\n",
				msg_hdr->len);
			return NULL;
		} else {
			LPRINTF("received: %d, %d\n",
				msg_hdr->index, msg_hdr->len);
		}
		if (msg_hdr->len) {
			if (!strncmp((d0 + sizeof(struct msg_hdr_s)),
				 SHUTDOWN, sizeof(SHUTDOWN))) {
				LPRINTF("Received shutdown\n");
				return NULL;
			}
		}
		memcpy(d1, d0, sizeof(struct msg_hdr_s) + msg_hdr->len);

		/* Update the d1 address */
		shm_mg->d1_pa = (uint64_t)metal_io_virt_to_phys(
				ch->shm_io, d1);
		/* memory barrier */
		atomic_thread_fence(memory_order_acq_rel);

		/* Send the message */
		LPRINTF("SENDING message...\n");
		metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);
	}

	return NULL;
}

int main(void)
{
	struct metal_device *device;
	struct metal_io_region *io;
	int irq;
	uint32_t val;
	int ret = 0;

	ret = sys_init();
	if (ret) {
		LPRINTF("ERROR: Failed to initialize platform.\n");
		return -1;
	}

	memset(&ch0, 0, sizeof(ch0));

	atomic_store(&ch0.notified, 1);

	/* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &device);
	if (ret) {
		LPRINTF("ERROR: Failed to open device %s.\n", IPI_DEV_NAME);
		goto out;
	}

	/* Map IPI device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		LPRINTF("ERROR: Failed to map io regio for %s.\n",
			  device->name);
		metal_device_close(device);
		ret = -ENODEV;
		goto out;
	}

	/* Store the IPI device and I/O region */
	ch0.ipi_dev = device;
	ch0.ipi_io = io;

	/* Open shared memory descriptor device */
	ret = metal_device_open(BUS_NAME, SHM_DESC_DEV_NAME, &device);
	if (ret) {
		LPRINTF("ERROR: Failed to open device %s.\n", SHM_DESC_DEV_NAME);
		goto out;
	}

	/* Map shared memory descriptor device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		LPRINTF("ERROR: Failed to map io regio for %s.\n",
			  device->name);
		metal_device_close(device);
		ret = -ENODEV;
		goto out;
	}
	if (!metal_io_mem_map(metal_io_phys(io, 0), io, io->size)) {
		LPRINTF("ERROR: Failed to memory map shmem descriptor.\n");
		metal_device_close(device);
		ret = -ENODEV;
		goto out;
	}

	/* Store the shared memory device and I/O region */
	ch0.shm_desc_dev = device;
	ch0.shm_desc_io = io;

	/* Open shared memory device */
	ret = metal_device_open(BUS_NAME, SHM_DEV_NAME, &device);
	if (ret) {
		LPRINTF("ERROR: Failed to open device %s.\n", SHM_DEV_NAME);
		goto out;
	}

	/* Map shared memory device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		LPRINTF("ERROR: Failed to map io regio for %s.\n",
			  device->name);
		metal_device_close(device);
		ret = -ENODEV;
		goto out;
	}
	if (!metal_io_mem_map(metal_io_phys(io, 0), io, io->size)) {
		LPRINTF("ERROR: Failed to memory map shmem.\n");
		metal_device_close(device);
		ret = -ENODEV;
		goto out;
	}

	/* Store the shared memory device and I/O region */
	ch0.shm_dev = device;
	ch0.shm_io = io;
	ch0.d1_start_offset = D1_SHM_OFFSET;

	/* Get interrupt ID from IPI metal device */
	irq = (intptr_t)ch0.ipi_dev->irq_info;
	if (irq < 0) {
		LPRINTF("ERROR: Failed to request interrupt for %s.\n",
			  device->name);
		ret = -EINVAL;
		goto out;
	}

	ch0.ipi_mask = IPI_MASK;

	LPRINTF("Try to register IPI interrupt.\n");
	ret =  metal_irq_register(irq, ipi_irq_isr, ch0.ipi_dev, &ch0);
	LPRINTF("registered IPI interrupt.\n");
	if (ret)
		goto out;

	/* Enable interrupt */
	metal_io_write32(ch0.ipi_io, IPI_IER_OFFSET, ch0.ipi_mask);
	val = metal_io_read32(ch0.ipi_io, IPI_IMR_OFFSET);
	if (val & ch0.ipi_mask) {
		LPRINTF("ERROR: Failed to enable IPI interrupt.\n");
		ret = -1;
		goto out;
	}
	LPRINTF("enabled IPI interrupt.\n");
	ret = run_comm_task(ipi_thread, &ch0);
	if (ret)
		LPRINTF("ERROR: Failed to run IPI communication task.\n");

out:
	if (ch0.ipi_dev)
		metal_device_close(ch0.ipi_dev);
	if (ch0.shm_dev)
		metal_device_close(ch0.shm_dev);

	sys_cleanup();

	return ret;
}
