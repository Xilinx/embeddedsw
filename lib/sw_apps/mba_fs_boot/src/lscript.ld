/******************************************************************************
*
* Copyright (c) 2013 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/*******************************************************************/
/* Description : MicroBlaze Linker Script                          */
/*                                                                 */
/*******************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x64;
/* _HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x0; */

/* Define Memories in the system */

MEMORY
{
   microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl : ORIGIN = 0x00000050, LENGTH = 0x00001FB0
}

/* Specify the default entry point to the program */

ENTRY(_start)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
.vectors.reset 0x00000000 : {
   *(.vectors.reset)
}

.vectors.sw_exception 0x00000008 : {
   *(.vectors.sw_exception)
}

.vectors.interrupt 0x00000010 : {
   *(.vectors.interrupt)
}

.vectors.hw_exception 0x00000020 : {
   *(.vectors.hw_exception)
}


.text : {
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.init : {
   KEEP (*(.init))
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.fini : {
   KEEP (*(.fini))
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.sdata2 : {
   . = ALIGN(8);
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   . = ALIGN(8);
   __sdata2_end = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.data : {
   . = ALIGN(4);
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   __data_end = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.got : {
   *(.got)
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.got1 : {
   *(.got1)
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.got2 : {
   *(.got2)
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.ctors : {
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.dtors : {
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   __DTOR_END__ = .;
   ___DTORS_END___ = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.eh_frame : {
   *(.eh_frame)
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.jcr : {
   *(.jcr)
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.gcc_except_table : {
   *(.gcc_except_table)
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.sdata : {
   . = ALIGN(8);
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.sbss : {
   . = ALIGN(4);
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   . = ALIGN(8);
   __sbss_end = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

.bss : {
   . = ALIGN(4);
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
   __bss_end = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */

/*.heap : {
   . = ALIGN(8);
   _heap = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl
*/

.stack : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
} > microblaze_0_i_bram_ctrl_microblaze_0_d_bram_ctrl

_end = .;
}
