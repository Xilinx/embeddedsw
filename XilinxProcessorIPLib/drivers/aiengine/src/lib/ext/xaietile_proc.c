/******************************************************************************
* (c) Copyright 2018 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information
* of Xilinx, Inc. and is protected under U.S. and
* international copyright and other intellectual property
* laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any
* rights to the materials distributed herewith. Except as
* otherwise provided in a valid license issued to you by
* Xilinx, and to the maximum extent permitted by applicable
* law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
* WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
* AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
* BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
* INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
* (2) Xilinx shall not be liable (whether in contract or tort,
* including negligence, or under any other theory of
* liability) for any loss or damage of any kind or nature
* related to, arising under or in connection with these
* materials, including for any direct, or any indirect,
* special, incidental, or consequential loss or damage
* (including loss of data, profits, goodwill, or any type of
* loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the
* possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-
* safe, or for use in any application requiring fail-safe
* performance, such as life-support or safety devices or
* systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any
* other applications that could lead to death, personal
* injury, or severe property or environmental damage
* (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and
* liability of any use of Xilinx products in Critical
* Applications, subject only to applicable laws and
* regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
* PART OF THIS FILE AT ALL TIMES.
******************************************************************************/

/*****************************************************************************/
/**
* @file xaietile_proc.c
* @{
*
* This file contains the API for AIE tile processor.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Hyun    08/17/2018  Initial creation
* 1.1  Hyun    10/15/2018  Don't start the remoteproc in elfloading to allow
*                          multiple elfloading.
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openamp/remoteproc.h>
#include <openamp/remoteproc_loader.h>

#include <metal/alloc.h>
#include <metal/device.h>
#include <metal/io.h>
#include <metal/mutex.h>
#include <metal/utilities.h>

#include "xaieio.h"
#include "xaietile_proc.h"

/************************** Constant Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XAieGbl_Config XAieGbl_ConfigTable[];

/************************** Function Prototypes  *****************************/

/************************** Function Definitions *****************************/

/*
 * remoteproc operations.
 */

static struct remoteproc *xaietile_proc_init(struct remoteproc *rproc,
		struct remoteproc_ops *ops, void *arg)
{
	rproc->priv = arg;
	rproc->ops = ops;
	return rproc;
}

static void xaietile_proc_remove(struct remoteproc *rproc)
{
}

static int xaietile_proc_start(struct remoteproc *rproc)
{
	return 0;
}

static int xaietile_proc_stop(struct remoteproc *rproc)
{
	return 0;
}

static int xaietile_proc_shutdown(struct remoteproc *rproc)
{
	struct metal_list *node;
	struct remoteproc_mem *mem;

	metal_list_for_each(&rproc->mems, node) {

		mem = metal_container_of(node, struct remoteproc_mem, node);
		metal_free_memory(mem->io);
		metal_free_memory(mem);
	}

	return 0;
}

/*
 * This is a key function to load an elf. Main functionality is to translate
 * addresses in the elf to the device / tile addresses.
 */
static void *xaietile_proc_mmap(struct remoteproc *rproc,
		metal_phys_addr_t *pa, metal_phys_addr_t *da,
		size_t size, unsigned int attribute,
		struct metal_io_region **io)
{
	XAieGbl_Tile *TileInstPtr = (XAieGbl_Tile *)rproc->priv;
	metal_phys_addr_t lpa, lda;
	s32 row, col;
	u64 tileaddr;

	if (!da || !pa)
		return NULL;

	lda = *da;
	lpa = *pa;

	row = TileInstPtr->RowId;
	col = TileInstPtr->ColId;

	/* This lpa should be updated in one of if statements below */
	lpa = METAL_BAD_PHYS;
	if (lda < 0x20000) {
		/* program memory */
		lpa = TileInstPtr->TileAddr + XAIEGBL_CORE_PRGMEM + lda;
	} else if (size < 0x8000) {
		/*
		 * data memory: Each memory is 32KB (0x8000)
		 * South starting from 0x20000
		 * West starting from 0x28000
		 * North starting from 0x30000
		 * East starting from 0x38000
		 */
		if (lda < 0x27fff) {
			/* south */
			row--;
			if (row < 1)
				return NULL;
			lda -= 0x20000;
		} else if (lda < 0x2ffff) {
			s32 parity;

			/*
			 * west: Handle checkerboard memory directions.
			 * The West memory is located in left in even row.
			 */
			parity = row % 2 ? -1 : 0;
			col += parity;
			if (col < 0 || col >= XAieGbl_ConfigTable->NumCols)
				return NULL;
			lda -= 0x28000;
		} else if (lda < 0x37fff) {
			/* north */
			row++;
			if (row > XAieGbl_ConfigTable->NumRows)
				return NULL;
			lda -= 0x30000;
		} else if (lda < 0x3ffff) {
			s32 parity;

			/*
			 * east: Handle checkerboard memory directions.
			 * The East memory is located in right in odd row.
			 */
			parity = row % 2 ? 0 : 1;
			col += parity;
			if (col < 0 || col >= XAieGbl_ConfigTable->NumCols)
				return NULL;
			lda -= 0x38000;
		}
		tileaddr = TileInstPtr->TileAddr;
		tileaddr &= ~(uint64_t)XAIEGBL_TILE_BASE_ADDRMASK;
		tileaddr |= col << XAIEGBL_TILE_ADDR_COL_SHIFT;
		tileaddr |= row << XAIEGBL_TILE_ADDR_ROW_SHIFT;
		lpa = tileaddr + lda;
	}

	if (lpa == METAL_BAD_PHYS)
		return NULL;

	*pa = lpa;
	*da = lda;
	*io = (struct metal_io_region *)_XAieIO_GetIO();
	return metal_io_phys_to_virt(*io, *pa);
}

static int xaietile_proc_config(struct remoteproc *rproc, void *data)
{
	return 0;
}

static struct remoteproc_ops xaietile_proc_ops = {
        .init		= xaietile_proc_init,
        .remove		= xaietile_proc_remove,
        .start		= xaietile_proc_start,
        .stop		= xaietile_proc_stop,
        .shutdown	= xaietile_proc_shutdown,
        .mmap		= xaietile_proc_mmap,
        .config		= xaietile_proc_config,
};

/*
 * Internal store operations to load the elf from a file or memory.
 */

struct elf_store {
	FILE *file;
	void *elf;
};

static int xaietile_store_load(void *store, size_t offset, size_t size,
		const void **data, metal_phys_addr_t pa,
		struct metal_io_region *io, char is_blocking)
{
	struct elf_store *elf_store = (struct elf_store *)store;
	const void *elf = elf_store->elf;

	(void)is_blocking;

	if (pa == METAL_BAD_PHYS) {
		if (data == NULL) {
			XAieLib_print("%s: data is NULL while pa is ANY\r\n",
				__func__);
			return -EINVAL;
		}
		*data = (const void *)((const char *)elf + offset);
	} else {
		void *va;
		unsigned int i;

		if (io == NULL) {
			XAieLib_print("%s, io is NULL while pa is not ANY\r\n",
				__func__);
			return -EINVAL;
		}
		va = metal_io_phys_to_virt(io, pa);
		if (va == NULL) {
			XAieLib_print("%s: no va is found\r\n", __func__);
			return -EINVAL;
		}

		/* FIXAIE: memcpy to registers doesn't work for some reason */
		/*memcpy(va, (const void *)((const char *)elf + offset),
		       (size + 3) & ~3);*/

		for (i = 0; i < size; i += 4)
			*(u32 *)((u64)va + i) = *(u32 *)((u64)elf + offset + i);
	}

	return size;
}

/*
 * FIXAIE: This is to workaround the incorrect bss sections in AIE elf.
 * The section holds some memory size with invalid address, and accordingly
 * to the elf specification, the address should be cleared upto memory size.
 * The below workaround works this around by fixing the memory size in place
 * of elf content in memory, @elf, to 0.
 */
static void xaietile_store_workaround(char *elf)
{
	Elf32_Ehdr *ehdr;
	Elf32_Shdr *shdr;
	Elf32_Shdr *strtab = NULL;
	Elf32_Phdr *phdr;
	unsigned int i;

	ehdr = (Elf32_Ehdr *)elf;
	shdr = (Elf32_Shdr *)(elf + ehdr->e_shoff);

	/* Fix the program header */
	phdr = (Elf32_Phdr *)((char *)elf + ehdr->e_phoff);
	for (i = 0; i < ehdr->e_phnum; i++) {
		/*
		 * The compiler incorrectly populates some bss sections with
		 * file size = 0, intending it's not written into memory.
		 * So in case of file size = 0, fix the memory size to 0
		 * in place.
		 */
		if (!phdr[i].p_filesz)
			phdr[i].p_memsz = 0;
	}

	/*
	 * Update the section header accordingly, even though this is not
	 * required.
	 */
	for (i = 0; i < ehdr->e_shnum; i++) {
		if (shdr[i].sh_type == SHT_STRTAB) {
			char *shname, *match = ".shstrtab";

			shname = (char *)elf + shdr[i].sh_offset +
				shdr[i].sh_name;
			if (strncmp(match, shname, sizeof(match)))
				continue;

			strtab = &shdr[i];
		}
	}

	if (!strtab)
		return;

	for (i = 0; i < ehdr->e_shnum; i++) {
		char *shname, *match = ".bss";

		/* bss sections without DMb in its name needs to be fixed */
		shname = (char *)elf + strtab->sh_offset + shdr[i].sh_name;
		if (strncmp(match, shname, sizeof(match)))
			continue;
		match = ".bss.DMb";
		if (!strncmp(match, shname, sizeof(match)))
			continue;
		shdr[i].sh_size = 0;
	}
}

static int xaietile_file_store_open(void *store, const char *path,
		const void **image_data)
{
	struct elf_store *elf_store = (struct elf_store *)store;
	FILE *file;
	int size;
	void *elf;

        file = fopen(path, "r");
	if (!file) {
		XAieLib_print("failed to open the elf file: %s\n", path);
		return -EINVAL;
	}
	elf_store->file = file;

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
	elf = metal_allocate_memory(size + 1);
	if (!elf) {
		XAieLib_print("failed to allocate a memory for elf\n");
		return -ENOMEM;
	}
	fread(elf, size, 1, file);
	elf_store->elf = elf;
	*image_data = elf;

	xaietile_store_workaround(elf);

	return size;
}

static void xaietile_file_store_close(void *store)
{
	struct elf_store *elf_store = (struct elf_store *)store;

	metal_free_memory(elf_store->elf);
	fclose(elf_store->file);
}

struct image_store_ops xaietile_file_store_ops = {
	.open		= xaietile_file_store_open,
	.close		= xaietile_file_store_close,
	.load		= xaietile_store_load,
	.features	= SUPPORT_SEEK,
};

/*
 * Internal store operations to load the elf from memory.
 */

static int xaietile_mem_store_open(void *store, const char *elf,
		const void **image_data)
{
	struct elf_store *elf_store = (struct elf_store *)store;

	if (!elf) {
		XAieLib_print("failed to allocate a memory for elf\n");
		return -EINVAL;
	}
	elf_store->elf = elf;
	*image_data = elf;

	xaietile_store_workaround((void *)elf);

	/* Return arbitrary size. The remoteproc elf load takes care of it*/
	return 0x100;
}

static void xaietile_mem_store_close(void *store)
{
}

static struct image_store_ops xaietile_mem_store_ops = {
	.open		= xaietile_mem_store_open,
	.close		= xaietile_mem_store_close,
	.load		= xaietile_store_load,
	.features	= SUPPORT_SEEK,
};

/*****************************************************************************/
/**
*
* This is the internal API to load the specified ELF to the target AIE Tile
* program memory through remoteproc APIs.
*
* @param	TileInstPtr - Pointer to the Tile instance structure.
* @param	ElfPtr: Pointer to elf. Can be a file path or memory.
* @param	store_ops: Image store operations.
*
* @return	XAIELIB_SUCCESS on success, else XAIELIB_FAILURE.
*
* @note		None.
*
*******************************************************************************/
static u32 xaietileproc_load(XAieGbl_Tile *TileInstPtr, u8 *ElfPtr,
		void *store, struct image_store_ops *store_ops)
{
	struct remoteproc *rproc = (struct remoteproc *)TileInstPtr->Private;
	int err;

	err = remoteproc_load(rproc, ElfPtr, store, store_ops, NULL);
	if (err) {
		XAieLib_print("failed to remoteproc_load()\n");
		return XAIELIB_FAILURE;
	}

	/*
	 * TODO: For proper life cycle management, the remoteproce instance
	 * should be started and stopped with support at the driver level.
	 */

	return XAIELIB_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the tile processor function to load an elf from a file
*
* @param	TileInstPtr: Tile instance pointer of common AIE driver
* @param	ElfPtr: a path to an elf file
* @param	LoadSym: UNUSED. Kept for compatibility
*
* @return	XAIELIB_SUCCESS on success, otherwise XAIELIB_FAILURE. The return
* code is also passed from xaietileproc_load().
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileProc_LoadElfFile(XAieGbl_Tile *TileInstPtr, u8 *ElfPtr, u8 LoadSym)
{
	struct elf_store *elf_store;
	u32 ret;

	elf_store = metal_allocate_memory(sizeof(*elf_store));
	if (!elf_store) {
		XAieLib_print("failed to allocae a memory()\n");
		return XAIELIB_FAILURE;
	}

	ret = xaietileproc_load(TileInstPtr, ElfPtr, elf_store,
			&xaietile_file_store_ops);
	if (ret) {
		metal_free_memory(elf_store);
		return ret;
	}

	return XAIELIB_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the tile processor function to load an elf from memory
*
* @param	TileInstPtr: Tile instance pointer of common AIE driver
* @param	ElfPtr: pointer to elf in memory
* @param	LoadSym: UNUSED. Kept for compatibility
*
* @return	XAIELIB_SUCCESS on success, otherwise XAIELIB_FAILURE. The return
* code is also passed from xaietileproc_load().
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileProc_LoadElfMem(XAieGbl_Tile *TileInstPtr, u8 *ElfPtr, u8 LoadSym)
{
	struct elf_store *elf_store;
	u32 ret;

	elf_store = metal_allocate_memory(sizeof(*elf_store));
	if (!elf_store) {
		XAieLib_print("failed to allocae a memory()\n");
		return XAIELIB_FAILURE;
	}

	ret = xaietileproc_load(TileInstPtr, ElfPtr, elf_store,
			&xaietile_mem_store_ops);
	if (ret) {
		metal_free_memory(elf_store);
		return ret;
	}

	return XAIELIB_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the tile processor function to free the remoteproc instance
*
* @param	TileInstPtr: Tile instance pointer of common AIE driver
*
* @return	XAIELIB_SUCCESS on success, otherwise XAIELIB_FAILURE.
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileProc_Finish(XAieGbl_Tile *TileInstPtr)
{
	struct remoteproc *rproc = (struct remoteproc *)TileInstPtr->Private;

	/* Better to call stop when disabling in tile core control */
	remoteproc_stop(rproc);
	remoteproc_shutdown(rproc);
	TileInstPtr->Private = NULL;
	metal_free_memory(rproc);

	return XAIELIB_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the tile processor function to initialize the remoteproc instance.
*
* @param	TileInstPtr: Tile instance pointer of common AIE driver
*
* @return	XAIELIB_SUCCESS on success, otherwise XAIELIB_FAILURE.
*
* @note		None.
*
*******************************************************************************/
u32 XAieTileProc_Init(XAieGbl_Tile *TileInstPtr)
{
	struct remoteproc *rproc, *ret_rproc;

	rproc = metal_allocate_memory(sizeof(*rproc));
	if (!rproc) {
		XAieLib_print("failed to allocae a memory for rproc\n");
		return XAIELIB_FAILURE;
	}

	ret_rproc = remoteproc_init(rproc, &xaietile_proc_ops, TileInstPtr);
	if (ret_rproc != rproc) {
		XAieLib_print("failed to remoteproc_init()\n");
		goto metal_free_memory;
	}

	remoteproc_config(rproc, NULL);
	TileInstPtr->Private = rproc;

	return XAIELIB_SUCCESS;

metal_free_memory:
	metal_free_memory(rproc);
	return XAIELIB_FAILURE;
}

/** @} */
