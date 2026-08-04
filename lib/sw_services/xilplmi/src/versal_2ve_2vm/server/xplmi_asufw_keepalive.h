/***************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xplmi_asufw_keepalive.h
*
* This file contains ASUFW keep-alive monitor APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ==============================================================================
* 1.0   vm   07/14/2026 Initial release
*       vm   07/31/2026 Updated Start API and RTCA address comment
*
* </pre>
*
***************************************************************************************************/

#ifndef XPLMI_ASUFW_KEEPALIVE_H
#define XPLMI_ASUFW_KEEPALIVE_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xil_types.h"
#include "xplmi_asu_update.h"

/************************************ Constant Definitions ****************************************/
#define XPLMI_ASU_RTCA_KEEP_ALIVE_COUNTER_ADDR	(XPLMI_ASU_RTCA_BASEADDR + 0x19CU)
						/**< ASUFW keep-alive counter at RTCA 0x19C;
						 * must match XASU_RTCA_KEEP_ALIVE_COUNTER_ADDR
						 * in xasu_sharedmem.h */
#define XPLMI_ASUFW_KEEP_ALIVE_INTERVAL_MS	(200U)
						/**< ASUFW keep-alive monitor period in ms */

/************************************ Function Prototypes *****************************************/
int XPlmi_AsufwKeepAliveInit(void);
void XPlmi_StartAsufwKeepAliveMonitor(void);

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ASUFW_KEEPALIVE_H */
