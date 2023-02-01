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
#include "ff.h"
#include "lwip/err.h"
#include "lwip/tcp.h"
#include "xbir_config.h"
#include "xstatus.h"
#include "xbir_nw.h"
#include "xbir_http.h"
#include "xbir_ws.h"

/************************** Constant Definitions *****************************/
#define XBIR_WS_SND_BUF_SIZE		(1400U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static int Xbir_WsInitFs (void);
static err_t Xbir_WsHttpSentCallback (void *Arg, struct tcp_pcb *Tpcb,
	u16_t Len);
static err_t Xbir_WsHttpRecvCallback (void *Arg, struct tcp_pcb *Tpcb,
	struct pbuf *Pkt, err_t Error);
static err_t Xbir_WsHttpAcceptCallback (void *Arg, struct tcp_pcb *Tpcb,
	err_t Error);

/************************** Variable Definitions *****************************/
/* Static variables controlling debug printf's in this file */
#define XBIR_WS_ENABLE_DEBUG 	(0U)

/*****************************************************************************/
/**
 * @brief
 * This functions configures and starts the web server.
 *
 * @param	None
 *
 * @return	XST_SUCCESS if web server successfully started
 *		XST_FAILURE if web server failed to start
 *
 *****************************************************************************/
int Xbir_WsStart (void)
{
	int Status = XST_FAILURE;
	struct tcp_pcb *TcpPcb;
	err_t Error;

	/* Initialize RAM based file system for web pages */
	Status = Xbir_WsInitFs();
	if (Status != XST_SUCCESS) {
		Xbir_Printf(DEBUG_INFO, " ERROR: Can't run webserver as FS init failed\r\n");
		goto END;
	}

	Status = XST_FAILURE;
	TcpPcb = tcp_new();
	if (TcpPcb == NULL) {
		Xbir_Printf(DEBUG_INFO, " Error: Creating PCB. Out of Memory\r\n");
		goto END;
	}

	Error = tcp_bind(TcpPcb, IP_ADDR_ANY, XBIR_NW_HTTP_PORT);
	if (Error != ERR_OK) {
		Xbir_Printf(DEBUG_INFO, " ERROR: Unable to bind to port 80: err = %d\r\n",
			Error);
		goto END;
	}

	/* We do not need any arguments to the first callback */
	tcp_arg(TcpPcb, NULL);

	TcpPcb = tcp_listen(TcpPcb);
	if (TcpPcb == NULL) {
		Xbir_Printf(DEBUG_INFO, " ERROR: Out of memory while tcp_listen\r\n");
		goto END;
	}

	/* Specify callback to use for incoming connections */
	tcp_accept(TcpPcb, Xbir_WsHttpAcceptCallback);

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This function initializes the FAT32 file system required by web server for
 * hosting web pages.
 *
 * @param	NetIf	Pointer to network interface instance
 *
 * @return	XST_SUCCESS if initialization is successful
 *		XST_FAILURE if initialization is failed
 *
 *****************************************************************************/
static int Xbir_WsInitFs (void)
{
	int Status = XST_FAILURE;
	static FATFS FatFs;
	static FIL FilObj;
	FRESULT Result;
	TCHAR *Path = "0:/";

	/* Register volume work area, initialize device */
	Result = f_mount(&FatFs, Path, 1);
	if (Result != FR_OK) {

		/* TBD: Do we need to format FAT? What about files */
		Xbir_Printf(DEBUG_INFO, " ERROR: Failed to mount FAT FS\r\n");
		goto END;
	}

	/* Try to open default html file */
	Result = f_open(&FilObj, "index.htm", FA_READ);
	if (Result != FR_OK) {
		Xbir_Printf(DEBUG_INFO, " ERROR: Unable to locate index.htm in FS\r\n");
		goto END;
	}

	f_close(&FilObj);
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This is callback function called after data has successfully been received
 * (i.e., acknowledged) by the remote host.
 *
 * @param	Arg	Pointer to Xbir_HttpArg instance
 * @param	Tpcb	Pointer to TCP PCB instance
 * @param	Len	The amount of byte acknowledged
 *
 * @return	ERR_OK 	Callback is executed successfully
 *		ERR_ABRT can be returned if it is decided to abort the
 * 			connection in future after calling tcp_abort
 *
 *****************************************************************************/
static err_t Xbir_WsHttpSentCallback (void *Arg, struct tcp_pcb *Tpcb,
	u16_t Len)
{
	u32 SndBuffer;
	u32 DataLen;
	char Buffer[XBIR_WS_SND_BUF_SIZE];
	Xbir_HttpArg *HttpArg = (Xbir_HttpArg *) Arg;

#if XBIR_WS_ENABLE_DEBUG
		Xbir_Printf(DEBUG_INFO, " %u (%u): S%u..\r\n", HttpArg ? HttpArg->Count : 0U,
			Tpcb->state, Len);
#endif

	if (Tpcb->state > ESTABLISHED) {
		Xbir_Printf(DEBUG_INFO, " TCP connection is dropped\r\n");
		if (HttpArg != NULL) {
			f_close(&HttpArg->Fil);
			Xbir_HttpPfreeArg(HttpArg);
			HttpArg = NULL;
		}

		Xbir_HttpClose(Tpcb);
		goto END;
	}

	 /* No more data to be sent */
	if (HttpArg->Fsize <= 0) {
		f_close(&HttpArg->Fil);
		goto END;
	}

	/* Read more data out of the file and send it */
	do {
		err_t Error;
		SndBuffer = tcp_sndbuf(Tpcb);
		if (SndBuffer < XBIR_WS_SND_BUF_SIZE) {
			goto END;
		}

		Xbir_Printf(DEBUG_INFO, " Attempting to read %u bytes, left = %u bytes\r\n",
			   XBIR_WS_SND_BUF_SIZE, HttpArg->Fsize);

		f_read(&HttpArg->Fil, Buffer, XBIR_WS_SND_BUF_SIZE,
			(unsigned int *)&DataLen);
		Error = tcp_write(Tpcb, Buffer, DataLen, TCP_WRITE_FLAG_COPY);
		if (Error != ERR_OK) {
			Xbir_Printf(DEBUG_INFO, " ERROR: Failed to write data; aborting\r\n");
			Xbir_HttpClose(Tpcb);
			break;
		}

		HttpArg->Fsize -= DataLen;
	} while (HttpArg->Fsize > 0U);

	f_close(&HttpArg->Fil);

END:
	return ERR_OK;
}

/*****************************************************************************/
/**
 * @brief
 * This is callback function called after data has been received.
 *
 * @param	Arg	Pointer to Xbir_HttpArg instance
 * @param	Tpcb	Pointer to TCP PCB instance
 * @param	Pkt	Pointer to packet payload
 * @param	Error	An error code if there has been an error receiving
 *
 * @return	ERR_OK 	Callback is executed successfully
 *		ERR_ABRT can be returned if it is decided to abort the
 * 			connection in future after calling tcp_abort
 *
 *****************************************************************************/
static err_t Xbir_WsHttpRecvCallback (void *Arg, struct tcp_pcb *Tpcb,
	struct pbuf *Pkt, err_t Error)
{
	err_t OutputError = ERR_VAL;
	Xbir_HttpArg *HttpArg = (Xbir_HttpArg *) Arg;

	if ((Error != ERR_OK) || (Pkt == NULL)) {
		Xbir_HttpClose(Tpcb);
		OutputError = ERR_OK;
		goto END;
	}

#if XBIR_WS_ENABLE_DEBUG
		Xbir_Printf(DEBUG_INFO, " %u (%u): R%u %u..\r\n",
			(HttpArg != NULL) ? HttpArg->Count : 0U,
			Tpcb->state, Pkt->len, Pkt->tot_len);
#endif

	/* Do not read the packet if we are not in ESTABLISHED state */
	if (Tpcb->state >= FIN_WAIT_1) {
		pbuf_free(Pkt);
		OutputError = ERR_MEM;
		goto END;
	}

	/* Acknowledge that we've read the payload */
	tcp_recved(Tpcb, Pkt->len);

	HttpArg->PktCount++;

	/* Is it first packet after connection? */
	if(HttpArg->PktCount == 1) {
		/* Read and decipher the request
		 * This function takes care of generating a response, sending it,
		 * and closing the connection if all data have been sent. If
		 * not, then it sets up the appropriate arguments to the sent
		 * callback handler.
		 */

		Xbir_HttpProcessReq(Tpcb, Pkt->payload, Pkt->len);
	}
	else {
		Xbir_HttpProcessAdditionalPayload(Tpcb, Pkt->payload, Pkt->len);
	}

	pbuf_free(Pkt);
	OutputError = ERR_OK;

END:
	return OutputError;
}

/*****************************************************************************/
/**
 * @brief
 * This is callback function called after receiving new connection.
 *
 * @param	Arg	Pointer to Xbir_HttpArg instance
 * @param	Tpcb	Pointer to TCP PCB instance
 * @param	Error	An error code if there has been an error accepting
 *
 * @return	ERR_OK 	Callback is executed successfully
 *		ERR_ABRT can be returned if it is decided to abort the
 * 			connection in future after calling tcp_abort
 *
 *****************************************************************************/
static err_t Xbir_WsHttpAcceptCallback (void *Arg, struct tcp_pcb *Tpcb,
	err_t Error)
{
	/* Keep a count of connection # */
	tcp_arg(Tpcb, (void *)Xbir_HttpPallocArg());

	tcp_recv(Tpcb, Xbir_WsHttpRecvCallback);
	tcp_sent(Tpcb, Xbir_WsHttpSentCallback);

	return ERR_OK;
}
