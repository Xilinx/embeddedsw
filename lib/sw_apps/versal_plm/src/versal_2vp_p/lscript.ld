/******************************************************************************
 *
 * Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
 * Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 *****************************************************************************/

INCLUDE "xpm_memory_pools.ld"
_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x2000;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x100;
_USER_CODE_SIZE = DEFINED(_USER_CODE_SIZE) ? _USER_CODE_SIZE : 0x4000;
_PLM_UPDATE_CODE_SIZE = DEFINED(_PLM_UPDATE_CODE_SIZE) ? _PLM_UPDATE_CODE_SIZE : 0x200;
_PROCBUFFER_SIZE = DEFINED(_PROCBUFFER_SIZE) ? _PROCBUFFER_SIZE : 0x3000;
/* Size reserved for PDI processing default is 16KB */
_PDI_PROCESSING_SIZE = DEFINED(_PDI_PROCESSING_SIZE) ? _PDI_PROCESSING_SIZE : 0x4000;

/* Define the length of TMR RAM */
TMR_RAM_BASE = 0xF0200000; /* Base address of TMR RAM */
TMR_RAM_LENGTH = 0x9FFB0; /* Total TMR RAM length */

/* PLM RAM length after user code and update manager code */
PLM_RAM_LENGTH = TMR_RAM_LENGTH - _USER_CODE_SIZE - _PLM_UPDATE_CODE_SIZE;

/* Data controller base address and length */
TMR_RAM_DATA_CNTRL_BASE = 0xF0280000; /* Base address for TMR RAM data controller */
TMR_RAM_DATA_CNTRL_LENGTH = 0x20000; /* Length of TMR RAM data controller memory 128KB*/
/* Update manager base address */
UPDATE_MGR_BASE = TMR_RAM_DATA_CNTRL_BASE + TMR_RAM_DATA_CNTRL_LENGTH - _PLM_UPDATE_CODE_SIZE - _PDI_PROCESSING_SIZE;
XILPM_BSS_BASE = TMR_RAM_DATA_CNTRL_BASE + TMR_RAM_DATA_CNTRL_LENGTH - _PLM_UPDATE_CODE_SIZE - _PDI_PROCESSING_SIZE - XILPM_BSS_MAX_SIZE;

/* Define Memories in the system */
MEMORY
{
   /* Total TMR RAM memory */
   tmr_ram_cntlr_Mem : ORIGIN = TMR_RAM_BASE, LENGTH = TMR_RAM_LENGTH
   /* Data controller memory */
   tmr_ram_data_cntlr_Mem : ORIGIN = TMR_RAM_DATA_CNTRL_BASE, LENGTH = TMR_RAM_DATA_CNTRL_LENGTH
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

.update_mgr_a : {
  . = ALIGN(16);
  __update_mgr_a_fn_start = .;
   KEEP (*(.update_mgr_a))
  __update_mgr_a_fn_end = .;
  _update_mgr_a_end = __update_mgr_a_fn_start + _PLM_UPDATE_CODE_SIZE;
  . = _update_mgr_a_end;
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


.struct_data : {
   . = ALIGN(4);
   __data_struct_start = .;
   KEEP(*(.struct_entries))
   __data_struct_end = .;
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

/* Define the .ProcBuffer section */
.ProcBuffer (NOLOAD) : {
 __ProcBuffer_start = .;
   KEEP(*(.ProcBuffer))
   . += _PROCBUFFER_SIZE;
   __ProcBuffer_end = .;
} > tmr_ram_cntlr_Mem

.xplm_modules (INFO) : {
   KEEP (*(.xplm_modules))
}

.struct_info (INFO): {
   KEEP(*(.struct_info_hdr))
   KEEP(*(.struct_versions))
}
/*******************************************************************************************
 * @brief xpm_initialized_data Xilpm initialized data section
 *
 * This section defines memory layout for initialized data that needs to be
 * saved and restored during XilPM (Xilinx Power Management) runtime operations.
 *
 * @details The init data section contains:
 * - Runtime variables that maintain state across PLM impactless update
 * - Configuration parameters that are initialized at boot but may change during runtime
 * - Critical system state information required for proper power management operation
 *
 *******************************************************************************************/
.xpm_initialized_data : {
   . = ALIGN(4);
   __xpm_init_data_start = . ;
   KEEP(*(.RefTable))
   . = ALIGN(4);
   KEEP(*(SORT(.RefTable.*)))
    __xpm_init_data_end = .;
} > tmr_ram_cntlr_Mem

/******************************************************************************************
 * @brief xpm_bss_data XilPm Uninitialized Data Section
 *
 * This section defines memory layout for XilPm (Xilinx Power Management)
 * uninitialized data that requires fixed memory locations. The fixed
 * positioning is essential to maintain consistent memory references
 * across impactless firmware updates, ensuring that power management
 * state and configuration data remain accessible and valid during
 * system updates without requiring a full system restart.
 *
 * @note Fixed memory locations prevent reference corruption during
 *       hot updates and maintain system stability.
 */
/*******************************************************************************************/
.xpm_bss_data XILPM_BSS_BASE (NOLOAD) : {
   /* TopoPoolMemBuffer */
   . = ALIGN(4);
   __xpm_bss_start = .;
   __xpm_topo_start = .;
   KEEP(*(.TopoPoolMemBuffer))
   __xpm_topo_end = .;
   __xpm_subsys_start = .;
      /* SubSystemPoolMemBuffer */
   . = ALIGN(4);
   KEEP(*(.SubSystemPoolMemBuffer))

   . = ALIGN(4);
   /* ReqmPoolMemBuffer */
   KEEP(*(.ReqmPoolMemBuffer))

   . = ALIGN(4);
   /* DevOpsPoolMemBuffer */
   KEEP(*(.DevOpsPoolMemBuffer))

   . = ALIGN(4);
   /* OtherPoolMemBuffer */
   KEEP(*(.OtherPoolMemBuffer))
   __xpm_subsys_end = .;
   __xpm_board_bss_start = .;
   . = ALIGN(4);
   KEEP(*(.BoardPoolMemBuffer))
   __xpm_board_bss_end = .;
   __xpm_bss_end = .;
} > tmr_ram_cntlr_Mem

/**********************************************/
.update_mgr_b  UPDATE_MGR_BASE (NOLOAD) : {
   __update_mgr_b = .;
   __update_mgr_b_start = .;
   . += _PLM_UPDATE_CODE_SIZE;
   __update_mgr_b_end = .;
} > tmr_ram_cntlr_Mem

_end = .;
}
