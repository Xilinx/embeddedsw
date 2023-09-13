/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_generic.c
* @addtogroup xplmi_apis XilPlmi Versal APIs
* @{
* @cond xplmi_internal
* This is the file which contains general commands.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   08/23/2018 Initial release
* 1.01  bsv  04/18/2019 Added support for NPI and CFI readback
*       bsv  05/01/2019 Added support to load CFI bitstreams larger
*			than 64K
*       ma   08/24/2019 Added SSIT commands
* 1.02  bsv  12/13/2019 Added support for NOP and SET commands
*       kc   12/17/2019 Add deferred error mechanism for mask poll
*       bsv  01/09/2020 Changes related to bitstream loading
*       bsv  01/31/2020 Added API to read device ID from hardware
*       ma   03/18/2020 Added event logging code
*       bsv  03/09/2020 Added support for CDO features command
*       bsv  04/04/2020 Code clean up
* 1.03  bsv  06/10/2020 Added SetBoard and GetBoard APIs
*       kc   07/28/2020 Added SetWdt command support
*       skd  07/29/2020 Cfi Write related changes for Qspi and Ospi
*       bm   08/03/2020 Added ReadBack Override support
*       bsv  09/30/2020 Added parallel DMA support for SBI, JTAG, SMAP and PCIE
*                       boot modes
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
*       ana  10/19/2020 Added doxygen comments
* 1.04  td   11/23/2020 MISRA C Rule 17.8 Fixes
*       bsv  01/04/2021 Added support for LogString and LogAddress commands
*       bm   02/17/2021 Added const to XPlmi_GenericCmds variable
*	    bsv  02/28/2021 Added code to avoid unaligned NPI writes
* 1.05  ma   03/04/2021 Added XPlmi_CheckIpiAccess handler to check access for
*                       secure IPI commands
*       ma   03/10/2021 Removed Get Device Id and Get Board commands from
*                       secure commands list
*       ma   03/18/2021 Added support for Marker command
*       bsv  04/13/2021 Added support for variable Keyhole sizes in
*                       DmaWriteKeyHole command
* 1.06  ma   06/17/2021 Added readback support for SSIT Slave SLRs
*       ma   06/28/2021 Added support for proc command
*       bsv  07/05/2021 Added code to handle case where bitstream data starts
*                       at 32K boundary
*       bsv  07/16/2021 Fix doxygen warnings
*       bsv  07/18/2021 Debug enhancements
*       bsv  08/02/2021 Code clean up to reduce size
*       bsv  08/15/2021 Removed unwanted goto statements
*       rb   08/19/2021 Fix compilation warning
* 1.07  bsv  10/26/2021 Code clean up
*       ma   11/22/2021 Remove hardcoding of Proc addresses
*       kpt  12/13/2021 Replaced Xil_SecureMemCpy with Xil_SMemCpy
*       is   01/10/2022 Added support for OT_CHECK command (XPlmi_OTCheck)
*       is   01/10/2022 Updated Copyright Year to 2022
*       bm   01/20/2022 Fix compilation warnings in Xil_SMemCpy
* 1.08  bsv  06/03/2022 Add CommandInfo to a separate section in elf
*       bm   07/06/2022 Refactor versal and versal_net code
*       ma   07/08/2022 Add support for storing procs to PMC RAM based on ID
*       ma   07/08/2022 Add ScatterWrite and ScatterWrite2 commands to versal
*       ma   07/08/2022 Add support for Tamper Trigger over IPI
*       ma   07/20/2022 Print XPlmi_MaskPoll failures in all cases
*       bm   07/20/2022 Retain critical data structures after In-Place PLM Update
*       bm   07/24/2022 Set PlmLiveStatus during boot time
*       bsv  08/23/2022 Clear BoardParams instance in case of failure
*       bm   08/24/2022 Support Begin, Break and End commands across chunk
*                       boundaries
*       bm   08/30/2022 Ignore strings in begin command beyond 24 characters
*                       instead of erroring out
*       bm   09/14/2022 Move ScatterWrite commands from common to versal_net
* 1.09  ng   11/11/2022 Updated doxygen comments
*       bm   01/03/2023 Create Secure Lockdown as a Critical Priority Task
*       bm   01/03/2023 Clear End Stack before processing a CDO partition
*       bm   01/03/2023 Notify Other SLRs about Secure Lockdown
*       sk   01/11/2023 Update XPlmi_MoveProc to handle 64 bit Address
*       bm   01/18/2023 Fix CFI readback logic with correct keyhole size
*       ng   03/07/2023 Fixed circular dependency between xilpm and xilplmi
*                       libraries
*       bm   03/11/2023 Added Temporal redundancy to tamper response condition
*       ng   03/16/2023 Added control to disable minimal timeout in maskpoll
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.10  ng   05/19/2023 Added null termination for log string buffer
*       bm   06/13/2023 Added unique error code support for mask poll.
*                       Also added generic logic for 32-bit and 64-bit mask poll.
*       bm   06/13/2023 Log PLM error before deferring
*       bm   06/23/2023 Added access permissions for IPI commands
*                       Added set_ipi_access cdo command
*       bm   07/06/2023 Updated Begin offset Stack logic
*       bm   07/06/2023 Refactored Proc logic to more generic logic
*       bm   07/06/2023 Added list commands
*       sk   08/30/2023 Added Address range check for PSM Buff List
*       dd   09/12/2023 MISRA-C violation Rule 14.4 fixed
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_generic.h"
#include "xstatus.h"
#include "xplmi_proc.h"
#include "xplmi_hw.h"
#include "xcfupmc.h"
#include "sleep.h"
#include "xplmi_event_logging.h"
#include "xplmi_wdt.h"
#include "xplmi_modules.h"
#include "xplmi_cmd.h"
#include "xil_util.h"
#include "xplmi_cdo.h"
#include "xplmi_sysmon.h"
#include "xplmi.h"
#ifndef VERSAL_NET
#include "xplmi_ssit.h"
#endif
#include "xplmi_plat.h"
#include "xplmi_tamper.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XPLMI_LOG_ADDR_ARG_LOW_ADDR_INDEX	(0U)
#define XPLMI_LOG_ADDR_ARG_HIGH_ADDR_INDEX	(1U)
#define XPLMI_LOG_ADDR_MAX_ARGS		(2U)
#define XPLMI_MAX_LOG_STR_LEN	(256U)
#define XPLMI_ERR_LOG_STRING	(2U)
#define XPLMI_SRC_ALIGN_REQ	(0U)
#define XPLMI_DEST_ALIGN_REQ	(1U)
#define XPLMI_LEN_ALIGN_REQ	(2U)

/* Mask poll related macros */
#define XPLMI_MASKPOLL_ADDR_INDEX		(0U)
#define XPLMI_MASKPOLL_ADDR_64BIT_INDEX		(1U)
#define XPLMI_MASKPOLL_MASK_INDEX		(1U)
#define XPLMI_MASKPOLL_EXP_VAL_INDEX		(2U)
#define XPLMI_MASKPOLL_TIMEOUT_INDEX		(3U)
#define XPLMI_MASKPOLL_FLAGS_INDEX		(4U)
#define XPLMI_MASKPOLL_MINOR_ERROR_MASK		(0xFFFFU)
#define XPLMI_MASK_POLL_32BIT_OFFSET	(0U)
#define XPLMI_MASK_POLL_64BIT_OFFSET	(1U)


#define XPLMI_BEGIN_MAX_LOG_STR_LEN			(24U)
#define XPLMI_BEGIN_LOG_STR_BUF_END_INDEX	(XPLMI_BEGIN_MAX_LOG_STR_LEN - 1U)
#define XPLMI_BEGIN_OFFSET_STACK_SIZE			(10U)
#define XPLMI_BEGIN_OFFEST_STACK_DEFAULT_POPLEVEL	(1U)
#define XPLMI_BEGIN_CMD_EXTRA_OFFSET			(2U)
#define XPLMI_BEGIN_MAX_STRING_WORD_LEN		(XPLMI_BEGIN_MAX_LOG_STR_LEN /  XPLMI_WORD_LEN)

/* Masks and shift defines of set ipi access command */
#define XPLMI_SET_IPI_ACCESS_MODULE_ID_MASK	(0xFFU)
#define XPLMI_SET_IPI_ACCESS_API_ID_LOWER_MASK	(0xFF00U)
#define XPLMI_SET_IPI_ACCESS_API_ID_LOWER_SHIFT	(8U)
#define XPLMI_SET_IPI_ACCESS_API_ID_UPPER_MASK	(0xFF0000U)
#define XPLMI_SET_IPI_ACCESS_API_ID_UPPER_SHIFT	(16U)

/* Index defines of set ipi access command payload */
#define XPLMI_SET_IPI_ACCESS_API_ID_VAL_INDEX	(0U)
#define XPLMI_SET_IPI_ACCESS_PERM_VAL_INDEX	(1U)

/* Maximum procs supported */
#define XPLMI_MAX_PSM_BUFFERS		(10U)
#define XPLMI_MAX_PMC_BUFFERS		(5U)

/* Secure Lockdown and SRST Tamper response mask */
#define XPLMI_SLD_AND_SRST_TAMPER_RESP_MASK	(0xEU)

/* CFU keyhole size in words */
#define XPLMI_CFU_KEYHOLE_SIZE		(0x10000U)

/**< Versal Subsystem node ID for PMC. */
#define XPLMI_PMC_SUBSYS_NODE_ID	(0x1c000001U)

/************************** Function Prototypes ******************************/
static int XPlmi_CfiWrite(u64 SrcAddr, u64 DestAddr, u32 Keyholesize, u32 Len,
        XPlmi_Cmd* Cmd);
static XPlmi_ReadBackProps* XPlmi_GetReadBackPropsInstance(void);
static int XPlmi_DmaUnalignedXfer(u64* SrcAddr, u64* DestAddr, u32* Len,
	u8 Flag);
static int XPlmi_KeyHoleXfr(XPlmi_KeyHoleXfrParams* KeyHoleXfrParams);
static int XPlmi_StackPush(XPlmi_CdoParamsStack *CdoParamsStack, u32 *Data);
static int XPlmi_StackPop(XPlmi_CdoParamsStack *CdoParamsStack, u32 PopLevel, u32 *Data);
static int XPlmi_TamperTrigger(XPlmi_Cmd *Cmd);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	Contains the module ID and PLM generic commands array
 *
 *****************************************************************************/
static XPlmi_Module XPlmi_Generic;

/**
 * @}
 * @endcond
 */

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief	This function checks if a particular PLM Command ID is supported
 * 			or not. Command ID is the only payload parameter.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_Features(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	XPLMI_EXPORT_CMD(XPLMI_FEATURES_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_MAJOR_MODULE_VERSION, XPLMI_MINOR_MODULE_VERSION);

	if (Cmd->Payload[0U] < XPlmi_Generic.CmdCnt) {
		Cmd->Response[1U] = (u32)XST_SUCCESS;
	} else {
		Cmd->Response[1U] = (u32)XST_FAILURE;
	}
	Status = XST_SUCCESS;
	Cmd->Response[0U] = (u32)Status;

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
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_Nop(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	XPLMI_EXPORT_CMD(XPLMI_NOP_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ZERO, XPLMI_UNLIMITED_ARG_CNT);

	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads the device ID and fills in the response buffer.
 *		Command: GetDeviceID
 *  		Reserved[31:24]=0 Length[23:16]=[0] PLM=1 CMD_DEVID=18
 *  		Payload = 0
 *  		The command reads PMC_TAP_IDCODE register and
 *		EFUSE_CACHE_IP_DISABLE_0 register and fills the command
 *		response array with these values.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_GetDeviceID(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 ExtIdCode;
	XPLMI_EXPORT_CMD(XPLMI_GET_DEVICE_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ZERO, XPLMI_CMD_ARG_CNT_ZERO);

	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);

	Cmd->Response[1U] = XPlmi_In32(PMC_TAP_IDCODE);
	ExtIdCode = XPlmi_In32(EFUSE_CACHE_IP_DISABLE_0)
			& EFUSE_CACHE_IP_DISABLE_0_EID_MASK;
	if (ExtIdCode != 0U) {
		if ((ExtIdCode & EFUSE_CACHE_IP_DISABLE_0_EID_SEL_MASK) == 0U) {
			ExtIdCode = (ExtIdCode & EFUSE_CACHE_IP_DISABLE_0_EID1_MASK)
						>> EFUSE_CACHE_IP_DISABLE_0_EID1_SHIFT;
		} else {
			ExtIdCode = (ExtIdCode & EFUSE_CACHE_IP_DISABLE_0_EID2_MASK)
					>> EFUSE_CACHE_IP_DISABLE_0_EID2_SHIFT;
		}
	}

	Cmd->Response[2U] = ExtIdCode;
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides generic implementation for both 32-bit and 64-bit
 *		mask poll command execution.
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
 * @param	Is64Bit is flag indicating if the address is 64-bit
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_GenericMaskPoll(XPlmi_Cmd *Cmd, u64 Addr, u32 Type)
{
	int Status = XST_FAILURE;
	u32 Mask;
	u32 ExpectedValue;
	u32 TimeOutInUs;
	u32 Flags = 0U;
	u32 Level = 0U;
	u16 DebugLevel = DEBUG_INFO;
	u32 SetMinTimeout = TRUE;
	u32 ExtLen;
	u32 MinErrCode = 0U;
	u32 Offset = XPLMI_MASK_POLL_32BIT_OFFSET;
#ifdef PLM_PRINT_PERF_POLL
	u64 PollTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime = {0U};
#endif

	if (Type == XPLMI_MASK_POLL_64BIT_TYPE) {
		Offset = XPLMI_MASK_POLL_64BIT_OFFSET;
	}

	/* Initialize variables */
	Mask = Cmd->Payload[XPLMI_MASKPOLL_MASK_INDEX + Offset];
	ExpectedValue = Cmd->Payload[XPLMI_MASKPOLL_EXP_VAL_INDEX + Offset];
	TimeOutInUs = Cmd->Payload[XPLMI_MASKPOLL_TIMEOUT_INDEX + Offset];
	ExtLen = XPLMI_MASKPOLL_LEN_EXT + Offset;

	/* Error out if Cmd length is greater than supported length */
	if (Cmd->Len > (ExtLen + 1U)) {
		goto END;
	}

	/* Disable MinTimeout for Success and break flags. Also disable if explicity requested using MSB */
	if (Cmd->Len >= ExtLen) {
		Flags = Cmd->Payload[XPLMI_MASKPOLL_FLAGS_INDEX + Offset] & XPLMI_MASKPOLL_FLAGS_MASK;
		if ((Flags == XPLMI_MASKPOLL_FLAGS_BREAK) || (Flags == XPLMI_MASKPOLL_FLAGS_SUCCESS) ||
			(Cmd->Payload[XPLMI_MASKPOLL_FLAGS_INDEX + Offset] &
			 XPLMI_MASKPOLL_FLAGS_DISABLE_MINIMAL_TIMEOUT)) {
				SetMinTimeout = FALSE;
		}
	}

	/* Set Minimum Timeout */
	if (SetMinTimeout == TRUE) {
		if (TimeOutInUs < XPLMI_MASK_POLL_MIN_TIMEOUT) {
			TimeOutInUs = XPLMI_MASK_POLL_MIN_TIMEOUT;
		}
	}

	/* Mask Poll for expected value */
	if (Type == XPLMI_MASK_POLL_64BIT_TYPE) {
		Status = XPlmi_UtilPoll64(Addr, Mask, ExpectedValue, TimeOutInUs);
	}
	else {
		Status = XPlmi_UtilPoll((u32)Addr, Mask, ExpectedValue, TimeOutInUs, NULL);
	}

	/* Print in case of failure or when DEBUG_INFO is enabled */
	if ((Flags != XPLMI_MASKPOLL_FLAGS_BREAK) && (Status != XST_SUCCESS)) {
		DebugLevel = DEBUG_GENERAL;
	}

	XPlmi_Printf(DebugLevel, "MaskPoll: Addr: 0x%0x%08x, Mask: 0x%0x, ExpVal: 0x%0x, Timeout: %u",
		(u32)(Addr >> 32U), (u32)(Addr & MASK_ALL), Mask, ExpectedValue, TimeOutInUs);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf_WoTS(DebugLevel, ", RegVal: 0x%0x ...ERROR\r\n", XPlmi_In64(Addr));
	}
	else {
		XPlmi_Printf_WoTS(DebugLevel, " ...DONE\r\n");
	}

#ifdef PLM_PRINT_PERF_POLL
	/* Print poll time */
	XPlmi_MeasurePerfTime(PollTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF,
		" %u.%03u ms Poll Time: Addr: 0x%x%08x, Mask: 0x%0x,"
		" ExpVal: 0x%0x, Timeout: %u \r\n",
		(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac,
		(u32)(Addr >> 32U), (u32)(Addr & MASK_ALL), Mask, ExpectedValue, TimeOutInUs);
#endif

	/*
	 * If command length is greater than Optional arguments length,
	 * then flags and error code are processed
	 */
	if ((Cmd->Len >= ExtLen) && (Status != XST_SUCCESS)) {
		MinErrCode = Cmd->Payload[ExtLen] & XPLMI_MASKPOLL_MINOR_ERROR_MASK;
		if (Flags == XPLMI_MASKPOLL_FLAGS_SUCCESS) {
			/* Ignore the error */
			Status = XST_SUCCESS;
		} else if (Flags == XPLMI_MASKPOLL_FLAGS_DEFERRED_ERR) {
			/* Defer the error till the end of CDO processing */
			Cmd->DeferredError = (u8)TRUE;
		} else if (Flags == XPLMI_MASKPOLL_FLAGS_BREAK) {
			Level = (Cmd->Payload[ExtLen - 1U] &
					XPLMI_MASKPOLL_FLAGS_BREAK_LEVEL_MASK) >>
					XPLMI_MASKPOLL_FLAGS_BREAK_LEVEL_SHIFT;
			/* Jump to "end" associated with break level */
			Status = XPlmi_GetJumpOffSet(Cmd, Level);
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
 * @brief	This function provides 32 bit mask poll command execution.
 *  		Command payload parameters are
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
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_MaskPoll(XPlmi_Cmd *Cmd)
{
	XPLMI_EXPORT_CMD(XPLMI_MASK_POLL_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_FOUR, XPLMI_CMD_ARG_CNT_SIX);

	return XPlmi_GenericMaskPoll(Cmd, (u64)Cmd->Payload[XPLMI_MASKPOLL_ADDR_INDEX],
			XPLMI_MASK_POLL_32BIT_TYPE);
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
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_MaskWrite(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Addr = Cmd->Payload[0U];
	u32 Mask = Cmd->Payload[1U];
	u32 Value = Cmd->Payload[2U];
	XPLMI_EXPORT_CMD(XPLMI_MASK_WRITE_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_THREE, XPLMI_CMD_ARG_CNT_THREE);

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%08x,  Mask 0x%08x, Value: 0x%08x\n\r",
		__func__, Addr, Mask, Value);

	XPlmi_UtilRMW(Addr, Mask, Value);
	Status = XST_SUCCESS;

	return Status;
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
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_Write(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Addr = Cmd->Payload[0U];
	u32 Value = Cmd->Payload[1U];
	XPLMI_EXPORT_CMD(XPLMI_WRITE_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x,  Val: 0x%0x\n\r",
		__func__, Addr, Value);

	XPlmi_Out32(Addr, Value);
	Status = XST_SUCCESS;

	return Status;
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
static int XPlmi_Delay(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Delay;
	XPLMI_EXPORT_CMD(XPLMI_DELAY_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);

	Delay = Cmd->Payload[0U];
	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Delay: %d\n\r",
		__func__, Delay);

	usleep(Delay);
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides functionality for DMA write.
 *
 * @param	Dest is the destination address
 * @param	Src is the source address
 * @param	Len is the number of words to be transferred
 * @param	Flags is the DMA transfer related flags
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_UNALIGNED_DMA_XFER if unaligned DMA transfer fails.
 *
 *****************************************************************************/
int XPlmi_DmaTransfer(u64 Dest, u64 Src, u32 Len, u32 Flags)
{
	int Status = XST_FAILURE;
	u32 UnalignedWordCount;
	u32 Offset;
	u64 DestAddr = Dest;
	u64 SrcAddr = Src;
	u32 Length = Len;

	Status = XPlmi_DmaUnalignedXfer(&SrcAddr, &DestAddr, &Length,
		XPLMI_DEST_ALIGN_REQ);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_UNALIGNED_DMA_XFER,
			Status);
		goto END;
	}

	UnalignedWordCount = Length % XPLMI_WORD_LEN;
	Length = Length - UnalignedWordCount;

	Status = XPlmi_DmaXfr(SrcAddr, DestAddr, Length, Flags);
	if(Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Failed\n\r");
		goto END;
	}

	Offset = Length * XPLMI_WORD_LEN;
	SrcAddr += Offset;
	DestAddr += Offset;
	Status = XPlmi_DmaUnalignedXfer(&SrcAddr, &DestAddr,
		&UnalignedWordCount, XPLMI_LEN_ALIGN_REQ);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_UNALIGNED_DMA_XFER,
			Status);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides DMA write command execution.
 *  		Command payload parameters are
 *		- High Dest Addr
 *		- Low Dest Addr
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_DmaWrite(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 DestAddr;
	u64 SrcAddr;
	u32 Len = Cmd->PayloadLen;
	u32 DestOffset = 0U;
	XPLMI_EXPORT_CMD(XPLMI_DMA_WRITE_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_TWO, XPLMI_UNLIMITED_ARG_CNT);

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	if (Cmd->ProcessedLen == 0U) {
		/* Store the destination address in resume data */
		Cmd->ResumeData[0U] = Cmd->Payload[0U];
		Cmd->ResumeData[1U] = Cmd->Payload[1U];
		SrcAddr = (u64)(UINTPTR) &Cmd->Payload[2U];
		Len -= 2U;
	} else {
		SrcAddr = (u64)(UINTPTR) &Cmd->Payload[0U];
		/* Decrement the destination offset by CMD params */
		DestOffset = 2U;
	}

	DestAddr = (u64) Cmd->ResumeData[0U];
	DestAddr = ((u64)Cmd->ResumeData[1U] | (DestAddr << 32U));
	DestAddr += (((u64)Cmd->ProcessedLen - DestOffset) * XPLMI_WORD_LEN);

	/* Call XPlmi_DmaTransfer with flags DMA0 and INCR */
	Status = XPlmi_DmaTransfer(DestAddr, SrcAddr, Len, XPLMI_PMCDMA_0);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides 64 bit mask poll command execution.
 *  		Command payload parameters are
 *		- High Address
 *		- Low Address
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
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_MASKPOLL64 if failed to poll a 64-bit register value.
 *
 *****************************************************************************/
static int XPlmi_MaskPoll64(XPlmi_Cmd *Cmd)
{
	u64 Addr;

	XPLMI_EXPORT_CMD(XPLMI_MASK_POLL64_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_FIVE, XPLMI_CMD_ARG_CNT_SEVEN);

	Addr = (u64)Cmd->Payload[XPLMI_MASKPOLL_ADDR_64BIT_INDEX];
	Addr |= (u64)Cmd->Payload[XPLMI_MASKPOLL_ADDR_INDEX] << 32U;

	return XPlmi_GenericMaskPoll(Cmd, Addr, XPLMI_MASK_POLL_64BIT_TYPE);
}

/*****************************************************************************/
/**
 * @brief	This function provides 64 bit mask write command execution.
 *  		Command payload parameters are
 *		- High Address
 *		- Low Address
 *		- Mask
 *		- Value
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_MaskWrite64(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 Addr = ((u64)Cmd->Payload[0U] << 32U) | Cmd->Payload[1U];
	u32 Mask = Cmd->Payload[2U];
	u32 Value = Cmd->Payload[3U];
	u32 ReadVal;
	XPLMI_EXPORT_CMD(XPLMI_MASK_WRITE64_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_FOUR, XPLMI_CMD_ARG_CNT_FOUR);

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x%08x,  Mask 0x%0x, Val: 0x%0x\n\r",
		__func__, (u32)(Addr >> 32U), (u32)Addr, Mask, Value);

	/*
	 * Read the Register value
	 */
	ReadVal = XPlmi_In64(Addr);
	ReadVal = (ReadVal & (~Mask)) | (Mask & Value);
	XPlmi_Out64(Addr, ReadVal);
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides 64 bit write command execution.
 *  		Command payload parameters are
 *		- High Address
 *		- Low Address
 *		- Value
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_Write64(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 Addr = ((u64)Cmd->Payload[0U] << 32U) | Cmd->Payload[1U];
	u32 Value = Cmd->Payload[2U];
	XPLMI_EXPORT_CMD(XPLMI_WRITE64_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_THREE, XPLMI_CMD_ARG_CNT_THREE);

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x%08x,  Val: 0x%0x\n\r",
		__func__, (u32)(Addr>>32), (u32)Addr, Value);

	XPlmi_Out64(Addr, Value);
	Status = XST_SUCCESS;

	return Status;
}

/**
 * @{
 * @cond xplmi_internal
 */
/*****************************************************************************/
/**
 * @brief	The function reads data from Npi address space.
 *
 * @param	Src Address in Npi address space.
 * @param   	Destination Address
 * @param	Len is number of words to be read
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static int XPlmi_NpiRead(u64 SrcAddr, u64 DestAddr, u32 Len)
{
	int Status = XST_FAILURE;
	u32 Count = Len;
	u32 Offset;
	u32 XferLen;
	XPlmi_ReadBackProps *ReadBackPtr = XPlmi_GetReadBackPropsInstance();
	u64 Dest = DestAddr;
	u64 Src = SrcAddr;

	/**
	 * Check if Readback Dest Addr is Overriden
	 */
	if ((XPLMI_READBACK_DEF_DST_ADDR != ReadBackPtr->DestAddr) &&
			(XPLMI_SBI_DEST_ADDR != Dest)) {
		Dest = ReadBackPtr->DestAddr;
		if ((Len + ReadBackPtr->ProcessedLen) > ReadBackPtr->MaxSize) {
			Status = XPLMI_ERR_READBACK_BUFFER_OVERFLOW;
			XPlmi_Printf(DEBUG_GENERAL,
				"ReadBack Buffer Overflow\n\r");
			goto END;
		}
	}

	/**
	 * For NPI READ command, the source address needs to be
	 * 16 byte aligned. Use XPlmi_Out64 till the destination address
	 * becomes 16 byte aligned.
	 */
	Status = XPlmi_DmaUnalignedXfer(&Src, &Dest, &Count,
		XPLMI_SRC_ALIGN_REQ);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_UNALIGNED_DMA_XFER,
			Status);
		goto END;
	}

	XferLen = Count & (~(XPLMI_WORD_LEN - 1U));

	if (Dest != XPLMI_SBI_DEST_ADDR) {
		Status = XPlmi_DmaXfr(Src, Dest, XferLen, XPLMI_PMCDMA_0);
	} else {
		Status = XPlmi_DmaSbiXfer(Src, XferLen, XPLMI_PMCDMA_1);
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 *  For NPI_READ command, Offset variable should
	 *  be updated with the unaligned bytes.
	 */
	Offset = XferLen * XPLMI_WORD_LEN;
	Src += Offset;
	Dest += Offset;
	Count = Count - XferLen;
	Status = XPlmi_DmaUnalignedXfer(&Src, &Dest, &Count,
		XPLMI_LEN_ALIGN_REQ);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_UNALIGNED_DMA_XFER,
			Status);
		goto END;
	}

	if ((XPLMI_READBACK_DEF_DST_ADDR != ReadBackPtr->DestAddr) &&
			(XPLMI_SBI_DEST_ADDR != Dest)) {
		ReadBackPtr->ProcessedLen += Len;
		ReadBackPtr->DestAddr += ((u64)Len * XPLMI_WORD_LEN);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function does unaligned DMA transfers.
 *
 * @param	SrcAddr is Source Address space
 * @param   	DestAddr is the Destination Address
 * @param	Len is the number of words to transfer out of which only
 *		unaligned words(0, 1, 2 or 3) would be transferred via
 *		memory writes in this API.
 * @param	Flag is the parameter that indicates whether SrcAddr, DestAddr
 *		or Len should be made aligned with 16 bytes
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_DmaUnalignedXfer(u64* SrcAddr, u64* DestAddr, u32* Len,
	u8 Flag)
{
	int Status = XST_FAILURE;
	u32 RegVal;
	u8 Offset;
	u8 Count;

	if (Flag == XPLMI_SRC_ALIGN_REQ) {
		Offset = (u8)(XPLMI_WORD_LEN -
			(((*SrcAddr) & XPLMI_SIXTEEN_BYTE_MASK) / XPLMI_WORD_LEN));
		Count = Offset % XPLMI_WORD_LEN;
	}
	else if (Flag == XPLMI_DEST_ALIGN_REQ) {
		Offset = (u8)(XPLMI_WORD_LEN -
			(((*DestAddr) & XPLMI_SIXTEEN_BYTE_MASK) / XPLMI_WORD_LEN));
		Count = Offset % XPLMI_WORD_LEN;
	}
	else if (Flag == XPLMI_LEN_ALIGN_REQ) {
		Count = (u8)((*Len) % XPLMI_WORD_LEN);
	}
	else {
		goto END;
	}

	if (Count > *Len) {
		Count = (u8)*Len;
	}
	*Len -= Count;
	while (Count > 0U) {
		RegVal = XPlmi_In64(*SrcAddr);
		XPlmi_Out64(*DestAddr, RegVal);
		*SrcAddr += XPLMI_WORD_LEN;
		*DestAddr += XPLMI_WORD_LEN;
		--Count;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}
/**
 * @}
 * @endcond
 */

/*****************************************************************************/
/**
 * @brief	This function provides DMA Xfer command execution.
 *  		Command payload parameters are
 *		- High Src Addr
 *		- Low Src Addr
 *		- High Dest Addr
 *		- Low Dest Addr
 *		- Params - AXI burst type (Fixed/INCR)
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_DmaXfer(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 DestAddr;
	u64 SrcAddr;
	u32 Len;
	u32 Flags;
	XPLMI_EXPORT_CMD(XPLMI_DMA_XFER_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_SIX, XPLMI_CMD_ARG_CNT_SIX);

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	if (Cmd->ProcessedLen == 0U) {
		/* Store the command fields in resume data */
		Cmd->ResumeData[0U] = Cmd->Payload[0U];
		Cmd->ResumeData[1U] = Cmd->Payload[1U];
		Cmd->ResumeData[2U] = Cmd->Payload[2U];
		Cmd->ResumeData[3U] = Cmd->Payload[3U];
		Cmd->ResumeData[4U] = Cmd->Payload[4U];
		Cmd->ResumeData[5U] = Cmd->Payload[5U];
	}

	SrcAddr = (u64)Cmd->ResumeData[0U];
	SrcAddr = ((u64)Cmd->ResumeData[1U] | (SrcAddr << 32U));
	DestAddr = (u64)Cmd->ResumeData[2U];
	DestAddr = ((u64)Cmd->ResumeData[3U] | (DestAddr << 32U));
	DestAddr += ((u64)Cmd->ProcessedLen * XPLMI_WORD_LEN);
	Len = Cmd->ResumeData[4U];

	/* Set DMA flags to DMA0 */
	Flags = Cmd->ResumeData[5U] | XPLMI_PMCDMA_0;

	if (DestAddr == XPLMI_SBI_DEST_ADDR) {
		XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE,
			SLAVE_BOOT_SBI_MODE_SELECT_MASK,
			SLAVE_BOOT_SBI_MODE_SELECT_MASK);
		if ((Flags & XPLMI_DMA_SRC_NPI) == XPLMI_DMA_SRC_NPI) {
			Status = XPlmi_NpiRead(SrcAddr, DestAddr, Len);
		} else {
			Status = XPlmi_DmaSbiXfer(SrcAddr, Len,
					Cmd->ResumeData[5U] | XPLMI_PMCDMA_1);
		}
		if (Status != XST_SUCCESS) {
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE,
				SLAVE_BOOT_SBI_MODE_SELECT_MASK, 0U);
			goto END;
		}

		Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
				SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_MASK,
				SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_VAL,
				XPLMI_TIME_OUT_DEFAULT, NULL);
		if (Status != XST_SUCCESS) {
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE,
				SLAVE_BOOT_SBI_MODE_SELECT_MASK, 0U);
			goto END;
		}

		Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
				SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_MASK,
				SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_VAL,
				XPLMI_TIME_OUT_DEFAULT, NULL);
		XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE,
				SLAVE_BOOT_SBI_MODE_SELECT_MASK, 0U);
	} else {
		if ((Flags & XPLMI_DMA_SRC_NPI) == XPLMI_DMA_SRC_NPI) {
			Status = XPlmi_NpiRead(SrcAddr, DestAddr, Len);
		} else {
			Status = XPlmi_DmaXfr(SrcAddr, DestAddr, Len, Flags);
		}
	}
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA XFER Failed\n\r");
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides INIT_SEQ command execution.
 *  		Command payload parameters are
 *		- DATA
 *
 * @param	Cmd is pointer to the command structure and unused
 *
 * @return
 * 			- XPLMI_ERR_CMD_NOT_SUPPORTED always.
 *
 *****************************************************************************/
static int XPlmi_InitSeq(XPlmi_Cmd *Cmd)
{
	XPLMI_EXPORT_CMD(XPLMI_INIT_SEQ_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ZERO, XPLMI_UNLIMITED_ARG_CNT);
	/* For MISRA C */
	(void)Cmd;

	return XPLMI_ERR_CMD_NOT_SUPPORTED;
}


/*****************************************************************************/
/**
 * @brief	This function processes and provides readback length
 *
 * @param	Len is the current readback length

 * @return	Readback length
 *
 *****************************************************************************/
static u32 XPlmi_GetReadbackLen(u32 Len)
{
	u32 ReadLen;

	if (Len > XPLMI_CFU_KEYHOLE_SIZE) {
		ReadLen = XPLMI_CFU_KEYHOLE_SIZE;
	} else {
		ReadLen = Len;
	}

	return ReadLen;
}

/*****************************************************************************/
/**
 * @brief	This function provides CFI READ command execution.
 *  		Command payload parameters are
 *		- Params - SMPA/JTAG/DDR
 *		- High Dest Addr
 *		- Low Dest Addr
 *		- Read Length in number of words to be read from CFU
 *		- DATA (CFU READ Packets)
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_READBACK_BUFFER_OVERFLOW if readback buffer overflows.
 *
 *****************************************************************************/
static int XPlmi_CfiRead(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 SrcType = Cmd->Payload[0U] & XPLMI_READBACK_SRC_MASK;
	u32 SlrType = (Cmd->Payload[0U] & XPLMI_READBACK_SLR_TYPE_MASK) >>
					XPLMI_READBACK_SLR_TYPE_SHIFT;
	u64 DestAddrHigh;
	u32 DestAddrLow;
	u64 SrcAddr;
	u64 DestAddrRead;
	u64 DestAddr = 0UL;
	XPlmi_ReadBackProps *ReadBackPtr = XPlmi_GetReadBackPropsInstance();
	u32 ReadLen;
	u32 Len = Cmd->Payload[3U];
	u32 CfiPayloadSrcAddr = (u32)(&Cmd->Payload[XPLMI_CFI_DATA_OFFSET]);
	XPLMI_EXPORT_CMD(XPLMI_CFI_READ_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_FOUR, XPLMI_UNLIMITED_ARG_CNT);

	/**
	 * Read the destination interface and destination address
	*/
	XPlmi_GetReadbackSrcDest(SlrType, &SrcAddr, &DestAddrRead);
	ReadLen = XPlmi_GetReadbackLen(Len);
	/** Set MaxOutCommands of PMC_DMA1 to 1 */
	XPlmi_SetMaxOutCmds(XPLMI_MAXOUT_CMD_MIN_VAL);

	/**
	 * If the destination interface is not DDR, write to the SLAVE BOOT REGISTER
	 * to set the SBI mode correctly.
	*/
	if (SrcType == XPLMI_READBK_INTF_TYPE_DDR) {
		/* Check if Readback Dest Addr is Overriden */
		if (XPLMI_READBACK_DEF_DST_ADDR != ReadBackPtr->DestAddr) {
			DestAddr = ReadBackPtr->DestAddr;
			if ((Len + ReadBackPtr->ProcessedLen) >
				ReadBackPtr->MaxSize) {
				Status = XPLMI_ERR_READBACK_BUFFER_OVERFLOW;
				XPlmi_Printf(DEBUG_GENERAL,
					"ReadBack Buffer Overflow\n\r");
				goto END;
			}
		}
		else {
			DestAddrHigh = Cmd->Payload[1U];
			DestAddrLow =  Cmd->Payload[2U];
			DestAddr = (DestAddrHigh << 32U) | DestAddrLow;
		}
		Status = XPlmi_DmaXfr(SrcAddr, DestAddr, ReadLen,
				XPLMI_PMCDMA_1 | XPLMI_DMA_SRC_NONBLK);
	} else {
		if (SrcType == XPLMI_READBK_INTF_TYPE_JTAG) {
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
				SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
				XPLMI_SBI_CTRL_INTERFACE_JTAG);
		} else if(SrcType == XPLMI_READBK_INTF_TYPE_SMAP) {
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_CTRL,
				SLAVE_BOOT_SBI_CTRL_INTERFACE_MASK,
				XPLMI_SBI_CTRL_INTERFACE_SMAP);
		} else {
			/* Do Nothing */
		}

		XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE,
			SLAVE_BOOT_SBI_MODE_SELECT_MASK,
			SLAVE_BOOT_SBI_MODE_SELECT_MASK);
		Status = XPlmi_DmaSbiXfer(SrcAddr, ReadLen, XPLMI_PMCDMA_1
				| XPLMI_DMA_SRC_NONBLK);
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/** Set MaxOutCommands of PMC_DMA1 to 8 */
	XPlmi_SetMaxOutCmds(XPLMI_MAXOUT_CMD_DEF_VAL);

	Status = XPlmi_DmaXfr((u64)CfiPayloadSrcAddr, DestAddrRead,
		Cmd->PayloadLen - XPLMI_CFI_DATA_OFFSET, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (SrcType == XPLMI_READBK_INTF_TYPE_DDR) {
		Status = XPlmi_WaitForNonBlkDma(XPLMI_PMCDMA_1);
	} else {
		Status = XPlmi_WaitForNonBlkSrcDma(XPLMI_PMCDMA_1);
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Len -= ReadLen;
	while (Len > 0U) {
		ReadLen = XPlmi_GetReadbackLen(Len);

		if (SrcType == XPLMI_READBK_INTF_TYPE_DDR) {
			DestAddr += ((u64)ReadLen * XPLMI_WORD_LEN);
			Status = XPlmi_DmaXfr(SrcAddr, DestAddr, ReadLen, XPLMI_PMCDMA_1);
		} else {
			Status = XPlmi_DmaSbiXfer(SrcAddr, ReadLen, XPLMI_PMCDMA_1);
		}
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Len -= ReadLen;
	}

	if (SrcType == XPLMI_READBK_INTF_TYPE_DDR) {
		if (XPLMI_READBACK_DEF_DST_ADDR != ReadBackPtr->DestAddr) {
			ReadBackPtr->ProcessedLen += Cmd->Payload[3U];
			ReadBackPtr->DestAddr += ((u64)Cmd->Payload[3U] * XPLMI_WORD_LEN);
		}
		goto END;
	}

	Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
		SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_MASK,
		SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_VAL,
		XPLMI_TIME_OUT_DEFAULT, NULL);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (SrcType == XPLMI_READBK_INTF_TYPE_SMAP) {
		Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
			SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_MASK,
			SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_VAL,
			XPLMI_TIME_OUT_DEFAULT, NULL);
	} else {
		Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
			SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_MASK,
			SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_VAL,
			XPLMI_TIME_OUT_DEFAULT, NULL);
	}

END:
	XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE, SLAVE_BOOT_SBI_MODE_SELECT_MASK, 0U);
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
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_Set(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 DestAddrHigh = Cmd->Payload[0U];
	u32 DestAddrLow = Cmd->Payload[1U];
	u64 DestAddr = (DestAddrHigh << 32U) | DestAddrLow;
	u32 Len = Cmd->Payload[2U];
	u32 Val = Cmd->Payload[3U];
	XPLMI_EXPORT_CMD(XPLMI_SET_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_FOUR, XPLMI_CMD_ARG_CNT_FOUR);

	Status = XPlmi_MemSet(DestAddr, Val, Len);

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
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_DmaWriteKeyHole(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 DestAddr;
	u64 SrcAddr;
	u32 Len = Cmd->PayloadLen;
	u32 Keyholesize;
	u64 BaseAddr;
	u32 DestOffset;
	XPlmi_KeyHoleXfrParams KeyHoleXfrParams;
#ifdef PLM_PRINT_PERF_KEYHOLE
	u64 KeyHoleTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime = {0U};
#endif
	XPLMI_EXPORT_CMD(XPLMI_WRITE_KEYHOLE_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_THREE, XPLMI_UNLIMITED_ARG_CNT);

	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	if (Cmd->ProcessedLen == 0U) {
		/* Store the destination address in resume data */
		Cmd->ResumeData[0U] = Cmd->Payload[0U];
		Cmd->ResumeData[1U] = Cmd->Payload[1U];
		Cmd->ResumeData[2U] = Cmd->Payload[2U];
		Keyholesize = Cmd->Payload[2U] * XPLMI_WORD_LEN;
		SrcAddr = (u64)(UINTPTR) &Cmd->Payload[3U];
		Len -= 3U;
		Cmd->ResumeData[3U] = 0U;
		DestOffset = 0U;
	} else {
		SrcAddr = (u64)(UINTPTR) &Cmd->Payload[0U];
		Keyholesize = Cmd->ResumeData[2U] * XPLMI_WORD_LEN;
		DestOffset = 3U;
	}

	DestAddr = (u64)Cmd->ResumeData[0U];
	DestAddr = (u64)Cmd->ResumeData[1U] | (DestAddr << 32U);
	BaseAddr = DestAddr;

	if (Cmd->KeyHoleParams.Func != NULL) {
		/* This is for direct DMA to CFI from PdiSrc bypassing PMCRAM */
		Status = XPlmi_CfiWrite(SrcAddr, DestAddr, Keyholesize, Len, Cmd);
		goto END;
	}

	DestAddr = ((((u64)Cmd->ProcessedLen - Cmd->ResumeData[3U] - DestOffset) *
			XPLMI_WORD_LEN) & (Keyholesize - 1U)) + BaseAddr;
	KeyHoleXfrParams.SrcAddr = SrcAddr;
	KeyHoleXfrParams.DestAddr = DestAddr;
	KeyHoleXfrParams.BaseAddr = BaseAddr;
	KeyHoleXfrParams.Len = Len * XPLMI_WORD_LEN;
	KeyHoleXfrParams.Keyholesize = Keyholesize;
	KeyHoleXfrParams.Flags = XPLMI_PMCDMA_0;
	KeyHoleXfrParams.Func = NULL;
	Status = XPlmi_KeyHoleXfr(&KeyHoleXfrParams);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
	}

END:
#ifdef PLM_PRINT_PERF_KEYHOLE
	XPlmi_MeasurePerfTime(KeyHoleTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF, " %u.%06u ms KeyHole Run Time \r\n",
	(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
#endif
	return Status;
}

/**
 * @{
 * @cond xplmi_internal
 */
/*****************************************************************************/
/**
 * @brief	This function stores the board name in a local variable based
 * on the parameters passed.
 *
 * @param	Cmd is pointer to the command structure
 * @param	GetFlag is set to TRUE for read and FALSE for write
 * @param	Len is pointer to the number of bytes to be copied
 *
 * @return	BoardName
 *
 *****************************************************************************/
static u8* XPlmi_BoardNameRW(const XPlmi_Cmd *Cmd, u8 GetFlag, u32 *Len)
{
	int Status = XST_FAILURE;
	XPlmi_BoardParams *BoardParams = XPlmi_GetBoardParams();

	if (*Len > XPLMI_MAX_NAME_WORDS) {
		*Len = XPLMI_MAX_NAME_WORDS;
	}

	if (GetFlag == (u8)FALSE) {
		/* Set Command */
		Status = XPlmi_DmaXfr((u64)(u32)&Cmd->Payload[0U],
				(u64)(u32)BoardParams->Name, *Len, XPLMI_PMCDMA_0);
		if (Status != XST_SUCCESS) {
			(void)XPlmi_MemSetBytes((void *)BoardParams,
				sizeof(XPlmi_BoardParams), 0U,
				sizeof(XPlmi_BoardParams));
			goto END;
		}

		BoardParams->Name[*Len * XPLMI_WORD_LEN] = 0U;
		BoardParams->Len = *Len;
	}
	else {
		*Len = BoardParams->Len;
	}

END:
	return BoardParams->Name;
}
/**
 * @}
 * @endcond
 */

/*****************************************************************************/
/**
 * @brief	This function provides SET BOARD command execution.
 *  		Command payload parameters are
 *		- Board Name
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_SetBoard(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Len = Cmd->PayloadLen;
	XPLMI_EXPORT_CMD(XPLMI_SET_BOARD_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ONE, XPLMI_UNLIMITED_ARG_CNT);

	(void)XPlmi_BoardNameRW(Cmd, (u8)FALSE, &Len);

	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides GET BOARD command execution.
 *
 * @param	Cmd is pointer to the command structure
 *		Command payload parameters are
 *		- High Addr
 *		- Low Addr
 *		- Max size in words
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_GetBoard(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 HighAddr = Cmd->Payload[0U];
	u32 LowAddr = Cmd->Payload[1U];
	u64 DestAddr = (HighAddr << XPLMI_NUM_BITS_IN_WORD) | LowAddr;
	u32 Len = Cmd->Payload[2U];
	u8* BoardName = XPlmi_BoardNameRW(Cmd, (u8)TRUE, &Len);
	XPLMI_EXPORT_CMD(XPLMI_GET_BOARD_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_THREE, XPLMI_CMD_ARG_CNT_THREE);

	Status = XPlmi_DmaXfr((u64)(u32)&BoardName[0U], DestAddr, Len,
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Cmd->Response[1U] = Len;
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets the WDT parameters used in PLM.
 *
 * @param	Cmd is pointer to the command structure
 *		Command payload parameters are
 *		- Node Id for PMC, PS MIO
 *		- Periodicity
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_SetWdtParam(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 NodeId = Cmd->Payload[0U];
	u32 Periodicity = Cmd->Payload[1U];
	XPLMI_EXPORT_CMD(XPLMI_SET_WDT_PARAM_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);

	XPlmi_Printf(DEBUG_INFO, "Enabling WDT with Node:0x%08x, "
		     "Periodicity: %u ms\n\r", NodeId, Periodicity);
	Status = XPlmi_EnableWdt(NodeId, Periodicity);

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
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_LOG_STRING if the string provided exceeds max length.
 *
 *****************************************************************************/
static int XPlmi_LogString(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Len = Cmd->PayloadLen * XPLMI_WORD_LEN;
	static u8 LogString[XPLMI_MAX_LOG_STR_LEN] __attribute__ ((aligned(4U)));
	static u32 StringIndex = 0U;
	XPLMI_EXPORT_CMD(XPLMI_LOG_STR_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ONE, XPLMI_UNLIMITED_ARG_CNT);

	if (Cmd->ProcessedLen == 0U) {
		/* Check for array overflow */
		if ((Cmd->Len * XPLMI_WORD_LEN) >= XPLMI_MAX_LOG_STR_LEN) {
			Status = (int)XPLMI_ERR_LOG_STRING;
			goto END;
		}
		Status = XPlmi_MemSet((u64)(u32)&LogString[0U], 0U,
			XPLMI_MAX_LOG_STR_LEN /  XPLMI_WORD_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		StringIndex = 0U;
	}

	/* Append the payload to local buffer */
	Status = Xil_SMemCpy(&LogString[StringIndex],
		(XPLMI_MAX_LOG_STR_LEN - StringIndex), (u8 *)&Cmd->Payload[0U],
		Len, Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	StringIndex += Len;
	if ((Cmd->ProcessedLen + Cmd->PayloadLen) == Cmd->Len) {
		/* Print the string only when complete payload is received */
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "%s\n\r", LogString);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads the value at an address and displays and adds
 * 			the value to PLM logs.
 *
 * @param	Cmd is pointer to the command structure
 *		Command payload parameters are
 *		- Low Address
 *		- High Address (Optional)
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_LogAddress(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 LowAddr = Cmd->Payload[XPLMI_LOG_ADDR_ARG_LOW_ADDR_INDEX];
	u64 HighAddr = 0UL;
	u64 Addr;
	u32 Val;
	XPLMI_EXPORT_CMD(XPLMI_LOG_ADDR_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_TWO);

	if (Cmd->Len == XPLMI_LOG_ADDR_MAX_ARGS) {
		/* This indicates non-zero 32-bit higher address */
		HighAddr = Cmd->Payload[XPLMI_LOG_ADDR_ARG_HIGH_ADDR_INDEX];
	}
	Addr = (HighAddr << XPLMI_NUM_BITS_IN_WORD) | (u64)LowAddr;
	Val = XPlmi_In64(Addr);

	XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Value at 0x%0x%08x: %0x\n\r",
			(u32)(Addr >> XPLMI_NUM_BITS_IN_WORD), (u32)Addr, Val);
	Status = XST_SUCCESS;

	return Status;
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
 * 			- XST_SUCCESS always.
 *
 *****************************************************************************/
static int XPlmi_Marker(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	XPLMI_EXPORT_CMD(XPLMI_MARKER_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ONE, XPLMI_UNLIMITED_ARG_CNT);

	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);
	/* Return success */
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides functionality to move procs when proc
 *          command is received for existing BufferId.
 *
 * @param	BufferIndex is the index of BufferId to be moved
 * @param	BufferList is the list of buffers whose buffers
 *              need to be moved
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_MoveBuffer(u8 BufferIndex, XPlmi_BufferList *BufferList)
{
	int Status = XST_FAILURE;
	u64 DestAddr;
	u64 SrcAddr;
	u32 Len;
	u8 Index = BufferIndex;
	u32 DeletedBufferLen;

	/**
	 * - If only one proc is available and if a new proc command is received with
	 * same ID, it can directly be overwritten.
	 */
	if ((BufferList->BufferCount == 1U) ||
		(Index == (BufferList->BufferCount - 1U))) {
		Status = XST_SUCCESS;
		goto END;
	}

	/**
	 * - If proc command is received for existing BufferId,
	 * move all procs behind this to front
	 */
	DestAddr = BufferList->Data[Index].Addr;
	SrcAddr = BufferList->Data[Index + 1U].Addr;
	Len = (u32)((BufferList->Data[BufferList->BufferCount].Addr -
			SrcAddr)/XPLMI_WORD_LEN);
	/* Length of the proc that is removed */
	DeletedBufferLen = (u32)(BufferList->Data[Index + 1U].Addr -
			BufferList->Data[Index].Addr);

	/** - Call XPlmi_DmaTransfer with flags DMA0 and INCR */
	Status = XPlmi_DmaTransfer(DestAddr, SrcAddr, Len, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Update BufferList with moved data */
	while(Index <= BufferList->BufferCount) {
		BufferList->Data[Index].Id = BufferList->Data[Index + 1U].Id;
		BufferList->Data[Index].Addr =
					BufferList->Data[Index + 1U].Addr - DeletedBufferLen;
		Index++;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function defines BufferList and returns the address of the same
 *
 * @param	BufferListType is the proc list type if it is stored in PMC or PSM RAM
 *
 * @return	BufferList is the address of BufferList structure
 *
 *****************************************************************************/
XPlmi_BufferList* XPlmi_GetBufferList(u32 BufferListType)
{
	/**
	 * - Create static BufferList structure and initialize with zero during
	 * initial call.
	 */
	static XPlmi_BufferList PsmBufferList = {0U};
	static XPlmi_BufferData PsmBuffers[XPLMI_MAX_PSM_BUFFERS + 1U] = {0U};
	static XPlmi_BufferList PmcBufferList = {0U};
	static XPlmi_BufferData PmcBuffers[XPLMI_MAX_PMC_BUFFERS + 1U] = {0U};
	XPlmi_BufferList *BufferList = &PsmBufferList;

	PsmBufferList.Data = PsmBuffers;
	PsmBufferList.MaxBufferCount = XPLMI_MAX_PSM_BUFFERS;
	PmcBufferList.Data = PmcBuffers;
	PmcBufferList.MaxBufferCount = XPLMI_MAX_PMC_BUFFERS;

	if (BufferListType == XPLMI_PMC_BUFFER_LIST) {
		BufferList = &PmcBufferList;

		/**
		 * - Initialize first Data address of the PmcBufferList to the PMC RAM
		 * reserved address and BufferMemSize with the Max Size allocated
		 */
		PmcBufferList.Data[0U].Addr = XPLMI_PMCRAM_BUFFER_MEMORY;
		PmcBufferList.BufferMemSize = XPLMI_PMCRAM_BUFFER_MEMORY_LENGTH;
		PmcBufferList.IsBufferMemAvailable = (u8)TRUE;
	}

	return BufferList;
}

/*****************************************************************************/
/**
 * @brief	This function sets PSM BufferList address to given Address and Size
 *
 * @param	Address is the address of Buffer reserved memory for PSM BufferList
 * @param	Size is the size of Buffer reserved memory for PSM BufferList in bytes
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 * 			- XPLMI_ERR_PROC_INVALID_ADDRESS_RANGE if provided proc address is
 * 			invalid.
 *
 *****************************************************************************/
int XPlmi_SetBufferList(u32 Address, u16 Size)
{
	int Status = XST_FAILURE;
	XPlmi_BufferList *BufferList = XPlmi_GetBufferList(XPLMI_PSM_BUFFER_LIST);
	u32 StartAddr;
	u32 EndAddr;

	/**
	 * - Validate the allocated memory address range.
	 *   Otherwise return an error.
	 */
	StartAddr = Address;
	EndAddr = (Address + (u32)Size - 1U);
	if ((StartAddr >= (u32)XPLMI_PSM_RAM_BASE_ADDR) &&
		(EndAddr <= (u32)XPLMI_PSM_RAM_HIGH_ADDR)) {
		/* PSM RAM is valid */
		Status = XST_SUCCESS;
	}
	else {
		Status = (int)XPLMI_ERR_PROC_INVALID_ADDRESS_RANGE;
		goto END;
	}

	/**
	 * - Initialize first Data address of PSM BufferList to the given Address
	 * and BufferMemSize with the given Size
	 */
	BufferList->Data[0U].Addr = Address;
	BufferList->BufferMemSize = Size;
	BufferList->IsBufferMemAvailable = (u8)TRUE;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function searches Buffer list with the buffer Id to
 *		provide buffer address and buffer length
 *
 * @param	BufferList is the pointer to the list of buffers to search
 * @param	BufferId is the id of the buffer to search for
 * @param	BufAddr is the address of the Buffer found
 * @param	BufLen is the length of the Buffer found
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 * 			- XPLMI_PROCID_NOT_VALID if provided proc id is invalid.
 *
 *****************************************************************************/
int XPlmi_SearchBufferList(XPlmi_BufferList *BufferList, u32 BufferId,
		u64 *BufAddr, u32 *BufLen)
{
	int Status = XST_FAILURE;
	u8 BufferIndex = 0U;

	/** Check if Buffer Memory is available */
	if (BufferList->IsBufferMemAvailable != (u8)TRUE) {
		Status = XPLMI_ERR_BUFFER_MEM_NOT_AVAILABLE;
		goto END;
	}
	/** Search for the buffer index which has a matching BufferId */
	BufferList->Data[BufferList->BufferCount].Id = BufferId;
	while (BufferList->Data[BufferIndex].Id != BufferId) {
		BufferIndex++;
	}

	/** - Execute proc if the received BufferId is valid. */
	if (BufferIndex >= BufferList->BufferCount) {
		Status = (int)XPLMI_PROCID_NOT_VALID;
		goto END;
	}
	/** Fill BufAddr and BufLen pointers with resulting values */
	*BufAddr = BufferList->Data[BufferIndex].Addr;
	*BufLen = (u32)(BufferList->Data[BufferIndex + 1U].Addr -
		BufferList->Data[BufferIndex].Addr) / XPLMI_WORD_LEN;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides functionality to execute given ProcId when
 *          request to execute a particular ProcId.
 *
 * @param	ProcId is ProcId to be executed
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_PROC_LPD_NOT_INITIALIZED if LPD failed to initialize.
 * 			- XPLMI_PROCID_NOT_VALID on invalid Proc ID.
 *
 *****************************************************************************/
int XPlmi_ExecuteProc(u32 ProcId)
{
	int Status = XST_FAILURE;
	XPlmiCdo ProcCdo;
	XPlmi_BufferList *BufferList = NULL;
	u32 BufferListType = XPLMI_PSM_BUFFER_LIST;
	u64 BufAddr;
	XPlmi_Printf(DEBUG_GENERAL, "Proc ID received: 0x%x\r\n", ProcId);

	/** - If the ProcId has MSB set, then its in PMC RAM memory. */
	if ((ProcId & XPLMI_PMC_RAM_PROC_ID_MASK) == XPLMI_PMC_RAM_PROC_ID_MASK) {
		BufferListType = XPLMI_PMC_BUFFER_LIST;
	}

	BufferList = XPlmi_GetBufferList(BufferListType);

	if (BufferListType == XPLMI_PSM_BUFFER_LIST) {
		/**
		 * - If LPD is not initialized or Proc memory is not available,
		 * do not execute the proc and return an error.
		 */
		if (XPlmi_IsLpdInitialized() != (u8)TRUE) {
			Status = (int)XPLMI_ERR_PROC_LPD_NOT_INITIALIZED;
			goto END;
		}
	}

	Status = XPlmi_MemSetBytes((void *const)&ProcCdo,
		sizeof(ProcCdo), 0U, sizeof(ProcCdo));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Search the buffer list for the proc */
	Status = XPlmi_SearchBufferList(BufferList, ProcId, &BufAddr, &ProcCdo.BufLen);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Fill ProcCdo structure with Proc related parameters. */
	ProcCdo.BufPtr = (u32 *)(UINTPTR)BufAddr;
	ProcCdo.CdoLen = ProcCdo.BufLen;
	ProcCdo.SubsystemId = XPLMI_PMC_SUBSYS_NODE_ID;
	/** - Execute Proc. */
	Status = XPlmi_ProcessCdo(&ProcCdo);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function copies the proc data to reserved PSM/PMC RAM when proc
 *          command is received.
 *
 * @param	Cmd is pointer to the command structure
 *              Command payload parameters are
 *              - ProcId
 *              - Data
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_UNSUPPORTED_PROC_LENGTH if received proc does not fit in
 * 			proc memory.
 *
 *****************************************************************************/
static int XPlmi_Proc(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 ProcId = 0U;
	XPlmi_BufferList *BufferList = NULL;
	u32 BufferListType = XPLMI_PSM_BUFFER_LIST;

	XPLMI_EXPORT_CMD(XPLMI_PROC_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ONE, XPLMI_UNLIMITED_ARG_CNT);

	if (Cmd->ProcessedLen == 0U) {
		ProcId = Cmd->Payload[0U];
		/* Check if the Procs are stored in PSM RAM or PMC RAM */
		if ((ProcId & XPLMI_PMC_RAM_PROC_ID_MASK) == XPLMI_PMC_RAM_PROC_ID_MASK) {
			BufferListType = XPLMI_PMC_BUFFER_LIST;
		}

		/* Get the buffer list */
		BufferList = XPlmi_GetBufferList(BufferListType);

		if (BufferListType == XPLMI_PSM_BUFFER_LIST) {
			/*
			 * Check if LPD is initialized and BufferList is initialized
			 * as we are using PSM RAM to store Buffer data
			 */
			if ((XPlmi_IsLpdInitialized() != (u8)TRUE) ||
					(BufferList->IsBufferMemAvailable != (u8)TRUE)) {
				Status = (int)XPLMI_ERR_PROC_LPD_NOT_INITIALIZED;
				goto END;
			}
		}
	}

	/** Store Proc in the respective Buffer List */
	Status = XPlmi_StoreBuffer(Cmd, ProcId, BufferList);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function stores the buffer data into the buffer list provided
 *
 * @param	Cmd is pointer to the command structure
 * @param	BufferId is the Id of the Buffer to be stored in the buffer list
 * @param	BufferList is the pointer to the BufferList
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_UNSUPPORTED_PROC_LENGTH if received proc does not fit in
 * 			proc memory.
 * 			- XPLMI_MAX_PROC_COMMANDS_RECEIVED on maximum supported proc commands
 * 			received.
 *
 *****************************************************************************/
int XPlmi_StoreBuffer(XPlmi_Cmd *Cmd, u32 BufferId, XPlmi_BufferList *BufferList)
{
	int Status = XST_FAILURE;
	u8 Index = 0U;
	u32 SrcAddr;
	u32 LenDiff = 0U;
	u32 CmdLenInBytes = (Cmd->Len - 1U) * XPLMI_WORD_LEN;
	u32 CurrPayloadLen;

	if (Cmd->ProcessedLen == 0U) {

		/* Check if Buffer memory is available */
		if (BufferList->IsBufferMemAvailable != (u8)TRUE) {
			Status = (int)XPLMI_ERR_BUFFER_MEM_NOT_AVAILABLE;
			goto END;
		}

		/* Check if received BufferId is already in memory */
		BufferList->Data[BufferList->BufferCount].Id = BufferId;
		while (BufferList->Data[Index].Id != BufferId) {
			Index++;
		}
		if (Index < BufferList->BufferCount) {
			/*
			 * Get Length difference if received proc length is greater than
			 * length of the proc in memory
			 * This is to check if new proc fits in available memory
			 */
			if (CmdLenInBytes > (BufferList->Data[Index + 1U].Addr -
					BufferList->Data[Index].Addr)) {
				LenDiff = (u32)(CmdLenInBytes - (BufferList->Data[Index + 1U].Addr -
						BufferList->Data[Index].Addr));
			}
			/* Check if new proc length fits in the proc allocated memory */
			if ((LenDiff + (BufferList->Data[BufferList->BufferCount].Addr -
					BufferList->Data[BufferList->BufferCount - 1U].Addr)) >
					BufferList->BufferMemSize) {
				Status = (int)XPLMI_UNSUPPORTED_PROC_LENGTH;
				goto END;
			}

			/*
			 * If proc command is received for existing proc,
			 * move other procs
			 */
			Status = XPlmi_MoveBuffer(Index, BufferList);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			--BufferList->BufferCount;
		} else {
			/* Check if proc list is full */
			if (BufferList->BufferCount == BufferList->MaxBufferCount) {
				Status = (int)XPLMI_MAX_PROC_COMMANDS_RECEIVED;
				goto END;
			}
		}

		/* New proc address where the proc data need to be copied */
		Cmd->ResumeData[0U] = (u32)BufferList->Data[BufferList->BufferCount].Addr;

		/* Check if new proc length fits in the proc allocated memory */
		if ((Cmd->ResumeData[0U] + CmdLenInBytes) > ((u32)BufferList->Data[0U].Addr +
				BufferList->BufferMemSize)) {
			Status = (int)XPLMI_UNSUPPORTED_PROC_LENGTH;
			goto END;
		}
		SrcAddr = (u32)(&Cmd->Payload[1U]);
		CurrPayloadLen = Cmd->PayloadLen - 1U;

		/* Add an entry in BufferList */
		BufferList->Data[BufferList->BufferCount].Id = BufferId;
		BufferList->BufferCount++;
		BufferList->Data[BufferList->BufferCount].Addr =
				BufferList->Data[BufferList->BufferCount - 1U].Addr + CmdLenInBytes;
	} else {
		/* Handle command resume for proc data */
		SrcAddr = (u32)(&Cmd->Payload[0U]);
		CurrPayloadLen = Cmd->PayloadLen;
	}

	/* Copy the received proc to proc memory */
	Status = XPlmi_DmaTransfer(Cmd->ResumeData[0U], SrcAddr, CurrPayloadLen,
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	/* Update destination address to handle resume case */
	Cmd->ResumeData[0U] += CurrPayloadLen * XPLMI_WORD_LEN;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides Over Temperature Check command execution.
 *
 * @param	Cmd is pointer to the command structure
 *              Command payload parameters are
 *              - Delay in milliseconds
 *
 * @return
 * 			- XST_SUCCESS on success and XST_FAILURE on failure
 *
 *****************************************************************************/
static int XPlmi_OTCheck(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 WaitInMSec = Cmd->Payload[0U];
	XPLMI_EXPORT_CMD(XPLMI_OT_CHECK_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ONE, XPLMI_CMD_ARG_CNT_ONE);

	XPlmi_Printf(DEBUG_INFO, "%s: Delay post OT Event (mSec): %u\n\r",
			__func__, WaitInMSec);

	/*
	 * Check if OT event is present, wait until it clears up,
	 * then wait additional *WaitInMSec* milliseconds
	 */
	XPlmi_SysMonOTDetect(WaitInMSec);

	Status = XST_SUCCESS;

	return Status;
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
static int XPlmi_Begin(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 EndOffSet = Cmd->ProcessedCdoLen + Cmd->Payload[0U] + XPLMI_BEGIN_CMD_EXTRA_OFFSET;
	u8  LogString[XPLMI_BEGIN_MAX_LOG_STR_LEN] __attribute__ ((aligned(4U)));
	u32 StrLen = 0U;

	XPLMI_EXPORT_CMD(XPLMI_BEGIN_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ONE, XPLMI_UNLIMITED_ARG_CNT);

	if (Cmd->ProcessedLen != 0U) {
		Status = XST_SUCCESS;
		goto END;
	}
	/* Max 10 nested begin supported */
	if (Cmd->CdoParamsStack.OffsetListTop == (int)(XPLMI_BEGIN_OFFSET_STACK_SIZE - 1U)) {
		XPlmi_Printf(DEBUG_GENERAL,"Max %d nested begin supported\n",
				XPLMI_BEGIN_OFFSET_STACK_SIZE);
		goto END;
	}

	/* Push "end" Offset to stack */
	Status = XPlmi_StackPush(&Cmd->CdoParamsStack, &EndOffSet);
	if ( Status != XST_SUCCESS) {
		goto END;
	}

	if (Cmd->PayloadLen > 1U) {
		/* Truncate string if its length exceeds 6 words (24 characters) */
		if (Cmd->PayloadLen >= XPLMI_CMD_RESP_SIZE) {
			StrLen = XPLMI_BEGIN_MAX_STRING_WORD_LEN;
		}
		else {
			StrLen = Cmd->PayloadLen - 1U;
		}

		Status = XPlmi_MemSet((u64)(UINTPTR)LogString, 0U,
				XPLMI_BEGIN_MAX_STRING_WORD_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* Add the payload to local buffer */
		Status = Xil_SMemCpy(LogString, XPLMI_BEGIN_MAX_LOG_STR_LEN,
				(u8 *)&Cmd->Payload[1U], (StrLen * XPLMI_WORD_LEN),
				(StrLen * XPLMI_WORD_LEN));
		if (Status != XST_SUCCESS) {
			goto END;
		}

		/* add null character at the end of log string buffer before reading it.*/
		LogString[XPLMI_BEGIN_LOG_STR_BUF_END_INDEX] = '\0';
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "%s\n\r", LogString);
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
static int XPlmi_End(XPlmi_Cmd *Cmd)
{
	u32 EndLength = 0U;
	int Status = XST_FAILURE;

	XPLMI_EXPORT_CMD(XPLMI_END_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ZERO, XPLMI_CMD_ARG_CNT_ZERO);

	/* Stack empty, End does not have begin */
	if (Cmd->CdoParamsStack.OffsetListTop < 0) {
		XPlmi_Printf(DEBUG_GENERAL,"End does not have valid begin\n");
		goto END;
	}

	/* Popped "end" length from stack should match with current ProcessedCdoLen */
	Status = XPlmi_StackPop(&Cmd->CdoParamsStack, XPLMI_BEGIN_OFFEST_STACK_DEFAULT_POPLEVEL,
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
static int XPlmi_Break(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Level = 0U;

	XPLMI_EXPORT_CMD(XPLMI_BREAK_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ZERO, XPLMI_CMD_ARG_CNT_ONE);
	/*
	 * Get Level to skip all commands until Levels number of nested “end”
	 * commands have been seen
	 */
	if (0U == Cmd->PayloadLen) {
		Level = XPLMI_BEGIN_OFFEST_STACK_DEFAULT_POPLEVEL;
	} else {
		Level = Cmd->Payload[0U] & XPLMI_BREAK_LEVEL_MASK;
	}

	/* Jump to "end" associated with break level */
	Status = XPlmi_GetJumpOffSet(Cmd, Level);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets ipi access permissions for the range of commands
 *		requested.
 *
 * @param	Cmd is pointer to the command structure
 *              Command payload parameters are
 *              - Api Id Upper Limit, Api Id Lower Limit, Module Id
 *              - Access Permission Mask
 *
 * @return
 * 		- XST_SUCCESS on success.
		- XPLMI_ERR_SET_IPI_MODULE_MAX if the module id is greater than
		max module count
		- XPLMI_ERR_SET_IPI_INVALID_API_ID_UPPER if the Api Id Upper limit
		is greater than maximum api count
		- XPLMI_ERR_SET_IPI_INVALID_API_ID_LOWER if the Api Id Lower limit
		is greater than Api Id upper limit
		- XPLMI_ERR_SET_IPI_ACCESS_PERM_NOT_SET if the Ipi Access permission
		is not set properly
		- XPLMI_ERR_SET_IPI_PERM_BUFF_NOT_REGISTERED if the permission buffer
		in which the access permissions has to be stored is not registered
 *
 *****************************************************************************/
static int XPlmi_SetIpiAccess(XPlmi_Cmd *Cmd)
{

	volatile u32 ApiIndex;
	int Status = XST_FAILURE;
	const XPlmi_Module *Module = NULL;
	u32 ModuleId = Cmd->Payload[XPLMI_SET_IPI_ACCESS_API_ID_VAL_INDEX] &
			XPLMI_SET_IPI_ACCESS_MODULE_ID_MASK;
	u32 ApiIdLower = (Cmd->Payload[XPLMI_SET_IPI_ACCESS_API_ID_VAL_INDEX] &
			XPLMI_SET_IPI_ACCESS_API_ID_LOWER_MASK) >> XPLMI_SET_IPI_ACCESS_API_ID_LOWER_SHIFT;
	u32 ApiIdUpper = (Cmd->Payload[XPLMI_SET_IPI_ACCESS_API_ID_VAL_INDEX] &
			XPLMI_SET_IPI_ACCESS_API_ID_UPPER_MASK) >> XPLMI_SET_IPI_ACCESS_API_ID_UPPER_SHIFT;
	XPlmi_AccessPerm_t AccessPermMask = (XPlmi_AccessPerm_t)(Cmd->Payload[XPLMI_SET_IPI_ACCESS_PERM_VAL_INDEX] &
			XPLMI_GET_ALL_IPI_MASK(XPLMI_FULL_IPI_ACCESS));

	XPLMI_EXPORT_CMD(XPLMI_SET_IPI_ACCESS_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);
	/**
	 * Validate module number
	 */
	if ((ModuleId >= XPLMI_MAX_MODULES)) {
		Status = XPLMI_ERR_SET_IPI_MODULE_MAX;
		goto END;
	}

	/** Check if the module is registered */
	Module = Modules[ModuleId];
	if (Module == NULL) {
		/* Ignore if module is not registered */
		Status = XST_SUCCESS;
		goto END;
	}

	/* Set permissions only if Access Permission buffer is registered */
	if (Module->AccessPermBufferPtr == NULL) {
		Status= XPLMI_ERR_SET_IPI_PERM_BUFF_NOT_REGISTERED;
		goto END;
	}

	/* Check if Api Id upper limit is valid */
	if (ApiIdUpper >= Module->CmdCnt) {
		Status = XPLMI_ERR_SET_IPI_INVALID_API_ID_UPPER;
		goto END;
	}

	/* Check if Api Id lower limit is valid */
	if (ApiIdLower > ApiIdUpper) {
		Status = XPLMI_ERR_SET_IPI_INVALID_API_ID_LOWER;
		goto END;
	}

	/* Set access permissions for api id range specified in the command */
	for (ApiIndex = ApiIdLower; ApiIndex <= ApiIdUpper; ApiIndex++) {
		/* Write and readback the requested access permission */
		Module->AccessPermBufferPtr[ApiIndex] = AccessPermMask;
		if (Module->AccessPermBufferPtr[ApiIndex] != AccessPermMask) {
			Status = XPLMI_ERR_SET_IPI_ACCESS_PERM_NOT_SET;
			break;
		}
	}

	if (ApiIndex <= ApiIdUpper) {
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function triggers secure lockdown when Tamper Trigger IPI is
 *          received.
 *
 * @param	Cmd is pointer to the command structure
 *              Command payload parameters are
 *              - Tamper Response
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_INVALID_TAMPER_RESPONSE on invalid tamper response received
 * 			for TamperTrigger IPI call.
 *
 *****************************************************************************/
static int XPlmi_TamperTrigger(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	volatile u32 TamperResp = Cmd->Payload[0U];
	volatile u32 TamperRespTmp = Cmd->Payload[0U];

	/* Check if the Tamper Response input is valid */
	if (((TamperResp & XPLMI_SLD_AND_SRST_TAMPER_RESP_MASK) == 0x0U) &&
		((TamperRespTmp & XPLMI_SLD_AND_SRST_TAMPER_RESP_MASK) == 0x0U)) {
		/* Return error code if the Tamper Response received is invalid */
		Status = (int)XPLMI_INVALID_TAMPER_RESPONSE;
		goto END;
	}

	/* Execute secure lockdown */
	XPlmi_TriggerTamperResponse(TamperResp, XPLMI_TRIGGER_TAMPER_TASK);

	if (Cmd->IpiMask != 0) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/**
 * @{
 * @cond xplmi_internal
 */

/*****************************************************************************/
/**
 * @brief	This function registers the PLM generic commands to the PLMI.
 *
 * @param	None
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_GenericInit(void)
{
	/* Contains the array of PLM generic commands */
	static const XPlmi_ModuleCmd XPlmi_GenericCmds[] =
	{
		XPLMI_MODULE_COMMAND(XPlmi_Features),
		XPLMI_MODULE_COMMAND(XPlmi_MaskPoll),
		XPLMI_MODULE_COMMAND(XPlmi_MaskWrite),
		XPLMI_MODULE_COMMAND(XPlmi_Write),
		XPLMI_MODULE_COMMAND(XPlmi_Delay),
		XPLMI_MODULE_COMMAND(XPlmi_DmaWrite),
		XPLMI_MODULE_COMMAND(XPlmi_MaskPoll64),
		XPLMI_MODULE_COMMAND(XPlmi_MaskWrite64),
		XPLMI_MODULE_COMMAND(XPlmi_Write64),
		XPLMI_MODULE_COMMAND(XPlmi_DmaXfer),
		XPLMI_MODULE_COMMAND(XPlmi_InitSeq),
		XPLMI_MODULE_COMMAND(XPlmi_CfiRead),
		XPLMI_MODULE_COMMAND(XPlmi_Set),
		XPLMI_MODULE_COMMAND(XPlmi_DmaWriteKeyHole),
		XPLMI_MODULE_COMMAND(XPlmi_SsitSyncMaster),
		XPLMI_MODULE_COMMAND(XPlmi_SsitSyncSlaves),
		XPLMI_MODULE_COMMAND(XPlmi_SsitWaitSlaves),
		XPLMI_MODULE_COMMAND(XPlmi_Nop),
		XPLMI_MODULE_COMMAND(XPlmi_GetDeviceID),
		XPLMI_MODULE_COMMAND(XPlmi_EventLogging),
		XPLMI_MODULE_COMMAND(XPlmi_SetBoard),
		XPLMI_MODULE_COMMAND(XPlmi_GetBoard),
		XPLMI_MODULE_COMMAND(XPlmi_SetWdtParam),
		XPLMI_MODULE_COMMAND(XPlmi_LogString),
		XPLMI_MODULE_COMMAND(XPlmi_LogAddress),
		XPLMI_MODULE_COMMAND(XPlmi_Marker),
		XPLMI_MODULE_COMMAND(XPlmi_Proc),
		XPLMI_MODULE_COMMAND(XPlmi_Begin),
		XPLMI_MODULE_COMMAND(XPlmi_End),
		XPLMI_MODULE_COMMAND(XPlmi_Break),
		XPLMI_MODULE_COMMAND(XPlmi_OTCheck),
		XPLMI_MODULE_COMMAND(XPlmi_PsmSequence),
		XPLMI_MODULE_COMMAND(XPlmi_InPlacePlmUpdate),
		XPLMI_MODULE_COMMAND(XPlmi_ScatterWrite),
		XPLMI_MODULE_COMMAND(XPlmi_ScatterWrite2),
		XPLMI_MODULE_COMMAND(XPlmi_TamperTrigger),
		XPLMI_MODULE_COMMAND(XPlmi_SetFipsKatMask),
		XPLMI_MODULE_COMMAND(XPlmi_SetIpiAccess),
		XPLMI_MODULE_COMMAND(XPlmi_RunProc),
		XPLMI_MODULE_COMMAND(XPlmi_ListSet),
		XPLMI_MODULE_COMMAND(XPlmi_ListWrite),
		XPLMI_MODULE_COMMAND(XPlmi_ListMaskWrite),
		XPLMI_MODULE_COMMAND(XPlmi_ListMaskPoll),
	};

	/* Buffer to store access permissions of xilplmi generic module */
	static XPlmi_AccessPerm_t XPlmi_GenericAccessPermBuff[XPLMI_ARRAY_SIZE(XPlmi_GenericCmds)] = {
		XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_FEATURES_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_MASK_POLL_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_MASK_WRITE_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_WRITE_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_DELAY_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_DMA_WRITE_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_MASK_POLL64_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_MASK_WRITE64_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_WRITE64_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_DMA_XFER_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_INIT_SEQ_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_CFI_READ_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_SET_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_WRITE_KEYHOLE_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_SSIT_SYNC_SLAVES_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_SSIT_SYNC_MASTER_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_SSIT_WAIT_SLAVES_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_NOP_CMD_ID),
		XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_GET_DEVICE_CMD_ID),
		XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_EVENT_LOGGING_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_SET_BOARD_CMD_ID),
		XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_GET_BOARD_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_SET_WDT_PARAM_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_LOG_STR_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_LOG_ADDR_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_MARKER_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_PROC_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_BEGIN_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_END_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_BREAK_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_OT_CHECK_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_PSM_SEQUENCE_CMD_ID),
#ifdef VERSAL_NET
		XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_INPLACE_PLM_UPDATE_CMD_ID),
#else
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_INPLACE_PLM_UPDATE_CMD_ID),
#endif
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_SCATTER_WRITE_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_SCATTER_WRITE2_CMD_ID),
		XPLMI_ALL_IPI_FULL_ACCESS(XPLMI_TAMPER_TRIGGER_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_SET_FIPS_MASK_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_SET_IPI_ACCESS_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_RUN_PROC_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_LIST_SET_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_LIST_WRITE_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_LIST_MASK_WRITE_CMD_ID),
		XPLMI_ALL_IPI_NO_ACCESS(XPLMI_LIST_MASK_POLL_CMD_ID),
	};

	/* This is to store CMD_END in xplm_modules section */
	XPLMI_EXPORT_CMD(XPLMI_CDO_END_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ZERO, XPLMI_CMD_ARG_CNT_ZERO);

	XPlmi_Generic.Id = XPLMI_MODULE_GENERIC_ID;
	XPlmi_Generic.CmdAry = XPlmi_GenericCmds;
	XPlmi_Generic.CmdCnt = XPLMI_ARRAY_SIZE(XPlmi_GenericCmds);
	XPlmi_Generic.AccessPermBufferPtr = XPlmi_GenericAccessPermBuff;
#ifdef VERSAL_NET
	XPlmi_Generic.UpdateHandler = XPlmi_GenericHandler;
#endif

	XPlmi_ModuleRegister(&XPlmi_Generic);
}

/*****************************************************************************/
/**
 * @brief	This function gives the address of ReadBack Properties instance
 *
 * @param	None
 *
 * @return	Address of ReadBack variable which is static to this function
 *
 *****************************************************************************/
static XPlmi_ReadBackProps* XPlmi_GetReadBackPropsInstance(void)
{
	static XPlmi_ReadBackProps ReadBack = {
		XPLMI_READBACK_DEF_DST_ADDR, 0U, 0U
	};

	return &ReadBack;
}

/*****************************************************************************/
/**
 * @brief	This function gets the ReadBack Properties Value after readback
 *
 * @param	ReadBackVal is the pointer to which the readback properties
 *		instance is copied
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
int XPlmi_GetReadBackPropsValue(XPlmi_ReadBackProps *ReadBackVal)
{
	int Status = XST_FAILURE;
	const XPlmi_ReadBackProps *ReadBackPtr = XPlmi_GetReadBackPropsInstance();

	Status = Xil_SMemCpy(ReadBackVal, sizeof(XPlmi_ReadBackProps),
			ReadBackPtr, sizeof(XPlmi_ReadBackProps),
			sizeof(XPlmi_ReadBackProps));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets the ReadBack Properties Value
 *
 * @param	ReadBack is the pointer to the Readback Instance that has to be
 * 		set.
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
int XPlmi_SetReadBackProps(const XPlmi_ReadBackProps *ReadBack)
{
	int Status = XST_FAILURE;
	XPlmi_ReadBackProps *ReadBackPtr = XPlmi_GetReadBackPropsInstance();

	Status = Xil_SMemCpy(ReadBackPtr, sizeof(XPlmi_ReadBackProps),
			ReadBack, sizeof(XPlmi_ReadBackProps),
			sizeof(XPlmi_ReadBackProps));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides DMA transfer to CFI bypassing PMCRAM
 *
 * @param	SrcAddr is the address to read cfi data from
 * @param	DestAddr is the address to write cfi data to
 * @param	Keyholesize is the size of the DMA key hole in bytes
 * @param	Len is number of words already transferred
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_CfiWrite(u64 SrcAddr, u64 DestAddr, u32 Keyholesize, u32 Len,
	XPlmi_Cmd* Cmd)
{
	int Status = XST_FAILURE;
	u64 BaseAddr = DestAddr;
	u32 RemData;
	u64 Src = SrcAddr;
	u32 LenTmp = Len << XPLMI_WORD_LEN_SHIFT;
	XPlmi_KeyHoleXfrParams KeyHoleXfrParams;

	KeyHoleXfrParams.SrcAddr = Src;
	KeyHoleXfrParams.DestAddr = DestAddr;
	KeyHoleXfrParams.BaseAddr = BaseAddr;
	KeyHoleXfrParams.Len = LenTmp;
	KeyHoleXfrParams.Keyholesize = Keyholesize;
	KeyHoleXfrParams.Flags = XPLMI_PMCDMA_0;
	KeyHoleXfrParams.Func = NULL;
	Status = XPlmi_KeyHoleXfr(&KeyHoleXfrParams);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
		goto END;
	}

	RemData = (Cmd->Len - Cmd->PayloadLen) * XPLMI_WORD_LEN;
	if (RemData == 0U) {
		goto END;
	}

	if (Cmd->KeyHoleParams.IsNextChunkCopyStarted == (u8)FALSE) {
		LenTmp = 0U;
		goto END2;
	}

	if (Src < XPLMI_PMCRAM_CHUNK_MEMORY_1) {
		Src = XPLMI_PMCRAM_CHUNK_MEMORY_1;
	}
	else {
		Src = XPLMI_PMCRAM_CHUNK_MEMORY;
	}

	Status = Cmd->KeyHoleParams.Func(Src, DestAddr, RemData,
			XPLMI_DEVICE_COPY_STATE_WAIT_DONE);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
		goto END;
	}
	if (RemData > (XPLMI_CHUNK_SIZE / 2U)) {
		LenTmp = (XPLMI_CHUNK_SIZE / 2U);
	}
	else {
		LenTmp = RemData;
	}
	XPlmi_SetPlmLiveStatus();
	KeyHoleXfrParams.SrcAddr = Src;
	KeyHoleXfrParams.Len = LenTmp;
	Status = XPlmi_KeyHoleXfr(&KeyHoleXfrParams);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
		goto END;
	}

	RemData = RemData - LenTmp;
	if (RemData == 0U) {
		goto END1;
	}

END2:
	KeyHoleXfrParams.SrcAddr = Cmd->KeyHoleParams.SrcAddr + LenTmp;
	KeyHoleXfrParams.Len = RemData;
	KeyHoleXfrParams.Flags = 0U;
	KeyHoleXfrParams.Func = Cmd->KeyHoleParams.Func;
	Status = XPlmi_KeyHoleXfr(&KeyHoleXfrParams);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
		goto END;
	}
END1:
	Cmd->KeyHoleParams.ExtraWords = Cmd->Len - Cmd->PayloadLen;
	Cmd->PayloadLen = Cmd->Len + 1U;
	Cmd->ProcessedLen = Cmd->Len;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides DMA transfer to CFI in chunks of
 * Keyholesize.
 *
 * @param	KeyHoleXfrParams is a pointer to instance of CfiParams structure
 *
 * @return
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_KeyHoleXfr(XPlmi_KeyHoleXfrParams* KeyHoleXfrParams)
{
	int Status = XST_FAILURE;
	u32 LenTemp = (u32)(((u64)KeyHoleXfrParams->Keyholesize + KeyHoleXfrParams->BaseAddr)
		- KeyHoleXfrParams->DestAddr);

	if (LenTemp > KeyHoleXfrParams->Len) {
		LenTemp = KeyHoleXfrParams->Len;
	}

	if (LenTemp == 0U) {
		Status = XST_SUCCESS;
		goto END;
	}

	if (KeyHoleXfrParams->Func == NULL) {
		Status = XPlmi_DmaXfr(KeyHoleXfrParams->SrcAddr,
			KeyHoleXfrParams->DestAddr, LenTemp / XPLMI_WORD_LEN,
			KeyHoleXfrParams->Flags);
	}
	else {
		Status = KeyHoleXfrParams->Func(KeyHoleXfrParams->SrcAddr,
			KeyHoleXfrParams->DestAddr, LenTemp, KeyHoleXfrParams->Flags);
	}
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
		XPlmi_SetPlmLiveStatus();
		LenTemp = KeyHoleXfrParams->Keyholesize;
		if (LenTemp > KeyHoleXfrParams->Len) {
			LenTemp = KeyHoleXfrParams->Len;
		}

		if (KeyHoleXfrParams->Func == NULL) {
			Status = XPlmi_DmaXfr(KeyHoleXfrParams->SrcAddr,
				KeyHoleXfrParams->BaseAddr,
				LenTemp / XPLMI_WORD_LEN, KeyHoleXfrParams->Flags);
		}
		else {
			Status = KeyHoleXfrParams->Func(KeyHoleXfrParams->SrcAddr,
				KeyHoleXfrParams->BaseAddr, LenTemp, KeyHoleXfrParams->Flags);
		}
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
 * 			- XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_StackPush(XPlmi_CdoParamsStack *CdoParamsStack, u32 *Data)
{
	int Status = XST_FAILURE;

	/* Validate stack top */
	if (CdoParamsStack->OffsetListTop < -1) {
		XPlmi_Printf(DEBUG_GENERAL,
				"Invalid top in End address stack\n");
		goto END;
	}

	if (CdoParamsStack->OffsetListTop >= (int)(XPLMI_BEGIN_OFFSET_STACK_SIZE - 1U)) {
		XPlmi_Printf(DEBUG_GENERAL,
				"End address stack is full\n");
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
static int XPlmi_StackPop(XPlmi_CdoParamsStack *CdoParamsStack, u32 PopLevel, u32 *Data)
{
	int Status = XST_FAILURE;
	u32 Index;

	/* Validate stack top */
	if (CdoParamsStack->OffsetListTop >= (int)XPLMI_BEGIN_OFFSET_STACK_SIZE) {
		XPlmi_Printf(DEBUG_GENERAL,
				"Invalid top in End address stack\n");
		goto END;
	}

	for (Index = 0U; Index < PopLevel; Index++) {
		if (CdoParamsStack->OffsetListTop < 0) {
			XPlmi_Printf(DEBUG_GENERAL,
				"End address stack is empty\n");
			Status = XST_FAILURE;
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
int XPlmi_GetJumpOffSet(XPlmi_Cmd *Cmd, u32 Level)
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
	if (Level > XPLMI_BEGIN_OFFEST_STACK_DEFAULT_POPLEVEL) {
		Status = XPlmi_StackPop(&Cmd->CdoParamsStack, --Level, &PopAddr);
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

/**
 * @}
 * @endcond
 */

 /** @} */
