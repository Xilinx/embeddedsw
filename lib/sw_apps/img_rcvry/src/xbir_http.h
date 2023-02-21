/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_http.h
*
* This file contains list of APIs provided by HTTP module (xbir_http.c file)
*
******************************************************************************/
#ifndef XBIR_HTTP_H
#define XBIR_HTTP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "ff.h"

/************************** Constant Definitions *****************************/
#define XBIR_HTTP_MAX_BOUNDARY_LEN	(1024U)


/* Clear callback functions and close connection */
static inline void Xbir_HttpClose(struct tcp_pcb * Tpcb)
{
	tcp_recv(Tpcb, NULL);
	tcp_sent(Tpcb, NULL);
	tcp_close(Tpcb);
}

/**************************** Type Definitions *******************************/
typedef struct {
	u32 Count;		/* Http Arg count */
	FIL Fil;		/* File Handler */
	u32 Fsize;		/* File size */
	u32 PktCount;		/* Packet count */
	u32 Offset;		/* File offset during file upload */
	u32 ImgId;		/* Image A /Image B */
	u32 ContentLen;		/* Content Length of HTTP Req */
	u32 BoundaryLen;	/* Boundary Len of Boundary in HTTP Req */
	u8 Boundary[XBIR_HTTP_MAX_BOUNDARY_LEN + 1U]; /* Boundary marker in
				HTTP Req */
} Xbir_HttpArg;

/************************** Function Prototypes ******************************/
int Xbir_HttpProcessReq (struct tcp_pcb *Tpcb, u8 *HttpReq, u16 HttpReqLen);
int Xbir_HttpProcessAdditionalPayload (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
int Xbir_HttpSendResponseJson (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen, char *JsonStr, u16 JsonStrLen);
Xbir_HttpArg *Xbir_HttpPallocArg (void);
void Xbir_HttpPfreeArg (Xbir_HttpArg *Arg);

#ifdef __cplusplus
}
#endif

#endif
