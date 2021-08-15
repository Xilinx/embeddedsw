/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
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
#include "xplmi_ssit.h"
#include "xplmi_event_logging.h"
#include "xplmi_wdt.h"
#include "xplmi_modules.h"
#include "xplmi_cmd.h"
#include "xil_util.h"
#include "xplmi_cdo.h"

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

/* SSIT readback max length in words */
#define XPLMI_SSIT_MAX_READBACK_SIZE	(0x10000U)

/* SSIT SLR related macros */
#define XPLMI_CFU_STREAM_2_SLR_OFFSET	\
	(CFU_STREAM_2_ADDR - XPLMI_PMC_LOCAL_BASEADDR)
#define XPLMI_CFU_FDRO_2_SLR_OFFSET		\
	(CFU_FDRO_2_ADDR - XPLMI_PMC_LOCAL_BASEADDR)

#define XPLMI_SLR1_CFU_FDRO_2_ADDR	\
	(XPLMI_PMC_ALIAS1_BASEADDR + XPLMI_CFU_FDRO_2_SLR_OFFSET)
#define XPLMI_SLR2_CFU_FDRO_2_ADDR	\
	(XPLMI_PMC_ALIAS2_BASEADDR + XPLMI_CFU_FDRO_2_SLR_OFFSET)
#define XPLMI_SLR3_CFU_FDRO_2_ADDR	\
	(XPLMI_PMC_ALIAS3_BASEADDR + XPLMI_CFU_FDRO_2_SLR_OFFSET)

#define XPLMI_SLR1_CFU_STREAM_2_ADDR	\
	(XPLMI_PMC_ALIAS1_BASEADDR + XPLMI_CFU_STREAM_2_SLR_OFFSET)
#define XPLMI_SLR2_CFU_STREAM_2_ADDR	\
	(XPLMI_PMC_ALIAS2_BASEADDR + XPLMI_CFU_STREAM_2_SLR_OFFSET)
#define XPLMI_SLR3_CFU_STREAM_2_ADDR	\
	(XPLMI_PMC_ALIAS3_BASEADDR + XPLMI_CFU_STREAM_2_SLR_OFFSET)

/************************** Function Prototypes ******************************/
static int XPlmi_CfiWrite(u64 SrcAddr, u64 DestAddr, u32 Keyholesize, u32 Len,
        XPlmi_Cmd* Cmd);
static XPlmi_ReadBackProps* XPlmi_GetReadBackPropsInstance(void);
static int XPlmi_DmaUnalignedXfer(u64* SrcAddr, u64* DestAddr, u32* Len,
	u8 Flag);
static int XPlmi_KeyHoleXfr(XPlmi_KeyHoleXfrParams* KeyHoleXfrParams);

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
		" %u.%03u ms Poll Time: Addr: 0x%0x,  Mask: 0x%0x,"
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
 * @brief	This function provides functionality for DMA write.
 *
 * @param	Dest is the destination address
 *          Src is the source address
 *          Len is the number of words to be transferred
 *          Flags is the DMA transfer related flags
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_DmaTransfer(u64 Dest, u64 Src, u32 Len, u32 Flags)
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
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_DmaWrite(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u64 DestAddr;
	u64 SrcAddr;
	u32 Len = Cmd->PayloadLen;
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
 * @return	XST_SUCCESS on success and error code on failure
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

	/* Check if Readback Dest Addr is Overriden */
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

	/* For NPI READ command, the source address needs to be
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

	/* For NPI_READ command, Offset variable should
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
 * @return	XST_SUCCESS on success and error code on failure
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
	u32 SrcType = Cmd->Payload[0U] & XPLMI_READBACK_SRC_MASK;
	u32 SlrType = (Cmd->Payload[0U] & XPLMI_READBACK_SLR_TYPE_MASK) >>
					XPLMI_READBACK_SLR_TYPE_SHIFT;
	u64 DestAddrHigh;
	u32 DestAddrLow;
	u64 SrcAddr = (u64)CFU_FDRO_2_ADDR;
	u64 DestAddrRead = (u64)CFU_STREAM_2_ADDR;
	u64 DestAddr = 0UL;
	XPlmi_ReadBackProps *ReadBackPtr = XPlmi_GetReadBackPropsInstance();
	u32 ReadLen;
	u32 Len = Cmd->Payload[3U];
	u32 CfiPayloadSrcAddr = (u32)(&Cmd->Payload[XPLMI_CFI_DATA_OFFSET]);

	if (SlrType == XPLMI_READBACK_SLR_TYPE_1) {
		SrcAddr = XPLMI_SLR1_CFU_FDRO_2_ADDR;
		DestAddrRead = XPLMI_SLR1_CFU_STREAM_2_ADDR;
	} else if (SlrType == XPLMI_READBACK_SLR_TYPE_2) {
		SrcAddr = XPLMI_SLR2_CFU_FDRO_2_ADDR;
		DestAddrRead = XPLMI_SLR2_CFU_STREAM_2_ADDR;
	} else if (SlrType == XPLMI_READBACK_SLR_TYPE_3) {
		SrcAddr = XPLMI_SLR3_CFU_FDRO_2_ADDR;
		DestAddrRead = XPLMI_SLR3_CFU_STREAM_2_ADDR;
	} else {
		/* For Misra-C */
	}

	if (Len > XPLMI_SSIT_MAX_READBACK_SIZE) {
		ReadLen = XPLMI_SSIT_MAX_READBACK_SIZE;
	} else {
		ReadLen = Len;
	}
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
		if (Len > XPLMI_SSIT_MAX_READBACK_SIZE) {
			ReadLen = XPLMI_SSIT_MAX_READBACK_SIZE;
		} else {
			ReadLen = Len;
		}

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
	u32 Keyholesize;
	u64 BaseAddr;
	u32 DestOffset;
	XPlmi_KeyHoleXfrParams KeyHoleXfrParams;
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
	u64 DestAddr = (HighAddr << XPLMI_NUM_BITS_IN_WORD) | LowAddr;
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
 * @brief	This function sets the WDT parameters used in PLM.
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
 * @brief	This function adds Debug string to PLM logs.
 *
 * @param	Cmd is pointer to the command structure
 *		Command payload parameters are
 *		- Debug String
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_LogString(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 Len = Cmd->PayloadLen;
	static u8 LogString[XPLMI_MAX_LOG_STR_LEN] __attribute__ ((aligned(4U)));
	static u32 StringIndex = 0U;

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
	Status = Xil_SecureMemCpy(&LogString[StringIndex],
		(XPLMI_MAX_LOG_STR_LEN - StringIndex), (u8 *)&Cmd->Payload[0U],
		(Len * XPLMI_WORD_LEN));
	if (Status != XST_SUCCESS) {
		goto END;
	}
	StringIndex += (Len * XPLMI_WORD_LEN);
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
 * the value to PLM logs.
 *
 * @param	Cmd is pointer to the command structure
 *		Command payload parameters are
 *		- Low Address
 *		- High Address (Optional)
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_LogAddress(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 LowAddr = Cmd->Payload[XPLMI_LOG_ADDR_ARG_LOW_ADDR_INDEX];
	u64 HighAddr = 0UL;
	u64 Addr;
	u32 Val;

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
 * @return	XST_SUCCESS
 *
 *****************************************************************************/
static int XPlmi_Marker(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);
	/* Return success */
	Status = XST_SUCCESS;

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides functionality to move procs when proc
 *          command is received for existing ProcId.
 *
 * @param	ProcIndex is the index of ProcId to be moved
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_MoveProc(u8 ProcIndex, XPlmi_ProcList *ProcList)
{
	int Status = XST_FAILURE;
	u32 DestAddr;
	u32 SrcAddr;
	u32 Len;
	u8 Index = ProcIndex;
	u32 DeletedProcLen;

	/*
	 * If only one proc is available and new proc command is received with
	 * same ID, it can directly be overwritten.
	 */
	if ((ProcList->ProcCount == 1U) ||
		(Index == (ProcList->ProcCount - 1U))) {
		Status = XST_SUCCESS;
		goto END;
	}

	/*
	 * If proc command is received for existing ProcId,
	 * move all procs behind this to front
	 */
	DestAddr = ProcList->ProcData[Index].Addr;
	SrcAddr = ProcList->ProcData[Index + 1U].Addr;
	Len = (ProcList->ProcData[ProcList->ProcCount].Addr -
			SrcAddr)/XPLMI_WORD_LEN;
	/* Length of the proc that is removed */
	DeletedProcLen = ProcList->ProcData[Index + 1U].Addr -
			ProcList->ProcData[Index].Addr;

	/* Call XPlmi_DmaTransfer with flags DMA0 and INCR */
	Status = XPlmi_DmaTransfer(DestAddr, SrcAddr, Len, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Update ProcList with moved data */
	while(Index <= ProcList->ProcCount) {
		ProcList->ProcData[Index].Id = ProcList->ProcData[Index + 1U].Id;
		ProcList->ProcData[Index].Addr =
					ProcList->ProcData[Index + 1U].Addr - DeletedProcLen;
		Index++;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function defines ProcList and returns the address of the same
 *
 * @return	ProcList is the address of ProcList structure
 *
 *****************************************************************************/
static XPlmi_ProcList* XPlmi_GetProcList(void)
{
	static XPlmi_ProcList ProcList = {0U};

	/* Initialize first ProcData address to PSM RAM Proc data start address */
	ProcList.ProcData[0U].Addr = XPLMI_PROC_LOCATION_ADDRESS;

	return &ProcList;
}

/*****************************************************************************/
/**
 * @brief	This function provides functionality to execute given ProcId when
 *          request to execute a particular ProcId.
 *
 * @param	ProcId is ProcId to be executed
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_ExecuteProc(u32 ProcId)
{
	int Status = XST_FAILURE;
	XPlmiCdo ProcCdo;
	XPlmi_ProcList *ProcList = NULL;
	u8 ProcIndex = 0U;
	XPlmi_Printf(DEBUG_GENERAL, "Proc ID received: 0x%x\r\n", ProcId);

	/* If LPD is not initialized, do not execute the proc */
	if ((LpdInitialized & LPD_INITIALIZED) != LPD_INITIALIZED) {
		XPlmi_Printf(DEBUG_GENERAL, "LPD is not initialized, "
				"Proc commands cannot be executed\r\n");
		Status = (int)XPLMI_ERR_PROC_LPD_NOT_INITIALIZED;
		goto END;
	}
	ProcList = XPlmi_GetProcList();
	ProcList->ProcData[ProcList->ProcCount].Id = ProcId;
	while (ProcList->ProcData[ProcIndex].Id != ProcId) {
		ProcIndex++;
	}
	/* Execute proc if the received ProcId is valid */
	if (ProcIndex < ProcList->ProcCount) {
		/* Pass the Proc CDO to CDO parser */
		Status = XPlmi_MemSetBytes((const void *)&ProcCdo,
			sizeof(ProcCdo), 0U, sizeof(ProcCdo));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* Fill ProcCdo structure with Proc related parameters */
		ProcCdo.BufPtr = (u32 *)(UINTPTR)ProcList->ProcData[ProcIndex].Addr;
		ProcCdo.BufLen = (ProcList->ProcData[ProcIndex + 1U].Addr -
			ProcList->ProcData[ProcIndex].Addr) / XPLMI_WORD_LEN;
		ProcCdo.CdoLen = ProcCdo.BufLen;
		/* Execute Proc */
		Status = XPlmi_ProcessCdo(&ProcCdo);
	} else {
		/* Return an error if the received ProcId is not valid */
		Status = (int)XPLMI_PROCID_NOT_VALID;
		XPlmi_Printf(DEBUG_GENERAL, "Invalid proc ID received\r\n");
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function copies the proc data to reserved PSM RAM when proc
 *          command is received.
 *
 * @param	Cmd is pointer to the command structure
 *              Command payload parameters are
 *              - ProcId
 *              - Data
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
static int XPlmi_Proc(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 ProcId;
	u8 Index = 0U;
	u32 SrcAddr;
	u32 LenDiff = 0U;
	u32 CmdLenInBytes = (Cmd->Len - 1U) * XPLMI_WORD_LEN;
	u32 CurrPayloadLen;
	XPlmi_ProcList *ProcList = NULL;

	if (Cmd->ProcessedLen == 0U) {
		/*
		 * Check if LPD is initialized as we are using PSM RAM to store
		 * Proc data
		 */
		if ((LpdInitialized & LPD_INITIALIZED) != LPD_INITIALIZED) {
			XPlmi_Printf(DEBUG_GENERAL, "LPD is not initialized, "
					"Proc commands cannot be stored\r\n");
			Status = (int)XPLMI_ERR_PROC_LPD_NOT_INITIALIZED;
			goto END;
		}

		ProcId = Cmd->Payload[0U];
		/* Get the proc list */
		ProcList = XPlmi_GetProcList();
		/* Check if received ProcId is already in memory */
		ProcList->ProcData[ProcList->ProcCount].Id = ProcId;
		while (ProcList->ProcData[Index].Id != ProcId) {
			Index++;
		}
		if (Index < ProcList->ProcCount) {
			/*
			 * Get Length difference if received proc length is greater than
			 * length of the proc in memory
			 * This is to check if new proc fits in available memory
			 */
			if (CmdLenInBytes > (ProcList->ProcData[Index + 1U].Addr -
					ProcList->ProcData[Index].Addr)) {
				LenDiff = CmdLenInBytes - (ProcList->ProcData[Index + 1U].Addr -
						ProcList->ProcData[Index].Addr);
			}
			/* Check if new proc length fits in the proc allocated memory */
			if ((LenDiff + (ProcList->ProcData[ProcList->ProcCount].Addr -
					ProcList->ProcData[ProcList->ProcCount - 1U].Addr)) >
					XPLMI_PROC_LOCATION_LENGTH) {
				Status = (int)XPLMI_UNSUPPORTED_PROC_LENGTH;
				goto END;
			}

			/*
			 * If proc command is received for existing proc,
			 * move other procs
			 */
			Status = XPlmi_MoveProc(Index, ProcList);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			--ProcList->ProcCount;
		} else {
			/* Check if proc list is full */
			if (ProcList->ProcCount == XPLMI_MAX_PROCS_SUPPORTED) {
				Status = (int)XPLMI_MAX_PROC_COMMANDS_RECEIVED;
				goto END;
			}
		}

		/* New proc address where the proc data need to be copied */
		Cmd->ResumeData[0U] = ProcList->ProcData[ProcList->ProcCount].Addr;

		/* Check if new proc length fits in the proc allocated memory */
		if ((Cmd->ResumeData[0U] + CmdLenInBytes) > (XPLMI_PROC_LOCATION_ADDRESS +
				XPLMI_PROC_LOCATION_LENGTH)) {
			Status = (int)XPLMI_UNSUPPORTED_PROC_LENGTH;
			goto END;
		}
		SrcAddr = (u32)(&Cmd->Payload[1U]);
		CurrPayloadLen = Cmd->PayloadLen - 1U;

		/* Add an entry in ProcList */
		ProcList->ProcData[ProcList->ProcCount].Id = ProcId;
		ProcList->ProcCount++;
		ProcList->ProcData[ProcList->ProcCount].Addr =
				ProcList->ProcData[ProcList->ProcCount - 1U].Addr + CmdLenInBytes;
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
 * @brief	This function checks if the IPI command is accessible or not
 *
 * @param	CmdId is the Command ID
 * @param	IpiReqType is the IPI command request type
 *
 * @return	XST_SUCCESS on success and XST_FAILURE on failure
 *
 *****************************************************************************/
static int XPlmi_CheckIpiAccess(u32 CmdId, u32 IpiReqType)
{
	int Status = XST_FAILURE;
	u32 ModuleCmdId = CmdId & XPLMI_PLM_GENERIC_CMD_ID_MASK;

	/* Secure check for PLMI IPI commands */
	switch (ModuleCmdId) {
		/*
		 * Check IPI request type for Event Logging IPI command
		 * and allow access only if the request is secure
		 */
		case XPLMI_PLM_GENERIC_EVENT_LOGGING_VAL:
			if (XPLMI_CMD_SECURE == IpiReqType) {
				Status = XST_SUCCESS;
			}
			break;

		/* Allow access for all other IPI commands */
		default:
			Status = XST_SUCCESS;
			break;
	}

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
 * @return	None
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
	};

	XPlmi_Generic.Id = XPLMI_MODULE_GENERIC_ID;
	XPlmi_Generic.CmdAry = XPlmi_GenericCmds;
	XPlmi_Generic.CmdCnt = XPLMI_ARRAY_SIZE(XPlmi_GenericCmds);
	XPlmi_Generic.CheckIpiAccess = XPlmi_CheckIpiAccess;

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
	const XPlmi_ReadBackProps *ReadBackPtr = XPlmi_GetReadBackPropsInstance();

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
int XPlmi_SetReadBackProps(const XPlmi_ReadBackProps *ReadBack)
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
	u64 BaseAddr = DestAddr;
	u32 RemData;
	u64 Src = SrcAddr;
	u32 LenTmp = Len;
	XPlmi_KeyHoleXfrParams KeyHoleXfrParams;

	KeyHoleXfrParams.SrcAddr = Src;
	KeyHoleXfrParams.DestAddr = DestAddr;
	KeyHoleXfrParams.BaseAddr = BaseAddr;
	KeyHoleXfrParams.Len = LenTmp * XPLMI_WORD_LEN;
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
 * @return	XST_SUCCESS on success and error code on failure
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

/**
 * @}
 * @endcond
 */

 /** @} */
