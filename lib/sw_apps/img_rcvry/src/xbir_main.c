/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xbir_main.c
*
* This file contains startup code for the Xilinx boot image recovery tool
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "netif/xadapter.h"
#include "xbir_platform.h"
#include "xbir_config.h"
#include "xbir_ws.h"
#include "xbir_nw.h"
#include "xbir_sys.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static void Xbir_PrintAppBanner (struct netif *NetIf);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief
 * This functions configured and starts the recovery image based on the
 * web server application.
 *
 * @param	None
 *
 * @return	This function ideally should never return.
 *
 *****************************************************************************/
int main (void)
{
	int Status = XST_FAILURE;
	struct netif NetIf = {0U};

	Status = Xbir_Platform_Init();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xbir_SysInit();
	if (XST_SUCCESS != Status) {
		Xbir_Printf(DEBUG_INFO, " ERROR: System intialization failed....");
		goto END;
	}

	Status = Xbir_NwCfgNetwork(&NetIf);
	if (XST_SUCCESS != Status) {
		Xbir_Printf(DEBUG_INFO, " ERROR: Network configuration failed....");
		goto END;
	}

	Status = Xbir_WsStart();
	if (Status == XST_SUCCESS) {
		Xbir_PrintAppBanner(&NetIf);
		Xbir_NwProcessPkts(&NetIf);
	}
	else {
		Xbir_Printf(DEBUG_INFO, " ERROR: Web server setup failed....");
	}

END:
	Xbir_Printf(DEBUG_INFO, " \r\n\r\nApplication closed..........\r\n");
	return Status;
}

/*****************************************************************************/
/**
 * @brief
 * This functions displays Xilinx boot image recovery tool web server banner.
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
static void Xbir_PrintAppBanner (struct netif *NetIf)
{
	Xbir_Printf(DEBUG_PRINT_ALWAYS, "Xilinx boot image recovery tool web server is running on port %d\r\n",
		XBIR_NW_HTTP_PORT);
	Xbir_Printf(DEBUG_PRINT_ALWAYS, "Please point your web browser to http://%u.%u.%u.%u",
		ip4_addr1(&NetIf->ip_addr),
		ip4_addr2(&NetIf->ip_addr),
		ip4_addr3(&NetIf->ip_addr),
		ip4_addr4(&NetIf->ip_addr));
	Xbir_Printf(DEBUG_PRINT_ALWAYS, "\r\n");
}
