/******************************************************************************
* Copyright (C) 2024-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x40000;*/
_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x80000;
/*_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x4000000;*/
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x1000000;
/*_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0xA000000;*/

_ABORT_STACK_SIZE = DEFINED(_ABORT_STACK_SIZE) ? _ABORT_STACK_SIZE : 1024;
_SUPERVISOR_STACK_SIZE = DEFINED(_SUPERVISOR_STACK_SIZE) ? _SUPERVISOR_STACK_SIZE : 2048;
_IRQ_STACK_SIZE = DEFINED(_IRQ_STACK_SIZE) ? _IRQ_STACK_SIZE : 1024;
_FIQ_STACK_SIZE = DEFINED(_FIQ_STACK_SIZE) ? _FIQ_STACK_SIZE : 1024;
_UNDEF_STACK_SIZE = DEFINED(_UNDEF_STACK_SIZE) ? _UNDEF_STACK_SIZE : 1024;

/*_FW_START_ADDR = DEFINED(_FW_START_ADDR) ? _FW_START_ADDR : 0x12000000;*/
/*_FW_START_SIZE = DEFINED(_FW_START_SIZE) ? _FW_START_SIZE : 0x10000000;*/

/* Base/size of the low-DDR window that holds the APU app: code, data, bss,
 * heap, stack AND the ISP frame-buffer pools (.large_pool_mem below).
 * Starts above the firmware/RPU-reserved bottom of DDR (load_calib @0xF800000,
 * mailbox @0x1E0A8000, tuning-DB blob @0x1F000000). Ends at 0x41000000, leaving
 * a 16MB gap before the DisplayPort frame buffers at 0x42000000 - the R52 MPU
 * region for the app must not touch the 0x42000000 display region boundary or
 * the CPU write to 0x42000000 stalls. Must match RPU0_FW_SIZE in
 * xvisp_ss_example.c. The ISP buffer pools (~289MB: 20*10MB large +
 * 1*25MB extralarge + 64*1MB) are placed in this region in low / 32-bit mode;
 * see BUFFER_POOL_REGION. */
/*_FW_START_ADDR = DEFINED(_FW_START_ADDR) ? _FW_START_ADDR : 0x31000000;*/
_FW_START_ADDR = DEFINED(_FW_START_ADDR) ? _FW_START_ADDR : 0x20000000;
/*_FW_START_SIZE = DEFINED(_FW_START_SIZE) ? _FW_START_SIZE : 0x21000000;*/
_FW_START_SIZE = DEFINED(_FW_START_SIZE) ? _FW_START_SIZE :   0x60000000;

MEMORY
{
   load_calib : ORIGIN =0xF800000 , LENGTH = 0x28000
	mailbox : ORIGIN = 0x1E0A8000, LENGTH = 0x400000	 /*Reserve this memory for APU<->RPU communication*/
	/* Tuning-database blob, pre-loaded by an external loader and read by
	 * t_database.c (APU_TDATABASE_BIN_ADDR). Lives in the free gap between the
	 * mailbox end (0x1E4A8000) and the axi_noc2_s0_ddr_memory ORIGIN (0x20000000),
	 * so it is never overlapped by the linker-managed region. */
	tdatabase_bin : ORIGIN = 0x1F000000, LENGTH = 0x100000
	pmc_ram : ORIGIN = 0xF2000000, LENGTH = 0x20000
   r52_tcm_alias : ORIGIN = 0x0, LENGTH = 0x2000
   r52_tcm_alias_bootdata : ORIGIN = 0x2000, LENGTH = 0x01E000
	axi_noc2_s0_ddr_memory : ORIGIN = _FW_START_ADDR, LENGTH =_FW_START_SIZE
   axi_noc2_s0_ddr_memory_high : ORIGIN = 0x820000000, LENGTH =_FW_START_SIZE
	/* Stream1's double-buffered 3840x2160 RGB888 canvas (see
	 * Video1CanvasFront/Back in mmi_dc_nonlive_test.c). Same fixed
	 * addresses/region this used to be reached via a raw pointer literal
	 * (IN_BUFFER_0_ADDR_V1=0x10000000 / DP_STREAM1_BACK_BUFFER_ADDR=
	 * 0x12000000) -- giving them a real linker-placed section makes them
	 * genuine, sizeof-able C arrays instead of magic numbers, with zero
	 * change to the physical address, MPU region, or cache attributes
	 * (still the dedicated STRONG_ORDERD_SHARED region at 0x10000000 in
	 * xvisp_ss_example.c, not the cached FW/heap region above). */
	stream1_canvas_front_region : ORIGIN = 0x10000000, LENGTH = 0x02000000
	stream1_canvas_back_region  : ORIGIN = 0x12000000, LENGTH = 0x02000000
	ps_wizard_0_ps11_0_ocm_ram_0_memory : ORIGIN = 0xbbf00000, LENGTH = 0x80000
	ps_wizard_0_ps11_0_ocm_ram_1_memory : ORIGIN = 0xbbf80000, LENGTH = 0x80000
	ps_wizard_0_ps11_0_ocm_ram_2_memory : ORIGIN = 0xbbe00000, LENGTH = 0x80000
	ps_wizard_0_ps11_0_ocm_ram_3_memory : ORIGIN = 0xbbe80000, LENGTH = 0x80000
}

/* ===========================================================================
 * ISP frame-buffer pool memory selection  --  SINGLE SWITCH
 *
 * Selects the region that holds the ISP frame-buffer pools (large + extralarge
 * + 1MB). Everything else auto-derives from the high 32 bits of
 * __large_pool_mem_start (read by memory_manager.c via the __large_pool_mem_base
 * pointer word): the RPU high-mem prefix sent by SetATM()/programmed into the HW
 * DMA, the is_64bit flag, and the software ATM-enable. No C code changes when
 * you switch regions.
 *
 * This platform is a 32-bit Cortex-R52, so only low / 32-bit DDR is addressable
 * (high DDR @0x8_00000000 is out of reach). The pools therefore live in
 * axi_noc2_s0_ddr_memory (low, prefix 0x0, is_64bit 0, ATM off).
 * axi_noc2_s0_ddr_memory_high (high, prefix 0x8, is_64bit 1, ATM on).
 * ===========================================================================*/
REGION_ALIAS("BUFFER_POOL_REGION", axi_noc2_s0_ddr_memory);   /* Low / 32-bit, prefix 0x0 */ /* high / 64-bit, prefix 0x8 */

/* Specify the default entry point to the program */

ENTRY(_vector_table)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.vectors : {
   KEEP (*(.vectors))
   *(.boot)
} > r52_tcm_alias

.bootdata : {
   *(.bootdata)
} > r52_tcm_alias_bootdata

.text : {
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
   *(.plt)
   *(.gnu_warning)
   *(.gcc_execpt_table)
   *(.glue_7)
   *(.glue_7t)
   *(.vfp11_veneer)
   *(.ARM.extab)
   *(.gnu.linkonce.armextab.*)
} > axi_noc2_s0_ddr_memory

.init : {
   KEEP (*(.init))
} > axi_noc2_s0_ddr_memory

.fini : {
   KEEP (*(.fini))
} > axi_noc2_s0_ddr_memory

.interp : {
   KEEP (*(.interp))
} > axi_noc2_s0_ddr_memory

.note-ABI-tag : {
   KEEP (*(.note-ABI-tag))
} > axi_noc2_s0_ddr_memory

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > axi_noc2_s0_ddr_memory

.rodata1 : {
   __rodata1_start = .;
   *(.rodata1)
   *(.rodata1.*)
   __rodata1_end = .;
} > axi_noc2_s0_ddr_memory

.sdata2 : {
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   __sdata2_end = .;
} > axi_noc2_s0_ddr_memory

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > axi_noc2_s0_ddr_memory

.data : {
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   *(.jcr)
   *(.got)
   *(.got.plt)
   __data_end = .;
} > axi_noc2_s0_ddr_memory

.data1 : {
   __data1_start = .;
   *(.data1)
   *(.data1.*)
   __data1_end = .;
} > axi_noc2_s0_ddr_memory

.got : {
   *(.got)
} > axi_noc2_s0_ddr_memory

.ctors : {
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > axi_noc2_s0_ddr_memory

.dtors : {
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   __DTOR_END__ = .;
   ___DTORS_END___ = .;
} > axi_noc2_s0_ddr_memory

.fixup : {
   __fixup_start = .;
   *(.fixup)
   __fixup_end = .;
} > axi_noc2_s0_ddr_memory

.eh_frame : {
   *(.eh_frame)
} > axi_noc2_s0_ddr_memory

.eh_framehdr : {
   __eh_framehdr_start = .;
   *(.eh_framehdr)
   __eh_framehdr_end = .;
} > axi_noc2_s0_ddr_memory

.gcc_except_table : {
   *(.gcc_except_table)
} > axi_noc2_s0_ddr_memory

.mmu_tbl (ALIGN(16384)) : {
   __mmu_tbl_start = .;
   *(.mmu_tbl)
   __mmu_tbl_end = .;
} > axi_noc2_s0_ddr_memory

.ARM.exidx : {
   __exidx_start = .;
   *(.ARM.exidx*)
   *(.gnu.linkonce.armexidix.*.*)
   __exidx_end = .;
} > axi_noc2_s0_ddr_memory

.preinit_array : {
   __preinit_array_start = .;
   KEEP (*(SORT(.preinit_array.*)))
   KEEP (*(.preinit_array))
   __preinit_array_end = .;
} > axi_noc2_s0_ddr_memory

.init_array : {
   __init_array_start = .;
   KEEP (*(SORT(.init_array.*)))
   KEEP (*(.init_array))
   __init_array_end = .;
} > axi_noc2_s0_ddr_memory

.fini_array : {
   __fini_array_start = .;
   KEEP (*(SORT(.fini_array.*)))
   KEEP (*(.fini_array))
   __fini_array_end = .;
} > axi_noc2_s0_ddr_memory

.drvcfg_sec : {
    . = ALIGN(8);
     __drvcfgsecdata_start = .;
    KEEP (*(.drvcfg_sec))
    __drvcfgsecdata_end = .;
    __drvcfgsecdata_size = __drvcfgsecdata_end - __drvcfgsecdata_start;
} > axi_noc2_s0_ddr_memory

.ARM.attributes : {
   __ARM.attributes_start = .;
   *(.ARM.attributes)
   __ARM.attributes_end = .;
} > axi_noc2_s0_ddr_memory

.sdata : {
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > axi_noc2_s0_ddr_memory

.sbss (NOLOAD) : {
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   __sbss_end = .;
} > axi_noc2_s0_ddr_memory

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > axi_noc2_s0_ddr_memory

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > axi_noc2_s0_ddr_memory

.bss (NOLOAD) : {
   . = ALIGN(4);
   __bss_start__ = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
   __bss_end__ = .;
} > axi_noc2_s0_ddr_memory

.large_pool_base_ptr : {
   . = ALIGN(8);
   /* Absolute base address of the ISP buffer-pool region, stored as a 64-bit
    * word that lives in low memory next to the code. memory_manager.c reads
    * this word (extern uint64_t __large_pool_mem_base) instead of referencing
    * __large_pool_mem_start directly, so the same source also works when the
    * pools are placed in high memory on 64-bit parts (avoids the AArch64
    * small-code-model ADRP relocation overflow). */
   __large_pool_mem_base = .;
   QUAD(__large_pool_mem_start);
} > axi_noc2_s0_ddr_memory

.large_pool_mem (NOLOAD) : {
   . = ALIGN(64);
   __large_pool_mem_start = .;
   /* ISP frame-buffer pools (large + extralarge + 1MB) placed in
    * BUFFER_POOL_REGION, selected by the REGION_ALIAS switch above.
    * Sizes MUST match MM_*_POOL_BLOCKS * MM_*_BLOCK_SIZE in memory_manager.h. */
    . += (64 * 10 * 1024 * 1024);  /* MM_LARGE_POOL_BLOCKS      * MM_LARGE_BLOCK_SIZE      */
    . += ( 24 * 25 * 1024 * 1024);  /* MM_EXTRALARGE_POOL_BLOCKS * MM_EXTRALARGE_BLOCK_SIZE */
    . += (64 *  1 * 1024 * 1024);  /* MM_1MB_POOL_BLOCKS        * MM_1MB_BLOCK_SIZE        */
   __large_pool_mem_end = .;
} > BUFFER_POOL_REGION

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */

.heap (NOLOAD) : {
   . = ALIGN(16);
   _heap = .;
   HeapBase = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
   HeapLimit = .;
} > axi_noc2_s0_ddr_memory

.stack (NOLOAD) : {
   . = ALIGN(16);
   _stack_end = .;
   . += _STACK_SIZE;
   _stack = .;
   __stack = _stack;
   . = ALIGN(16);
   _irq_stack_end = .;
   . += _IRQ_STACK_SIZE;
   __irq_stack = .;
   _supervisor_stack_end = .;
   . += _SUPERVISOR_STACK_SIZE;
   . = ALIGN(16);
   __supervisor_stack = .;
   _abort_stack_end = .;
   . += _ABORT_STACK_SIZE;
   . = ALIGN(16);
   __abort_stack = .;
   _fiq_stack_end = .;
   . += _FIQ_STACK_SIZE;
   . = ALIGN(16);
   __fiq_stack = .;
   _undef_stack_end = .;
   . += _UNDEF_STACK_SIZE;
   . = ALIGN(16);
   __undef_stack = .;
} > axi_noc2_s0_ddr_memory

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

.stream1_canvas_front : {
	. = ALIGN(64);
	*(.stream1_canvas_front)
} > stream1_canvas_front_region

.stream1_canvas_back : {
	. = ALIGN(64);
	*(.stream1_canvas_back)
} > stream1_canvas_back_region
}
