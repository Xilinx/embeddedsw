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
 *       platform.c
 *
 * DESCRIPTION
 *
 *       This file is the Implementation of IPC hardware layer interface
 *       for Xilinx Zynq ZC702EVK platform.
 *
 **************************************************************************/

#include "openamp/hil.h"
#include "openamp/machine/machine_common.h"

/* ------------------------- Macros --------------------------*/
#define ESAL_DP_SLCR_BASE                  0xF8000000
#define PERIPH_BASE                        0xF8F00000
#define GIC_DIST_BASE                      (PERIPH_BASE + 0x00001000)
#define GIC_DIST_SOFTINT                   0xF00
#define GIC_SFI_TRIG_CPU_MASK              0x00FF0000
#define GIC_SFI_TRIG_SATT_MASK             0x00008000
#define GIC_SFI_TRIG_INTID_MASK            0x0000000F
#define GIC_CPU_ID_BASE                    (1 << 4)
#define A9_CPU_SLCR_RESET_CTRL             0x244
#define A9_CPU_SLCR_CLK_STOP               (1 << 4)
#define A9_CPU_SLCR_RST                    (1 << 0)

#define unlock_slcr()                       HIL_MEM_WRITE32(ESAL_DP_SLCR_BASE + 0x08, 0xDF0DDF0D)
#define lock_slcr()                         HIL_MEM_WRITE32(ESAL_DP_SLCR_BASE + 0x04, 0x767B767B)


/* L2Cpl310 L2 cache controller base address. */
#define         HIL_PL130_BASE              0xF8F02000

/********************/
/* Register offsets */
/********************/

#define         HIL_PL130_INVALLINE         0x770
#define         HIL_PL130_CLEANINVLINE      0x7F0


#define         HIL_PA_SBZ_MASK             ~(HIL_CACHE_LINE_SIZE - 1UL)
#define         HIL_CACHE_LINE_SIZE         32
#define         HIL_CACHE_INV_ALL_WAYS      0xFF
#define         HIL_CACHE_UNLOCK_ALL_WAYS   0xFFFF0000
#define         HIL_CACHE_CLEAR_INT         0x1FF

/*--------------------------- Declare Functions ------------------------ */
static int _enable_interrupt(struct proc_vring *vring_hw);
static void _notify(int cpu_id, struct proc_intr *intr_info);
static int _boot_cpu(int cpu_id, unsigned int load_addr);
static void _shutdown_cpu(int cpu_id);

/*--------------------------- Globals ---------------------------------- */
struct hil_platform_ops proc_ops = {
	.enable_interrupt = _enable_interrupt,
	.notify           = _notify,
	.boot_cpu         = _boot_cpu,
	.shutdown_cpu     = _shutdown_cpu,
};

static int _enable_interrupt(struct proc_vring *vring_hw)
{

	/* Register ISR */
	env_register_isr(vring_hw->intr_info.vect_id, vring_hw, platform_isr);

	/* Enable the interrupts */
	env_enable_interrupt(vring_hw->intr_info.vect_id,
			     vring_hw->intr_info.priority,
			     vring_hw->intr_info.trigger_type);
	return 0;
}

static void _notify(int cpu_id, struct proc_intr *intr_info)
{

	unsigned long mask = 0;

	mask = ((1 << (GIC_CPU_ID_BASE + cpu_id)) | (intr_info->vect_id))
	    & (GIC_SFI_TRIG_CPU_MASK | GIC_SFI_TRIG_INTID_MASK);

	HIL_MEM_WRITE32((GIC_DIST_BASE + GIC_DIST_SOFTINT), mask);
}

extern char zynq_trampoline;
extern char zynq_trampoline_jump;
extern char zynq_trampoline_end;

static int _boot_cpu(int cpu_id, unsigned int load_addr)
{
	unsigned int reg;
	unsigned int tramp_size;
	unsigned int tramp_addr = 0;

	if (load_addr) {
		tramp_size = zynq_trampoline_end - zynq_trampoline;
		if ((load_addr < tramp_size) || (load_addr & 0x3)) {
			return -1;
		}

		tramp_size = &zynq_trampoline_jump - &zynq_trampoline;

		/*
		 * Trampoline code is copied to address 0 from where remote core is expected to
		 * fetch first instruction after reset.If master is using the address 0 then
		 * this mem copy will screwed the system. It is user responsibility to not
		 * copy trampoline code in such cases.
		 *
		 */
		env_memcpy((char *)tramp_addr, &zynq_trampoline, tramp_size);
		/* Write image address at the word reserved at the trampoline end */
		HIL_MEM_WRITE32((char *)(tramp_addr + tramp_size), load_addr);
	}

	unlock_slcr();

	reg = HIL_MEM_READ32(ESAL_DP_SLCR_BASE + A9_CPU_SLCR_RESET_CTRL);
	reg &= ~(A9_CPU_SLCR_CLK_STOP << cpu_id);
	HIL_MEM_WRITE32(ESAL_DP_SLCR_BASE + A9_CPU_SLCR_RESET_CTRL, reg);
	/* De-assert reset signal and start clock to start the core */
	reg &= ~(A9_CPU_SLCR_RST << cpu_id);
	HIL_MEM_WRITE32(ESAL_DP_SLCR_BASE + A9_CPU_SLCR_RESET_CTRL, reg);

	lock_slcr();

	return 0;
}

static void _shutdown_cpu(int cpu_id)
{
	unsigned int reg;

	unlock_slcr();

	reg = HIL_MEM_READ32(ESAL_DP_SLCR_BASE + A9_CPU_SLCR_RESET_CTRL);
	/* Assert reset signal and stop clock to halt the core */
	reg |= (A9_CPU_SLCR_CLK_STOP | A9_CPU_SLCR_RST) << cpu_id;
	HIL_MEM_WRITE32(ESAL_DP_SLCR_BASE + A9_CPU_SLCR_RESET_CTRL, reg);

	lock_slcr();
}


