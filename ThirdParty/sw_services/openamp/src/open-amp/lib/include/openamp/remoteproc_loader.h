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
 *       remoteproc_loader.h
 *
 * COMPONENT
 *
 *         OpenAMP stack.
 *
 * DESCRIPTION
 *
 *       This file provides definitions for remoteproc loader
 *
 *
 **************************************************************************/
#ifndef REMOTEPROC_LOADER_H_
#define REMOTEPROC_LOADER_H_

#include "openamp/remoteproc.h"

/**
 * enum loader_type - dynamic name service announcement flags
 *
 * @ELF_LOADER: an ELF loader
 * @FIT_LOADER: a loader for Flattened Image Trees
 */
enum loader_type {
	ELF_LOADER = 0, FIT_LOADER = 1, LAST_LOADER = 2,
};

/* Loader structure definition. */

struct remoteproc_loader {
	enum loader_type type;
	void *remote_firmware;
	/* Pointer to firmware decoded info control block */
	void *fw_decode_info;

	/* Loader callbacks. */
	void *(*retrieve_entry) (struct remoteproc_loader * loader);
	void *(*retrieve_rsc) (struct remoteproc_loader * loader,
			       unsigned int *size);
	int (*load_firmware) (struct remoteproc_loader * loader);
	int (*attach_firmware) (struct remoteproc_loader * loader,
				void *firmware);
	int (*detach_firmware) (struct remoteproc_loader * loader);
	void *(*retrieve_load_addr) (struct remoteproc_loader * loader);

};

/* RemoteProc Loader functions. */
struct remoteproc_loader *remoteproc_loader_init(enum loader_type type);
int remoteproc_loader_delete(struct remoteproc_loader *loader);
int remoteproc_loader_attach_firmware(struct remoteproc_loader *loader,
				      void *firmware_image);
void *remoteproc_loader_retrieve_entry_point(struct remoteproc_loader *loader);
void *remoteproc_loader_retrieve_resource_section(struct remoteproc_loader
						  *loader, unsigned int *size);
int remoteproc_loader_load_remote_firmware(struct remoteproc_loader *loader);
void *remoteproc_get_load_address(struct remoteproc_loader *loader);

/* Supported loaders */
extern int elf_loader_init(struct remoteproc_loader *loader);

#endif				/* REMOTEPROC_LOADER_H_ */
