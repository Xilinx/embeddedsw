/******************************************************************************
*
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*******************************************************************/
/*                                                                 */
/* Description : Cortex-A53 Linker Script                          */
/*                                                                 */
/*******************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x2000;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x2000;

_EL0_STACK_SIZE = DEFINED(_EL0_STACK_SIZE) ? _EL0_STACK_SIZE : 1024;
_EL1_STACK_SIZE = DEFINED(_EL1_STACK_SIZE) ? _EL1_STACK_SIZE : 2048;
_EL2_STACK_SIZE = DEFINED(_EL2_STACK_SIZE) ? _EL2_STACK_SIZE : 1024;

/* Define Memories in the system */

MEMORY
{
   psu_bbram_0_S_AXI_BASEADDR : ORIGIN = 0xFFCD0000, LENGTH = 0x10000
   psu_ddr_0_S_AXI_BASEADDR_ : ORIGIN = 0x0, LENGTH = 0x20000000
   psu_ocm_ram_0_S_AXI_BASEADDR : ORIGIN = 0xFFFC0000, LENGTH = 0x30000
   psu_ocm_ram_1_S_AXI_BASEADDR : ORIGIN = 0xFFFF0000, LENGTH = 0x10000
   psu_ocm_xmpu_cfg_S_AXI_BASEADDR : ORIGIN = 0xFFA70000, LENGTH = 0x10000
   psu_pmu_ram_S_AXI_BASEADDR : ORIGIN = 0xFFDC0000, LENGTH = 0x20000
   psu_qspi_linear_0_S_AXI_BASEADDR : ORIGIN = 0xC0000000, LENGTH = 0x20000000
}

/* Specify the default entry point to the program */

ENTRY(_vector_table)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.text : {
   KEEP (*(.text))
   *(.text.*)
   *(.gnu.linkonce.t.*)
   *(.plt)
   *(.gnu_warning)
   *(.gcc_execpt_table)
   *(.glue_7)
   *(.glue_7t)
   *(.ARM.extab)
   *(.gnu.linkonce.armextab.*)
} > psu_ocm_ram_0_S_AXI_BASEADDR

.boot : {
   KEEP (*(.vectors))
   *(.boot)
} > psu_ocm_ram_1_S_AXI_BASEADDR

.init (ALIGN(64)) : {
   KEEP (*(.init))
} > psu_ocm_ram_1_S_AXI_BASEADDR

.fini (ALIGN(64)) : {
   KEEP (*(.fini))
} > psu_ocm_ram_1_S_AXI_BASEADDR

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > psu_ocm_ram_0_S_AXI_BASEADDR

.interp : {
   KEEP (*(.interp))
} > psu_ocm_ram_0_S_AXI_BASEADDR

.note-ABI-tag : {
   KEEP (*(.note-ABI-tag))
} > psu_ocm_ram_0_S_AXI_BASEADDR

.rodata : {
   . = ALIGN(64);
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.rodata1 : {
   . = ALIGN(64);
   __rodata1_start = .;
   *(.rodata1)
   *(.rodata1.*)
   __rodata1_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.sdata2 : {
   . = ALIGN(64);
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   __sdata2_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.sbss2 : {
   . = ALIGN(64);
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

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
} > psu_ocm_ram_0_S_AXI_BASEADDR

.data1 : {
   . = ALIGN(64);
   __data1_start = .;
   *(.data1)
   *(.data1.*)
   __data1_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.got : {
   *(.got)
} > psu_ocm_ram_0_S_AXI_BASEADDR

.got1 : {
   *(.got1)
} > psu_ocm_ram_0_S_AXI_BASEADDR

.got2 : {
   *(.got2)
} > psu_ocm_ram_0_S_AXI_BASEADDR

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
} > psu_ocm_ram_0_S_AXI_BASEADDR

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
} > psu_ocm_ram_0_S_AXI_BASEADDR

.fixup : {
   __fixup_start = .;
   *(.fixup)
   __fixup_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.eh_frame : {
   *(.eh_frame)
} > psu_ocm_ram_0_S_AXI_BASEADDR

.eh_framehdr : {
   __eh_framehdr_start = .;
   *(.eh_framehdr)
   __eh_framehdr_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.gcc_except_table : {
   *(.gcc_except_table)
} > psu_ocm_ram_0_S_AXI_BASEADDR

.mmu_tbl0 (ALIGN(4096)) : {
   __mmu_tbl0_start = .;
   *(.mmu_tbl0)
   __mmu_tbl0_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.mmu_tbl1 (ALIGN(4096)) : {
   __mmu_tbl1_start = .;
   *(.mmu_tbl1)
   __mmu_tbl1_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.mmu_tbl2 (ALIGN(4096)) : {
   __mmu_tbl2_start = .;
   *(.mmu_tbl2)
   __mmu_tbl2_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.ARM.exidx : {
   __exidx_start = .;
   *(.ARM.exidx*)
   *(.gnu.linkonce.armexidix.*.*)
   __exidx_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.preinit_array : {
   . = ALIGN(64);
   __preinit_array_start = .;
   KEEP (*(SORT(.preinit_array.*)))
   KEEP (*(.preinit_array))
   __preinit_array_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.init_array : {
   . = ALIGN(64);
   __init_array_start = .;
   KEEP (*(SORT(.init_array.*)))
   KEEP (*(.init_array))
   __init_array_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.fini_array : {
   . = ALIGN(64);
   __fini_array_start = .;
   KEEP (*(SORT(.fini_array.*)))
   KEEP (*(.fini_array))
   __fini_array_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.ARM.attributes : {
   __ARM.attributes_start = .;
   *(.ARM.attributes)
   __ARM.attributes_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.sdata : {
   . = ALIGN(64);
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.sbss (NOLOAD) : {
   . = ALIGN(64);
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   . = ALIGN(64);
   __sbss_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.tdata : {
   . = ALIGN(64);
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.tbss : {
   . = ALIGN(64);
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

.bss (NOLOAD) : {
   . = ALIGN(64);
   __bss_start__ = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(64);
   __bss_end__ = .;
} > psu_ocm_ram_0_S_AXI_BASEADDR

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
} > psu_ocm_ram_1_S_AXI_BASEADDR

.comment : {
   *(.comment)
} > psu_ocm_ram_0_S_AXI_BASEADDR


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
} > psu_ocm_ram_1_S_AXI_BASEADDR

_end = .;
}


