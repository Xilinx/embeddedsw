/******************************************************************************
*
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x1000;

/* Define Memories in the system */
MEMORY
{
	PMU_RAM : ORIGIN = 0xFFDC0000, LENGTH = 0x1F6E0	/* ~125.7 k */
	PMU_RAM_EXT_TABLE : ORIGIN = 0xFFDDF6E0, LENGTH = 0x400	/* 1 k */
}

/* Specify the default entry point to the program */
ENTRY(XPfw_Init)

/* Define the sections, and where they are mapped in memory */

SECTIONS
{
XpbrServHndlrTbl = 0xffd07bc8;

.vectors.reset 0xFFDC0000 : {
   KEEP (*(.vectors.reset))
} > PMU_RAM

.vectors.sw_exception 0xFFDC0008 : {
   KEEP (*(.vectors.sw_exception))
 } > PMU_RAM

.vectors.interrupt 0xFFDC0010 : {
   KEEP (*(.vectors.interrupt))
} > PMU_RAM

.vectors.hw_exception 0xFFDC0020 : {
   KEEP (*(.vectors.hw_exception))
   _MB_RESERVED_START = .;
} > PMU_RAM

.text 0xFFDC0050: {
   *(.text)
   *(.text.*)
   *(.gnu.linkonce.t.*)
} > PMU_RAM

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > PMU_RAM

.rodata : {
   __rodata_start = .;
   *(.rodata)
   *(.rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > PMU_RAM

.data : {
   . = ALIGN(4);
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   __data_end = .;
} > PMU_RAM

.sdata2 : {
   . = ALIGN(8);
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   . = ALIGN(8);
   __sdata2_end = .;
} > PMU_RAM

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > PMU_RAM

.sdata : {
   . = ALIGN(8);
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > PMU_RAM

.sbss (NOLOAD) : {
   . = ALIGN(4);
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   . = ALIGN(8);
   __sbss_end = .;
} > PMU_RAM

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > PMU_RAM

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > PMU_RAM

.bss (NOLOAD) : {
   . = ALIGN(4);
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   . = ALIGN(4);
   __bss_end = .;
} > PMU_RAM

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

.srdata : {
   __srdata_start = .;
   KEEP (*(.srdata))
   __srdata_end = .;
} > PMU_RAM

/* Generate Stack definitions */
.stack (NOLOAD) : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
} > PMU_RAM

.xpbr_serv_ext_tbl 0xffddf6e0: {
   KEEP (*(.xpbr_serv_ext_tbl))
} > PMU_RAM_EXT_TABLE

_end = .;
}
