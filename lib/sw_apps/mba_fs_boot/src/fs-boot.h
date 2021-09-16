/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************
*
* @file fs-boot.h
*
* DESCRIPTION:
*     Header file for the Xilinx Inc. first-stage bootloader FS-BOOT.
*
*******************************************************************************/
#include "auto-config.h"
#include "xparameters.h"

#ifdef CONFIG_UARTLITE
#include "xuartlite_l.h"
#elif CONFIG_UART16550
#include "xuartns550_l.h"
#endif

#ifdef DEBUG
#include <stdio.h>
#endif

/* Weak declarations of custom hooks to be optionally provided by the user. */
/*
   __fs_preinit is called right at the start of fs-boot, before serial/stdio
     is available.  Might be used for custom flash or memory chip select setup.
     Arguments:
        None
     Return value:
        None
*/
void __fs_preinit(void) __attribute__((weak));

/*
  __fs_preboot is called immediately prior to jumping off to the real boot image
     (typically u-boot, but possibly the kernel in direct boot scenarios).
     Arguments:
        unsigned bootaddr - the address that fs-boot is about to jump to
     Return value:
        0 - image is fine for booting
        otherwise - fs-boot will not boot the image

    This might be used to do a checksum or other check on the boot image.
*/
int __fs_preboot(unsigned bootaddr) __attribute__((weak));

/*
     __fs_bad_image is called when booting from image failed.
     user can overwrite it by custom code. Failure reasons are listed below.
     Arguments:
        u32 reason - the reason of failure
     Return value:
        None
*/
#define REASON_BAD_ADDRESS	1
#define REASON_BAD_MAGIC	2
#define REASON_FLASH_FAIL	3

void __fs_bad_image(unsigned int reason) __attribute__((weak));

#define BAD_IMAGE(reason) \
	if(__fs_bad_image) \
		__fs_bad_image((reason));\
	else \
		while(1) \
			;


/*! Debug flag to turn on debug messages */
//#define DEBUG

/* enable SREC functionality for system without flash */
#ifndef CONFIG_XILINX_FLASH_START /* without parallel flash */
#ifndef CONFIG_PRIMARY_FLASH_SPI /* without SPI flash */
#define CONFIG_NO_FLASH
#endif
#endif

/* Memory access type */
#define mtype   volatile unsigned long
#define REG32_READ(addr,offset)       (*(mtype *)(addr + offset))
#define REG32_WRITE(addr,data)        (*(mtype *)(addr) = data)

#ifndef CONFIG_NO_FLASH
/*! FLASH size */
#define FLASH_SIZE      CONFIG_XILINX_FLASH_SIZE
#endif

/*! SDRAM size */
#define RAM_SIZE        CONFIG_XILINX_ERAM_SIZE

/*! @defgroup mmap1 Memory Map Addressing Definitions
 * MEMORY MAP PERIPHERAL ADDRESS DEFINITIONS
 * @{
 */

/*! Start address of UART device. */
#define UART_BASEADDR CONFIG_STDINOUT_BASEADDR

#ifndef CONFIG_NO_FLASH
/*! Start address of FLASH device */
#define FLASH_BASE      CONFIG_XILINX_FLASH_START
/*! End address of FLASH device */
#define FLASH_END       (FLASH_BASE + FLASH_SIZE)
#endif

/*! Start address of SDRAM device */
#define RAM_START       CONFIG_XILINX_ERAM_START
/*! End address of SDRAM device */
#define RAM_END         (RAM_START + RAM_SIZE - 1)

/*! @} */

/* -FIXME
 * This is taken from the MTD Mapping for ML401 board
 * so we remember to synchronise it with the MTD mappings
 */
/*! Offset from FLASH base to the location of bootloader partition */
/* Allow possibility it's defined on the gcc command line with -D, so
   hardware projects can place u-boot at different locations in flash */
#ifndef CONFIG_FS_BOOT_OFFSET
#define CONFIG_FS_BOOT_OFFSET   0
#endif

/*! Start address in FLASH of the 2nd Stage bootloader */
#ifndef CONFIG_NO_FLASH
#ifdef CONFIG_PRIMARY_FLASH_SPI
#define CONFIG_FS_BOOT_START	CONFIG_FS_BOOT_OFFSET
#else
#define CONFIG_FS_BOOT_START	FLASH_BASE + CONFIG_FS_BOOT_OFFSET
#endif
/*! Maximum size of the 2nd Stage bootloader partition in FLASH */
#define CONFIG_FS_BOOT_MAXSIZE  (256 * 1024)
/*! End address in FLASH of the 2nd Stage bootloader */
#define CONFIG_FS_BOOT_END	CONFIG_FS_BOOT_START + CONFIG_FS_BOOT_MAXSIZE
#endif

/*
 * GLOBAL FUNCTION PROTOTYPES
 */
void fsprint(char *s);
