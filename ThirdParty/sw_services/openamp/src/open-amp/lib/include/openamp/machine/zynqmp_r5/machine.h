/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 * Copyright (c) 2015 Xilinx, Inc. All rights reserved.
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

#ifndef _MACHINE_H
#define _MACHINE_H

#include "xil_types.h"

/* IPI REGs OFFSET */
#define IPI_TRIG_OFFSET          0x00000000	/* IPI trigger register offset */
#define IPI_OBS_OFFSET           0x00000004	/* IPI observation register offset */
#define IPI_ISR_OFFSET           0x00000010	/* IPI interrupt status register offset */
#define IPI_IMR_OFFSET           0x00000014	/* IPI interrupt mask register offset */
#define IPI_IER_OFFSET           0x00000018	/* IPI interrupt enable register offset */
#define IPI_IDR_OFFSET           0x0000001C	/* IPI interrupt disable register offset */

#include "openamp/machine/machine_common.h"

#define IPI_TOTAL 11

typedef void (*ipi_handler_t)(unsigned long ipi_base_addr, unsigned int intr_mask, void *data);

struct ipi_handler_info {
	unsigned long ipi_base_addr;
	unsigned int intr_mask;
	void *data;
	ipi_handler_t ipi_handler;
};

struct ipi_info {
	uint32_t ipi_base_addr;
	uint32_t ipi_chn_mask;
};

void ipi_register_handler(unsigned long ipi_base_addr, unsigned int intr_mask,
			  void *data, void *ipi_handler);
void ipi_unregister_handler(unsigned long ipi_base_addr, unsigned int intr_mask);

#endif				/* _MACHINE_H */
