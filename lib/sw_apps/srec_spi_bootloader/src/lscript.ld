/******************************************************************************
* Copyright (C) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/********************************************************************/
/* Description : MicroBlaze Linker Script                           */
/*                                                                  */
/********************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x400;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x0;

/* Define Memories in the system */

MEMORY
{
   microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem : ORIGIN = 0x50, LENGTH = 0xAFB0
   ddr3_sdram_memaddr : ORIGIN = 0x80000000, LENGTH = 0x40000000
}

/* Specify the default entry point to the program */

ENTRY(_start)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.vectors.reset 0x0 : {
   KEEP (*(.vectors.reset))
}

.vectors.sw_exception 0x8 : {
   KEEP (*(.vectors.sw_exception))
}

.vectors.interrupt 0x10 : {
   KEEP (*(.vectors.interrupt))
}

.vectors.hw_exception 0x20 : {
   KEEP (*(.vectors.hw_exception))
}

.text : {
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.init : {
   KEEP (*(.init))
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.fini : {
   KEEP (*(.fini))
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.ctors : {
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.dtors : {
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   PROVIDE(__DTOR_END__ = .);
   PROVIDE(___DTORS_END___ = .);
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.sdata2 : {
   . = ALIGN(8);
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   . = ALIGN(8);
   __sdata2_end = .;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.data : {
   . = ALIGN(4);
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   __data_end = .;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.got : {
   *(.got)
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.got1 : {
   *(.got1)
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.got2 : {
   *(.got2)
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.eh_frame : {
   *(.eh_frame)
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.jcr : {
   *(.jcr)
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.gcc_except_table : {
   *(.gcc_except_table)
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.sdata : {
   . = ALIGN(8);
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.sbss (NOLOAD) : {
   . = ALIGN(4);
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   . = ALIGN(8);
   __sbss_end = .;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.bss (NOLOAD) : {
   . = ALIGN(4);
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
   __bss_end = .;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */

.heap (NOLOAD) : {
   . = ALIGN(8);
   _heap = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

.stack (NOLOAD) : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
} > microblaze_0_local_memory_ilmb_bram_if_cntlr_Mem_microblaze_0_local_memory_dlmb_bram_if_cntlr_Mem

_end = .;
}
