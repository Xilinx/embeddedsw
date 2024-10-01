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

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

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
/** Error if readback is disabled in secure boot*/
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

/** Error if failed to store the end offset of begin command. */
#define XPLM_ERR_STORE_END_OFFSET		(0x6U)

/* Error codes for End and Break CDO cmds */
/** Error if begin and end cmds are not paired */
#define XPLM_ERR_INVLD_BEGIN_END_PAIR	(0x2U)

/** Error if end address is not valid */
#define XPLM_ERR_INVLD_END_ADDR		(0x3U)

/**************************** Type Definitions *******************************/

/**
 * Structure to hold the information to transfer the data to Capture Control Unit.
 */
typedef struct {
	u32 SrcAddr; /**< Address to the keyhole payload */
	u32 DestAddr; /**< Address to CCU write stream */
	u32 BaseAddr; /**< Start address of the CCU write stream */
	u32 Len; /**< Length of the payload to write */
	u32 Keyholesize; /**< Keyhole limit */
	u32 Flags; /**< Keyhole transfer DMA flags */
} XPlm_KeyHoleXfrParams;

/***************** Macros (Inline Functions) Definitions *********************/

/** @cond spartanup_plm_internal */
#define XPLM_READBK_INTF_TYPE_SMAP		(0x0U)
#define XPLM_READBK_INTF_TYPE_JTAG		(0x1U)
#define XPLM_READBACK_SRC_MASK			(0xFFU)

/* CCU Stream addresses */
#define XPLM_CCU_RD_STREAM_BASEADDR		(0x04060000U)
#define XPLM_CCU_RD_STREAM_SIZE_BYTES		(0x00010000U)
#define XPLM_CCU_RD_STREAM_SIZE_WORDS		(XPLM_BYTES_TO_WORDS(XPLM_CCU_RD_STREAM_SIZE_BYTES))
#define XPLM_CCU_WR_STREAM_BASEADDR		(0x04070000U)

#define XPLM_READBK_SBI_CFG_MODE		(0x1U)

#define XPLM_CFI_DATA_OFFSET			(4U)

/* Mask poll command flag descriptions */
#define XPLM_MASKPOLL_LEN_EXT			(5U)
#define XPLM_MASKPOLL_FLAGS_MASK		(0x3U)
#define XPLM_MASKPOLL_FLAGS_SUCCESS		(0x1U)
#define XPLM_MASKPOLL_FLAGS_DEFERRED_ERR	(0x2U)
#define XPLM_MASKPOLL_FLAGS_BREAK		(0x3U)
#define XPLM_MASKPOLL_FLAGS_BREAK_LEVEL_MASK	(0xFF000000U)
#define XPLM_MASKPOLL_FLAGS_BREAK_LEVEL_SHIFT	(24U)

/* Define related to break */
#define XPLM_BREAK_LEVEL_MASK			(0xFFU)
/** @endcond */

/************************** Function Prototypes ******************************/
void XPlm_GenericInit(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLM_GENERIC_H */

/** @} end of spartanup_plm_apis group*/
