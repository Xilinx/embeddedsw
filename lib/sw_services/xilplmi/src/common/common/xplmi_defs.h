/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_defs.h
 *
 * This file contains the xilplmi API IDs
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *       pre  07/10/24 Added support for configure secure communication command
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XPLMI_DEFS_H
#define XPLMI_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

#include "xil_printf.h"
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/* Macros for command ids */
#define XPLMI_FEATURES_CMD_ID		    (0U) /**< command id for features */
#define XPLMI_MASK_POLL_CMD_ID		    (1U) /**< command id for mask poll */
#define XPLMI_MASK_WRITE_CMD_ID		    (2U) /**< command id for mask write */
#define XPLMI_WRITE_CMD_ID              (3U) /**< command id for write */
#define XPLMI_DELAY_CMD_ID              (4U) /**< command id for delay */
#define XPLMI_DMA_WRITE_CMD_ID		    (5U) /**< command id for DMA write */
#define XPLMI_MASK_POLL64_CMD_ID	    (6U) /**< command id for mask poll 64 */
#define XPLMI_MASK_WRITE64_CMD_ID	    (7U) /**< command id for  mask write 64 */
#define XPLMI_WRITE64_CMD_ID		    (8U) /**< command id for write 64 */
#define XPLMI_DMA_XFER_CMD_ID		    (9U) /**< command id for  DMA transfer */
#define XPLMI_INIT_SEQ_CMD_ID		    (10U) /**< command id for Init sequence */
#define XPLMI_CFI_READ_CMD_ID		    (11U) /**< command id for CFI read */
#define XPLMI_SET_CMD_ID                (12U) /**< command id for set */
#define XPLMI_WRITE_KEYHOLE_CMD_ID	    (13U) /**< command id for write keyhole */
#define XPLMI_SSIT_SYNC_SLAVES_CMD_ID	(14U) /**< command id for SSIT slaves sync */
#define XPLMI_SSIT_SYNC_MASTER_CMD_ID	(15U) /**< command id for SSIT master sync */
#define XPLMI_SSIT_WAIT_SLAVES_CMD_ID	(16U) /**< command id for SSIT slaves wait */
#define XPLMI_NOP_CMD_ID                (17U) /**< command id for NOP */
#define XPLMI_GET_DEVICE_CMD_ID		    (18U) /**< command id for get device */
#define XPLMI_EVENT_LOGGING_CMD_ID	    (19U) /**< command id for event logging */
#define XPLMI_SET_BOARD_CMD_ID		    (20U) /**< command id for set board */
#define XPLMI_GET_BOARD_CMD_ID		    (21U) /**< command id for get board */
#define XPLMI_SET_WDT_PARAM_CMD_ID	    (22U) /**< command id for set WDT param  */
#define XPLMI_LOG_STR_CMD_ID		    (23U) /**< command id for Log str */
#define XPLMI_LOG_ADDR_CMD_ID		    (24U) /**< command id for Log address */
#define XPLMI_MARKER_CMD_ID		        (25U) /**< command id for marker */
#define XPLMI_PROC_CMD_ID		        (26U) /**< command id for proc */
#define XPLMI_BEGIN_CMD_ID		        (27U) /**< command id for begin */
#define XPLMI_END_CMD_ID		        (28U) /**< command id for end */
#define XPLMI_BREAK_CMD_ID		        (29U) /**< command id for break */
#define XPLMI_OT_CHECK_CMD_ID		    (30U) /**< command id for OT check */
#define XPLMI_PSM_SEQUENCE_CMD_ID	    (31U) /**< command id for PSM sequence */
#define XPLMI_INPLACE_PLM_UPDATE_CMD_ID	(32U) /**< command id for inplace plm update */
#define XPLMI_SCATTER_WRITE_CMD_ID	    (33U) /**< command id for scatter write */
#define XPLMI_SCATTER_WRITE2_CMD_ID	    (34U) /**< command id for scatter write 2 */
#define XPLMI_TAMPER_TRIGGER_CMD_ID	    (35U) /**< command id for tamper trigger */
#define XPLMI_SET_FIPS_MASK_CMD_ID	    (36U) /**< command id for FIPS mask */
#define XPLMI_SET_IPI_ACCESS_CMD_ID	    (37U) /**< command id for set ipi access */
#define XPLMI_RUN_PROC_CMD_ID		    (38U) /**< command id for run proc */
#define XPLMI_LIST_SET_CMD_ID		    (39U) /**< command id for list set */
#define XPLMI_LIST_WRITE_CMD_ID		    (40U) /**< command id for list write */
#define XPLMI_LIST_MASK_WRITE_CMD_ID	(41U) /**< command id for list mask write */
#define XPLMI_LIST_MASK_POLL_CMD_ID	    (42U) /**< command id for list mask poll */
#define XPLMI_CONFIG_SECCOMM_CMD_ID     (43U) /**< command id for secure communication configuration */
#define XPLMI_CDO_END_CMD_ID		    (0xFFU) /**< command id for CDO end */

#define XPLMI_HEADER_LEN_0			(0U) /**< Header Length 0 */
#define XPLMI_HEADER_LEN_1			(1U) /**< Header Length 1 */
#define XPLMI_HEADER_LEN_2			(2U) /**< Header Length 2 */
#define XPLMI_HEADER_LEN_3			(3U) /**< Header Length 3 */
#define XPLMI_HEADER_LEN_4			(4U) /**< Header Length 4 */

#define XPLMI_KEY_SIZE_WORDS (8U) /**< Key size in words */
#define XPLMI_KEY_SIZE_BYTES (32U) /**< Key size in bytes */

/************************************** Type Definitions *****************************************/
typedef struct {
	u32 IVs[XPLMI_KEY_SIZE_WORDS]; /**< IV1 and IV2 */
    u32 Key[XPLMI_KEY_SIZE_WORDS]; /**< Key */
}XPlmi_IVsandKey;

typedef struct {
	u32 SlrIndex; /**< SLR index number */
    XPlmi_IVsandKey IVsandKey; /**< Ivs and key */
}XPlmi_SsitSecComm;

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_DEFS_H */
