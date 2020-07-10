/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_generic.h
*
* This is the file which contains .
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/23/2018 Initial release
* 1.01  bsv  04/18/2019 Added support for NPI and CFI readback
*       bsv  05/01/2019 Added support to load CFI bitstreams larger
*						than 64K
*       ma   08/24/2019 Added SSIT commands
* 1.02  bsv  12/13/2019 Added support for NOP and SET commands
*       kc   12/17/2019 Add deferred error mechanism for mask poll
*       bsv  01/09/2020 Changes related to bitstream loading
*       bsv  01/31/2020 Added API to read device ID from hardware
*       ma   03/18/2020 Added event logging code
*       bsv  03/09/2020 Added support for CDO features command
*       bsv  04/04/2020 Code clean up
* 1.03  bsv  06/10/2020 Added SetBoard and GetBoard APIs
*
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XPLMI_GENERIC_H
#define XPLMI_GENERIC_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xplmi_debug.h"
#include "xil_io.h"
#include "xil_assert.h"
#include "xplmi_modules.h"
#include "xplmi_dma.h"
#include "xplmi_cmd.h"

/************************** Constant Definitions *****************************/
enum {
	XPLMI_ERR_MASKPOLL = 0x10U,
	XPLMI_ERR_MASKPOLL64, /**< 0x11U */
	XPLMI_ERR_CMD_NOT_SUPPORTED, /**< 0x12U */
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_SBI_DEST_ADDR			(0xFFFFFFFFFFFFFFFFL)
#define XPLMI_READBK_INTF_TYPE_SMAP		(0x0U)
#define XPLMI_READBK_INTF_TYPE_JTAG		(0x1U)
#define XPLMI_READBK_INTF_TYPE_DDR		(0x2U)
#define XPLMI_SET_CHUNK_SIZE			(128U)
/* Max board name length supported is 256 bytes */
#define XPLMI_MAX_NAME_LEN			(256U)
#define XPLMI_MAX_NAME_WORDS			(XPLMI_MAX_NAME_LEN / XPLMI_WORD_LEN)

/* Mask poll command flag descriptions */
#define XPLMI_MASKPOLL_LEN_EXT			(5U)
#define XPLMI_MASKPOLL64_LEN_EXT		(6U)
#define XPLMI_MASKPOLL_FLAGS_MASK		(0x3U)
#define XPLMI_MASKPOLL_FLAGS_ERR		(0x0U)
#define XPLMI_MASKPOLL_FLAGS_SUCCESS		(0x1U)
#define XPLMI_MASKPOLL_FLAGS_DEFERRED_ERR	(0x2U)
#define XPLMI_PLM_GENERIC_CMD_ID_MASK		(0xFFU)
#define XPLMI_PLM_MODULES_FEATURES_VAL		(0x00U)
#define XPLMI_PLM_GENERIC_DEVICE_ID_VAL		(0x12U)
#define XPLMI_PLM_GENERIC_EVENT_LOGGING_VAL	(0x13U)
#define XPLMI_PLM_MODULES_SET_BOARD_VAL		(0x14U)
#define XPLMI_PLM_MODULES_GET_BOARD_VAL		(0x15U)

/************************** Function Prototypes ******************************/
void XPlmi_GenericInit(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_GENERIC_H */
