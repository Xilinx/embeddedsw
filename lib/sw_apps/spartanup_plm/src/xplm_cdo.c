/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_cdo.c
 *
 * This file contains code to handling the CDO Buffer and commands present in it.
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

/***************************** Include Files *********************************/
#include "xplm_cdo.h"
#include "xplm_util.h"
#include "xil_util.h"
#include "xplm_debug.h"
#include "xplm_error.h"
#include "xplm_hw.h"

/************************** Constant Definitions *****************************/
/* This buffer is used to store commands which extend across 32K boundaries */

/** @cond spartanup_plm_internal */
#define XPLM_CMD_LEN_TEMPBUF		(0x8U)
#define XPLM_CMD_ID_MASK		(0x1FFU)
#define XPLM_ERR_CMD_ID_SHIFT		(0x4U)
#define XPLM_MIN_ERR_MASK		(0xFU)
#define XPLM_CMD_API_ID_MASK		(0xFFU)
#define XPLM_CMD_MODULE_ID_MASK		(0xFF00U)
#define XPLM_CMD_LEN_MASK		(0xFF0000U)
#define XPLM_CMD_MODULE_ID_SHIFT	(8U)
/** @endcond */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
static XPlm_Module *Modules[XPLM_MAX_MODULES];

/*****************************************************************************/

/*****************************************************************************/
/**
 * @brief	Register the Module.
 *
 * @param	Module is pointer to XPlm Module
 *
 *****************************************************************************/
void XPlm_ModuleRegister(XPlm_Module *Module)
{
	u32 ModuleId = Module->Id;
	Xil_AssertVoid(ModuleId < XPLM_MAX_MODULES);

	Xil_AssertVoid(Modules[ModuleId] == NULL);
	Modules[ModuleId] = Module;
}

/*****************************************************************************/
/**
 * @brief	This function will call the command handler registered with the
 * 		command. Command handler shall execute the command till the
 * 		payload length.
 *
 * @param	CmdPtr is pointer to command structure
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_MODULE_NOT_REGISTERED if the module is not registered.
 * 		- XPLM_ERR_CMD_APIID on invalid module and unregistered CMD ID.
 * 		- XPLM_ERR_CMD_HANDLER_NULL if command handler is not registered.
 * 		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_CmdExecute(XPlm_Cmd *CmdPtr)
{
	u32 Status = (u32)XST_FAILURE;
	u32 ModuleId = (CmdPtr->CmdId & XPLM_CMD_MODULE_ID_MASK) >> XPLM_CMD_MODULE_ID_SHIFT;
	u32 ApiId = CmdPtr->CmdId & XPLM_CMD_API_ID_MASK;
	const XPlm_Module *Module = NULL;
	const XPlm_ModuleCmd *ModuleCmd = NULL;

	/** - Validate if the Module is registered. */
	if (ModuleId < XPLM_MAX_MODULES) {
		Module = Modules[ModuleId];
	}
	if (Module == NULL) {
		Status = (u32)XPLM_ERR_MODULE_NOT_REGISTERED;
		goto END;
	}

	/** - Validate if the API is registered. */
	if (ApiId >= Module->CmdCnt) {
		Status = (u32)XPLM_ERR_CMD_APIID;
		goto END;
	}

	/** - Validate if the handler for the API ID is registered. */
	ModuleCmd = &Module->CmdAry[ApiId];
	if (ModuleCmd->Handler == NULL) {
		Status = (u32)XPLM_ERR_CMD_HANDLER_NULL;
		goto END;
	}
	XPlm_Printf(DEBUG_DETAILED, "CMD 0x%0x, Len 0x%0x, PayloadLen 0x%0x \n\r",
		    CmdPtr->CmdId, CmdPtr->Len, CmdPtr->PayloadLen);

	/** - Execute the API. */
	Status = ModuleCmd->Handler(CmdPtr);
	if (Status != (u32)XST_SUCCESS) {
		Status = ((CmdPtr->CmdId & XPLM_CMD_ID_MASK) << XPLM_ERR_CMD_ID_SHIFT) |
			 (Status & XPLM_MIN_ERR_MASK);
		if (CmdPtr->DeferredError != (u8)TRUE) {
			goto END;
		} else {
			/* If Deferred Error, log the error and continue */
			XPlm_Printf(DEBUG_GENERAL, "Deferring CDO Error\n\r");
			XPlm_LogPlmErr(Status);
			Status = XST_SUCCESS;
		}
	}

	/**
	 * - Store the handler and processed payload length for resuming the command if it
	 * spans across multiple chunks.
	 */
	CmdPtr->ProcessedLen += CmdPtr->PayloadLen;
	CmdPtr->ResumeHandler = ModuleCmd->Handler;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function resumes the command after being partially executed.
 * 		Resume handler shall execute the command till the payload length.
 *
 * @param	CmdPtr is pointer to command structure
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_INVLD_RESUME_HANDLER if handler is not valid.
 * 		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_CmdResume(XPlm_Cmd *CmdPtr)
{
	u32 Status = (u32)XST_FAILURE;

	XPlm_Printf(DEBUG_DETAILED, "CMD Resume \n\r");

	/** - Validate the handler. */
	if (CmdPtr->ResumeHandler ==  NULL) {
		Status = (u32)XPLM_ERR_INVLD_RESUME_HANDLER;
		goto END;
	}

	/** - Execute the handler. */
	Status = CmdPtr->ResumeHandler(CmdPtr);
	if (Status != (u32)XST_SUCCESS) {
		Status = ((CmdPtr->CmdId & XPLM_CMD_ID_MASK) << XPLM_ERR_CMD_ID_SHIFT) |
			 (Status & XPLM_MIN_ERR_MASK);
		XPlm_Printf(DEBUG_INFO, "CDO resume failed\r\n");
		goto END;
	}

	/** - Update the processed length if the current CDO command spans across multiple chunks. */
	CmdPtr->ProcessedLen += CmdPtr->PayloadLen;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Calculate the size of the command. Bits 16 to 23 in command header denote the size
 * of the command. If the value is 255, then the word following CmdId would denote the size of the
 * command.
 *
 * @param	Buf is pointer to buffer with CDO command.
 * @param	Len is length of the command that is available in memory to parse.
 *
 * @return
 * 		- Size of the command
 *
 *****************************************************************************/
static u32 XPlm_CmdSize(const u32 *Buf, u32 Len)
{
	u32 Size = 1U;
	if (Len >= Size) {
		u32 CmdId = Buf[0U];
		u32 PayloadLen = (CmdId & XPLM_CMD_LEN_MASK) >> 16U;
		if (PayloadLen == XPLM_MAX_SHORT_CMD_LEN) {
			Size = XPLM_LONG_CMD_HDR_LEN;
			if (Len >= Size) {
				PayloadLen = Buf[1U];
			}
			if (PayloadLen > XPLM_MAX_LONG_CMD_LEN) {
				PayloadLen = XPLM_MAX_LONG_CMD_LEN;
			}
		}
		Size += PayloadLen;
	}
	return Size;
}

/*****************************************************************************/
/**
 * @brief	This function will setup the command structure.
 *
 * @param	Cmd is pointer to the command structure
 * @param	Buf is pointer to the command buffer
 * @param	BufLen is length of the buffer. It may not have total Command
 * length if buffer is not available.
 *
 *****************************************************************************/
static void XPlm_SetupCmd(XPlm_Cmd *Cmd, u32 *Buf, u32 BufLen)
{
	u32 HdrLen = 1U;

	Cmd->CmdId = Buf[0U];
	Cmd->Len = (Cmd->CmdId >> XPLM_SHORT_CMD_LEN_SHIFT) & XPLM_MAX_SHORT_CMD_LEN;
	Cmd->Payload = &Buf[1U];
	Cmd->ProcessedLen = 0U;
	if (Cmd->Len == XPLM_MAX_SHORT_CMD_LEN) {
		HdrLen = XPLM_LONG_CMD_HDR_LEN;
		Cmd->Len = Buf[1U];
		Cmd->Payload = &Buf[XPLM_LONG_CMD_HDR_LEN];
	}

	/* Assign the available payloadlen in the buffer */
	if (Cmd->Len > (BufLen - HdrLen)) {
		Cmd->PayloadLen = BufLen - HdrLen;
	} else {
		Cmd->PayloadLen = Cmd->Len;
	}
}

/*****************************************************************************/
/**
 * @brief	Validate the CDO header by calculating the sum of all words in the header.
 *
 * @param	CdoPtr is pointer to the CDO structure
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_CDO_HDR_ID on invalid CDO header.
 * 		- XPLM_ERR_CDO_CHECKSUM if CDO checksum fails.
 *
 *****************************************************************************/
static u32 XPlm_CdoVerifyHeader(const XPlmCdo *CdoPtr)
{
	u32 Status = (u32)XST_FAILURE;
	u32 CheckSum = 0U;
	const u32 *CdoHdr = CdoPtr->BufPtr;
	u32 Index;

	/** - Validate the header identification word with @ref XPLM_CDO_HDR_IDN_WRD. */
	if (CdoHdr[1U] != XPLM_CDO_HDR_IDN_WRD) {
		XPlm_Printf(DEBUG_INFO, "CDO Header Identification Failed\n\r");
		Status = (u32)XPLM_ERR_CDO_HDR_ID;
		goto END;
	}

	/** - Calculate the sum of all words in header. */
	for (Index = 0U; Index < (XPLM_CDO_HDR_LEN - 1U); Index++) {
		CheckSum += CdoHdr[Index];
	}

	/** - Invert the calculated checksum and verify with the checksum provided in header. */
	CheckSum ^= XPLM_ALLFS;
	if (CheckSum != CdoHdr[Index]) {
		XPlm_Printf(DEBUG_INFO, "Config Object Checksum Failed\n\r");
		Status = (u32)XPLM_ERR_CDO_CHECKSUM;
		goto END;
	} else {
		Status = XST_SUCCESS;
	}

	XPlm_Printf(DEBUG_INFO, "Config Object Version 0x%08x\n\r", CdoHdr[2U]);
	XPlm_Printf(DEBUG_INFO, "Length 0x%08x\n\r", CdoHdr[3U]);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function initializes the CDO pointer structure.
 *
 * @param	CdoPtr is pointer to the CDO structure
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XST_FAILURE if failed to initialize the CDO structure to '0'.
 *
 *****************************************************************************/
u32 XPlm_InitCdo(XPlmCdo *CdoPtr)
{
	u32 Status = (u32)XST_FAILURE;

	/** - Initialize the entire CDO structure members to '0'. */
	Status = Xil_SMemSet(CdoPtr, sizeof(XPlmCdo), 0U, sizeof(XPlmCdo));
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	/** - Update the structure to indicate its first chunk. */
	CdoPtr->Cdo1stChunk = (u8)TRUE;

	/** - Enable CDO offset logging. */
	CdoPtr->LogCdoOffset = (u8)TRUE;

	/** - Initialize the begin-end stack to '-1' to indicate it's empty. */
	CdoPtr->Cmd.CdoParamsStack.OffsetListTop = -1;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will update the command pointer and resume the
 * command from previous state.
 *
 * @param	CdoPtr Pointer to the CDO structure
 * @param	BufPtr Pointer to the buffer
 * @param	BufLen Len of the buffer
 * @param	Size Pointer to the Size consumed by the command execution
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_CdoCmdResume(XPlmCdo *CdoPtr, u32 *BufPtr, u32 BufLen, u32 *Size)
{
	u32 Status = (u32)XST_FAILURE;
	XPlm_Cmd *CmdPtr = &CdoPtr->Cmd;
	u32 PrintLen;

	/* Update the Payload buffer and length */
	if (CmdPtr->Len > (CmdPtr->ProcessedLen + BufLen)) {
		CmdPtr->PayloadLen = BufLen;
	} else {
		CmdPtr->PayloadLen = CmdPtr->Len - CmdPtr->ProcessedLen;
		CdoPtr->CmdState = XPLM_CMD_STATE_START;
	}

	CmdPtr->Payload = BufPtr;
	CmdPtr->ProcessedCdoLen = CdoPtr->ProcessedCdoLen;
	*Size = CmdPtr->PayloadLen;
	Status = XPlm_CmdResume(CmdPtr);
	if (Status != (u32)XST_SUCCESS) {
		XPlm_Printf(DEBUG_GENERAL,
			    "CMD: 0x%08x Resume failed, Processed Cdo Length 0x%0x\n\r",
			    CmdPtr->CmdId, CdoPtr->ProcessedCdoLen * XPLM_WORD_LEN);
		PrintLen = CmdPtr->PayloadLen;
		if (PrintLen > XPLM_CMD_LEN_TEMPBUF) {
			PrintLen = XPLM_CMD_LEN_TEMPBUF;
		}
		XPlm_PrintArray(DEBUG_GENERAL, (u32)(UINTPTR)CmdPtr->Payload, PrintLen,
				"CMD payload");
	}

	CdoPtr->ProcessedCdoLen += *Size;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function executes the CDO commands.
 *
 * @param	CdoPtr is pointer to the CDO structure
 * @param	BufPtr is pointer to the buffer
 * @param	BufLen is length of the buffer
 * @param	Size is pointer to the Size consumed by the command execution
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_MEMCPY_CMD_EXEC if failed to save the current CDO command for resuming.
 * 		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_CdoCmdExecute(XPlmCdo *CdoPtr, u32 *BufPtr, u32 BufLen, u32 *Size)
{
	u32 Status = (u32)XST_FAILURE;
	XPlm_Cmd *CmdPtr = &CdoPtr->Cmd;
	u32 PrintLen;
	u32 BufSize;

	/**
	 * - Skip executing the remaining commands in CDO upon detecting the 'END' CDO command.
	 */
	if (BufPtr[0U] == XPLM_CMD_END) {
		XPlm_Printf(DEBUG_INFO, "CMD END detected \n\r");
		CdoPtr->CmdEndDetected = (u8)TRUE;
		Status = XST_SUCCESS;
		goto END;
	}

	*Size = XPlm_CmdSize(BufPtr, BufLen);
	CmdPtr->Len = *Size;
	/**
	 * - Save the current CDO command header information to the top of next chunk, if it spans
	 * across multiple chunks.
	 */
	if ((*Size > BufLen) && (BufLen < XPLM_CMD_LEN_TEMPBUF)) {
		BufSize = BufLen * XPLM_WORD_LEN;
		CdoPtr->TempCmdBuf = (u32 *)(CdoPtr->NextChunkAddr - BufSize);

		/* Copy Cmd to temporary buffer */
		Status = Xil_SMemCpy(CdoPtr->TempCmdBuf, BufSize, BufPtr, BufSize, BufSize);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_MEMCPY_CMD_EXEC;
			goto END;
		}
		CdoPtr->CopiedCmdLen = BufLen;
		*Size = BufLen;
		Status = XST_SUCCESS;
		goto END;
	}

	/**
	 * - Update the current command state to indicate that it should resume the broken command
	 * on the next chunk if the command length is greater than the buffer length present in the
	 * current chunk.
	 */
	if (*Size > BufLen) {
		*Size = BufLen;
		CdoPtr->CmdState = XPLM_CMD_STATE_RESUME;
	}
	CmdPtr->BreakLength = 0U;

	XPlm_SetupCmd(CmdPtr, BufPtr, *Size);
	CmdPtr->DeferredError = (u8)FALSE;
	CmdPtr->ProcessedCdoLen = CdoPtr->ProcessedCdoLen;

	/** - Store the current CDO command offset to PMC_FW_DATA if CDO offset logging is enabled. */
	if (CdoPtr->LogCdoOffset == TRUE) {
		Xil_Out32(PMC_GLOBAL_PMC_FW_DATA, CdoPtr->PartitionOffset +
			  CdoPtr->ProcessedCdoLen + XPLM_CDO_HDR_LEN);
	}

	/** - Execute the command. */
	Status = XPlm_CmdExecute(CmdPtr);
	if (Status != (u32)XST_SUCCESS) {
		XPlm_Printf(DEBUG_PRINT_ALWAYS,
			    "CMD: 0x%08x execute failed, Processed Cdo Length 0x%0x\n\r",
			    CmdPtr->CmdId, (CdoPtr->ProcessedCdoLen + XPLM_CDO_HDR_LEN) * XPLM_WORD_LEN);
		PrintLen = CmdPtr->PayloadLen;
		if (PrintLen > XPLM_CMD_LEN_TEMPBUF) {
			PrintLen = XPLM_CMD_LEN_TEMPBUF;
		}
		XPlm_PrintArray(DEBUG_GENERAL, (u32)(UINTPTR)CmdPtr->Payload, PrintLen,
				"CMD Payload");
		goto END;
	}

	CdoPtr->ProcessedCdoLen += *Size;
	if (CmdPtr->Len == (CmdPtr->PayloadLen - 1U)) {
		CmdPtr->PayloadLen = CmdPtr->Len;
		CdoPtr->CmdState = XPLM_CMD_STATE_START;
		CdoPtr->CopiedCmdLen = 0U;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function processes the CDO file.
 *
 * @param	CdoPtr is pointer to the CDO structure
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_INVALID_BREAK_LENGTH on invalid break length provided in CDO.
 * 		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
u32 XPlm_ProcessCdo(XPlmCdo *CdoPtr)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Size = 0U;
	u32 *BufPtr = CdoPtr->BufPtr;
	u32 BufLen = CdoPtr->BufLen;
	u32 RemainingLen;

	/** - Verify the header for the first chunk of CDO */
	if (CdoPtr->Cdo1stChunk == (u8)TRUE) {
		Status = XPlm_CdoVerifyHeader(CdoPtr);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		CdoPtr->Cdo1stChunk = (u8)FALSE;
		CdoPtr->CdoLen = BufPtr[3U];

		BufPtr = &BufPtr[XPLM_CDO_HDR_LEN];
		BufLen -= XPLM_CDO_HDR_LEN;
		CdoPtr->BufLen -= XPLM_CDO_HDR_LEN;
	}

	/**
	 * - Check if BufLen is greater than CdoLen
	 * This is required if more buffer is copied than CDO len.
	 * Mainly for PLM CDO where BufLen is not present and is
	 * given as Max PRAM len.
	 */
	if ((BufLen + CdoPtr->ProcessedCdoLen + CdoPtr->CopiedCmdLen) > (CdoPtr->CdoLen)) {
		BufLen = CdoPtr->CdoLen - CdoPtr->ProcessedCdoLen - CdoPtr->CopiedCmdLen;
		CdoPtr->BufLen = BufLen;
	}

	/** - Stop processing if command END is detected in previous iteration. */
	if (CdoPtr->CmdEndDetected == (u8)TRUE) {
		Status = XST_SUCCESS;
		goto END;
	}

	XPlm_Printf(DEBUG_INFO, "Processing CDO, Chunk Len 0x%08x\n\r", BufLen);
	/* Check if cmd data is copied partially during the last iteration */
	if (CdoPtr->CopiedCmdLen > 0U) {
		BufPtr = CdoPtr->TempCmdBuf;
		BufLen += CdoPtr->CopiedCmdLen;
		CdoPtr->CopiedCmdLen = 0x0U;
	}

	/**
	 * - Handle the break command occured in previous chunk.
	 * 	- If the end is not present in current chunk, stop processing the current chunk.
	 * 	- If the end is present in current chunk, jump to end command.
	 */
	if (CdoPtr->Cmd.BreakLength > 0U) {
		RemainingLen = CdoPtr->Cmd.BreakLength - CdoPtr->ProcessedCdoLen;
		if (RemainingLen >= BufLen) {
			/* If the end is not present in current chunk, skip this chunk */
			CdoPtr->ProcessedCdoLen += BufLen;
			Status = XST_SUCCESS;
			goto END;
		} else {
			/* If the end is present in current chunk, jump to end command */
			CdoPtr->ProcessedCdoLen += RemainingLen;
			BufLen -= RemainingLen;
			BufPtr = &BufPtr[RemainingLen];
			CdoPtr->Cmd.BreakLength = 0U;
		}
	}

	/** - Start executing the commands from the Cdo Buffer. */
	while (BufLen > 0U) {
		/* Check if cmd has to be resumed */
		if (CdoPtr->CmdState == XPLM_CMD_STATE_RESUME) {
			Status = XPlm_CdoCmdResume(CdoPtr, BufPtr, BufLen, &Size);
		} else {
			Status = XPlm_CdoCmdExecute(CdoPtr, BufPtr, BufLen, &Size);
		}
		CdoPtr->DeferredError |= CdoPtr->Cmd.DeferredError;
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		/* If command end is detected, exit the loop */
		if (CdoPtr->CmdEndDetected == (u8)TRUE) {
			goto END;
		}

		/* Handle the break command processed in current chunk */
		if (CdoPtr->Cmd.BreakLength > 0U) {
			if (CdoPtr->Cmd.BreakLength < CdoPtr->ProcessedCdoLen) {
				Status = (u32)XPLM_ERR_INVALID_BREAK_LENGTH;
				goto END;
			}
			if (BufLen > (Size + CdoPtr->Cmd.BreakLength - CdoPtr->ProcessedCdoLen)) {
				/* If the end is present in current chunk, jump to it */
				Size += CdoPtr->Cmd.BreakLength - CdoPtr->ProcessedCdoLen;
				CdoPtr->ProcessedCdoLen += CdoPtr->Cmd.BreakLength - CdoPtr->ProcessedCdoLen;
				CdoPtr->Cmd.BreakLength = 0U;
			} else {
				/*
				 * If the end is not present in current chunk, skip processing
				 * rest of the chunk
				 */
				CdoPtr->ProcessedCdoLen += BufLen - Size;
				break;
			}
		}

		/* Update the parameters for next iteration */
		BufPtr = &BufPtr[Size];
		BufLen -= Size;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/** @} end of spartanup_plm_apis group*/
