/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_generic.c
*
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
*       kc   07/28/2020 Added SetWdt command support
*       skd  07/29/2020 Cfi Write related changes for Qspi and Ospi
*       bm   08/03/2020 Added ReadBack Override support
*       bsv  09/30/2020 Added parallel DMA support for SBI, JTAG, SMAP and PCIE
*                       boot modes
*       bm   10/14/2020 Code clean up
*       td   10/19/2020 MISRA C Fixes
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_generic.h"
#include "xstatus.h"
#include "xplmi_proc.h"
#include "xplmi_hw.h"
#include "xcfupmc.h"
#include "sleep.h"
#include "xplmi_ssit.h"
#include "xplmi_event_logging.h"
#include "xplmi_wdt.h"
#include "xplmi_modules.h"
#include "xplmi_cmd.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XPlmi_CfiWrite(u64 SrcAddr, u64 DestAddr, u32 Keyholesize, u32 Len,
        XPlmi_Cmd* Cmd);
static XPlmi_ReadBackProps* XPlmi_GetReadBackPropsInstance(void);
static int XPlmi_NpiUnalignedXfer(u64 SrcAddr, u64 DestAddr, u32 Count);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	Contains the module ID and PLM generic commands array
 *
 *****************************************************************************/
static XPlmi_Module XPlmi_Generic;

/*****************************************************************************/
/*****************************************************************************/
/**
 * @brief	This function checks if a particular PLM Command ID is supported
 * or not. Command ID is the only payload parameter.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XPlmi_Features(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;

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
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XPlmi_Nop(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;

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
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XPlmi_GetDeviceID(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 ExtIdCode;

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
 * @brief	This function provides 32 bit mask poll command execution.
 *  		Command payload parameters are
 *		- Address
 *		- Mask
 *		- Expected Value
 *		- Timeout in us
 *		- Deferred Error flag - Optional
 *		-	0 - Return error in case of failure,
 *		-	1 - Ignore error, return success always
 *		-	2 - Defer error till the end of partition load
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_MaskPoll(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Addr = Cmd->Payload[0U];
	u32 Mask = Cmd->Payload[1U];
	u32 ExpectedValue = Cmd->Payload[2U];
	u32 TimeOutInUs = Cmd->Payload[3U];
	u32 Flags = 0U;
#ifdef PLM_PRINT_PERF_POLL
	u64 PollTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime = {0U};
#endif

	/* HACK - waiting for min of 1s **/
	if (TimeOutInUs < XPLMI_MASK_POLL_MIN_TIMEOUT) {
		TimeOutInUs = XPLMI_MASK_POLL_MIN_TIMEOUT;
	}

	Status = XPlmi_UtilPoll(Addr, Mask, ExpectedValue, TimeOutInUs);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL,
			"%s: Addr: 0x%0x,  Mask: 0x%0x, ExpVal: 0x%0x, "
			"Timeout: %u ...ERROR\r\n",  __func__,
			Addr, Mask, ExpectedValue, TimeOutInUs);
	} else {
		XPlmi_Printf(DEBUG_INFO,
			"%s: Addr: 0x%0x,  Mask: 0x%0x, ExpVal: 0x%0x, "
			"Timeout: %u ...Done\r\n",  __func__,
			Addr, Mask, ExpectedValue, TimeOutInUs);
	}
#ifdef PLM_PRINT_PERF_POLL
	XPlmi_MeasurePerfTime(PollTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF,
		" %u.%06u ms Poll Time: Addr: 0x%0x,  Mask: 0x%0x,"
		" ExpVal: 0x%0x, Timeout: %u \r\n",
		(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac,
		Addr, Mask, ExpectedValue, TimeOutInUs);
#endif

	/* If command length is 5, flags are included */
	if ((Cmd->Len == XPLMI_MASKPOLL_LEN_EXT) && (Status != XST_SUCCESS)) {
		Flags = Cmd->Payload[4U] & XPLMI_MASKPOLL_FLAGS_MASK;
		if (Flags == XPLMI_MASKPOLL_FLAGS_SUCCESS) {
			/* Ignore the error */
			Status = XST_SUCCESS;
		} else if (Flags == XPLMI_MASKPOLL_FLAGS_DEFERRED_ERR) {
			/* Defer the error till the end of CDO processing */
			Status = XST_SUCCESS;
			Cmd->DeferredError = (u8)TRUE;
		} else {
			/* Return mask_poll status */
		}
	}

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
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XPlmi_MaskWrite(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Addr = Cmd->Payload[0U];
	u32 Mask = Cmd->Payload[1U];
	u32 Value = Cmd->Payload[2U];

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
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XPlmi_Write(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Addr = Cmd->Payload[0U];
	u32 Value = Cmd->Payload[1U];

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
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XPlmi_Delay(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Delay;

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
 * @brief	This function provides DMA write command execution.
 *  		Command payload parameters are
 *		- High Dest Addr
 *		- Low Dest Addr
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_DmaWrite(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 DestAddr;
	u64 SrcAddr;
	u32 Len = Cmd->PayloadLen;
	u32 Flags;
	u32 DestOffset = 0U;

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

	/* Set DMA flags to DMA0 and INCR */
	Flags = XPLMI_PMCDMA_0;

	Status = XPlmi_DmaXfr(SrcAddr, DestAddr, Len, Flags);
	if(Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Failed\n\r");
		goto END;
	}

END:
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
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_MaskPoll64(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 Addr = ((u64)Cmd->Payload[0U] << 32U) | Cmd->Payload[1U];
	u32 Mask = Cmd->Payload[2U];
	u32 ExpectedValue = Cmd->Payload[3U];
	u32 TimeOutInUs = Cmd->Payload[4U];
	u32 Flags = 0U;

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x%08x,  Mask 0x%0x, ExpVal: 0x%0x, Timeout: %u\n\r",
		__func__, (u32)(Addr >> 32U), (u32)Addr, Mask, ExpectedValue, TimeOutInUs);

	Status = XPlmi_UtilPoll64(Addr, Mask, ExpectedValue, TimeOutInUs);
	if (Status != XST_SUCCESS) {
		Status = XPLMI_ERR_MASKPOLL64;
	}

	/* If command length is 6, flags are included */
	if ((Cmd->Len == XPLMI_MASKPOLL64_LEN_EXT) && (Status != XST_SUCCESS)) {
		Flags = Cmd->Payload[5U] & XPLMI_MASKPOLL_FLAGS_MASK;
		if (Flags == XPLMI_MASKPOLL_FLAGS_SUCCESS) {
			/* Ignore the error */
			Status = XST_SUCCESS;
		} else if (Flags == XPLMI_MASKPOLL_FLAGS_DEFERRED_ERR) {
			/* Defer the error till the end of CDO processing */
			Status = XST_SUCCESS;
			Cmd->DeferredError = (u8)TRUE;
		} else {
			/* Return mask_poll status */
		}
	}

	return Status;
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
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XPlmi_MaskWrite64(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 Addr = ((u64)Cmd->Payload[0U] << 32U) | Cmd->Payload[1U];
	u32 Mask = Cmd->Payload[2U];
	u32 Value = Cmd->Payload[3U];
	u32 ReadVal;

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
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XPlmi_Write64(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 Addr = ((u64)Cmd->Payload[0U] << 32U) | Cmd->Payload[1U];
	u32 Value = Cmd->Payload[2U];

	XPlmi_Printf(DEBUG_DETAILED,
		"%s, Addr: 0x%0x%08x,  Val: 0x%0x\n\r",
		__func__, (u32)(Addr>>32), (u32)Addr, Value);

	XPlmi_Out64(Addr, Value);
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	The function reads data from Npi address space.
 *
 * @param	Src Address in Npi address space.
 * @param   	Destination Address
 * @param	Len is number of words to be read
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
static int XPlmi_NpiRead(u64 SrcAddr, u64 DestAddr, u32 Len)
{
	int Status = XST_FAILURE;
	u32 Count = 0U;
	u32 Offset;
	u32 ProcWords = 0U;
	u32 XferLen;
	XPlmi_ReadBackProps *ReadBackPtr = XPlmi_GetReadBackPropsInstance();

	/* Check if Readback Dest Addr is Overriden */
	if ((XPLMI_READBACK_DEF_DST_ADDR != ReadBackPtr->DestAddr) &&
			(XPLMI_SBI_DEST_ADDR != DestAddr)) {
		DestAddr = ReadBackPtr->DestAddr;
		if ((Len + ReadBackPtr->ProcessedLen) > ReadBackPtr->MaxSize) {
			Status = XPLMI_ERR_READBACK_BUFFER_OVERFLOW;
			XPlmi_Printf(DEBUG_GENERAL,
				"ReadBack Buffer Overflow\n\r");
			goto END;
		}
	}

	/* For NPI READ command, the source address needs to be
	 * 16 byte aligned. Use XPlmi_Out64 till the destination address
	 * becomes 16 byte aligned.
	 */
	if (((SrcAddr & XPLMI_SIXTEEN_BYTE_MASK) != 0U) ||
		(Len < XPLMI_SIXTEEN_BYTE_WORDS)) {
		if (Len < XPLMI_SIXTEEN_BYTE_WORDS) {
			Count = Len;
		}
		else {
			Count = (u32)(SrcAddr & XPLMI_SIXTEEN_BYTE_MASK);
			Count = (u32)(((u32)XPLMI_SIXTEEN_BYTE_VALUE - Count) /
					(u32)XPLMI_WORD_LEN);
		}
		Status = XPlmi_NpiUnalignedXfer(SrcAddr, DestAddr, Count);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		ProcWords += Count;
	}

	XferLen = (Len - ProcWords) & (~(XPLMI_WORD_LEN - 1U));
	Offset = ProcWords * XPLMI_WORD_LEN;

	if (DestAddr != XPLMI_SBI_DEST_ADDR) {
		Status = XPlmi_DmaXfr((u64)(SrcAddr + Offset),
				(u64)(DestAddr + Offset), XferLen, XPLMI_PMCDMA_0);
	} else {
		Status = XPlmi_DmaSbiXfer((u64)(SrcAddr + Offset),
				XferLen, XPLMI_PMCDMA_1);
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* For NPI_READ command, Offset variable should
	 *  be updated with the unaligned bytes.
	 */
	ProcWords += XferLen;
	Offset = ProcWords * XPLMI_WORD_LEN;

	if (ProcWords < Len) {
		XferLen = Len - ProcWords;
		Status = XPlmi_NpiUnalignedXfer((u64)(SrcAddr + Offset),
				(u64)(DestAddr + Offset), XferLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		ProcWords += XferLen;
	}

	if ((XPLMI_READBACK_DEF_DST_ADDR != ReadBackPtr->DestAddr) &&
			(XPLMI_SBI_DEST_ADDR != DestAddr)) {
		ReadBackPtr->ProcessedLen += Len;
		ReadBackPtr->DestAddr += ((u64)Len * XPLMI_WORD_LEN);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function does unaligned Npi transfer
 *
 * @param	SrcAddr is Source Address in Npi address space
 * @param   	DestAddr is the Destination Address
 * @param	Count is the number of words to transfer
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_NpiUnalignedXfer(u64 SrcAddr, u64 DestAddr, u32 Count)
{
	int Status = XST_FAILURE;
	u32 RegVal;
	u32 Index = 0;

	if (DestAddr != XPLMI_SBI_DEST_ADDR) {
		for (Index = 0; Index < Count; Index++) {
			RegVal = XPlmi_In64(SrcAddr + ((u64)Index * XPLMI_WORD_LEN));
			XPlmi_Out64(DestAddr + ((u64)Index * XPLMI_WORD_LEN), RegVal);
		}
		Status = XST_SUCCESS;
	} else {
		Status = XPlmi_DmaSbiXfer(SrcAddr, Count, XPLMI_PMCDMA_1);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

END:
	return Status;
}

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
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_DmaXfer(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 DestAddr;
	u64 SrcAddr;
	u32 Len;
	u32 Flags;

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
				XPLMI_TIME_OUT_DEFAULT);
		if (Status != XST_SUCCESS) {
			XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE,
				SLAVE_BOOT_SBI_MODE_SELECT_MASK, 0U);
			goto END;
		}

		Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
				SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_MASK,
				SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_VAL,
				XPLMI_TIME_OUT_DEFAULT);
		XPlmi_UtilRMW(SLAVE_BOOT_SBI_MODE,
				SLAVE_BOOT_SBI_MODE_SELECT_MASK, 0U);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	} else {
		if ((Flags & XPLMI_DMA_SRC_NPI) == XPLMI_DMA_SRC_NPI) {
			Status = XPlmi_NpiRead(SrcAddr, DestAddr, Len);
		} else {
			Status = XPlmi_DmaXfr(SrcAddr, DestAddr, Len, Flags);
		}
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "DMA XFER Failed\n\r");
			goto END;
		}
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
 * @return	XPLMI_ERR_CMD_NOT_SUPPORTED
 *
 *****************************************************************************/
static int XPlmi_InitSeq(XPlmi_Cmd *Cmd)
{
	/* For MISRA C */
	(void)Cmd;

	return XPLMI_ERR_CMD_NOT_SUPPORTED;
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
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_CfiRead(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 SrcType = Cmd->Payload[0U];
	u64 DestAddrHigh;
	u32 DestAddrLow;
	u32 Len = Cmd->Payload[3U];
	u32 SrcAddr = CFU_FDRO_ADDR;
	u64 DestAddr;
	XPlmi_ReadBackProps *ReadBackPtr = XPlmi_GetReadBackPropsInstance();

	XPlmi_SetMaxOutCmds(XPLMI_MAXOUT_CMD_MIN_VAL);

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
		Status = XPlmi_DmaXfr(CFU_FDRO_ADDR, DestAddr, Len,
				XPLMI_PMCDMA_1 | XPLMI_SRC_CH_AXI_FIXED |
				XPLMI_DMA_SRC_NONBLK);
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
		Status = XPlmi_DmaSbiXfer(CFU_FDRO_ADDR, Len, XPLMI_PMCDMA_1
				| XPLMI_SRC_CH_AXI_FIXED | XPLMI_DMA_SRC_NONBLK);
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}
	XPlmi_SetMaxOutCmds(XPLMI_MAXOUT_CMD_DEF_VAL);
	SrcAddr = (u32)(&Cmd->Payload[XPLMI_CFI_DATA_OFFSET]);

	Status = XPlmi_DmaXfr(SrcAddr, (u64)CFU_STREAM_ADDR,
		Cmd->PayloadLen - XPLMI_CFI_DATA_OFFSET, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (SrcType == XPLMI_READBK_INTF_TYPE_DDR) {
		Status = XPlmi_WaitForNonBlkDma(XPLMI_PMCDMA_1);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (XPLMI_READBACK_DEF_DST_ADDR != ReadBackPtr->DestAddr) {
			ReadBackPtr->ProcessedLen += Len;
			ReadBackPtr->DestAddr += ((u64)Len * XPLMI_WORD_LEN);
		}
		goto END;
	}
	Status = XPlmi_WaitForNonBlkSrcDma(XPLMI_PMCDMA_1);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
		SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_MASK,
		SLAVE_BOOT_SBI_STATUS_CMN_BUF_SPACE_VAL,
		XPLMI_TIME_OUT_DEFAULT);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (SrcType == XPLMI_READBK_INTF_TYPE_SMAP) {
		Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
			SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_MASK,
			SLAVE_BOOT_SBI_STATUS_SMAP_DOUT_FIFO_SPACE_VAL,
			XPLMI_TIME_OUT_DEFAULT);
	} else {
		Status = XPlmi_UtilPoll(SLAVE_BOOT_SBI_STATUS,
			SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_MASK,
			SLAVE_BOOT_SBI_STATUS_JTAG_DOUT_FIFO_SPACE_VAL,
			XPLMI_TIME_OUT_DEFAULT);
	}
	if(Status != XST_SUCCESS) {
		goto END;
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
 * @return	XST_SUCCESS
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
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_DmaWriteKeyHole(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 DestAddr;
	u64 SrcAddr;
	u32 Len = Cmd->PayloadLen;
	u32 Flags;
	u32 Keyholesize;
	u32 ChunkLen;
	u32 Count;
	u32* CfiDataPtr;
	u64 BaseAddr;
	u32 DestOffset;
	u32 ChunkLenTemp = 0U;
#ifdef PLM_PRINT_PERF_KEYHOLE
	u64 KeyHoleTime = XPlmi_GetTimerValue();
	XPlmi_PerfTime PerfTime = {0U};
#endif
	XPlmi_Printf(DEBUG_DETAILED, "%s \n\r", __func__);

	if (Cmd->ProcessedLen == 0U) {
		/* Store the destination address in resume data */
		Cmd->ResumeData[0U] = Cmd->Payload[0U];
		Cmd->ResumeData[1U] = Cmd->Payload[1U];
		Cmd->ResumeData[2U] = Cmd->Payload[2U];
		Keyholesize = Cmd->Payload[2U];
		SrcAddr = (u64)(UINTPTR) &Cmd->Payload[3U];
		Len -= 3U;
		Cmd->ResumeData[3U] = 0U;
		DestOffset = 0U;
	} else {
		SrcAddr = (u64)(UINTPTR) &Cmd->Payload[0U];
		Keyholesize = Cmd->ResumeData[2U];
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
	/* Set DMA flags to DMA0 */
	Flags = XPLMI_PMCDMA_0;
	if (Cmd->ProcessedLen != 0U) {
		Count = Cmd->ResumeData[3U];
		if (Count > 0U) {
			while (Count < XPLMI_KEYHOLE_RESUME_SIZE) {
				CfiDataPtr = (u32*)(UINTPTR)SrcAddr;
				Cmd->ResumeData[Count + XPLMI_WORD_LEN] = *CfiDataPtr;
				SrcAddr += XPLMI_WORD_LEN;
				--Len;
				++Count;
			}
			Status = XPlmi_DmaXfr((u32)&Cmd->ResumeData[4U], DestAddr,
					XPLMI_KEYHOLE_RESUME_SIZE, Flags);
			if(Status != XST_SUCCESS) {
				XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
				goto END;
			}
			DestAddr += XPLMI_KEYHOLE_RESUME_SIZE * XPLMI_WORD_LEN;
		}
		Cmd->ResumeData[3U] = 0U;
	}

	ChunkLen = Len - (Len & (XPLMI_WORD_LEN - 1U));
	if ((DestAddr + ((u64)ChunkLen * XPLMI_WORD_LEN)) > (BaseAddr + Keyholesize)) {
		ChunkLenTemp = (u32)(((BaseAddr + Keyholesize) - DestAddr) / XPLMI_WORD_LEN);
		Status = XPlmi_DmaXfr(SrcAddr, DestAddr, ChunkLenTemp, Flags);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
			goto END;
		}
		DestAddr = BaseAddr;
	}

	Status = XPlmi_DmaXfr(SrcAddr + ((u64)ChunkLenTemp * XPLMI_WORD_LEN),
		DestAddr, (ChunkLen - ChunkLenTemp), Flags);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
		goto END;
	}

	Cmd->ResumeData[3U]= Len & (XPLMI_WORD_LEN - 1U);
	CfiDataPtr = (u32*)(UINTPTR)(SrcAddr + ((u64)ChunkLen * XPLMI_WORD_LEN));
	Count = 0U;
	while (ChunkLen < Len) {
		Cmd->ResumeData[Count + 4U] =  *CfiDataPtr;
		CfiDataPtr++;
		ChunkLen++;
		++Count;
	}

	/*
	 * Send unaligned bytes left at the end if any
	 */
	if ((Cmd->ProcessedLen + (ChunkLen - (Len % XPLMI_WORD_LEN)) +
		(Cmd->PayloadLen - Len)) == (Cmd->Len - (Len % XPLMI_WORD_LEN))) {
		if (Count > 0U) {
			XPlmi_Printf(DEBUG_DETAILED, "Last remaining bytes\r\n");
			Status = XPlmi_DmaXfr((u32)&Cmd->ResumeData[4U], DestAddr,
				Count, Flags);
		}
	}

END:
#ifdef PLM_PRINT_PERF_KEYHOLE
	XPlmi_MeasurePerfTime(KeyHoleTime, &PerfTime);
	XPlmi_Printf(DEBUG_PRINT_PERF, " %u.%06u ms KeyHole Run Time \r\n",
	(u32)PerfTime.TPerfMs, (u32)PerfTime.TPerfMsFrac);
#endif
	return Status;
}

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
static u8* XPlmi_BoardNameRW(XPlmi_Cmd *Cmd, u8 GetFlag, u32 *Len)
{
	int Status = XST_FAILURE;
	static u8 BoardName[XPLMI_MAX_NAME_LEN + 1U] = {0U,};
	static u32 BoardLen = 0U;

	if (*Len > XPLMI_MAX_NAME_WORDS) {
		*Len = XPLMI_MAX_NAME_WORDS;
	}

	if (GetFlag == (u8)FALSE) {
		/* Set Command */
		Status = XPlmi_DmaXfr((u64)(u32)&Cmd->Payload[0U],
				(u64)(u32)BoardName, *Len, XPLMI_PMCDMA_0);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		BoardName[*Len * XPLMI_WORD_LEN] = 0U;
		BoardLen = *Len;
	}
	else {
		*Len = BoardLen;
	}

END:
	return BoardName;
}

/*****************************************************************************/
/**
 * @brief	This function provides SET BOARD command execution.
 *  		Command payload parameters are
 *		- Board Name
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XPlmi_SetBoard(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Len = Cmd->PayloadLen;

	(void)XPlmi_BoardNameRW(Cmd, (u8)FALSE, &Len);

	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides GET BOARD command execution.
 * 		No payload parameters
 *
 * @param	Cmd is pointer to the command structure
 *		Command payload parameters are
 *		- High Addr
 *		- Low Addr
 *		- Max size in words
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_GetBoard(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 HighAddr = Cmd->Payload[0U];
	u32 LowAddr = Cmd->Payload[1U];
	u64 DestAddr = (HighAddr << 32U) | LowAddr;
	u32 Len = Cmd->Payload[2U];
	u8* BoardName = XPlmi_BoardNameRW(Cmd, (u8)TRUE, &Len);

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
 * @brief	This function sets the WDT parameters used in PLM
 *
 * @param	Cmd is pointer to the command structure
 *		Command payload parameters are
 *		- Node Id for PMC, PS MIO
 *		- Periodicity
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_SetWdtParam(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 NodeId = Cmd->Payload[0U];
	u32 Periodicity = Cmd->Payload[1U];

	XPlmi_Printf(DEBUG_INFO, "Enabling WDT with Node:0x%08x, "
		     "Periodicity: %u ms\n\r", NodeId, Periodicity);
	Status = XPlmi_EnableWdt(NodeId, Periodicity);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	Contains the array of PLM generic commands
 *
 *****************************************************************************/
static XPlmi_ModuleCmd XPlmi_GenericCmds[] =
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
};

/*****************************************************************************/
/**
 * @brief	This function registers the PLM generic commands to the PLMI.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_GenericInit(void)
{
	XPlmi_Generic.Id = XPLMI_MODULE_GENERIC_ID;
	XPlmi_Generic.CmdAry = XPlmi_GenericCmds;
	XPlmi_Generic.CmdCnt = XPLMI_ARRAY_SIZE(XPlmi_GenericCmds);

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
 * @return	None
 *
 *****************************************************************************/
int XPlmi_GetReadBackPropsValue(XPlmi_ReadBackProps *ReadBackVal)
{
	int Status = XST_FAILURE;
	XPlmi_ReadBackProps *ReadBackPtr = XPlmi_GetReadBackPropsInstance();

	Status = Xil_SecureMemCpy(ReadBackVal, sizeof(XPlmi_ReadBackProps),
			ReadBackPtr, sizeof(XPlmi_ReadBackProps));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sets the ReadBack Properties Value
 *
 * @param	ReadBack is the pointer to the Readback Instance that has to be
 * 		set.
 *
 * @return	None
 *
 *****************************************************************************/
int XPlmi_SetReadBackProps(XPlmi_ReadBackProps *ReadBack)
{
	int Status = XST_FAILURE;
	XPlmi_ReadBackProps *ReadBackPtr = XPlmi_GetReadBackPropsInstance();

	Status = Xil_SecureMemCpy(ReadBackPtr, sizeof(XPlmi_ReadBackProps),
			ReadBack, sizeof(XPlmi_ReadBackProps));

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
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_CfiWrite(u64 SrcAddr, u64 DestAddr, u32 Keyholesize, u32 Len,
	XPlmi_Cmd* Cmd)
{
	int Status = XST_FAILURE;
	u64 BaseAddr;
	u32 RemData;
	u32 ChunkLen;

	Status = XPlmi_DmaXfr(SrcAddr, DestAddr, Len, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
		goto END;
	}

	BaseAddr = DestAddr;
	DestAddr = (((u64)Len * XPLMI_WORD_LEN) % Keyholesize) + BaseAddr;
	RemData = (Cmd->Len - Cmd->PayloadLen) * XPLMI_WORD_LEN;
	if (RemData == 0U) {
		goto END;
	}

	if (SrcAddr < XPLMI_PMCRAM_CHUNK_MEMORY_1) {
		SrcAddr = XPLMI_PMCRAM_CHUNK_MEMORY_1;
	}
	else {
		SrcAddr = XPLMI_PMCRAM_CHUNK_MEMORY;
	}

	Status = Cmd->KeyHoleParams.Func(SrcAddr, DestAddr, RemData,
			XPLMI_DEVICE_COPY_STATE_WAIT_DONE);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
		goto END;
	}
	if (RemData > (XPLMI_CHUNK_SIZE / 2U)) {
		Len = (XPLMI_CHUNK_SIZE / 2U);
	}
	else {
		Len = RemData;
	}
	Status = XPlmi_DmaXfr(SrcAddr, DestAddr, (Len / XPLMI_WORD_LEN),
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
		goto END;
	}

	RemData = RemData - Len;
	if (RemData == 0U) {
		goto END1;
	}
	SrcAddr = Cmd->KeyHoleParams.SrcAddr + Len;
	DestAddr = ((DestAddr + Len) % Keyholesize) + BaseAddr;

	/* The block is for qspi, ospi, ddr, sbi, jtag, smap and pcie boot modes */
	if (Cmd->KeyHoleParams.InChunkCopy == (u8)FALSE) {
		/*
		 * The block is for sbi and smap boot modes which
		 * support fixed modes
		 */
		Status = Cmd->KeyHoleParams.Func(SrcAddr, DestAddr, RemData,
				XPLMI_DST_CH_AXI_FIXED);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
			goto END;
		}
	} else {
		/*
		 * Qspi and Ospi donot support fixed modes.
		 * Hence the bitstream is copied in chunks of keyhole size.
		 * This block of code will also get executed for SSIT.
		 */
		Len = Keyholesize - (u32)(DestAddr - BaseAddr);
		if (Len > RemData) {
			Len = RemData;
		}
		Status = Cmd->KeyHoleParams.Func(SrcAddr, DestAddr, Len, 0U);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL, "DMA WRITE Key Hole Failed\n\r");
			goto END;
		}
		RemData = RemData - Len;
		if (RemData == 0U) {
			goto END1;
		}
		DestAddr = BaseAddr;
		SrcAddr += Len;
		while (RemData > 0U) {
			if (RemData > Keyholesize) {
				ChunkLen = Keyholesize;
			} else {
				ChunkLen = RemData;
			}

			Status = Cmd->KeyHoleParams.Func(SrcAddr,
				DestAddr, ChunkLen, 0U);
			if (Status != XST_SUCCESS) {
				XPlmi_Printf(DEBUG_GENERAL,
					"DMA WRITE Key Hole Failed\n\r");
				goto END;
			}

			if (RemData > Keyholesize) {
				SrcAddr += ChunkLen;
				RemData -= ChunkLen;
			} else {
				break;
			}
		}
	}

END1:
	Cmd->KeyHoleParams.ExtraWords = Cmd->Len - Cmd->PayloadLen;
	Cmd->PayloadLen = Cmd->Len + 1U;
	Cmd->ProcessedLen = Cmd->Len;
END:
	return Status;
}
