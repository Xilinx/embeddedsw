/******************************************************************************
* Copyright (C) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x400;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x800;

/* Define Memories in the system */

MEMORY
{
	lpddrmc_0_ddr_memory_0 : ORIGIN = 0x90000000, LENGTH = 0x70000000
	microblaze_riscv_0_local_memory_dlmb_bram_if_cntlr_memory_0 : ORIGIN = 0x0, LENGTH = 0x20000
	new_memory_0_app : ORIGIN = 0x80000000, LENGTH = 0x1000000
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
} > new_memory_0_app

.text : {
   . = ALIGN(2048);
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
} > new_memory_0_app

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > new_memory_0_app

.init : {
   KEEP (*(.init))
} > new_memory_0_app

.fini : {
   KEEP (*(.fini))
} > new_memory_0_app

.ctors : {
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > new_memory_0_app

.dtors : {
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   __DTOR_END__ = .;
   ___DTORS_END___ = .;
} > new_memory_0_app

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > new_memory_0_app

.srodata : {
   __srodata_start = .;
   *(.srodata)
   __srodata_end = .;
} > new_memory_0_app

.sdata2 : {
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   __sdata2_end = .;
} > new_memory_0_app

.data : {
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   _edata = .;
} > new_memory_0_app

.got : {
   *(.got)
} > new_memory_0_app

.got1 : {
   *(.got1)
} > new_memory_0_app

.got2 : {
   *(.got2)
} > new_memory_0_app

.eh_frame : {
   *(.eh_frame)
} > new_memory_0_app

.jcr : {
   *(.jcr)
} > new_memory_0_app

.gcc_except_table : {
   *(.gcc_except_table)
} > new_memory_0_app

.init_array : {
   __init_array_start = .;
   KEEP (*(SORT(.init_array.*)))
   KEEP (*(.init_array))
   __init_array_end = .;
} > new_memory_0_app

.fini_array : {
   __fini_array_start = .;
   KEEP (*(SORT(.fini_array.*)))
   KEEP (*(.fini_array))
   __fini_array_end = .;
} > new_memory_0_app

.sdata : {
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > new_memory_0_app

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > new_memory_0_app

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > new_memory_0_app

.bss (NOLOAD) : {
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   __bss_end = .;
} > new_memory_0_app

.sbss (NOLOAD) : {
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   __sbss_end = .;
} > new_memory_0_app

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > new_memory_0_app

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */

.heap (NOLOAD) : {
   . = ALIGN(8);
   _heap = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
} > new_memory_0_app

.stack (NOLOAD) : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
} > new_memory_0_app

__global_pointer$ = MIN(__sdata_start + 0x800, MAX(__data_start + 0x800, __bss_end - 0x800));
_end = .;
}
