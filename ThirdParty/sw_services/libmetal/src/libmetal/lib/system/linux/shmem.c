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

/*
 * @file	linux/shmem.c
 * @brief	Linux libmetal shared memory handling.
 */

#include <metal/shmem.h>
#include <metal/sys.h>
#include <metal/utilities.h>

struct metal_shmem {
	struct metal_io_region	io;
	metal_phys_addr_t	*phys;
};

static void metal_shmem_io_close(struct metal_io_region *io)
{
	metal_unmap(io->virt, io->size);
	free((void *)io->physmap);
}

static const struct metal_io_ops metal_shmem_io_ops = {
	NULL, NULL, NULL, NULL, NULL, metal_shmem_io_close
};

static int metal_shmem_try_map(struct metal_page_size *ps, int fd, size_t size,
			       struct metal_io_region **result)
{
	size_t pages, page, phys_size;
	struct metal_io_region *io;
	metal_phys_addr_t *phys;
	uint8_t *virt;
	void *mem;
	int error;

	size = metal_align_up(size, ps->page_size);
	pages = size / ps->page_size;

	error = metal_map(fd, 0, size, 1, ps->mmap_flags, &mem);
	if (error) {
		metal_log(METAL_LOG_ERROR, "failed to mmap shmem - %s\n",
			  strerror(-error));
		return error;
	}

	error = metal_mlock(mem, size);
	if (error) {
		metal_log(METAL_LOG_WARNING, "failed to mlock shmem - %s\n",
			  strerror(-error));
	}

	phys_size = sizeof(*phys) * pages;
	phys = malloc(phys_size);
	if (!phys) {
		metal_unmap(mem, size);
		return -ENOMEM;
	}

	io = malloc(sizeof(*io));
	if (!io) {
		free(phys);
		metal_unmap(mem, size);
		return -ENOMEM;
	}

	if (_metal.pagemap_fd < 0) {
		phys[0] = 0;
		metal_log(METAL_LOG_WARNING,
		"shmem - failed to get va2pa mapping. use offset as pa.\n");
		metal_io_init(io, mem, phys, size, -1, 0, &metal_shmem_io_ops);
	} else {
		for (virt = mem, page = 0; page < pages; page++) {
			size_t offset = page * ps->page_size;
			error = metal_virt2phys(virt + offset, &phys[page]);
			if (error < 0)
				phys[page] = METAL_BAD_OFFSET;
		}
		metal_io_init(io, mem, phys, size, ps->page_shift, 0,
			&metal_shmem_io_ops);
	}
	*result = io;

	return 0;
}

int metal_shmem_open(const char *name, size_t size,
		     struct metal_io_region **result)
{
	struct metal_page_size *ps;
	int fd, error;

	error = metal_shmem_open_generic(name, size, result);
	if (!error)
		return error;

	error = metal_open(name, 1);
	if (error < 0) {
		metal_log(METAL_LOG_ERROR, "Failed to open shmem file :%s\n", name);
		return error;
	}
	fd = error;

	/* Iterate through page sizes in decreasing order. */
	metal_for_each_page_size_down(ps) {
		if (ps->page_size > 2 * size)
			continue;
		error = metal_shmem_try_map(ps, fd, size, result);
		if (!error)
			break;
	}

	close(fd);
	return error;
}
