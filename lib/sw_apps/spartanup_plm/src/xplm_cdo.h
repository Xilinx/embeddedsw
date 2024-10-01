/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_cdo.h
 *
 * This is the file which contains cdo related code.
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

#ifndef XPLM_CDO_H
#define XPLM_CDO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_debug.h"
#include "xplm_status.h"

/************************** Constant Definitions *****************************/

#define XPLM_CDO_HDR_IDN_WRD		(0x004F4443U)	/**< CDO header identification word. */

#define XPLM_CDO_HDR_LEN		(0x5U)	/**< CDO header length in words. */

#define XPLM_CMD_END			(0x01FFU)	/**< END CDO command value. */

#define XPLM_CMD_STATE_START		(0U)	/**< Command Start state. */
#define XPLM_CMD_STATE_RESUME		(1U)	/**< Command Resume state. */

#define XPLM_MAX_SHORT_CMD_LEN		(255U)
/**< Maximum supported payload length of the short CDO command. */

#define XPLM_LONG_CMD_HDR_LEN		(2U)	/**< Long command header length. */

#define XPLM_MAX_LONG_CMD_LEN		(0xFFFFFFFDU)
/**< Maximum supported payload length of the long CDO command. */

#define XPLM_SHORT_CMD_LEN_SHIFT	(16U)	/**< Shift value for the length field in CDO cmd. */

/* Defines for PDI types */
#define XPLM_PDI_TYPE_FULL		(u32)(0x00FFU)	/**< Identifier for FULL PDI. */
#define XPLM_PDI_TYPE_PARTIAL		(u32)(0xFF00U)	/**< Identifier for PARTIAL PDI. */

#define XPLM_BEGIN_OFFSET_STACK_SIZE	(10U)	/**< Maximum supported begin-end pair offset stack size. */
#define XPLM_CMD_RESUME_DATALEN		(8U)	/**< Resume command data length. */

#define XPLM_MAX_MODULES		(2U)	/**< Maximum supported CDO modules. */
#define XPLM_MODULE_GENERIC_ID		(1U)	/**< Module ID for the generic CDO commands. */

/** @cond spartanup_plm_internal */
#define XPLM_MODULE_COMMAND(FUNC)	{ (FUNC) }
/** @endcond */

/**************************** Type Definitions *******************************/

/**
 * Structure to store the offsets of begin-end pair CDO offsets.
 */
typedef struct {
	u32 OffsetList[XPLM_BEGIN_OFFSET_STACK_SIZE];	/**< Array to store end offsets. */
	s32 OffsetListTop;	/**< Index of the top element in stack. */
} XPlm_CdoParamsStack;

typedef struct XPlm_Cmd XPlm_Cmd;

/**
 * Structure to store the information related to CDO commands.
 */
struct XPlm_Cmd {
	u32 CmdId; /**< Command ID */
	u32 Len; /**< Command length */
	u32 ProcessedLen; /**< Processed length */
	u32 PayloadLen; /**< Payload length */
	u32 *Payload; /**< Start address of payload */
	u32 (*ResumeHandler)(XPlm_Cmd *CmdPtr); /**< CDO command handler function address to resume
						  * CDO command execution */
	u32 ResumeData[XPLM_CMD_RESUME_DATALEN]; /**< Buffer to store required information to resume */
	XPlm_CdoParamsStack CdoParamsStack; /**< Instance of @ref XPlm_CdoParamsStack */
	u32 BreakLength; /**< Offset address of 'END' CDO command */
	u32 ProcessedCdoLen; /**< Length of the processed CDO command */
	u8 DeferredError; /**< Defer error indicator */
};

/**
 * Structure to hold the CDO command handler functions.
 */
typedef struct {
	u32 (*Handler)(XPlm_Cmd *Cmd); /**< CDO cmd handler function */
} XPlm_ModuleCmd;

/**
 * Structure for the Modules.
 */
typedef struct {
	u32 Id; /**< Module ID */
	const XPlm_ModuleCmd *CmdAry; /**< Pointer to the array of CDO handler functions */
	u32 CmdCnt; /**< Supported CDO commands count */
} XPlm_Module;

/**
 * Structure to hold the CDO related information.
 */
typedef struct {
	u32 *BufPtr;		/**< CDO Buffer */
	u32 NextChunkAddr;	/**< Address of the next chunk */
	u32 BufLen;		/**< Buffer length */
	u32 CdoLen;		/**< CDO length */
	u32 ProcessedCdoLen;	/**< Processed CDO length */
	u32 PartitionOffset;	/**< Current partition offset */
	u32 CopiedCmdLen;	/**< Copied command length */
	u32 *TempCmdBuf;	/**< Temporary buffer pointer */
	XPlm_Cmd Cmd;		/**< Cmd instance */
	u8 LogCdoOffset;	/**< Flag to determine whether to log CDO offset */
	u8 CmdState;		/**< Cmd processing state */
	u8 CmdEndDetected;	/**< Flag to detect end of commands */
	u8 Cdo1stChunk;		/**< Indicates if it's first chunk or not to validate CDO header */
	u8 DeferredError;	/**< Defer the error for any command till the
				  end of CDO processing */
	u32 PdiType;		/**< To indicate if the PDI is FULL or PARTIAL */
} XPlmCdo;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XPlm_InitCdo(XPlmCdo *CdoPtr);
u32 XPlm_ProcessCdo(XPlmCdo *CdoPtr);
void XPlm_ModuleRegister(XPlm_Module *Module);

/************************** Variable Definitions *****************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of XPLM_CDO_H */

/** @} end of spartanup_plm_apis group*/
