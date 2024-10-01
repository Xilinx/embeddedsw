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
#ifndef XPLM_CDO_H
#define XPLM_CDO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xplm_debug.h"
#include "xplm_status.h"

/************************** Constant Definitions *****************************/
/** CDO Header definitions */
#define XPLM_CDO_HDR_IDN_WRD		(0x004F4443U)
#define XPLM_CDO_HDR_LEN		(0x5U)

/* Commands defined */
#define XPLM_CMD_END			(0x01FFU)

#define XPLM_CMD_STATE_START		(0U)
#define XPLM_CMD_STATE_RESUME		(1U)

/* Define for Max short command length */
#define XPLM_MAX_SHORT_CMD_LEN		(255U)

/* Define for Long command header length */
#define XPLM_LONG_CMD_HDR_LEN		(2U)

/* Define for Max Long command length */
#define XPLM_MAX_LONG_CMD_LEN		(0xFFFFFFFDU)

/* Define for Short command length shift */
#define XPLM_SHORT_CMD_LEN_SHIFT	(16U)

/* Defines for PDI types */
#define XPLM_PDI_TYPE_FULL		(u32)(0x00FFU)
#define XPLM_PDI_TYPE_PARTIAL		(u32)(0xFF00U)

#define XPLM_BEGIN_OFFSET_STACK_SIZE	(10U)
#define XPLM_CMD_RESUME_DATALEN		(8U)

#define XPLM_MAX_MODULES		(2U)
#define XPLM_MODULE_GENERIC_ID		(1U)
#define XPLM_MODULE_COMMAND(FUNC)	{ (FUNC) }

/**************************** Type Definitions *******************************/

/**
 * Structure to store the offsets of begin-end pair CDO offsets.
 */
typedef struct {
	u32 OffsetList[XPLM_BEGIN_OFFSET_STACK_SIZE];	/**< Array for CDO offsets. */
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
	u32 (*ResumeHandler)(XPlm_Cmd *CmdPtr); /**< CDO command handler function address to resume */
	u32 ResumeData[XPLM_CMD_RESUME_DATALEN]; /**< Buffer to store required information to resume */
	XPlm_CdoParamsStack CdoParamsStack; /**< Instance of @ref XPlm_CdoParamsStack */
	u32 BreakLength; /**< Break command level */
	u32 ProcessedCdoLen; /**< Length of the processed CDO command */
	u8 DeferredError; /**< Defer error indicator */
};

/**
 * Structure to hold the CDO command handler functions.
 */
typedef struct {
	u32 (*Handler)(XPlm_Cmd *Cmd); /**< CDO cmd handler function */
} XPlm_ModuleCmd;

typedef struct {
	u32 Id;
	const XPlm_ModuleCmd *CmdAry;
	u32 CmdCnt;
} XPlm_Module;

/**
 * The XPlmCdo is instance data. The user is required to allocate a
 * variable of this type for every Cdo processing. A pointer
 * to a variable of this type is then passed to the PLMI CDO API functions.
 */
typedef struct {
	u32 *BufPtr;		/**< CDO Buffer */
	u32 NextChunkAddr;	/**< Address of the next chunk */
	u32 BufLen;		/**< Buffer length */
	u32 CdoLen;		/**< CDO length */
	u32 ProcessedCdoLen;	/**< Processed CDO length */
	u32 PartitionOffset;	/**< Current parition offset */
	u32 CopiedCmdLen;	/**< Copied Command length */
	u32 *TempCmdBuf;	/**< Temporary buffer to store commands
				 between iterations */
	XPlm_Cmd Cmd;		/**< Cmd instance */
	u8 LogCdoOffset;	/**< Flag to determine whether to log CDO offset */
	u8 CmdState;		/**< Cmd processing state */
	u8 CmdEndDetected;	/**< Flag to detect end of commands */
	u8 Cdo1stChunk;		/**< This is used for first time to validate
				CDO header*/
	u8 DeferredError;	/**< Defer the error for any command till the
				  end of CDO processing */
	u32 PdiType;		/** Type of the PDI */
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
