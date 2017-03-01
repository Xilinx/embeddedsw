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

 /*****************************************************************************
  * libmetal_amp_demo.c
  *
  * This application shows how to use IPI to trigger interrupt and how to
  * setup shared memory with libmetal API for communication between processors.
  *
  * This app does the following:
  * 1.  In the main application it does the following,
  *     * open the registered libmetal devices: IPI device, shared memory
  *       descriptor device and shared memory device.
  * 2.  Register the IPI interrupt handler with libmetal.
  * 3.  Run the atomic demo task ipi_task_shm_atomic():
  *     * Trigger the IPI to the remote, the remote will then start doing atomic
  *       add calculation.
  *     * Start atomic add by 1 for 1000 times to the first 32bit of the shared
  *       memory descriptor location.
  *     * Once it receives the IPI interrupt, it will check if the value stored
  *       in the shared memory descriptor location is 2000. If yes, the atomic
  *       across the shared memory passed, otherwise, it failed.
  * 4.  Run the shared memory echo demo task ipi_task_echo()
  *     * Write message to the APU to RPU shared buffer.
  *     * Update the APU to RPU shared memory available index.
  *     * Trigger IPI to the remote.
  *     * Repeat the above 3 sub steps until it sends all the packages.
  *     * Wait for IPI to receive all the packages
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
#include <time.h>

#define IPI_TRIG_OFFSET 0x0
#define IPI_OBS_OFFSET  0x4
#define IPI_ISR_OFFSET  0x10
#define IPI_IMR_OFFSET  0x14
#define IPI_IER_OFFSET  0x18
#define IPI_IDR_OFFSET  0x1C

#define IPI_MASK        0x100

#define IPI_DEV_NAME    "ff340000.ipi"
#define SHM0_DESC_DEV_NAME    "3ed00000.shm_desc"
#define SHM1_DESC_DEV_NAME    "3ed10000.shm_desc"
#define SHM_DEV_NAME    "3ed20000.shm"
#define BUS_NAME        "platform"
#define D0_SHM_OFFSET   0x00000
#define D1_SHM_OFFSET   0x20000

#define NS_PER_S        (1000 * 1000 * 1000)

#define PKGS_TOTAL 1024

#define SHUTDOWN "shutdown"

#define LPRINTF(format, ...) \
	printf("CLIENT> " format, ##__VA_ARGS__)

struct shm_mg_s {
	uint32_t avails;
	uint32_t used;
};

typedef uint64_t shm_addr_t;

struct msg_hdr_s {
	uint32_t index;
	uint32_t len;
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

/**
 * @breif get_timestamp() - Get the timestamp
 *        IT gets the timestamp and return nanoseconds.
 *
 * @return nano seconds.
 */
static unsigned long long get_timestamp (void)
{
	unsigned long long t = 0;
	struct timespec tp;
	int r;

	r = clock_gettime(CLOCK_MONOTONIC, &tp);
	if (r == -1) {
		LPRINTF("ERROR: Bad clock_gettime!\n");
		return t;
	} else {
		t = tp.tv_sec * (NS_PER_S);
		t += tp.tv_nsec;
	}
	return t;
}

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
	(void) vect_id;
	struct channel_s *ch = (struct channel_s *)priv;
	uint64_t val = 1;

	if (!priv)
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
 * @brief   ipi_task_shm_atomic() - Shared memory atomic operation demo
 *          This task will:
 *          * Trigger IPI to notifty the remote to start atmoic add on the
 *            shared memory descriptor memory for 1000 times.
 *          * Start atomic add by 1 for 1000 times to the first 32bit of the
 *            shared memory descriptor location.
 *          * Wait for the remote to trigger IPI
 *          * Once it received the IPI kick from the remote, it will check if
 *            the value stored in the shared memory for the atomic add is 2000.
 *          * It will print if the atomic add test has passed or not.
 *
 * @param[in] arg - channel information
 */
static void *ipi_task_shm_atomic(void *arg)
{
	int i;
	atomic_int *shm_int;
	struct channel_s *ch = (struct channel_s *)arg;

	shm_int = (atomic_int *)metal_io_virt(ch->shm0_desc_io, 0);
	atomic_store(shm_int, 0);

	LPRINTF("Start shm atomic testing...\n");
	/* Kick the remote to start counting */
	metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);
	for (i = 0; i < 1000; i++) {
		atomic_fetch_add(shm_int, 1);
	}

	do {
		if (!atomic_flag_test_and_set(&ch->notified)) {
			metal_cpu_yield();
			break;
		}
	} while(1);
	if (atomic_load(shm_int) == 2000) {
		LPRINTF("shm atomic testing PASS!\n");
	} else {
		LPRINTF("shm atomic testing FAILED. actual: %u\n", atomic_load(shm_int));
	}

	return NULL;
}

/**
 * @brief   ipi_task_echo() - shared memory ping-pong demo
 *          This task will:
 *          * Get the timestamp and put it into the ping shared memory
 *          * Update the shared memory descriptor for the new available
 *            ping buffer.
 *          * Trigger IPI to notifty the remote.
 *          * Repeat the above steps until it sends out all the packages.
 *          * Monitor IPI interrupt, verify every received package.
 *          * After all the packages are received, it sends out shutdown
 *            message to the remote.
 *
 * @param[in] arg - channel information
 */
static void *ipi_task_echo(void *arg)
{
	int i;
	struct channel_s *ch = (struct channel_s *)arg;
	struct shm_mg_s *shm0_mg, *shm1_mg;
	shm_addr_t *shm0_addr_array, *shm1_addr_array;
	struct msg_hdr_s *msg_hdr, *msg_hdr_echo;
	void *d0, *d1;
	metal_phys_addr_t d1_pa;
	unsigned long long tstart, tend;
	long long tdiff;
	long long tdiff_avg_s = 0, tdiff_avg_ns = 0;

	shm0_mg = (struct shm_mg_s *)metal_io_virt(ch->shm0_desc_io, 0);
	shm1_mg = (struct shm_mg_s *)metal_io_virt(ch->shm1_desc_io, 0);
	shm0_addr_array = (void *)shm0_mg + sizeof(struct shm_mg_s);
	shm1_addr_array = (void *)shm1_mg + sizeof(struct shm_mg_s);
	d0 = metal_io_virt(ch->shm_io, ch->d0_start_offset);

	LPRINTF("Start echo flood testing....\n");
	LPRINTF("It sends msgs to the remote.\n");
	LPRINTF("And then it waits for msgs to echo back and verifiy.\n");
	/* Clear shared memory descriptors of both directions */
	shm0_mg->avails = 0;
	shm0_mg->used = 0;
	shm1_mg->avails = 0;
	shm1_mg->used = 0;
	for (i = 0; i < PKGS_TOTAL; i++) {
		tstart = get_timestamp();
		/* Construct a message to send */
		msg_hdr = (struct msg_hdr_s *)d0;
		msg_hdr->index = i;
		msg_hdr->len = sizeof(tstart);
		d0 += (sizeof(struct msg_hdr_s));
		metal_memcpy_io(d0, (void *)&tstart, msg_hdr->len);
		/* Update the shared memory management information
		 * Tell the other end where the d0 buffer is.
		 */
		shm0_addr_array[i] = (shm_addr_t)metal_io_virt_to_phys(
				ch->shm_io, msg_hdr);
		d0 += msg_hdr->len;
		shm0_mg->avails++;

		/* memory barrier */
		atomic_thread_fence(memory_order_acq_rel);

		/* Send the message */
		metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);
	}
	i = 0;
	d0 = metal_io_virt(ch->shm_io, ch->d0_start_offset);
	while (shm1_mg->used != PKGS_TOTAL) {
		do {
			if (!atomic_flag_test_and_set(&ch->notified)) {
				metal_cpu_yield();
				break;
			}
		} while(1);
		atomic_thread_fence(memory_order_acq_rel);
		while ((shm1_mg->used != PKGS_TOTAL) &&
			(shm1_mg->used != shm1_mg->avails)) {
			/* Received pong from the other side */

			/* Get the d1 buffer location from the shared memory
			 * management.
			 */
			d1_pa = (metal_phys_addr_t)shm1_addr_array[shm1_mg->used];
			d1 = metal_io_phys_to_virt(ch->shm_io, d1_pa);
			if (!d1) {
				LPRINTF("ERROR: failed to get rx address: 0x%lx.\n",
					d1_pa);
				return NULL;
			}
			msg_hdr_echo = (struct msg_hdr_s *)d1;

			/* Verify the message */
			if (msg_hdr_echo->index != (uint32_t)i) {
				LPRINTF("ERROR: wrong msg: expected: %d, actual: %d\n",
					i, msg_hdr_echo->index);
				return NULL;
			}
			d1 += sizeof(struct msg_hdr_s);
			d0 += sizeof(struct msg_hdr_s);
			if (*(unsigned long long *)d0 !=
				*(unsigned long long *)d1) {
				LPRINTF("ERROR: wrong message, [%d], %llu:%llu\n",
					i, *(unsigned long long *)d0,
					*(unsigned long long *)d1);
				return NULL;
			}
			d0 += msg_hdr_echo->len;
			shm1_mg->used++;
			i++;
		}
	}
	tend = get_timestamp();
	tdiff = tend - tstart;

	/* Send shutdown message */
	msg_hdr = (struct msg_hdr_s *)d0;
	msg_hdr->index = i;
	d0 += sizeof(struct msg_hdr_s);
	sprintf(d0, SHUTDOWN);
	msg_hdr->len = sizeof(d0);
	shm0_addr_array[i] = (uint64_t)metal_io_virt_to_phys(
				ch->shm_io, msg_hdr);
	shm0_mg->avails++;
	atomic_thread_fence(memory_order_acq_rel);
	LPRINTF("Sending shutdown message...\n");
	metal_io_write32(ch->ipi_io, IPI_TRIG_OFFSET, ch->ipi_mask);

	tdiff /= i;
	tdiff_avg_s = tdiff / NS_PER_S;
	tdiff_avg_ns = tdiff % NS_PER_S;
	LPRINTF("Total packages: %d, time_avg = %lds, %ldns\n",
		i, (long int)tdiff_avg_s, (long int)tdiff_avg_ns);

	return NULL;
}

/**
 * @brief    cleanup - cleanup the application
 *           The cleanup funciton will disable the IPI interrupt
 *           close the metal devices and finish the libmetal environment.
 */
void cleanup(void)
{
	int irq;
	/* Disable IPI interrupt */
	if (ch0.ipi_io) {
		metal_io_write32(ch0.ipi_io, IPI_IDR_OFFSET, ch0.ipi_mask);
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
	metal_finish();
}

/**
 * @brief    main function of the demo application.
 *           Here are the steps for the main function:
 *           * call system_init() function for system related initialization.
 *           * call metal_init() function to initialize libmetal environment.
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
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;

	if (system_init()) {
		LPRINTF("ERROR: Failed to initialize system\n");
		return -1;
	}
	if (metal_init(&init_param)) {
		LPRINTF("ERROR: Failed to run metal initialization\n");
		return -1;
	}
	memset(&ch0, 0, sizeof(ch0));
	atomic_store(&ch0.notified, 1);

	/* Open IPI device */
	ret = metal_device_open(BUS_NAME, IPI_DEV_NAME, &device);
	if (ret) {
		LPRINTF("ERROR: Failed to open device ff340000.ipi.\n");
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
		LPRINTF("ERROR: Failed to open device %s.\n", SHM0_DESC_DEV_NAME);
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
		LPRINTF("ERROR: Failed to memory map shmem descriptor.\n");
		metal_device_close(device);
		ret = -ENODEV;
		goto out;
	}

	/* Store the shared memory device and I/O region */
	ch0.shm0_desc_dev = device;
	ch0.shm0_desc_io = io;

	/* Open shared memory1 descriptor device */
	ret = metal_device_open(BUS_NAME, SHM1_DESC_DEV_NAME, &device);
	if (ret) {
		LPRINTF("ERROR: Failed to open device %s.\n", SHM1_DESC_DEV_NAME);
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
	/* The metal_io_mem_map() does nothing for Linux userspace, as the memory
	 * mapping is done by the kernel. It is put here for the code can be portable
	 * across different system.
	 */
	if (!metal_io_mem_map(metal_io_phys(io, 0), io, io->size)) {
		LPRINTF("ERROR: Failed to memory map shmem descriptor.\n");
		metal_device_close(device);
		ret = -ENODEV;
		goto out;
	}

	/* Store the shared memory device and I/O region */
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
	ch0.d0_start_offset = D0_SHM_OFFSET;

	/* Get interrupt ID from IPI metal device */
	irq = (intptr_t)ch0.ipi_dev->irq_info;
	if (irq < 0) {
		LPRINTF("ERROR: Failed to request interrupt for %s.\n",
			  device->name);
		ret = -EINVAL;
		goto out;
	}

	ch0.ipi_mask = IPI_MASK;
	ret =  metal_irq_register(irq, ipi_irq_isr, ch0.ipi_dev, &ch0);
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

	ret = run_comm_task(ipi_task_shm_atomic, &ch0);
	if (ret) {
		LPRINTF("ERROR: Failed to run shared memory atomic task.\n");
		goto out;
	}

	ret = run_comm_task(ipi_task_echo, &ch0);
	if (ret)
		LPRINTF("ERROR: Failed to run IPI communication task.\n");

out:
	cleanup();

	return ret;
}
