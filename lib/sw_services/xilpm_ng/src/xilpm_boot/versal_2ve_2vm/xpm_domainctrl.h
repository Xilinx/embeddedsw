/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_DOMAINCTRL_H_
#define XPM_DOMAINCTRL_H_

#if defined(XILPM_NG_DOMAIN_CONTROL_GPIO)

#include "xpm_node.h"
#include "xpm_power.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_GPIO_DOMAIN_CTRL_DOMAINS 	2U /**< Maximum number of GPIO domain control domains */
#define MAX_GPIO_CTRL_CMD_LEN 		3U /**< Number of arguments for GPIO control command: Offset, Mask and Value */
#define MAX_DOMAIN_CONTROL_MODES	2U /**< Maximum number of domain control modes */
#define GPIO_DOMAIN_CTRL_CMDLEN_SHIFT 	8U /**< Shift for command length in GPIO domain control arguments */
#define GPIO_DOMAIN_CTRL_MODE_MASK 	0xFFU /**< Mask for (CmdLen|Mode) in GPIO domain control arguments */

typedef enum {
	XPM_DOMAIN_CONTROL_GPIO = 0x2U /**< Domain control through GPIO */
} XPm_DomainCtrlType;

/* Forward declaration to avoid circular include */
typedef struct XPm_PowerDomain XPm_PowerDomain;
typedef struct {
	u16 Offset;			/* GPIO Register address offset */
	u32 Mask;			/* GPIO pin mask */
	u32 Value;			/* GPIO pin value */
} XPmDomainCtrl_GPIO;
typedef struct {
	u32 GpioNodeId;						/* GPIO Node Id */
	XPmDomainCtrl_GPIO GpioCtrl[MAX_DOMAIN_CONTROL_MODES];	/* GPIO Configuration for each mode */
} XPm_DomainCtrl;

XStatus XPmDomainCtrl_Init(XPm_DomainCtrl *DomainCtrl, u32 DomainCtrlId, const u32 *Args, u32 NumArgs);
XStatus XPmDomainCtrl_Control(const XPm_PowerDomain *PwrDomain, u32 State);

#ifdef __cplusplus
}
#endif
#endif /* XILPM_NG_DOMAIN_CONTROL_GPIO */
/** @} */
#endif /* XPM_DOMAINCTRL_H_ */
