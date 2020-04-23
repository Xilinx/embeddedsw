/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPFW_AIB_H_
#define XPFW_AIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xstatus.h"
#include "xil_types.h"


enum XPfwAib {
	XPFW_AIB_RPU0_TO_LPD,
	XPFW_AIB_RPU1_TO_LPD,
	XPFW_AIB_LPD_TO_DDR,
	XPFW_AIB_LPD_TO_FPD,
	XPFW_AIB_LPD_TO_RPU0,
	XPFW_AIB_LPD_TO_RPU1,
	XPFW_AIB_LPD_TO_USB0,
	XPFW_AIB_LPD_TO_USB1,
	XPFW_AIB_LPD_TO_OCM,
	XPFW_AIB_LPD_TO_AFI_FS2,
	XPFW_AIB_FPD_TO_LPD_NON_OCM,
	XPFW_AIB_FPD_TO_LPD_OCM,
	XPFW_AIB_FPD_TO_AFI_FS0,
	XPFW_AIB_FPD_TO_AFI_FS1,
	XPFW_AIB_FPD_TO_GPU,
	XPFW_AIB_ID_MAX
};

/**
 * Wait times for HW related actions
 */
#define AIB_ACK_TIMEOUT	(0xFU)

void XPfw_AibEnable(enum XPfwAib AibId);
void XPfw_AibDisable(enum XPfwAib AibId);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_AIB_H_ */
