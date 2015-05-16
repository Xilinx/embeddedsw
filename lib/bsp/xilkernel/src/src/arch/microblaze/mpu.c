/******************************************************************************
*
* Copyright (C) 2010 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

//----------------------------------------------------------------------------------------------------//
//! @file mpu.c
//! Microblaze MPU hardware specific initialization
//----------------------------------------------------------------------------------------------------//
#include <os_config.h>
#include <config/config_param.h>
#include <config/config_cparam.h>
#include <xparameters.h>
#include <mb_interface.h>
#include <sys/mpu.h>
#include <sys/decls.h>
#include <stdio.h>

#if (XPAR_MICROBLAZE_USE_MMU >= 2) && !defined (XILKERNEL_MB_MPU_DISABLE)

//----------------------------------------------------------------------------------------------------//
// Declarations
//----------------------------------------------------------------------------------------------------//

#define TLBSIZE 64                                  /* Total number of available TLB entries */

#define MSR_VM_MASK             0x00002000
#define MSR_VMS_MASK            0x00004000
#define MSR_EE_MASK             0x00000100

static const unsigned int sizemask[8] = {
    0xfffffc00, 0xfffff000, 0xffffc000, 0xffff0000,
    0xfffc0000, 0xfff00000, 0xffc00000, 0xff000000
};

static void tlb_add (int tlbindex, unsigned int tlbhi, unsigned int tlblo);
static int  tlb_add_entries (unsigned int base, unsigned int high, unsigned int tlbaccess);
int mpu_init(void);
//----------------------------------------------------------------------------------------------------//
// Data
//----------------------------------------------------------------------------------------------------//

static struct {
    unsigned int tlbhi;
    unsigned int tlblo;
} tlbentry[TLBSIZE];

extern xilkernel_io_range_t system_io_range[];
extern int user_io_nranges __attribute__((weak));
extern xilkernel_io_range_t user_io_range[] __attribute__((weak));

//----------------------------------------------------------------------------------------------------//
// Definitions
//----------------------------------------------------------------------------------------------------//


//----------------------------------------------------------------------------------------------------//
//  @func - tlb_add
//! @desc
//!   Add a single TLB entry
//! @param
//!   - tlbindex is the index of the TLB entry to be updated
//!   - tlbhi is the value of the TLBHI field
//!   - tlblo is the value of the TLBLO field
//! @return
//!   - none
//! @note
//!   - none
//----------------------------------------------------------------------------------------------------//
static inline void tlb_add(int tlbindex, unsigned int tlbhi, unsigned int tlblo)
{
    __asm__ __volatile__ ("mts rtlbx,  %2 \n\t"
                          "mts rtlbhi, %0 \n\t"
                          "mts rtlblo, %1 \n\t"
                          :: "r" (tlbhi),
                          "r" (tlblo),
                          "r" (tlbindex));

    tlbentry[tlbindex].tlbhi = tlbhi;
    tlbentry[tlbindex].tlblo = tlblo;
}



//----------------------------------------------------------------------------------------------------//
//  @func - mpu_init
//! @desc
//!   Given a base and high address, figure out the minimum number page mappings/TLB entries required
//!   to cover the area. This function uses recursion to figure out the entire range of mappings.
//! @param
//!   - base is the base address of the region of memory
//!   - high is the high address of the region of memory
//!   - tlbaccess is the type of access required for this region of memory. It can be
//!     a logical or-ing of the following flags.
//!         - 0 indicates read-only access
//!         - TLB_ACCESS_EXECUTABLE means the region is executable
//!         - TLB_ACCESS_WRITABLE means the region is writable
//! @return
//!   - 1 on success and 0 on failure
//! @note
//!   - none
//----------------------------------------------------------------------------------------------------//
static int tlb_add_entries (unsigned int base, unsigned int high, unsigned int tlbaccess)
{
    int sizeindex, tlbsizemask;
    unsigned int tlbhi, tlblo;
    unsigned int area_base, area_high, area_size;
    static int tlbindex = 0;

    // Align base and high to 1KB boundaries
    base = base & 0xfffffc00;
    high = (high >= 0xfffffc00) ? 0xffffffff : ((high + 0x400) & 0xfffffc00) - 1;

    // Start trying to allocate pages from 16 MB granularity down to 1 KB
    area_size   = 0x1000000;            // 16 MB
    tlbsizemask = 0x380;                // TLBHI[SIZE] = 7 (16 MB)

    for (sizeindex = 7; sizeindex >= 0; sizeindex--) {
        area_base = base & sizemask[sizeindex];
        area_high = area_base + (area_size - 1);

        // if (area_base <= (0xffffffff - (area_size - 1))) {

        if ((area_base >= base) && (area_high <= high)) {

            if (tlbindex < TLBSIZE) {
                tlbhi = (base & sizemask[sizeindex]) | tlbsizemask | 0x40;          // TLBHI: TAG, SIZE, V
                tlblo = (base & sizemask[sizeindex]) | tlbaccess | 0x8;             // TLBLO: RPN, EX, WR, W
                tlb_add (tlbindex, tlbhi, tlblo);

                // DPRINTF ("XMK: TLB %d region: 0x%x - 0x%x, size: 0x%x, access: 0x%x.\r\n", tlbindex, base & sizemask[sizeindex],
                // (base & sizemask[sizeindex]) + area_size - 1,
                // tlbsizemask >> 7, tlbaccess >> 8);

                tlbindex++;
            } else {
                // We only handle the 64 entry UTLB management for now
                DPRINTF ("XMK: Out of TLB entries.\r\n");
                return 0;
            }

            // Recursively add entries for lower area
            if (area_base > base)
                if (!tlb_add_entries (base, area_base - 1, tlbaccess))
                    return 0;

            // Recursively add entries for higher area
            if (area_high < high)
                if (!tlb_add_entries(area_high + 1, high, tlbaccess))
                    return 0;

            break;
        }

        // else, we try the next lower page size
        area_size   = area_size >> 2;
        tlbsizemask = tlbsizemask - 0x80;
    }

    return 1;
}

//----------------------------------------------------------------------------------------------------//
//  @func - mpu_init
//! @desc
//!   Initialize and use MicroBlaze MPU/MMU features for providing memory protection.
//1
//!   - This code assumes the following logical sections are present in the linker script,
//!     aligned at a 1K boundary and are clearly demarcated by the labels shown. If not,
//!     this function ignore the missing logical sections silently.
//!     ---------------------------------------------------------------------------------------
//!     SECTION         .vectors
//!     START_LABEL     n.a
//!     END_LABEL       n.a
//!     CONTAINS        Executable vector sections. Do not need TLBs as MB always enters these
//!                     in priv/unprotected mode.
//!     ---------------------------------------------------------------------------------------
//!     SECTION         .text
//!     START_LABEL     _ftext
//!     END_LABEL       _etext
//!     CONTAINS        Executable instruction sections.
//!     ---------------------------------------------------------------------------------------
//!     SECTION         .data
//!     START_LABEL     _fdata
//!     END_LABEL       _edata
//!     CONTAINS        Read-write data sections including small data sections.
//!     ---------------------------------------------------------------------------------------
//!     SECTION         .rodata
//!     START_LABEL     _frodata
//!     END_LABEL       _erodata
//!     CONTAINS        Read only data sections including small data sections
//!     ---------------------------------------------------------------------------------------
//!     SECTION         .stack
//!     START_LABEL     _stack_end
//!     END_LABEL       _stack
//!     CONTAINS        Kernel stack with 1 KB guard page above and below
//!     ---------------------------------------------------------------------------------------
//!     SECTION         stack guard page (top)
//!     START_LABEL     _fstack_guard_top
//!     END_LABEL       _estack_guard_top
//!     CONTAINS        Top kernel stack guard page (1 KB)
//!     ---------------------------------------------------------------------------------------
//!     SECTION         stack guard page (bottom)
//!     START_LABEL     _fstack_guard_bottom
//!     END_LABEL       _estack_guard_bottom
//!     CONTAINS        Bottom kernel stack guard page (1 KB)
//!     ---------------------------------------------------------------------------------------
//!
//! @param
//!   - none
//! @return
//!   - 1 on success and 0 on failure
//! @note
//!   - none
//----------------------------------------------------------------------------------------------------//
int mpu_init(void)
{
    unsigned int msr;
    int i;

    DPRINTF ("XMK: Initializing memory protection.\r\n");

    // Invalidate all TLB entries first
        for (i = 0; i < TLBSIZE; i++)
            tlb_add (i, 0, 0);

        // Add TLB entries for CODE
        if (&_ftext != 0 && &_etext != 0) {
            if (!tlb_add_entries ((unsigned int)&_ftext, (unsigned int)&_etext, MPU_PROT_EXEC))
                return 0;
        }

        // Add TLB entries for DATA
        if (&_fdata != 0 && &_edata != 0) {
            if (!tlb_add_entries ((unsigned int)&_fdata, (unsigned int)&_edata,  MPU_PROT_READWRITE))
            return 0;
        }

        // Add TLB entries for RODATA
        if (&_frodata != 0 && &_erodata != 0) {
            if (!tlb_add_entries ((unsigned int)&_frodata, (unsigned int)&_erodata, 0))
            return 0;
        }

        // Add TLB entries for STACK
        if (&_stack_end != 0 && &__stack != 0) {
            if (!tlb_add_entries ((unsigned int)&_stack_end, (unsigned int)&__stack, MPU_PROT_READWRITE))
                return 0;
        }

        // Add TLB entries for Vector Table
        if (!tlb_add_entries ((unsigned int)0x00000000, (unsigned int)0x0000004F, MPU_PROT_EXEC))
		return 0;

        // Add TLB entries for System I/O ranges
        for (i = 0; i < XILKERNEL_IO_NRANGES; i++)
            if (!tlb_add_entries ((unsigned int)system_io_range[i].baseaddr, (unsigned int)system_io_range[i].highaddr, system_io_range[i].flags))
                return 0;

        // Add TLB entries for User I/O ranges (if specified)
        if (user_io_nranges != 0) {
            for (i = 0; i < user_io_nranges; i++)
                if (!tlb_add_entries ((unsigned int)user_io_range[i].baseaddr, (unsigned int)user_io_range[i].highaddr, user_io_range[i].flags))
                    return 0;
        }

        // Initialize PID register
        asm volatile ("mts  rpid, r0 \n\t"
                      "bri  4\n\t");

        // Initialize ZPR register
        asm volatile ("mts  rzpr, r0 \n\t"
                      "bri  4\n\t");

        // Initialize VM mode
        asm volatile ("mfs  %0, rmsr \n\t"
                      "ori  %0, %0, %1 \n\t;"
                      "mts  rmsr, %0 \n\t"
                  "bri  4\n\t" : "=r"(msr) : "i" (MSR_VM_MASK | MSR_VMS_MASK | MSR_EE_MASK));

    return 1;
}

#endif /* (XPAR_MICROBLAZE_USE_MMU >= 2) && !defined (XILKERNEL_MB_MPU_DISABLE) */
