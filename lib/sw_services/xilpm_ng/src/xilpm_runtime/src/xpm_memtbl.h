/******************************************************************************
 * Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/**
 * @file xpm_memtbl.h
 *
 * Memory-range reachability table in PMC RAM.
 *
 * A static, sorted subsystem->memory-region table is populated in PMC RAM
 * (at XPLMI_MEMTBL_ADDR) by the configuration tooling. PLM is the sole writer
 * of the runtime-mutable fields - each row's @c State (Active/Inactive
 * reachability) and the header @c UpdateCounter seqlock - so that consumers
 * (e.g. ASU firmware) see correct region reachability. PLM only produces the
 * table; it does not consume it for its own address validation.
 *
 * Table layout (little-endian words in PMC RAM):
 *   word0 : NumEntries[31:16] | EntrySize[15:12]=0x4 | HeaderSize[11:8]=0x2 |
 *           Version[7:0]
 *   word1 : UpdateCounter (seqlock; odd => mutation in progress)
 *   entry[i] (16 B each, EntrySize = 4 words):
 *     +0x0  Attr     : State[31:28] | MemFlags[27:24] | Reserved[23:16] |
 *                      AccessPerm[15:0]
 *     +0x4  Size     : region size in 4KB units (ActualBytes = Size << 12)
 *     +0x8  AddrHigh : region address [63:32]
 *     +0xC  AddrLow  : region address [31:0]
 *
 ******************************************************************************/

#ifndef XPM_MEMTBL_H_
#define XPM_MEMTBL_H_

#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Cast-ready overlay of a single table row (16 B, EntrySize = 4 words).
 *
 * The table is accessed through XPm_In32()/XPm_Out32() rather than by
 * dereferencing this type (matching the RTCA read idiom), but the type is
 * kept as the canonical description of the on-wire row layout.
 */
typedef struct {
	u32 Attr;     /**< State[31:28]|MemFlags[27:24]|Rsvd[23:16]|AccessPerm[15:0] */
	u32 Size;     /**< Region size in 4KB units (ActualBytes = Size << 12) */
	u32 AddrHigh; /**< Region address [63:32] */
	u32 AddrLow;  /**< Region address [31:0] */
} XPm_MemTblEntry;

/* ------------------------------------------------------------------ */
/* Table geometry                                                      */
/* ------------------------------------------------------------------ */

/** Total bytes reserved for the table in PMC RAM (see XPLMI_MEMTBL_LENGTH). */
#define XPM_MEMTBL_SIZE_BYTES	 (0x800U)

/** Byte offset of header word0 within the table. */
#define XPM_MEMTBL_HDR0_OFFSET	 (0x0U)
/** Byte offset of the UpdateCounter (header word1) within the table. */
#define XPM_MEMTBL_UPDCNT_OFFSET	 (0x4U)
/** Byte offset of the first entry (header is HeaderSize = 2 words = 8 B). */
#define XPM_MEMTBL_ENTRY0_OFFSET	 (0x8U)

/* ------------------------------------------------------------------ */
/* Header word0 fields                                                 */
/* ------------------------------------------------------------------ */

#define XPM_MEMTBL_VERSION_MASK	 (0x000000FFU)	/**< Header word0 Version field mask (bits[7:0]). */
#define XPM_MEMTBL_VERSION_SHIFT	 (0U)		/**< Header word0 Version field shift. */
#define XPM_MEMTBL_HDRSIZE_MASK	 (0x00000F00U)	/**< Header word0 HeaderSize field mask (bits[11:8], words). */
#define XPM_MEMTBL_HDRSIZE_SHIFT	 (8U)		/**< Header word0 HeaderSize field shift. */
#define XPM_MEMTBL_ENTSIZE_MASK	 (0x0000F000U)	/**< Header word0 EntrySize field mask (bits[15:12], words). */
#define XPM_MEMTBL_ENTSIZE_SHIFT	 (12U)		/**< Header word0 EntrySize field shift. */
#define XPM_MEMTBL_NUMENT_MASK	 (0xFFFF0000U)	/**< Header word0 NumEntries field mask (bits[31:16]). */
#define XPM_MEMTBL_NUMENT_SHIFT	 (16U)		/**< Header word0 NumEntries field shift. */

/*
 * Expected constant header nibbles. These fixed values double as a de-facto
 * 12-bit magic for present-detection (absent PMC RAM reads back 0xDEADBEEF,
 * which fails these checks), so no separate table-format change is needed.
 */
#define XPM_MEMTBL_ENTSIZE_VAL	 (0x4U) /**< Entry size in words */
#define XPM_MEMTBL_HDRSIZE_VAL	 (0x2U) /**< Header size in words */
#define XPM_MEMTBL_VERSION_VAL	 (0x1U) /**< Expected table version (spec default 0x01) */
#define XPM_MEMTBL_MAX_ENTRIES	 (64U)	/**< Upper bound on NumEntries */

/* ------------------------------------------------------------------ */
/* Per-entry (Attr word) fields                                        */
/* ------------------------------------------------------------------ */

#define XPM_MEMTBL_ENTRY_ATTR_OFFSET	 (0x0U)		/**< Byte offset of the Attr word within an entry. */
#define XPM_MEMTBL_ENTRY_SIZE_OFFSET	 (0x4U)		/**< Byte offset of the Size word within an entry. */
#define XPM_MEMTBL_ENTRY_ADDRHIGH_OFFSET (0x8U)		/**< Byte offset of the AddrHigh word within an entry. */
#define XPM_MEMTBL_ENTRY_ADDRLOW_OFFSET	 (0xCU)		/**< Byte offset of the AddrLow word within an entry. */

#define XPM_MEMTBL_ENTRY_STATE_MASK	 (0xF0000000U)	/**< Attr word State field mask (bits[31:28]). */
#define XPM_MEMTBL_ENTRY_STATE_SHIFT	 (28U)		/**< Attr word State field shift. */
#define XPM_MEMTBL_ENTRY_MEMFLAGS_MASK	 (0x0F000000U)	/**< Attr word MemFlags field mask (bits[27:24]). */
#define XPM_MEMTBL_ENTRY_MEMFLAGS_SHIFT	 (24U)		/**< Attr word MemFlags field shift. */
#define XPM_MEMTBL_ENTRY_ACCPERM_MASK	 (0x0000FFFFU)	/**< Attr word AccessPerm field mask (bits[15:0]). */

/** Region State value (Attr[31:28]): region not reachable. */
#define XPM_MEMTBL_REGION_INACTIVE		 (0x0U)
#define XPM_MEMTBL_REGION_ACTIVE		 (0x1U)		/**< Region State value (Attr[31:28]): region reachable. */

/** MemFlags value (Attr[27:24]) indicating a PL-fabric region. */
#define XPM_MEMTBL_MEMFLAGS_PL		 (0x1U)

/** Left-shift to convert a Size field (4KB units) into bytes. */
#define XPM_MEMTBL_SIZE_SHIFT	 (12U)

/************************** Function Prototypes ******************************/

u32 XPm_MemTblIsPresent(void);
void XPm_MemTblRecomputeAll(void);
void XPm_MemTblOnDeviceStateChange(u32 DeviceId);
void XPm_MemTblOnPowerEvent(u32 NodeId);
/* Debug helper (no callers by default; PmDbg): dump the table header + entries. */
void XPm_MemTblDump(const char *Label);

#ifdef __cplusplus
}
#endif

#endif /* XPM_MEMTBL_H_ */
