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

#include <string.h>
#include "metal/alloc.h"
#include "openamp/remoteproc_loader.h"

/**
 * remoteproc_loader_init
 *
 * Initializes the remoteproc loader.
 *
 * @param type - loader type
 *
 * @return  - remoteproc_loader
 */
struct remoteproc_loader *remoteproc_loader_init(enum loader_type type)
{

	struct remoteproc_loader *loader;

	/* Check for valid loader type. */
	if (type >= LAST_LOADER) {
		return RPROC_NULL;
	}

	/* Allocate a loader handle. */
	loader = metal_allocate_memory(sizeof(struct remoteproc_loader));

	if (!loader) {
		return RPROC_NULL;
	}

	/* Clear loader handle. */
	memset(loader, 0, sizeof(struct remoteproc_loader));

	/* Save loader type. */
	loader->type = type;

	switch (type) {

	case ELF_LOADER:
		elf_loader_init(loader);
		break;

	default:
		/* Loader not supported. */
		metal_free_memory(loader);
		loader = RPROC_NULL;
		break;
	}

	return loader;
}

/**
 * remoteproc_loader_delete
 *
 * Deletes the remoteproc loader.
 *
 * @param loader    - pointer to remoteproc loader
 *
 * @return  - 0 if success, error otherwise
 */
int remoteproc_loader_delete(struct remoteproc_loader *loader)
{

	int status = 0;

	if (!loader) {
		return RPROC_ERR_PARAM;
	}

	/* Check if a firmware is attached. */
	if (loader->remote_firmware) {

		/* Detach firmware first. */
		status = loader->detach_firmware(loader);
	}

	/* Recover the allocated memory. */
	metal_free_memory(loader);

	return status;
}

/**
 * remoteproc_loader_attach_firmware
 *
 * Attaches an ELF firmware to the loader
 *
 * @param loader    - pointer to remoteproc loader
 * @param firmware  - pointer to the firmware start location
 *
 * @return  - 0 if success, error otherwise
 */
int remoteproc_loader_attach_firmware(struct remoteproc_loader *loader,
				      void *firmware_image)
{

	int status = RPROC_SUCCESS;

	if (!loader || !firmware_image) {
		return RPROC_ERR_PARAM;
	}

	if (loader->attach_firmware) {

		/* Check if a firmware is already attached. */
		if (loader->remote_firmware) {

			/* Detach firmware first. */
			status = loader->detach_firmware(loader);
		}

		/* Attach firmware. */
		if (!status) {
			status =
			    loader->attach_firmware(loader, firmware_image);

			/* Save firmware address. */
			if (!status) {
				loader->remote_firmware = firmware_image;
			}
		}
	} else {
		status = RPROC_ERR_LOADER;
	}

	return status;
}

/**
 * remoteproc_loader_retrieve_entry_point
 *
 * Provides entry point address.
 *
 * @param loader - pointer to remoteproc loader
 *
 * @return  - entrypoint
 */
void *remoteproc_loader_retrieve_entry_point(struct remoteproc_loader *loader)
{

	if (!loader) {
		return RPROC_NULL;
	}

	if (loader->retrieve_entry) {
		return loader->retrieve_entry(loader);
	} else {
		return RPROC_NULL;
	}
}

/**
 * remoteproc_loader_retrieve_resource_section
 *
 * Provides resource section address.
 *
 * @param loader - pointer to remoteproc loader
 * @param size   - pointer to hold size of resource section
 *
 * @return  - pointer to resource section
 */
void *remoteproc_loader_retrieve_resource_section(struct remoteproc_loader
						  *loader, unsigned int *size)
{

	if (!loader) {
		return RPROC_NULL;
	}

	if (loader->retrieve_rsc) {
		return loader->retrieve_rsc(loader, size);
	} else {
		return RPROC_NULL;
	}
}

/**
 * remoteproc_loader_load_remote_firmware
 *
 * Loads the firmware in memory
 *
 * @param loader - pointer to remoteproc loader
 *
 * @return  - 0 if success, error otherwise
 */
int remoteproc_loader_load_remote_firmware(struct remoteproc_loader *loader)
{

	if (!loader) {
		return RPROC_ERR_PARAM;
	}

	if (loader->load_firmware) {
		return loader->load_firmware(loader);
	} else {
		return RPROC_ERR_LOADER;
	}
}

/**
 * remoteproc_get_load_address
 *
 * Provides firmware load address.
 *
 * @param loader - pointer to remoteproc loader
 *
 * @return  - load address pointer
 */
void *remoteproc_get_load_address(struct remoteproc_loader *loader)
{

	if (!loader) {
		return RPROC_ERR_PTR;
	}

	if (loader->retrieve_load_addr) {
		return loader->retrieve_load_addr(loader);
	} else {
		return RPROC_ERR_PTR;
	}
}
