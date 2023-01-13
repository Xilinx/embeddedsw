/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*****************************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x1000;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x100;
_USER_CODE_SIZE = DEFINED(_USER_CODE_SIZE) ? _USER_CODE_SIZE : 0x0;
/* Define Memories in the system */

MEMORY
{
   tmr_ram_cntlr_Mem : ORIGIN = 0xF0200000, LENGTH = (0x5FFB0 - _USER_CODE_SIZE)
   tmr_ram_instr_cntlr_Mem : ORIGIN = 0xF0200000, LENGTH = 0x3FFB0
   tmr_ram_data_cntlr_Mem : ORIGIN = 0xF0240000, LENGTH = 0x20000
}

/* Specify the default entry point to the program */

ENTRY(_start)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.vectors.reset 0xF0200000 : {
   KEEP (*(.vectors.reset))
} > tmr_ram_cntlr_Mem

.vectors.sw_exception 0xF0200008 : {
   KEEP (*(.vectors.sw_exception))
} > tmr_ram_cntlr_Mem

.vectors.interrupt 0xF0200010 : {
   KEEP (*(.vectors.interrupt))
} > tmr_ram_cntlr_Mem

.vectors.hw_exception 0xF0200020 : {
   KEEP (*(.vectors.hw_exception))
} > tmr_ram_cntlr_Mem

.text 0xF0200050 : {
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
   PROVIDE(__DTOR_END__ = .);
   PROVIDE(___DTORS_END___ = .);
} > tmr_ram_cntlr_Mem

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > tmr_ram_cntlr_Mem

.data : {
   . = ALIGN(4);
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   __data_end = .;
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
   . = ALIGN(8);
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > tmr_ram_cntlr_Mem

.sdata2 : {
   . = ALIGN(8);
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   . = ALIGN(8);
   __sdata2_end = .;
} > tmr_ram_cntlr_Mem

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > tmr_ram_cntlr_Mem
.sbss (NOLOAD) : {
   . = ALIGN(4);
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   . = ALIGN(8);
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
   . = ALIGN(4);
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
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
}
