/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file ssi.h
*
* This file contains list of APIs provided by server side interface (ssi.c file)
*
******************************************************************************/
#ifndef XBIR_SSI_H
#define XBIR_SSI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "lwip/tcp.h"
#include "xbir_sys.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int Xbir_SsiJsonBuildSysInfo (char *JsonStr, u16 JsonStrLen);
int Xbir_SsiJsonBuildBootImgStatus (char *JsonStr, u16 JsonStrLen);
int Xbir_SsiJsonBuildFlashEraseStatus(char *JsonStr, u16 JsonStrLen);
int Xbir_SsiJsonCfgBootImgStatus (char *JsonStr, u16 JsonStrLen);
int Xbir_SsiProcessAdditionalPayload (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
int Xbir_SsiUpdateImgA (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
int Xbir_SsiUpdateImgB (struct tcp_pcb *Tpcb, u8 *HttpReq,
	u16 HttpReqLen);
u32 Xbir_SsiValidateLastUpdate (char *JsonStr, u16 JsonStrLen);
int Xbir_SsiUpdateImgWIC (struct tcp_pcb *Tpcb, u8 *HttpReq, u16 HttpReqLen);

#ifdef __cplusplus
}
#endif

#endif
