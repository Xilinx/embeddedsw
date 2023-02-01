/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_http.c
*
* This file contains functions to process http packets
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xstatus.h"
#include "xbir_config.h"
#include "lwip/tcp.h"
#include "xbir_util.h"
#include "xbir_sys.h"
#include "xbir_ssi.h"
#include "xbir_http.h"
#include <string.h>

/************************** Constant Definitions *****************************/
#define XBIR_HTTP_ARG_ARRAY_SIZE		(1000U)
#define XBIR_HTTP_404_ERR_MAX_RESPONSE_LEN	(1024U)
#define XBIR_HTTP_BUFFER_SIZE			(1400U)
#define XBIR_HTTP_MAX_FILE_NAME_LEN		(50U)

/* HTTP 404 not found error header */
const char *Xbir_HttpNotFoundHdr =
	"<html> \
	<head> \
		<title>404</title> \
		<style type=\"text/css\"> \
		div#request {background: #eeeeee} \
		</style> \
	</head> \
	<body> \
	<h1>404 Page Not Found</h1> \
	<div id=\"request\">";

/* HTTP 404 not found error footer */
const char *Xbir_HttpNotFoundFooter =
	"</div> \
	</body> \
	</html>";

/**************************** Type Definitions *******************************/
/* Type of supported HTTP requests */
typedef enum {
	XBIR_HTTP_GET,
	XBIR_HTTP_POST,
	XBIR_HTTP_HEAD,
	XBIR_HTTP_UNKNOWN
} Xbir_HttpReqType;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static Xbir_HttpReqType Xbir_HttpDecodeReq (char *HttpReq, u16 Len);
static int Xbir_HttpProcessGetReq (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
static int Xbir_HttpProcessPostReq (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
static int Xbir_HttpSendResponse404 (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
static int Xbir_HttpGenerateHdr(char *Buffer, const char *FileExt, u16 FileSize);
static int Xbir_HttpProcessGetFileReq (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
static void Xbir_HttpExtractFileName(char *HttpReq, u16 HttpReqLen,
	char *FileName, u8 MaxFileNameLen);
static int Xbir_HttpProcessGetSysInfoReq (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
static int Xbir_HttpProcessGetBootImgStatusReq (struct tcp_pcb *Tpcb,
	u8 *HttpReq, u16 HttpReqLen);
static int Xbir_HttpSendStatus (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen, u32 InStatus);
static int Xbir_HttpProcessFlashErase(struct tcp_pcb *Tpcb,
	u8 *HttpReq, u16 HttpReqLen, Xbir_SysBootImgId BootImgId);
static int Xbir_HttpProcessFlashEraseStatus(struct tcp_pcb *Tpcb,
	u8 *HttpReq, u16 HttpReqLen);

/************************** Variable Definitions *****************************/
static Xbir_HttpArg Http_ArgArray[XBIR_HTTP_ARG_ARRAY_SIZE];
static int Http_ArgCount;
static int Http_ArgArrayIndex;

/*****************************************************************************/
/**
 * @brief
 * This function processes input HTTP request to send response.
 * This assumes that tcp_sndbuf is high enough to send atleast 1 packet.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS on success
 *		Error code on failure
 *
 *****************************************************************************/
int Xbir_HttpProcessReq (struct tcp_pcb *Tpcb, u8 *HttpReq, u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	Xbir_HttpReqType ReqType;

	ReqType = Xbir_HttpDecodeReq((char *)HttpReq, HttpReqLen);
	switch (ReqType) {
	case XBIR_HTTP_GET:
	case XBIR_HTTP_HEAD:
		Status = Xbir_HttpProcessGetReq(Tpcb, HttpReq, HttpReqLen);
		break;

	case XBIR_HTTP_POST:
		Status = Xbir_HttpProcessPostReq(Tpcb, HttpReq, HttpReqLen);
		break;

	default:
		Xbir_Printf(DEBUG_INFO, " ERROR: RequestType != GET|POST\r\n");
		Status = Xbir_HttpSendResponse404(Tpcb, HttpReq, HttpReqLen);
		break;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function processes additional payload after first packet of
 * every connection.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS on success
 *		Error code on failure
 *
 *****************************************************************************/
int Xbir_HttpProcessAdditionalPayload (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen)
{
	int Status = XST_FAILURE;

	Status = Xbir_SsiProcessAdditionalPayload(Tpcb, HttpReq, HttpReqLen);
	if(Status != XST_SUCCESS) {
		Xbir_HttpClose(Tpcb);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function sends the HTTP Response with JSON payload.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 * @param	JsonStr		Json payload
 * @param	JsonStrLen	Json payload length
 *
 * @return	XST_SUCCESS if request is processed successfully
 *		XST_FAILURE if request is not processed
 *
 *****************************************************************************/
int Xbir_HttpSendResponseJson (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen, char *JsonStr, u16 JsonStrLen)
{
	int Status = XST_FAILURE;
	int Error;
	char Buffer[XBIR_HTTP_BUFFER_SIZE];
	u32 HdrLen;

	/* Send the HTTP headers */
	HdrLen = Xbir_HttpGenerateHdr(Buffer, "jsn", JsonStrLen);

	if (Tpcb->state > ESTABLISHED) {
		Xbir_Printf(DEBUG_INFO, " ERROR :TCP connection is dropped\r\n");
		Xbir_HttpClose(Tpcb);
		goto END;
	}

	Error = tcp_write(Tpcb, Buffer, HdrLen,
		TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
	if (Error != ERR_OK) {
		Xbir_Printf(DEBUG_INFO, " ERROR: (%d) writing http header to socket\r\n",
			Error);
		Xbir_Printf(DEBUG_INFO, " attempted to write #bytes = %d, tcp_sndbuf = %d\r\n",
			HdrLen, tcp_sndbuf(Tpcb));
		Xbir_Printf(DEBUG_INFO, " HTTP header = %s\r\n", Buffer);
		Xbir_HttpClose(Tpcb);
		goto END;
	}

	Error = tcp_write(Tpcb, JsonStr, JsonStrLen,
		TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
	if (Error != ERR_OK) {
		Xbir_Printf(DEBUG_INFO, " Attempted to lwip_write %d bytes,"
			" tcp write error = %d\r\n", JsonStrLen, Error);
		Xbir_HttpClose(Tpcb);
		goto END;
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function allocates the Xbir_HttpArg instance from preallocated static
 * memory.
 *
 * @param	None
 *
 * @return	Pointer to Xbir_HttpArg instance
 *
 *****************************************************************************/
Xbir_HttpArg *Xbir_HttpPallocArg (void)
{
	Xbir_HttpArg *HttpArg = &(Http_ArgArray[Http_ArgArrayIndex]);
	Http_ArgArrayIndex++;
	if (Http_ArgArrayIndex == XBIR_HTTP_ARG_ARRAY_SIZE)
		Http_ArgArrayIndex = 0;
	HttpArg->Count = Http_ArgCount++;
	memset(&HttpArg->Fil, 0, sizeof(HttpArg->Fil));
	HttpArg->Fsize = 0;
	HttpArg->PktCount = 0;

	return HttpArg;
}

/*****************************************************************************/
/**
 * @brief
 * This function releases the Xbir_HttpArg instance of preallocated static
 * memory.
 *
 * @param	Xbir_HttpArg	Pointer to Xbir_HttpArg instance
 *
 * @return	None
 *
 *****************************************************************************/
void Xbir_HttpPfreeArg (Xbir_HttpArg *HttpArg)
{
	memset(&HttpArg->Fil, 0x00, sizeof(HttpArg->Fil));
	memset(HttpArg, 0x00, sizeof(Xbir_HttpArg));
}

/*****************************************************************************/
/**
 * @brief
 * This function determines the HTTP request based on input HTTP request string.
 *
 * @param	HttpReq		Pointer to HTTP request string
 *
 * @return	Xbir_HttpReqType	HTTP request type
 *
 *****************************************************************************/
static Xbir_HttpReqType Xbir_HttpDecodeReq (char *HttpReq, u16 HttpReqLen)
{
	Xbir_HttpReqType ReqType = XBIR_HTTP_UNKNOWN;
	char *GetReqStr = "GET";
	char *PostReqStr = "POST";
	char *HeadReqStr = "HEAD";

	if (strncmp(HttpReq, GetReqStr, strlen(GetReqStr)) == 0) {
		ReqType = XBIR_HTTP_GET;
	}
	else if (strncmp(HttpReq, HeadReqStr, strlen(HeadReqStr)) == 0) {
		ReqType = XBIR_HTTP_HEAD;
	}
	else if (strncmp(HttpReq, PostReqStr, strlen(PostReqStr)) == 0) {
		ReqType = XBIR_HTTP_POST;
	}
	else {
		ReqType = XBIR_HTTP_UNKNOWN;
	}

	return ReqType;
}

/*****************************************************************************/
/**
 * @brief
 * This function processes HTTP GET request and sends the response
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if request is processed successfully
 *		XST_FAILURE if request is not processed
 *
 *****************************************************************************/
static int Xbir_HttpProcessGetReq (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	char FileName[XBIR_HTTP_MAX_FILE_NAME_LEN] = "";
	Xbir_FlashEraseStats *FlashEraseStats = Xbir_GetFlashEraseStats();

	Xbir_HttpExtractFileName((char *)HttpReq, HttpReqLen, FileName,
		XBIR_HTTP_MAX_FILE_NAME_LEN - 1U);

	/*
	 * When Flash Erase is started ignore all commands other than
	 * Flash Erase Status
	 */
	if (FlashEraseStats->State == XBIR_FLASH_ERASE_STARTED) {
		if (strncmp(FileName, "flash_erase_status",
			strlen("flash_erase_status")) != 0) {
			goto END;
		}
	}

	if (strncmp(FileName, "sys_info", strlen("sys_info")) == 0) {
		Xbir_HttpProcessGetSysInfoReq(Tpcb, HttpReq, HttpReqLen);
	}
	else if (strncmp(FileName, "boot_img_status",
		strlen("boot_img_status")) == 0) {
		Xbir_HttpProcessGetBootImgStatusReq(Tpcb, HttpReq, HttpReqLen);
	}
	else if (strncmp(FileName, "flash_erase_imgA",
		strlen("flash_erase_imgA")) == 0) {
		Xbir_HttpProcessFlashErase(Tpcb, HttpReq, HttpReqLen,
				XBIR_SYS_BOOT_IMG_A_ID);
	}
	else if (strncmp(FileName, "flash_erase_imgB",
		strlen("flash_erase_imgB")) == 0) {
		Xbir_HttpProcessFlashErase(Tpcb, HttpReq, HttpReqLen,
				XBIR_SYS_BOOT_IMG_B_ID);
	}
	else if (strncmp(FileName, "flash_erase_imgWIC",
		strlen("flash_erase_imgWIC")) == 0) {
		Xbir_HttpProcessFlashErase(Tpcb, HttpReq, HttpReqLen,
				XBIR_SYS_BOOT_IMG_WIC);
	}
	else if (strncmp(FileName, "flash_erase_status",
		strlen("flash_erase_status")) == 0) {
		Xbir_HttpProcessFlashEraseStatus(Tpcb, HttpReq, HttpReqLen);
	}
	else {
		Xbir_HttpProcessGetFileReq(Tpcb, HttpReq, HttpReqLen);
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function processes HTTP POST request and sends the response
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if request is processed successfully
 *		XST_FAILURE if request is not processed
 *
 *****************************************************************************/
static int Xbir_HttpProcessPostReq (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	char FileName[XBIR_HTTP_MAX_FILE_NAME_LEN] = "";
	char *JsonStr;

	Xbir_HttpExtractFileName((char *)HttpReq, HttpReqLen, FileName,
		XBIR_HTTP_MAX_FILE_NAME_LEN - 1U);

	if (strncmp(FileName, "cfg_boot_img", strlen("cfg_boot_img")) == 0) {
		JsonStr = strstr((char *)HttpReq, "\r\n\r\n");
		if (JsonStr != NULL) {
			JsonStr += strlen("\r\n\r\n");
			Status = Xbir_SsiJsonCfgBootImgStatus(JsonStr,
					strlen(JsonStr));
		}
		else {
			Status = XST_SUCCESS;
		}
		Xbir_HttpSendStatus(Tpcb, HttpReq, HttpReqLen, Status);
	}
	else if (strncmp(FileName, "download_imgA", strlen("download_imgA")) == 0) {
		Status = Xbir_SsiUpdateImgA(Tpcb, HttpReq, HttpReqLen);
	}
	else if (strncmp(FileName, "download_imgB", strlen("download_imgB")) == 0) {
		Status = Xbir_SsiUpdateImgB(Tpcb, HttpReq, HttpReqLen);
	}
	else if (strncmp(FileName, "download_imgWIC", strlen("download_imgWIC")) == 0) {
		Status = Xbir_SsiUpdateImgWIC(Tpcb, HttpReq, HttpReqLen);
	}
	else if (strncmp(FileName, "validate_crc", strlen("validate_crc")) == 0) {
		JsonStr = strstr((char *)HttpReq, "\r\n\r\n");
		if (JsonStr != NULL) {
			JsonStr += strlen("\r\n\r\n");
			Status = Xbir_SsiValidateLastUpdate(JsonStr,
				strlen(JsonStr));
		}
		Status = Xbir_HttpSendStatus(Tpcb, HttpReq, HttpReqLen, Status);
	}
	else {
		Xbir_HttpSendResponse404(Tpcb, HttpReq, HttpReqLen);
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function dynamically generates HTTP 404 error response and sends it.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if request is processed successfully
 *		XST_FAILURE if request is not processed
 *
 *****************************************************************************/
static int Xbir_HttpSendResponse404 (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	u16 Len;
	u16 Hlen;
	char Buffer[XBIR_HTTP_404_ERR_MAX_RESPONSE_LEN];
	err_t Error;

	Len = strlen(Xbir_HttpNotFoundHdr) + strlen(Xbir_HttpNotFoundFooter) +
		HttpReqLen;

	Hlen = Xbir_HttpGenerateHdr(Buffer, "html", Len);

	if (tcp_sndbuf(Tpcb) < Hlen) {
		Xbir_Printf(DEBUG_INFO, " ERROR : Cannot send 404 message, "
			"tcp_sndbuf = %d bytes, message length = %d bytes\r\n",
			tcp_sndbuf(Tpcb), Hlen);
		goto END;
	}

	Error = tcp_write(Tpcb, Buffer, Hlen, TCP_WRITE_FLAG_COPY);
	if (Error != ERR_OK) {
		Xbir_Printf(DEBUG_INFO, " ERROR: (%d) writing 404 http header\r\n", Error);
		goto END;
	}

	Error = tcp_write(Tpcb, Xbir_HttpNotFoundHdr,
		strlen(Xbir_HttpNotFoundHdr), TCP_WRITE_FLAG_COPY);
	if (Error != ERR_OK) {
		Xbir_Printf(DEBUG_INFO, " ERROR: (%d) writing not found header\r\n", Error);
		goto END;
	}

	Error = tcp_write(Tpcb, HttpReq, HttpReqLen, TCP_WRITE_FLAG_COPY);
	if (Error != ERR_OK) {
		Xbir_Printf(DEBUG_INFO, " ERROR: (%d) writing org req\r\n", Error);
		goto END;
	}

	Error = tcp_write(Tpcb, Xbir_HttpNotFoundFooter,
		strlen(Xbir_HttpNotFoundFooter), TCP_WRITE_FLAG_COPY);
	if (Error != ERR_OK) {
		Xbir_Printf(DEBUG_INFO, " ERROR: (%d) writing not found footer\r\n", Error);
		goto END;
	}

	Status = XST_SUCCESS;

END:
	Xbir_HttpClose(Tpcb);
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function dynamically generates HTTP header based on file type and size.
 *
 * @param	Buffer	Pointer to buffer to store HTTP Header
 * @param	Ext	Extension of file or data type string
 * @param	Size	Size of file or size of data
 *
 * @return	Len	Length of the HTTP header
 *
 *****************************************************************************/
static int Xbir_HttpGenerateHdr (char *Buffer, const char *Ext, u16 Size)
{
	char LocalBuffer[40U];
	char *HttpHdr = (char *) Buffer;

	strcpy(HttpHdr, "HTTP/1.1 200 OK\r\nContent-Type: ");

	if (Ext == NULL) {
		strcat(HttpHdr, "text/html");	/* For unknown types */
	}
	else if (strncmp(Ext, "htm", strlen("htm")) == 0) {
		strcat(HttpHdr, "text/html");
	}
	else if (strncmp(Ext, "jsn", strlen("jsn")) == 0) {
		strcat(HttpHdr, "application/json");
	}
	else if (strncmp(Ext, "js", strlen("js")) == 0) {
		strcat(HttpHdr, "application/javascript");
	}
	else if (strncmp(Ext, "css", strlen("css")) == 0) {
		strcat(HttpHdr, "text/css");
	}
	else if (strncmp(Ext, "ico", strlen("ico")) == 0) {
		strcat(HttpHdr, "image/ico");
	}
	else if (strncmp(Ext, "svg", strlen("svg")) == 0) {
		strcat(HttpHdr, "image/svg+xml");
	}
	else {
		/* For unknown types */
		strcat(HttpHdr, "text/plain");
	}
	strcat(HttpHdr, "\r\n");

	sprintf(LocalBuffer, "Content-length: %d", Size);
	strcat(HttpHdr, LocalBuffer);
	strcat(HttpHdr, "\r\n");

	strcat(HttpHdr, "Connection: close\r\n");
	strcat(HttpHdr, "\r\n");

	return strlen(HttpHdr);
}

/*****************************************************************************/
/**
 * @brief
 * This function processes HTTP GET command for accessing web server files.
 * It initiates file transfer if request is ligitimate, else sends HTTP 404
 * error.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if request is processed successfully
 *		XST_FAILURE if request is not processed
 *
 *****************************************************************************/
static int Xbir_HttpProcessGetFileReq (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	int Error;
	char FileName[XBIR_HTTP_MAX_FILE_NAME_LEN] = "";
	char Buffer[XBIR_HTTP_BUFFER_SIZE];
	FIL Fil;
	const char *FileExt;
	u32 HdrLen;
	u32 FileSize;
	u32 DataReadLen;
	FRESULT Result;

	Xbir_HttpExtractFileName((char *)HttpReq, HttpReqLen, FileName,
		XBIR_HTTP_MAX_FILE_NAME_LEN - 1U);

	/* Check if specified file present in web server's FAT file system */
	Result = f_open(&Fil, FileName, FA_READ);
	if (Result) {
		Xbir_Printf(DEBUG_INFO, " Error: Requested file %s not found, returning 404\r\n",
			   FileName);
		Xbir_HttpSendResponse404(Tpcb, HttpReq, HttpReqLen);
		goto END;
	}

	Xbir_Printf(DEBUG_INFO, " \r\n HTTP GET: %s\r\n", FileName);
	FileExt = Xbir_UtilGetFileExt(FileName);
	FileSize = f_size(&Fil);

	/* Send the HTTP headers */
	HdrLen = Xbir_HttpGenerateHdr(Buffer, FileExt, FileSize);
	Error = tcp_write(Tpcb, Buffer, HdrLen,
		TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
	if (Error != ERR_OK) {
		Xbir_Printf(DEBUG_INFO, " ERROR: (%d) writing http header to socket\r\n",
				Error);
		Xbir_Printf(DEBUG_INFO, " attempted to write #bytes = %d, tcp_sndbuf = %d\r\n",
			HdrLen, tcp_sndbuf(Tpcb));
		Xbir_Printf(DEBUG_INFO, " HTTP header = %s\r\n", Buffer);
		f_close(&Fil);
		Xbir_HttpClose(Tpcb);
		goto END;
	}

	/* Now send the file */
	while (FileSize > 0U) {
		int SndBuffer = tcp_sndbuf(Tpcb);
		if (SndBuffer < XBIR_HTTP_BUFFER_SIZE) {
			/* Not enough space in sndbuf, so send remaining bytes
			 * when there is space. this is done by storing the file
			 * descriptor in as part of the tcp_arg, so that the sent
			 * callback handler knows to send data
			 */
			Xbir_HttpArg *HttpArg = (Xbir_HttpArg *)Tpcb->callback_arg;
			memcpy(&HttpArg->Fil, &Fil, sizeof(Fil));
			HttpArg->Fsize = FileSize;
			goto END;
		}

		f_read(&Fil, (void *)Buffer, XBIR_HTTP_BUFFER_SIZE, &DataReadLen);
		Error = tcp_write(Tpcb, Buffer, DataReadLen,
			TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
		if (Error != ERR_OK) {
			Xbir_Printf(DEBUG_INFO, " Error: writing file (%s) to socket,"
				"remaining unwritten bytes = %d\r\n",
				FileName, FileSize - DataReadLen);
			Xbir_Printf(DEBUG_INFO, " Attempted to lwip_write %d bytes,"
				" tcp write error = %d\r\n", DataReadLen, Error);
			Xbir_HttpClose(Tpcb);
			break;
		}

		FileSize -= DataReadLen;
	}

	f_close(&Fil);
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function extracts the file name from the HTTP request.
 *
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 * @param	FileName	Pointer to memory for storing file name
 * @param	FileNameSize	Memory available for storing file name
 *
 * @return	None
 *
 *****************************************************************************/
static void Xbir_HttpExtractFileName(char *HttpReq, u16 HttpReqLen,
	char *FileName, u8 FileNameSize)
{
	char *Start, *End;
	u16 Offset;

	/* First locate the file name in the request.
	 * Requests are of the form GET /path/to/filename HTTP... OR
	 * POST /path/to/filename HTTP...
	 */
	if (strcmp(HttpReq, "GET ") == 0)
		Offset = strlen("GET ");
	else {
		Offset = strlen("POST ");
	}

	if (HttpReq[Offset] == '/')
		Offset++;

	Start = HttpReq + Offset;   /* Start marker */

	/* File name finally ends in a space */
	while (HttpReq[Offset] != ' ')
		Offset++;

	End = HttpReq + Offset - 1; /* End marker */

	if (End < Start) {
		strcpy(FileName, "index.htm");
		goto END;
	}

	/* If there is something wrong with the URL & we ran for for more than
	 * the HTTP buffer length (Offset > HttpReqLen) or the file name is too
	 * long, throw 404 error */
	if ((Offset > HttpReqLen) ||
		((End - Start) > FileNameSize)) {
		*End = 0;
		strcpy(FileName, "404.htm");
		Xbir_Printf(DEBUG_INFO, " ERROR: Request filename is too long, length = %d,"
			"file = %s (truncated), max = %d\r\n",
			(int)(End - Start), Start, FileNameSize);
		goto END;
	}

	/* Copy over the filename */
	strncpy(FileName, Start, End - Start + 1U);
	FileName[End - Start + 1U] = 0U;

	/* If last character is a '/', append index.htm */
	if (*End == '/')
		strcat(FileName, "index.htm");

END:
	return;
}

/*****************************************************************************/
/**
 * @brief
 * This function processes HTTP GET system information request.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if request is processed successfully
 *		XST_FAILURE if request is not processed
 *
 *****************************************************************************/
static int Xbir_HttpProcessGetSysInfoReq (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	char Buffer[XBIR_HTTP_BUFFER_SIZE];

	Status = Xbir_SsiJsonBuildSysInfo(Buffer, XBIR_HTTP_BUFFER_SIZE);

	if (XST_SUCCESS == Status) {
		Status = Xbir_HttpSendResponseJson(Tpcb, HttpReq, HttpReqLen,
			 Buffer, strlen(Buffer));
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function processes HTTP GET boot image status request.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if request is processed successfully
 *		XST_FAILURE if request is not processed
 *
 *****************************************************************************/
static int Xbir_HttpProcessGetBootImgStatusReq (struct tcp_pcb *Tpcb,
	u8 *HttpReq, u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	char Buffer[100U] = {0U};

	Status = Xbir_SsiJsonBuildBootImgStatus(Buffer, 100U);
	if (Status == XST_SUCCESS) {
		Status = Xbir_HttpSendResponseJson(Tpcb, HttpReq, HttpReqLen,
			Buffer, strlen(Buffer));
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function processes HTTP GET Flash erase request.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 * @param	BootImgId	Boot Image ID
 *
 * @return	XST_SUCCESS if request is processed successfully
 *		XST_FAILURE if request is not processed
 *
 *****************************************************************************/
static int Xbir_HttpProcessFlashErase(struct tcp_pcb *Tpcb,
	u8 *HttpReq, u16 HttpReqLen, Xbir_SysBootImgId BootImgId)
{
	int Status = XST_FAILURE;

	Status = Xbir_SysEraseBootImg(BootImgId);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xbir_HttpProcessFlashEraseStatus(Tpcb, HttpReq, HttpReqLen);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function processes HTTP GET Flash erase status request.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 *
 * @return	XST_SUCCESS if request is processed successfully
 *		XST_FAILURE if request is not processed
 *
 *****************************************************************************/
static int Xbir_HttpProcessFlashEraseStatus(struct tcp_pcb *Tpcb,
	u8 *HttpReq, u16 HttpReqLen)
{
	int Status = XST_FAILURE;
	char Buffer[100U] = {0U};

	Status = Xbir_SsiJsonBuildFlashEraseStatus(Buffer, 100U);
	if (Status == XST_SUCCESS) {
		Status = Xbir_HttpSendResponseJson(Tpcb, HttpReq, HttpReqLen,
			Buffer, strlen(Buffer));
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function sends the input Status in JSON formave over HTTP as a response.
 *
 * @param	Tpcb		Pointer to TCP PCB
 * @param	HttpReq		Pointer to HTTP payload
 * @param	HttpReqLen	HTTP payload length
 * @param	Status		Status to be sent
 *
 * @return	XST_SUCCESS if request is processed successfully
 *		XST_FAILURE if request is not processed
 *
 *****************************************************************************/
static int Xbir_HttpSendStatus (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen, u32 InStatus)
{
	int Status = XST_FAILURE;
	if ( XST_SUCCESS == InStatus) {
		Status = Xbir_HttpSendResponseJson(Tpcb, HttpReq, HttpReqLen,
			"{\"Status\":\"Success\"}",
			strlen("{\"Status\":\"Success\"}"));
	}
	else {
		Status = Xbir_HttpSendResponseJson(Tpcb, HttpReq, HttpReqLen,
			"{\"Status\":\"Failed\"}",
			strlen("{\"Status\":\"Failed\"}"));
	}

	return Status;
}
