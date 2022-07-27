/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#ifndef XPM_POWER_H_
#define XPM_POWER_H_

#include "xpm_defs.h"
#include "xpm_node.h"
#include "xpm_power_plat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	/* Default FSM states */
	XPM_POWER_STATE_OFF = 0,
	XPM_POWER_STATE_INITIALIZING,
	XPM_POWER_STATE_ON,
	XPM_POWER_STATE_STANDBY,
	XPM_POWER_STATE_PWR_UP_PARENT,
	XPM_POWER_STATE_PWR_DOWN_PARENT,
	XPM_POWER_STATE_PWR_UP_SELF,
	XPM_POWER_STATE_PWR_DOWN_SELF,
} XPm_PowerState;

typedef enum {
	XPM_POWER_EVENT_PWR_UP,
	XPM_POWER_EVENT_PARENT_UP_DONE,
	XPM_POWER_EVENT_SELF_UP_DONE,
	XPM_POWER_EVENT_PWR_DOWN,
	XPM_POWER_EVENT_SELF_DOWN_DONE,
	XPM_POWER_EVENT_PARENT_DOWN_DONE,
	XPM_POWER_EVENT_TIMER,
} XPm_PowerEvent;

#define MAX_I2C_COMMAND_LEN	16

typedef struct {
	u8 CmdLen; /** Total no of commands to configure this regulator */
	u8 CmdArr[MAX_I2C_COMMAND_LEN]; /** Array of i2c command bytes. For example, Len1,bytes, Len2, bytes, Len3,bytes etc */
} XPm_I2cCmd;

/************************** Function Prototypes ******************************/
XPm_Power *XPmPower_GetById(u32 Id);
XStatus XPmPower_Init(XPm_Power *Power,
	u32 Id, u32 BaseAddress, XPm_Power *Parent);
XStatus XPmPower_AddParent(u32 Id, const u32 *Parents, u32 NumParents);
XStatus XPmPower_GetStatus(const u32 SubsystemId, const u32 DeviceId, XPm_DeviceStatus *const DeviceStatus);
XStatus XPmPower_GetWakeupLatency(const u32 DeviceId, u32 *Latency);
XStatus XPmPower_ForcePwrDwn(u32 NodeId);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWER_H_ */
