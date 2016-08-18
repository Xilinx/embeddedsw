/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
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
#include <stdio.h>
#include <string.h>
#include "baremetal.h"

void arm_ar_map_mem_region(unsigned int vrt_addr, unsigned int phy_addr,
			   unsigned int size, int is_mem_mapped,
			   CACHE_TYPE cache_type);


/* FIQ Handler */
void __attribute__ ((interrupt("FIQ"))) __cs3_isr_fiq()
{
	while (1) ;
}

/***********************************************************************
 *
 *
 * arm_ar_map_mem_region
 *
 *
 * This function sets-up the region of memory based on the given
 * attributes

 *
 * @param vrt_addr       - virtual address of region
 * @param phy_addr       - physical address of region
 * @parma size           - size of region
 * @param is_mem_mapped  - memory mapped or not

 * @param cache_type     - cache type of region
 *
 *
 *   OUTPUTS
 *
 *       None
 *
 ***********************************************************************/
void arm_ar_map_mem_region(unsigned int vrt_addr, unsigned int phy_addr,
			   unsigned int size, int is_mem_mapped,
			   CACHE_TYPE cache_type)
{
	unsigned int section_offset;
	unsigned int ttb_offset;
	unsigned int ttb_value;
	unsigned int ttb_base;

	/* Read ttb base address */
	ARM_AR_CP_READ(ARM_AR_CP15, 0, &ttb_base, ARM_AR_C2, ARM_AR_C0, 0);

	/* Ensure the virtual and physical addresses are aligned on a
	   section boundary */
	vrt_addr &= ARM_AR_MEM_TTB_SECT_SIZE_MASK;
	phy_addr &= ARM_AR_MEM_TTB_SECT_SIZE_MASK;

	/* Loop through entire region of memory (one MMU section at a time).
	   Each section requires a TTB entry. */
	for (section_offset = 0; section_offset < size; section_offset +=
	     ARM_AR_MEM_TTB_SECT_SIZE) {

		/* Calculate translation table entry offset for this memory section */
		ttb_offset = ((vrt_addr + section_offset)
			      >> ARM_AR_MEM_TTB_SECT_TO_DESC_SHIFT);

		/* Build translation table entry value */
		ttb_value = (phy_addr + section_offset)
		    | ARM_AR_MEM_TTB_DESC_ALL_ACCESS;

		if (!is_mem_mapped) {

			/* Set cache related bits in translation table entry.
			   NOTE: Default is uncached instruction and data. */
			if (cache_type == WRITEBACK) {
				/* Update translation table entry value */
				ttb_value |=
				    (ARM_AR_MEM_TTB_DESC_B |
				     ARM_AR_MEM_TTB_DESC_C);
			} else if (cache_type == WRITETHROUGH) {
				/* Update translation table entry value */
				ttb_value |= ARM_AR_MEM_TTB_DESC_C;
			}
			/* In case of un-cached memory, set TEX 0 bit to set memory
			   attribute to normal. */
			else if (cache_type == NOCACHE) {
				ttb_value |= ARM_AR_MEM_TTB_DESC_TEX;
			}
		}

		/* Write translation table entry value to entry address */
		MEM_WRITE32(ttb_base + ttb_offset, ttb_value);

	}			/* for loop */
}

/*==================================================================*/
/* The function definitions below are provided to prevent the build */
/* warnings for missing I/O function stubs in case of unhosted libs */
/*==================================================================*/

#include            <sys/stat.h>

/**
 * _fstat
 *
 * Status of an open file. For consistency with other minimal
 * implementations in these examples, all files are regarded
 * as character special devices.
 *
 * @param file    - Unused.
 * @param st      - Status structure.
 *
 *
 *       A constant value of 0.
 *
 **/
__attribute__ ((weak))
int _fstat(int file, struct stat *st)
{
	return (0);
}

/**
 *  isatty
 *
 *
 * Query whether output stream is a terminal. For consistency
 * with the other minimal implementations, which only support
 * output to stdout, this minimal implementation is suggested
 *
 * @param file    - Unused
 *
 * @return s - A constant value of 1.
 *
 */
__attribute__ ((weak))
int _isatty(int file)
{
	return (1);
}

/**
 *_lseek
 *
 * Set position in a file. Minimal implementation.

 *
 * @param file    - Unused
 *
 * @param ptr     - Unused
 *
 * @param dir     - Unused
 *
 * @return - A constant value of 0.
 *
 */
__attribute__ ((weak))
int _lseek(int file, int ptr, int dir)
{
	return (0);
}

/**
 *  _open
 *
 * Open a file.  Minimal implementation
 *
 * @param filename    - Unused
 * @param flags       - Unused
 * @param mode        - Unused
 *
 * return -  A constant value of 1.
 *
 */
__attribute__ ((weak))
int _open(const char *filename, int flags, int mode)
{
	/* Any number will work. */
	return (1);
}

/**
 *  _close
 *
 * Close a file.  Minimal implementation.
 *
 *
 * @param file    - Unused
 *
 *
 * return A constant value of -1.
 *
 */
__attribute__ ((weak))
int _close(int file)
{
	return (-1);
}

/**
 * _read
 *
 *  Low level function to redirect IO to serial.
 *
 * @param fd          - Unused
 * @param buffer      - Buffer where read data will be placed.
 * @param buflen      - Size (in bytes) of buffer.
 *
 * return -  A constant value of 1.
 *
 */
__attribute__ ((weak))
int _read(int fd, char *buffer, int buflen)
{
	return -1;
}

/**
 * _write
 *
 * Low level function to redirect IO to serial.
 *
 *
 * @param file                          - Unused
 * @param CHAR *ptr                         - String to output
 * @param len                           - Length of the string
 *
 * return len                            - The length of the string
 *
 */
__attribute__ ((weak))
int _write(int file, const char *ptr, int len)
{
	return 0;
}
