/******************************************************************************
* Copyright (c) 2013 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
* @file fs-boot.c
*
* DESCRIPTION:
*     This is the main program for the first-stage bootloader FS-BOOT for the
*     PetaLinux distribution.
*
*     This bootloader is targeted for reconfigurable platform and is designed
*     to be run from BRAM.  Hence, elf size must remain below 8K bytes.
*
*     It supports the booting of any second-stage bootloader from
*     FLASH or RAM memory.
*
*     Note: This program requires the following hardware support:
*           - XuartLite or uart16550
*           - BRAM >= 8Kb
*
*******************************************************************************/
#include <xenv_standalone.h>
#include "fs-boot.h"
#ifdef CONFIG_PRIMARY_FLASH_SPI
#include "fs-xspi.h"
#endif

/* Stubbed out version of this _exit hook, reduce code size significantly */
void __call_exitprocs(void)
{
}

/* Stubbed out version of this vector, reduce code size significantly */
void __interrupt_handler(void) __attribute__ ((interrupt_handler));
void __interrupt_handler(void)
{
}

/* Stubbed out version of this vector, reduce code size significantly */
void _hw_exception_handler(void)
{
}

/*
 * Magic words corresponding to the first instructions of the
 * bootstub routine in petalinux-reloc-blob.
 * This allows us to check if a valid u-boot image is in FLASH.
 * Note: any changes to the bootstub code will need to update this.
 */
#ifdef __PPC__
#define RELOC_MAGIC 0x7c0004ac
#elif __MICROBLAZE__
#define RELOC_MAGIC 0xb8b40008
#else
#error "Unknown architecture"
#endif

/* Get the program (text) start address */
#ifdef __MICROBLAZE__
extern void *_start;
#endif

#ifdef DEBUG
static char *inttohex(int n, char *s) {
	static char digits[] = "0123456789abcdef";
	int i;

	s[10] = '\0';

	for(i = 9 ; i >= 2; i--) {
		s[i] = digits[n & 0xF];
		n = n >> 4;
	}
	return s;
}

int debug_fsprint_integer(char *prestr, char *subfixstr, unsigned integer)
{
	char s[10] = "0x";
	fsprint(prestr);
	fsprint(inttohex(integer, s));
	fsprint(subfixstr);
	return 0;
}
#endif

/*! \brief Macro for Jump Instruction */
#ifdef __PPC__
static void GO(unsigned long addr)
{
	/* Xilinx has put us into IS=0, DS=1 mode. We should really be booting
	 * in IS=0, DS=0 mode.  */
	asm volatile ("mtsrr0   %0\n"
		      "mtsrr1   %1\n"
		      "rfi\n" : : "r" (addr), "r" (0));
}
#else
 #define GO(addr) { ((void(*)(void))(addr))(); }
#endif


/*! \brief Macro for boot wrapper size */
#define BOOT_WRAPPER_SIZE 0x10C
#define BOOT_WRAPPER_ADDROFFSET 0x100

#ifdef CONFIG_UARTLITE
#ifdef XUartLite_SetControlReg
#define uartlite_set_controlreg(base,mask) \
				XUartLite_SetControlReg(base, mask)
#define uartlite_is_recv_empty(base) XUartLite_IsReceiveEmpty(base)
#define uartlite_is_trans_full(base) XUartLite_IsTransmitFull(base)
#else
#define uartlite_set_controlreg(base,mask) \
				XUartLite_mSetControlReg(base, mask)
#define uartlite_is_recv_empty(base) XUartLite_mIsReceiveEmpty(base)
#define uartlite_is_trans_full(base) XUartLite_mIsTransmitFull(base)
#endif
#elif CONFIG_UART16550
#ifdef XUartNs550_SetLineControlReg
#define uartns550_set_linecontrolreg(base, mask) \
				XUartNs550_SetLineControlReg(base, mask)
#define uartns550_is_recv_data(base) XUartNs550_IsReceiveData(base)
#define uartns550_read_reg(base, offset) XUartNs550_ReadReg(base, offset)
#else
#define uartns550_set_linecontrolreg(base, mask) \
				XUartNs550_mSetLineControlReg(base, mask)
#define uartns550_is_recv_data(base) XUartNs550_mIsReceiveData(base)
#define uartns550_read_reg(base, offset) XUartNs550_mReadReg(base, offset)
#endif
#endif


/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

/*!
 * Run initialisation code to setup the UART for communication.
 * For UARTLITE, the bulk of the configuration is done in hardware.
 *
 * @param  None.
 *
 * @return  None.
 */
#ifdef CONFIG_UARTLITE
static void uart_init(void)
{
	/* All mode and baud setup is done in hardware level */
	/* Reset FIFO and Enable Interrupt */
	uartlite_set_controlreg(UART_BASEADDR, (XUL_CR_ENABLE_INTR    |
						XUL_CR_FIFO_RX_RESET  |
						XUL_CR_FIFO_TX_RESET));
}
#elif CONFIG_UART16550
static void uart_init(void)
{
	XUartNs550_SetBaud(UART_BASEADDR, XPAR_UARTNS550_0_CLOCK_FREQ_HZ, 115200);
	uartns550_set_linecontrolreg(UART_BASEADDR, XUN_LCR_8_DATA_BITS);
}
#endif

/*!
 * Sends a single character to the uart fifo.
 *
 * @param  data - The character to send
 *
 * @return  None.
 */
#ifdef CONFIG_UARTLITE
void put_ch(unsigned char data)
{
	while (uartlite_is_trans_full(UART_BASEADDR));
	XUartLite_SendByte(UART_BASEADDR, data);

	return;
}
#elif CONFIG_UART16550
void put_ch (unsigned char data)
{
	XUartNs550_SendByte(UART_BASEADDR, data);
	return;
}
#endif

#ifndef CONFIG_NO_FLASH
#ifndef CONFIG_PRIMARY_FLASH_SPI
/*!
 * This routine send a command to the CUI
 * to put the FLASH into Read Array mode.
 *
 * @param  addr - Memory address of FLASH device
 *
 * @return  none
 */
static void flash_readarray_mode(unsigned long flash_addr)
{

	/*
	 * We assume it is all CFI FLASH
	 * Write 32-bit to take care off all flash
	 * configurations.
	 */
	/*
	 * The axi_emc_v1_01_a core is not able to response/react to the
	 * subsequent read operation immediately while it is busy
	 * performing write operation ( e.g. four 8-bit flash write or
	 * 2 16-bit flash write ).
	 *
	 * The temporary workaround is issue four 8bit bus write instead
	 * of a single 32bit bus write. This is temporary solution until
	 * axi_emc is fixed.
	 */
	*((volatile unsigned char *)(flash_addr)) = 0xFF;
	*((volatile unsigned char *)((flash_addr)+1)) = 0xFF;
	*((volatile unsigned char *)((flash_addr)+2)) = 0xFF;
	*((volatile unsigned char *)((flash_addr)+3)) = 0xFF;
}

static int image_exist(unsigned long image_addrflash)
{
	unsigned long *addr;

	flash_readarray_mode(image_addrflash);

	addr = (unsigned long *)image_addrflash;

	if (*addr != RELOC_MAGIC) {
		return REASON_BAD_MAGIC;
	}
	return 0;
}

static unsigned long copy_image_from_flash(unsigned long image_addrflash)
{
	volatile unsigned long *mem = (volatile unsigned long *)(image_addrflash + BOOT_WRAPPER_ADDROFFSET);
	volatile unsigned char *image_ptr = 0, *image_ptr_save;
	volatile unsigned char *image_flash = (volatile unsigned char *)(image_addrflash + BOOT_WRAPPER_SIZE);
	volatile unsigned jump_offset=0;
	int image_size = 0, image_size_save=0;

	flash_readarray_mode(image_addrflash + BOOT_WRAPPER_ADDROFFSET);
	image_ptr = (volatile unsigned char *)(*mem);
	mem++;
	image_size = (int)(*mem);
	jump_offset = (unsigned)(*(mem+1));
	image_size_save = image_size;
	image_ptr_save = image_ptr;

#ifdef DEBUG
	debug_fsprint_integer("image_src: ", "\r\n", (unsigned)image_flash);
	debug_fsprint_integer("image_ptr: ", "\r\n", (unsigned)image_ptr);
	debug_fsprint_integer("image_size: ", "\r\n", image_size);
	debug_fsprint_integer("jump_offset: ", "\r\n", (unsigned)jump_offset);
	debug_fsprint_integer("RAM_START: ", "\r\n", (unsigned)RAM_START);
	debug_fsprint_integer("RAM_END: ", "\r\n", (unsigned)RAM_END);
	debug_fsprint_integer("RAM_SIZE: ", "\r\n", (unsigned)RAM_SIZE);
#endif

	if (image_size == 0) {
		fsprint("FS-BOOT: ERROR: Image size is 0.");
		return REASON_BAD_ADDRESS;
	}
	if ((unsigned long)image_ptr < RAM_START) {
		fsprint("FS-BOOT: ERROR: Image start address will be below memory start address.\r\n");
		return REASON_BAD_ADDRESS;
	}
	if ((unsigned long)(image_ptr + image_size) >= RAM_END) {
		fsprint("FS-BOOT: ERROR: Image will be outside the memory.\r\n");
		return REASON_BAD_ADDRESS;
	}

	flash_readarray_mode((unsigned long)image_flash);
	for(; image_size > 0; image_size--) {
		*image_ptr = *image_flash;
		image_ptr++;
		image_flash++;
	}
#ifdef DEBUG
	debug_fsprint_integer("flushing cache(", ",", (unsigned) image_ptr_save);
	debug_fsprint_integer("", ")\r\n", image_size_save);
#endif
	XCACHE_FLUSH_DCACHE_RANGE(image_ptr_save, image_size_save);
	XCACHE_INVALIDATE_ICACHE();
	return (unsigned long) (image_ptr_save + jump_offset);
}

#else /* Boot from SPI FLASH */
static int image_exist(unsigned long image_addrflash)
{
	volatile unsigned long mem;
	int i;

	if (spi_flash_probe()) {
		fsprint("FS-BOOT: ERROR: Failed to probe SPI FLASH.\r\n");
		return REASON_FLASH_FAIL;
	}

	/* Get wrapper from SPI FLASH */
	/* Retry for a couple times in case the FLASH is not ready */
	i=10;
	while(i-- && (mem != RELOC_MAGIC)) {
		spi_flash_read(image_addrflash, (unsigned char *)(&mem), 4, 1, 1);
	}

	if (mem == RELOC_MAGIC) {
		return 0;
	}

	return REASON_BAD_MAGIC;
}

static unsigned long copy_image_from_flash(unsigned long image_addrflash)
{
	volatile unsigned long mem[2];
	volatile unsigned char *image_ptr = 0;
	volatile int image_size = 0;

	/* Get image ram address and image size information from SPI FLASH */
	spi_flash_read((image_addrflash + BOOT_WRAPPER_ADDROFFSET), (volatile unsigned char *)mem, 8, 1, 1);
	image_ptr = (volatile unsigned char *)(*mem);
	image_size = (volatile int)(*(mem + 1));

	if (image_size == 0) {
		fsprint("FS-BOOT: ERROR: Image size is 0.");
		return REASON_BAD_ADDRESS;
	}
	if ((unsigned long)image_ptr < RAM_START) {
		fsprint("FS-BOOT: ERROR: Image start address will be below memory start address.\r\n");
		return REASON_BAD_ADDRESS;
	}
	if ((unsigned long)(image_ptr + image_size) >= RAM_END) {
		fsprint("FS-BOOT: ERROR: Image will be outside the memory.\r\n");
		return REASON_BAD_ADDRESS;
	}

	if (spi_flash_read((image_addrflash + BOOT_WRAPPER_SIZE), image_ptr, image_size, 1, 1) != image_size) {
		fsprint("FS-BOOT: ERROR: Failed to load image from FLASH to memory.\r\n");
		return REASON_BAD_ADDRESS;
	}
	XCACHE_FLUSH_DCACHE_RANGE(image_ptr, image_size);
	XCACHE_INVALIDATE_ICACHE();
	return *mem;

}

#endif /* SPI FLASH or Parallel FLASH */
#endif /* Where FLASH is configured */

/*!
 * Lightweight print function to avoid using stdio.
 *
 * @param  s - The string to print.
 *
 * @return  None.
 */
void fsprint(char *s)
{
	while (*s) {
		put_ch(*s);
		s++;
	}
}

/*---------------------------------------------------------------------------*/

int main()
{
#ifndef CONFIG_NO_FLASH
	unsigned long image_addrflash = 0;   /* The address of the boot image in FLASH */
#endif
	unsigned long image_start = 0;    /* The address of the final boot image in memory */
	int failed_reason;

#ifdef __MICROBLAZE__
	int cpu;
	int pvr;
#endif

	XCACHE_DISABLE_CACHE();

	/* Call any early platform init handler provided by the user.
	 * Be careful - no stdio is available yet */
	if(__fs_preinit)
		__fs_preinit();

	/* UART Initialisation - no printing before this */
	uart_init();

#ifdef __MICROBLAZE__
	pvr = mfmsr() & 0x400;
        cpu = mfpvr(0) & 0xff;
        if (pvr != 0 && cpu > 0) {
		/* Sleep until IPI #0 issued by CPU 0 */
		mb_sleep();

		/* Jump to kernel */
		GO(XPAR_MICROBLAZE_ICACHE_BASEADDR);

		/* Shouldn't return */
		while(1)
		  ;
	}
#endif

	fsprint("FS-BOOT First Stage Bootloader (c) 2013-2014 Xilinx Inc.\r\n" \
	"Build date: "__DATE__" "__TIME__ "  "
	"\r\nSerial console: "
#ifdef CONFIG_UARTLITE
	"Uartlite\r\n"
#elif CONFIG_UART16550
	"Uart16550\r\n"
#endif
	);

#ifndef CONFIG_NO_FLASH
	/* Set the default bootloader boot parameters */
	image_addrflash = CONFIG_FS_BOOT_START;

	failed_reason = image_exist(image_addrflash);
	if (!failed_reason) /* failed_reason = 0 means NO reason to fail */
		fsprint("FS-BOOT: Booting from FLASH.\r\n");
	else
		fsprint("FS-BOOT: No existing image in FLASH.\r\n");
#else
	fsprint("FS-BOOT: FLASH is not configured.\r\n");
	failed_reason = REASON_BAD_ADDRESS;
#endif
	if (failed_reason > 0) {
		fsprint("FS-BOOT: Please download the image with JTAG.\r\n");
		BAD_IMAGE(failed_reason) /* Additional 40B */
	}

#ifndef CONFIG_NO_FLASH
	image_start = copy_image_from_flash(image_addrflash);
#endif
	if (image_start == REASON_BAD_ADDRESS) {
		BAD_IMAGE(image_start) /* Additional 40B */
	}

#ifdef DEBUG
	debug_fsprint_integer("Jumping to ", "\r\n", (unsigned)image_start);
#endif
	/*
	 * Call any preboot handler provided by user.  Non-zero return value
	 * means DON'T BOOT.  We do this silently to save memory space, the
	 * __fs_preboot can use fsprint() etc to output messages if required */
	if (__fs_preboot) {
		if(__fs_preboot(image_start))
			;
	}

	GO(image_start);

	/* Shouldn't return */
	while(1)
		;
}
