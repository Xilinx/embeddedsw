/*
 * Copyright (c) 2016 Xilinx, Inc. All rights reserved.
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

#include <string.h>
#include "openamp/hil.h"
#include "openamp/remoteproc_plat.h"
#include "metal/device.h"
#include "metal/io.h"
#include "metal/shmem.h"

/**
 * rproc_platform_open_metal_dev
 *
 * Open metal device and get the I/O region
 *
 * @param[in]  bus_name - metal device bus name
 * @param[in]  dev_name - metal device name
 * @param[out] dev - return metal device handle
 * @param[out] io - return metal device I/O region
 *
 * @return - 0 for success, non-zero for errors.
 */
static int rproc_plat_open_metal_dev(char *bus_name, char *dev_name,
				     struct metal_device **dev,
				     struct metal_io_region **io)
{
	int ret;
	ret = metal_device_open(bus_name, dev_name, dev);
	if (ret)
		return ret;
	*io = metal_device_io_region(*dev, 0);
	if (!(*io)) {
		metal_device_close(*dev);
		*dev = 0;
		return -1;
	}
	return 0;
}

int rproc_init_plat_data(void *pdata,
			     struct hil_proc *proc)
{
	unsigned int type;
	struct metal_device *dev;
	struct metal_io_region *io;
	int num_vrings = 0;
	int num_rpmsg_chnls = 0;
	int ret;

	if (!proc || !pdata)
		return -1;
	memcpy(proc, pdata, sizeof(struct proc_info_hdr));

	pdata += sizeof(struct proc_info_hdr);
	while(1) {
		type = *((unsigned int *)pdata);
		if (type > PLAT_RSC_LAST) {
			return -1;
		} else if (type == PLAT_RSC_LAST) {
			proc->vdev.num_vrings = num_vrings;
			proc->num_chnls = num_rpmsg_chnls;
			return 0;
		} else if (type == PLAT_RSC_VRING) {
			struct plat_vring *pvring =
				(struct plat_vring *)pdata;
			struct proc_vring *vring =
				&proc->vdev.vring_info[num_vrings];
			/* Open the vring IPI device */
			if (pvring->ipi_dev_name) {
				ret = rproc_plat_open_metal_dev(
						pvring->ipi_bus_name,
						pvring->ipi_dev_name,
						&dev, &io);
				if (ret)
					return ret;
				vring->intr_info.dev = dev;
				vring->intr_info.io = io;
				vring->intr_info.vect_id =
					(uintptr_t)dev->irq_info;
			}
			vring->intr_info.data = pvring->ipi_dev_data;
			/* Open the vring descriptor memory device */
			if (pvring->vdev_name) {
				ret = rproc_plat_open_metal_dev(
						pvring->vdev_bus_name,
						pvring->vdev_name,
						&dev, &io);
				if (ret)
					return ret;
				vring->dev = dev;
				vring->io = io;
			}
			num_vrings++;
			pdata += sizeof(struct plat_vring);
		} else if (type == PLAT_RSC_SHM) {
			struct plat_shm *pshm =
				(struct plat_shm *)pdata;
			/* Open the shared memory device */
			ret = metal_shmem_open(pshm->shm_name,
						pshm->size,
						&io);
			if (ret)
				return ret;
			proc->sh_buff.io = io;
			if (io->size != (size_t)(-1)) {
				metal_io_mem_map(
					metal_io_virt_to_phys(io, io->virt),
					io, io->size);
				proc->sh_buff.start_addr = io->virt;
				proc->sh_buff.size = io->size;
			}
			pdata += sizeof(struct plat_shm);
		} else if (type == PLAT_RSC_RPMSG_CHANNEL) {
			struct plat_rpmsg_chnl *pchl =
				(struct plat_rpmsg_chnl *)pdata;
			strcpy(proc->chnls[num_rpmsg_chnls].name, pchl->name);
			num_rpmsg_chnls++;
			pdata += sizeof(struct plat_rpmsg_chnl);
		}
	}

}

void rproc_close_plat(struct hil_proc *proc)
{
	if (!proc)
		return;
	int i;
	int num_vrings = proc->vdev.num_vrings;
	struct metal_device *dev;
	struct proc_vring *vring;

	/* Close the vrings devices */
	for (i = 0; i < num_vrings; i++) {
		vring = &proc->vdev.vring_info[i];
		dev = vring->dev;
		if (dev) {
			metal_device_close(dev);
			vring->dev = NULL;
		}
		dev = vring->intr_info.dev;
		if (dev) {
			metal_device_close(dev);
			vring->intr_info.dev = NULL;
		}
	}
}
