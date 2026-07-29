/******************************************************************************
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x40000;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x1000000;

_EL0_STACK_SIZE = DEFINED(_EL0_STACK_SIZE) ? _EL0_STACK_SIZE : 1024;
_EL1_STACK_SIZE = DEFINED(_EL1_STACK_SIZE) ? _EL1_STACK_SIZE : 2048;
_EL2_STACK_SIZE = DEFINED(_EL2_STACK_SIZE) ? _EL2_STACK_SIZE : 1024;

MEMORY
{
	load_calib : ORIGIN =0xF800000 , LENGTH = 0x28000
	  mailbox : ORIGIN = 0x1E0A8000, LENGTH = 0x400000	 /*Reserve this memory for APU<->RPU communication*/
	  /* Tuning-database blob, pre-loaded by an external loader and read by
	   * t_database.c (APU_TDATABASE_BIN_ADDR). Lives in the free gap between the
	   * mailbox end (0x1E4A8000) and the DDR_LOW_0 ORIGIN (0x20000000), so it is
	   * never overlapped by the linker-managed region. */
	  tdatabase_bin : ORIGIN = 0x1F000000, LENGTH = 0x100000
	/* NoC_C0_C1_ddr_memory_DDR_LOW_0 : ORIGIN = 0x12000000, LENGTH = 0x10000000*/
	/* Start above the firmware/RPU-reserved bottom of DDR (load_calib @0xF800000,
	 * mailbox @0x1E0A8000, TF-A/PLM, and the tuning-DB blob @0x1F000000 loaded by
	 * t_database.c). LENGTH caps the region at the 2GB boundary
	 * (0x20000000 + 0x60000000 = 0x80000000) so it never overruns DDR_LOW_0.
	 * The larger size is required when the ISP buffer pools are placed in
	 * DDR_LOW_0 (low / 32-bit mode); see BUFFER_POOL_REGION switch below. */
   NoC_C0_C1_ddr_memory_DDR_LOW_0 : ORIGIN = 0x20000000, LENGTH = 0x60000000
	NoC_C0_C1_ddr_memory_DDR_LOW_1 : ORIGIN = 0x800000000, LENGTH = 0x80000000
	NoC_C2_ddr_memory_DDR_CH_1 : ORIGIN = 0x50000000000, LENGTH = 0x80000000
	NoC_C3_ddr_memory_DDR_CH_1A : ORIGIN = 0x60000000000, LENGTH = 0x80000000
	NoC_C4_ddr_memory_DDR_CH_2 : ORIGIN = 0x70000000000, LENGTH = 0x80000000
	ps_wizard_0_ps11_0_ocm_ram_0_memory_0 : ORIGIN = 0xbbf00000, LENGTH = 0x80000
	ps_wizard_0_ps11_0_ocm_ram_1_memory_1 : ORIGIN = 0xbbf80000, LENGTH = 0x80000
	ps_wizard_0_ps11_0_ocm_ram_2_memory_2 : ORIGIN = 0xbbe00000, LENGTH = 0x80000
	ps_wizard_0_ps11_0_ocm_ram_3_memory_3 : ORIGIN = 0xbbe80000, LENGTH = 0x80000
	axi_bram_ctrl_0_memory_0 : ORIGIN = 0xb0000000, LENGTH = 0x2000
}

/* ===========================================================================
 * ISP frame-buffer pool memory selection  --  SINGLE SWITCH
 *
 * Pick exactly ONE region below for the ISP frame-buffer pools. Everything
 * else auto-derives from the high 32 bits of __large_pool_mem_start:
 *   - the RPU high-mem prefix sent by SetATM() and programmed into the HW DMA
 *   - the is_64bit flag sent to the RPU
 *   - the software ATM-enable used for address reconstruction
 * so no C code needs to change when you switch regions.
 *
 *   DDR_LOW_0 : low / 32-bit memory, prefix 0x0.   Reachable by 32-bit cores
 *               (Cortex-R52 / AArch32). is_64bit = 0, ATM off. (default)
 *   DDR_LOW_1 : high memory @0x8_00000000,   prefix 0x8.   64-bit core only.
 *               is_64bit = 1, ATM on.
 * To switch, keep exactly ONE line below uncommented (GNU ld rejects the same
 * alias name being defined twice, so do not enable more than one).
 * ===========================================================================*/
REGION_ALIAS("BUFFER_POOL_REGION", NoC_C0_C1_ddr_memory_DDR_LOW_0);   /* Low / 32-bit (default), prefix 0x0   */
/* REGION_ALIAS("BUFFER_POOL_REGION", NoC_C0_C1_ddr_memory_DDR_LOW_1);    High memory DDR_LOW_1,   prefix 0x8   */

/* Specify the default entry point to the program */

ENTRY(_vector_table)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.text : {
   KEEP (*(.vectors))
   *(.boot)
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
   *(.plt)
   *(.gnu_warning)
   *(.gcc_execpt_table)
   *(.glue_7)
   *(.glue_7t)
   *(.ARM.extab)
   *(.gnu.linkonce.armextab.*)
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > NoC_C0_C1_ddr_memory_DDR_LOW_0


.init (ALIGN(64)) : {
   KEEP (*(.init))
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.fini (ALIGN(64)) : {
   KEEP (*(.fini))
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.interp : {
   KEEP (*(.interp))
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.note-ABI-tag : {
   KEEP (*(.note-ABI-tag))
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.rodata : {
   . = ALIGN(64);
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.rodata1 : {
   . = ALIGN(64);
   __rodata1_start = .;
   *(.rodata1)
   *(.rodata1.*)
   __rodata1_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.sdata2 : {
   . = ALIGN(64);
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   __sdata2_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.sbss2 : {
   . = ALIGN(64);
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.data : {
   . = ALIGN(64);
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   *(.jcr)
   *(.got)
   *(.got.plt)
   __data_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.data1 : {
   . = ALIGN(64);
   __data1_start = .;
   *(.data1)
   *(.data1.*)
   __data1_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.got : {
   *(.got)
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.got1 : {
   *(.got1)
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.got2 : {
   *(.got2)
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.ctors : {
   . = ALIGN(64);
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.dtors : {
   . = ALIGN(64);
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   __DTOR_END__ = .;
   ___DTORS_END___ = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.fixup : {
   __fixup_start = .;
   *(.fixup)
   __fixup_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.eh_frame : {
   *(.eh_frame)
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.eh_framehdr : {
   __eh_framehdr_start = .;
   *(.eh_framehdr)
   __eh_framehdr_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.gcc_except_table : {
   *(.gcc_except_table)
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.mmu_tbl0 (ALIGN(4096)) : {
   __mmu_tbl0_start = .;
   *(.mmu_tbl0)
   __mmu_tbl0_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.mmu_tbl1 (ALIGN(4096)) : {
   __mmu_tbl1_start = .;
   *(.mmu_tbl1)
   __mmu_tbl1_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.mmu_tbl2 (ALIGN(4096)) : {
   __mmu_tbl2_start = .;
   *(.mmu_tbl2)
   __mmu_tbl2_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.ARM.exidx : {
   __exidx_start = .;
   *(.ARM.exidx*)
   *(.gnu.linkonce.armexidix.*.*)
   __exidx_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.preinit_array : {
   . = ALIGN(64);
   __preinit_array_start = .;
   KEEP (*(SORT(.preinit_array.*)))
   KEEP (*(.preinit_array))
   __preinit_array_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.init_array : {
   . = ALIGN(64);
   __init_array_start = .;
   KEEP (*(SORT(.init_array.*)))
   KEEP (*(.init_array))
   __init_array_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.fini_array : {
   . = ALIGN(64);
   __fini_array_start = .;
   KEEP (*(SORT(.fini_array.*)))
   KEEP (*(.fini_array))
   __fini_array_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.drvcfg_sec : {
    . = ALIGN(8);
     __drvcfgsecdata_start = .;
    KEEP (*(.drvcfg_sec))
    __drvcfgsecdata_end = .;
    __drvcfgsecdata_size = __drvcfgsecdata_end - __drvcfgsecdata_start;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.ARM.attributes : {
   __ARM.attributes_start = .;
   *(.ARM.attributes)
   __ARM.attributes_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.sdata : {
   . = ALIGN(64);
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.sbss (NOLOAD) : {
   . = ALIGN(64);
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   . = ALIGN(64);
   __sbss_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.tdata : {
   . = ALIGN(64);
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.tbss : {
   . = ALIGN(64);
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.bss (NOLOAD) : {
   . = ALIGN(64);
   __bss_start__ = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(64);
   __bss_end__ = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.large_pool_base_ptr : {
   . = ALIGN(8);
   /* Absolute base address of the ISP buffer-pool region, stored as a 64-bit
    * word that ALWAYS lives in low memory (DDR_LOW_0) next to the code.
    * memory_manager.c reads this word instead of referencing
    * __large_pool_mem_start directly: when the pools are placed in high memory
    * (DDR_LOW_1 @0x8_00000000) that symbol is >4GB
    * from .text and the AArch64 small-code-model ADRP relocation
    * (R_AARCH64_ADR_PREL_PG_HI21) overflows ("relocation truncated to fit").
    * This pointer word is near the code, so loading it is always in range. */
   __large_pool_mem_base = .;
   QUAD(__large_pool_mem_start);
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.large_pool_mem (NOLOAD) : {
   . = ALIGN(64);
   __large_pool_mem_start = .;
   /* ISP frame-buffer pools (large + extralarge + 1MB) placed in
    * BUFFER_POOL_REGION, selected by the REGION_ALIAS switch above.
    * Software (memory_manager.c) derives the RPU high-mem prefix
    * (0x8 high / 0x0 low) from the linked address of __large_pool_mem_start.
    * Sizes MUST match MM_*_POOL_BLOCKS * MM_*_BLOCK_SIZE in memory_manager.h. */
   . += (64 * 10 * 1024 * 1024);  /* MM_LARGE_POOL_BLOCKS      * MM_LARGE_BLOCK_SIZE      */
   . += (24 * 25 * 1024 * 1024);  /* MM_EXTRALARGE_POOL_BLOCKS * MM_EXTRALARGE_BLOCK_SIZE */
   . += (64 *  1 * 1024 * 1024);  /* MM_1MB_POOL_BLOCKS        * MM_1MB_BLOCK_SIZE        */
   __large_pool_mem_end = .;
} > BUFFER_POOL_REGION

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */

.heap (NOLOAD) : {
   . = ALIGN(64);
   _heap = .;
   HeapBase = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
   HeapLimit = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0

.stack (NOLOAD) : {
   . = ALIGN(64);
   _el3_stack_end = .;
   . += _STACK_SIZE;
   __el3_stack = .;
   _el2_stack_end = .;
   . += _EL2_STACK_SIZE;
   . = ALIGN(64);
   __el2_stack = .;
   _el1_stack_end = .;
   . += _EL1_STACK_SIZE;
   . = ALIGN(64);
   __el1_stack = .;
   _el0_stack_end = .;
   . += _EL0_STACK_SIZE;
   . = ALIGN(64);
   __el0_stack = .;
} > NoC_C0_C1_ddr_memory_DDR_LOW_0


.cam_load_calib : {
	__cam_load_calib_start = .;
	*(.cam_load_calib)
	*(.cam_load_calib.*)
	__cam_load_calib_end = .;
} > load_calib

.mbox : {
	__mbox_start = .;
	*(.mbox)
	*(.mbox.*)
	__mbox_end = .;
} > mailbox

.tdatabase_bin (NOLOAD) : {
	/* Start address of the external-loaded tuning-DB blob. t_database.c reads
	 * __tdatabase_bin_start instead of hardcoding the address, so the memory map
	 * is owned solely by this linker script. */
	__tdatabase_bin_start = .;
} > tdatabase_bin
_end = .;
}
