/******************************************************************************
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x400;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x800;

/* Define Memories in the system */

MEMORY
{
	microblaze_ss_0_dlmb_bram_if_cntlr_memory_0 : ORIGIN = 0x0, LENGTH = 0x20000
}

/* Specify the default entry point to the program */

ENTRY(_boot)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.text.init : {
   __text.init_start = .;
   *(.text.init)
   __text.init_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.text : {
   . = ALIGN(2048);
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.init : {
   KEEP (*(.init))
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.fini : {
   KEEP (*(.fini))
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.ctors : {
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.dtors : {
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   __DTOR_END__ = .;
   ___DTORS_END___ = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.srodata : {
   __srodata_start = .;
   *(.srodata)
   __srodata_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.sdata2 : {
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   __sdata2_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.data : {
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   _edata = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.got : {
   *(.got)
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.got1 : {
   *(.got1)
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.got2 : {
   *(.got2)
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.eh_frame : {
   *(.eh_frame)
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.jcr : {
   *(.jcr)
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.gcc_except_table : {
   *(.gcc_except_table)
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.init_array : {
   __init_array_start = .;
   KEEP (*(SORT(.init_array.*)))
   KEEP (*(.init_array))
   __init_array_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.fini_array : {
   __fini_array_start = .;
   KEEP (*(SORT(.fini_array.*)))
   KEEP (*(.fini_array))
   __fini_array_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.sdata : {
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.bss (NOLOAD) : {
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   __bss_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.sbss (NOLOAD) : {
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   __sbss_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */

.heap (NOLOAD) : {
   . = ALIGN(8);
   _heap = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

.stack (NOLOAD) : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
} > microblaze_ss_0_dlmb_bram_if_cntlr_memory_0

__global_pointer$ = MIN(__sdata_start + 0x800, MAX(__data_start + 0x800, __bss_end - 0x800));
_end = .;
}
