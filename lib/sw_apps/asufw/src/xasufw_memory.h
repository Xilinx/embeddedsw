/**************************************************************************************************
* Copyright (c) 2024 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_memory.h
 *
 * This file contains defines related to ASUFW internal memory.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   07/23/23 Initial release
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       rmv  09/12/25 Added shared memory related macros
 *       rmv  08/21/26 Added memory region access table address and size macros
 *       vm   09/15/26 Added the storage shared by the key vault session and the provisioning
 *                     scratch to the users of the shared crypto pool, and grew the crypto
 *                     algorithm data block and the pool to fit it
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_MEMORY_H_
#define XASUFW_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

/************************************ Constant Definitions ***************************************/

/**
 * ASUFW Memory Address Map (0xEBE00000 - 0xEBE5FFFF):
 *
 * 0xEBE00000 - 0xEBE3FFFF : Instruction RAM		(256KB)
 * 0xEBE40000 - 0xEBE40FFF : RTCA Configuration Memory	(4KB)
 * 0xEBE41000 - 0xEBE48FFF : IPI Shared Memory		(32KB)
 * 0xEBE49000 - 0xEBE49FFF : Debug Log Buffer		(4KB)
 * 0xEBE4A000 - 0xEBE4C7FF : OCP Memory			(10KB)
 * 0xEBE4C800 - 0xEBE507FF : Crypto Algorithm Data Block	(16KB)
 *   0xEBE4C800 - 0xEBE504FF : Shared pool, reused by every overlay serialized algorithm
 *   0xEBE50500 - 0xEBE5070F : Auxiliary buffer for the two commands outside that serialization
 * 0xEBE50800 - 0xEBE509FF : Update Manager		(512B)
 * 0xEBE50A00 - 0xEBE5FFFF : Overlay Data Window	(61.5KB)
 */

/*  ASUFW Data Ram Start and end address related defines */
#define XASUFW_RAM_START_ADDR		(0xEBE00000U)	/**< Start address of ASUFW RAM */

#define XASUFW_RAM_END_ADDR		(0xEBE5FFFFU)	/**< End address of ASUFW RAM */

/*  ASUFW Run Time Configuration Area related register defines */
#define	XASUFW_RTCA_BASEADDR			(0xEBE40000U) /**< ASUFW RTCA Base Address */
#define XASUFW_RTCA_ACCESS_PERM_ADDR	(XASUFW_RTCA_BASEADDR + 0xC00U)
							/**< ASUFW RTCA commands access permissions data address */
#define XASUFW_RTCA_IDENTIFICATION_ADDR		(XASUFW_RTCA_BASEADDR + 0x0U)
							/**< RTCA identification address */
#define XASUFW_RTCA_VERSION_ADDR		(XASUFW_RTCA_BASEADDR + 0x4U)
							/**< RTCA version address */
#define XASUFW_RTCA_SIZE_ADDR			(XASUFW_RTCA_BASEADDR + 0x8U)
							/**< RTCA size address */
#define XASUFW_RTCA_COMM_CHANNEL_INFO_ADDR	(XASUFW_RTCA_BASEADDR + 0x10U)
							/**< RTCA channel information address */

/* Default values of ASUFW Run Time Configuration Area registers */
#define XASUFW_RTCA_IDENTIFICATION_STRING	(0x41435452U)
							/**< RTCA identification string */
#define XASUFW_RTCA_VERSION			(0x1U)
							/**< RTCA version */
#define XASUFW_RTCA_SIZE			(0x1000U)
							/**< RTCA size */
/* Shared memory related macros */
#define XASUFW_SHARED_MEMORY_ADDRESS		(0xEBE41000U)	/**< Reserved address in ASU DATA
								RAM for channel buffers shared
								memory */
#define XASUFW_SHARED_MEMORY_SIZE		(0x8000U)	/**< 32KB size for shared memory */

#define XASUFW_DEBUG_LOG_BUFFER_SIZE	(0x1000U)	/**< Size of debug log buffer in bytes */

#define XASUFW_DEBUG_LOG_BUFFER_ADDR	(0xEBE49000U)	/**< Address of debug log buffer in
								data RAM */

/*
 * Base address and size of the crypto algorithm data block. These must match the
 * ORIGIN and LENGTH of tmr_ram_crypto_data_block_Mem in lscript.ld; keep both in sync
 * when the linker memory map changes.
 *
 * The crypto algorithm data block is shared across multiple crypto algorithms (RSA, key wrap,
 * key manager, KAT, etc.). The usable region is limited to XASUFW_CRYPTO_ALGO_DATA_BLOCK_SIZE;
 * layouts are shared among users and concurrent access across tasks is unsupported.
 * This block can be used in blocking mode only to avoid concurrent access across tasks. If it
 * is required to use it in non-blocking mode, then it should be hanlded using resource protection
 * to make sure that only one crypto algorithm is using it at a time.
 */
#define XASUFW_CRYPTO_ALGO_DATA_BLOCK		(0xEBE4C800U)	/**< Base address of the crypto
								algorithm data block in data RAM */

#define XASUFW_CRYPTO_ALGO_DATA_BLOCK_SIZE	(0x4000U)	/**< Size of the crypto algorithm
								data block in bytes (16KB) */

/*
 * Region at the base of the block, addressed directly as XASUFW_CRYPTO_ALGO_DATA_BLOCK and reused
 * by every algorithm that stages data here: RSA and key wrap work from the base, the key manager
 * stages a generated RSA key pair there and lays XKeyManager_SharedStorage over the base, holding
 * either a key vault export or import session or the scratch buffer provisioning stages its
 * working data in, and ML-DSA and ML-KEM each carve a scratch region out of it to suit themselves,
 * in xmldsa.h and xmlkem.h, with the known-answer tests taking their output buffers from the space
 * just above whichever scratch is in use. Reusing the same bytes is safe because every command
 * that reaches this pool holds XASUFW_OVERLAY for its whole duration, including across a
 * non-blocking suspend, so no two of them are ever in flight together. Each user asserts its own
 * footprint against this size: the largest algorithm carve is ML-DSA-87's scratch plus the
 * known-answer test buffer above it, asserted in xasufw_kat.c, and the size here is set by the
 * provisioning scratch arm of XKeyManager_SharedStorage in xkeymanager_aekv.h, which is larger
 * still and which the key vault session shares rather than reserving RAM of its own.
 *
 * This macro, not the linker script, decides where every user of the block lands: the auxiliary
 * buffer below starts at the block base plus this size, and lscript.ld only mirrors the value to
 * record the layout of the region it reserves. A build failure guards what matters, that this
 * size plus the auxiliary buffer stays inside the block, asserted in xasufw_kat.c. Divergence from
 * the mirror in lscript.ld therefore misleads a reader rather than moving any address.
 */
#define XASUFW_CRYPTO_SHARED_POOL_SIZE		(0x3D00U)	/**< Size of the shared pool at the
								base of the block in bytes;
								mirrored by _CRYPTO_SHARED_POOL_SIZE
								in lscript.ld */

/*
 * Buffer following the shared pool, for the two commands that stage data in the block without
 * holding XASUFW_OVERLAY: the AES counter-measure KAT, which writes a mask and masked data, and
 * the key manager store-key path, which packs an RSA public key as PubExp || Modulus. Either can
 * be dispatched while an overlay holding command sits suspended with live key material in the
 * shared pool, which is why they stay out of it. The two share these bytes: each holds
 * XASUFW_AES for its whole duration and neither ever suspends, so their use of the buffer begins
 * and ends within a single dispatch. The size covers the larger of the two.
 */
#define XASUFW_CRYPTO_AUX_BUF_ADDR		(XASUFW_CRYPTO_ALGO_DATA_BLOCK + \
						XASUFW_CRYPTO_SHARED_POOL_SIZE)
							/**< Base address of the auxiliary buffer */

#define XASUFW_CRYPTO_AUX_BUF_SIZE		(0x210U)	/**< Size of the auxiliary buffer in
								bytes; mirrored by
								_CRYPTO_AUX_BUF_SIZE in
								lscript.ld */

/*
 * Base address and size of the OCP CDO data block. These must match the ORIGIN and
 * LENGTH of tmr_ram_data_ocp_Mem in lscript.ld; keep both in sync when the linker
 * memory map changes.
 */
#define XASUFW_OCP_CDO_DATA_ADDR		(0xEBE4A000U)	/**< User data address */

#define XASUFW_OCP_CDO_DATA_SIZE		(0x2800U)	/**< Size of the OCP CDO data block
								in bytes (10KB) */

/* PLM RunTime Configuration Area Base Address */
#define XASUFW_PLMI_RTCFG_BASEADDR	(0xF2014000U) /**< PLM runtime configuration base address */

/*
 * Memory region access table in PMC RAM. The configuration tool populates the table and PLM
 * maintains it at runtime; ASUFW only reads it. The base address and size must match
 * XPLMI_MEMTBL_ADDR and XPLMI_MEMTBL_LENGTH on the PLM side.
 */
#define XASUFW_MEM_REGN_TABLE_BASE_ADDR	(0xF201C800U)	/**< PMC RAM base address of the memory
							region access table */
#define XASUFW_MEM_REGN_TABLE_SIZE_BYTES (0x800U)	/**< Size of the PMC RAM region reserved
							for the memory region access table */
#define XASUFW_PLM_RTCA_EFUSE_0_IV_ADDR	(XASUFW_PLMI_RTCFG_BASEADDR + 0x384U)
						/**< Address of efuse 0 IV of size 12 bytes */
#define XASUFW_PLM_RTCA_EFUSE_1_IV_ADDR	(XASUFW_PLMI_RTCFG_BASEADDR + 0x390U)
						/**< Address of efuse 1 IV of size 12 bytes */

/* DDR Reserved Area Configuration. */
#define XASUFW_RTCFG_RSVD_DDR_ADDR	(XASUFW_PLMI_RTCFG_BASEADDR + 0x2A8U)
					/**< Reserved DDR address register */
#define XASUFW_RTCFG_RSVD_DDR_SIZE	(XASUFW_PLMI_RTCFG_BASEADDR + 0x2ACU)
					/**< Reserved DDR size register. */
#define XASUFW_2GB_END_ADDR		(0x7FFFFFFFU)	/**< 2GB end address */

#define XASUFW_DDR_RSVD_SIZE		(1024U)	/**< Size of DDR reserved area for ASUFW in bytes. */

/**
 * Keep data structure out of the shared overlay data window so it persists across overlay
 * switches. Any variable belonging to a module that is compiled into an overlay image, but
 * whose state must survive a switch, can be marked with this attribute. Linker script maps
 * the section into loaded, non-overlay memory to preserve their values across overlay switches.
 */
#define XASUFW_OVERLAY_PERSISTENT_DATA	__attribute__((section(".overlay_persistent_data")))

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_MEMORY_H_ */
/** @} */
