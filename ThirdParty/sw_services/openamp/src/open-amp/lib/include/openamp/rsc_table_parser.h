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

#ifndef RSC_TABLE_PARSER_H
#define RSC_TABLE_PARSER_H

#include "openamp/remoteproc.h"
#include "openamp/env.h"
#include "openamp/hil.h"

#define RSC_TAB_SUPPORTED_VERSION           1
#define RSC_TAB_HEADER_SIZE                 12
#define RSC_TAB_MAX_VRINGS                  2

/* Standard control request handling. */
typedef int (*rsc_handler) (struct remote_proc * rproc, void *rsc);

/* Function prototypes */
int handle_rsc_table(struct remote_proc *rproc,
		     struct resource_table *rsc_table, int len);
int handle_carve_out_rsc(struct remote_proc *rproc, void *rsc);
int handle_trace_rsc(struct remote_proc *rproc, void *rsc);
int handle_dev_mem_rsc(struct remote_proc *rproc, void *rsc);
int handle_vdev_rsc(struct remote_proc *rproc, void *rsc);
int handle_rproc_mem_rsc(struct remote_proc *rproc, void *rsc);
int handle_fw_chksum_rsc(struct remote_proc *rproc, void *rsc);
int handle_mmu_rsc(struct remote_proc *rproc, void *rsc);

#endif				/* RSC_TABLE_PARSER_H */
