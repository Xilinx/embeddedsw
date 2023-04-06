/******************************************************************************
 * Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
 * Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x400;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x800;

/* Define Memories in the system */

MEMORY
{
   psmfw_Mem : ORIGIN = 0xEBC00000, LENGTH = 0x26000
   proc_reserved_Mem : ORIGIN = 0xEBC26000, LENGTH = 0x2000
}

/* Specify the default entry point to the program */

ENTRY(_start)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.vectors.reset 0xEBC00000 : {
   KEEP (*(.vectors.reset))
} > psmfw_Mem

.vectors.sw_exception 0xEBC00008 : {
   KEEP (*(.vectors.sw_exception))
} > psmfw_Mem

.vectors.interrupt 0xEBC00010 : {
   KEEP (*(.vectors.interrupt))
} > psmfw_Mem

.vectors.hw_exception 0xEBC00020 : {
   KEEP (*(.vectors.hw_exception))
} > psmfw_Mem

.text : {
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
} > psmfw_Mem

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > psmfw_Mem

.init : {
   KEEP (*(.init))
} > psmfw_Mem

.fini : {
   KEEP (*(.fini))
} > psmfw_Mem

.ctors : {
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > psmfw_Mem

.dtors : {
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   PROVIDE(__DTOR_END__ = .);
   PROVIDE(___DTORS_END___ = .);
} > psmfw_Mem

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > psmfw_Mem


.data : {
   . = ALIGN(4);
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   __data_end = .;
} > psmfw_Mem

.got : {
   *(.got)
} > psmfw_Mem

.got1 : {
   *(.got1)
} > psmfw_Mem

.got2 : {
   *(.got2)
} > psmfw_Mem

.eh_frame : {
   *(.eh_frame)
} > psmfw_Mem

.jcr : {
   *(.jcr)
} > psmfw_Mem

.gcc_except_table : {
   *(.gcc_except_table)
} > psmfw_Mem

.sdata : {
   . = ALIGN(8);
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > psmfw_Mem

.sdata2 : {
   . = ALIGN(8);
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   . = ALIGN(8);
   __sdata2_end = .;
} > psmfw_Mem

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > psmfw_Mem

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > psmfw_Mem

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > psmfw_Mem

.sbss (NOLOAD) : {
   . = ALIGN(4);
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   . = ALIGN(8);
   __sbss_end = .;
} > psmfw_Mem

.bss (NOLOAD) : {
   . = ALIGN(4);
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
   __bss_end = .;
} > psmfw_Mem

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */

.heap (NOLOAD) : {
   . = ALIGN(8);
   _heap = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
} > psmfw_Mem

.stack (NOLOAD) : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
} > psmfw_Mem

.proc_reserved_memory (NOLOAD) : {
	KEEP (*(.proc_reserved_memory))
} > proc_reserved_Mem
_end = .;
}