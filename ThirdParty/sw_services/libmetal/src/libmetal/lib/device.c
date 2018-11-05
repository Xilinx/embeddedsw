/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
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

#include <string.h>
#include <errno.h>
#include <metal/device.h>
#include <metal/list.h>
#include <metal/log.h>
#include <metal/sys.h>
#include <metal/utilities.h>
#include <metal/dma.h>
#include <metal/cache.h>

int metal_bus_register(struct metal_bus *bus)
{
	if (!bus || !bus->name || !strlen(bus->name))
		return -EINVAL;
	if (metal_bus_find(bus->name, NULL) == 0)
		return -EEXIST;
	metal_list_init(&bus->devices);
	metal_list_add_tail(&_metal.common.bus_list, &bus->node);
	metal_log(METAL_LOG_DEBUG, "registered %s bus\n", bus->name);
	return 0;
}

int metal_bus_unregister(struct metal_bus *bus)
{
	struct metal_list *node;
	struct metal_device *device;

	while ((node = metal_list_first(&bus->devices)) != NULL) {
		device = metal_container_of(node, struct metal_device, node);
		metal_device_close(device);
	}

	metal_list_del(&bus->node);
	if (bus->ops.bus_close)
		bus->ops.bus_close(bus);
	metal_log(METAL_LOG_DEBUG, "unregistered %s bus\n", bus->name);
	return 0;
}

int metal_bus_find(const char *name, struct metal_bus **result)
{
	struct metal_list *node;
	struct metal_bus *bus;

	metal_list_for_each(&_metal.common.bus_list, node) {
		bus = metal_container_of(node, struct metal_bus, node);
		if (strcmp(bus->name, name) != 0)
			continue;
		if (result)
			*result = bus;
		return 0;
	}
	return -ENOENT;
}

int metal_device_open(const char *bus_name, const char *dev_name,
		      struct metal_device **device)
{
	struct metal_bus *bus;
	int error;

	if (!bus_name || !strlen(bus_name) ||
	    !dev_name || !strlen(dev_name) ||
	    !device)
		return -EINVAL;

	error = metal_bus_find(bus_name, &bus);
	if (error)
		return error;

	if (!bus->ops.dev_open)
		return -ENODEV;

	error = (*bus->ops.dev_open)(bus, dev_name, device);
	if (error)
		return error;

	if (bus != &metal_generic_bus)
		metal_list_add_tail(&bus->devices, &(*device)->node);

	return 0;
}

void metal_device_close(struct metal_device *device)
{
	assert(device && device->bus);
	if (device->bus != &metal_generic_bus)
		metal_list_del(&device->node);
	if (device->bus->ops.dev_close)
		device->bus->ops.dev_close(device->bus, device);
}

int metal_register_generic_device(struct metal_device *device)
{
	if (!device->name || !strlen(device->name) ||
	    device->num_regions > METAL_MAX_DEVICE_REGIONS)
		return -EINVAL;

	device->bus = &metal_generic_bus;
	metal_list_add_tail(&_metal.common.generic_device_list,
			    &device->node);
	return 0;
}

static int metal_generic_dev_open(struct metal_bus *bus, const char *dev_name,
				  struct metal_device **device)
{
	struct metal_list *node;
	struct metal_device *dev;

	(void)bus;

	metal_list_for_each(&_metal.common.generic_device_list, node) {
		dev = metal_container_of(node, struct metal_device, node);
		if (strcmp(dev->name, dev_name) != 0)
			continue;
		*device = dev;
		return metal_generic_dev_sys_open(dev);
	}

	return -ENODEV;
}

static int metal_generic_dev_dma_map(struct metal_bus *bus,
			     struct metal_device *device,
			     uint32_t dir,
			     struct metal_sg *sg_in,
			     int nents_in,
			     struct metal_sg *sg_out)
{
	(void)bus;
	(void)device;
	int i;

	if (sg_out != sg_in)
		memcpy(sg_out, sg_in, nents_in*(sizeof(struct metal_sg)));
	for (i = 0; i < nents_in; i++) {
		if (dir == METAL_DMA_DEV_W) {
			metal_cache_flush(sg_out[i].virt, sg_out[i].len);
		}
		metal_cache_invalidate(sg_out[i].virt, sg_out[i].len);
	}

	return nents_in;
}

void metal_generic_dev_dma_unmap(struct metal_bus *bus,
		  struct metal_device *device,
		  uint32_t dir,
		  struct metal_sg *sg,
		  int nents)
{
	(void)bus;
	(void)device;
	(void)dir;
	int i;

	for (i = 0; i < nents; i++) {
		metal_cache_invalidate(sg[i].virt, sg[i].len);
	}
}

struct metal_bus metal_generic_bus = {
	.name = "generic",
	.ops  = {
		.bus_close = NULL,
		.dev_open  = metal_generic_dev_open,
		.dev_close = NULL,
		.dev_irq_ack = NULL,
		.dev_dma_map = metal_generic_dev_dma_map,
		.dev_dma_unmap = metal_generic_dev_dma_unmap,
	},
};
