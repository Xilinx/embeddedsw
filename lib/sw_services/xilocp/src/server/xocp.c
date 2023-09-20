/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp.c
*
* This file contains the implementation of the interface functions for DME
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   vns  06/26/22 Initial release
* 1.1   kal  01/05/23 Added PCR Extend and Pcr Logging functions
*       am   01/10/23 Modified function argument type to u64 in
*                     XOcp_GenerateDmeResponse().
* 1.2   kpt  06/02/23 Fixed circular buffer issues during HWPCR logging
*       kal  06/02/23 Added SW PCR extend and logging functions
*       yog  08/07/23 Replaced trng API calls using trngpsx driver
*
* </pre>
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xocp.h"
#include "xocp_keymgmt.h"
#include "xocp_hw.h"
#include "xplmi_hw.h"
#include "xplmi.h"
#include "xplmi_plat.h"
#include "xplmi_dma.h"
#include "xil_util.h"
#include "xsecure_sha384.h"
#include "xplmi_status.h"
#include "xsecure_init.h"
#include "xsecure_trng.h"
#include "xil_error_node.h"
#include "xplmi_err.h"
#include "xsecure_sha384.h"

/************************** Constant Definitions *****************************/
#define XOCP_SHA3_LEN_IN_BYTES		(48U) /**< Length of Sha3 hash in bytes */
#define XOCP_PCR_IN_BYTE1_OF_PLOAD_MASK	(0x0000FF00U) /**< Payload mask of PCR in byte 1 */
#define XOCP_PCR_IN_BYTE3_OF_PLOAD_MASK	(0xFF000000U) /**< Payload mask of PCR in byte 3 */
#define XOCP_DIGESTS_IN_BYTE0_PLOAD_MASK (0x000000FFU) /**< Payload mask of digests in byte 0 */
#define XOCP_DIGESTS_IN_BYTE2_PLOAD_MASK (0x00FF0000U) /**< Payload mask of digests in byte 2 */
#define XOCP_MEASUREIDX_IN_PLOAD_MASK (0x000000FFU) /**< Payload mask of measure id */
#define XOCP_HALF_WORD_SHIFT_LEN	(16U) /**< Used in the extraction of data of half word */
#define XOCP_SINGLE_BYTE_SHIFT		(8U) /**< To shift 8-bit */
#define XOCP_PAYLOAD_START_INDEX	(1U) /**< Start index of payload */
#define XOCP_DOUBLE_NUM_OF_WORDS	(2U) /**< To double number of words */
#define XOCP_XPPU_MAX_APERTURES         (19U) /**< Maximum XPPU apertures */
#define XOCP_XPPU_ENABLED               (0xFFFFFFFFU) /**< XPPU enabled */
#define XOCP_XPPU_MASTER_ID_0           (17U) /**< XPPU master id 0 */
#define XOCP_XPPU_MASTER_ID_1           (18U) /**< XPPU master id 1 */
#define XOCP_GET_ALL_PCR_MASK           (0x000000FFU) /**< All PCR read mask */
#define XOCP_HW_PCR                     (0x0U)  /**< HW PCR type */
#define XOCP_SW_PCR                     (0x1U)  /**< SW PCR type */
#define XOCP_SW_PCR_NUM_FOR_ROM_DIGEST  (0U)    /**< SW PCR number to extend ROM digest */
#define XOCP_SW_PCR_NUM_FOR_PLM_DIGEST  (1U)    /**< SW PCR number to extend PLM digest */
#define XOCP_SW_PCR_MEASUREMENT_IDX_ROM_PLM     (0U) /**< Measurement index for ROM and PLM extension */

/**************************** Type Definitions *******************************/
static XOcp_DmeXppuCfg XOcp_DmeXppuCfgTable[XOCP_XPPU_MAX_APERTURES] =
{
	{PMC_XPPU_APERPERM_017, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC Global register space */
	{PMC_XPPU_APERPERM_018, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC Global register space */
	{PMC_XPPU_APERPERM_019, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC Global register space */
	{PMC_XPPU_APERPERM_020, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC Global register space */
	{PMC_XPPU_APERPERM_021, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC TAP */
	{PMC_XPPU_APERPERM_026, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC TAP */
	{PMC_XPPU_APERPERM_027, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PMC DMA0 */
	{PMC_XPPU_APERPERM_028, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* AES */
	{PMC_XPPU_APERPERM_030, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* ECDSA RSA */
	{PMC_XPPU_APERPERM_032, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* SHA0 */
	{PMC_XPPU_APERPERM_033, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* TRNG */
	{PMC_XPPU_APERPERM_035, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* EFUSE CACHE */
	{PMC_XPPU_APERPERM_037, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* CRP */
	{PMC_XPPU_APERPERM_038, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* PPU1 RAM */
	{PMC_XPPU_APERPERM_386, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL, 0U, 0U},
	/* Configure [23:16] bits of Aperture_049 address */
	{PMC_XPPU_DYNAMIC_RECONFIG_APER_ADDR, XOCP_XPPU_DYNAMIC_RECONFIG_APER_SET_VALUE, 0U, 0U},
	/* Configure PPU0 to enable reconfiguration and PPU1 to configure XPPU registers after DME opertaion */
	{PMC_XPPU_DYNAMIC_RECONFIG_APER_PERM, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL, 0U, 0U},
	/* MASTER ID 00 */
	{PMC_XPPU_MASTER_ID00, XOCP_XPPU_MASTER_ID0_PPU0_CONFIG_VAL, 0U, 0U},
	/* MASTER ID 01 */
	{PMC_XPPU_MASTER_ID01, XOCP_XPPU_MASTER_ID1_PPU1_CONFIG_VAL, 0U, 0U},
};

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int XOcp_UpdateHwPcrLog(XOcp_HwPcr PcrNum, u64 ExtHashAddr, u32 DataSize);
static u32 XOcp_CountNumOfOnesInWord(u32 Num);
static int XOcp_DataMeasurement(u32 DigestIdx, u8 *Hash);
static u32 XOcp_GetPcrOffsetInLog(u32 PcrNum);
static XOcp_SwPcrStore *XOcp_GetSwPcrInstance(void);
static XOcp_SwPcrConfig *XOcp_GetSwPcrConfigInstance(void);
static void XOcp_PrintData(const u8 *Data, u32 Size, char *Str, u32 LogLevel);
static int XOcp_CalculateSwPcr(u32 PcrNum, u8 *ExtendedHash);
static int XOcp_DigestMeasurementAndUpdateLog(u32 PcrNum);
static int XOcp_StoreNoOfDigestPerPcr(u32 PcrNum, u32 NumOfDigests);
static int XOcp_StoreEventIdConfig(u32 *Pload, u32 CurrIdx, u32 DigestCount, u32 Len);
static int XOcp_ClearDigestData(u32 PcrNum);
static void XOcp_DmeStoreXppuDefaultConfig(void);
static void XOcp_DmeRestoreXppuDefaultConfig(void);
static int XOcp_GetPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize, u32 PcrType);

/************************** Variable Definitions *****************************/
/**< HW PCR log struture */
static XOcp_HwPcrLog HwPcrLog = {0U};

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function updates the index of HWPCR log
 *
 * @param	HwPcrIndex pointer to the index to be incremented
 * @param  	Val        Increment value
 *
 ******************************************************************************/
static INLINE void XOcp_UpdateHwPcrIndex(u32 *HwPcrIndex, u32 Val)
{
	*HwPcrIndex += Val;
	if (*HwPcrIndex >= XOCP_MAX_NUM_OF_HWPCR_EVENTS) {
		*HwPcrIndex = 0U;
	}
}

/*****************************************************************************/
/**
 * @brief	This function extends the PCR with provided hash by requesting
 * 		ROM service
 *
 * @param	PcrNum is the variable of enum XOcp_HwPcr to select the PCR
 * 		to be extended.
 * @param	ExtHashAddr is the address of the buffer which holds the hash
 *		to extended.
 * @param 	DataSize Data Size to be extended
 *
 * @return
 *		- XST_SUCCESS - If PCR extend is success
 *		- XST_INVALID_PARAM - On invalid input parameter
 *		- XOCP_PCR_ERR_NOT_COMPLETED - HW PCR operation is not done
 *		- XOCP_PCR_ERR_OPERATION - Error in HW PCR operation
 *		- XOCP_PCR_ERR_IN_UPDATE_LOG - Error in HW PCR log update
 *
 ******************************************************************************/
int XOcp_ExtendHwPcr(XOcp_HwPcr PcrNum, u64 ExtHashAddr, u32 DataSize)
{
	volatile int Status = XST_FAILURE;
	u32 RegValue;

	if ((PcrNum < XOCP_PCR_2) || (PcrNum > XOCP_PCR_7)) {
		Status = (int)XOCP_PCR_ERR_PCR_SELECT;
		goto END;
	}

	if (DataSize != XOCP_PCR_SIZE_BYTES) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Status = XOcp_MemCopy(ExtHashAddr, XOCP_PMC_GLOBAL_PCR_EXTEND_INPUT_0,
					XOCP_PCR_SIZE_WORDS, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		if (Status == XST_SUCCESS) {
			Status = (int)XST_GLITCH_ERROR;
		}
		goto END;
	}

	XPlmi_Out32(XOCP_PMC_GLOBAL_PCR_OP,
			(u32)PcrNum << XOCP_PMC_GLOBAL_PCR_OP_IDX_SHIFT);
	Status = XPlmi_RomISR(XPLMI_PCR_OP);
	if (Status != XST_SUCCESS) {
		Status = (int)XOCP_PCR_ERR_OPERATION;
		goto END;
	}
	/* Check PCR extend status */
	RegValue = XPlmi_In32(XOCP_PMC_GLOBAL_PCR_OP_STATUS);
	if ((RegValue & XOCP_PMC_GLOBAL_PCR_OP_STATUS_DONE_MASK) == 0x0U) {
		Status = (int)XOCP_PCR_ERR_NOT_COMPLETED;
	}
	if ((RegValue & XOCP_PMC_GLOBAL_PCR_OP_STATUS_ERROR_MASK) != 0x0U) {
		Status = (int)XOCP_PCR_ERR_OPERATION;
	}

	if (Status == XST_SUCCESS) {
		/* Update HwPcr Log */
		Status = XOcp_UpdateHwPcrLog(PcrNum, ExtHashAddr, DataSize);
		if (Status != XST_SUCCESS) {
			Status = (int)XOCP_PCR_ERR_IN_UPDATE_LOG;
		}
	}

END:
	return Status;
}



/*****************************************************************************/
/**
 * @brief	This function reads the HWPCR log and info into the user provided
 * 		buffer.
 *
 * @param	HwPcrEventsAddr  Pointer to the XOcp_HwPcrEvent
 * @param	HwPcrLogInfoAddr Pointer to the XOcp_HwPcrLogInfo
 * @param	NumOfLogEntries	 Maximum number of log entries to be read
 *
 * @return
 *		- XST_SUCCESS - If log read is successful
 *		- XOCP_PCR_ERR_INVALID_LOG_READ_REQUEST - Invalid HW PCR read request
 *		- XST_FAILURE - Upon any other failure
 ******************************************************************************/
int XOcp_GetHwPcrLog(u64 HwPcrEventsAddr, u64 HwPcrLogInfoAddr, u32 NumOfLogEntries)
{
	int Status = XST_FAILURE;
	u32 ReqHwPcrLogEntries = NumOfLogEntries;
	u32 RemHwPcrLogEvents = 0U;
	u32 TotalRdHwPcrLogEvents = 0U;

	if (ReqHwPcrLogEntries > XOCP_MAX_NUM_OF_HWPCR_EVENTS) {
		Status = (int)XOCP_PCR_ERR_INVALID_LOG_READ_REQUEST;
		goto END;
	}

	if ((HwPcrLog.LogInfo.RemainingHwPcrEvents == 0U) || (ReqHwPcrLogEntries == 0U)) {
		Status = XST_SUCCESS;
		goto END1;
	}

	if (ReqHwPcrLogEntries > HwPcrLog.LogInfo.RemainingHwPcrEvents) {
		ReqHwPcrLogEntries = HwPcrLog.LogInfo.RemainingHwPcrEvents;
	}

	TotalRdHwPcrLogEvents = ReqHwPcrLogEntries;
	/*
	 * From current TailIndex if number of entries are more than XOCP_MAX_NUM_OF_HWPCR_EVENTS
	 * then copy log entries from TailIndex to XOCP_MAX_NUM_OF_HWPCR_EVENTS and update
	 * log entries and TailIndex.
	 */
	if ((HwPcrLog.TailIndex + ReqHwPcrLogEntries) >= XOCP_MAX_NUM_OF_HWPCR_EVENTS) {
		RemHwPcrLogEvents = XOCP_MAX_NUM_OF_HWPCR_EVENTS - HwPcrLog.TailIndex;
		Status = XPlmi_MemCpy64(HwPcrEventsAddr, (u64)(UINTPTR)&HwPcrLog.Buffer[HwPcrLog.TailIndex],
				(RemHwPcrLogEvents * sizeof(XOcp_HwPcrEvent)));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		/* Update HWPCR events and log entries to handle remaining entries */
		HwPcrLog.LogInfo.RemainingHwPcrEvents -= RemHwPcrLogEvents;
		ReqHwPcrLogEntries -= RemHwPcrLogEvents;
		HwPcrEventsAddr += (u64)RemHwPcrLogEvents * sizeof(XOcp_HwPcrEvent);
		HwPcrLog.TailIndex = 0U;
    }

	if (ReqHwPcrLogEntries != 0U) {
		Status = XPlmi_MemCpy64(HwPcrEventsAddr, (u64)(UINTPTR)&HwPcrLog.Buffer[HwPcrLog.TailIndex],
			(ReqHwPcrLogEntries * sizeof(XOcp_HwPcrEvent)));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		HwPcrLog.LogInfo.RemainingHwPcrEvents -= ReqHwPcrLogEntries;
		XOcp_UpdateHwPcrIndex(&HwPcrLog.TailIndex, ReqHwPcrLogEntries);
	}
END1:
	/* Update current HWPCR log status */
	HwPcrLog.LogInfo.HwPcrEventsRead = TotalRdHwPcrLogEvents;
	Status = XPlmi_MemCpy64(HwPcrLogInfoAddr, (u64)(UINTPTR)&HwPcrLog.LogInfo,
				sizeof(XOcp_HwPcrLogInfo));
	if (Status != XST_SUCCESS) {
		goto END;
	}
	HwPcrLog.LogInfo.OverflowCntSinceLastRd = 0U;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function gets the PCR value from requested PCRs.
 *
 * @param	PcrMask - Mask to tell which PCRs to read
 * @param	PcrBuf is the address of the 48 bytes buffer to store the
 * 		requested PCR contents
 * @param	PcrBufSize is the Size of the PCR buffer provided
 *
 * @return
 *		- XST_SUCCESS - If PCR contents are copied
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
int XOcp_GetHwPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize)
{
	return XOcp_GetPcr(PcrMask, PcrBuf, PcrBufSize, XOCP_HW_PCR);
}

/*****************************************************************************/
/**
 * @brief	This function gets the PCR value from requested SW PCR.
 *
 * @param	PcrMask Mask to tell which PCRs to read
 * @param	PcrBuf 	Address of the 48 bytes buffer to store the
 * 		requested PCR contents
 * @param	PcrBufSize Size of the PCR buffer provided
 *
 * @return
 *		- XST_SUCCESS - If PCR contents are copied
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
int XOcp_GetSwPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize)
{
	return XOcp_GetPcr(PcrMask, PcrBuf, PcrBufSize, XOCP_SW_PCR);
}

/*****************************************************************************/
/**
 * @brief	This function gets the PCR value from requested SW PCR/HW PCR.
 *
 * @param	PcrMask Mask to tell which PCRs to read
 * @param	PcrBuf 	Address of the 48 bytes buffer to store the
 * 		requested PCR contents
 * @param	PcrBufSize Size of the PCR buffer provided
 * @param	PcrType	HW PCR or SW PCR
 *
 * @return
 *		- XST_SUCCESS - If PCR contents are copied
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
static int XOcp_GetPcr(u32 PcrMask, u64 PcrBuf, u32 PcrBufSize, u32 PcrType)
{
	int Status = XST_FAILURE;
	u8 ExtendedHash[XOCP_PCR_HASH_SIZE_IN_BYTES] = {0U};
	u32 NumOfBitsSetInMask;
	u32 Mask = PcrMask;
	u32 PcrOffset = 0U;
	u32 BufOffset = 0U;
	u32 PcrNum = 0U;

	NumOfBitsSetInMask = XOcp_CountNumOfOnesInWord(Mask);
	if (PcrBufSize < (NumOfBitsSetInMask * XOCP_PCR_SIZE_BYTES)) {
		Status = (int)XOCP_PCR_ERR_INSUFFICIENT_BUF_MEM;
		goto END;
	}
	if (NumOfBitsSetInMask == 0U) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if (Mask > XOCP_GET_ALL_PCR_MASK) {
		Status = XOCP_PCR_ERR_PCR_SELECT;
		goto END;
	}

	while (Mask != 0x0U) {
		if ((Mask & 0x01U) != 0U) {
			if (PcrType == XOCP_SW_PCR) {
				Status = XOcp_CalculateSwPcr(PcrNum, ExtendedHash);
				if (Status != XST_SUCCESS) {
					break;
				}
				PcrOffset = (UINTPTR)&ExtendedHash;
			}
			else {
				PcrOffset = XOCP_PMC_GLOBAL_PCR_0_0 + (u32)PcrNum * XOCP_PCR_SIZE_BYTES;
				if (PcrOffset > XOCP_PMC_GLOBAL_PCR_7_0) {
					Status = (int)XOCP_PCR_ERR_PCR_SELECT;
					goto END;
				}
			}
			Status = XOcp_MemCopy(PcrOffset,
					PcrBuf + BufOffset,
					XOCP_PCR_SIZE_WORDS, XPLMI_PMCDMA_0);
			if (Status != XST_SUCCESS) {
				break;
			}
			BufOffset += XOCP_PCR_SIZE_BYTES;
		}
		Mask = Mask >> 1U;
		PcrNum++;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function extends the SW PCR with the provided data and also
 * 		stores the data into SW PCR log.
 *
 * @param	PcrNum 		To which SwPcr data needs to be extended
 * @param	MeasurementIdx	Position in which order the data has to be
 * 				extended
 * @param	DataAddr 	Address where the data to be extended is stored
 * @param 	DataSize 	Size of the data to be extended.
 *				If the data size exceeds the 48 bytes, the data
 *				pointer is stored and its caller responsibility
 *				to retain this data till lifetime of the PCR extended.
 *				Otherwise data is copied to internal PCR buffer.
 *		OverWrite	TRUE or FALSE
 *
 * @return
 *		- XST_SUCCESS - Upon success
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
int XOcp_ExtendSwPcr(u32 PcrNum, u32 MeasurementIdx, u64 DataAddr, u32 DataSize, u32 OverWrite)
{
	int Status = XST_FAILURE;
	u8 DataBlobHash[XOCP_PCR_HASH_SIZE_IN_BYTES] = {0U};
	XOcp_SwPcrStore *SwPcr = XOcp_GetSwPcrInstance();
	XOcp_SwPcrConfig *SwPcrConfig = XOcp_GetSwPcrConfigInstance();
	u32 DigestIdxInLog = 0U;

	/* If SW PCR number is more than 7, throw and error */
	if (PcrNum > (u32)XOCP_PCR_7) {
		Status = (int)XOCP_PCR_ERR_PCR_SELECT;
		goto END;
	}

	/* If SW PCR config is not received before extend request, throw an error */
	if (SwPcrConfig->IsPcrConfigReceived == FALSE) {
		Status = (int)XOCP_PCR_ERR_SWPCR_CONFIG_NOT_RECEIVED;
		goto END;
	}

	/* If MeasurementIdx is greater than number of digests configured, throw an error */
	if (MeasurementIdx >= SwPcrConfig->DigestsForPcr[PcrNum]) {
		Status = (int)XOCP_PCR_ERR_MEASURE_IDX_SELECT;
		goto END;
	}

	/* Check if the DigestIdx calculated is not overflowing */
	DigestIdxInLog = XOcp_GetPcrOffsetInLog(PcrNum) + MeasurementIdx;
	if (DigestIdxInLog > XOCP_MAX_NUM_OF_SWPCRS) {
		Status = (int)XOCP_PCR_ERR_MEASURE_IDX_SELECT;
		goto END;
	}

	/* Validate the pdi type */
	if ((OverWrite != TRUE) &&
		(OverWrite != FALSE)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	/* If duplicate extend request, throw an error */
	if ((OverWrite == FALSE) &&
		(SwPcr->Data[DigestIdxInLog].IsReqExtended == TRUE)) {
		Status = (int)XOCP_PCR_ERR_SWPCR_DUP_EXTEND;
		goto END;
	}

	/*
	 * Clear Digest data if it is already extended,
	 * when OverWrite is TRUE.
	 */
	if ((OverWrite == TRUE) &&
		(SwPcr->Data[DigestIdxInLog].IsReqExtended == TRUE)) {
		Status = XOcp_ClearDigestData(PcrNum);
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	DigestIdxInLog = XOcp_GetPcrOffsetInLog(PcrNum) + MeasurementIdx;

	/* Store the SW PCR Extend request details to SW PCR Log */
	SwPcr->Data[DigestIdxInLog].Measurement.DataLength = DataSize;
	XPlmi_Printf_WoTS(DEBUG_INFO,
			"\r\nSwPcrNum: %x MeasurementIdx: %x DigestIdxInLog: %x\r\n",
			PcrNum, MeasurementIdx, DigestIdxInLog);

	if (DataSize > XOCP_PCR_HASH_SIZE_IN_BYTES) {
		SwPcr->Data[DigestIdxInLog].DataAddr = DataAddr;
	}
	else {
		Status = XPlmi_MemCpy64((u64)(UINTPTR)&SwPcr->Data[DigestIdxInLog].DataToExtend,
			DataAddr, DataSize);
		if (Status != XST_SUCCESS) {
		        goto END;
		}
	}

	XOcp_PrintData((const u8 *)&SwPcr->Data[DigestIdxInLog].Measurement.EventId,
			XOCP_EVENT_ID_NUM_OF_BYTES, "Event Id:", DEBUG_INFO);
	XOcp_PrintData((const u8 *)&SwPcr->Data[DigestIdxInLog].Measurement.Version,
			XOCP_VERSION_NUM_OF_BYTES, "Version:", DEBUG_INFO);
	XOcp_PrintData((const u8 *)SwPcr->Data[DigestIdxInLog].DataToExtend,
			DataSize, "Data to be extended:", DEBUG_INFO);

	/* Calculate and store the DataBlob Hash into Log, where DataBlob is
	 * (EventId || Version || Data to be Extended)
	 */
	Status = XOcp_DataMeasurement(DigestIdxInLog, DataBlobHash);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_MemCpy64((u64)(UINTPTR)SwPcr->Data[DigestIdxInLog].Measurement.HashOfData,
			(u64)(UINTPTR)&DataBlobHash, XOCP_PCR_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XOcp_PrintData((const u8 *)SwPcr->Data[DigestIdxInLog].Measurement.HashOfData,
			XOCP_PCR_HASH_SIZE_IN_BYTES, "Datablob Hash:", DEBUG_INFO);

	SwPcr->Data[DigestIdxInLog].IsReqExtended = TRUE;
	SwPcr->CountPerPcr[PcrNum] += 1U ;

	/* Send Notification to the subscriber about the log update */
	XPlmi_HandleSwError(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
                        XIL_EVENT_ERROR_PCR_LOG_UPDATE);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads the data extended to specified SW PCR
 * 		at specified measurement index.
 *
 * @param	Addr  64 bit address of the XOcp_SwPcrReadData
 *
 * @return
 *		- XST_SUCCESS - Upon success
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
int XOcp_GetSwPcrData(u64 Addr)
{
	int Status = XST_FAILURE;
	XOcp_SwPcrReadData Data;
	XOcp_SwPcrStore *SwPcr = XOcp_GetSwPcrInstance();
	XOcp_SwPcrConfig *SwPcrConfig = XOcp_GetSwPcrConfigInstance();
	u8 ReturnedBytesOffset = (u8)(u32)&(((XOcp_SwPcrReadData *)0)->ReturnedBytes);
	u32 ReturnedBytes;
	u32 DigestIdx;
	u32 CurrDataLen;

	Status = XPlmi_MemCpy64((u64)(UINTPTR)&Data, Addr, sizeof(Data));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* If SW PCR number is more than 7, throw and error */
	if (Data.PcrNum > (u32)XOCP_PCR_7) {
		Status = (int)XOCP_PCR_ERR_PCR_SELECT;
		goto END;
	}

	/* If MeasurementIdx is greater than number of digests
	 * configured, throw an error.
	 */
	if (Data.MeasurementIdx >= SwPcrConfig->DigestsForPcr[Data.PcrNum]) {
		Status = (int)XOCP_PCR_ERR_MEASURE_IDX_SELECT;
		goto END;
	}

	/* Calculate the Digest index in the log using SW PCR number
	 * and measurement index
	 */
	DigestIdx = XOcp_GetPcrOffsetInLog(Data.PcrNum) + Data.MeasurementIdx;
	CurrDataLen = SwPcr->Data[DigestIdx].Measurement.DataLength;
	if (Data.DataStartIdx > CurrDataLen - 1U) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}

	if (Data.BufSize < (CurrDataLen - Data.DataStartIdx)) {
		ReturnedBytes = Data.BufSize;
	} else {
		ReturnedBytes = CurrDataLen - Data.DataStartIdx;
	}

	/* Copy the number of returned bytes to user provided buffer address */
	Status = XPlmi_MemCpy64((u64)(UINTPTR)(Addr + (u64)ReturnedBytesOffset),
                                (u64)(UINTPTR)&ReturnedBytes, XOCP_WORD_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Copy the Data with DataStartIdx to the user provided buffer address */
	if (CurrDataLen > XOCP_PCR_HASH_SIZE_IN_BYTES) {
		Status = XPlmi_MemCpy64((u64)(UINTPTR)Data.BufAddr,
			(u64)(UINTPTR)SwPcr->Data[DigestIdx].DataAddr + Data.DataStartIdx,
			ReturnedBytes);
	} else {
		Status = XPlmi_MemCpy64((u64)(UINTPTR)Data.BufAddr,
			(u64)(UINTPTR)&SwPcr->Data[DigestIdx].DataToExtend[Data.DataStartIdx],
			ReturnedBytes);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads the SW PCR log of the specified PCR.
 * 		SW PCR log contains information about digests extended to all
 * 		the SW PCRs. Here the request is for one SW PCR.
 *
 * @param	Addr	64 bit address of the XOcp_SwPcrLogReadData
 *
 * @return
 *		- XST_SUCCESS - Upon success
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
int XOcp_GetSwPcrLog(u64 Addr)
{
	int Status = XST_FAILURE;
	XOcp_SwPcrStore *SwPcr = XOcp_GetSwPcrInstance();
	XOcp_SwPcrConfig *SwPcrConfig = XOcp_GetSwPcrConfigInstance();
	XOcp_SwPcrLogReadData Log;
	u8 DigestCountOffset = (u8)(u32)&(((XOcp_SwPcrLogReadData *)0)->DigestCount);
	u32 NumOfDigests;
	u32 PcrIdxInLog;
	u32 Index;
	u32 BufOffset = 0U;

	Status = XPlmi_MemCpy64((u64)(UINTPTR)&Log, Addr, sizeof(Log));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* If SW PCR number is more than 7, throw and error */
	if (Log.PcrNum > (u32)XOCP_PCR_7) {
		Status = (int)XOCP_PCR_ERR_PCR_SELECT;
		goto END;
	}

	NumOfDigests = SwPcr->CountPerPcr[Log.PcrNum];
	PcrIdxInLog = XOcp_GetPcrOffsetInLog(Log.PcrNum);

	if (Log.LogSize < (sizeof(XOcp_PcrMeasurement) * NumOfDigests)) {
		Status = (int)XOCP_PCR_ERR_INSUFFICIENT_BUF_MEM;
		goto END;
	}
	/* Calculate intermediate Hash values for all the SW PCRs in
	 * the log.
	 */
	Status = XOcp_DigestMeasurementAndUpdateLog(Log.PcrNum);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Copy number of digests extended to requested SW PCR into
	 * user provided buffer.
	 */
	Status = XPlmi_MemCpy64((u64)(UINTPTR)(Addr + (u64)DigestCountOffset),
				(u64)(UINTPTR)&NumOfDigests, XOCP_WORD_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Copy the SW PCR log for the requested PCR into user provided
	 * buffer.
	 */
	for(Index = PcrIdxInLog; Index < (PcrIdxInLog + SwPcrConfig->DigestsForPcr[Log.PcrNum]); Index++) {
		if (SwPcr->Data[Index].IsReqExtended == TRUE) {
			Status = XPlmi_MemCpy64((u64)(UINTPTR)Log.PcrLogAddr + BufOffset,
				(u64)(UINTPTR)&SwPcr->Data[Index].Measurement,
				sizeof(XOcp_PcrMeasurement));
			if (Status != XST_SUCCESS) {
				goto END;
			}
			BufOffset += sizeof(XOcp_PcrMeasurement);
		}
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function generates the response to DME challenge request and
 *		configures the XPPU for requesting DME service
 *		to ROM.
 *
 * @param	NonceAddr holds the address of 32 bytes buffer Nonce,
 *		which shall be used to fill one of the member of DME structure
 * @param	DmeStructResAddr is the address to the 224 bytes buffer,
 *		which is used to store the response to DME challenge request of
 *		type XOcp_DmeResponse.
 *
 * @return
 *		- XST_SUCCESS - Upon success and
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
int XOcp_GenerateDmeResponse(u64 NonceAddr, u64 DmeStructResAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u32 *DevIkPubKey = (u32 *)XOCP_PMC_GLOBAL_DEV_IK_PUBLIC_X_0;
	u8 Sha3Hash[XOCP_SHA3_LEN_IN_BYTES];
	XOcp_DmeResponse DmeResponse = {0U};
	XOcp_Dme *DmePtr = &DmeResponse.Dme;
	XTrngpsx_Instance *TrngInstance = NULL;
	u32 XppuEnabled = 0U;
	u32 Index;

	/*
	 * Check if XPPU_LOCK is enabled.
	 * Check if Dynamic reconfiguration is enabled by default.
	 */
	if ((Xil_In32(PMC_XPPU_LOCK) != PMC_XPPU_LOCK_DEFVAL) ||
		(Xil_In32(PMC_XPPU_DYNAMIC_RECONFIG_EN) != PMC_XPPU_DYNAMIC_RECONFIG_EN_DEFVAL)) {
		Status = XOCP_ERR_INVALID_XPPU_CONFIGURATION;
		goto RET;
	}

	/* Zeorizing the DME structure */
	Status = Xil_SMemSet((void *)DmePtr,
				sizeof(XOcp_Dme), 0U, sizeof(XOcp_Dme));
	if (Status != XST_SUCCESS) {
		goto RET;
	}

	/* Fill the DME structure's DEVICE ID field with hash of DEV IK Public key */
	if (XOcp_IsDevIkReady() != FALSE) {
		Status = XSecure_Sha384Digest((u8 *)(UINTPTR)DevIkPubKey,
				XOCP_SIZE_OF_ECC_P384_PUBLIC_KEY_BYTES, Sha3Hash);
		if (Status != XST_SUCCESS) {
			goto RET;
		}
		Status = Xil_SMemCpy((void *)DmePtr->DeviceID,
			 XOCP_SHA3_LEN_IN_BYTES,
			(const void *)Sha3Hash,
			 XOCP_SHA3_LEN_IN_BYTES,
			 XOCP_SHA3_LEN_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto RET;
		}
	}

	/* Fill the DME structure with Nonce */
	Status = XPlmi_MemCpy64((u64)(UINTPTR)DmePtr->Nonce, NonceAddr,
			XOCP_DME_NONCE_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto RET;
	}

	/* Store the XPPU registers initial configuration */
	XOcp_DmeStoreXppuDefaultConfig();

	for (Index = 0U; Index < XOCP_XPPU_MAX_APERTURES; Index++) {
		if (Index == XOCP_XPPU_MASTER_ID_0) {
			if (XOCP_XPPU_MASTER_ID0_PPU0_CONFIG_VAL == Xil_In32(PMC_XPPU_MASTER_ID00)) {
				XOcp_DmeXppuCfgTable[Index].IsModified = FALSE;
			}
		}
		else if (Index == XOCP_XPPU_MASTER_ID_1) {
			if (XOCP_XPPU_MASTER_ID1_PPU1_CONFIG_VAL == Xil_In32(PMC_XPPU_MASTER_ID01)) {
				XOcp_DmeXppuCfgTable[Index].IsModified = FALSE;
			}
		} else {
			Xil_Out32(XOcp_DmeXppuCfgTable[Index].IsModified, TRUE);
		}
		/* Configure the XPPU Apertures with configuration */
		Xil_Out32(XOcp_DmeXppuCfgTable[Index].XppuAperAddr,
				XOcp_DmeXppuCfgTable[Index].XppuAperWriteCfgVal);
	}

	/* Enabling Dynamic Reconfiguration */
	Xil_Out32(PMC_XPPU_DYNAMIC_RECONFIG_EN, PMC_XPPU_DYNAMIC_RECONFIG_EN_DEFVAL);
	/* XPPU */
	Xil_Out32(PMC_XPPU_APERPERM_049, XOCP_XPPU_EN_PPU0_APERPERM_CONFIG_VAL);

	/* If XPPU is not enabled, enable XPPU */
	if (((Xil_In32(PMC_XPPU_CTRL) & PMC_XPPU_CTRL_ENABLE_MASK)) == 0U) {
		Xil_SecureRMW32(PMC_XPPU_CTRL, PMC_XPPU_CTRL_ENABLE_MASK,
			XOCP_PMC_XPPU_CTRL_ENABLE_VAL);
		XppuEnabled = XOCP_XPPU_ENABLED;
	}

	/* Mention the Address and Size of DME structure for ROM service */
	XPlmi_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE5, (u32)DmePtr);
	XPlmi_Out32(PMC_GLOBAL_GLOBAL_GEN_STORAGE6, sizeof(XOcp_Dme));

	Status = XPlmi_RomISR(XPLMI_DME_CHL_SIGN_GEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Check if any ROM error occurred during DME request */
	Status = Xil_In32(PMC_GLOBAL_PMC_BOOT_ERR);
	if (Status != 0x0U) {
		Status = (int)XOCP_DME_ROM_ERROR;
		goto END;
	}
	/* Copy the contents to user DME response structure */
	Status = Xil_SMemCpy(DmeResponse.DmeSignatureR,
			XOCP_ECC_P384_SIZE_BYTES,
			(const void *)XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_R_0,
			XOCP_ECC_P384_SIZE_BYTES,
			XOCP_ECC_P384_SIZE_BYTES);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemCpy(DmeResponse.DmeSignatureS,
			XOCP_ECC_P384_SIZE_BYTES,
			(const void *)XOCP_PMC_GLOBAL_DME_CHALLENGE_SIGNATURE_S_0,
			XOCP_ECC_P384_SIZE_BYTES,
			XOCP_ECC_P384_SIZE_BYTES);

END:
	XOcp_DmeRestoreXppuDefaultConfig();

	if (XppuEnabled == XOCP_XPPU_ENABLED) {
		Xil_SecureRMW32(PMC_XPPU_CTRL, PMC_XPPU_CTRL_ENABLE_MASK,
				XOCP_PMC_XPPU_CTRL_DISABLE_VAL);
	}

	if (Status == XST_SUCCESS) {
		Status = XPlmi_MemCpy64(DmeStructResAddr, (u64)(UINTPTR)&DmeResponse,
			sizeof(DmeResponse));
		if (Status != XST_SUCCESS) {
			Status = Status | XOCP_DME_ERR;
		}
	}
RET:
	if (Status != XST_SUCCESS) {
		ClearStatus = XPlmi_MemSet((u64)(UINTPTR)&DmeResponse, 0U, sizeof(XOcp_DmeResponse));
		if (ClearStatus != XST_SUCCESS) {
			Status = Status | XLOADER_SEC_BUF_CLEAR_ERR;
		}
		else {
			Status = Status | XLOADER_SEC_BUF_CLEAR_SUCCESS;
		}
	}

	/*
	 * ROM uses TRNG for DME service and resets the core after the usage
	 * in this case TRNG state should be set to uninitialized state
	 * so that PLM can re-initialize during runtime requests.
	 */
	TrngInstance = XSecure_GetTrngInstance();
	if (TrngInstance->State != XTRNGPSX_UNINITIALIZED_STATE){
		SStatus = XTrngpsx_Uninstantiate(TrngInstance);
		if ((Status == XST_SUCCESS) && (Status == XST_SUCCESS)) {
			if(SStatus != XST_SUCCESS) {
				Status = SStatus | XOCP_DME_ERR;
			}
		}
		XSecure_UpdateTrngCryptoStatus(XSECURE_CLEAR_BIT);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function stores default XPPU aperture configuration
 *		before DME operation.
 *
 ******************************************************************************/
static void XOcp_DmeStoreXppuDefaultConfig(void)
{
	volatile u32 Index;

	for (Index = 0U; Index < XOCP_XPPU_MAX_APERTURES; Index++) {
		XOcp_DmeXppuCfgTable[Index].XppuAperReadCfgVal =
			Xil_In32(XOcp_DmeXppuCfgTable[Index].XppuAperAddr);
	}
}

/*****************************************************************************/
/**
 * @brief	This function restores default XPPU aperture configuration
 *		after DME operation.
 *
 ******************************************************************************/
static void XOcp_DmeRestoreXppuDefaultConfig(void)
{
	volatile u32 Index;

	/* Restore XPPU registers to their previous state */
	for (Index = 0U; Index < XOCP_XPPU_MAX_APERTURES; Index++) {
		if (XOcp_DmeXppuCfgTable[Index].IsModified == TRUE) {
			Xil_Out32(XOcp_DmeXppuCfgTable[Index].XppuAperAddr,
				XOcp_DmeXppuCfgTable[Index].XppuAperReadCfgVal);
		}
	}
	Xil_Out32(PMC_XPPU_APERPERM_049, XOCP_XPPU_EN_PPU0_PPU1_APERPERM_CONFIG_VAL);
	Xil_Out32(PMC_XPPU_DYNAMIC_RECONFIG_EN, 0x00);
}

/*****************************************************************************/
/**
 * @brief	This function parses and store the SW PCR config sent via CDO.
 *
 * @param	Pload 	Pointer to the Command Payload
 * @param	Len 	CDO payload length
 *
 * @return
 *		- XST_SUCCESS - Upon success
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
int XOcp_StoreSwPcrConfig(u32 *Pload, u32 Len)
{
	volatile int Status = XST_FAILURE;
	XOcp_SwPcrConfig *SwPcrConfig = XOcp_GetSwPcrConfigInstance();
	u32 PcrNum;
	u32 NoOfDigests;
	u32 TotalDigestCount = 0U;
	u32 NumOfPcrRecords = Pload[0U];
	u32 Data;
	u32 Index;
	u32 CurrPloadIdx;

	if (SwPcrConfig->IsPcrConfigReceived == TRUE) {
		Status = (int)XOCP_PCR_ERR_SWPCR_DUP_CONFIG;
		goto END;
	}

	/* NumOfPcrRecords describes how many data records are present
	 * in the CDO Payload
	 */
	Index = XOCP_PAYLOAD_START_INDEX;
	Data = Pload[Index];

	while ((Index <= NumOfPcrRecords) && (Data != 0x0U)) {
		/* Parse the digests configuration from the payload */
		PcrNum = (Data & XOCP_PCR_IN_BYTE1_OF_PLOAD_MASK) >> XOCP_SINGLE_BYTE_SHIFT;
		NoOfDigests = Data & XOCP_DIGESTS_IN_BYTE0_PLOAD_MASK;
		TotalDigestCount += NoOfDigests;

		/* Validate total digests configured */
		if(TotalDigestCount > XOCP_MAX_NUM_OF_SWPCRS) {
			Status = (int)XOCP_PCR_ERR_IN_SWPCR_CONFIG;
			goto END;
		}

		/* Validate SW PCR number */
		if (PcrNum >= NumOfPcrRecords) {
			Status = (int)XOCP_PCR_ERR_PCR_SELECT;
			goto END;
		}

		Status = XOcp_StoreNoOfDigestPerPcr(PcrNum, NoOfDigests);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Data = Data >> XOCP_HALF_WORD_SHIFT_LEN;
		if (Data == 0x0U) {
			Index++;
			Data = Pload[Index];
		}
	}

	CurrPloadIdx = Index;

	/* Calculate and store PCR start indices in the config */
	SwPcrConfig->PcrIdxInLog[0U] = 0U;
	for (Index = XOCP_PAYLOAD_START_INDEX; Index < NumOfPcrRecords; Index++) {
		SwPcrConfig->PcrIdxInLog[Index] =
		SwPcrConfig->PcrIdxInLog[Index - 1U] + SwPcrConfig->DigestsForPcr[Index - 1U];
	}

	Status = XOcp_StoreEventIdConfig(Pload, CurrPloadIdx, TotalDigestCount, Len);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	SwPcrConfig->IsPcrConfigReceived = TRUE;

	Status = XOcp_ExtendSwPcr(XOCP_SW_PCR_NUM_FOR_ROM_DIGEST,
			XOCP_SW_PCR_MEASUREMENT_IDX_ROM_PLM,
			(u64)(UINTPTR)PMC_GLOBAL_ROM_VALIDATION_DIGEST_0,
			XOCP_PCR_HASH_SIZE_IN_BYTES, FALSE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XOcp_ExtendSwPcr(XOCP_SW_PCR_NUM_FOR_PLM_DIGEST,
			XOCP_SW_PCR_MEASUREMENT_IDX_ROM_PLM,
			(u64)(UINTPTR)PMC_GLOBAL_PMC_FW_HASH_0,
			XOCP_PCR_HASH_SIZE_IN_BYTES, FALSE);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function clears the all digest data for the specified SW PCR.
 *
 * @param	PcrNum	SW PCR number
 *
 * @return
 *		- XST_SUCCESS - Upon success
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
static int XOcp_ClearDigestData(u32 PcrNum)
{
	int Status = XST_FAILURE;
	u32 DigestIdx = XOcp_GetPcrOffsetInLog(PcrNum);
	XOcp_SwPcrStore *SwPcr = XOcp_GetSwPcrInstance();
	u32 Index;

	for (Index = DigestIdx; Index < (DigestIdx + SwPcr->CountPerPcr[PcrNum]); Index++) {
		Status = Xil_SMemSet((void *)SwPcr->Data[Index].DataToExtend,
				XOCP_PCR_HASH_SIZE_IN_BYTES, 0U, XOCP_PCR_HASH_SIZE_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = Xil_SMemSet((void *)SwPcr->Data[Index].Measurement.MeasuredData,
				XOCP_PCR_HASH_SIZE_IN_BYTES, 0U, XOCP_PCR_HASH_SIZE_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		Status = Xil_SMemSet((void *)SwPcr->Data[Index].Measurement.HashOfData,
				XOCP_PCR_HASH_SIZE_IN_BYTES, 0U, XOCP_PCR_HASH_SIZE_IN_BYTES);
		if (Status != XST_SUCCESS) {
			goto END;
		}

		SwPcr->Data[Index].DataAddr = 0U;
		SwPcr->Data[Index].Measurement.DataLength = 0U;
		SwPcr->Data[Index].IsReqExtended = 0U;
	}

	SwPcr->CountPerPcr[PcrNum] = 0U;

	Status = XST_SUCCESS;
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function stores the SW PCR digest config sent via CDO.
 *
 * @param	PcrNum	SW PCR number
 * @param	NumOfDigests	Digests for SW PCR
 *
 * @return
 *		- XST_SUCCESS - Upon success
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
static int XOcp_StoreNoOfDigestPerPcr(u32 PcrNum, u32 NumOfDigests)
{
	int Status = XST_FAILURE;
	XOcp_SwPcrConfig *SwPcrConfig = XOcp_GetSwPcrConfigInstance();

	if (SwPcrConfig->PcrDigestsConfigured[PcrNum] == TRUE) {
		Status = (int)XOCP_PCR_ERR_SWPCR_DUP_CONFIG;
		goto END;
	}

	SwPcrConfig->DigestsForPcr[PcrNum] = (u8)NumOfDigests;
	SwPcrConfig->PcrDigestsConfigured[PcrNum] = TRUE;

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function stores the EventID for each Measurement Index of
 * 		SW PCR into SW PCR log.
 *
 * @param	Pload 		Pointer to the command payload
 * @param	CurrIdx 	Current index of the payload
 * @param	DigestCount	Total no digests configured
 * @param	Len 		CDO payload length
 *
 * @return
 *		- XST_SUCCESS - Upon success
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
static int XOcp_StoreEventIdConfig(u32 *Pload, u32 CurrIdx, u32 DigestCount, u32 Len)
{
	int Status = XST_FAILURE;
	XOcp_SwPcrStore *SwPcr = XOcp_GetSwPcrInstance();
	XOcp_SwPcrConfig *SwPcrConfig = XOcp_GetSwPcrConfigInstance();
	u32 NumOfPcrRecords = Pload[0U];
	u32 PcrNum;
	u32 MeasurementIdx;
	u32 DigestIdx;
	u32 Index;

	for (Index = CurrIdx;
		Index < (CurrIdx + DigestCount * XOCP_DOUBLE_NUM_OF_WORDS);
		Index = Index + XOCP_DOUBLE_NUM_OF_WORDS) {
		if (Index > (Len - 1U)) {
			Status = (int)XOCP_PCR_ERR_IN_SWPCR_CONFIG;
			goto END;
		}
		/* Parse the Event ID configuration from CDO payload */
		PcrNum = (Pload[Index] & XOCP_PCR_IN_BYTE1_OF_PLOAD_MASK) >> XOCP_SINGLE_BYTE_SHIFT;
		MeasurementIdx = Pload[Index] & XOCP_MEASUREIDX_IN_PLOAD_MASK;
		/* Validate SW PCR number */
		if (PcrNum >= NumOfPcrRecords) {
			Status = (int)XOCP_PCR_ERR_PCR_SELECT;
			goto END;
		}
		/* If MeasurementIdx is greater than number of digests
		 * configured, throw an error.
		 */
		if (MeasurementIdx >= SwPcrConfig->DigestsForPcr[PcrNum]) {
			Status = (int)XOCP_PCR_ERR_MEASURE_IDX_SELECT;
			goto END;
		}

		DigestIdx = SwPcrConfig->PcrIdxInLog[PcrNum] + MeasurementIdx;
		SwPcr->Data[DigestIdx].Measurement.EventId = Pload[Index + 1U];
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function computes PCR extend value using SHA2-384.
 *
 * @param	PcrNum		SW PCR number
 * @param 	ExtendedHash	Pointer to the Hash where the resultant hash
 *				shall be placed
 * @return
 *		- XST_SUCCESS - Upon success
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
static int XOcp_CalculateSwPcr(u32 PcrNum, u8 *ExtendedHash)
{
	int Status = XST_FAILURE;
	XOcp_SwPcrStore *SwPcr = XOcp_GetSwPcrInstance();
	XOcp_SwPcrConfig *SwPcrConfig = XOcp_GetSwPcrConfigInstance();
	u32 DigestIdx = XOcp_GetPcrOffsetInLog(PcrNum);
	u32 Index;

	/* Iterate over Digests extended to that PCR */
	for (Index = 0; Index < SwPcrConfig->DigestsForPcr[PcrNum]; Index++) {
		/* If any digest is not extended in the order then skip it */
		if (SwPcr->Data[DigestIdx + Index].IsReqExtended == FALSE) {
			continue;
		}

		/* Extend current Data hash with the previous digest PCR measurement */
		XSecure_Sha384Start();

		Status = XSecure_Sha384Update(ExtendedHash, XOCP_PCR_HASH_SIZE_IN_BYTES);
		if(Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_Sha384Update((u8 *)(UINTPTR)&SwPcr->Data[DigestIdx + Index].Measurement.HashOfData,
					XOCP_PCR_HASH_SIZE_IN_BYTES);
		if(Status != XST_SUCCESS) {
			goto END;
		}

		/* Push the calculated hash to same buffer */
		Status = XSecure_Sha384Finish((XSecure_Sha2Hash *)ExtendedHash);
		if(Status != XST_SUCCESS) {
			goto END;
		}
	}

	XOcp_PrintData((const u8 *)ExtendedHash, XOCP_PCR_HASH_SIZE_IN_BYTES,
			"Extended Hash:", DEBUG_INFO);
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function prints the given data on the console
 *
 * @param   Data	Pointer to any given data buffer
 * @param   Size	Size of the given buffer
 * @param   Str		Pointer to the data that is printed along the data
 * @param   LogLevel 	Printing of the array will happen as defined by the debug type
 *
 *****************************************************************************/
static void XOcp_PrintData(const u8 *Data, u32 Size, char *Str, u32 LogLevel)
{
	u32 Index;

	if (((LogLevel) & XPlmiDbgCurrentTypes) != 0U) {
		XPlmi_Printf_WoTS(LogLevel,"%s START:\n\r", Str);
		for (Index = 0U; Index < Size; Index++) {
			XPlmi_Printf_WoTS(LogLevel,"%02x", Data[Index]);
		}
		XPlmi_Printf_WoTS(LogLevel,"\r\n");
		XPlmi_Printf_WoTS(LogLevel,"%s END\n\r", Str);
	}
}

/*****************************************************************************/
/**
 * @brief	This function performs data measurement as part of SW PCR
 * 		calculation.
 * 		That is (Event Id || Version || Data) for a specific digest.
 *
 * @param	DigestIdx	PCR Digest index in the SW PCR log
 * 		Hash		Pointer to the Hash where the resultant hash
 * 				shall be placed
 *
 * @return
 *		- XST_SUCCESS - Upon success
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
static int XOcp_DataMeasurement(u32 DigestIdx, u8 *Hash)
{
	int Status = XST_FAILURE;
	XOcp_SwPcrStore *SwPcr = XOcp_GetSwPcrInstance();

	/* Start SHA2 engine */
	XSecure_Sha384Start();

	/* Update EventId to SHA2 */
	Status = XSecure_Sha384Update((u8 *)(UINTPTR)&SwPcr->Data[DigestIdx].Measurement.EventId,
			XOCP_EVENT_ID_NUM_OF_BYTES);
	if(Status != XST_SUCCESS) {
		goto END;
	}
	/* Update Version to SHA2 */
	Status = XSecure_Sha384Update((u8 *)(UINTPTR)&SwPcr->Data[DigestIdx].Measurement.Version,
			XOCP_VERSION_NUM_OF_BYTES);
	if(Status != XST_SUCCESS) {
		goto END;
	}
	/* Update Data to SHA2 */
	if (SwPcr->Data[DigestIdx].Measurement.DataLength > XOCP_PCR_HASH_SIZE_IN_BYTES) {
		Status = XSecure_Sha384Update(
				(u8 *)(UINTPTR)SwPcr->Data[DigestIdx].DataAddr,
				SwPcr->Data[DigestIdx].Measurement.DataLength);

	} else {
		Status = XSecure_Sha384Update(
				(u8 *)(UINTPTR)&SwPcr->Data[DigestIdx].DataToExtend,
				SwPcr->Data[DigestIdx].Measurement.DataLength);
	}
	if(Status != XST_SUCCESS) {
		goto END;
	}

	/* Calculate the SHA2 hash */
	Status = XSecure_Sha384Finish((XSecure_Sha2Hash *)Hash);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function computes PCR extend value using SHA2-384
 * 		and stores in the SW PCR log.
 *
 * @param	PcrNum	SW PCR number.
 *
 * @return
 *		- XST_SUCCESS - Upon success
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
static int XOcp_DigestMeasurementAndUpdateLog(u32 PcrNum)
{
	int Status = XST_FAILURE;
	u32 StartIdx = XOcp_GetPcrOffsetInLog(PcrNum);
	XOcp_SwPcrStore *SwPcr = XOcp_GetSwPcrInstance();
	XOcp_SwPcrConfig *SwPcrConfig = XOcp_GetSwPcrConfigInstance();
	u32 CurrIdx = 0U;
	u32 PrevIdx = 0U;
	u32 IsPrevIdxUpdated = 0U;

	/* Iterate over the digests extended to SW PCR in log */
	for (CurrIdx = StartIdx; CurrIdx < (StartIdx + SwPcrConfig->DigestsForPcr[PcrNum]); CurrIdx++) {
		/* If any digest is not extended in the order then skip it */
		if (SwPcr->Data[CurrIdx].IsReqExtended == FALSE) {
			continue;
		}
		if (IsPrevIdxUpdated == FALSE) {
			PrevIdx = CurrIdx;
		}

		/* Start the Sha2-384 */
		XSecure_Sha384Start();

		/* Extend previous digest PCR measurement */
		Status = XSecure_Sha384Update(
			(u8 *)(UINTPTR)&SwPcr->Data[PrevIdx].Measurement.MeasuredData,
			XOCP_PCR_HASH_SIZE_IN_BYTES);
		if(Status != XST_SUCCESS) {
			goto END;
		}
		/* Extend DataHash calculated of the current digest */
		Status = XSecure_Sha384Update(
				(u8 *)(UINTPTR)&SwPcr->Data[CurrIdx].Measurement.HashOfData,
				XOCP_PCR_HASH_SIZE_IN_BYTES);
		if(Status != XST_SUCCESS) {
			goto END;
		}

		/* Calculate and get the SW PCR measurement */
		Status = XSecure_Sha384Finish(
			(XSecure_Sha2Hash *)(UINTPTR)&SwPcr->Data[CurrIdx].Measurement.MeasuredData);
		if(Status != XST_SUCCESS) {
			goto END;
		}

		PrevIdx = CurrIdx;
		IsPrevIdxUpdated = TRUE;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function returns PCR index in the SW PCR Log.
 *
 * @param	PcrNum	SW PCR number.
 *
 * @return
 *		- XST_SUCCESS - Upon success
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
static u32 XOcp_GetPcrOffsetInLog(u32 PcrNum)
{
	XOcp_SwPcrConfig *SwPcrConfig = XOcp_GetSwPcrConfigInstance();

	return SwPcrConfig->PcrIdxInLog[PcrNum];
}

/*****************************************************************************/
/**
 * @brief	This function configures the XPPU for requesting DME service
 *		to ROM.
 *
 * @return	None.
 ******************************************************************************/


/*****************************************************************************/
/**
 * @brief	This function updates the HWPCR log.
 *
 * @param	PcrNum		PCR register number
 * @param	ExtHashAddr	Address of the hash to be extended
 * @param	DataSize	Size of the Data
 *
 * @return
 *		- XST_SUCCESS - If log update is successful
 *		- XST_FAILURE - Upon failure
 *
 ******************************************************************************/
static int XOcp_UpdateHwPcrLog(XOcp_HwPcr PcrNum, u64 ExtHashAddr, u32 DataSize)
{
	volatile int Status = XST_FAILURE;

	/* If number of PCR events is greater than XOCP_MAX_NUM_OF_HWPCR_EVENTS
	 * update overflow count as true and  decrement number of PCR events for
	 * new update and update the tail index.
	 */
	if (HwPcrLog.LogInfo.RemainingHwPcrEvents >= XOCP_MAX_NUM_OF_HWPCR_EVENTS) {
		HwPcrLog.LogInfo.OverflowCntSinceLastRd++;
		if (HwPcrLog.HeadIndex == HwPcrLog.TailIndex) {
			XOcp_UpdateHwPcrIndex(&HwPcrLog.TailIndex, 1U);
		}
		HwPcrLog.LogInfo.RemainingHwPcrEvents--;
	}

	HwPcrLog.Buffer[HwPcrLog.HeadIndex].PcrNo = (u8)PcrNum;
	Status = XOcp_MemCopy(ExtHashAddr,
		(u64)(UINTPTR)HwPcrLog.Buffer[HwPcrLog.HeadIndex].Hash,
		DataSize / XOCP_WORD_LEN, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		if (Status == XST_SUCCESS) {
			Status = (int)XST_GLITCH_ERROR;
		}
		goto END;
	}

	Status = XOcp_MemCopy(((u64)XOCP_PMC_GLOBAL_PCR_0_0 +
		((u64)PcrNum * XOCP_PCR_SIZE_BYTES)),
		(u64)(UINTPTR)HwPcrLog.Buffer[HwPcrLog.HeadIndex].PcrValue,
		XOCP_PCR_SIZE_WORDS, XPLMI_PMCDMA_0);
	if (Status != XST_SUCCESS) {
		if (Status == XST_SUCCESS) {
			Status = (int)XST_GLITCH_ERROR;
		}
		goto END;
	}

	HwPcrLog.LogInfo.RemainingHwPcrEvents++;
	HwPcrLog.LogInfo.TotalHwPcrLogEvents++;
	XOcp_UpdateHwPcrIndex(&HwPcrLog.HeadIndex, 1U);

	XPlmi_HandleSwError(XIL_NODETYPE_EVENT_ERROR_SW_ERR,
			XIL_EVENT_ERROR_PCR_LOG_UPDATE);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function counts number of 1's in the given number.
 *
 * @param	Num	Given number to check number of set bits
 *
 * @return	Returns Count number of bits set
 *
 ******************************************************************************/
static u32 XOcp_CountNumOfOnesInWord(u32 Num)
{
	int Count = 0U;

	while (Num != 0U)
	{
		Num = Num & (Num - 1U);
		Count++;
	}

	return Count;
}

/*****************************************************************************/
/**
 * @brief       This function provides the pointer to the XOcp_SwPcr
 *		instance which has to be used across the file to store the
 *		log data.
 *
 * @return
 *		Pointer to the XOcp_SwPcr instance
 *
 ******************************************************************************/
static XOcp_SwPcrStore *XOcp_GetSwPcrInstance(void)
{
	static XOcp_SwPcrStore SwPcr = {0U};

	return &SwPcr;
}

/*****************************************************************************/
/**
 * @brief       This function provides the pointer to the XOcp_SwPcrConfig
 *		instance which has to be used across the file to store the
 *		SW PCR config.
 *
 * @return
 *      	Pointer to the XOcp_SwPcrConfig instance
 *
 ******************************************************************************/
static XOcp_SwPcrConfig *XOcp_GetSwPcrConfigInstance(void)
{
	static XOcp_SwPcrConfig SwPcrConfig = {0U};

	return &SwPcrConfig;
}
