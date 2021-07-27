/*
* Copyright (c) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


/*********************************************************************
 * USB slaves data structures
 *********************************************************************/

#ifndef PM_USB_H_
#define PM_USB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_slave.h"
#include "xpfw_aib.h"

/*********************************************************************
 * Macro definitions
 ********************************************************************/
#define USB3_0_FPD_PIPE_CLK				0xFF9D007CU
#define USB3_0_FPD_PIPE_CLK_OPTION_MASK			0x00000001U

/*********************************************************************
 * Structure definitions
 ********************************************************************/
/**
 * PmSlaveUsb - Structure used for Usb
 * @slv         Base slave structure
 * @PwrDn   Pointer to a power down pmu-rom handler
 * @PwrUp   Pointer to a power up pmu-rom handler
 * @rstId   USB reset ID
 */
typedef struct PmSlaveUsb {
	PmSlave slv;
	PmTranHandler PwrDn;
	PmTranHandler PwrUp;
	const u32 rstId;
	const enum XPfwAib aibId;
} PmSlaveUsb;

/*********************************************************************
 * Global data declarations
 ********************************************************************/
extern PmSlaveUsb pmSlaveUsb0_g;
extern PmSlaveUsb pmSlaveUsb1_g;

#ifdef __cplusplus
}
#endif

#endif /* PM_USB_H_ */
