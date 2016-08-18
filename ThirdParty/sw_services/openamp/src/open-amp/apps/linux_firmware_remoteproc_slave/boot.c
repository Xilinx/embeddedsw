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

#include "libfdt/types.h"
#include "libfdt/libfdt.h"
#include "zlib/zlib.h"

/* External variables. */
extern unsigned int _image_start;
extern unsigned int _image_end;
extern unsigned int _bss_start;
extern unsigned int _bss_end;

/* Definitions.*/
#define FIT_IMAGE_START       (void *)&_image_start
#define FIT_IMAGE_END         (void *)&_image_end

#define BSS_START             (void *)&_bss_start
#define BSS_END               (void *)&_bss_end

#define BSS_SIZE              (((unsigned int)BSS_END) - ((unsigned int)BSS_START))

#define XILINX_ARM_MACHINE    3343

#define KERNEL_RESERVED_SPACE 0x7FF2000

#define PUTC(a)                 ((*((volatile unsigned int *) 0xE0001030)) = (a))

/* Globals. */
unsigned int linux_kernel_start, dtb_start, linux_kernel_size, dtb_size;

/* Static functions. */
static void boot_linux_fit_image(void);

static int process_and_relocate_fit_image(char *image_start,
					  unsigned int image_size);

extern void start_linux_with_dtb(void);

static void clear_bss(void);
static void invalidate_cache(void);
static void clean_system(void);

void put_char(char c)
{
	PUTC(c);

	while (((*((volatile unsigned int *)0xE000102C)) & 0x00000008) == 0) ;
}

void putstring(const char *str)
{
	while (*str) {
		put_char(*str++);
	}
}

/* Boots the linux kernel. */
void boot_linux(void)
{
	/* Clear BSS */
	clear_bss();

	clean_system();

	putstring("\n\r********************************* \n\r");
	putstring("OpenAMP Linux Bootstrap.");
	putstring("\n\r********************************* \n\r");

	/* Currently supporting only FIT image format. */
	boot_linux_fit_image();
}

/* Boots a FIT format linux image. */
static void boot_linux_fit_image(void)
{
	unsigned int image_size, status;

	char *image_start;

	/* Retrieve linux image start and end addresses. */
	image_start = (char *)FIT_IMAGE_START;

	/* Retrieve linux image size. */
	image_size = (FIT_IMAGE_END - FIT_IMAGE_START);

	/* Check for a valid linux image size. */
	if (image_size > 0) {

		/* let us parse and relocate the FIT image. */
		status =
		    process_and_relocate_fit_image(image_start, image_size);

		/* Image processed and relocated successfully. */
		if (!status) {

			putstring("\n\rLinux Bootstrap: Booting Linux. \n\r");

			/* Image has been processed and relocated. Now boot linux */
			start_linux_with_dtb();
		} else {
			/* Go into an error loop. */
			while (1) ;
		}
	} else {
		/* Go into an error loop. */
		while (1) ;
	}
}

/* Returns zero for success. */
static int process_and_relocate_fit_image(char *image_start,
					  unsigned int image_size)
{
	unsigned int fit_image_start, compressed = 0;
	unsigned long kernel_address;
	int size, load_size, load_address, dtb_address;
	char *conf_name = NULL;
	void *data;
	int cfg_offset, offset, ret;
	z_stream strm;

	putstring
	    ("\n\rLinux Bootstrap: Locating Linux Kernel and DTB from FIT image.\n\r");

	fit_image_start = (unsigned int)image_start;

	/* Retrieve default FIT image configuration node. */
	offset =
	    fdt_path_offset((const void *)fit_image_start, "/configurations");

	if (offset >= 0) {
		/* Retrieve default configuration name. */
		conf_name =
		    (char *)fdt_getprop((const void *)fit_image_start, offset,
					"default", &size);
	}

	if (conf_name) {
		/* Retrieve the offset of configuration node. */
		cfg_offset =
		    fdt_subnode_offset((const void *)fit_image_start, offset,
				       conf_name);
	}

	/* Retrieve kernel node using the config node. */
	conf_name =
	    (char *)fdt_getprop((const void *)fit_image_start, cfg_offset,
				"kernel", &size);

	if (conf_name) {
		offset =
		    fdt_path_offset((const void *)fit_image_start, "/images");

		if (offset >= 0) {
			offset =
			    fdt_subnode_offset((const void *)fit_image_start,
					       offset, conf_name);
		}
	}

	if (offset >= 0) {
		/* Retrieve kernel image address and size. */
		kernel_address =
		    (unsigned long)fdt_getprop((const void *)fit_image_start,
					       offset, "data", &load_size);

		/* Retrieve kernel load address. */
		data =
		    (void *)fdt_getprop((const void *)fit_image_start, offset,
					"load", &size);

		load_address = *((int *)data);

		load_address = be32_to_cpu(load_address);

		/* Check kernel image for compression. */
		data =
		    (void *)fdt_getprop((const void *)fit_image_start, offset,
					"compression", &size);

		if (data != NULL) {
			if (!(strcmp(data, "gzip"))) {
				compressed = 1;
			}
		}
	}

	memset((void *)load_address, 0, 0x0600000 - load_address);

	if (compressed == 1) {
		putstring
		    ("\n\rLinux Bootstrap: Kernel image is compressed. Starting decompression process. It may take a while...\n\r");

		/* Initialize zlib stream. */
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		strm.avail_in = 0;
		strm.next_in = Z_NULL;

		/* Initialize the zlib state for de-compression. */
		ret = inflateInit2(&strm, MAX_WBITS + 16);

		if (ret == Z_OK) {
			strm.next_in = (Bytef *) kernel_address;
			strm.avail_out = KERNEL_RESERVED_SPACE;
			strm.avail_in = load_size;

			/* Pointer to output space. */
			strm.next_out = (Bytef *) load_address;

			/* Call the de-compression engine. */
			ret = inflate(&strm, Z_FINISH);
		}

		(void)inflateEnd(&strm);

		if ((ret != Z_OK) && (ret != Z_STREAM_END)) {

			/* return with an error. */
			return 1;
		}

		putstring
		    ("\n\rLinux Bootstrap: Linux image decompression complete. \n\r");

	} else {
		/* Uncompressed image. Just load to the load address. */
		memcpy((void *)load_address, (void *)kernel_address, load_size);
	}

	putstring
	    ("\n\rLinux Bootstrap: Linux kernel image has been loaded into memory. \n\r");

	/* Save kernel load address and size. */
	linux_kernel_start = load_address;
	linux_kernel_size = load_size;

	/* Retrieve DTB node using the config node. */
	conf_name =
	    (char *)fdt_getprop((const void *)fit_image_start, cfg_offset,
				"fdt", &size);

	if (conf_name) {
		offset =
		    fdt_path_offset((const void *)fit_image_start, "/images");

		if (offset >= 0) {
			offset =
			    fdt_subnode_offset((const void *)fit_image_start,
					       offset, conf_name);
		}
	}

	if (offset >= 0) {
		/* Retrieve DTB address and size. */
		dtb_address =
		    (unsigned long)fdt_getprop((const void *)fit_image_start,
					       offset, "data", &load_size);
	}

	dtb_start = (linux_kernel_start + KERNEL_RESERVED_SPACE) & 0xFFFFFF00;
	dtb_size = load_size;

	memcpy((void *)dtb_start, (void *)dtb_address, load_size);

	putstring("\n\rLinux Bootstrap: Loaded DTB. \n\r");

	return 0;
}

static void clear_bss(void)
{
	memset(BSS_START, 0, BSS_SIZE);
}

/*
 * The code in this section is for invalidating the cache at startup
 *
 */

/* ARM Coprocessor registers */
#define         ARM_AR_CP0                 p0
#define         ARM_AR_CP1                 p1
#define         ARM_AR_CP2                 p2
#define         ARM_AR_CP3                 p3
#define         ARM_AR_CP4                 p4
#define         ARM_AR_CP5                 p5
#define         ARM_AR_CP6                 p6
#define         ARM_AR_CP7                 p7
#define         ARM_AR_CP8                 p8
#define         ARM_AR_CP9                 p9
#define         ARM_AR_CP10                p10
#define         ARM_AR_CP11                p11
#define         ARM_AR_CP12                p12
#define         ARM_AR_CP13                p13
#define         ARM_AR_CP14                p14
#define         ARM_AR_CP15                p15

/* CRn and CRm register values */
#define         ARM_AR_C0                  c0
#define         ARM_AR_C1                  c1
#define         ARM_AR_C2                  c2
#define         ARM_AR_C3                  c3
#define         ARM_AR_C4                  c4
#define         ARM_AR_C5                  c5
#define         ARM_AR_C6                  c6
#define         ARM_AR_C7                  c7
#define         ARM_AR_C8                  c8
#define         ARM_AR_C9                  c9
#define         ARM_AR_C10                 c10
#define         ARM_AR_C11                 c11
#define         ARM_AR_C12                 c12
#define         ARM_AR_C13                 c13
#define         ARM_AR_C14                 c14
#define         ARM_AR_C15                 c15

/* This define is used to add quotes to anything passed in */
#define         ARM_AR_QUOTES(x)           #x

/* This macro writes to a coprocessor register */
#define         ARM_AR_CP_WRITE(cp, op1, cp_value, crn, crm, op2)              \
                {                                                              \
                    asm volatile("    MCR    " ARM_AR_QUOTES(cp) ","           \
                                             #op1                              \
                                             ", %0, "                          \
                                             ARM_AR_QUOTES(crn) ","            \
                                             ARM_AR_QUOTES(crm) ","            \
                                             #op2                              \
                                    : /* No outputs */                         \
                                    : "r" (cp_value));                         \
                }

/* This macro reads from a coprocessor register */
#define         ARM_AR_CP_READ(cp, op1, cp_value_ptr, crn, crm, op2)           \
                {                                                              \
                    asm volatile("    MRC    " ARM_AR_QUOTES(cp) ","           \
                                             #op1                              \
                                             ", %0, "                          \
                                             ARM_AR_QUOTES(crn) ","            \
                                             ARM_AR_QUOTES(crm) ","            \
                                             #op2                                   \
                                        : "=r" (*(unsigned long *)(cp_value_ptr))   \
                                        : /* No inputs */ );                        \
                }

/* This macro executes a ISB instruction */
#define         ARM_AR_ISB_EXECUTE()                                           \
                {                                                                   \
                    asm volatile("    ISB");                                        \
                }

/* This macro executes a DSB instruction */
#define         ARM_AR_DSB_EXECUTE()                                           \
                {                                                                   \
                    asm volatile("    DSB");                                        \
                }

/* CLIDR and CCSIDR mask values */
#define     ARM_AR_MEM_CLIDR_LOC_MASK              0x7000000
#define     ARM_AR_MEM_CCSIDR_LINESIZE_MASK        0x7
#define     ARM_AR_MEM_CCSIDR_ASSOC_MASK           0x3FF
#define     ARM_AR_MEM_CCSIDR_NUMSET_MASK          0x7FFF

/* CLIDR and CCSIDR shift values */
#define     ARM_AR_MEM_CLIDR_LOC_RSHT_OFFSET       24
#define     ARM_AR_MEM_CCSIDR_ASSOC_RSHT_OFFSET    3
#define     ARM_AR_MEM_CCSIDR_NUMSET_RSHT_OFFSET   13

/* Extract 'encoded' line length of the cache */
#define     ARM_AR_MEM_CCSIDR_LINESIZE_GET(ccsidr_reg) (ccsidr_reg &                           \
                                                         ARM_AR_MEM_CCSIDR_LINESIZE_MASK)

/* Extract 'encoded' way size of the cache */
#define     ARM_AR_MEM_CCSIDR_ASSOC_GET(ccsidr_reg)    (ARM_AR_MEM_CCSIDR_ASSOC_MASK &        \
                                                        (ccsidr_reg >>                          \
                                                         ARM_AR_MEM_CCSIDR_ASSOC_RSHT_OFFSET))

/* Extract 'encoded' maximum number of index size */
#define     ARM_AR_MEM_CCSIDR_NUMSET_GET(ccsidr_reg)   (ARM_AR_MEM_CCSIDR_NUMSET_MASK &       \
                                                        (ccsidr_reg >>                          \
                                                         ARM_AR_MEM_CCSIDR_NUMSET_RSHT_OFFSET))

/* Refer to chapter B3.12.31 c7, Cache and branch predictor maintenance functions in the
 ARM Architecture Reference Manual ARMv7-A and ARMv7-R Edition 1360*/
/* Calculate # of bits to be shifted for set size and way size */

/* log2(line size in bytes) = ccsidr_linesize + 2 + logbase2(4) */
#define     ARM_AR_MEM_L_CALCULATE(linesize)           (linesize + 2 + 2)

/* log2(nsets) = 32 - way_size_bit_pos */

/* Find the bit position of way size increment */
#define     ARM_AR_MEM_A_CALCULATE(assoc, a_offset_ref)                                        \
            {                                                                                   \
                unsigned int  temp_pos = 0x80000000;                                                  \
                                                                                                \
                *a_offset_ref = 0;                                                              \
                                                                                                \
                /* Logic to count the number of leading zeros before the first 1 */             \
                while(!((assoc & temp_pos) == temp_pos))                                        \
                {                                                                               \
                    (*a_offset_ref)++;                                                          \
                    temp_pos = temp_pos >> 1;                                                   \
                }                                                                               \
            }

/* Factor way, cache number, index number */
#define     ARM_AR_MEM_DCCISW_SET(dccisw_ref, level, numsets, assoc, l_offset, a_offset)       \
            {                                                                                   \
                *dccisw_ref = (level | (numsets << l_offset) | (assoc << a_offset));            \
            }

/* This macro extracts line size, assoc and set size from CCSIDR */
#define     ARM_AR_MEM_CCSIDR_VALS_GET(linesize_ref, assoc_ref, numsets_ref,                   \
                                        l_offset_ref, a_offset_ref)                             \
            {                                                                                   \
                unsigned int  ccsidr_val;                                                             \
                                                                                                \
                /* Read the selected cache's CCSIDR */                                          \
                ARM_AR_CP_READ(ARM_AR_CP15, 1, &ccsidr_val,                           \
                                    ARM_AR_C0, ARM_AR_C0, 0);                         \
                                                                                                \
                /* Extract 'encoded' line length of the cache */                                \
                *linesize_ref = ARM_AR_MEM_CCSIDR_LINESIZE_GET(ccsidr_val);                    \
                                                                                                \
                /* Extract 'encoded' way size of the cache */                                   \
                *assoc_ref = ARM_AR_MEM_CCSIDR_ASSOC_GET(ccsidr_val);                          \
                                                                                                \
                /* Extract 'encoded' maximum number of index size */                            \
                *numsets_ref = ARM_AR_MEM_CCSIDR_NUMSET_GET(ccsidr_val);                       \
                                                                                                \
                /* Calculate # of bits to be shifted for set size and way size */               \
                                                                                                \
                /* log2(line size in bytes) = ccsidr_linesize + 2 + log2(4) */                  \
                *l_offset_ref = ARM_AR_MEM_L_CALCULATE(*linesize_ref);                         \
                                                                                                \
                /* log2(nsets) = 32 - way_size_bit_pos */                                       \
                ARM_AR_MEM_A_CALCULATE(*assoc_ref, a_offset_ref);                              \
            }

/* This macro invalidates all of the instruction cache at the core level. */
#define     ARM_AR_MEM_ICACHE_ALL_INVALIDATE()                         \
            {                                                           \
                ARM_AR_CP_WRITE(ARM_AR_CP15, 0,               \
                                     0, ARM_AR_C7,                 \
                                     ARM_AR_C5, 0);                \
            }

/* This macro invalidates all of the data cache at the core level. */
void ARM_AR_MEM_DCACHE_ALL_OP(int type)
{
	unsigned int clidr_val = 0;
	unsigned int clidr_loc = 0;
	unsigned int cache_number = 0;
	unsigned int cache_type = 0;
	unsigned int ccsidr_linesize = 0;
	unsigned int ccsidr_assoc = 0;
	int ccsidr_numsets = 0;
	int way_size_copy = 0;
	unsigned int set_size_bit_pos = 0;
	unsigned int cache_number_pos = 0;
	unsigned int way_size_bit_pos = 0;
	unsigned int set_way_value = 0;

	/* Read CLIDR to extract level of coherence (LOC) */
	ARM_AR_CP_READ(ARM_AR_CP15, 1, &clidr_val, ARM_AR_C0, ARM_AR_C0, 1);

	/* Extract LOC from CLIDR and align it at bit 1 */
	clidr_loc = (clidr_val & ARM_AR_MEM_CLIDR_LOC_MASK) >>
	    ARM_AR_MEM_CLIDR_LOC_RSHT_OFFSET;

	/* Proceed only iff LOC is non-zero */
	if (clidr_loc != 0) {
		do {
			/* Extract cache type from CLIDR */
			cache_number_pos = cache_number + (cache_number >> 1);
			cache_type = (clidr_val >> cache_number_pos) & 0x7;

			/* Continue only iff data cache */
			if (cache_type >= 2) {
				/* Select desired cache level in CSSELR */
				ARM_AR_CP_WRITE(ARM_AR_CP15, 2, cache_number,
						ARM_AR_C0, ARM_AR_C0, 0);

				ARM_AR_ISB_EXECUTE();

				/* Get data like linesize, assoc and set size */
				ARM_AR_MEM_CCSIDR_VALS_GET(&ccsidr_linesize,
							   &ccsidr_assoc,
							   &ccsidr_numsets,
							   &set_size_bit_pos,
							   &way_size_bit_pos);

				do {
					way_size_copy = ccsidr_assoc;

					do {
						/* Factor way, cache number, index number */
						ARM_AR_MEM_DCCISW_SET
						    (&set_way_value,
						     cache_number,
						     ccsidr_numsets,
						     way_size_copy,
						     set_size_bit_pos,
						     way_size_bit_pos);

						/* Execute invalidate if type = 0 */
						if (type == 0) {
							ARM_AR_CP_WRITE
							    (ARM_AR_CP15, 0,
							     set_way_value,
							     ARM_AR_C7,
							     ARM_AR_C6, 2);
						} else {
							ARM_AR_CP_WRITE
							    (ARM_AR_CP15, 0,
							     set_way_value,
							     ARM_AR_C7,
							     ARM_AR_C14, 2);
						}

						/* decrement the way */
					} while ((--way_size_copy) >= 0);

					/* decrement the set */
				} while ((--ccsidr_numsets) >= 0);

			}

			/* end if */
			/* Increment cache number */
			cache_number += 2;

			/* end do-while */
		} while (clidr_loc >= cache_number);

	}

	/* Switch back to cache level 0 in CSSELR */
	ARM_AR_CP_WRITE(ARM_AR_CP15, 2, 0, ARM_AR_C0, ARM_AR_C0, 0);

	/* Sync */
	ARM_AR_DSB_EXECUTE();
	ARM_AR_ISB_EXECUTE();
}

/* This macro invalidates all of the data cache at the core level. */
void ARM_AR_MEM_DCACHE_ALL_INVALIDATE(void)
{
	ARM_AR_MEM_DCACHE_ALL_OP(0);
}

/* This macro invalidates all of the cache at the core level. */
void ARM_AR_MEM_CACHE_ALL_INVALIDATE(void)
{
	ARM_AR_MEM_ICACHE_ALL_INVALIDATE();
	ARM_AR_MEM_DCACHE_ALL_INVALIDATE();
}

static void clean_system(void)
{

	invalidate_cache();

}

static void invalidate_cache(void)
{
	ARM_AR_MEM_CACHE_ALL_INVALIDATE();
}
