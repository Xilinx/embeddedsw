/******************************************************************************
 * Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

#ifndef XPM_ACCESS_H_
#define XPM_ACCESS_H_

#include "xpm_node.h"
#include "xpm_power.h"
#include "xpm_regnode.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPm_NodeAccess XPm_NodeAccess;
typedef struct XPm_NodeAper XPm_NodeAper;

/* Minimum number of arguments in node access command */
#define NODE_ACCESS_ARG_MIN_LEN 		(3U)

/* Size of node access table*/
#define XPM_NODE_ACCESS_TABLE_SIZE		(1U)

/* Size of bit fields for XPm_NodeAper struct */
#define NODE_APER_OFFSET_BIT_FIELD_SIZE		(20U)
#define NODE_APER_SIZE_BIT_FIELD_SIZE		(8U)
#define NODE_APER_ACCESS_BIT_FIELD_SIZE		(4U)
#define NODE_APER_CLEAR_TYPE_BIT_FIELD_SIZE	(2U)	/**< ClearType field width in bits */
#define NODE_APER_EFLAG0_BIT_FIELD_SIZE		(1U)	/**< EFlag0 field width in bits */
#define NODE_APER_EFLAG1_BIT_FIELD_SIZE		(1U)	/**< EFlag1 field width in bits */

/* SET_NODE_ACCESS Masks/Shifts */
#define NODE_APER_OFFSET_SHIFT			(0U)
#define NODE_APER_OFFSET_MASK			(0xFFFFFU)

#define NODE_APER_SIZE_SHIFT			(24U)
#define NODE_APER_SIZE_MASK			(0xFFU)

#define NODE_APER_ACCESS_SHIFT			(0U)
#define NODE_APER_ACCESS_MASK			(0xFU)

/*
 * Extended attributes packed in the free bits of SET_NODE_ACCESS word2
 * (Args[i+1]). Layout: [7] flag1  [6] flag0  [5:4] clear_type  [3:0] access.
 * Legacy CDOs leave these bits 0 (clear_type = CLEAR_NONE, no event flags),
 * so the parser remains backward compatible.
 */
#define NODE_APER_CLEARTYPE_SHIFT		(4U)	/**< clear_type field shift in word2 */
#define NODE_APER_CLEARTYPE_MASK		(0x3U)	/**< clear_type field mask (2 bits) */
#define NODE_APER_EFLAG0_SHIFT			(6U)	/**< flag0 bit position in word2 */
#define NODE_APER_EFLAG1_SHIFT			(7U)	/**< flag1 bit position in word2 */
#define NODE_APER_FLAG_MASK			(0x1U)	/**< single-bit mask for flag0/flag1 */

/*
 * SET_NODE_ACCESS word2 (Args[i+1]) uses only bits [7:0] (access, clear_type,
 * flag0, flag1). Bits [31:8] are reserved and MUST be zero; the parser rejects
 * a non-zero reserved field so the contract is self-enforcing regardless of the
 * CDO generator (no longer relying on generators to zero-fill the free bits).
 */
#define NODE_APER_WORD2_RSVD_MASK		(0xFFFFFF00U)	/**< reserved bits; must be 0 */

#define NODE_APER_OFFSET(Ap)			(((Ap) >> NODE_APER_OFFSET_SHIFT) & NODE_APER_OFFSET_MASK)
#define NODE_APER_SIZE(Ap)			(((Ap) >> NODE_APER_SIZE_SHIFT) & NODE_APER_SIZE_MASK)
#define NODE_APER_ACCESS(Ap)			(((Ap) >> NODE_APER_ACCESS_SHIFT) & NODE_APER_ACCESS_MASK)
/** Extract clear_type field from a SET_NODE_ACCESS word2 value */
#define NODE_APER_CLEARTYPE(Ap)			(((Ap) >> NODE_APER_CLEARTYPE_SHIFT) & NODE_APER_CLEARTYPE_MASK)
/** Extract event-flag0 bit from a SET_NODE_ACCESS word2 value */
#define NODE_APER_EFLAG0(Ap)			(((Ap) >> NODE_APER_EFLAG0_SHIFT) & NODE_APER_FLAG_MASK)
/** Extract event-flag1 bit from a SET_NODE_ACCESS word2 value */
#define NODE_APER_EFLAG1(Ap)			(((Ap) >> NODE_APER_EFLAG1_SHIFT) & NODE_APER_FLAG_MASK)

/*
 * GET_EVENT_STATUS_REGS request "Flags" word bit definitions. These live inside
 * the request buffer, not in the IPI arg words.
 */
#define XPM_EVENT_STATUS_FLAG_CLEAR_AFTER_READ	(0x1U << 0U)	/**< Clear ISR after read */
#define XPM_EVENT_STATUS_FLAG_READ_NODE_LIST	(0x1U << 1U)	/**< Return flat node-id list */
#define XPM_EVENT_STATUS_FLAG_READ_NODES	(0x1U << 2U)	/**< Read request-specified ranges */
#define XPM_EVENT_STATUS_FLAG_EFLAG0		(0x1U << 3U)	/**< Select EFlag0 apertures */
#define XPM_EVENT_STATUS_FLAG_EFLAG1		(0x1U << 4U)	/**< Select EFlag1 apertures */

/** All defined GET_EVENT_STATUS_REGS request flag bits; any bit outside is rejected */
#define XPM_EVENT_STATUS_FLAG_VALID_MASK	\
	(XPM_EVENT_STATUS_FLAG_CLEAR_AFTER_READ |	\
	 XPM_EVENT_STATUS_FLAG_READ_NODE_LIST |		\
	 XPM_EVENT_STATUS_FLAG_READ_NODES |		\
	 XPM_EVENT_STATUS_FLAG_EFLAG0 |			\
	 XPM_EVENT_STATUS_FLAG_EFLAG1)

/*
 * Maximum GET_EVENT_STATUS_REGS reply the PLM stages per call, in 32-bit words,
 * used to size the reply staging buffer (xpm_runtime_api.c). versal_2ve_2vm worst case is
 * 5 controllers x (MAIN 34 + NOC 21) + reply word0 = 276 words (~1.1 KB);
 * 320 gives ~16% headroom. A caller request larger than this
 * is clamped and reported via XST_BUFFER_TOO_SMALL so it can resize and retry.
 * The deferred ISR-clear store (xpm_access.c) is sized SEPARATELY from a
 * hardware bound, not from this value. Overridable via a build flag.
 */
#ifndef XPM_EVENT_STATUS_REPLY_MAX_WORDS
#define XPM_EVENT_STATUS_REPLY_MAX_WORDS	(320U)	/**< Reply staging cap in 32-bit words */
#endif

/*
 * ReadNodes range descriptor bit layout (request "Args" words):
 *   { rsvd[31:16], size[15:10], offset[9:0] }
 *
 * Offset-unit convention: the REQUEST descriptor carries a 10-bit
 * *word* offset (units of 4 bytes) giving a 4 KB reach from the RegNode base;
 * the REPLY descriptor and SET_NODE_ACCESS aperture carry a 20-bit *byte*
 * offset. All DDRMC registers sit < 4 KB from their RegNode base (ECCR 0xEB0,
 * ADEC 0x40), so the 10-bit word offset suffices; widen only if a future node
 * needs a > 4 KB reach.
 */
#define XPM_READNODES_OFFSET_SHIFT		(0U)	/**< READ_NODES offset field shift */
#define XPM_READNODES_OFFSET_MASK		(0x3FFU)	/**< READ_NODES offset mask */
#define XPM_READNODES_SIZE_SHIFT		(10U)	/**< READ_NODES size field shift */
#define XPM_READNODES_SIZE_MASK			(0x3FU)	/**< READ_NODES size mask */
/** Extract word offset from a READ_NODES range descriptor */
#define XPM_READNODES_OFFSET(Arg)		(((Arg) >> XPM_READNODES_OFFSET_SHIFT) & XPM_READNODES_OFFSET_MASK)
/** Extract size (register count) from a READ_NODES range descriptor */
#define XPM_READNODES_SIZE(Arg)			(((Arg) >> XPM_READNODES_SIZE_SHIFT) & XPM_READNODES_SIZE_MASK)

/**
 * Node access types.
 */
typedef enum {
	ACCESS_RESERVED = 0U,		/**< 0x0 */
	ACCESS_ANY_RO,			/**< 0x1 */
	ACCESS_ANY_RW,			/**< 0x2 */
	ACCESS_SEC_RO,			/**< 0x3 */
	ACCESS_SEC_RW,			/**< 0x4 */
	ACCESS_SEC_NS_SUBSYS_RO,	/**< 0x5 */
	ACCESS_SEC_NS_SUBSYS_RW,	/**< 0x6 */
	ACCESS_SEC_SUBSYS_RO,		/**< 0x7 */
	ACCESS_SEC_SUBSYS_RW,		/**< 0x8 */

	/* Always keep this enum last */
	ACCESS_TYPE_MAX,		/**< 0x9 */
} XPm_NodeAccessTypes;

/**
 * Aperture post-read clear behavior (used by GET_EVENT_STATUS_REGS ClearAfterRead).
 */
enum {
	CLEAR_NONE = 0U,		/**< 0x0 - no clear */
	CLEAR_W1TC = 1U,		/**< 0x1 - write-1-to-clear (write back snapshot) */
	CLEAR_W0TC = 2U,		/**< 0x2 - write-0-to-clear (write back inverse) */

	/* Always keep this enum last */
	CLEAR_TYPE_MAX,			/**< 0x3 - reserved (reject) */
};

/**
 * Node aperture access attributes for a node.
 */
struct XPm_NodeAper {
	u32 Offset : NODE_APER_OFFSET_BIT_FIELD_SIZE;	/**< Aperture offset */
	u32 Size : NODE_APER_SIZE_BIT_FIELD_SIZE;	/**< Aperture size */
	u32 Access : NODE_APER_ACCESS_BIT_FIELD_SIZE;	/**< Aperture access policy */
	u32 ClearType : NODE_APER_CLEAR_TYPE_BIT_FIELD_SIZE;	/**< Post-read clear type (CLEAR_* values) */
	u32 EFlag0 : NODE_APER_EFLAG0_BIT_FIELD_SIZE;	/**< Event-status selection flag 0 */
	u32 EFlag1 : NODE_APER_EFLAG1_BIT_FIELD_SIZE;	/**< Event-status selection flag 1 */
	XPm_NodeAper *NextAper;		/**< Next aperture of the same node */
};

/**
 * Node access table.
 */
struct XPm_NodeAccess {
	u32 Id;				/**< Node Id for given node */
	XPm_NodeAper *Aperture;		/**< Node aperture attributes */
	XPm_NodeAccess *NextNode;	/**< Link to next entry in the table */
};

/************************** Function Prototypes ******************************/
/* Add regnodes to pm database */
void XPmAccess_RegnodeInit(XPm_RegNode *RegNode,
			   u32 NodeId, u32 BaseAddress, XPm_Power *Power);

/* Add requirements on a regnode from different subsystems */
XStatus XPmAccess_AddRegnodeRequirement(u32 SubsystemId, u32 RegnodeId);

/* Set node access handler (idempotent; updates in place) */
XStatus XPmAccess_UpdateTable(const u32 *Args, u32 NumArgs);

/* Debug only function meant for printing regnodes and "Node Access Table" */

/* IOCTL handlers */
XStatus XPmAccess_ReadReg(u32 SubsystemId,
			  u32 DeviceId,
			  pm_ioctl_id IoctlId,
			  u32 Offset, u32 Count,
			  u32 *const Response, u32 CmdType);

XStatus XPmAccess_MaskWriteReg(u32 SubsystemId,
			       u32 DeviceId,
			       pm_ioctl_id IoctlId,
			       u32 Offset, u32 Mask, u32 Value,
			       u32 CmdType);

/**
 * @brief GET_EVENT_STATUS_REGS core: source-trace read of the nodes linked to
 * {EventNodeId, Mask}. Post-read ISR clears are only recorded (and their write
 * permission verified) here; they are applied by XPmAccess_ApplyEventClears()
 * after the reply has been delivered, so a clear failure can never destroy the
 * snapshot.
 */
XStatus XPmAccess_GetEventStatusRegs(u32 SubsystemId, u32 EventNodeId, u32 Mask,
				     u32 Flags, const u32 *ReqArgs, u32 ReqArgCnt,
				     u32 *Reply, u32 *ReplyUsed, u32 CmdType);

/* Apply the ISR clears deferred by the last XPmAccess_GetEventStatusRegs() call */
XStatus XPmAccess_ApplyEventClears(u32 SubsystemId, u32 CmdType);

#ifdef __cplusplus
}
#endif

#endif /* XPM_ACCESS_H_ */
