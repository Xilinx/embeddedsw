/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_generic.h
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  ng   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/
#ifndef XPLM_GENERIC_H
#define XPLM_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_debug.h"
#include "xplm_cdo.h"
#include "xplm_util.h"

/************************** Constant Definitions *****************************/

/* Error codes for CFrame readback */
/** Error if readback is disabled in secure */
#define XPLM_ERR_RDBK_DISABLED		(0x2U)

/** Error if the invalid interface type is selected for readback */
#define XPLM_ERR_RDBK_INVALID_INFR_SEL	(0x3U)

/** Error if failed to transfer CFI read cmd paylod to CCU. */
#define XPLM_ERR_RDBK_PAYLOAD_TO_CCU	(0x4U)

/** Error if failed to readback the bitstream from CCU. */
#define XPLM_ERR_RDBK_CCU_READ		(0x5U)

/** Error if failed to read the readback data from the SBI interface. */
#define XPLM_ERR_RDBK_READ_TIMEOUT	(0x6U)


/* Error codes for Set CDO cmd */
/** Error if failed to set the memory with the value. */
#define XPLM_ERR_SET_MEM		(0x2U)


/* Error codes for Keyhole transfer */
/** Error if failed to transfer the payload to CCU. */
#define XPLM_ERR_KEYHOLE_XFER		(0x2U)


/* Error codes for Log String */
/** Error if the log string is greater than 256 characters. */
#define XPLM_ERR_MAX_LOG_STR_LEN	(0x2U)

/** Error if failed to initialize the log string buffer to zero */
#define XPLM_ERR_LOG_STR_BUF_CLR	(0x3U)

/** Error if failed to copy the log string from payload to buffer */
#define XPLM_ERR_COPY_LOG_STR_TO_BUF	(0x4U)


/* Error codes for Begin */
/** Error if the number of nested begin exceeds the limit of 10. */
#define XPLM_ERR_MAX_NESTED_BEGIN		(0x2U)

/** Error if failed to initialize the string buffer to zero */
#define XPLM_ERR_BEGIN_STR_BUF_CLR		(0x3U)

/** Error if failed to copy the string from payload to buffer */
#define XPLM_ERR_BEGIN_STR_COPY_TO_BUF		(0x4U)

/** Error if failed to store the end address */
#define XPLM_ERR_BEGIN_END_ADDR_STORE		(0x5U)


/* Error codes for End and Break CDO cmds */
/** Error if begin and end cmds are not paired */
#define XPLM_ERR_INVLD_BEGIN_END_PAIR	(0x2U)

/** Error if end address is not valid */
#define XPLM_ERR_INVLD_END_ADDR		(0x3U)

/**************************** Type Definitions *******************************/

typedef struct {
	u32 SrcAddr;
	u32 DestAddr;
	u32 BaseAddr;
	u32 Len;
	u32 Keyholesize;
	u32 Flags;
	int (*Func) (u32 SrcAddr, u32 DestAddr, u32 Len, u32 Flags);
} XPlm_KeyHoleXfrParams;

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLM_READBK_INTF_TYPE_SMAP		(0x0U)
#define XPLM_READBK_INTF_TYPE_JTAG		(0x1U)
#define XPLM_READBACK_SRC_MASK			(0xFFU)

/* CCU Stream addresses */
#define XPLM_CCU_RD_STREAM_BASEADDR		(0x04060000U)
#define XPLM_CCU_RD_STREAM_SIZE_BYTES		(0x00010000U)
#define XPLM_CCU_RD_STREAM_SIZE_WORDS		(XPLM_BYTES_TO_WORDS(XPLM_CCU_RD_STREAM_SIZE_BYTES))
#define XPLM_CCU_WR_STREAM_BASEADDR		(0x04070000U)
#define XPLM_CCU_WR_STREAM_SIZE			(XPLM_CCU_RD_STREAM_SIZE)

#define XPLM_READBK_SBI_CFG_MODE		(0x1U)

#define XPLM_MASK_POLL_MIN_TIMEOUT		(1000000U)
#define XPLM_MAXOUT_CMD_MIN_VAL			(1U)
#define XPLM_MAXOUT_CMD_DEF_VAL			(8U)
#define XPLM_CFI_DATA_OFFSET			(4U)
#define XPLM_SIXTEEN_BYTE_MASK			(0xFU)
#define XPLM_NUM_BITS_IN_WORD			(32U)

/* Mask poll command flag descriptions */
#define XPLM_MASKPOLL_LEN_EXT			(5U)
#define XPLM_MASKPOLL_FLAGS_MASK		(0x3U)
#define XPLM_MASKPOLL_FLAGS_SUCCESS		(0x1U)
#define XPLM_MASKPOLL_FLAGS_DEFERRED_ERR	(0x2U)
#define XPLM_MASKPOLL_FLAGS_BREAK		(0x3U)
#define XPLM_MASKPOLL_FLAGS_BREAK_LEVEL_MASK	(0xFF000000U)
#define XPLM_MASKPOLL_FLAGS_BREAK_LEVEL_SHIFT	(24U)
/* if bit 31 in flags is set, then disable minimal timeout. */
#define XPLM_MASKPOLL_FLAGS_DISABLE_MINIMAL_TIMEOUT	(XPLM_BIT(31))
#define XPLM_MASK_POLL_32BIT_TYPE		(0U)

/* Define related to break */
#define XPLM_BREAK_LEVEL_MASK			(0xFFU)

/************************** Function Prototypes ******************************/
void XPlm_GenericInit(void);
int XPlm_GetJumpOffSet(XPlm_Cmd *Cmd, u32 Level);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLM_GENERIC_H */
