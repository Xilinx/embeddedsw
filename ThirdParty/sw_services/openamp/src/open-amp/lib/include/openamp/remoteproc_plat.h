#ifndef _REMOTEPROC_PLAT_H_
#define _REMOTEPROC_PLAT_H_

/*
 * Copyright (c) 2016 Xilinx, Inc.
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
 * 3. Neither the name of the <ORGANIZATION> nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
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
 *       remoteproc_platform.h
 *
 * DESCRIPTION
 *
 *       This file defines the platform data structure, user can use those
 *       structures to specify platform dependent data. Remoteproc drivers
 *       can use them to get the platform data.
 *
 ***************************************************************************/

#include "openamp/hil.h"
#include "metal/list.h"
#include "metal/io.h"


/** Platform data resource type */
enum plat_rsc_type {
	PLAT_RSC_VRING = 0,
	PLAT_RSC_SHM = 1,
	PLAT_RSC_RPMSG_CHANNEL = 2,
	PLAT_RSC_LAST = 3,
};

/** vring data structure */
struct plat_vring {
	unsigned int type;
	char *ipi_bus_name;
	char *ipi_dev_name;
	void *ipi_dev_data;
	char *vdev_bus_name;
	char *vdev_name;
};

/** shared memory structure */
struct plat_shm {
	unsigned int type;
	char *shm_name;
	size_t size;
};

/** RPMSG channel structure */
struct plat_rpmsg_chnl {
	unsigned int type;
	char *name;
};

/**
 * rproc_init_plat_data
 *
 * setup remoteproc common data based on the input platform data.
 *
 * @param[in] pdata	- input platform data
 * @param[in] proc      - hil proc to set with the platform data
 *
 * @returns - 0 for success, non-zero for errors.
 *
 */
int rproc_init_plat_data(void *pdata, struct hil_proc *proc);

/**
 * rproc_close_plat
 *
 * close remoteproc platform resource
 *
 * @param[in] proc      - hil proc which holds the platform data
 *
 */
void rproc_close_plat(struct hil_proc *proc);

#endif				/* _REMOTEPROC_PLAT_H_ */
