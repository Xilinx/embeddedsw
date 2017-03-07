/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of Mentor Graphics Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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

/**************************************************************************
 * FILE NAME
 *
 *       hil.c
 *
 * COMPONENT
 *
 *         OpenAMP  Stack.
 *
 * DESCRIPTION
 *
 *       This file is implementation of generic part of HIL.
 *
 *
 *
 **************************************************************************/

#include "openamp/hil.h"
#include "openamp/remoteproc.h"
#include <metal/io.h>
#include <metal/alloc.h>
#include <metal/device.h>
#include <metal/shmem.h>
#include <metal/utilities.h>
#include <metal/time.h>

#define DEFAULT_VRING_MEM_SIZE 0x10000

/*--------------------------- Globals ---------------------------------- */
static METAL_DECLARE_LIST (procs);

#if defined (OPENAMP_BENCHMARK_ENABLE)

unsigned long long boot_time_stamp;
unsigned long long shutdown_time_stamp;

#endif

metal_phys_addr_t hil_generic_start_paddr = 0;
struct metal_io_region hil_shm_generic_io = {
	0,
	&hil_generic_start_paddr,
	(size_t)(-1),
	(sizeof(metal_phys_addr_t) << 3),
	(metal_phys_addr_t)(-1),
	0,
	{NULL},
};

struct metal_io_region hil_devmem_generic_io = {
	0,
	&hil_generic_start_paddr,
	(size_t)(-1),
	(sizeof(metal_phys_addr_t) << 3),
	(metal_phys_addr_t)(-1),
	METAL_UNCACHED | METAL_SHARED_MEM,
	{NULL},
};

struct hil_proc *hil_create_proc(struct hil_platform_ops *ops,
			unsigned long cpu_id, void *pdata)
{
	struct hil_proc *proc = 0;
	int i;

	proc = metal_allocate_memory(sizeof(struct hil_proc));
	if (!proc)
		return NULL;
	memset(proc, 0, sizeof(struct hil_proc));

	proc->ops = ops;
	proc->num_chnls = 1;
	proc->cpu_id = cpu_id;
	proc->pdata = pdata;

	/* Setup generic shared memory I/O region */
	proc->sh_buff.io = &hil_shm_generic_io;
	/* Setup generic vdev I/O region */
	proc->vdev.io = &hil_devmem_generic_io;
	/* Setup generic vrings I/O region */
	for (i = 0; i < HIL_MAX_NUM_VRINGS; i++)
		proc->vdev.vring_info[i].io = &hil_devmem_generic_io;

	metal_mutex_init(&proc->lock);
	metal_list_add_tail(&procs, &proc->node);

	return proc;
}

/**
 * hil_delete_proc
 *
 * This function deletes the given proc instance and frees the
 * associated resources.
 *
 * @param proc - pointer to hil remote_proc instance
 *
 */
void hil_delete_proc(struct hil_proc *proc)
{
	struct metal_list *node;
	struct metal_device *dev;
	struct metal_io_region *io;
	struct proc_vring *vring;
	int i;
	metal_list_for_each(&procs, node) {
		if (proc ==
			metal_container_of(node, struct hil_proc, node)) {
			metal_list_del(&proc->node);
			metal_mutex_acquire(&proc->lock);
			proc->ops->release(proc);
			/* Close shmem device */
			dev = proc->sh_buff.dev;
			io = proc->sh_buff.io;
			if (dev) {
				metal_device_close(dev);
			} else if (io && io->ops.close) {
				io->ops.close(io);
			}

			/* Close Vdev device */
			dev = proc->vdev.dev;
			io = proc->vdev.io;
			if (dev) {
				metal_device_close(dev);
			} else if (io && io->ops.close) {
				io->ops.close(io);
			}

			/* Close vring device */
			for (i = 0; i < HIL_MAX_NUM_VRINGS; i++) {
				vring = &proc->vdev.vring_info[i];
				dev = vring->dev;
				io = vring->io;
				if (dev) {
					metal_device_close(dev);
				} if (io && io->ops.close) {
					io->ops.close(io);
				}
			}

			metal_mutex_release(&proc->lock);
			metal_mutex_deinit(&proc->lock);
			metal_free_memory(proc);
			return;
		}
	}
}

int hil_init_proc(struct hil_proc *proc)
{
	int ret = 0;
	if (!proc->is_initialized && proc->ops->initialize) {
		ret = proc->ops->initialize(proc);
		if (!ret)
			proc->is_initialized = 1;
		else
			return -1;
	}
	return 0;
}

/**
 * hil_get_chnl_info
 *
 * This function returns channels info for given proc.
 *
 * @param proc - pointer to proc info struct
 * @param num_chnls - pointer to integer variable to hold
 *                    number of available channels
 *
 * @return - pointer to channel info control block
 *
 */
struct proc_chnl *hil_get_chnl_info(struct hil_proc *proc, int *num_chnls)
{
	*num_chnls = proc->num_chnls;
	return (proc->chnls);
}

void hil_notified(struct hil_proc *proc, uint32_t notifyid)
{
	struct proc_vdev *pvdev = &proc->vdev;
	struct fw_rsc_vdev *vdev_rsc = pvdev->vdev_info;
	int i;
	if (vdev_rsc->status & VIRTIO_CONFIG_STATUS_NEEDS_RESET) {
		if (pvdev->rst_cb)
			pvdev->rst_cb(proc, 0);
	} else {
		for(i = 0; i < (int)pvdev->num_vrings; i++) {
			struct fw_rsc_vdev_vring *vring_rsc;
			vring_rsc = &vdev_rsc->vring[i];
			if (notifyid == (uint32_t)(-1) ||
				notifyid == vring_rsc->notifyid)
				virtqueue_notification(
					pvdev->vring_info[i].vq);
		}
	}
}

/**
 * hil_get_vdev_info
 *
 * This function return virtio device for remote core.
 *
 * @param proc - pointer to remote proc
 *
 * @return - pointer to virtio HW device.
 *
 */

struct proc_vdev *hil_get_vdev_info(struct hil_proc *proc)
{
	return (&proc->vdev);

}

/**
 * hil_get_vring_info
 *
 * This function returns vring_info_table. The caller will use
 * this table to get the vring HW info which will be subsequently
 * used to create virtqueues.
 *
 * @param vdev - pointer to virtio HW device
 * @param num_vrings - pointer to hold number of vrings
 *
 * @return - pointer to vring hardware info table
 */
struct proc_vring *hil_get_vring_info(struct proc_vdev *vdev, int *num_vrings)
{
	struct fw_rsc_vdev *vdev_rsc;
	struct fw_rsc_vdev_vring *vring_rsc;
	struct proc_vring *vring;
	int i;

	vdev_rsc = vdev->vdev_info;
	if (vdev_rsc) {
		vring = &vdev->vring_info[0];
		for (i = 0; i < vdev_rsc->num_of_vrings; i++) {
			/* Initialize vring with vring resource */
			vring_rsc = &vdev_rsc->vring[i];
			vring[i].num_descs = vring_rsc->num;
			vring[i].align = vring_rsc->align;
			/* Enable acccess to vring memory region */
			vring[i].vaddr =
				metal_io_mem_map(
					(metal_phys_addr_t)vring_rsc->da,
					vring[i].io,
					vring_size(vring_rsc->num,
					vring_rsc->align));
		}
	}
	*num_vrings = vdev->num_vrings;
	return (vdev->vring_info);

}

/**
 * hil_get_shm_info
 *
 * This function returns shared memory info control block. The caller
 * will use this information to create and manage memory buffers for
 * vring descriptor table.
 *
 * @param proc - pointer to proc instance
 *
 * @return - pointer to shared memory region used for buffers
 *
 */
struct proc_shm *hil_get_shm_info(struct hil_proc *proc)
{
	return (&proc->sh_buff);
}

void hil_free_vqs(struct virtio_device *vdev)
{
	struct hil_proc *proc = vdev->device;
	struct proc_vdev *pvdev = &proc->vdev;
	int num_vrings = (int)pvdev->num_vrings;
	int i;

	metal_mutex_acquire(&proc->lock);
	for(i = 0; i < num_vrings; i++) {
		struct proc_vring *pvring = &pvdev->vring_info[i];
		struct virtqueue *vq = pvring->vq;
		if (vq) {
			virtqueue_free(vq);
			pvring->vq = 0;
		}
	}
	metal_mutex_release(&proc->lock);
}

int hil_enable_vdev_notification(struct hil_proc *proc, int id)
{
	/* We only support single vdev in hil_proc */
	(void)id;
	if (!proc)
		return -1;
	if (proc->ops->enable_interrupt)
		proc->ops->enable_interrupt(&proc->vdev.intr_info);
	return 0;
}

/**
 * hil_enable_vring_notifications()
 *
 * This function is called after successful creation of virtqueues.
 * This function saves queue handle in the vring_info_table which
 * will be used during interrupt handling .This function setups
 * interrupt handlers.
 *
 * @param vring_index - index to vring HW table
 * @param vq          - pointer to virtqueue to save in vring HW table
 *
 * @return            - execution status
 */
int hil_enable_vring_notifications(int vring_index, struct virtqueue *vq)
{
	struct hil_proc *proc_hw = (struct hil_proc *)vq->vq_dev->device;
	struct proc_vring *vring_hw = &proc_hw->vdev.vring_info[vring_index];
	/* Save virtqueue pointer for later reference */
	vring_hw->vq = vq;

	if (proc_hw->ops->enable_interrupt) {
		proc_hw->ops->enable_interrupt(&vring_hw->intr_info);
	}

	return 0;
}

/**
 * hil_vdev_notify()
 *
 * This function generates IPI to let the other side know that there is
 * update in the vritio dev configs
 *
 * @param vdev - pointer to the viritio device
 *
 */
void hil_vdev_notify(struct virtio_device *vdev)
{
	struct hil_proc *proc = vdev->device;
	struct proc_vdev *pvdev = &proc->vdev;

	if (proc->ops->notify) {
		proc->ops->notify(proc, &pvdev->intr_info);
	}
}

/**
 * hil_vring_notify()
 *
 * This function generates IPI to let the other side know that there is
 * job available for it. The required information to achieve this, like interrupt
 * vector, CPU id etc is be obtained from the proc_vring table.
 *
 * @param vq - pointer to virtqueue
 *
 */
void hil_vring_notify(struct virtqueue *vq)
{
	struct hil_proc *proc_hw = (struct hil_proc *)vq->vq_dev->device;
	struct proc_vring *vring_hw =
	    &proc_hw->vdev.vring_info[vq->vq_queue_index];

	if (proc_hw->ops->notify) {
		proc_hw->ops->notify(proc_hw, &vring_hw->intr_info);
	}
}

/**
 * hil_get_status
 *
 * This function is used to check if the given core is up and running.
 * This call will return after it is confirmed that remote core has
 * started.
 *
 * @param proc - pointer to proc instance
 *
 * @return - execution status
 */
int hil_get_status(struct hil_proc *proc)
{
	(void)proc;

	/* For future use only. */
	return 0;
}

/**
 * hil_set_status
 *
 * This function is used to update the status
 * of the given core i.e it is ready for IPC.
 *
 * @param proc - pointer to remote proc
 *
 * @return - execution status
 */
int hil_set_status(struct hil_proc *proc)
{
	(void)proc;

	/* For future use only. */
	return 0;
}

/**
 * hil_boot_cpu
 *
 * This function boots the remote processor.
 *
 * @param proc       - pointer to remote proc
 * @param start_addr - start address of remote cpu
 *
 * @return - execution status
 */
int hil_boot_cpu(struct hil_proc *proc, unsigned int start_addr)
{

	if (proc->ops->boot_cpu) {
		proc->ops->boot_cpu(proc, start_addr);
	}
#if defined (OPENAMP_BENCHMARK_ENABLE)
	boot_time_stamp = metal_get_timestamp();
#endif

	return 0;
}

/**
 * hil_shutdown_cpu
 *
 *  This function shutdowns the remote processor
 *
 * @param proc - pointer to remote proc
 *
 */
void hil_shutdown_cpu(struct hil_proc *proc)
{
	if (proc->ops->shutdown_cpu) {
		proc->ops->shutdown_cpu(proc);
	}
#if defined (OPENAMP_BENCHMARK_ENABLE)
	shutdown_time_stamp = metal_get_timestamp();
#endif
}

/**
 * hil_get_firmware
 *
 * This function returns address and size of given firmware name passed as
 * parameter.
 *
 * @param fw_name    - name of the firmware
 * @param start_addr - pointer t hold start address of firmware
 * @param size       - pointer to hold size of firmware
 *
 * returns -  status of function execution
 *
 */
int hil_get_firmware(char *fw_name, uintptr_t *start_addr,
		     unsigned int *size)
{
	return (config_get_firmware(fw_name, start_addr, size));
}

int hil_poll (struct hil_proc *proc, int nonblock)
{
	return proc->ops->poll(proc, nonblock);
}

int hil_set_shm (struct hil_proc *proc,
		 const char *bus_name, const char *name,
		 metal_phys_addr_t paddr, size_t size)
{
	struct metal_device *dev;
	struct metal_io_region *io;
	int ret;
	if (!proc)
		return -1;
	if (name && bus_name) {
		ret = metal_device_open(bus_name, name, &dev);
		if (ret)
			return ret;
		io = metal_device_io_region(dev, 0);
		if (!io)
			return -1;
		proc->sh_buff.io = io;
		proc->sh_buff.dev = dev;
	} else if (name) {
		ret = metal_shmem_open(name, size, &io);
		if (ret)
			return ret;
		proc->sh_buff.io = io;
	}
	if (!paddr && io) {
		proc->sh_buff.start_paddr = io->physmap[0];
		proc->sh_buff.start_addr = io->virt;
	} else {
		proc->sh_buff.start_paddr = paddr;
	}
	if (!size && io)
		proc->sh_buff.size = io->size;
	else
		proc->sh_buff.size = size;

	metal_io_mem_map(proc->sh_buff.start_paddr, proc->sh_buff.io,
			 proc->sh_buff.size);
	return 0;
}

int hil_set_vdev (struct hil_proc *proc,
		const char *bus_name, const char *name)
{
	struct metal_device *dev;
	struct metal_io_region *io;
	int ret;

	if (!proc)
		return -1;

	if (name && bus_name) {
		ret = metal_device_open(bus_name, name, &dev);
		if (ret)
			return ret;
		io = metal_device_io_region(dev, 0);
		if (!io)
			return -1;
		proc->vdev.io = io;
		proc->vdev.dev = dev;
	} else if (name) {
		ret = metal_shmem_open(name, DEFAULT_VRING_MEM_SIZE, &io);
		if (ret)
			return ret;
		proc->vdev.io = io;
	} else {
		proc->vdev.io = NULL;
	}

	return 0;
}

int hil_set_vring (struct hil_proc *proc, int index,
		 const char *bus_name, const char *name)
{
	struct metal_device *dev;
	struct metal_io_region *io;
	struct proc_vring *vring;
	int ret;

	if (!proc)
		return -1;
	if (index >= HIL_MAX_NUM_VRINGS)
		return -1;
	vring = &proc->vdev.vring_info[index];
	if (name && bus_name) {
		ret = metal_device_open(bus_name, name, &dev);
		if (ret)
			return ret;
		io = metal_device_io_region(dev, 0);
		if (!io)
			return -1;
		vring->io = io;
		vring->dev = dev;
	} else if (name) {
		ret = metal_shmem_open(name, DEFAULT_VRING_MEM_SIZE, &io);
		if (ret)
			return ret;
		vring->io = io;
	}

	return 0;
}

int hil_set_vdev_ipi (struct hil_proc *proc, int index,
		 unsigned int irq, void *data)
{
	struct proc_intr *vring_intr;

	/* As we support only one vdev for now */
	(void)index;

	if (!proc)
		return -1;
	vring_intr = &proc->vdev.intr_info;
	vring_intr->vect_id = irq;
	vring_intr->data = data;
	return 0;
}

int hil_set_vring_ipi (struct hil_proc *proc, int index,
		 unsigned int irq, void *data)
{
	struct proc_intr *vring_intr;

	if (!proc)
		return -1;
	vring_intr = &proc->vdev.vring_info[index].intr_info;
	vring_intr->vect_id = irq;
	vring_intr->data = data;
	return 0;
}

int hil_set_rpmsg_channel (struct hil_proc *proc, int index,
			   char *name)
{
	if (!proc)
		return -1;
	if (index >= HIL_MAX_NUM_CHANNELS)
		return -1;
	strcpy(proc->chnls[index].name, name);
	return 0;
}

int hil_set_vdev_rst_cb (struct hil_proc *proc, int index,
		hil_proc_vdev_rst_cb_t cb)
{
	(void)index;
	proc->vdev.rst_cb = cb;
	return 0;
}
