/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_modules.h
*
* This is the header file which contains definitions for the modules
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/20/2018 Initial release
* 1.01  bsv  04/04/2020 Code clean up
* 1.02  rama 08/12/2020 Added STL module ID
*       bm   10/14/2020 Code clean up
* 1.03  bm   02/17/2021 Added const to CmdAry
*       ma   03/04/2021 Added CheckIpiAccessHandler handler to XPlmi_Module
*       rama 03/22/2021 Added STL module ID to support STL execution
*       kal  03/30/2021 Added XilSecure module ID
* 1.04  bsv  07/16/2021 Fix doxygen warnings
*       kal  07/17/2021 Added XilNvm module ID
*       bsv  08/02/2021 Removed incorrect comment
* 1.05  kpt  01/04/2022 Added XilPuf module ID
* 1.06  bsv  06/03/2022 Add CommandInfo to a separate section in elf
*       SK   06/14/2022 Updated macro XPLMI_EXPORT_CMD to create
*                       unique variable names
*       bm   07/06/2022 Refactor versal and versal_net code
*       ma   07/08/2022 Move ScatterWrite and ScatterWrite2 APIs to common code
*       jd   08/11/2022 Increase command argument count macros from 6 to 12
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_MODULES_H
#define XPLMI_MODULES_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplmi_cmd.h"
#include "xplmi_plat.h"
#include "xparameters.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/
#define XPLMI_MAX_MODULES			(13U)
#define XPLMI_MODULE_GENERIC_ID			(1U)
#define XPLMI_MODULE_XILPM_ID			(2U)
#define XPLMI_MODULE_SEM_ID			(3U)
#define XPLMI_MODULE_XILSECURE_ID		(5U)
#define XPLMI_MODULE_XILPSM_ID			(6U)
#define XPLMI_MODULE_LOADER_ID			(7U)
#define XPLMI_MODULE_ERROR_ID			(8U)
#define XPLMI_MODULE_STL_ID			(10U)
#define XPLMI_MODULE_XILNVM_ID			(11U)
#define XPLMI_MODULE_XILPUF_ID			(12U)
#define XPLMI_MODULE_COMMAND(FUNC)		{ (FUNC) }

/**************************** Type Definitions *******************************/
typedef struct {
	int (*Handler)(XPlmi_Cmd *Cmd);
} XPlmi_ModuleCmd;

typedef struct {
	u32 Id;
	const XPlmi_ModuleCmd *CmdAry;
	u32 CmdCnt;
	int (*CheckIpiAccess)(u32 CmdId, u32 IpiReqType);
#ifdef VERSAL_NET
	XPlmi_UpdateHandler_t UpdateHandler;
#endif
} XPlmi_Module;

typedef struct {
	u8 CmdId;
	u8 ModuleId;
	u16 Reserved;
	u8 MinArgCnt;
	u8 MaxArgCnt;
	u16 AlsoReserved;
} XPlmi_CmdInfo;

/***************** Macros (Inline Functions) Definitions *********************/
/* Macros for command ids */
#define XPLMI_EXPORT_CMD(CmdIdVal, ModuleIdVal, MinArgCntVal, MaxArgCntVal) {	\
		static volatile const XPlmi_CmdInfo CmdInfo_##ModuleIdVal##_##CmdIdVal __attribute__((unused)) \
		__attribute__((section (".xplm_modules"))) = {	\
		.CmdId = CmdIdVal,\
		.ModuleId = ModuleIdVal,\
		.Reserved = 0U,\
		.MinArgCnt = MinArgCntVal,\
		.MaxArgCnt = MaxArgCntVal,\
		.AlsoReserved = 0U}; \
}

/* Macros for command arguments count */
#define XPLMI_CMD_ARG_CNT_ZERO		(0U)
#define XPLMI_CMD_ARG_CNT_ONE		(1U)
#define XPLMI_CMD_ARG_CNT_TWO		(2U)
#define XPLMI_CMD_ARG_CNT_THREE		(3U)
#define XPLMI_CMD_ARG_CNT_FOUR		(4U)
#define XPLMI_CMD_ARG_CNT_FIVE		(5U)
#define XPLMI_CMD_ARG_CNT_SIX		(6U)
#define XPLMI_CMD_ARG_CNT_SEVEN		(7U)
#define XPLMI_CMD_ARG_CNT_EIGHT		(8U)
#define XPLMI_CMD_ARG_CNT_NINE		(9U)
#define XPLMI_CMD_ARG_CNT_TEN		(10U)
#define XPLMI_CMD_ARG_CNT_ELEVEN	(11U)
#define XPLMI_CMD_ARG_CNT_TWELVE	(12U)
#define XPLMI_UNLIMITED_ARG_CNT		(0xFFU)

#define XPLMI_MAJOR_MODULE_VERSION	(1U)
#define XPLMI_MINOR_MODULE_VERSION	(1U)

/* Macros for command ids */
#define XPLMI_FEATURES_CMD_ID		(0U)
#define XPLMI_MASK_POLL_CMD_ID		(1U)
#define XPLMI_MASK_WRITE_CMD_ID		(2U)
#define XPLMI_WRITE_CMD_ID		(3U)
#define XPLMI_DELAY_CMD_ID		(4U)
#define XPLMI_DMA_WRITE_CMD_ID		(5U)
#define XPLMI_MASK_POLL64_CMD_ID	(6U)
#define XPLMI_MASK_WRITE64_CMD_ID	(7U)
#define XPLMI_WRITE64_CMD_ID		(8U)
#define XPLMI_DMA_XFER_CMD_ID		(9U)
#define XPLMI_INIT_SEQ_CMD_ID		(10U)
#define XPLMI_CFI_READ_CMD_ID		(11U)
#define XPLMI_SET_CMD_ID		(12U)
#define XPLMI_WRITE_KEYHOLE_CMD_ID	(13U)
#define XPLMI_NOP_CMD_ID		(17U)
#define XPLMI_GET_DEVICE_CMD_ID		(18U)
#define XPLMI_EVENT_LOGGING_CMD_ID	(19U)
#define XPLMI_SET_BOARD_CMD_ID		(20U)
#define XPLMI_GET_BOARD_CMD_ID		(21U)
#define XPLMI_SET_WDT_PARAM_CMD_ID	(22U)
#define XPLMI_LOG_STR_CMD_ID		(23U)
#define XPLMI_LOG_ADDR_CMD_ID		(24U)
#define XPLMI_MARKER_CMD_ID		(25U)
#define XPLMI_PROC_CMD_ID		(26U)
#define XPLMI_OT_CHECK_CMD_ID		(30U)
#define XPLMI_SCATTER_WRITE_CMD_ID	(33U)
#define XPLMI_SCATTER_WRITE2_CMD_ID	(34U)
#define XPLMI_END_CMD_ID		(0xFFU)

/************************** Function Prototypes ******************************/
void XPlmi_ModuleRegister(XPlmi_Module *Module);

/************************** Variable Definitions *****************************/
extern XPlmi_Module *Modules[XPLMI_MAX_MODULES];

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_MODULES_H */
