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

 /***************************************************************************
  * libmetal_amp_demo.c
  *
  * This application shows how to use IPI to trigger interrupt and how to
  * setup shared memory with libmetal API for communication between processors.
  *
  * This app does the following:
  * 1.  Initialize the platform hardware such as UART, GIC.
  * 2.  Connect the IPI interrupt.
  * 3.  Register IPI device, shared memory descriptor device and shared memory
  *     device with libmetal in the intialization.
  * 4.  In the main application it does the following,
  *     * open the registered libmetal devices: IPI device, shared memory
  *       descriptor device and shared memory device.
  *     * Map the shared memory descriptor as non-cached memory.
  *     * Map the shared memory as non-cached memory. If you do not map the
  *       shared memory as non-cached memory, make sure you flush the cache,
  *       before you notify the remote.
  * 7.  Register the IPI interrupt handler with libmetal.
  * 8.  Run the atomic demo task ipi_task_shm_atomicd():
  *     * Wait for the IPI interrupt from the remote.
  *     * Once it receives the interrupt, it does atomic add by 1 to the
  *       first 32bit of the shared memory descriptor location by 1000 times.
  *     * It will then notify the remote after the calucation.
  *     * As the remote side also does 1000 times add after it has notified
  *       this end. The remote side will check if the result is 2000, if not,
  *       it will error.
  * 9.  Run the shared memory echo demo task ipi_task_echod()
  *     * Wait for the IPI interrupt from the other end.
  *     * If an IPI interrupt is received, copy the message to the current
  *       available RPU to APU buffer, increase the available buffer indicator,
  *       and trigger IPI to notify the remote.
  *     * If "shutdown" message is received, cleanup the libmetal source.
  */
#include <unistd.h>

#include <metal/sys.h>
#include <metal/device.h>
#include <metal/irq.h>
#include <metal/atomic.h>
#include <metal/cpu.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "sys_init.h"

#define IPI_TRIG_OFFSET 0x0
#define IPI_OBS_OFFSET  0x4
#define IPI_ISR_OFFSET  0x10
#define IPI_IMR_OFFSET  0x14
#define IPI_IER_OFFSET  0x18
#define IPI_IDR_OFFSET  0x1C

#define IPI_MASK        0x1000000

#define IPI_DEV_NAME    "ff310000.ipi"
#define SHM0_DESC_DEV_NAME    "3ed00000.shm_desc"
#define SHM1_DESC_DEV_NAME    "3ed10000.shm_desc"
#define SHM_DEV_NAME    "3ed20000.shm"
#define BUS_NAME        "generic"
#define D0_SHM_OFFSET   0x00000
#define D1_SHM_OFFSET   0x20000

#define SHUTDOWN "shutdown"

#define LPRINTF(format, ...) \
	printf("SERVER> " format, ##__VA_ARGS__)

struct shm_mg_s {
	uint32_t avails;
	uint32_t used;
};

typedef uint64_t shm_addr_t;

struct msg_hdr_s {
	uint32_t index;
	int32_t len;
};

struct channel_s {
	struct metal_device *ipi_dev;
	struct metal_io_region *ipi_io;
	unsigned int ipi_mask;
	struct metal_device *shm0_desc_dev;
	struct metal_io_region *shm0_desc_io;
	struct metal_device *shm1_desc_dev;
	struct metal_io_region *shm1_desc_io;
	struct metal_device *shm_dev;
	struct metal_io_region *shm_io;
	atomic_int notified;
	unsigned long d0_start_offset;
	unsigned long d1_start_offset;
};

static struct channel_s ch0;

extern int system_init();
extern int run_comm_task(void *task, void *arg);
extern void wait_for_interrupt(void);

/**
 * @brief ipi_irq_isr() - IPI interrupt handler
 *        It will clear the notified flag to mark it's got an IPI interrupt.
 *
 * @param[in]     vect_id - IPI interrupt vector ID
 * @param[in/out] priv    - communication channel data for this application.
 *
 * @return - If the IPI interrupt is triggered by its remote, it returns
 *          METAL_IRQ_HANDLED. It returns METAL_IRQ_NOT_HANDLED, if it is
 *          not the interupt it expected.
 */
static int ipi_irq_isr (int vect_id, void *priv)
{
	(void)vect_id;
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

/**
 * @brief   ipi_task_shm_atomicd() - Shared memory atomic operation demo
 *          This task will:
 *          * Wait for the remote to trigger IPI.
 *          * Once it receives the IPI interrupt, it start atomic add by 1 for
 *            1000 times to the first 32bit of the shared memory descriptor
 *            location.
 *          * Trigger IPI to notify the remote once it finishes calculation.
 *
 * @param[in] arg - channel information
 */
static void *ipi_task_shm_atomicd(void *arg)
{
	struct channel_s *ch = (struct channel_s *)arg;
	atomic_int *shm_int;
	unsigned int flags;
	int i;

	shm_int = (atomic_int *)metal_io_virt(ch->shm0_desc_io, 0);

	LPRINTF("Wait for atomic test to start.\n");
	while (1) {
		do {
			flags = metal_irq_save_disable();
			if (!atomic_flag_test_and_set(&ch->notified)) {
				metal_irq_restore_enable(flags);
				break;
			}
			wait_for_interrupt();
			metal_irq_restore_enable(flags);
		} while(1);
		for (i = 0; i < 1000; i++)
			atomic_fetch_add(shm_int, 1);
		/* memory barrier */
		atomic_thread_fence(memory_order_acq_rel);

		/* Send the message */
		LPRINTF("SENDING message...\n");
		metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);
		break;
	}

	return NULL;
}

/**
 * @brief   ipi_task_echod() - shared memory ping-pong demo
 *          This task will:
 *          * Wait for IPI interrupt from the remote
 *          * Once it received the interrupt, copy the content from
 *            the ping buffer to the pong buffer.
 *          * Update the shared memory descriptor for the new available
 *            pong buffer.
 *          * Trigger IPI to notifty the remote.
 *
 * @param[in] arg - channel information
 */
static void *ipi_task_echod(void *arg)
{
	struct channel_s *ch = (struct channel_s *)arg;
	struct shm_mg_s *shm0_mg, *shm1_mg;
	shm_addr_t *shm0_addr_array, *shm1_addr_array;
	struct msg_hdr_s *msg_hdr;
	unsigned int flags;
	void *d0, *d1;
	metal_phys_addr_t d0_pa;

	shm0_mg = (struct shm_mg_s *)metal_io_virt(ch->shm0_desc_io, 0);
	shm1_mg = (struct shm_mg_s *)metal_io_virt(ch->shm1_desc_io, 0);
	shm0_addr_array = (void *)shm0_mg + sizeof(struct shm_mg_s);
	shm1_addr_array = (void *)shm1_mg + sizeof(struct shm_mg_s);
	d1 = metal_io_virt(ch->shm_io, ch->d1_start_offset);

	LPRINTF("Wait for echo test to start.\n");
	while (1) {
		do {
			flags = metal_irq_save_disable();
			if (!atomic_flag_test_and_set(&ch->notified)) {
				metal_irq_restore_enable(flags);
				break;
			}
			wait_for_interrupt();
			metal_irq_restore_enable(flags);
		} while(1);
		atomic_thread_fence(memory_order_acq_rel);
		while(shm0_mg->used != shm0_mg->avails) {
			d0_pa = (metal_phys_addr_t)shm0_addr_array[shm0_mg->used];
			d0 = metal_io_phys_to_virt(ch->shm_io, d0_pa);
			if (!d0) {
				LPRINTF("ERROR: failed to get rx address: 0x%lx.\n",
					d0_pa);
				return NULL;
			}
			msg_hdr = (struct msg_hdr_s *)d0;
			if (msg_hdr->len < 0) {
				LPRINTF("ERROR: wrong msg length: %d.\n",
					(int)msg_hdr->len);
				return NULL;
#if DEBUG
			} else {
				LPRINTF("received: %d, %d\n",
					(int)msg_hdr->index, (int)msg_hdr->len);
#endif
			}
			if (msg_hdr->len) {
				if (!strncmp((d0 + sizeof(struct msg_hdr_s)),
					 SHUTDOWN, sizeof(SHUTDOWN))) {
					LPRINTF("Received shutdown message\n");
					return NULL;
				}
			}
			memcpy(d1, d0, sizeof(struct msg_hdr_s) + msg_hdr->len);

			/* Update the d1 address */
			shm1_addr_array[shm1_mg->avails] = (uint64_t)metal_io_virt_to_phys(
					ch->shm_io, d1);
			d1 += (sizeof(struct msg_hdr_s) + msg_hdr->len);
			shm0_mg->used++;
			shm1_mg->avails++;
			/* memory barrier */
			atomic_thread_fence(memory_order_acq_rel);

			/* Send the message */
			metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);
		}
	}

	return NULL;
}

/**
 * @brief    cleanup - cleanup the application
 *           The cleanup funciton will disable the IPI interrupt
 *           close the metal devices and clean the system.
 */
void cleanup(void)
{
	int irq;
	/* Disable IPI interrupt */
	if (ch0.ipi_io) {
		metal_io_write32(ch0.ipi_io, IPI_IDR_OFFSET, ch0.ipi_mask);
		/* Get interrupt ID from IPI metal device */
		irq = (intptr_t)ch0.ipi_dev->irq_info;
		metal_irq_register(irq, 0, ch0.ipi_dev, &ch0);
	}
	if (ch0.ipi_dev)
		metal_device_close(ch0.ipi_dev);
	if (ch0.shm0_desc_dev)
		metal_device_close(ch0.shm0_desc_dev);
	if (ch0.shm1_desc_dev)
		metal_device_close(ch0.shm1_desc_dev);
	if (ch0.shm_dev)
		metal_device_close(ch0.shm_dev);
	sys_cleanup();
}

/**
 * @brief    main function of the demo application.
 *           Here are the steps for the main function:
 *           * call sys_init() function for system related initialization and
 *             metal device registration.
 *           * Open the IPI, shared memory descriptors, and shared memory
 *             devices, and stored the I/O region.
 *           * Register the IPI interrupt handler.
 *           * Enable the IPI interrupt.
 *           * Run the atomic across shared memory task.
 *           * Run the echo demo with shared memory task.
 *           * cleanup the libmetal resource before return.
 * @return   0 - succeeded, non-zero for failures.
 */
int main(void)
{
	struct metal_device *device;
	struct metal_io_region *io;
	int irq;
	uint32_t val;
	int ret = 0;

	if (sys_init()) {
		LPRINTF("ERROR: Failed to initialize system\n");
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

	/* Open shared memory0 descriptor device */
	ret = metal_device_open(BUS_NAME, SHM0_DESC_DEV_NAME, &device);
	if (ret) {
		LPRINTF("ERROR: Failed to open device %s.\n",
			SHM0_DESC_DEV_NAME);
		goto out;
	}

	/* Map shared memory0 descriptor device IO region */
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

	/* Store the shared memory0 descriptor device and I/O region */
	ch0.shm0_desc_dev = device;
	ch0.shm0_desc_io = io;

	/* Open shared memory1 descriptor device */
	ret = metal_device_open(BUS_NAME, SHM1_DESC_DEV_NAME, &device);
	if (ret) {
		LPRINTF("ERROR: Failed to open device %s.\n",
			SHM1_DESC_DEV_NAME);
		goto out;
	}

	/* Map shared memory1 descriptor device IO region */
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

	/* Store the shared memory1 descriptor device and I/O region */
	ch0.shm1_desc_dev = device;
	ch0.shm1_desc_io = io;

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
	ret = run_comm_task(ipi_task_shm_atomicd, &ch0);
	if (ret) {
		LPRINTF("ERROR: Failed to run shared memory atomic task.\n");
		goto out;
	}
	ret = run_comm_task(ipi_task_echod, &ch0);
	if (ret)
		LPRINTF("ERROR: Failed to run IPI communication task.\n");

out:
	cleanup();

	return ret;
}
