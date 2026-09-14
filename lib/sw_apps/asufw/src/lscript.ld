/**************************************************************************************************
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*
 * Both sizes below come from a measured worst case, so each records the operation that produced
 * it and the margin that is left.
 *
 * The stack peak is 27,436 bytes, reached by ML-KEM-1024 key pair generation measured across the
 * whole request window, from task dispatch to the response write. That path is the deepest because
 * the pairwise consistency test inside key pair generation performs a full decapsulation, which
 * places it just above standalone ML-KEM-1024 decapsulation at 27,388 bytes. The deepest ML-DSA
 * path is ML-DSA-87 signing at 24,092 bytes, which sits a consistent 464 bytes above ML-DSA-87 key
 * pair generation, so key pair generation on its own does not bound either algorithm. The peak
 * reproduced byte for byte across three different stack and heap configurations, covering all three
 * parameter sets of both algorithms, key generation, signing, verification, encapsulation and
 * decapsulation, the key address and key ID paths, and the NIST ACVP vectors. It leaves 1,236 bytes
 * spare here.
 *
 * The heap serves a single allocation: SHA3_shake256_inc_init() takes a 224-byte Keccak state at
 * 32-byte alignment, which costs 304 bytes of arena once the alignment slack and the chunk header
 * are counted. That is the only allocation site in the firmware, and its size varies with neither
 * the algorithm nor the parameter set nor the message length, so 1,024 leaves 720 bytes spare.
 * That margin has to hold. The allocation sits behind liboqs' OQS_EXIT_IF_NULLPTR guard, which
 * calls exit(EXIT_FAILURE) on a null return, and both exit and _exit are linked into asufw, so a
 * heap too small to serve it stops the firmware rather than failing the command; the guard and the
 * fprintf stub are described in build_liboqs.sh. Note that a larger heap makes the arena look
 * bigger than it really is without the firmware needing more, because newlib rounds its sbrk()
 * claim up to the next page boundary whenever the room is there; the true demand is only visible
 * once the heap is too small to permit that rounding.
 *
 * Raising either size is bounded by the overlay instruction window that follows the stack, and the
 * space below it depends on which optional modules are built in, so any increase has to be re-linked
 * against the intended module set rather than checked against a trimmed build.
 */
_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x7000;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x400;
_LOG_BUF_SIZE = DEFINED(_LOG_BUF_SIZE) ? _LOG_BUF_SIZE : 0x1000;
_ASUFW_UPDATE_MNG_SIZE = DEFINED(_ASUFW_UPDATE_MNG_SIZE) ? _ASUFW_UPDATE_MNG_SIZE : 0x200;
/*
 * Reserved region at the base of the crypto data block, addressed directly as
 * XASUFW_CRYPTO_ALGO_DATA_BLOCK rather than through a placed section, and reused by every
 * algorithm that stages data there: RSA, RSA padding, key wrap and the key manager key pair
 * generation work from the base, the key manager also lays XKeyManager_SharedStorage over it for a
 * key vault export or import session and for the scratch buffer provisioning works in, while
 * ML-DSA and ML-KEM each carve a scratch out of it, in xmldsa.h and xmlkem.h, for marshalling
 * client buffers and for holding a generated key pair until the caller has stored it, with the
 * known-answer tests taking their output buffers from the space just above. Every user overwrites
 * what the previous one left, which is safe because each of them holds XASUFW_OVERLAY for the
 * whole command, including across a non-blocking suspend, so no two can be in flight together. The
 * size is therefore the largest single carve rather than their sum, which is the provisioning
 * scratch arm of XKeyManager_SharedStorage, ahead of ML-DSA-87's.
 *
 * Every address in this block is computed in C from XASUFW_CRYPTO_ALGO_DATA_BLOCK and
 * XASUFW_CRYPTO_SHARED_POOL_SIZE in xasufw_memory.h, and each user asserts its own footprint
 * against the latter. Nothing defined here is read by C, so the split below serves only to record
 * the layout and to reserve the region. That makes this a fixed mirror of the header rather than a
 * tunable: overriding it would move the reservation while leaving every C address where it was,
 * which is why it is not wrapped in DEFINED().
 */
_CRYPTO_SHARED_POOL_SIZE = 0x3D00;
/*
 * Buffer following it, addressed directly as XASUFW_CRYPTO_AUX_BUF_ADDR, for the two commands that
 * stage data in the block without holding XASUFW_OVERLAY: the AES counter-measure KAT and the key
 * manager store-key path. Either can be dispatched while an overlay holding command sits suspended
 * with live key material in the shared pool, so they must not alias it. They do share these bytes
 * with each other, since both hold XASUFW_AES and neither ever suspends. Mirrors
 * XASUFW_CRYPTO_AUX_BUF_SIZE in xasufw_memory.h on the same terms as the pool size above.
 */
_CRYPTO_AUX_BUF_SIZE = 0x210;
_OVERLAY_DDR_BASE = DEFINED(_OVERLAY_DDR_BASE) ? _OVERLAY_DDR_BASE : 0x09800000;
_OVERLAY_STORE_SIZE = 0x100000;
_OVERLAY_IMG_ALIGN = 64;

/* Define Memories in the system */

MEMORY
{
   tmr_ram_cntlr_Mem : ORIGIN = 0xEBE00000, LENGTH = 0x60000
   /*
    * The 0x40000 instruction RAM is split into a resident part and an overlay window pinned at
    * its top. The window is pinned rather than derived from where resident content happens to
    * end, because the overlay output sections must be matched ahead of the resident .text
    * catch-all to claim the per-function .text sections that LTO emits for ML-DSA and ML-KEM
    * (.text.PQCP_*, .text.XMldsa*, .text.XMlkem*, .text.mld_*, .text.mlk_*), and an input
    * section goes to the first output section that matches it.
    * The two lengths must sum to 0x40000, which the ASSERTs at the end of this file enforce
    * along with checking that every overlay image fits the window; the linker checks that
    * resident content fits below it.
    */
   tmr_ram_instr_cntlr_Mem : ORIGIN = 0xEBE00000, LENGTH = 0x38800
   tmr_ram_overlay_win_Mem : ORIGIN = 0xEBE38800, LENGTH = 0x7800
   tmr_ram_data_Confg_Mem : ORIGIN = 0xEBE40000, LENGTH = 0x1000
   tmr_ram_data_ipi_shared_Mem : ORIGIN = 0xEBE41000, LENGTH = 0x8000
   tmr_ram_data_log_buf_Mem : ORIGIN = 0xEBE49000, LENGTH = _LOG_BUF_SIZE
   tmr_ram_data_ocp_Mem : ORIGIN = 0xEBE4A000, LENGTH = 0x2800
   tmr_ram_crypto_data_block_Mem : ORIGIN = 0xEBE4C800, LENGTH = 0x4000
   update_mgr_Mem : ORIGIN = 0xEBE50800, LENGTH = _ASUFW_UPDATE_MNG_SIZE
   tmr_ram_overlay_data_Mem : ORIGIN = 0xEBE50A00, LENGTH = 0xF600

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

/*
 * Overlay region: VMA(Virtual Memory Address) is the pinned instruction-RAM window,
 * LMA(Load Memory Address) is the DDR address; images are loaded on demand at runtime.
 *
 * This block sits ahead of the resident .text below, and the order is load bearing. An input
 * section goes to the first output section that matches it, and .text ends in a *(.text.*)
 * catch-all, so an overlay defined after it would never see the sections it names. RSA, ECC,
 * ECIES, LMS and SLH-DSA are additionally kept out of .text by its EXCLUDE_FILE list, but ML-DSA
 * and ML-KEM cannot be: they are compiled with LTO, and LTO code reaches the linker as
 * /tmp/cc*.ltrans*.o partitions whose names change on every build, so no file pattern can name
 * them. Ordering is the only thing that routes them here.
 *
 * The resident EXCLUDE_FILE lists name exactly the files the images below claim, so the two stay
 * in step: a file excluded from resident .text but claimed by no image would be left orphaned by
 * the linker, with no error and no ASSERT to catch it. A new source file in ecc, rsa_ecc, ecies,
 * lms or slhdsa therefore has to be added to an image and to the exclusion lists together, or to
 * neither, in which case it stays resident.
 *
 * Both sets of patterns name objects by the file name prefix each directory uses, x<algo>, rather
 * than by directory, because "*ecc/*" also matches the rsa_ecc objects that belong to a different
 * image. A file that departs from that prefix is matched by no pattern and stays resident, so it
 * needs the patterns widened to cover it.
 */
__overlay_window_start = ORIGIN(tmr_ram_overlay_win_Mem);
OVERLAY __overlay_window_start : NOCROSSREFS AT (ORIGIN(ddr_overlay_store_Mem))
{
   .overlay_lms
   {
      KEEP(*(.overlay_lms.header))
      "*lms/xlms*.o*"(.text .text.*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }
   /*
    * ML-DSA and ML-KEM are selected by symbol namespace rather than by object file. Two things
    * rule out an object glob: the archive compiles each parameter set as its own member and
    * reuses basenames across parameter sets and across both algorithms, so a basename selects
    * neither, and LTO dissolves object identity anyway. What does survive LTO is the per-function
    * section names, so PQCP_MLDSA and PQCP_MLKEM select the liboqs exported entry points,
    * XMldsa_ and XMlkem_ select the ASU wrappers, and mld_ and mlk_ select the liboqs file
    * statics. The mld_ and mlk_ globs also collect the clones LTO emits with .constprop and
    * .lto_priv suffixes.
    *
    * The shared liboqs core (SHA3 and Keccak) is deliberately left resident: it is called from
    * both images, and NOCROSSREFS forbids one overlay from referencing another, so it could not
    * live in either without breaking the other.
    *
    * Neither image carries its algorithm's constant tables. They do get per-symbol sections
    * (.rodata.mld_zetas, .rodata.mlk_zetas), but the resident .rodata output section is matched
    * before the data overlay below and its .rodata.* catch-all claims them first, so moving them
    * would mean reordering those two blocks. Their KAT vectors are the exception and do get a
    * data image, because xasufw_kat.c names their section directly.
    */
   .overlay_mldsa
   {
      KEEP(*(.overlay_mldsa.header))
      *(.text.PQCP_MLDSA*)
      *(.text.XMldsa*)
      *(.text.mld_*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }

   .overlay_mlkem
   {
      KEEP(*(.overlay_mlkem.header))
      *(.text.PQCP_MLKEM*)
      *(.text.XMlkem*)
      *(.text.mlk_*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }

   .overlay_slhdsa
   {
      KEEP(*(.overlay_slhdsa.header))
      "*slhdsa/xslhdsa*.o*"(.text .text.*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }

   /*
    * The RSA image holds the RSA primitives and padding. Its two objects are named
    * individually because an "*rsa_ecc/xrsa*" glob would also take the xrsa_ecc and
    * xrsa_eccinfo objects that belong to the ECC image below, and the linker assigns each
    * input section to the first matching pattern. Both spellings of the object name have to
    * be covered: the CMake build emits xrsa.c.obj and the Makefile build emits xrsa.o.
    *
    * The modular exponentiation and RSA key generation routines from the prebuilt
    * libxilasu_mbv.a are selected by symbol namespace for the reason given at ML-DSA: the
    * archive is compiled with LTO, so its members reach the linker as ltrans partitions and
    * no file pattern can name them. rsa5x_keygen has to share this image with rsa5x_rsa
    * rather than stand alone, because it calls RSA_ExpoptQ, crtMul, invMod2_32, mulDigit and
    * rsaRRN, and NOCROSSREFS forbids one overlay from reaching into another. The trailing
    * wildcards collect the .constprop and .lto_priv clones LTO emits. The shared rsa5x_common
    * routines stay resident, as does anything the compiler leaves out of line under a generic
    * name such as add, mod, mul or sub: those are too weakly named to glob without risking an
    * unrelated object, and resident code is callable from an overlay in any case.
    */
   .overlay_rsa
   {
      KEEP(*(.overlay_rsa.header))
      "*rsa_ecc/xrsa.*o*"(.text .text.*)
      "*rsa_ecc/xrsa_padding*.o*"(.text .text.*)
      *(.text.RSA_ExpQ* .text.RSA_ExpCrtQ* .text.RSA_ExpoptQ*)
      *(.text.rsaexp* .text.rsaexpopt* .text.rsaRRN* .text.crtMul*)
      *(.text.mulDigit*)
      *(.text.rsaprvkeyinit_Q* .text.rsaprvkeystep_Q*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }

   /*
    * The key vault blob layers: the AEKV blob format, the chunk walk and the AES-CBC, SHA-384
    * and IV primitives both are built from. All three are pure code over buffers the caller
    * owns in the crypto shared pool, so none carries file scope data and this image has no data
    * half to reload.
    *
    * These are named by object rather than by directory, unlike the algorithms above, because
    * they sit among resident key manager sources that must not follow them into the window. The
    * sequencers in xkeymanager.c stay resident: they call the UDE sign and verify paths and the
    * ECC known answer test, all of which run out of the ECC image, and NOCROSSREFS forbids
    * one overlay from referencing another. A resident caller reaching either image is fine,
    * which is why the split falls there.
    */
   .overlay_keyvault
   {
      KEEP(*(.overlay_keyvault.header))
      "*xkeymanager_aekv*.o*"(.text .text.*)
      "*xkeymanager_chunk*.o*"(.text .text.*)
      "*xkeymanager_crypto*.o*"(.text .text.*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }

   /*
    * Add other algorithms above this block. The ECC image must remain the last
    * algorithm because its LOADADDR + SIZEOF defines the end of the text overlay
    * store LMA (__overlay_store_text_window_end). Adding an algorithm after it
    * would require reworking that computation.
    */

   /*
    * Algorithm Last - ECC text. This image holds every elliptic curve operation, whichever
    * core it runs on: the dedicated P-256/P-384 ECC core, the curve operations layered on the
    * RSA core (xrsa_ecc, xrsa_eccinfo) and ECIES. ECIES belongs here because its only
    * cryptographic dependency is those curve operations, and NOCROSSREFS forbids it from
    * calling into this image from another one.
    *
    * The ECDSA routines from the prebuilt libxilasu_mbv.a join them, selected by symbol
    * namespace because LTO dissolves that archive's object identity. The entry points are
    * listed one by one rather than globbed as Ecdsa_*, because Ecdsa_ClearEccRam and
    * Ecdsa_TrngGenerate live in rsa5x_common and are called from the RSA image as well, so
    * they have to stay resident.
    */
   .overlay_ecc
   {
      KEEP(*(.overlay_ecc.header))
      "*ecc/xecc*.o*"(.text .text.*)
      "*rsa_ecc/xrsa_ecc*.o*"(.text .text.*)
      "*ecies/xecies*.o*"(.text .text.*)
      *(.text.Ecdsa_GenerateSign* .text.Ecdsa_GenerateEdSign* .text.Ecdsa_GeneratePublicKey*)
      *(.text.Ecdsa_VerifySign* .text.Ecdsa_ValidateKey* .text.Ecdsa_ModEccOrder*)
      *(.text.Ecdsa_CDH_Q*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }
} > tmr_ram_overlay_win_Mem

__overlay_lms_txt_size = SIZEOF(.overlay_lms);
__overlay_mldsa_txt_size = SIZEOF(.overlay_mldsa);
__overlay_mlkem_txt_size = SIZEOF(.overlay_mlkem);
__overlay_slhdsa_txt_size = SIZEOF(.overlay_slhdsa);
__overlay_keyvault_txt_size = SIZEOF(.overlay_keyvault);
__overlay_rsa_txt_size = SIZEOF(.overlay_rsa);
__overlay_ecc_txt_size = SIZEOF(.overlay_ecc);

__overlay_window_size = MAX(MAX(MAX(MAX(MAX(MAX(__overlay_ecc_txt_size, __overlay_rsa_txt_size),
                            __overlay_lms_txt_size), __overlay_mldsa_txt_size),
                            __overlay_mlkem_txt_size), __overlay_slhdsa_txt_size),
                            __overlay_keyvault_txt_size);

__overlay_store_text_window_end = LOADADDR(.overlay_ecc) + SIZEOF(.overlay_ecc);

.text : {
   . = ALIGN(2048);
   /* Overlay exclusion list: every resident section carries a copy; keep them in step. */
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .text)
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .text.*)
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
   /* Overlay exclusion list: every resident section carries a copy; keep them in step. */
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .rodata)
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .rodata.*)
   *(.gnu.linkonce.r.*)
   __rodata_end = .;
} > tmr_ram_instr_cntlr_Mem

.srodata : {
   __srodata_start = .;
   /* Overlay exclusion list: every resident section carries a copy; keep them in step. */
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .srodata)
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .srodata.*)
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
   /* Overlay exclusion list: every resident section carries a copy; keep them in step. */
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .data)
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .data.*)
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
   /* Overlay exclusion list: every resident section carries a copy; keep them in step. */
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .sdata)
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .sdata.*)
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
   /* Overlay exclusion list: every resident section carries a copy; keep them in step. */
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .sbss)
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .sbss.*)
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
   /* Overlay exclusion list: every resident section carries a copy; keep them in step. */
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .bss)
   *(EXCLUDE_FILE("*ecc/xecc*.o*" "*rsa_ecc/xrsa.*o*" "*rsa_ecc/xrsa_ecc*.o*"
                 "*rsa_ecc/xrsa_padding*.o*" "*lms/xlms*.o*" "*ecies/xecies*.o*"
                 "*slhdsa/xslhdsa*.o*") .bss.*)
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
   __crypto_shared_pool_start = .;
   . += _CRYPTO_SHARED_POOL_SIZE;
   __crypto_shared_pool_end = .;
   . = ALIGN(8);
   __crypto_aux_buf_start = .;
   . += _CRYPTO_AUX_BUF_SIZE;
   __crypto_aux_buf_end = .;
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
} > tmr_ram_instr_cntlr_Mem

__global_pointer$ = MIN(__sdata_start + 0x800, MAX(__data_start + 0x800, __bss_end - 0x800));
_end = .;

__overlay_data_window_start = ORIGIN(tmr_ram_overlay_data_Mem);
OVERLAY __overlay_data_window_start : NOCROSSREFS AT (__overlay_store_text_window_end)
{
   .overlay_lms_data
   {
      *(.overlay_lms_data)
      "*lms/xlms*.o*"(.srodata .srodata.*)
      "*lms/xlms*.o*"(.rodata .rodata.*)
      "*lms/xlms*.o*"(.sdata .sdata.*)
      "*lms/xlms*.o*"(.data .data.*)
      "*lms/xlms*.o*"(.sbss .sbss.*)
      "*lms/xlms*.o*"(.bss .bss.*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }
   /*
    * ML-DSA and ML-KEM carry no algorithm constant tables. Those do get per-symbol sections
    * (.rodata.mld_zetas, .rodata.mlk_zetas), but the resident .rodata output section is matched
    * ahead of this block and its .rodata.* catch-all claims them first, as noted at the text
    * overlays above. The PQC scratch and staging regions stay resident regardless: they live in
    * the crypto algorithm data block and are addressed by both algorithms and the key manager
    * across overlay switches. What these images do carry is the KAT vectors, which xasufw_kat.c
    * places here by naming the section directly.
    */
   .overlay_mldsa_data
   {
      *(.overlay_mldsa_data)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }

   .overlay_mlkem_data
   {
      *(.overlay_mlkem_data)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }

   .overlay_slhdsa_data
   {
      *(.overlay_slhdsa_data)
      "*slhdsa/xslhdsa*.o*"(.srodata .srodata.*)
      "*slhdsa/xslhdsa*.o*"(.rodata .rodata.*)
      "*slhdsa/xslhdsa*.o*"(.sdata .sdata.*)
      "*slhdsa/xslhdsa*.o*"(.data .data.*)
      "*slhdsa/xslhdsa*.o*"(.sbss .sbss.*)
      "*slhdsa/xslhdsa*.o*"(.bss .bss.*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }

   /* The RSA objects are named individually for the reason given at the text overlays. */
   .overlay_rsa_data
   {
      *(.overlay_rsa_data)
      "*rsa_ecc/xrsa.*o*"(.srodata .srodata.*)
      "*rsa_ecc/xrsa.*o*"(.rodata .rodata.*)
      "*rsa_ecc/xrsa.*o*"(.sdata .sdata.*)
      "*rsa_ecc/xrsa.*o*"(.data .data.*)
      "*rsa_ecc/xrsa.*o*"(.sbss .sbss.*)
      "*rsa_ecc/xrsa.*o*"(.bss .bss.*)
      "*rsa_ecc/xrsa_padding*.o*"(.srodata .srodata.*)
      "*rsa_ecc/xrsa_padding*.o*"(.rodata .rodata.*)
      "*rsa_ecc/xrsa_padding*.o*"(.sdata .sdata.*)
      "*rsa_ecc/xrsa_padding*.o*"(.data .data.*)
      "*rsa_ecc/xrsa_padding*.o*"(.sbss .sbss.*)
      "*rsa_ecc/xrsa_padding*.o*"(.bss .bss.*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }

   /*
    * Add other algorithms above this block. The ECC image must remain the last
    * algorithm because its LOADADDR + SIZEOF defines the end of the data overlay
    * store LMA (__overlay_store_data_window_end). Adding an algorithm after it
    * would require reworking that computation.
    */

   /* Algorithm Last - ECC data */
   .overlay_ecc_data
   {
      *(.overlay_ecc_data)
      "*ecc/xecc*.o*"(.srodata .srodata.*)
      "*ecc/xecc*.o*"(.rodata .rodata.*)
      "*ecc/xecc*.o*"(.sdata .sdata.*)
      "*ecc/xecc*.o*"(.data .data.*)
      "*ecc/xecc*.o*"(.sbss .sbss.*)
      "*ecc/xecc*.o*"(.bss .bss.*)
      "*rsa_ecc/xrsa_ecc*.o*"(.srodata .srodata.*)
      "*rsa_ecc/xrsa_ecc*.o*"(.rodata .rodata.*)
      "*rsa_ecc/xrsa_ecc*.o*"(.sdata .sdata.*)
      "*rsa_ecc/xrsa_ecc*.o*"(.data .data.*)
      "*rsa_ecc/xrsa_ecc*.o*"(.sbss .sbss.*)
      "*rsa_ecc/xrsa_ecc*.o*"(.bss .bss.*)
      "*ecies/xecies*.o*"(.srodata .srodata.*)
      "*ecies/xecies*.o*"(.rodata .rodata.*)
      "*ecies/xecies*.o*"(.sdata .sdata.*)
      "*ecies/xecies*.o*"(.data .data.*)
      "*ecies/xecies*.o*"(.sbss .sbss.*)
      "*ecies/xecies*.o*"(.bss .bss.*)
      . = ALIGN(_OVERLAY_IMG_ALIGN);
   }
} > tmr_ram_overlay_data_Mem

__overlay_lms_data_size = SIZEOF(.overlay_lms_data);
__overlay_lms_img_size = SIZEOF(.overlay_lms) + SIZEOF(.overlay_lms_data);
/* ML-DSA and ML-KEM carry only their KAT vectors as data; see the data overlay block above. */
__overlay_mldsa_data_size = SIZEOF(.overlay_mldsa_data);
__overlay_mldsa_img_size = SIZEOF(.overlay_mldsa) + SIZEOF(.overlay_mldsa_data);
__overlay_mlkem_data_size = SIZEOF(.overlay_mlkem_data);
__overlay_mlkem_img_size = SIZEOF(.overlay_mlkem) + SIZEOF(.overlay_mlkem_data);
__overlay_slhdsa_data_size = SIZEOF(.overlay_slhdsa_data);
__overlay_slhdsa_img_size = SIZEOF(.overlay_slhdsa) + SIZEOF(.overlay_slhdsa_data);
__overlay_rsa_data_size = SIZEOF(.overlay_rsa_data);
__overlay_rsa_img_size = SIZEOF(.overlay_rsa) + SIZEOF(.overlay_rsa_data);
__overlay_ecc_data_size = SIZEOF(.overlay_ecc_data);
__overlay_ecc_img_size = SIZEOF(.overlay_ecc) + SIZEOF(.overlay_ecc_data);
/*
 * The key vault image is text only. Its three translation units hold no file scope data and no
 * constant tables, so a .overlay_keyvault_data output section would be empty, and the linker
 * discards an empty output section, leaving LOADADDR and the load start symbol unresolvable.
 * The size is therefore stated here rather than taken from SIZEOF, and the catalog entry in
 * xoverlay.c pairs it with a null data LMA. XOverlay_Switch() skips the data copy on a zero
 * data size, and still zeroizes the data window so the previous image leaves nothing behind.
 */
__overlay_keyvault_data_size = 0;
__overlay_keyvault_img_size = SIZEOF(.overlay_keyvault);

__overlay_data_window_size = MAX(MAX(MAX(MAX(MAX(MAX(__overlay_ecc_data_size,
                             __overlay_rsa_data_size), __overlay_lms_data_size),
                             __overlay_mldsa_data_size), __overlay_mlkem_data_size),
                             __overlay_slhdsa_data_size), __overlay_keyvault_data_size);
__overlay_store_data_window_end = LOADADDR(.overlay_ecc_data) + SIZEOF(.overlay_ecc_data);

/*
 * Only one image occupies the window at a time, so the window is sized by the largest image
 * rather than by their sum. The ASSERTs below report an image that outgrows the window, as
 * "overlay <algo> code window does not fit in instruction RAM"; a liboqs re-drop or a toolchain
 * update is the likely cause. Growing tmr_ram_overlay_win_Mem is not free: it and
 * tmr_ram_instr_cntlr_Mem tile the 0x40000 of instruction RAM exactly, so the window can only
 * grow by taking space from the resident code below it.
 */
ASSERT(__overlay_ecc_txt_size <= LENGTH(tmr_ram_overlay_win_Mem),
      "overlay ecc code window does not fit in instruction RAM")
ASSERT(__overlay_rsa_txt_size <= LENGTH(tmr_ram_overlay_win_Mem),
      "overlay rsa code window does not fit in instruction RAM")
ASSERT(__overlay_lms_txt_size <= LENGTH(tmr_ram_overlay_win_Mem),
      "overlay lms code window does not fit in instruction RAM")
ASSERT(__overlay_mldsa_txt_size <= LENGTH(tmr_ram_overlay_win_Mem),
      "overlay mldsa code window does not fit in instruction RAM")
ASSERT(__overlay_mlkem_txt_size <= LENGTH(tmr_ram_overlay_win_Mem),
      "overlay mlkem code window does not fit in instruction RAM")
ASSERT(__overlay_slhdsa_txt_size <= LENGTH(tmr_ram_overlay_win_Mem),
      "overlay slhdsa code window does not fit in instruction RAM")
ASSERT(__overlay_keyvault_txt_size <= LENGTH(tmr_ram_overlay_win_Mem),
      "overlay keyvault code window does not fit in instruction RAM")
ASSERT(__overlay_ecc_data_size <= LENGTH(tmr_ram_overlay_data_Mem),
      "overlay ecc data window does not fit in ASU data memory")
ASSERT(__overlay_rsa_data_size <= LENGTH(tmr_ram_overlay_data_Mem),
      "overlay rsa data window does not fit in ASU data memory")
ASSERT(__overlay_lms_data_size <= LENGTH(tmr_ram_overlay_data_Mem),
      "overlay lms data window does not fit in ASU data memory")
ASSERT(__overlay_mldsa_data_size <= LENGTH(tmr_ram_overlay_data_Mem),
      "overlay mldsa data window does not fit in ASU data memory")
ASSERT(__overlay_mlkem_data_size <= LENGTH(tmr_ram_overlay_data_Mem),
      "overlay mlkem data window does not fit in ASU data memory")
ASSERT(__overlay_slhdsa_data_size <= LENGTH(tmr_ram_overlay_data_Mem),
      "overlay slhdsa data window does not fit in ASU data memory")
ASSERT(__overlay_keyvault_data_size <= LENGTH(tmr_ram_overlay_data_Mem),
      "overlay keyvault data window does not fit in ASU data memory")

/*
 * The resident region and the overlay window tile instruction RAM exactly, so a change to
 * either length has to be paid for out of the other.
 */
ASSERT(LENGTH(tmr_ram_instr_cntlr_Mem) + LENGTH(tmr_ram_overlay_win_Mem) == 0x40000,
      "resident region and overlay window must tile the 0x40000 instruction RAM exactly")

ASSERT(__crypto_aux_buf_end <=
      ORIGIN(tmr_ram_crypto_data_block_Mem) + LENGTH(tmr_ram_crypto_data_block_Mem),
      "crypto data block overflow: shared pool plus auxiliary buffer exceeds the reserved region")
/*
 * XASUFW_CRYPTO_AUX_BUF_ADDR is the block base plus the pool size, so the two must abut. The
 * ALIGN in the section above is a no-op while the pool size is a multiple of eight and would
 * otherwise pad the reservation past the address C uses.
 */
ASSERT(__crypto_aux_buf_start == __crypto_shared_pool_end,
      "crypto auxiliary buffer must directly follow the shared pool; alignment padding would put the reservation above XASUFW_CRYPTO_AUX_BUF_ADDR in xasufw_memory.h")

/*
 * The overlay store boundaries are computed assuming ECC is the last overlay in the store.
 * Enforce that no overlay image extends past its store boundary, so an overlay added after
 * ECC cannot overlap the store or run off its end.
 *
 * Each check is skipped when the image is empty. An algorithm turned off in the BSP contributes
 * no input sections, and the linker discards the empty output section, after which LOADADDR no
 * longer resolves to a position inside the store and the comparison would fail on an image that
 * is not there. ECC needs no guard: it is the anchor both boundaries are derived from.
 */
ASSERT(SIZEOF(.overlay_rsa) == 0 ||
       __overlay_store_text_window_end >= LOADADDR(.overlay_rsa) + SIZEOF(.overlay_rsa),
       "overlay text store truncated by .overlay_rsa; ECC text must remain last")
ASSERT(SIZEOF(.overlay_lms) == 0 ||
       __overlay_store_text_window_end >= LOADADDR(.overlay_lms) + SIZEOF(.overlay_lms),
       "overlay text store truncated by .overlay_lms; ECC text must remain last")
ASSERT(SIZEOF(.overlay_mldsa) == 0 ||
       __overlay_store_text_window_end >= LOADADDR(.overlay_mldsa) + SIZEOF(.overlay_mldsa),
       "overlay text store truncated by .overlay_mldsa; ECC text must remain last")
ASSERT(SIZEOF(.overlay_mlkem) == 0 ||
       __overlay_store_text_window_end >= LOADADDR(.overlay_mlkem) + SIZEOF(.overlay_mlkem),
       "overlay text store truncated by .overlay_mlkem; ECC text must remain last")
ASSERT(SIZEOF(.overlay_slhdsa) == 0 ||
       __overlay_store_text_window_end >= LOADADDR(.overlay_slhdsa) + SIZEOF(.overlay_slhdsa),
       "overlay text store truncated by .overlay_slhdsa; ECC text must remain last")
ASSERT(SIZEOF(.overlay_keyvault) == 0 ||
       __overlay_store_text_window_end >= LOADADDR(.overlay_keyvault) + SIZEOF(.overlay_keyvault),
       "overlay text store truncated by .overlay_keyvault; ECC text must remain last")
ASSERT(SIZEOF(.overlay_rsa_data) == 0 ||
       __overlay_store_data_window_end >= LOADADDR(.overlay_rsa_data) + SIZEOF(.overlay_rsa_data),
       "overlay data store truncated by .overlay_rsa_data; ECC data must remain last")
ASSERT(SIZEOF(.overlay_lms_data) == 0 ||
       __overlay_store_data_window_end >= LOADADDR(.overlay_lms_data) + SIZEOF(.overlay_lms_data),
       "overlay data store truncated by .overlay_lms_data; ECC data must remain last")
ASSERT(SIZEOF(.overlay_mldsa_data) == 0 ||
       __overlay_store_data_window_end >= LOADADDR(.overlay_mldsa_data) + SIZEOF(.overlay_mldsa_data),
       "overlay data store truncated by .overlay_mldsa_data; ECC data must remain last")
ASSERT(SIZEOF(.overlay_mlkem_data) == 0 ||
       __overlay_store_data_window_end >= LOADADDR(.overlay_mlkem_data) + SIZEOF(.overlay_mlkem_data),
       "overlay data store truncated by .overlay_mlkem_data; ECC data must remain last")
ASSERT(SIZEOF(.overlay_slhdsa_data) == 0 ||
       __overlay_store_data_window_end >= LOADADDR(.overlay_slhdsa_data) + SIZEOF(.overlay_slhdsa_data),
       "overlay data store truncated by .overlay_slhdsa_data; ECC data must remain last")

ASSERT(__overlay_store_data_window_end - ORIGIN(ddr_overlay_store_Mem) <= _OVERLAY_STORE_SIZE,
       "overlay images do not fit in the DDR overlay store (_OVERLAY_STORE_SIZE)")
}
