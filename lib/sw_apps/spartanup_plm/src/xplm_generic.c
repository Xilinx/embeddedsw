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
 *       ng   09/17/24 Fixed mask poll flags
 * </pre>
 *
 ******************************************************************************/

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplm_generic.h"
#include "xplm_debug.h"
#include "xstatus.h"
#include "xil_util.h"
#include "xplm_util.h"
#include "xplm_cdo.h"
#include "sleep.h"
#include "xplm_dma.h"
#include "xplm_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/** @cond spartanup_plm_internal */
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

#define XPLM_BEGIN_OFFSET_STACK_SIZE_LIMIT		((s32)XPLM_BEGIN_OFFSET_STACK_SIZE - 1)
#define XPLM_BEGIN_OFFEST_STACK_DEFAULT_POPLEVEL	(1U)
#define XPLM_BEGIN_CMD_EXTRA_OFFSET			(2U)

#define XPLM_RDBK_DIS	(0x1U)

/* DMA write keyhole payload arguments indexes */
#define XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_ADDR_ARG_INDEX			(0x1U)
#define XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_SIZE_ARG_INDEX			(0x2U)
#define XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_DATA_ARG_INDEX			(0x3U)
#define XPLM_WR_KEYHOLE_RESUME_DATA_KEYHOLE_ADDR_ARG_INDEX		(0x0U)
#define XPLM_WR_KEYHOLE_RESUME_DATA_KEYHOLE_SIZE_ARG_INDEX		(0x1U)
#define XPLM_WR_KEYHOLE_CMD_RESUME_PAYLOAD_KEYHOLE_DATA_ARG_INDEX	(0x0U)
/** @endcond */
/************************** Function Prototypes ******************************/
/** @cond spartanup_plm_internal */
static u32 XPlm_KeyHoleXfr(XPlm_KeyHoleXfrParams *KeyHoleXfrParams);
static u32 XPlm_StackPush(XPlm_CdoParamsStack *CdoParamsStack, u32 *Data);
static u32 XPlm_StackPop(XPlm_CdoParamsStack *CdoParamsStack, u32 PopLevel, u32 *Data);
static u32 XPlm_GetJumpOffSet(XPlm_Cmd *Cmd, u32 Level);
/** @endcond */

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	Stores the information about the module ID and the pointers to the
 * CDO command handlers.
 *
 *****************************************************************************/
static XPlm_Module XPlm_Generic;

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief	This function checks if a particular PLM Command ID is supported or not.
 *
 * @param	Cmd is pointer to the command structure with Command ID as the only payload.
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 *****************************************************************************/
static u32 XPlm_Features(XPlm_Cmd *Cmd)
{
	u32 Status = (u32)XST_FAILURE;

	if (Cmd->Payload[0U] < XPlm_Generic.CmdCnt) {
		Status = (u32)XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides no operation. The command is used for
 * alignment purposes.
 *
 * @param	Cmd is not used and no payload is required
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 *****************************************************************************/
static u32 XPlm_Nop(XPlm_Cmd *Cmd)
{
	(void)Cmd;
	XPlm_Printf(DEBUG_DETAILED, "%s\n\r", __func__);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides 32-bit mask poll command execution.
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- Address
 *			- Mask
 *			- Expected Value
 *			- Timeout in us
 *			- Deferred Error flag - Optional
 *				0 - Return error in case of failure,
 *				1 - Ignore error, return success always
 *				2 - Defer error till the end of partition load
 *				3 - Break to end offset in case of failure
 *			- Error Code
 * @return
 * 		- XST_SUCCESS on success.
 * 		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_MaskPoll(XPlm_Cmd *Cmd)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Addr = Cmd->Payload[XPLM_MASKPOLL_ADDR_INDEX];
	u32 Mask = Cmd->Payload[XPLM_MASKPOLL_MASK_INDEX];
	u32 ExpectedValue = Cmd->Payload[XPLM_MASKPOLL_EXP_VAL_INDEX];
	u32 TimeOutInUs = Cmd->Payload[XPLM_MASKPOLL_TIMEOUT_INDEX];
	u32 Flags = 0U;
	u32 Level = 0U;
	u32 DebugLevel = DEBUG_INFO;
	u32 ExtLen = XPLM_MASKPOLL_LEN_EXT;
	u32 MinErrCode = 0U;

	/** - Error out if Cmd length is greater than supported length of 6 parameters. */
	if (Cmd->Len > (ExtLen + 1U)) {
		Status = (u32)XPLM_ERR_MASK_POLL_INVLD_CMD_LEN;
		goto END;
	}

	/* Read flags from the CDO cmd. */
	if (Cmd->Len >= ExtLen) {
		Flags = Cmd->Payload[XPLM_MASKPOLL_FLAGS_INDEX] & XPLM_MASKPOLL_FLAGS_MASK;
	}
	/** - Poll for the expected value till timeout. */
	Status = XPlm_UtilPoll(Addr, Mask, ExpectedValue, TimeOutInUs, NULL);

	/* Print in case of failure or when DEBUG_INFO is enabled */
	if ((Flags != XPLM_MASKPOLL_FLAGS_BREAK) && (Status != (u32)XST_SUCCESS)) {
		DebugLevel = DEBUG_GENERAL;
	}

	XPlm_Printf(DebugLevel, "MaskPoll: Addr: 0x%08x, Mask: 0x%0x, ExpVal: 0x%0x, Timeout: %u",
		    Addr, Mask, ExpectedValue, TimeOutInUs);
	if (Status != (u32)XST_SUCCESS) {
		XPlm_Printf(DebugLevel, ", RegVal: 0x%0x ...ERROR\r\n", Xil_In32(Addr));
	} else {
		XPlm_Printf(DebugLevel, " ...DONE\r\n");
	}

	/**
	 * - Process flags and error code if available in command arguments.
	 * If Flag is set to '0', return the error.
	 * If Flag is set to '1', ignore the error and return XST_SUCCESS.
	 * If Flag is set to '2', defer the error till the end of CDO processing.
	 * If Flag is set to '3', Jump to "end" associated with break level.
	 *
	 * If error code is set, then return it instead of XST_FAILURE.
	 */
	if ((Cmd->Len >= ExtLen) && (Status != (u32)XST_SUCCESS)) {
		MinErrCode = Cmd->Payload[ExtLen] & XPLM_MASKPOLL_MINOR_ERROR_MASK;
		if (Flags == XPLM_MASKPOLL_FLAGS_SUCCESS) {
			/* Ignore the error */
			Status = (u32)XST_SUCCESS;
		} else if (Flags == XPLM_MASKPOLL_FLAGS_DEFERRED_ERR) {
			/* Defer the error till the end of CDO processing */
			Cmd->DeferredError = (u8)TRUE;
		} else if (Flags == XPLM_MASKPOLL_FLAGS_BREAK) {
			Level = (Cmd->Payload[ExtLen - 1U] & XPLM_MASKPOLL_FLAGS_BREAK_LEVEL_MASK) >>
				XPLM_MASKPOLL_FLAGS_BREAK_LEVEL_SHIFT;
			/* Jump to "end" associated with break level */
			Status = XPlm_GetJumpOffSet(Cmd, Level);
		}
		if ((Cmd->Len == (ExtLen + 1U)) && (MinErrCode != 0U) && (Status != (u32)XST_SUCCESS)) {
			/*
			 * Overwrite the error code with the error code value present
			 * in payload argument.
			 */
			Status = (u32)MinErrCode;
		}
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides 32-bit mask write command execution.
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- Address
 *			- Mask
 *			- Value
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 *****************************************************************************/
static u32 XPlm_MaskWrite(XPlm_Cmd *Cmd)
{
	u32 Addr = Cmd->Payload[0U];
	u32 Mask = Cmd->Payload[1U];
	u32 Value = Cmd->Payload[2U];

	XPlm_Printf(DEBUG_DETAILED, "%s, Addr: 0x%08x,  Mask 0x%08x, Value: 0x%08x\n\r", __func__, Addr,
		    Mask, Value);

	XPlm_UtilRMW(Addr, Mask, Value);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides 32 bit Write command execution.
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- Address
 *			- Value
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 *****************************************************************************/
static u32 XPlm_Write(XPlm_Cmd *Cmd)
{
	u32 Addr = Cmd->Payload[0U];
	u32 Value = Cmd->Payload[1U];

	XPlm_Printf(DEBUG_DETAILED, "%s, Addr: 0x%0x,  Val: 0x%0x\n\r", __func__, Addr, Value);

	Xil_Out32(Addr, Value);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides delay command execution.
 *  		Command payload parameter delay in microseconds
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 		- XST_SUCCESS always.
 *
 *****************************************************************************/
static u32 XPlm_Delay(XPlm_Cmd *Cmd)
{
	u32 Delay;

	Delay = Cmd->Payload[0U];
	XPlm_Printf(DEBUG_DETAILED, "%s, Delay: %d\n\r", __func__, Delay);

	usleep(Delay);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides CFI READ command execution.
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- Params - SMAP/JTAG
 *			- Reserved
 *			- Dest Addr
 *			- Read Length in number of words to be read from CFU
 *			- DATA (CFU READ Packets)
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XPLM_ERR_RDBK_DISABLED if readback is disabled.
 *		- XPLM_ERR_RDBK_INVALID_INFR_SEL if the invalid interface type is selected for
 *		readback.
 *		- XPLM_ERR_RDBK_PAYLOAD_TO_CCU if failed to send readback paylod to CCU.
 *		- XPLM_ERR_RDBK_CCU_READ if failed to get readback data from CCU.
 *		- XPLM_ERR_RDBK_READ_TIMEOUT if failed to get readback data from CCU within 268
 *		seconds.
 *
 *****************************************************************************/
static u32 XPlm_CfiRead(XPlm_Cmd *Cmd)
{
	u32 Status = (u32)XST_FAILURE;
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

	/** - Save current SBI control configuration. */
	SbiCtrlCfg = Xil_In32(SLAVE_BOOT_SBI_CTRL);

	/** - Set the readback interface in SBI control config based on the payload. */
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

	/** - Change the SBI mode to configuration mode. */
	XPlm_UtilRMW(SLAVE_BOOT_SBI_MODE, SLAVE_BOOT_SBI_MODE_SELECT_MASK, XPLM_READBK_SBI_CFG_MODE);

	/** - Transfer the readback payload to CCU. */
	Status = XPlm_DmaXfr(CfiPayloadSrcAddr, XPLM_CCU_WR_STREAM_BASEADDR,
			     (Cmd->PayloadLen - XPLM_CFI_DATA_OFFSET), XPLM_DMA_INCR_MODE);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_RDBK_PAYLOAD_TO_CCU;
		goto END;
	}

	/** - Transfer the readback data to SBI buffer. */
	while (Len > 0U) {
		if (Len < XPLM_CCU_RD_STREAM_SIZE_WORDS) {
			ReadLen = Len;
		}
		Status = XPlm_DmaSbiXfer(XPLM_CCU_RD_STREAM_BASEADDR, ReadLen, XPLM_DMA_INCR_MODE);
		if (Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_RDBK_CCU_READ;
			goto END;
		}
		Len -= ReadLen;
	}

	if (SrcType == XPLM_READBK_INTF_TYPE_SMAP) {
		Status = XPlm_UtilPoll(SLAVE_BOOT_SBI_STATUS,
				       SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_MASK,
				       (SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_DEFVAL <<
					SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_SHIFT),
				       XPLM_TIME_OUT_DEFAULT, NULL);
	} else {
		Status = XPlm_UtilPoll(SLAVE_BOOT_SBI_STATUS,
				       SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_MASK,
				       (SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_DEFVAL <<
					SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_SHIFT),
				       XPLM_TIME_OUT_DEFAULT, NULL);
	}

	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_RDBK_READ_TIMEOUT;
	}

END:
	/** - Restore saved SBI control configuration. */
	Xil_Out32(SLAVE_BOOT_SBI_CTRL, SbiCtrlCfg);
	XPlm_UtilRMW(SLAVE_BOOT_SBI_MODE, SLAVE_BOOT_SBI_MODE_SELECT_MASK,
		     SLAVE_BOOT_SBI_MODE_SELECT_DEFVAL);

END1:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides SET command execution.
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- Reserved
 *			- Dest Addr
 *			- Length (Length of words to set to value)
 *			- Value
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XPLM_ERR_SET_MEM if failed to set the memory with the value.
 */
/*****************************************************************************/
static u32 XPlm_Set(XPlm_Cmd *Cmd)
{
	u32 Status = (u32)XST_FAILURE;
	u32 DestAddr = Cmd->Payload[1U];
	u32 Len = Cmd->Payload[2U];
	u32 Val = Cmd->Payload[3U];

	Status = XPlm_MemSet(DestAddr, Val, Len);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_SET_MEM;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides DMA keyhole write command execution.
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- Reserved
 *			- Dest Addr
 *			- Keyhole Size in bytes
 *			- DATA
 *
 * @return
 *		- XST_SUCCESS on success.
 *		- XPLM_ERR_KEYHOLE_XFER if failed to write keyhole data to CCU.
 *
 *****************************************************************************/
static u32 XPlm_DmaWriteKeyHole(XPlm_Cmd *Cmd)
{
	u32 Status = (u32)XST_FAILURE;
	u32 DestAddr;
	u32 SrcAddr;
	u32 Len = Cmd->PayloadLen;
	u32 Keyholesize;
	u32 BaseAddr;
	u32 DestOffset;
	XPlm_KeyHoleXfrParams KeyHoleXfrParams;

	XPlm_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	/* Store the destination address and keyholesize in resume data */
	if (Cmd->ProcessedLen == 0U) {
		Cmd->ResumeData[XPLM_WR_KEYHOLE_RESUME_DATA_KEYHOLE_ADDR_ARG_INDEX] =
			Cmd->Payload[XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_ADDR_ARG_INDEX];
		Cmd->ResumeData[XPLM_WR_KEYHOLE_RESUME_DATA_KEYHOLE_SIZE_ARG_INDEX] =
			Cmd->Payload[XPLM_WR_KEYHOLE_PAYLOAD_KEYHOLE_SIZE_ARG_INDEX];
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
	/**
	 * - Start transferring the keyhole data to CCU, upon reaching the limit of keyhole size,
	 * wrap the keyhole address to the beginning.
	 */
	DestAddr = (((Cmd->ProcessedLen - DestOffset) * XPLM_WORD_LEN) & (Keyholesize - 1U)) + BaseAddr;
	KeyHoleXfrParams.SrcAddr = SrcAddr;
	KeyHoleXfrParams.DestAddr = DestAddr;
	KeyHoleXfrParams.BaseAddr = BaseAddr;
	KeyHoleXfrParams.Len = Len * XPLM_WORD_LEN;
	KeyHoleXfrParams.Keyholesize = Keyholesize;
	KeyHoleXfrParams.Flags = XPLM_DMA_INCR_MODE;
	Status = XPlm_KeyHoleXfr(&KeyHoleXfrParams);
	if (Status != (u32)XST_SUCCESS) {
		Status = (u32)XPLM_ERR_KEYHOLE_XFER;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function adds Debug string to PLM logs.
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- Debug String
 *
 * @return
 *		- XST_SUCCESS always.
 *
 *****************************************************************************/
static u32 XPlm_LogString(XPlm_Cmd *Cmd)
{
	u32 Len = Cmd->PayloadLen * XPLM_WORD_LEN;
	u32 DebugFlag = DEBUG_PRINT_ALWAYS | XPLM_DEBUG_PRINT_STAGE_INFO_MASK ;

	/* Print without timestamp if it is not the beginning of the string */
	if (Cmd->ProcessedLen != 0U) {
		DebugFlag = DEBUG_PRINT_ALWAYS;
	}

	/* Print the string*/
	XPlm_Printf_WoS(DebugFlag, "%.*s", Len, (u8 *)&Cmd->Payload[0U]);

	/* Print new line */
	if (Cmd->Len == (Cmd->ProcessedLen + Cmd->PayloadLen)) {
		XPlm_Printf_WoS(DEBUG_PRINT_ALWAYS, "\n\r");
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function reads the value at an address and displays and adds
 *		the value to PLM logs.
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- Address
 *
 * @return
 *		- XST_SUCCESS always
 *
 *****************************************************************************/
static u32 XPlm_LogAddress(XPlm_Cmd *Cmd)
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
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- Type
 *			- String
 *
 * @return
 *		- XST_SUCCESS always.
 *
 *****************************************************************************/
static u32 XPlm_Marker(XPlm_Cmd *Cmd)
{
	XPlm_Printf(DEBUG_DETAILED, "%s\n\r", __func__);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief	This function provides start of the block.
 *
 * @param	Cmd is pointer to the command structure with the following parameters as payload:
 *			- Offset : specifies no. of words until "end" of the block
 *			- String : optional for debugging purpose (max. 6 words)
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_MAX_NESTED_BEGIN if the nested begin command exceeds the max limit.
 * 		- XPLM_ERR_STORE_END_OFFSET if failed to store the end offset.
 *
 *****************************************************************************/
static u32 XPlm_Begin(XPlm_Cmd *Cmd)
{
	u32 Status = (u32)XST_FAILURE;
	u32 EndOffSet;
	u32 StrLen;
	u32 StartOffset;
	u32 DebugFlag ;

	/* Push to stack, selection of 'StrLen' and 'StartOffset' based on processed length */
	if (Cmd->ProcessedLen == 0U) {
		/* Max 10 nested begin supported */
		if (Cmd->CdoParamsStack.OffsetListTop > XPLM_BEGIN_OFFSET_STACK_SIZE_LIMIT) {
			Status = (u32)XPLM_ERR_MAX_NESTED_BEGIN;
			XPlm_Printf(DEBUG_DETAILED, "Max %d nested begin supported\n", XPLM_BEGIN_OFFSET_STACK_SIZE);
			goto END;
		}

		/* Calculate 'EndOffSet' based on command length */
		if (Cmd->Len > XPLM_MAX_SHORT_CMD_LEN) {
			EndOffSet = Cmd->ProcessedCdoLen + Cmd->Payload[0U] + XPLM_BEGIN_CMD_EXTRA_OFFSET + 1U;
		} else {
			EndOffSet = Cmd->ProcessedCdoLen + Cmd->Payload[0U] + XPLM_BEGIN_CMD_EXTRA_OFFSET;
		}

		/* Push "end" Offset to stack */
		Status = XPlm_StackPush(&Cmd->CdoParamsStack, &EndOffSet);
		if ( Status != (u32)XST_SUCCESS) {
			Status = (u32)XPLM_ERR_STORE_END_OFFSET;
			goto END;
		}

		StrLen = (Cmd->PayloadLen - 1U) * XPLM_WORD_LEN;
		StartOffset = 1U;

		/* Print with timestamp if it is the beginning of the string */
		DebugFlag = DEBUG_PRINT_ALWAYS | XPLM_DEBUG_PRINT_STAGE_INFO_MASK ;
	} else {
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
	Status = (u32)XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides end of the block.
 *
 * @param	Cmd is pointer to the command structure to retrieve the end offset.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XPLM_ERR_INVLD_BEGIN_END_PAIR if begin-end commands are not correctly paired.
 * 		- and errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_End(XPlm_Cmd *Cmd)
{
	u32 Status = (u32)XST_FAILURE;
	u32 EndLength = 0U;

	/* Stack empty, End does not have begin */
	if (Cmd->CdoParamsStack.OffsetListTop < 0) {
		Status = (u32)XPLM_ERR_INVLD_BEGIN_END_PAIR;
		XPlm_Printf(DEBUG_DETAILED, "End does not have valid begin\n");
		goto END;
	}

	/* Popped "end" length from stack should match with current ProcessedCdoLen */
	Status = XPlm_StackPop(&Cmd->CdoParamsStack, XPLM_BEGIN_OFFEST_STACK_DEFAULT_POPLEVEL,
			       &EndLength);
	if (Status != (u32)XST_SUCCESS) {
		goto END;
	}

	if (EndLength != Cmd->ProcessedCdoLen) {
		Status = (u32)XST_FAILURE;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function skips all the commands until the Levels
 * number of nested "end".
 *
 * @param	Cmd is pointer to the command structure with break level as payload.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- Errors from @ref XPlm_Status_t.
 *
 *****************************************************************************/
static u32 XPlm_Break(XPlm_Cmd *Cmd)
{
	u32 Status = (u32)XST_FAILURE;
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
 * @brief	Register the generic CDO command handlers.
 *
 *****************************************************************************/
void XPlm_GenericInit(void)
{
	/* Contains the array of PLM generic commands */
	static const XPlm_ModuleCmd XPlm_GenericCmds[] = {
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

/** @cond spartanup_plm_internal */
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
static u32 XPlm_KeyHoleXfr(XPlm_KeyHoleXfrParams *KeyHoleXfrParams)
{
	u32 Status = (u32)XST_FAILURE;
	u32 LenTemp = (KeyHoleXfrParams->Keyholesize + KeyHoleXfrParams->BaseAddr) -
		      KeyHoleXfrParams->DestAddr;

	if (LenTemp > KeyHoleXfrParams->Len) {
		LenTemp = KeyHoleXfrParams->Len;
	}

	if (LenTemp == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	Status = XPlm_DmaXfr(KeyHoleXfrParams->SrcAddr, KeyHoleXfrParams->DestAddr, LenTemp / XPLM_WORD_LEN,
			     KeyHoleXfrParams->Flags);
	if (Status != (u32)XST_SUCCESS) {
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

		Status = XPlm_DmaXfr(KeyHoleXfrParams->SrcAddr, KeyHoleXfrParams->BaseAddr, LenTemp / XPLM_WORD_LEN,
				     KeyHoleXfrParams->Flags);
		if (Status != (u32)XST_SUCCESS) {
			goto END;
		}
		KeyHoleXfrParams->Len -= LenTemp;
		KeyHoleXfrParams->SrcAddr += LenTemp;
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
 * @param	CdoParamsStack is pointer to the structure to store end offset
 * @param	Data is pointer to the data to be stored on stack
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static u32 XPlm_StackPush(XPlm_CdoParamsStack *CdoParamsStack, u32 *Data)
{
	u32 Status = (u32)XST_FAILURE;

	/* Validate stack top */
	if (CdoParamsStack->OffsetListTop < -1) {
		XPlm_Printf(DEBUG_DETAILED, "Invalid top in End address stack\n");
		goto END;
	}

	if (CdoParamsStack->OffsetListTop >= XPLM_BEGIN_OFFSET_STACK_SIZE_LIMIT) {
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
 * @param	CdoParamsStack is pointer to the structure to get the end offset
 * @param	PopLevel is the number of elements to remove from stack.
 * @param	Data is pointer to store removed data from stack.
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static u32 XPlm_StackPop(XPlm_CdoParamsStack *CdoParamsStack, u32 PopLevel, u32 *Data)
{
	u32 Status = (u32)XST_FAILURE;
	u32 Index;

	/* Validate stack top */
	if (CdoParamsStack->OffsetListTop > XPLM_BEGIN_OFFSET_STACK_SIZE_LIMIT) {
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
 *		supported commands.
 *
 * @param	Cmd is pointer to the command structure
 * @param	Level is the break level
 *
 * @return
 *		- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static u32 XPlm_GetJumpOffSet(XPlm_Cmd *Cmd, u32 Level)
{
	u32 Status = (u32)XST_FAILURE;
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
		Level = Level - 1;
		Status = XPlm_StackPop(&Cmd->CdoParamsStack, Level, &PopAddr);
		if ( Status != (u32)XST_SUCCESS) {
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
/** @endcond */

/** @} end of spartanup_plm_apis group*/
