/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2017 Xilinx, Inc.
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
 *       platform_info.c
 *
 * DESCRIPTION
 *
 *       This file define platform specific data and implements APIs to set
 *       platform specific information for OpenAMP.
 *
 **************************************************************************/

#include "openamp/hil.h"
#include "metal/atomic.h"
#include "platform_info.h"

#define APU_CPU_ID     0 /* another APU core CPU index. We only talk
			  * to one CPU in the example. We set the CPU
			  * index to 0. */

/* processor operations for hil_proc for A9. It defines
 * notification operation and remote processor management. */
extern struct hil_platform_ops zynq_a9_proc_ops;

/* Firmware_info and fw_table_size are required in current version (2017.3)
 * of OpenAMP library to pass compilation. Will be removed in next release
 * if the OpenAMP application is not used to boot the remote. */
const struct firmware_info fw_table[] =
{
	{"unknown",
	 0,
	 0}
};
const int fw_table_size = sizeof(fw_table)/sizeof(struct firmware_info);

struct hil_proc *platform_create_proc(int proc_index)
{
	(void) proc_index;
	/* structure to represent a remote processor. It encapsulates the
	 * shared memory and notification info required for inter processor
	 * communication. */
	struct hil_proc *proc;
	proc = hil_create_proc(&zynq_a9_proc_ops, APU_CPU_ID, NULL);
	if (!proc)
		return NULL;

	/*************************************************************
	 * Set vrings notification private data to hil_proc.
	 * We don't set VirtIO device notification because we don't
	 * support VirtIO status changes nofication kick on Zynq7.
	 *************************************************************/
	/* Set vring 0 nofication private data. */
	hil_set_vring_ipi(proc, 0, VRING0_IPI_INTR_VECT, NULL);
	/* Set vring 1 nofication private data. */
	hil_set_vring_ipi(proc, 1, VRING1_IPI_INTR_VECT, NULL);
	/* Set name of RPMsg channel 0 */
	hil_set_rpmsg_channel(proc, 0, RPMSG_CHAN_NAME);

	return proc;
}
