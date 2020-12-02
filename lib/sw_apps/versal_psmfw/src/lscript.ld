/******************************************************************************
 *
 * Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x400;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x800;

/* Define Memories in the system */

MEMORY
{
   instruction_Mem : ORIGIN = 0xFFC00050, LENGTH = 0x1FFB0
   data_Mem : ORIGIN = 0xFFC20000, LENGTH = 0x1A000
   dvsec_reserved_Mem : ORIGIN = 0xFFC3A000, LENGTH = 0x1000
   sem_reserved_Mem : ORIGIN = 0xFFC3B000, LENGTH = 0x4000
   tmr_ram_reserved_Mem : ORIGIN = 0xFFC3FF00, LENGTH = 0x100
}

/* Specify the default entry point to the program */

ENTRY(_start)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.vectors.reset 0xFFC00000 : {
   KEEP (*(.vectors.reset))
} 

.vectors.sw_exception 0xFFC00008 : {
   KEEP (*(.vectors.sw_exception))
} 

.vectors.interrupt 0xFFC00010 : {
   KEEP (*(.vectors.interrupt))
} 

.vectors.hw_exception 0xFFC00020 : {
   KEEP (*(.vectors.hw_exception))
} 

.text : {
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
} > instruction_Mem

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > instruction_Mem

.init : {
   KEEP (*(.init))
} > instruction_Mem

.fini : {
   KEEP (*(.fini))
} > instruction_Mem

.ctors : {
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > instruction_Mem

.dtors : {
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   PROVIDE(__DTOR_END__ = .);
   PROVIDE(___DTORS_END___ = .);
} > instruction_Mem

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > data_Mem

.sdata2 : {
   . = ALIGN(8);
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   . = ALIGN(8);
   __sdata2_end = .;
} > data_Mem

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > data_Mem

.data : {
   . = ALIGN(4);
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   __data_end = .;
} > data_Mem

.got : {
   *(.got)
} > data_Mem

.got1 : {
   *(.got1)
} > data_Mem

.got2 : {
   *(.got2)
} > data_Mem

.eh_frame : {
   *(.eh_frame)
} > data_Mem

.jcr : {
   *(.jcr)
} > data_Mem

.gcc_except_table : {
   *(.gcc_except_table)
} > data_Mem

.sdata : {
   . = ALIGN(8);
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > data_Mem

.sbss (NOLOAD) : {
   . = ALIGN(4);
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   . = ALIGN(8);
   __sbss_end = .;
} > data_Mem

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > data_Mem

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > data_Mem

.bss (NOLOAD) : {
   . = ALIGN(4);
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
   __bss_end = .;
} > data_Mem

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */

.heap (NOLOAD) : {
   . = ALIGN(8);
   _heap = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
} > data_Mem

.stack (NOLOAD) : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
} > data_Mem

.reserved_memory : {
   KEEP (*(.reserved_memory))
} > tmr_ram_reserved_Mem

.sem_reserved_memory : {
	KEEP (*(.sem_reserved_memory))
} > sem_reserved_Mem

.dvsec_reserved_memory : {
	KEEP (*(.dvsec_reserved_memory))
} > dvsec_reserved_Mem
_end = .;
}
