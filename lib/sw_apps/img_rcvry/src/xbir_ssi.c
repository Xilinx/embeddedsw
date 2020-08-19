/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
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

/************************** Constant Definitions *****************************/
#define XBIR_SSI_JSON_OBJ_START			'{'
#define XBIR_SSI_JSON_OBJ_END			'}'
#define XBIR_SSI_JSON_OBJ_SEPERATOR		','
#define XBIR_SSI_JSON_NAME_VAL_SEPERATOR	':'

#define XBIR_SSI_JSON_SOM_INFO_NAME		"SomInfo"
#define XBIR_SSI_JSON_CC_INFO_NAME		"CcInfo"
#define XBIR_SSI_JSON_BRD_NAME			"BrdName"
#define XBIR_SSI_JSON_REV_NAME			"RevisionNo"
#define XBIR_SSI_JSON_SERIAL_NAME		"SerialNo"
#define XBIR_SSI_JSON_STATE_NAME		"State"
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
#define XBIR_SSI_IMG_MAX_BOUNDARY_LEN		(1024U)

#define XBIR_SSI_JSON_SUCCESS_RESPONSE		"{\"Status\":\"Success\"}"

#define XBIR_SSI_CARD_COUNT			(2U)

/**************************** Type Definitions *******************************/
typedef enum {
	XBIR_HTTP_REQ_GET_CONTENT_TYPE,
	XBIR_HTTP_REQ_GET_CONTENT_LEN,
	XBIR_HTTP_REQ_GET_BOUDARY,
	XBIR_HTTP_REQ_GET_CONTENT,
} XBIR_HTTP_REQ_PARSE_ACTION;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static const char* Xbir_SsiStrRTrim (const char *Str);
static const char* Xbir_SsiJsonGetName (const char *JsonStr, char *Name,
	u16 NameLen);
static const char* Xbir_SsiJsonGetSeperator (const char *JsonStr,
	char *Seperator);
static const char* Xbir_SsiJsonGetVal(const char *JsonStr, char *Val,
	u16 ValLen);
static int Xbir_SsiGetImgInfo (u8 *HttpReq, u32 HttpReqLen, u8 **ImgData,
	u32 *ImgSize, u32 *ContentLen);
int Xbir_SsiProcessRemainingReq (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
static int Xbir_SsiUpdateImg (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
static int Xbir_SsiInitiateImgUpdate (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen, Xbir_SysBootImgId BootImgId);

/************************** Variable Definitions *****************************/
static u32 Xbir_SsiLastUploadSize;
static Xbir_SysBootImgId Xbir_SsiLastImgUpload;

/*****************************************************************************/
/**
 * @brief
 * This function builds the JSON payload with SoM and Carrier Card information.
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
	u32 Idx = 0U;
	u16 Len = JsonStrLen;
	char *Str = JsonStr;
	const Xbir_SysBoardInfo *BrdInfo;
	const Xbir_SysBoardInfo *SomInfo;
	const Xbir_SysBoardInfo *CcInfo;

	/* 60U = For double quotes and spaces in JSON string*/
	u16 TotalLen = (sizeof(Xbir_SysBoardInfo) + 2U /* JSON_ OBJ_START */ +
		2U /* XBIR_SSI_JSON_OBJ_END */ +
		5U /*XBIR_SSI_JSON_OBJ_SEPERATOR */ +
		strlen(XBIR_SSI_JSON_BRD_NAME) + strlen(XBIR_SSI_JSON_REV_NAME) +
		strlen(XBIR_SSI_JSON_SERIAL_NAME) + strlen(XBIR_SSI_JSON_STATE_NAME) +
		strlen(XBIR_SSI_JSON_PART_NO_NAME) + strlen(XBIR_SSI_JSON_UUID_NAME) +
		60U);

	if (Len < TotalLen) {
		goto END;
	}

	SomInfo = Xbir_SysGetSomInfo();
	CcInfo = Xbir_SysGetCcInfo();

	Str[0U] = XBIR_SSI_JSON_OBJ_START;
	Str += 1U;
	Len -= 1U;
	for (Idx = 0U; Idx < XBIR_SSI_CARD_COUNT; Idx++) {
		BrdInfo = (0U == Idx) ? SomInfo : CcInfo;
		snprintf(Str, Len,
			"\"%s\"%c%c \"%s\"%c\"%s\"%c \"%s\"%c\"%s\"%c "
			"\"%s\"%c\"%s\"%c \"%s\"%c\"%s\"%c \"%s\"%c\"%s\"%c "
			"\"%s\"%c\"%s\"%c %c",
			(0U == Idx) ? XBIR_SSI_JSON_SOM_INFO_NAME : XBIR_SSI_JSON_CC_INFO_NAME,
			XBIR_SSI_JSON_NAME_VAL_SEPERATOR,

			XBIR_SSI_JSON_OBJ_START,

			XBIR_SSI_JSON_BRD_NAME,
			XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
			BrdInfo->BoardPrdName,
			XBIR_SSI_JSON_OBJ_SEPERATOR,

			XBIR_SSI_JSON_REV_NAME,
			XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
			BrdInfo->RevNum,
			XBIR_SSI_JSON_OBJ_SEPERATOR,

			XBIR_SSI_JSON_SERIAL_NAME,
			XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
			BrdInfo->BoardSerialNumber,
			XBIR_SSI_JSON_OBJ_SEPERATOR,

			XBIR_SSI_JSON_STATE_NAME,
			XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
			BrdInfo->PcieInfo,
			XBIR_SSI_JSON_OBJ_SEPERATOR,

			XBIR_SSI_JSON_PART_NO_NAME,
			XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
			BrdInfo->BoardPartNum,
			XBIR_SSI_JSON_OBJ_SEPERATOR,

			XBIR_SSI_JSON_UUID_NAME,
			XBIR_SSI_JSON_NAME_VAL_SEPERATOR,
			BrdInfo->UUID,

			XBIR_SSI_JSON_OBJ_END,

			(0U == Idx) ? XBIR_SSI_JSON_OBJ_SEPERATOR : XBIR_SSI_JSON_OBJ_END);

		Len -= strlen(Str);
		Str += strlen(Str);
	}

	Status = XST_SUCCESS;
	printf("\n[SysInfo]\n%s\n", JsonStr);

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
	int Status = XST_FAILURE;
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

	printf("\n[BootImgInfo]\n%s\n", JsonStr);

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
	int Status = XST_FAILURE;
	const char *Str = JsonStr;
	char Name[XBIR_SSI_JSON_MAX_NAME_LEN + 1U];
	char Val[XBIR_SSI_JSON_MAX_VALUE_LEN + 1U];
	char Seperator;
	u8 ImgABootable = 0xFFU;
	u8 ImgBBootable = 0xFFU;
	u8 ReqBootImg = 0xFFU;

	Str = Xbir_SsiStrRTrim(Str);
	if (Str[0U] != XBIR_SSI_JSON_OBJ_START)
	{
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

		Xbir_Printf("[%s = %s]\r\n", Name, Val);
		if (strcmp(Name, XBIR_SSI_JSON_IMG_A_BOOTABLE_NAME) == 0U) {
			if (strcmp(Val, "true") == 0U) {
				ImgABootable = 1U;
			}
			else if (strcmp(Val, "false") == 0U) {
				ImgABootable = 0U;
			}
			else {
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
				break;
			}
		}

		Str = Xbir_SsiJsonGetSeperator(Str, &Seperator);
		if (Seperator == XBIR_SSI_JSON_OBJ_END) {
			if ((ImgABootable != 0xFFU) && (ImgABootable != 0xFFU) &&
				(ReqBootImg != 0xFFU)) {
				Status = XST_SUCCESS;
			}
			break;
		}

		if (Seperator != XBIR_SSI_JSON_OBJ_SEPERATOR) {
			break;
		}
	}
	while (JsonStr != NULL);

	if (XST_SUCCESS == Status) {
		Xbir_Printf("Updating boot image status %d %d %d\r\n",
			ImgABootable, ImgBBootable, ReqBootImg);
		Status = Xbir_SysUpdateBootImgStatus(ImgABootable, ImgBBootable,
			ReqBootImg);
	}

END:
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

	BootImgStatus = Xbir_SysGetBootImgStatus();

	Xbir_Printf("Making the boot img A non-bootable\r\n");
	Status = Xbir_SysUpdateBootImgStatus(0U, BootImgStatus->ImgBBootable,
		BootImgStatus->RequestedBootImg);
	if (XST_SUCCESS == Status) {
		Xbir_Printf("Initiating img A upload\r\n");
		Status = Xbir_SsiInitiateImgUpdate(Tpcb, HttpReq, HttpReqLen,
			XBIR_SYS_BOOT_IMG_A_ID);
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

	BootImgStatus = Xbir_SysGetBootImgStatus();

	Xbir_Printf("Making the boot img B non-bootable\r\n");
	Status = Xbir_SysUpdateBootImgStatus(BootImgStatus->ImgBBootable, 0U,
		BootImgStatus->RequestedBootImg);
	if (XST_SUCCESS == Status) {
		Xbir_Printf("Initiating img B upload\r\n");
		Status = Xbir_SsiInitiateImgUpdate(Tpcb, HttpReq, HttpReqLen,
			XBIR_SYS_BOOT_IMG_B_ID);
	}
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

	Status = Xbir_SsiUpdateImg(Tpcb, HttpReq, HttpReqLen);

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
	char Name[XBIR_SSI_JSON_MAX_NAME_LEN + 1U];
	char Val[XBIR_SSI_JSON_MAX_VALUE_LEN + 1U];
	char Seperator;
	const Xbir_SysPersistentState *BootImgStatus;

	Str = Xbir_SsiStrRTrim(Str);
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

	Xbir_Printf("Validating CRC\r\n");
	Status = Xbir_SysValidateCrc(Xbir_SsiLastImgUpload,
		Xbir_SsiLastUploadSize, atol(Val));

	if (XST_SUCCESS == Status) {
		BootImgStatus = Xbir_SysGetBootImgStatus();
		if (XBIR_SYS_BOOT_IMG_A_ID == Xbir_SsiLastImgUpload) {
			Xbir_Printf("Making the boot image A requested image\r\n");
			Status = Xbir_SysUpdateBootImgStatus(BootImgStatus->ImgABootable,
				 BootImgStatus->ImgBBootable,
				 XBIR_SYS_BOOT_IMG_A_ID);
		}
		else if (XBIR_SYS_BOOT_IMG_B_ID == Xbir_SsiLastImgUpload) {
			Xbir_Printf("Making the boot image B requested image\r\n");
			Status = Xbir_SysUpdateBootImgStatus(BootImgStatus->ImgABootable,
				 BootImgStatus->ImgBBootable,
				 XBIR_SYS_BOOT_IMG_B_ID);
		}

		if (Status == XST_SUCCESS) {
			Xbir_Printf("Download Complete....\r\n");
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
 * @param	Val	Pointer where value fro name value pair will be stored
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
 * @param	HttpReq		HTTP payload
 * @param	HttpReqLen	HTTP payload length
 * @param	ImgData		Stores pointer to start of image data in HTTP
 *				request
 * @param	ImgLen		Size of the image that is getting downloaded
 * @param	ContentLen	Content length stored in HTTP request
 *
 * @return	XST_SUCCESS if the information of the image getting downlodaded
 *			is extracted successfully from input HTTP request
 *		XST_FAILURE otherwise
 *
 *****************************************************************************/
static int Xbir_SsiGetImgInfo (u8 *HttpReq, u32 HttpReqLen, u8 **ImgData,
	u32 *ImgSize, u32 *ContentLen)
{
	int Status = XST_FAILURE;
	char *Line = (char *) HttpReq;
	char* FileInfo = NULL;
	u32 Len = 0U;
	u32 Size;
	u16 BoundaryLen = 0U;
	XBIR_HTTP_REQ_PARSE_ACTION Action = XBIR_HTTP_REQ_GET_CONTENT_TYPE;
	char Boundary[XBIR_SSI_IMG_MAX_BOUNDARY_LEN + 1U] = "--";

	Line = strtok(Line, "\r\n");
	Len += strnlen(Line, HttpReqLen);
	while((Line != NULL) && (Len < HttpReqLen) && (Status == XST_FAILURE)) {
		switch (Action) {
		case XBIR_HTTP_REQ_GET_CONTENT_TYPE:
			Size = strlen(XBIR_SSI_IMG_DOWNLOAD_CONTENT_TYPE);
			if (strncmp(XBIR_SSI_IMG_DOWNLOAD_CONTENT_TYPE,	Line,
				Size) == 0U) {
				strncpy(&Boundary[strlen(Boundary)],
					Line + Size,
					XBIR_SSI_IMG_MAX_BOUNDARY_LEN);
				BoundaryLen = strnlen(Boundary,
					XBIR_SSI_IMG_MAX_BOUNDARY_LEN);
				Xbir_Printf("[%u] Boundary (%u): %s\r\n",
					XBIR_HTTP_REQ_GET_CONTENT_TYPE,
					BoundaryLen, Boundary);

				Action = XBIR_HTTP_REQ_GET_CONTENT_LEN;
			}
			break;

		case XBIR_HTTP_REQ_GET_CONTENT_LEN:
			Size = strlen(XBIR_SSI_IMG_DOWNLOAD_CONTENT_LEN);
			if (strncmp(XBIR_SSI_IMG_DOWNLOAD_CONTENT_LEN, Line,
				Size) == 0U) {
				*ContentLen = atol(&Line[Size]);
				Xbir_Printf("[%u] Length= %u\r\n",
					XBIR_HTTP_REQ_GET_CONTENT_LEN,
					*ContentLen);
				Action = XBIR_HTTP_REQ_GET_BOUDARY;
			}
			break;

		case XBIR_HTTP_REQ_GET_BOUDARY:
			if (strncmp(Boundary, Line, BoundaryLen) == 0) {
				Action = XBIR_HTTP_REQ_GET_CONTENT;
				FileInfo = Line;
				Xbir_Printf("[%u]\r\n", XBIR_HTTP_REQ_GET_BOUDARY);
			}
			break;

		case XBIR_HTTP_REQ_GET_CONTENT:
			if (Line[0U] == '\r') {
				*ImgSize = *ContentLen -
					(u64)(Line - FileInfo) - 2U -
					BoundaryLen - 5U;
				Xbir_Printf("ImgSize = %ld\r\n", *ImgSize);
				*ImgData = (u8 *)Line + 2U;
				Status = XST_SUCCESS;
			}
			break;

		default:
			break;
		}

		if (Status == XST_FAILURE) {
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

	if (HttpArg->Fsize > 0U) {
		if (HttpArg->Fsize <= HttpReqLen) {
			DataSize = HttpArg->Fsize;
		}
		else {
			DataSize = HttpReqLen;
		}

		if (HttpArg->Fsize == DataSize) {
			Xbir_SysWriteFlash(HttpArg->Offset, HttpReq, DataSize,
				XBIR_SYS_LAST_DATA_CHUNK);
		}
		else {
			Xbir_SysWriteFlash(HttpArg->Offset, HttpReq, DataSize,
				XBIR_SYS_PARTIAL_DATA_CHUNK);
		}
		HttpArg->Fsize -= DataSize;
		HttpArg->Offset += DataSize;
	}

	if (HttpArg->RemainingRxLen <= HttpReqLen) {
		HttpArg->RemainingRxLen = 0U;
		Status = Xbir_HttpSendResponseJson(Tpcb, HttpReq, HttpReqLen,
				XBIR_SSI_JSON_SUCCESS_RESPONSE,
				strlen(XBIR_SSI_JSON_SUCCESS_RESPONSE));
	}
	else {
		HttpArg->RemainingRxLen -= HttpReqLen;
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
	u8 *ImgHdr;
	Xbir_HttpArg *HttpArg;
	u32 ImgSize;
	u32 ContentLen;
	u32 ImgSizeInThisPkt;
	u32 Offset;

	ImgHdr = (u8 *) strstr((char *)HttpReq, "\r\n\r\n") + strlen("\r\n\r\n");

	Status = Xbir_SsiGetImgInfo (HttpReq, HttpReqLen, &ImgData,
		&ImgSize, &ContentLen);
	if (XST_FAILURE == Status) {
		goto END;
	}

	Xbir_Printf("Erasing img\r\n");
	Status = Xbir_SysEraseBootImg(BootImgId);
	if (Status != XST_SUCCESS) {
		Xbir_Printf("Erase Failed\r\n");
		goto END;
	}
	Xbir_Printf("Erasing complete\r\n");

	Status = Xbir_SysGetBootImgOffset(BootImgId, &Offset);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	HttpArg = (Xbir_HttpArg *)Tpcb->callback_arg;
	HttpArg->Fsize = ImgSize;
	HttpArg->RemainingRxLen = ContentLen - (u16)(ImgData - ImgHdr);
	HttpArg->Offset = Offset;
	ImgSizeInThisPkt = HttpReqLen - (u16)(ImgData - HttpReq);

	Xbir_Printf("Starting image update\r\n");
	Status = Xbir_SsiUpdateImg (Tpcb, ImgData, ImgSizeInThisPkt);

	Xbir_SsiLastImgUpload = BootImgId;
	Xbir_SsiLastUploadSize = ImgSize;

END:
	return Status;
}