/**************************************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x400;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x800;

/* Define Memories in the system */

MEMORY
{
   tmr_ram_cntlr_Mem : ORIGIN = 0xEBE00000, LENGTH = 0x60000
   tmr_ram_instr_cntlr_Mem : ORIGIN = 0xEBE00000, LENGTH = 0x40000
   tmr_ram_data_cntlr_Mem : ORIGIN = 0xEBE40000, LENGTH = 0x20000
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
} > tmr_ram_cntlr_Mem

.text : {
   . = ALIGN(2048);
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
} > tmr_ram_cntlr_Mem

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > tmr_ram_cntlr_Mem

.init : {
   KEEP (*(.init))
} > tmr_ram_cntlr_Mem

.fini : {
   KEEP (*(.fini))
} > tmr_ram_cntlr_Mem

.ctors : {
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > tmr_ram_cntlr_Mem

.dtors : {
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   __DTOR_END__ = .;
   ___DTORS_END___ = .;
} > tmr_ram_cntlr_Mem

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > tmr_ram_cntlr_Mem

.srodata : {
   __srodata_start = .;
   *(.srodata)
   __srodata_end = .;
} > tmr_ram_cntlr_Mem

.sdata2 : {
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   __sdata2_end = .;
} > tmr_ram_cntlr_Mem

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > tmr_ram_cntlr_Mem

.data : {
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   _edata = .;
} > tmr_ram_cntlr_Mem

.got : {
   *(.got)
} > tmr_ram_cntlr_Mem

.got1 : {
   *(.got1)
} > tmr_ram_cntlr_Mem

.got2 : {
   *(.got2)
} > tmr_ram_cntlr_Mem

.eh_frame : {
   *(.eh_frame)
} > tmr_ram_cntlr_Mem

.jcr : {
   *(.jcr)
} > tmr_ram_cntlr_Mem

.gcc_except_table : {
   *(.gcc_except_table)
} > tmr_ram_cntlr_Mem

.sdata : {
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > tmr_ram_cntlr_Mem

.sbss (NOLOAD) : {
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   __sbss_end = .;
} > tmr_ram_cntlr_Mem

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > tmr_ram_cntlr_Mem

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > tmr_ram_cntlr_Mem

.bss (NOLOAD) : {
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   __bss_end = .;
} > tmr_ram_cntlr_Mem

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */

.heap (NOLOAD) : {
   . = ALIGN(8);
   _heap = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
} > tmr_ram_cntlr_Mem

.stack (NOLOAD) : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
} > tmr_ram_cntlr_Mem

__global_pointer$ = MIN(__sdata_start + 0x800, MAX(__data_start + 0x800, __bss_end - 0x800));
_end = .;
}
