/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_generic.c
 * This is the file which contains general commands.
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

/***************************** Include Files *********************************/
#include "xplm_generic.h"
#include "xplm_debug.h"
#include "xstatus.h"
#include "xil_util.h"
#include "xplm_util.h"
#include "xplm_cdo.h"
#include "sleep.h"
#include "xplm_dma.h"
#include "xplm_hooks.h"
#include "xplm_load.h"
#include "xplm_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLM_LOG_ADDR_ARG_LOW_ADDR_INDEX	(0U)
#define XPLM_LOG_ADDR_ARG_HIGH_ADDR_INDEX	(1U)
#define XPLM_LOG_ADDR_MAX_ARGS		(2U)
#define XPLM_MAX_LOG_STR_LEN	(256U)

/* Mask poll related macros */
#define XPLM_MASKPOLL_ADDR_INDEX		(0U)
#define XPLM_MASKPOLL_MASK_INDEX		(1U)
#define XPLM_MASKPOLL_EXP_VAL_INDEX		(2U)
#define XPLM_MASKPOLL_TIMEOUT_INDEX		(3U)
#define XPLM_MASKPOLL_FLAGS_INDEX		(4U)
#define XPLM_MASKPOLL_MINOR_ERROR_MASK		(0xFFFFU)

#define XPLM_BEGIN_MAX_LOG_STR_LEN			(24U)
#define XPLM_BEGIN_LOG_STR_BUF_END_INDEX	(XPLM_BEGIN_MAX_LOG_STR_LEN - 1U)
#define XPLM_BEGIN_OFFSET_STACK_SIZE			(10U)
#define XPLM_BEGIN_OFFEST_STACK_DEFAULT_POPLEVEL	(1U)
#define XPLM_BEGIN_CMD_EXTRA_OFFSET			(2U)
#define XPLM_BEGIN_MAX_STRING_WORD_LEN		(XPLM_BEGIN_MAX_LOG_STR_LEN /  XPLM_WORD_LEN)

#define XPLM_RDBK_DIS	(0x1U)

/* DMA write keyhole payload arguments indexes */
#define XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_ADDR_ARG_INDEX			(0x1U)
#define XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_SIZE_ARG_INDEX			(0x2U)
#define XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_DATA_ARG_INDEX			(0x3U)
#define XPLM_WR_KEYHOLE_RESUME_DATA_KEYHOLE_ADDR_ARG_INDEX		(0x0U)
#define XPLM_WR_KEYHOLE_RESUME_DATA_KEYHOLE_SIZE_ARG_INDEX		(0x1U)
#define XPLM_WR_KEYHOLE_CMD_RESUME_PAYLOAD_KEYHOLE_DATA_ARG_INDEX	(0x0U)

/************************** Function Prototypes ******************************/
static int XPlm_KeyHoleXfr(XPlm_KeyHoleXfrParams* KeyHoleXfrParams);
static int XPlm_StackPush(XPlm_CdoParamsStack *CdoParamsStack, u32 *Data);
static int XPlm_StackPop(XPlm_CdoParamsStack *CdoParamsStack, u32 PopLevel, u32 *Data);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	Contains the module ID and PLM generic commands array
 *
 *****************************************************************************/
static XPlm_Module XPlm_Generic;

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief	This function checks if a particular PLM Command ID is supported
 * 			or not. Command ID is the only payload parameter.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlm_Features(XPlm_Cmd *Cmd)
{
	u32 Status = (u32)XST_FAILURE;

	if (Cmd->Payload[0U] < XPlm_Generic.CmdCnt) {
		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides no operation. The command is supported for
 * alignment purposes. Zero command payload parameters.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlm_Nop(XPlm_Cmd *Cmd)
{
	XPlm_Printf(DEBUG_DETAILED, "%s\n\r", __func__);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides 32 bit mask poll command execution.
 *		Command payload parameters are
 *		- Address
 *		- Mask
 *		- Expected Value
 *		- Timeout in us
 *		- Deferred Error flag - Optional
 *			0 - Return error in case of failure,
 *			1 - Ignore error, return success always
 *			2 - Defer error till the end of partition load
 *			3 - Break to end offset in case of failure
 *		- Error Code
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlm_MaskPoll(XPlm_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Addr = Cmd->Payload[XPLM_MASKPOLL_ADDR_INDEX];
	u32 Mask = Cmd->Payload[XPLM_MASKPOLL_MASK_INDEX];
	u32 ExpectedValue = Cmd->Payload[XPLM_MASKPOLL_EXP_VAL_INDEX];
	u32 TimeOutInUs = Cmd->Payload[XPLM_MASKPOLL_TIMEOUT_INDEX];
	u32 Flags = 0U;
	u32 Level = 0U;
	u32 DebugLevel = DEBUG_INFO;
	u32 ExtLen = XPLM_MASKPOLL_LEN_EXT;
	u32 MinErrCode = 0U;

	/* Error out if Cmd length is greater than supported length */
	if (Cmd->Len > (ExtLen + 1U)) {
		goto END;
	}

	/* Mask Poll for expected value */
	Status = XPlm_UtilPoll(Addr, Mask, ExpectedValue, TimeOutInUs, NULL);

	/* Print in case of failure or when DEBUG_INFO is enabled */
	if ((Flags != XPLM_MASKPOLL_FLAGS_BREAK) && (Status != XST_SUCCESS)) {
		DebugLevel = DEBUG_GENERAL;
	}

	XPlm_Printf(DebugLevel, "MaskPoll: Addr: 0x%08x, Mask: 0x%0x, ExpVal: 0x%0x, Timeout: %u",
		Addr, Mask, ExpectedValue, TimeOutInUs);
	if (Status != XST_SUCCESS) {
		XPlm_Printf(DebugLevel, ", RegVal: 0x%0x ...ERROR\r\n", Xil_In32(Addr));
	}
	else {
		XPlm_Printf(DebugLevel, " ...DONE\r\n");
	}

	/*
	 * If command length is greater than Optional arguments length,
	 * then flags and error code are processed
	 */
	if ((Cmd->Len >= ExtLen) && (Status != XST_SUCCESS)) {
		MinErrCode = Cmd->Payload[ExtLen] & XPLM_MASKPOLL_MINOR_ERROR_MASK;
		if (Flags == XPLM_MASKPOLL_FLAGS_SUCCESS) {
			/* Ignore the error */
			Status = XST_SUCCESS;
		} else if (Flags == XPLM_MASKPOLL_FLAGS_DEFERRED_ERR) {
			/* Defer the error till the end of CDO processing */
			Cmd->DeferredError = (u8)TRUE;
		} else if (Flags == XPLM_MASKPOLL_FLAGS_BREAK) {
			Level = (Cmd->Payload[ExtLen - 1U] &
					XPLM_MASKPOLL_FLAGS_BREAK_LEVEL_MASK) >>
					XPLM_MASKPOLL_FLAGS_BREAK_LEVEL_SHIFT;
			/* Jump to "end" associated with break level */
			Status = XPlm_GetJumpOffSet(Cmd, Level);
		}
		if ((Cmd->Len == (ExtLen + 1U)) && (MinErrCode != 0U) && (Status != XST_SUCCESS)) {
			/*
			 * Overwrite the error code with the error code value present
			 * in payload argument.
			 */
			Status = (int)MinErrCode;
		}
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides 32 bit mask write command execution.
 *  		Command payload parameters are
 *		- Address
 *		- Mask
 *		- Value
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlm_MaskWrite(XPlm_Cmd *Cmd)
{
	u32 Addr = Cmd->Payload[0U];
	u32 Mask = Cmd->Payload[1U];
	u32 Value = Cmd->Payload[2U];

	XPlm_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%08x,  Mask 0x%08x, Value: 0x%08x\n\r",
		__func__, Addr, Mask, Value);

	XPlm_UtilRMW(Addr, Mask, Value);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides 32 bit Write command execution.
 *  		Command payload parameters are
 *		- Address
 *		- Value
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlm_Write(XPlm_Cmd *Cmd)
{
	u32 Addr = Cmd->Payload[0U];
	u32 Value = Cmd->Payload[1U];

	XPlm_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x,  Val: 0x%0x\n\r",
		__func__, Addr, Value);

	Xil_Out32(Addr, Value);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides delay command execution.
 *  		Command payload parameter delay in micro seconds
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlm_Delay(XPlm_Cmd *Cmd)
{
	u32 Delay;

	Delay = Cmd->Payload[0U];
	XPlm_Printf(DEBUG_DETAILED,
		"%s, Delay: %d\n\r",
		__func__, Delay);

	usleep(Delay);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides CFI READ command execution.
 *		Command payload parameters are
 *		- Params - SMPA/JTAG/DDR
 *		- High Dest Addr
 *		- Low Dest Addr
 *		- Read Length in number of words to be read from CFU
 *		- DATA (CFU READ Packets)
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XPLM_ERR_RDBK_INVALID_INFR_SEL Error if the invalid interface
 *		type is selected for readback.
 *
 *****************************************************************************/
static int XPlm_CfiRead(XPlm_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 SrcType = Cmd->Payload[0U] & XPLM_READBACK_SRC_MASK;
	u32 ReadLen = XPLM_CCU_RD_STREAM_SIZE_WORDS;
	u32 Len = Cmd->Payload[3U];
	u32 CfiPayloadSrcAddr = (u32)(&Cmd->Payload[XPLM_CFI_DATA_OFFSET]);
	u32 SbiCtrlCfg;
	u32 IsRdbkDis = Xil_In32(SLAVE_BOOT_SBI_RDBK);

	/**
	 * - Verify if Readback is disabled, if disabled return XPLM_ERR_RDBK_DISABLED,
	 * otherwise proceed with reading.
	 */
	if (IsRdbkDis == XPLM_RDBK_DIS) {
		Status = (u32)XPLM_ERR_RDBK_DISABLED;
		goto END1;
	}

	/** Save previous SBI control configuration. */
	SbiCtrlCfg = Xil_In32(SLAVE_BOOT_SBI_CTRL);

	/** Set the SBI control register interface read from the command. */
	if (SrcType == XPLM_READBK_INTF_TYPE_JTAG) {
		XPlm_UtilRMW(SLAVE_BOOT_SBI_CTRL, SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
			XPLM_READBK_INTF_TYPE_JTAG << SLAVE_BOOT_SBI_CTRL_INTERFACE_SHIFT);
	} else if (SrcType == XPLM_READBK_INTF_TYPE_SMAP) {
		XPlm_UtilRMW(SLAVE_BOOT_SBI_CTRL, SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
			XPLM_READBK_INTF_TYPE_SMAP << SLAVE_BOOT_SBI_CTRL_INTERFACE_SHIFT);
	} else {
		/*
		 * Return XPLM_ERR_RDBK_INVALID_INFR_SEL if the selected interface type
		 * is neither JTAG nor SMAP.
		 */
		Status = (u32)XPLM_ERR_RDBK_INVALID_INFR_SEL;
		goto END;
	}

	/** Set the SBI mode register to configuration mode. */
	XPlm_UtilRMW(SLAVE_BOOT_SBI_MODE, SLAVE_BOOT_SBI_MODE_SELECT_MASK, XPLM_READBK_SBI_CFG_MODE);

	Status = XPlm_DmaXfr(CfiPayloadSrcAddr, XPLM_CCU_WR_STREAM_BASEADDR,
			(Cmd->PayloadLen - XPLM_CFI_DATA_OFFSET), XPLM_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		Status = (u32)XPLM_ERR_RDBK_PAYLOAD_TO_CCU;
		goto END;
	}


	while (Len > 0U) {
		if (Len < XPLM_CCU_RD_STREAM_SIZE_WORDS) {
			ReadLen = Len;
		}
		Status = XPlm_DmaSbiXfer(XPLM_CCU_RD_STREAM_BASEADDR, ReadLen, XPLM_SBI_READ_FLAGS_NONE);
		if (Status != XST_SUCCESS) {
			Status = (u32)XPLM_ERR_RDBK_CCU_READ;
			goto END;
		}
		Len -= ReadLen;
	}

	if (SrcType == XPLM_READBK_INTF_TYPE_SMAP) {
		Status = XPlm_UtilPoll(SLAVE_BOOT_SBI_STATUS,
			SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_MASK,
			(SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_DEFVAL << SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_SHIFT),
			XPLM_TIME_OUT_DEFAULT, NULL);
	} else {
		Status = XPlm_UtilPoll(SLAVE_BOOT_SBI_STATUS,
			SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_MASK,
			(SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_DEFVAL << SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_SHIFT),
			XPLM_TIME_OUT_DEFAULT, NULL);
	}

	if (Status != XST_SUCCESS) {
		Status = (u32)XPLM_ERR_RDBK_READ_TIMEOUT;
	}

END:
	/** Restore previous SBI control configuration. */
	Xil_Out32(SLAVE_BOOT_SBI_CTRL, SbiCtrlCfg);
	XPlm_UtilRMW(SLAVE_BOOT_SBI_MODE, SLAVE_BOOT_SBI_MODE_SELECT_MASK, SLAVE_BOOT_SBI_MODE_SELECT_DEFVAL);

END1:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function provides SET command execution.
*  		Command payload parameters are
*		- High Dest Addr
*		- Low Dest Addr
*		- Length (Length of words to set to value)
*		- Value
*
* @param	Cmd is pointer to the command structure
*
* @return
*
*****************************************************************************/
static int XPlm_Set(XPlm_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 DestAddr = Cmd->Payload[1U];
	u32 Len = Cmd->Payload[2U];
	u32 Val = Cmd->Payload[3U];

	Status = XPlm_MemSet(DestAddr, Val, Len);
	if (Status != XST_SUCCESS) {
		Status = (u32)XPLM_ERR_SET_MEM;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function provides DMA key hole write command execution.
*  		Command payload parameters are
*		- High Dest Addr
*		- Low Dest Addr
*		- Keyhole Size in bytes
*		- DATA
*
* @param	Cmd is pointer to the command structure
*
* @return
*
*****************************************************************************/
static int XPlm_DmaWriteKeyHole(XPlm_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 DestAddr;
	u32 SrcAddr;
	u32 Len = Cmd->PayloadLen;
	u32 Keyholesize;
	u32 BaseAddr;
	u32 DestOffset;
	XPlm_KeyHoleXfrParams KeyHoleXfrParams;

	XPlm_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	/** - Store the destination address and keyholesize in resume data */
	if (Cmd->ProcessedLen == 0U) {
		Cmd->ResumeData[XPLM_WR_KEYHOLE_RESUME_DATA_KEYHOLE_ADDR_ARG_INDEX] = Cmd->Payload[XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_ADDR_ARG_INDEX];
		Cmd->ResumeData[XPLM_WR_KEYHOLE_RESUME_DATA_KEYHOLE_SIZE_ARG_INDEX] = Cmd->Payload[XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_SIZE_ARG_INDEX];
		Keyholesize = Cmd->Payload[XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_SIZE_ARG_INDEX] * XPLM_WORD_LEN;
		SrcAddr = (u32)(UINTPTR) &Cmd->Payload[XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_DATA_ARG_INDEX];
		Len -= 3U;
		DestOffset = 0U;
	} else {
		SrcAddr = (u32)(UINTPTR) &Cmd->Payload[XPLM_WR_KEYHOLE_CMD_RESUME_PAYLOAD_KEYHOLE_DATA_ARG_INDEX];
		Keyholesize = Cmd->ResumeData[XPLM_WR_KEYHOLE_RESUME_DATA_KEYHOLE_SIZE_ARG_INDEX] * XPLM_WORD_LEN;
		DestOffset = 3U;
	}

	BaseAddr = Cmd->ResumeData[XPLM_WR_KEYHOLE_RESUME_DATA_KEYHOLE_ADDR_ARG_INDEX];

	/* wrap the CCU keyhole address to the beginning upon exceeding the keyhole size. */
	DestAddr = (((Cmd->ProcessedLen - DestOffset) * XPLM_WORD_LEN) & (Keyholesize - 1U)) + BaseAddr;
	KeyHoleXfrParams.SrcAddr = SrcAddr;
	KeyHoleXfrParams.DestAddr = DestAddr;
	KeyHoleXfrParams.BaseAddr = BaseAddr;
	KeyHoleXfrParams.Len = Len * XPLM_WORD_LEN;
	KeyHoleXfrParams.Keyholesize = Keyholesize;
	KeyHoleXfrParams.Flags = XPLM_PMCDMA_0;
	KeyHoleXfrParams.Func = NULL;
	Status = XPlm_KeyHoleXfr(&KeyHoleXfrParams);
	if (Status != XST_SUCCESS) {
		Status = (u32)XPLM_ERR_KEYHOLE_XFER;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function adds Debug string to PLM logs.
 *
 * @param	Cmd is pointer to the command structure
 *		Command payload parameters are
 *		- Debug String
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_MAX_LOG_STR_LEN if the string provided exceeds max length.
 *
 *****************************************************************************/
static int XPlm_LogString(XPlm_Cmd *Cmd)
{
	u32 Len = Cmd->PayloadLen * XPLM_WORD_LEN;
	u32 DebugFlag = DEBUG_PRINT_ALWAYS | XPLM_DEBUG_PRINT_STAGE_INFO_MASK ;

	/* Print without timestamp if it is not the beginning of the string */
	if (Cmd->ProcessedLen != 0U) {
		DebugFlag = DEBUG_PRINT_ALWAYS;
	}

	/* Print the string*/
	XPlm_Printf_WoS(DebugFlag, "%.*s", Len,(u8 *)&Cmd->Payload[0U]);

	/* Print new line */
	if (Cmd->Len == (Cmd->ProcessedLen + Cmd->PayloadLen)) {
		XPlm_Printf_WoS(DEBUG_PRINT_ALWAYS, "\n\r");
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function reads the value at an address and displays and adds
 * 			the value to PLM logs.
 *
 * @param	Cmd is pointer to the command structure
 *		Command payload parameters are
 *		- Low Address
 *		- High Address (Optional) (not used)
 *
 * @return
 * 		- XST_SUCCESS always
 *
 *****************************************************************************/
static int XPlm_LogAddress(XPlm_Cmd *Cmd)
{
	u32 Addr = Cmd->Payload[XPLM_LOG_ADDR_ARG_LOW_ADDR_INDEX];
	u32 Val;

	Val = Xil_In32(Addr);

	XPlm_Printf(DEBUG_PRINT_ALWAYS, "Value at 0x%08x: %0x\n\r", Addr, Val);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides Marker command execution.
 *
 * @param	Cmd is pointer to the command structure
 *              Command payload parameters are
 *              - Type
 *              - String
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlm_Marker(XPlm_Cmd *Cmd)
{
	XPlm_Printf(DEBUG_DETAILED, "%s\n\r", __func__);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides start of the block.
 *  		Command payload parameters are
 *		- Offset : specifies no. of words until "end" of the block
 *		- String : optional for debugging purpose (max. 6 words)
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlm_Begin(XPlm_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 EndOffSet;
	u32 StrLen;
	u32 StartOffset;
	u32 DebugFlag ;

	/* Push to stack, selection of 'StrLen' and 'StartOffset' based on processed length */
	if (Cmd->ProcessedLen == 0U) {
		/* Max 10 nested begin supported */
		if (Cmd->CdoParamsStack.OffsetListTop == (int)(XPLM_BEGIN_OFFSET_STACK_SIZE - 1U)) {
			Status = (u32)XPLM_ERR_MAX_NESTED_BEGIN;
			XPlm_Printf(DEBUG_DETAILED,"Max %d nested begin supported\n", XPLM_BEGIN_OFFSET_STACK_SIZE);
			goto END;
		}

		/* Calculate 'EndOffSet' based on command length */
		if (Cmd->Len > XPLM_MAX_SHORT_CMD_LEN) {
			EndOffSet = Cmd->ProcessedCdoLen + Cmd->Payload[0U] + XPLM_BEGIN_CMD_EXTRA_OFFSET+ 1U;
		}
		else {
			EndOffSet = Cmd->ProcessedCdoLen + Cmd->Payload[0U] + XPLM_BEGIN_CMD_EXTRA_OFFSET;
		}

		/* Push "end" Offset to stack */
		Status = XPlm_StackPush(&Cmd->CdoParamsStack, &EndOffSet);
		if ( Status != XST_SUCCESS) {
			goto END;
		}

		StrLen = (Cmd->PayloadLen - 1U) * XPLM_WORD_LEN;
		StartOffset = 1U;

		/* Print with timestamp if it is the beginning of the string */
		DebugFlag = DEBUG_PRINT_ALWAYS | XPLM_DEBUG_PRINT_STAGE_INFO_MASK ;
	}
	else {
		StrLen = Cmd->PayloadLen * XPLM_WORD_LEN;
		StartOffset = 0U;

		/* Print without timestamp if it is not the beginning of the string */
		DebugFlag = DEBUG_PRINT_ALWAYS;
	}

	if (Cmd->PayloadLen > 1U) {
		/* Print string */
		XPlm_Printf_WoS(DebugFlag, "%.*s", StrLen, (u8 *)&Cmd->Payload[StartOffset]);

		/* Print new line */
		if (Cmd->Len == (Cmd->ProcessedLen + Cmd->PayloadLen)) {
			XPlm_Printf_WoS(DEBUG_PRINT_ALWAYS, "\n\r");
		}
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides end of the block.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlm_End(XPlm_Cmd *Cmd)
{
	u32 EndLength = 0U;
	int Status = XST_FAILURE;

	/* Stack empty, End does not have begin */
	if (Cmd->CdoParamsStack.OffsetListTop < 0) {
		Status = (u32)XPLM_ERR_INVLD_BEGIN_END_PAIR;
		XPlm_Printf(DEBUG_DETAILED,"End does not have valid begin\n");
		goto END;
	}

	/* Popped "end" length from stack should match with current ProcessedCdoLen */
	Status = XPlm_StackPop(&Cmd->CdoParamsStack, XPLM_BEGIN_OFFEST_STACK_DEFAULT_POPLEVEL,
			&EndLength);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (EndLength != Cmd->ProcessedCdoLen) {
		Status = XST_FAILURE;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function skips all the commands until the Levels
 * number of nested "end".
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlm_Break(XPlm_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Level = 0U;

	/*
	 * Get Level to skip all commands until Levels number of nested “end”
	 * commands have been seen
	 */
	if (0U == Cmd->PayloadLen) {
		Level = XPLM_BEGIN_OFFEST_STACK_DEFAULT_POPLEVEL;
	} else {
		Level = Cmd->Payload[0U] & XPLM_BREAK_LEVEL_MASK;
	}

	/* Jump to "end" associated with break level */
	Status = XPlm_GetJumpOffSet(Cmd, Level);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function registers the PLM generic commands to the PLMI.
 *
 *****************************************************************************/
void XPlm_GenericInit(void)
{
	/* Contains the array of PLM generic commands */
	static const XPlm_ModuleCmd XPlm_GenericCmds[] =
	{
		XPLM_MODULE_COMMAND(XPlm_Features),
		XPLM_MODULE_COMMAND(XPlm_MaskPoll),
		XPLM_MODULE_COMMAND(XPlm_MaskWrite),
		XPLM_MODULE_COMMAND(XPlm_Write),
		XPLM_MODULE_COMMAND(XPlm_Delay),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(XPlm_CfiRead),
		XPLM_MODULE_COMMAND(XPlm_Set),
		XPLM_MODULE_COMMAND(XPlm_DmaWriteKeyHole),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(XPlm_Nop),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(XPlm_LogString),
		XPLM_MODULE_COMMAND(XPlm_LogAddress),
		XPLM_MODULE_COMMAND(XPlm_Marker),
		XPLM_MODULE_COMMAND(NULL),
		XPLM_MODULE_COMMAND(XPlm_Begin),
		XPLM_MODULE_COMMAND(XPlm_End),
		XPLM_MODULE_COMMAND(XPlm_Break),
	};

	XPlm_Generic.Id = XPLM_MODULE_GENERIC_ID;
	XPlm_Generic.CmdAry = XPlm_GenericCmds;
	XPlm_Generic.CmdCnt = XPLM_ARRAY_SIZE(XPlm_GenericCmds);

	XPlm_ModuleRegister(&XPlm_Generic);
}

/*****************************************************************************/
/**
* @brief	This function provides DMA transfer to CCU in chunks of Keyholesize.
*
* @param	KeyHoleXfrParams is a pointer to instance of CfiParams structure
*
* @return
* 		- XST_SUCCESS on success and error code on failure
*
*****************************************************************************/
static int XPlm_KeyHoleXfr(XPlm_KeyHoleXfrParams* KeyHoleXfrParams)
{
	int Status = XST_FAILURE;
	u32 LenTemp = (KeyHoleXfrParams->Keyholesize + KeyHoleXfrParams->BaseAddr) - KeyHoleXfrParams->DestAddr;

	if (LenTemp > KeyHoleXfrParams->Len) {
		LenTemp = KeyHoleXfrParams->Len;
	}

	if (LenTemp == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	Status = XPlm_DmaXfr(KeyHoleXfrParams->SrcAddr, KeyHoleXfrParams->DestAddr, LenTemp / XPLM_WORD_LEN, KeyHoleXfrParams->Flags);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	KeyHoleXfrParams->SrcAddr += LenTemp;
	KeyHoleXfrParams->Len -= LenTemp;
	if (KeyHoleXfrParams->Len == 0U) {
		KeyHoleXfrParams->DestAddr += LenTemp;
		goto END;
	}

	KeyHoleXfrParams->DestAddr = KeyHoleXfrParams->BaseAddr;

	while (KeyHoleXfrParams->Len > 0U) {
		LenTemp = KeyHoleXfrParams->Keyholesize;
		if (LenTemp > KeyHoleXfrParams->Len) {
			LenTemp = KeyHoleXfrParams->Len;
		}

		Status = XPlm_DmaXfr(KeyHoleXfrParams->SrcAddr, KeyHoleXfrParams->BaseAddr, LenTemp / XPLM_WORD_LEN, KeyHoleXfrParams->Flags);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		KeyHoleXfrParams->Len -= LenTemp;
		KeyHoleXfrParams->SrcAddr +=LenTemp;
	}
	if (LenTemp < KeyHoleXfrParams->Keyholesize) {
		KeyHoleXfrParams->DestAddr += LenTemp;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function pushes data on stack.
 *
 * @param	Data is pointer to the data to be stored on stack
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlm_StackPush(XPlm_CdoParamsStack *CdoParamsStack, u32 *Data)
{
	int Status = XST_FAILURE;

	/* Validate stack top */
	if (CdoParamsStack->OffsetListTop < -1) {
		XPlm_Printf(DEBUG_DETAILED, "Invalid top in End address stack\n");
		goto END;
	}

	if (CdoParamsStack->OffsetListTop >= (int)(XPLM_BEGIN_OFFSET_STACK_SIZE - 1U)) {
		XPlm_Printf(DEBUG_DETAILED, "End address stack is full\n");
	} else {
		CdoParamsStack->OffsetListTop++;
		CdoParamsStack->OffsetList[CdoParamsStack->OffsetListTop] = *Data;
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function pops data from stack.
 *
 * @param	PopLevel is the number of elements to remove from stack.
 * @param	Data is pointer to store removed data from stack.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlm_StackPop(XPlm_CdoParamsStack *CdoParamsStack, u32 PopLevel, u32 *Data)
{
	int Status = XST_FAILURE;
	u32 Index;

	/* Validate stack top */
	if (CdoParamsStack->OffsetListTop >= (int)XPLM_BEGIN_OFFSET_STACK_SIZE) {
		Status = (u32)XPLM_ERR_INVLD_END_ADDR;
		XPlm_Printf(DEBUG_DETAILED, "Invalid top in End address stack\n");
		goto END;
	}

	for (Index = 0U; Index < PopLevel; Index++) {
		if (CdoParamsStack->OffsetListTop < 0) {
			XPlm_Printf(DEBUG_GENERAL, "End address stack is empty\n");
			Status = (u32)XPLM_ERR_INVLD_BEGIN_END_PAIR;
			break;
		} else {
			*Data = CdoParamsStack->OffsetList[CdoParamsStack->OffsetListTop];
			--CdoParamsStack->OffsetListTop;
			Status = XST_SUCCESS;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function gets the jump offset for break command and break
 * 			supported commands.
 *
 * @param	Cmd is pointer to the command structure
 * @param	Level is the break level
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlm_GetJumpOffSet(XPlm_Cmd *Cmd, u32 Level)
{
	int Status = XST_FAILURE;
	u32 PopAddr = 0U;

	/*
	 * - Break level should not be 0.
	 * - Stack empty, Break does not have begin
	 * - Break level should be always less than no. of stack element
	 */
	if ((Level == 0U) || (Cmd->CdoParamsStack.OffsetListTop < 0) ||
			(Level > (u32)(Cmd->CdoParamsStack.OffsetListTop + 1))) {
		goto END;
	}

	/* If level > 1, then remove (Level - 1) lengths from stack */
	if (Level > XPLM_BEGIN_OFFEST_STACK_DEFAULT_POPLEVEL) {
		Status = XPlm_StackPop(&Cmd->CdoParamsStack, --Level, &PopAddr);
		if ( Status != XST_SUCCESS) {
			goto END;
		}
	}

	/*
	 * Calculate jump offset using "end" length available at the top of
	 * the stack
	 */
	Cmd->BreakLength = Cmd->CdoParamsStack.OffsetList[Cmd->CdoParamsStack.OffsetListTop];

	Status = XST_SUCCESS;
END:
	return Status;
}
