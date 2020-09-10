/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_event_logging.c
*
* This is the file which contains event logging related code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ma   02/18/2020 Initial release
* 1.01  ma   02/21/2020 Added support for retrieve buffer information
* 						event logging command through IPI
*       ma   03/02/2020 Added support for logging trace events
*       bsv  04/04/2020 Code clean up
* 1.02  bm   08/19/2020 Added ImageInfo Table and related APIs
*       bm   09/08/2020 Updated RunTime Configuration Registers in
*                       XPlmi_StoreImageInfo
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_event_logging.h"
#include "xplmi_dma.h"
#include "xplmi_debug.h"
#include "xplmi_hw.h"
#include "xplmi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/* Image Info Table Max Count*/
#define XPLMI_IMAGE_INFO_TBL_MAX_NUM	(XPLMI_IMAGE_INFO_TBL_BUFFER_LEN / \
						sizeof(XPlmi_ImageInfo))

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XPlmi_LogInfo DebugLog = {
	.LogBuffer.StartAddr = XPLMI_DEBUG_LOG_BUFFER_ADDR,
	.LogBuffer.Len = XPLMI_DEBUG_LOG_BUFFER_LEN,
	.LogBuffer.CurrentAddr = XPLMI_DEBUG_LOG_BUFFER_ADDR,
	.LogBuffer.IsBufferFull = FALSE,
	.LogLevel = (u8)XPlmiDbgCurrentTypes,
};

/* Trace log buffer */
XPlmi_CircularBuffer TraceLog = {
	.StartAddr = XPLMI_TRACE_LOG_BUFFER_ADDR,
	.Len = XPLMI_TRACE_LOG_BUFFER_LEN,
	.CurrentAddr = XPLMI_TRACE_LOG_BUFFER_ADDR,
	.IsBufferFull = FALSE,
};

/* Image Info Table */
XPlmi_ImageInfoTbl ImageInfoTbl = {
	.TblPtr = (XPlmi_ImageInfo *)XPLMI_IMAGE_INFO_TBL_BUFFER_ADDR,
	.Count = 0U,
	.IsBufferFull = FALSE,
};

/*****************************************************************************/
/**
 * @brief	This function retrieves remaining buffer data to the destination. location
 *
 * @param 	SourceAddr from where the buffer data is read
 * @param 	DestAddr to which the buffer data is copied
 * @param 	Len of data to be copied
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_RetrieveRemBytes(u64 SourceAddr, u64 DestAddr, u64 Len)
{
	u32 RemLen;
	u32 Index;

	RemLen = Len & (XPLMI_WORD_LEN - 1U);
	for (Index = 0U; Index < RemLen; ++Index) {
		XPlmi_OutByte64((DestAddr + (Len & ~(XPLMI_WORD_LEN - 1U)) + Index),
		XPlmi_InByte64((SourceAddr + (Len & ~(XPLMI_WORD_LEN - 1U)) + Index)));
	}
}

/*****************************************************************************/
/**
 * @brief	This function retrieves buffer data to the destination location.
 *
 * @param 	Buffer Circular buffer structure to which the data should be written
 * @param 	DestAddr to which the buffer data is to be copied
 *
 * @return	None
 *
 *****************************************************************************/
static void XPlmi_RetrieveBufferData(XPlmi_CircularBuffer * Buffer, u64 DestAddr)
{
	u32 Len;

	if (Buffer->IsBufferFull== TRUE) {
		Len = ((Buffer->StartAddr + Buffer->Len) - Buffer->CurrentAddr);
		XPlmi_DmaXfr(Buffer->CurrentAddr, DestAddr, (Len / XPLMI_WORD_LEN),
			XPLMI_PMCDMA_0);
		/* Retrieve remaining bytes */
		XPlmi_RetrieveRemBytes(Buffer->CurrentAddr, DestAddr, Len);
		XPlmi_DmaXfr(Buffer->StartAddr, (DestAddr+Len),
			((Buffer->Len - Len) / XPLMI_WORD_LEN), XPLMI_PMCDMA_0);
		/* Retrieve remaining bytes */
		XPlmi_RetrieveRemBytes(Buffer->StartAddr, (DestAddr+Len),
				(Buffer->Len - Len));
	} else {
		XPlmi_DmaXfr(Buffer->StartAddr, DestAddr,
			(Buffer->Len / XPLMI_WORD_LEN), XPLMI_PMCDMA_0);
		/* Retrieve remaining bytes */
		XPlmi_RetrieveRemBytes(Buffer->StartAddr, DestAddr, Buffer->Len);
	}
}

/*****************************************************************************/
/**
 * @brief	This function provides Event Logging command execution.
 *			Command payload parameters are
 *	*		Sub command
 *	*		1 - Configure print log level
 *	*			@Arg1 - Log Level
 *	*		2 - Configure Debug Log buffer memory
 *	*			@Arg1 - High Address
 *	*			@Arg2 - Low Address
 *	*			@Arg3 - Length
 *	*		3 - Retrieve Debug Log buffer
 *	*			@Arg1 - High Address
 *	*			@Arg2 - Low Address
 *	*		4 - Retrieve Debug Log buffer information
 *	*		5 - Configure Trace Log buffer memory
 *	*			@Arg1 - High Address
 *	*			@Arg2 - Low Address
 *	*			@Arg3 - Length
 *	*		6 - Retrieve Trace Log buffer
 *	*			@Arg1 - High Address
 *	*			@Arg2 - Low Address
 *	*		7 - Retrieve Trace Log buffer information
 *
 * @param	Pointer to the command structure

 * @return	Returns the Status of Event Logging API
 *
 *****************************************************************************/
int XPlmi_EventLogging(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	u32 LoggingCmd = Cmd->Payload[0U];
	u32 Arg1 = Cmd->Payload[1U];
	u32 Arg2 = Cmd->Payload[2U];
	u32 Arg3 = Cmd->Payload[3U];
	u64 Addr;

	switch (LoggingCmd) {
		case XPLMI_LOGGING_CMD_CONFIG_LOG_LEVEL:
			if (Arg1 <= XPlmiDbgCurrentTypes) {
				DebugLog.LogLevel = Arg1;
				Status = XST_SUCCESS;
			} else {
				Status = XPLMI_ERR_INVALID_LOG_LEVEL;
			}
			break;
		case XPLMI_LOGGING_CMD_CONFIG_LOG_MEM:
			if (Arg3 != 0U) {
				Addr = (((u64)Arg1 << 32U) | Arg2);
				if (((Addr >= XPLMI_PMCRAM_BASEADDR) &&
						(Addr < XPLMI_DEBUG_LOG_BUFFER_ADDR)) ||
						((Addr >= XPAR_PSV_PMC_RAM_INSTR_CNTLR_S_AXI_BASEADDR) &&
						(Addr <= XPAR_PSV_PMC_RAM_DATA_CNTLR_S_AXI_HIGHADDR))) {
					Status = XPLMI_ERR_INVALID_LOG_BUF_ADDR;
				} else {
					DebugLog.LogBuffer.StartAddr = Addr;
					DebugLog.LogBuffer.CurrentAddr = Addr;
					DebugLog.LogBuffer.Len = Arg3;
					DebugLog.LogBuffer.IsBufferFull = FALSE;
					Status = XST_SUCCESS;
				}
			} else {
				Status = XPLMI_ERR_INVALID_LOG_BUF_LEN;
			}
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_LOG_DATA:
			XPlmi_RetrieveBufferData(&DebugLog.LogBuffer,
				(((u64)Arg1 << 32U) | Arg2));
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_LOG_BUFFER_INFO:
			Cmd->Response[1U] = DebugLog.LogBuffer.StartAddr >> 32U;
			Cmd->Response[2U] = DebugLog.LogBuffer.StartAddr & 0xFFFFFFFFU;
			Cmd->Response[3U] = (DebugLog.LogBuffer.CurrentAddr -
					DebugLog.LogBuffer.StartAddr);
			Cmd->Response[4U] = DebugLog.LogBuffer.Len;
			Cmd->Response[5U] = DebugLog.LogBuffer.IsBufferFull;
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_CONFIG_TRACE_MEM:
			if (Arg3 != 0U) {
				Addr = (((u64)Arg1 << 32U) | Arg2);
				if (((Addr >= XPLMI_PMCRAM_BASEADDR) &&
					(Addr < XPLMI_TRACE_LOG_BUFFER_ADDR)) ||
					((Addr >= XPAR_PSV_PMC_RAM_INSTR_CNTLR_S_AXI_BASEADDR) &&
					(Addr <= XPAR_PSV_PMC_RAM_DATA_CNTLR_S_AXI_HIGHADDR))) {
					Status = XPLMI_ERR_INVALID_LOG_BUF_ADDR;
				} else {
					TraceLog.StartAddr = Addr;
					TraceLog.CurrentAddr = Addr;
					TraceLog.Len = Arg3;
					TraceLog.IsBufferFull = FALSE;
					Status = XST_SUCCESS;
				}
			} else {
				Status = XPLMI_ERR_INVALID_LOG_BUF_LEN;
			}
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_DATA:
			XPlmi_RetrieveBufferData(&TraceLog, (((u64)Arg1 << 32U) | Arg2));
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_BUFFER_INFO:
			Cmd->Response[1U] = TraceLog.StartAddr >> 32U;
			Cmd->Response[2U] = TraceLog.StartAddr & 0xFFFFFFFFU;
			Cmd->Response[3U] = (u32)(TraceLog.CurrentAddr -
					TraceLog.StartAddr);
			Cmd->Response[4U] = TraceLog.Len;
			Cmd->Response[5U] = TraceLog.IsBufferFull;
			Status = XST_SUCCESS;
			break;
		default:
			XPlmi_Printf(DEBUG_GENERAL,
				"Received invalid event logging command\n\r");
			Status = XST_INVALID_PARAM;
			break;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function stores the trace events to the Trace Log buffer.
 *
 * @param	TraceData to be stored to buffer
 * @param	Len is number of words in TraceData
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_StoreTraceLog(u32 *TraceData, u32 Len)
{
	u32 Index;
	XPlmi_PerfTime PerfTime = {0U};

	/* Get time stamp of PLM */
	XPlmi_MeasurePerfTime((((u64)(XPLMI_PIT1_RESET_VALUE) << 32U) |
		XPLMI_PIT2_RESET_VALUE), &PerfTime);

	TraceData[0U] = TraceData[0U] | (Len << XPLMI_TRACE_LOG_LEN_SHIFT);
	TraceData[1U] = (u32)PerfTime.TPerfMs;
	TraceData[2U] = (u32)PerfTime.TPerfMsFrac;

	for (Index = 0U; Index < Len; Index++) {
		if (TraceLog.CurrentAddr >=
				(TraceLog.StartAddr + TraceLog.Len)) {
			TraceLog.CurrentAddr = TraceLog.StartAddr;
			TraceLog.IsBufferFull = TRUE;
		}

		XPlmi_Out64(TraceLog.CurrentAddr, TraceData[Index]);
		TraceLog.CurrentAddr += XPLMI_WORD_LEN;
	}
}

/*****************************************************************************/
/**
 * @brief	This function returns the ImageInfoEntry by checking if an entry
 * exists for that paritcular ImgId in the ImgInfoTbl
 *
 * @param	ImgId of the the entry that has to be stored
 * @param	ImgIndex of the ImageEntry that is being returned
 *
 * @return	Address of ImageInfo Entry in the table
 *
 *****************************************************************************/
XPlmi_ImageInfo* XPlmi_GetImageInfoEntry(u32 ImgID, u32 *ImgIndex)
{
	XPlmi_ImageInfo *ImageEntry = NULL;
	u32 Index;

	if (ImgIndex == NULL) {
		goto END;
	}

	*ImgIndex = (u32)XPLMI_IMG_INDEX_NOT_FOUND;

	for (Index = 0U; Index < XPLMI_IMAGE_INFO_TBL_MAX_NUM; Index++) {
		/* Check if it's a existing valid Image Entry*/
		if (Index < ImageInfoTbl.Count) {
			if (ImageInfoTbl.TblPtr[Index].ImgID == ImgID) {
				ImageEntry = &ImageInfoTbl.TblPtr[Index];
				*ImgIndex = Index;
				break;
			}
		}
		else {
			ImageEntry = &ImageInfoTbl.TblPtr[Index];
			ImageEntry->ImgID = XPLMI_INVALID_IMG_ID;
			*ImgIndex = Index;
			break;
		}
	}

END:
	return ImageEntry;
}

/*****************************************************************************/
/**
 * @brief	This function stores the ImageInfo to Image Info Table
 *
 * @param	Pointer to ImageInfo that has to be written.
 *
 * @return	XST_SUCCESS when succesfully stored,
 *		XPLMI_ERR_IMAGE_INFO_TBL_OVERFLOW when buffer overflow occured
 *
 *****************************************************************************/
int XPlmi_StoreImageInfo(XPlmi_ImageInfo *ImageInfo)
{
	int Status = XST_FAILURE;
	XPlmi_ImageInfo *ImageEntry;
	u32 Index;
	u32 ChangeCount;
	u32 RtCfgLen;

	ImageEntry = XPlmi_GetImageInfoEntry(ImageInfo->ImgID, &Index);
	if (ImageEntry == NULL) {
		ImageInfoTbl.IsBufferFull = TRUE;
		Status = XPLMI_ERR_IMAGE_INFO_TBL_OVERFLOW;
		goto END;
	}

	(void)memcpy(ImageEntry, ImageInfo, sizeof(XPlmi_ImageInfo));
	if ((Index + 1U) > ImageInfoTbl.Count) {
		ImageInfoTbl.Count++;
	}

	/* Read ChangeCount and  increment it */
	ChangeCount = ((XPlmi_In32(XPLMI_RTCFG_IMGINFOTBL_LEN_ADDR) &
			XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_MASK)
			>> XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_SHIFT);
	ChangeCount++;

	/* Update ChangeCount and number of entries in the RunTime config register */
	RtCfgLen = (ImageInfoTbl.Count & XPLMI_RTCFG_IMGINFOTBL_NUM_ENTRIES_MASK);
	RtCfgLen |= ((ChangeCount << XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_SHIFT) &
				XPLMI_RTCFG_IMGINFOTBL_CHANGE_CTR_MASK);
	XPlmi_Out32(XPLMI_RTCFG_IMGINFOTBL_LEN_ADDR, RtCfgLen);

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function loads the imageinfo Table to given memory address
 *
 * @param	64 bit Destination Address
 * @param	Max Size of Buffer present at Destination Address
 * @param	NumEntries that are loaded from the Image Info Table
 *
 * @return	XST_SUCCESS when succesfully loaded,
 * 		XPLMI_ERR_IMAGE_INFO_TBL_OVERFLOW when buffer overflow occured.
 * 		XST_FAILURE when DmaXfr Failed
 *
 *****************************************************************************/
int XPlmi_LoadImageInfoTbl(u64 DestAddr, u32 MaxSize, u32 *NumEntries)
{
	int Status = XST_FAILURE;
	u32 Len = ImageInfoTbl.Count;
	u32 MaxLen = MaxSize / sizeof(XPlmi_ImageInfo);

	if (Len > MaxLen) {
		Len = MaxLen;
	}
	Status = XPlmi_DmaXfr((u64)XPLMI_IMAGE_INFO_TBL_BUFFER_ADDR, DestAddr,
			(Len * sizeof(XPlmi_ImageInfo)) / XPLMI_WORD_LEN,
			XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	if (ImageInfoTbl.IsBufferFull == TRUE) {
		Status = XPLMI_ERR_IMAGE_INFO_TBL_OVERFLOW;
		XPlmi_Printf(DEBUG_INFO, "Image Info Table Overflowed");
	}
	*NumEntries = Len;

END:
	return Status;
}
