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
 * @file	generic/shmem.c
 * @brief	Generic libmetal shared memory handling.
 */

#include <errno.h>
#include <metal/shmem.h>
#include <metal/sys.h>
#include <metal/utilities.h>

int metal_shmem_register_generic(struct metal_generic_shmem *shmem)
{
	/* Make sure that we can be found. */
	assert(shmem->name && strlen(shmem->name) != 0);

	/* Statically registered shmem regions cannot have a destructor. */
	assert(!shmem->io.ops.close);

	metal_list_add_tail(&_metal.common.generic_shmem_list,
			    &shmem->node);
	return 0;
}

int metal_shmem_open_generic(const char *name, size_t size,
			     struct metal_io_region **result)
{
	struct metal_generic_shmem *shmem;
	struct metal_list *node;

	metal_list_for_each(&_metal.common.generic_shmem_list, node) {
		shmem = metal_container_of(node, struct metal_generic_shmem, node);
		if (strcmp(shmem->name, name) != 0)
			continue;
		if (size > metal_io_region_size(&shmem->io))
			continue;
		*result = &shmem->io;
		return 0;
	}

	return -ENOENT;
}
