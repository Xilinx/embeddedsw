/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_nw.h
*
* This file contains list of APIs provided by nw module (xbir_nw.c file)
*
******************************************************************************/

#ifndef XBIR_NW_H
#define XBIR_NW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/* Default system network configuration */
#define XBIR_NW_DEFAULT_IP_ADDRESS	"192.168.0.111"
#define XBIR_NW_DEFAULT_IP_MASK		"255.255.255.0"
#define XBIR_NW_DEFAULT_GW_ADDRESS	"192.168.0.1"
/* Protocol definition */
#define XBIR_NW_HTTP_PORT	(80U)

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/
int Xbir_NwCfgNetwork (struct netif *ServerNetIf);
void Xbir_NwPrintIpCfg (ip_addr_t *Ip, ip_addr_t *Mask, ip_addr_t *Gw);
void Xbir_NwProcessPkts (struct netif* ServerNetIf);

#ifdef __cplusplus
}
#endif

#endif
