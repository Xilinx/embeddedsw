/**************************************************************************************************
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x1000;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x800;
_LOG_BUF_SIZE = DEFINED(_LOG_BUF_SIZE) ? _LOG_BUF_SIZE : 0x1000;
_ASUFW_UPDATE_MNG_SIZE = DEFINED(_ASUFW_UPDATE_MNG_SIZE) ? _ASUFW_UPDATE_MNG_SIZE : 0x200;
_OVERLAY_DDR_BASE = DEFINED(_OVERLAY_DDR_BASE) ? _OVERLAY_DDR_BASE : 0x7FFC0000;
_OVERLAY_STORE_SIZE = 0x40000;
_OVERLAY_IMG_ALIGN = 64;

/* Define Memories in the system */

MEMORY
{
   tmr_ram_cntlr_Mem : ORIGIN = 0xEBE00000, LENGTH = 0x60000
   tmr_ram_instr_cntlr_Mem : ORIGIN = 0xEBE00000, LENGTH = 0x40000
   tmr_ram_data_Confg_Mem : ORIGIN = 0xEBE40000, LENGTH = 0x1000
   tmr_ram_data_ipi_shared_Mem : ORIGIN = 0xEBE41000, LENGTH = 0x8000
   tmr_ram_data_log_buf_Mem : ORIGIN = 0xEBE49000, LENGTH = _LOG_BUF_SIZE
   tmr_ram_data_ocp_Mem : ORIGIN = 0xEBE4A000, LENGTH = 0x2800
   tmr_ram_crypto_data_block_Mem : ORIGIN = 0xEBE4C800, LENGTH = 0x1200
   update_mgr_Mem : ORIGIN = 0xEBE4DA00, LENGTH = _ASUFW_UPDATE_MNG_SIZE
   tmr_ram_overlay_data_Mem : ORIGIN = 0xEBE4DC00, LENGTH = 0x12400

   ddr_overlay_store_Mem : ORIGIN = _OVERLAY_DDR_BASE, LENGTH = _OVERLAY_STORE_SIZE
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
} > tmr_ram_instr_cntlr_Mem

.text : {
   . = ALIGN(2048);
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .text)
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .text.*)
   *(.gnu.linkonce.t.*)
} > tmr_ram_instr_cntlr_Mem

.note.gnu.build-id : {
   KEEP (*(.note.gnu.build-id))
} > tmr_ram_instr_cntlr_Mem

.init : {
   KEEP (*(.init))
} > tmr_ram_instr_cntlr_Mem

.fini : {
   KEEP (*(.fini))
} > tmr_ram_instr_cntlr_Mem

.init_array : {
  __init_array_start = .;
  KEEP (*(SORT(.init_array.*)))
  KEEP (*(.init_array))
  __init_array_end = .;
} > tmr_ram_instr_cntlr_Mem

.fini_array : {
  __fini_array_start = .;
  KEEP (*(SORT(.fini_array.*)))
  KEEP (*(.fini_array))
  __fini_array_end = .;
} > tmr_ram_instr_cntlr_Mem

.ctors : {
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > tmr_ram_instr_cntlr_Mem

.dtors : {
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   __DTOR_END__ = .;
   ___DTORS_END___ = .;
} > tmr_ram_instr_cntlr_Mem

.rodata : {
   __rodata_start = .;
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .rodata)
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > tmr_ram_instr_cntlr_Mem

.srodata : {
   __srodata_start = .;
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .srodata)
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .srodata.*)
   __srodata_end = .;
} > tmr_ram_instr_cntlr_Mem

.sdata2 : {
   __sdata2_start = .;
   *(.sdata2)
   *(.sdata2.*)
   *(.gnu.linkonce.s2.*)
   __sdata2_end = .;
} > tmr_ram_instr_cntlr_Mem

.sbss2 : {
   __sbss2_start = .;
   *(.sbss2)
   *(.sbss2.*)
   *(.gnu.linkonce.sb2.*)
   __sbss2_end = .;
} > tmr_ram_instr_cntlr_Mem

.data : {
   __data_start = .;
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .data)
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .data.*)
   *(.gnu.linkonce.d.*)
   /*
    * Data structures kept out of the overlay window and placed in loaded, non-overlay memory
    * so they persist across overlay switches and keep their initial values.
    */
   *(.overlay_persistent_data)
   *(.overlay_persistent_data.*)
   _edata = .;
} > tmr_ram_instr_cntlr_Mem

.got : {
   *(.got)
} > tmr_ram_instr_cntlr_Mem

.got1 : {
   *(.got1)
} > tmr_ram_instr_cntlr_Mem

.got2 : {
   *(.got2)
} > tmr_ram_instr_cntlr_Mem

.eh_frame : {
   *(.eh_frame)
} > tmr_ram_instr_cntlr_Mem

.jcr : {
   *(.jcr)
} > tmr_ram_instr_cntlr_Mem

.gcc_except_table : {
   *(.gcc_except_table)
} > tmr_ram_instr_cntlr_Mem

.sdata : {
   __sdata_start = .;
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .sdata)
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > tmr_ram_instr_cntlr_Mem

.struct_entries : {
   __data_struct_start = .;
   KEEP(*(.struct_entries))
   KEEP(*(.struct_entries*))
   __data_struct_end = .;
} > tmr_ram_instr_cntlr_Mem

/* Collect DS version records for optional-data block id 0x04 in IHT */
.struct_info_asu (INFO) : {
   KEEP(*(.struct_info_hdr))
   KEEP(*(.struct_versions))
}

.update_mgr_a : {
   . = ALIGN(16);
   __update_mgr_a_fn_start = .;
   KEEP (*(.update_mgr_a))
   __update_mgr_a_fn_end = .;
   _update_mgr_a_end = __update_mgr_a_fn_start + _ASUFW_UPDATE_MNG_SIZE;
   . = _update_mgr_a_end;
} > tmr_ram_instr_cntlr_Mem

.sbss (NOLOAD) : {
   __sbss_start = .;
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .sbss)
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .sbss.*)
   *(.gnu.linkonce.sb.*)
   __sbss_end = .;
} > tmr_ram_instr_cntlr_Mem

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > tmr_ram_instr_cntlr_Mem

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > tmr_ram_instr_cntlr_Mem

.bss (NOLOAD) : {
   __bss_start = .;
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .bss)
   *(EXCLUDE_FILE("*ecc/*.o*" "*rsa_ecc/*.o*" "*lms/*.o*" "*ecies/*.o*") .bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   __bss_end = .;
} > tmr_ram_instr_cntlr_Mem

.debug_log_buffer (NOLOAD):
{
   __debug_log_buffer_start = .;
   *(.debug_log_buffer)
   *(.debug_log_buffer.*)
   *(.gnu.linkonce.rs.*)
   __debug_log_buffer_end = .;
} > tmr_ram_data_log_buf_Mem

.crypto_algo_data_block (NOLOAD) : {
   __crypto_algo_data_block_start = .;
   . = ORIGIN(tmr_ram_crypto_data_block_Mem) + LENGTH(tmr_ram_crypto_data_block_Mem);
   __crypto_algo_data_block_end = .;
} > tmr_ram_crypto_data_block_Mem

.update_mgr_b (NOLOAD) : {
   __update_mgr_b = .;
   __update_mgr_b_start = .;
   . += _ASUFW_UPDATE_MNG_SIZE;
   __update_mgr_b_end = .;
} > update_mgr_Mem

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */

.heap (NOLOAD) : {
   . = ALIGN(8);
   _heap = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
} > tmr_ram_instr_cntlr_Mem

.stack (NOLOAD) : {
   _stack_end = .;
   . += _STACK_SIZE;
   . = ALIGN(8);
   _stack = .;
   __stack = _stack;
   __overlay_window_start = .;
} > tmr_ram_instr_cntlr_Mem

__global_pointer$ = MIN(__sdata_start + 0x800, MAX(__data_start + 0x800, __bss_end - 0x800));
_end = .;

/*
 * Overlay region: VMA(Virtual Memory Address) is the fixed instruction-RAM window,
 * LMA(Load Memory Address) is the DDR address; images are loaded on demand at runtime.
 */
OVERLAY __overlay_window_start : NOCROSSREFS AT (ORIGIN(ddr_overlay_store_Mem))
{
   .overlay_lms
   {
      KEEP(*(.overlay_lms.header))
      "*lms/*.o*"(.text .text.*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }
   /*
    * Add other algorithms above this block. The combined RSA-ECC image must
    * remain the last algorithm because its LOADADDR + SIZEOF defines the end
    * of the text overlay store LMA (__overlay_store_text_window_end). Adding an
    * algorithm after it would require reworking that computation. The rsa_ecc,
    * ecc and ecies objects share this single overlay image; the rsa_ecc glob is
    * listed ahead of the ecc glob because "*ecc/*" also matches rsa_ecc objects
    * and the linker assigns each input section to the first matching pattern.
    */

   /* Algorithm Last - combined RSA-ECC text */
   .overlay_rsa_ecc
   {
      KEEP(*(.overlay_rsa_ecc.header))
      "*rsa_ecc/*.o*"(.text .text.*)
      "*ecc/*.o*"(.text .text.*)
      "*ecies/*.o*"(.text .text.*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }
} > tmr_ram_instr_cntlr_Mem

__overlay_lms_txt_size = SIZEOF(.overlay_lms);
__overlay_rsa_ecc_txt_size = SIZEOF(.overlay_rsa_ecc);

__overlay_window_size = MAX(__overlay_rsa_ecc_txt_size, __overlay_lms_txt_size);

__overlay_store_text_window_end = LOADADDR(.overlay_rsa_ecc) + SIZEOF(.overlay_rsa_ecc);
__overlay_data_window_start = ORIGIN(tmr_ram_overlay_data_Mem);
OVERLAY __overlay_data_window_start : NOCROSSREFS AT (__overlay_store_text_window_end)
{
   .overlay_lms_data
   {
      "*lms/*.o*"(.srodata .srodata.*)
      "*lms/*.o*"(.rodata .rodata.*)
      "*lms/*.o*"(.sdata .sdata.*)
      "*lms/*.o*"(.data .data.*)
      "*lms/*.o*"(.sbss .sbss.*)
      "*lms/*.o*"(.bss .bss.*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }
   /*
    * Add other algorithms above this block. The combined RSA-ECC image must
    * remain the last algorithm because its LOADADDR + SIZEOF defines the end
    * of the data overlay store LMA (__overlay_store_data_window_end). Adding an
    * algorithm after it would require reworking that computation. The rsa_ecc,
    * ecc and ecies objects share this single overlay image; the rsa_ecc glob is
    * listed ahead of the ecc glob because "*ecc/*" also matches rsa_ecc objects
    * and the linker assigns each input section to the first matching pattern.
    */

   /* Algorithm Last - combined RSA-ECC data */
   .overlay_rsa_ecc_data
   {
      "*rsa_ecc/*.o*"(.srodata .srodata.*)
      "*rsa_ecc/*.o*"(.rodata .rodata.*)
      "*rsa_ecc/*.o*"(.sdata .sdata.*)
      "*rsa_ecc/*.o*"(.data .data.*)
      "*rsa_ecc/*.o*"(.sbss .sbss.*)
      "*rsa_ecc/*.o*"(.bss .bss.*)
      "*ecc/*.o*"(.srodata .srodata.*)
      "*ecc/*.o*"(.rodata .rodata.*)
      "*ecc/*.o*"(.sdata .sdata.*)
      "*ecc/*.o*"(.data .data.*)
      "*ecc/*.o*"(.sbss .sbss.*)
      "*ecc/*.o*"(.bss .bss.*)
      "*ecies/*.o*"(.srodata .srodata.*)
      "*ecies/*.o*"(.rodata .rodata.*)
      "*ecies/*.o*"(.sdata .sdata.*)
      "*ecies/*.o*"(.data .data.*)
      "*ecies/*.o*"(.sbss .sbss.*)
      "*ecies/*.o*"(.bss .bss.*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }
} > tmr_ram_overlay_data_Mem

__overlay_lms_data_size = SIZEOF(.overlay_lms_data);
__overlay_lms_img_size = SIZEOF(.overlay_lms) + SIZEOF(.overlay_lms_data);
__overlay_rsa_ecc_data_size = SIZEOF(.overlay_rsa_ecc_data);
__overlay_rsa_ecc_img_size = SIZEOF(.overlay_rsa_ecc) + SIZEOF(.overlay_rsa_ecc_data);

__overlay_data_window_size = MAX(__overlay_rsa_ecc_data_size, __overlay_lms_data_size);
__overlay_store_data_window_end = LOADADDR(.overlay_rsa_ecc_data) + SIZEOF(.overlay_rsa_ecc_data);

ASSERT(__overlay_rsa_ecc_txt_size <=
      ORIGIN(tmr_ram_instr_cntlr_Mem) + LENGTH(tmr_ram_instr_cntlr_Mem) - __overlay_window_start,
      "overlay rsa_ecc code window does not fit in instruction RAM")
ASSERT(__overlay_lms_txt_size <=
      ORIGIN(tmr_ram_instr_cntlr_Mem) + LENGTH(tmr_ram_instr_cntlr_Mem) - __overlay_window_start,
      "overlay lms code window does not fit in instruction RAM")
ASSERT(__overlay_rsa_ecc_data_size <= LENGTH(tmr_ram_overlay_data_Mem),
      "overlay rsa_ecc data window does not fit in ASU data memory")
ASSERT(__overlay_lms_data_size <= LENGTH(tmr_ram_overlay_data_Mem),
      "overlay lms data window does not fit in ASU data memory")

/*
 * The overlay store boundaries are computed assuming RSA-ECC is the last overlay in the store.
 * Enforce that no overlay image extends past its store boundary, so an overlay added after
 * RSA-ECC cannot overlap the store or run off its end.
 */
ASSERT(__overlay_store_text_window_end >= LOADADDR(.overlay_lms) + SIZEOF(.overlay_lms),
       "overlay data store overlaps a text image; RSA-ECC text must remain the last text overlay")
ASSERT(__overlay_store_data_window_end >= LOADADDR(.overlay_lms_data) + SIZEOF(.overlay_lms_data),
       "overlay data store truncated; RSA-ECC data must remain the last data overlay")

ASSERT(__overlay_store_data_window_end - ORIGIN(ddr_overlay_store_Mem) <= _OVERLAY_STORE_SIZE,
       "overlay images do not fit in the DDR overlay store (_OVERLAY_STORE_SIZE)")
}
