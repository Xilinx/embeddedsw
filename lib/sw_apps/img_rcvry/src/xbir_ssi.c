/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_ssi.c
*
* This file contains server side interface functions
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xbir_config.h"
#include "xbir_sys.h"
#include "xbir_ssi.h"
#include "xbir_http.h"
#include "xbir_util.h"
#include "xbir_err.h"

/************************** Constant Definitions *****************************/
#define XBIR_SSI_JSON_OBJ_START			'{'
#define XBIR_SSI_JSON_OBJ_END			'}'
#define XBIR_SSI_JSON_OBJ_SEPERATOR		','
#define XBIR_SSI_JSON_NAME_VAL_SEPERATOR	':'

#define XBIR_SSI_JSON_SYS_BOARD_INFO_NAME	"SysBoardInfo"
#define XBIR_SSI_JSON_CC_INFO_NAME		"CcInfo"
#define XBIR_SSI_JSON_BRD_NAME			"BoardName"
#define XBIR_SSI_JSON_REV_NAME			"RevisionNo"
#define XBIR_SSI_JSON_SERIAL_NAME		"SerialNo"
#define XBIR_SSI_JSON_PART_NO_NAME		"PartNo"
#define XBIR_SSI_JSON_UUID_NAME			"UUID"

#define XBIR_SSI_JSON_MAX_SYS_INFO_LEN		(800U)
#define XBIR_SSI_JSON_MAX_NAME_LEN		(20U)
#define XBIR_SSI_JSON_MAX_VALUE_LEN		(20U)

#define XBIR_SSI_JSON_IMG_A_BOOTABLE_NAME	"ImgABootable"
#define XBIR_SSI_JSON_IMG_B_BOOTABLE_NAME	"ImgBBootable"
#define XBIR_SSI_JSON_REQ_IMG_NAME		"ReqBootImg"
#define XBIR_SSI_JSON_LAST_BOOTABLE_IMG_NAME	"LastBootImg"

#define XBIR_SSI_IMG_DOWNLOAD_CONTENT_TYPE	\
	"Content-Type: multipart/form-data; boundary="
#define XBIR_SSI_IMG_DOWNLOAD_CONTENT_LEN	"Content-Length: "

#define XBIR_SSI_JSON_SUCCESS_RESPONSE		"{\"Status\":\"Success\"}"

#define XBIR_SSI_CARD_COUNT			(2U)
#define XBIR_SSI_IMG_BOOTABLE			(1U)
#define XBIR_SSI_IMG_NON_BOOTABLE		(0U)

/**************************** Type Definitions *******************************/
typedef enum {
	XBIR_HTTP_REQ_GET_CONTENT_TYPE,
	XBIR_HTTP_REQ_GET_CONTENT_LEN,
	XBIR_HTTP_REQ_GET_BOUDARY,
	XBIR_HTTP_REQ_GET_CONTENT,
} XBIR_HTTP_REQ_PARSE_ACTION;

typedef int (*Xbir_WriteDevice) (u32 Offset, u8 *Data, u32 Size,
	Xbir_ImgDataStatus IsLast);

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static const char* Xbir_SsiStrRTrim (const char *Str);
static const char* Xbir_SsiJsonGetName (const char *JsonStr, char *Name,
	u16 NameLen);
static const char* Xbir_SsiJsonGetSeperator (const char *JsonStr,
	char *Seperator);
static const char* Xbir_SsiJsonGetVal(const char *JsonStr, char *Val,
	u16 ValLen);
static int Xbir_SsiGetImgInfo (Xbir_HttpArg *HttpArg, u8 *HttpReq,
	u32 HttpReqLen, u8 **ImgData);
static int Xbir_SsiFindImgInHttpReq(Xbir_HttpArg *HttpArg, u8 *HttpReq,
	const u32 HttpReqLen, u8 **ImgData);
int Xbir_SsiProcessRemainingReq (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
static int Xbir_SsiUpdateImg (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
static int Xbir_SsiInitiateImgUpdate (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen, Xbir_SysBootImgId BootImgId);

/************************** Variable Definitions *****************************/
static u32 Xbir_SsiLastUploadSize;
static Xbir_SysBootImgId Xbir_SsiLastImgUpload;
static u8 PendingCfgCmd = FALSE;
static u8 PendingCrcCmd = FALSE;

/*****************************************************************************/
/**
 * @brief
 * This function builds the JSON payload with SysBoard and Carrier Card information.
 *
 * @param	JsonStr		Json payload
 * @param	JsonStrLen	Json payload length
 *
 * @return	XST_SUCCESS on successful building of JSON string
 *		XST_FAILURE on insufficient input memory to store JSON string
 *
 *****************************************************************************/
int Xbir_SsiJsonBuildSysInfo (char *JsonStr, u16 JsonStrLen)
{
	int Status = XST_FAILURE;
	u16 Len = JsonStrLen;
	char *Str = JsonStr;
	const Xbir_SysInfo *SysBoardInfo;
	const Xbir_CCInfo *CcInfo;

	u16 TotalLen = sizeof(SysBoardInfo->BoardPrdName) +
		sizeof(SysBoardInfo->RevNum) +
		sizeof(SysBoardInfo->BoardSerialNumber) +
		sizeof(SysBoardInfo->BoardPartNum) +
		sizeof(SysBoardInfo->UUID) + strlen(XBIR_SSI_JSON_BRD_NAME) +
		strlen(XBIR_SSI_JSON_REV_NAME) +
		strlen(XBIR_SSI_JSON_SERIAL_NAME) +
		strlen(XBIR_SSI_JSON_PART_NO_NAME) +
		strlen(XBIR_SSI_JSON_UUID_NAME) +
		strlen(XBIR_SSI_JSON_SYS_BOARD_INFO_NAME) +
		strlen(XBIR_SSI_JSON_CC_INFO_NAME) +
		44U + /* Number of double quotes */
		14U + /* Number of spaces */
		3U + /* Number of XBIR_SSI_JSON_OBJ_START */
		3U + /* Number of XBIR_SSI_JSON_OBJ_END */
		12U + /* Number of XBIR_SSI_JSON_OBJ_SEPERATOR */
		12U + /* Number of XBIR_SSI_JSON_NAME_VAL_SEPERATOR */
		1U; /* NULL termination */

	if (Len < TotalLen) {
		goto END;
	}

	SysBoardInfo = Xbir_SysGetSysBoardInfo();
	CcInfo = Xbir_SysGetCcInfo();

	Str[0U] = XBIR_SSI_JSON_OBJ_START;
	Str += 1U;
	Len -= 1U;

	snprintf(Str, Len,
		"\"%s\"%c%c \"%s\"%c\"%s\"%c \"%s\"%c\"%s\"%c "
		"\"%s\"%c\"%s\"%c \"%s\"%c\"%s\"%c \"%s\"%c\"%s\"%c %c",
		XBIR_SSI_JSON_SYS_BOARD_INFO_NAME,

		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,

		XBIR_SSI_JSON_OBJ_START,

		XBIR_SSI_JSON_BRD_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		SysBoardInfo->BoardPrdName,
		XBIR_SSI_JSON_OBJ_SEPERATOR,

		XBIR_SSI_JSON_REV_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		SysBoardInfo->RevNum,
		XBIR_SSI_JSON_OBJ_SEPERATOR,

		XBIR_SSI_JSON_SERIAL_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		SysBoardInfo->BoardSerialNumber,
		XBIR_SSI_JSON_OBJ_SEPERATOR,

		XBIR_SSI_JSON_PART_NO_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		SysBoardInfo->BoardPartNum,
		XBIR_SSI_JSON_OBJ_SEPERATOR,

		XBIR_SSI_JSON_UUID_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		SysBoardInfo->UUID,

		XBIR_SSI_JSON_OBJ_END,
		XBIR_SSI_JSON_OBJ_SEPERATOR);
	Len -= strlen(Str);
	Str += strlen(Str);

	snprintf(Str, Len,
		"\"%s\"%c%c \"%s\"%c\"%s\"%c \"%s\"%c\"%s\"%c "
		"\"%s\"%c\"%s\"%c \"%s\"%c\"%s\"%c \"%s\"%c\"%s\"%c %c",
		XBIR_SSI_JSON_CC_INFO_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,

		XBIR_SSI_JSON_OBJ_START,

		XBIR_SSI_JSON_BRD_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		CcInfo->BoardPrdName,
		XBIR_SSI_JSON_OBJ_SEPERATOR,

		XBIR_SSI_JSON_REV_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		CcInfo->RevNum,
		XBIR_SSI_JSON_OBJ_SEPERATOR,

		XBIR_SSI_JSON_SERIAL_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		CcInfo->BoardSerialNumber,
		XBIR_SSI_JSON_OBJ_SEPERATOR,

		XBIR_SSI_JSON_PART_NO_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
			CcInfo->BoardPartNum,
		XBIR_SSI_JSON_OBJ_SEPERATOR,

		XBIR_SSI_JSON_UUID_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		CcInfo->UUID,

		XBIR_SSI_JSON_OBJ_END,
		XBIR_SSI_JSON_OBJ_END);
	Status = XST_SUCCESS;
	Xbir_Printf(DEBUG_INFO, " \r\n[SysInfo] %s\r\n", JsonStr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function builds the JSON payload with boot image status.
 *
 * @param	JsonStr		Json payload
 * @param	JsonStrLen	Json payload length
 *
 * @return	XST_SUCCESS on successful building of JSON string
 *		XST_FAILURE on insufficient input memory to store JSON string
 *
 *****************************************************************************/
int Xbir_SsiJsonBuildBootImgStatus (char *JsonStr, u16 JsonStrLen)
{
	int Status = XBIR_ERROR_BOOT_IMG_STATUS_LEN;
	const Xbir_SysPersistentState *BootImgStatus;
	u16 TotalLen;

	BootImgStatus = Xbir_SysGetBootImgStatus();

	JsonStr[0U] = '\0';
	/* 12U = Number of double quote
	 * {"ImgABootable":true, "ImgBBootable":true, "ReqBootImg":"ImageA", "LastBootImg":"ImageA"}
	 */
	TotalLen = 1U /* XBIR_SSI_JSON_OBJ_START */ +
		strlen(XBIR_SSI_JSON_IMG_A_BOOTABLE_NAME) + strlen("false") +
		strlen(XBIR_SSI_JSON_IMG_B_BOOTABLE_NAME) + strlen("false") +
		strlen(XBIR_SSI_JSON_REQ_IMG_NAME) + strlen("ImageA") +
		strlen(XBIR_SSI_JSON_LAST_BOOTABLE_IMG_NAME) + strlen("ImageA") +
		1U /*JSON_OBJ_EN */ + 12U;

	if (TotalLen > JsonStrLen) {
		Xbir_Printf(DEBUG_INFO, " ERROR: Invalid Len of Boot Image Status data\r\n");
		goto END;
	}

	snprintf(JsonStr, JsonStrLen,
		"%c \"%s\"%c%s%c \"%s\"%c%s%c \"%s\"%c\"%s\"%c \"%s\"%c\"%s\" %c",
		XBIR_SSI_JSON_OBJ_START,

		XBIR_SSI_JSON_IMG_A_BOOTABLE_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		(BootImgStatus->ImgABootable != 0x00U) ? "true" : "false",
		XBIR_SSI_JSON_OBJ_SEPERATOR,

		XBIR_SSI_JSON_IMG_B_BOOTABLE_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		(BootImgStatus->ImgBBootable != 0x00U) ? "true" : "false",
		XBIR_SSI_JSON_OBJ_SEPERATOR,

		XBIR_SSI_JSON_REQ_IMG_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		(BootImgStatus->RequestedBootImg != 0x00U) ? "ImageB" : "ImageA",
		XBIR_SSI_JSON_OBJ_SEPERATOR,

		XBIR_SSI_JSON_LAST_BOOTABLE_IMG_NAME,
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		(BootImgStatus->LastBootedImg != 0x00U) ? "ImageB" : "ImageA",

		XBIR_SSI_JSON_OBJ_END);
	Status = XST_SUCCESS;

	Xbir_Printf(DEBUG_INFO, " [BootImgInfo] %s\r\n", JsonStr);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function builds the JSON payload with Flash Erase status.
 *
 * @param	JsonStr		Json payload
 * @param	JsonStrLen	Json payload length
 *
 * @return	XST_SUCCESS on successful building of JSON string
 *		XST_FAILURE on insufficient input memory to store JSON string
 *
 *****************************************************************************/
int Xbir_SsiJsonBuildFlashEraseStatus(char *JsonStr, u16 JsonStrLen)
{
	int Status = XBIR_ERROR_BOOT_IMG_STATUS_LEN;
	u16 TotalLen;
	char ProgressString[4U];
	Xbir_FlashEraseStats *FlashEraseStats = Xbir_GetFlashEraseStats();
	u32 Progress = (FlashEraseStats->NumOfSectorsErased * 100) /
				FlashEraseStats->TotalNumOfSectors;

	snprintf(ProgressString, 4U, "%3u", Progress);
	JsonStr[0U] = '\0';
	/* 12U = Number of double quote
	 * {"Progress":100}
	 */
	TotalLen = 1U /* XBIR_SSI_JSON_OBJ_START */ +
		strlen("Progress") + strlen(ProgressString) +
		1U /*JSON_OBJ_EN */ + 12U;

	if (TotalLen > JsonStrLen) {
		Xbir_Printf(DEBUG_INFO, " ERROR: Invalid Len of Boot Image Status data\r\n");
		goto END;
	}

	snprintf(JsonStr, JsonStrLen,
		"%c \"%s\"%c%s %c",
		XBIR_SSI_JSON_OBJ_START,
		"Progress",
		XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
		ProgressString,
		XBIR_SSI_JSON_OBJ_END);

	if (FlashEraseStats->State == XBIR_FLASH_ERASE_REQUESTED) {
		FlashEraseStats->State = XBIR_FLASH_ERASE_STARTED;
		Xbir_Printf(DEBUG_INFO, " FlashEraseStatus Progress: \n\r");
	}
	Xbir_Printf(DEBUG_INFO, " %3u %%\r", Progress);

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function parses the JSON string and updates the boot persistent state
 * register based on the inputs in JSON string.
 *
 * @param	JsonStr		Json payload
 * @param	JsonStrLen	Json payload length
 *
 * @return	XST_SUCCESS if boot image status updated successfully
 *		Error code otherwise
 *
 *****************************************************************************/
int Xbir_SsiJsonCfgBootImgStatus (char *JsonStr, u16 JsonStrLen)
{
	int Status = XBIR_ERROR_INVALID_JSON_OBJ;
	const char *Str;
	char Name[XBIR_SSI_JSON_MAX_NAME_LEN + 1U];
	char Val[XBIR_SSI_JSON_MAX_VALUE_LEN + 1U];
	char Seperator;
	u8 ImgABootable = 0xFFU;
	u8 ImgBBootable = 0xFFU;
	u8 ReqBootImg = 0xFFU;

	if ((*JsonStr == 0) && (PendingCfgCmd == FALSE)) {
		PendingCfgCmd = TRUE;
		Status = XST_SUCCESS;
		goto END;
	}

	Str = Xbir_SsiStrRTrim(JsonStr);
	Str = strstr(Str, "{");
	if (Str[0U] != XBIR_SSI_JSON_OBJ_START)
	{
		PendingCfgCmd = TRUE;
		Status = XST_SUCCESS;
		goto END;
	}
	Str++;


	do {
		Str = Xbir_SsiJsonGetName(Str, Name, XBIR_SSI_JSON_MAX_NAME_LEN);
		if (Name[0U] == '\0') {
			break;
		}

		Str = Xbir_SsiJsonGetSeperator(Str, &Seperator);
		if (Seperator != XBIR_SSI_JSON_NAME_VAL_SEPERATOR) {
			break;
		}

		Str = Xbir_SsiJsonGetVal(Str, Val, XBIR_SSI_JSON_MAX_VALUE_LEN);
		if (Val[0U] == '\0') {
			break;
		}

		Xbir_Printf(DEBUG_INFO, " [%s = %s]\r\n", Name, Val);
		if (strcmp(Name, XBIR_SSI_JSON_IMG_A_BOOTABLE_NAME) == 0U) {
			if (strcmp(Val, "true") == 0U) {
				ImgABootable = 1U;
			}
			else if (strcmp(Val, "false") == 0U) {
				ImgABootable = 0U;
			}
			else {
				Xbir_Printf(DEBUG_INFO, " ERROR: Invalid value for ImageA Bootable (%s)\r\n", Val);
				Status = XBIR_ERROR_JSON_IMG_A_BOOTABLE_VAL;
				break;
			}
		}
		else if (strcmp(Name, XBIR_SSI_JSON_IMG_B_BOOTABLE_NAME) == 0U) {
			if (strcmp(Val, "true") == 0U) {
				ImgBBootable = 1U;
			}
			else if (strcmp(Val, "false") == 0U) {
				ImgBBootable = 0U;
			}
			else {
				Xbir_Printf(DEBUG_INFO, " ERROR: Invalid value for ImageB Bootable (%s)\r\n", Val);
				Status = XBIR_ERROR_JSON_IMG_B_BOOTABLE_VAL;
				break;
			}
		}
		else if (strcmp(Name, XBIR_SSI_JSON_REQ_IMG_NAME) == 0U) {
			if (strcmp(Val, "ImageA") == 0U) {
				ReqBootImg = 0x00U;
			}
			else if (strcmp(Val, "ImageB") == 0U) {
				ReqBootImg = 0x01U;
			}
			else {
				Xbir_Printf(DEBUG_INFO, " ERROR: Invalid Requested Image name (%s)\r\n", Val);
				Status = XBIR_ERROR_JSON_REQ_IMG_NAME;
				break;
			}
		}

		Str = Xbir_SsiJsonGetSeperator(Str, &Seperator);
		if (Seperator == XBIR_SSI_JSON_OBJ_END) {
			if ((ImgABootable != 0xFFU) && (ImgABootable != 0xFFU) &&
				(ReqBootImg != 0xFFU)) {
				Status = XST_SUCCESS;
			}
			else {
				Xbir_Printf(DEBUG_INFO, " Incomplete boot image cfg request\r\n");
				Status = XBIR_ERROR_JSON_INCOMPLETE_IMG_CFG_REQ;
			}
			break;
		}

		if (Seperator != XBIR_SSI_JSON_OBJ_SEPERATOR) {
			Xbir_Printf(DEBUG_INFO, " ERROR: Invalid JSON OBJ Separator\r\n");
			Status = XBIR_ERROR_JSON_OBJ_SEPARATOR;
			break;
		}
	}
	while (JsonStr != NULL);

	if (XST_SUCCESS == Status) {
		Xbir_Printf(DEBUG_INFO, " Updating boot image status %d %d %d\r\n",
			ImgABootable, ImgBBootable, ReqBootImg);
		Status = Xbir_SysUpdateBootImgStatus(ImgABootable, ImgBBootable,
			ReqBootImg);
	}

END:
	if (XBIR_ERROR_INVALID_JSON_OBJ == Status) {
		Xbir_Printf(DEBUG_INFO, " ERROR: Invalid JSON format\r\n");
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function initiates the update of image A.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if the image A update starts successfully
 *		Error code otherwise
 *
 *****************************************************************************/
int Xbir_SsiUpdateImgA (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	const Xbir_SysPersistentState *BootImgStatus;

	Xbir_Printf(DEBUG_INFO, " \r\n[Image Update Request]\r\n");

	BootImgStatus = Xbir_SysGetBootImgStatus();

	Xbir_Printf(DEBUG_INFO, " Making the boot img A non-bootable\r\n");
	Status = Xbir_SysUpdateBootImgStatus(XBIR_SSI_IMG_NON_BOOTABLE,
		BootImgStatus->ImgBBootable, BootImgStatus->RequestedBootImg);
	if (XST_SUCCESS == Status) {
		Xbir_Printf(DEBUG_INFO, " Initiating Img A upload\r\n");
		Status = Xbir_SsiInitiateImgUpdate(Tpcb, HttpReq, HttpReqLen,
			XBIR_SYS_BOOT_IMG_A_ID);
	}
	else {
		Xbir_Printf(DEBUG_INFO, " ERROR: Failed to make Img A non bootable\r\n");
		Status = XBIR_ERROR_IMG_A_UPLOAD;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function initiates the update of image B.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if the image b update starts successfully
 *		Error code otherwise
 *
 *****************************************************************************/
int Xbir_SsiUpdateImgB (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	const Xbir_SysPersistentState *BootImgStatus;

	Xbir_Printf(DEBUG_INFO, " \r\n[Image Update Request]\r\n");

	BootImgStatus = Xbir_SysGetBootImgStatus();
	Xbir_Printf(DEBUG_INFO, " Making the boot img B non-bootable\r\n");
	Status = Xbir_SysUpdateBootImgStatus(BootImgStatus->ImgABootable,
		XBIR_SSI_IMG_NON_BOOTABLE,
		BootImgStatus->RequestedBootImg);
	if (XST_SUCCESS == Status) {
		Xbir_Printf(DEBUG_INFO, " Initiating Img B upload\r\n");
		Status = Xbir_SsiInitiateImgUpdate(Tpcb, HttpReq, HttpReqLen,
			XBIR_SYS_BOOT_IMG_B_ID);
	}
	else {
		Xbir_Printf(DEBUG_INFO, " ERROR: Failed to make Img B non bootable\r\n");
		Status = XBIR_ERROR_IMG_B_UPLOAD;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function initiates the update of WIC Image.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if the image A update starts successfully
 *		Error code otherwise
 *
 *****************************************************************************/
int Xbir_SsiUpdateImgWIC (struct tcp_pcb *Tpcb, u8 *HttpReq, u16 HttpReqLen)
{
	int Status = XST_FAILURE;

	Xbir_Printf(DEBUG_INFO, " \r\n[Image Update Request]\r\n");
	Xbir_Printf(DEBUG_INFO, " Initiating Img WIC upload\r\n");
	Status = Xbir_SsiInitiateImgUpdate(Tpcb, HttpReq, HttpReqLen,
		XBIR_SYS_BOOT_IMG_WIC);

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function handles the multipacket request. The first packet of HTTP
 * POST request contains the size of the content data, and this function
 * processes the input data based on that information.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if the additional payload is stored in QSPI
 * 			 successfully
 *		Error code otherwise
 *
 *****************************************************************************/
int Xbir_SsiProcessAdditionalPayload (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	u32 ImgSize;
	u32 ImgSizeInThisPkt;
	u8 *ImgData;
	Xbir_HttpArg *HttpArg = (Xbir_HttpArg *)Tpcb->callback_arg;

	if (PendingCfgCmd == TRUE) {
		Status = Xbir_SsiJsonCfgBootImgStatus((char *)HttpReq,
			strlen((char *)HttpReq));
		PendingCfgCmd = FALSE;
		goto END;
	}

	if (PendingCrcCmd == TRUE) {
		Status = Xbir_SsiValidateLastUpdate((char *)HttpReq,
			HttpReqLen);
		PendingCrcCmd = FALSE;
		goto END;
	}
	PendingCfgCmd = FALSE;
	PendingCrcCmd = FALSE;

	if (HttpArg->Fsize == 0U) {
		Status = Xbir_SsiFindImgInHttpReq (HttpArg, HttpReq,
				HttpReqLen, &ImgData);
		if (Status == XST_SUCCESS) {
			ImgSizeInThisPkt = HttpReqLen - (u16)(ImgData - HttpReq);
			ImgSize = HttpArg->Fsize;

			Xbir_Printf(DEBUG_INFO, " Starting img update\r\n");

			Xbir_Printf(DEBUG_INFO, " Starting img upload to flash\r\n");
			Status = Xbir_SsiUpdateImg(Tpcb, ImgData, ImgSizeInThisPkt);
			Xbir_SsiLastUploadSize = ImgSize;
		}
	}
	else {
		Status = Xbir_SsiUpdateImg(Tpcb, HttpReq, HttpReqLen);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function validates the last downloaded image. This function parses
 * input JSON string for extracting CRC and then compares this CRC with CRC
 * calculated over last uploaded image.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if input CRC matches with CRC of last uploaded image
 *		Error code otherwise
 *
 *****************************************************************************/
u32 Xbir_SsiValidateLastUpdate (char *JsonStr, u16 JsonStrLen)
{
	int Status = XST_FAILURE;
	const char *Str = JsonStr;
	char Name[XBIR_SSI_JSON_MAX_NAME_LEN + 1U] = {0U};
	char Val[XBIR_SSI_JSON_MAX_VALUE_LEN + 1U] = {0U};
	char Seperator;
	const Xbir_SysPersistentState *BootImgStatus;

	if (*JsonStr == 0) {
		PendingCrcCmd = TRUE;
		Status = XST_SUCCESS;
		goto END;
	}

	Str = Xbir_SsiStrRTrim(JsonStr);
	Str = strstr(Str, "{");
	if (Str[0U] != XBIR_SSI_JSON_OBJ_START)
	{
		goto END;
	}
	Str++;

	Str = Xbir_SsiJsonGetName(Str, Name, XBIR_SSI_JSON_MAX_NAME_LEN);
	if (strncmp(Name, "crc", strlen("crc")) != 0U) {
		goto END;
	}
	Str = Xbir_SsiJsonGetSeperator(Str, &Seperator);
	if (Seperator != XBIR_SSI_JSON_NAME_VAL_SEPERATOR) {
		goto END;
	}

	Str = Xbir_SsiJsonGetVal(Str, Val, XBIR_SSI_JSON_MAX_VALUE_LEN);
	if ((Val[0U] == '\0') && (Xbir_UtilIsNumber(Val) == 0U)) {
		goto END;
	}

	Xbir_Printf(DEBUG_INFO, " Validating CRC\r\n");
	Status = Xbir_SysValidateCrc(Xbir_SsiLastImgUpload,
		Xbir_SsiLastUploadSize, atol(Val));
	if (XST_SUCCESS == Status) {
		if (XBIR_SYS_BOOT_IMG_A_ID == Xbir_SsiLastImgUpload) {
			BootImgStatus = Xbir_SysGetBootImgStatus();
			Xbir_Printf(DEBUG_INFO, " Making the boot image A requested image\r\n");
			Status = Xbir_SysUpdateBootImgStatus(XBIR_SSI_IMG_BOOTABLE,
				 BootImgStatus->ImgBBootable,
				 XBIR_SYS_BOOT_IMG_A_ID);
		}
		else if (XBIR_SYS_BOOT_IMG_B_ID == Xbir_SsiLastImgUpload) {
			BootImgStatus = Xbir_SysGetBootImgStatus();
			Xbir_Printf(DEBUG_INFO, " Making the boot image B requested image\r\n");
			Status = Xbir_SysUpdateBootImgStatus(BootImgStatus->ImgABootable,
				 XBIR_SSI_IMG_BOOTABLE,
				 XBIR_SYS_BOOT_IMG_B_ID);
		}
		else if (XBIR_SYS_BOOT_IMG_WIC == Xbir_SsiLastImgUpload) {
			Status = XST_SUCCESS;
		}
		else {
			Xbir_Printf(DEBUG_INFO, " ERROR: Invalid img verification request\r\n");
			goto END;
		}

		if (Status == XST_SUCCESS) {
			Xbir_Printf(DEBUG_INFO, " Download Complete....\r\n");
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function returns a pointer to first occurance of non space and tab
 * character i.e. it returns pointer by skipping leading spaces and tabs
 * from input string.
 *
 * @param	Str	Pointer to input string
 *
 * @return	Pointer to string starting with non-space character
 *
 *****************************************************************************/
static const char* Xbir_SsiStrRTrim (const char *Str)
{
	const char *InStr = Str;

	while ((InStr != NULL) && ((*InStr == ' ') || (*InStr == '\t'))) {
		InStr++;
	}

	return InStr;
}

/*****************************************************************************/
/**
 * @brief
 * This function extracts the name from name value pair of JSON string. It can
 * also be used to read string surrounded by double quote (").
 *
 * @param	JsonStr	Pointer to input JSON string
 * @param	Name	Pointer to string where Name will be stored
 * @param	NameLen	Maximum space available to store Name.
 *
 * @return	Pointer to remaining string after extrating the name
 *
 *****************************************************************************/
static const char* Xbir_SsiJsonGetName (const char *JsonStr, char *Name,
	u16 NameLen)
{
	const char *Str = JsonStr;
	u16 Idx;

	Str = Xbir_SsiStrRTrim(Str);
	if (Str[0U] != '"')
	{
		Str = NULL;
		goto END;
	}
	Str++;
	Idx = 0U;
	while (Idx < NameLen && Str != NULL) {
		Name[Idx] = Str[Idx];
		if (Name[Idx] == '"') {
			break;
		}

		Idx++;
	}

	if (Name[Idx] == '"') {
		Name[Idx] = '\0';
		Str = &Str[Idx + 1];
	}
	else {
		Name[0U] = '\0';
		Str = JsonStr;
	}

END:
	return Str;
}

/*****************************************************************************/
/**
 * @brief
 * This function extracts the seperator (obj or name value pair seperator) from
 * the JSON string. It can also be used to read first non space character from
 * JSON string.
 *
 * @param	JsonStr		Pointer to input JSON string
 * @param	Seperator	Pointer where first non space character will be
 *				stored
 * @return	Pointer to string after seperator or non space character
 *
 *****************************************************************************/
static const char* Xbir_SsiJsonGetSeperator (const char *JsonStr,
	char *Seperator)
{
	const char *Str = JsonStr;

	Str = Xbir_SsiStrRTrim(Str);
	*Seperator = Str[0U];

	return (Str + 1U);
}

/*****************************************************************************/
/**
 * @brief
 * This function extracts the value from name value pair of JSON string.
 *
 * @param	JsonStr	Pointer to input JSON string
 * @param	Val	Pointer where value from name value pair will be stored
 *
 * @return	Pointer to remaining string after extrating the value
 *
 *****************************************************************************/
static const char* Xbir_SsiJsonGetVal(const char *JsonStr, char *Val,
	u16 ValLen)
{
	const char *Str = JsonStr;
	u16 Idx;

	Str = Xbir_SsiStrRTrim(Str);

	if (Str[0U] == '"') {
		Str = Xbir_SsiJsonGetName(Str, Val, ValLen);
	}
	else {
		Idx = 0U;
		while ((Idx < ValLen) && (Str != NULL)) {
			Val[Idx] = Str[Idx];
			if (isspace((int)Val[Idx]) || (Val[Idx] == ',')) {
				Val[Idx] = '\0';
				break;
			}
			Idx++;
		}
		if (Val[Idx] == '\0') {
			Str = &Str[Idx];
		}
		else {
			Str = JsonStr;
			*Val = '\0';
		}
	}

	return Str;
}

/*****************************************************************************/
/**
 * @brief
 * This function parses the HTTP packet to extract the information about image
 * to be uploaded.
 *
 * @param	HttpArg		Pointer to Xbir_HttpArg instance
 * @param	HttpReq		HTTP payload
 * @param	HttpReqLen	HTTP payload length
 * @param	ImgData		Stores pointer to start of image data in HTTP
 *				request
 *
 * @return	XST_SUCCESS if the information of the image getting downlodaded
 *			is extracted successfully from input HTTP request
 *		XST_FAILURE otherwise
 *
 *****************************************************************************/
static int Xbir_SsiGetImgInfo (Xbir_HttpArg *HttpArg, u8 *HttpReq,
	u32 HttpReqLen, u8 **ImgData)
{
	int Status = XST_FAILURE;
	char *Line = (char *) HttpReq;
	u32 Len = 0U;
	u32 Size;

	Line = strtok(Line, "\n");
	Len += strnlen(Line, HttpReqLen) + 1U;
	while((Line != NULL) && (Len < HttpReqLen)) {
		Size = strlen(XBIR_SSI_IMG_DOWNLOAD_CONTENT_TYPE);
		if ((strncmp(XBIR_SSI_IMG_DOWNLOAD_CONTENT_TYPE, Line,
			Size) == 0U) && (HttpArg->BoundaryLen == 0U)) {
			HttpArg->Boundary[0U] = '-';
			HttpArg->Boundary[1U] = '-';
			strncpy((char *) &HttpArg->Boundary[2U], Line + Size,
					XBIR_HTTP_MAX_BOUNDARY_LEN - 2U);
			HttpArg->BoundaryLen = strnlen((char *) HttpArg->Boundary,
					XBIR_HTTP_MAX_BOUNDARY_LEN);
		}
		else {
			Size = strlen(XBIR_SSI_IMG_DOWNLOAD_CONTENT_LEN);
			if (strncmp(XBIR_SSI_IMG_DOWNLOAD_CONTENT_LEN, Line,
				Size) == 0U) {
				HttpArg->ContentLen = atol(&Line[Size]);
			}
		}

		if (Status == XST_FAILURE) {
			Line = strtok(NULL, "\n");
			Len += (strnlen (Line, HttpReqLen) + 1U);
			if (Line[0U] == '\r' || Line[0U] == '\n')
				break;
		}
	}

	if (Len < HttpReqLen) {
		Status = Xbir_SsiFindImgInHttpReq(HttpArg, &HttpReq[Len],
			(HttpReqLen - Len), ImgData);
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function Finds the image in the HTTP Request
 *
 * @param	HttpArg		Pointer to Xbir_HttpArg instance
 * @param	HttpReq		HTTP payload
 * @param	HttpReqLen	HTTP payload length
 * @param	ImgData		Stores pointer to start of image data in HTTP
 *				request
 *
 * @return	XST_SUCCESS if we find the image information of the image in
			HTTP request
 *		XST_FAILURE otherwise
 *
 *****************************************************************************/
int Xbir_SsiFindImgInHttpReq(Xbir_HttpArg *HttpArg, u8 *HttpReq,
	const u32 HttpReqLen, u8 **ImgData)
{
	int Status = XST_FAILURE;
	u32 BoundaryFound = FALSE;
	char *Line = (char *) HttpReq;
	char *FileInfo = NULL;
	u32 Len = 0U;

	Line = strtok(Line, "\n");
	Len += strnlen(Line, HttpReqLen);

	while((Line != NULL) && (Len < HttpReqLen) && (Status != XST_SUCCESS)) {
		if (BoundaryFound == FALSE) {
			if (strncmp((char *)HttpArg->Boundary, Line,
				HttpArg->BoundaryLen) == 0U) {
				FileInfo = Line;
				BoundaryFound = TRUE;
			}
		}
		else {
			if (Line[0U] == '\r') {
				HttpArg->Fsize = HttpArg->ContentLen -
					(u64)(Line - FileInfo) - 2U -
					HttpArg->BoundaryLen - 5U;
				*ImgData = (u8 *)Line + 2U;
				Xbir_Printf(DEBUG_INFO, " Size of Image to be downloaded = %u\r\n",
					HttpArg->Fsize);
				Status = XST_SUCCESS;
			}
		}

		if (Status != XST_SUCCESS) {
			Line  = strtok(NULL, "\n");
			Len += strnlen (Line, HttpReqLen);
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function uploads the input image data to flash.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if the input data is successfully written to flash
 *		Error code otherwise
 *
 *****************************************************************************/
static int Xbir_SsiUpdateImg (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	u32 DataSize;
	Xbir_HttpArg *HttpArg = (Xbir_HttpArg *)Tpcb->callback_arg;
	Xbir_FlashEraseStats *FlashEraseStats = Xbir_GetFlashEraseStats();
	Xbir_WriteDevice WriteDevice = NULL;

#if (defined(XBIR_SD_0) || defined(XBIR_SD_1))
	if (XBIR_SYS_BOOT_IMG_WIC == Xbir_SsiLastImgUpload)
		WriteDevice = Xbir_SysWriteSD;
	else
#endif
		WriteDevice = Xbir_SysWriteFlash;

	if (HttpArg->Fsize > 0U) {
		if (HttpArg->Fsize <= HttpReqLen) {
			DataSize = HttpArg->Fsize;
		}
		else {
			DataSize = HttpReqLen;
		}

		if (HttpArg->Fsize == DataSize) {
			WriteDevice(HttpArg->Offset, HttpReq, DataSize,
				XBIR_SYS_LAST_DATA_CHUNK);
		}
		else {
			WriteDevice(HttpArg->Offset, HttpReq, DataSize,
				XBIR_SYS_PARTIAL_DATA_CHUNK);
		}
		HttpArg->Fsize -= DataSize;
		HttpArg->Offset += DataSize;
	}

	if (HttpArg->Fsize == 0U) {
		if (FlashEraseStats->State == XBIR_FLASH_ERASE_COMPLETED) {
			FlashEraseStats->NumOfSectorsErased = 0U;
			FlashEraseStats->State = XBIR_FLASH_ERASE_NOTSTARTED;
		}
		Status = Xbir_HttpSendResponseJson(Tpcb, HttpReq, HttpReqLen,
				XBIR_SSI_JSON_SUCCESS_RESPONSE,
				strlen(XBIR_SSI_JSON_SUCCESS_RESPONSE));
		Xbir_Printf(DEBUG_INFO, " Img upload to flash complete\r\n");
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function initiates the image update. It processes the first packet of
 * image update HTTP POST request.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 * @param 	Offset		Offset
 *
 * @return	XST_SUCCESS if image update process is started successfully
 *		Error code otherwise
 *
 *****************************************************************************/
static int Xbir_SsiInitiateImgUpdate (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen, Xbir_SysBootImgId BootImgId)
{
	int Status = XST_SUCCESS;
	u8 *ImgData;
	Xbir_HttpArg *HttpArg = (Xbir_HttpArg *)Tpcb->callback_arg;
	u32 ImgSize;
	u32 ImgSizeInThisPkt;
	u32 Offset;
	Xbir_FlashEraseStats *FlashEraseStats = Xbir_GetFlashEraseStats();

	HttpArg->Fsize = 0U;
	HttpArg->BoundaryLen = 0U;
	HttpArg->ContentLen = 0U;
	HttpArg->ImgId = BootImgId;

	Status = Xbir_SsiGetImgInfo (HttpArg, HttpReq, HttpReqLen, &ImgData);
	if (XST_FAILURE == Status) {
		goto END;
	}

	Status = Xbir_SysGetBootImgOffset(BootImgId, &Offset);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	HttpArg->Offset = Offset;

	if (HttpArg->Fsize > 0U) {
		if ((FlashEraseStats->State != XBIR_FLASH_ERASE_COMPLETED) ||
			(FlashEraseStats->CurrentImgErased != BootImgId)) {
			goto END;
		}
		Xbir_SsiLastImgUpload = BootImgId;
		ImgSizeInThisPkt = HttpReqLen - (u16)(ImgData - HttpReq);
		ImgSize = HttpArg->Fsize;
		Xbir_Printf(DEBUG_INFO, " Starting image update\r\n");
		Status = Xbir_SsiUpdateImg (Tpcb, ImgData, ImgSizeInThisPkt);
		Xbir_SsiLastUploadSize = ImgSize;
	}
	else {
		Xbir_SsiLastImgUpload = BootImgId;
	}

END:
	return Status;
}
