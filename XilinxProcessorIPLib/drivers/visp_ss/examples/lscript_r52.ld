/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x40000;*/
_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x80000;
/*_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x4000000;*/
/*_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x1000000;*/
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0xA000000;

_ABORT_STACK_SIZE = DEFINED(_ABORT_STACK_SIZE) ? _ABORT_STACK_SIZE : 1024;
_SUPERVISOR_STACK_SIZE = DEFINED(_SUPERVISOR_STACK_SIZE) ? _SUPERVISOR_STACK_SIZE : 2048;
_IRQ_STACK_SIZE = DEFINED(_IRQ_STACK_SIZE) ? _IRQ_STACK_SIZE : 1024;
_FIQ_STACK_SIZE = DEFINED(_FIQ_STACK_SIZE) ? _FIQ_STACK_SIZE : 1024;
_UNDEF_STACK_SIZE = DEFINED(_UNDEF_STACK_SIZE) ? _UNDEF_STACK_SIZE : 1024;

/*_FW_START_ADDR = DEFINED(_FW_START_ADDR) ? _FW_START_ADDR : 0x12000000;*/
/*_FW_START_SIZE = DEFINED(_FW_START_SIZE) ? _FW_START_SIZE : 0x10000000;*/

/*_FW_START_ADDR = DEFINED(_FW_START_ADDR) ? _FW_START_ADDR : 0x20000000;*/
_FW_START_ADDR = DEFINED(_FW_START_ADDR) ? _FW_START_ADDR : 0x31000000;
_FW_START_SIZE = DEFINED(_FW_START_SIZE) ? _FW_START_SIZE : 0x10000000;
/*__large_pool_mem_base = _FW_START_ADDR;*/


MEMORY
{
   load_calib : ORIGIN =0xF800000 , LENGTH = 0x28000
	mailbox : ORIGIN = 0x1E0A8000, LENGTH = 0x400000	 /*Reserve this memory for APU<->RPU communication*/
	pmc_ram : ORIGIN = 0xF2000000, LENGTH = 0x20000
   r52_tcm_alias : ORIGIN = 0x0, LENGTH = 0x2000
   r52_tcm_alias_bootdata : ORIGIN = 0x2000, LENGTH = 0x01E000
	axi_noc2_s0_ddr_memory : ORIGIN = _FW_START_ADDR, LENGTH =_FW_START_SIZE
	ps_wizard_0_ps11_0_ocm_ram_0_memory : ORIGIN = 0xbbf00000, LENGTH = 0x80000
	ps_wizard_0_ps11_0_ocm_ram_1_memory : ORIGIN = 0xbbf80000, LENGTH = 0x80000
	ps_wizard_0_ps11_0_ocm_ram_2_memory : ORIGIN = 0xbbe00000, LENGTH = 0x80000
	ps_wizard_0_ps11_0_ocm_ram_3_memory : ORIGIN = 0xbbe80000, LENGTH = 0x80000
}

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

}
