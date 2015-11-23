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

#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdio.h>
#include "hil.h"

/* IPC Device parameters */
#define SHM_ADDR                          (void *)0x08008000
#define SHM_SIZE                          0x00200000
#define VRING0_IPI_VECT                   15
#define VRING1_IPI_VECT                   14
#define MASTER_CPU_ID                     0
#define REMOTE_CPU_ID                     1
/* ------------------------- Macros --------------------------*/
#define ESAL_DP_SLCR_BASE                  0xF8000000
#define PERIPH_BASE1                        0xF8F00000
#define GIC_DIST_BASE                      (PERIPH_BASE1 + 0x00001000)
#define GIC_DIST_SOFTINT                   0xF00
#define GIC_SFI_TRIG_CPU_MASK              0x00FF0000
#define GIC_SFI_TRIG_SATT_MASK             0x00008000
#define GIC_SFI_TRIG_INTID_MASK            0x0000000F
#define GIC_CPU_ID_BASE                    (1 << 4)
#define A9_CPU_SLCR_RESET_CTRL             0x244
#define A9_CPU_SLCR_CLK_STOP               (1 << 4)
#define A9_CPU_SLCR_RST                    (1 << 0)

#define unlock_slcr()                       Xil_Out32(ESAL_DP_SLCR_BASE + 0x08, 0xDF0DDF0D)
#define lock_slcr()                         Xil_Out32(ESAL_DP_SLCR_BASE + 0x04, 0x767B767B)


int _enable_interrupt(struct proc_vring *vring_hw);
void _notify(int cpu_id, struct proc_intr *intr_info);
int _boot_cpu(int cpu_id, unsigned int load_addr);
void _shutdown_cpu(int cpu_id);
void platform_isr(int vect_id, void *data,unsigned int intr_status);

#endif /* PLATFORM_H_ */
