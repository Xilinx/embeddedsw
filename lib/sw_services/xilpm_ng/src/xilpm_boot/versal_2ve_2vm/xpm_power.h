/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XPM_POWER_H_
#define XPM_POWER_H_

#include "xpm_defs.h"
#include "xpm_node.h"

#ifdef __cplusplus
extern "C" {
#endif
/* TODO: Added below for compilation only. Need to update/delete */
/* Support for up to 7 words of data for I2C commands */
#define MAX_I2C_COMMAND_LEN	28
#define PSM_API_SHUTDOWN_PSM                   (9U) /** Shutdown PSM*/
#define MICROSECOND_TO_TICKS(x)		((x) * (((u32)XPAR_CPU_CORE_CLOCK_FREQ_HZ)/1000000U))
#define NANOSECOND_TO_TICKS(x)		((MICROSECOND_TO_TICKS(x))/1000U)

#define OCM_PWR_STATE_ACK_TIMEOUT	MICROSECOND_TO_TICKS(5U)
#define OCM_PWR_UP_WAIT_TIME		NANOSECOND_TO_TICKS(50U)

#define TCM_PWR_STATE_ACK_TIMEOUT	MICROSECOND_TO_TICKS(5U)
#define TCM_PWR_UP_WAIT_TIME		NANOSECOND_TO_TICKS(50U)

#define GEM0_PWR_STATE_ACK_TIMEOUT	MICROSECOND_TO_TICKS(5U)
#define GEM1_PWR_STATE_ACK_TIMEOUT	NANOSECOND_TO_TICKS(50U)
#define GEM0_PWR_UP_WAIT_TIME		MICROSECOND_TO_TICKS(5U)
#define GEM1_PWR_UP_WAIT_TIME		NANOSECOND_TO_TICKS(50U)

#define PWRUP_ACPU_CHN0_TO		MICROSECOND_TO_TICKS(5U)
#define PWRUP_ACPU_CHN1_TO		MICROSECOND_TO_TICKS(5U)
#define PWRUP_ACPU_CHN2_TO		MICROSECOND_TO_TICKS(5U)
#define PWRUP_ACPU_CHN3_TO		MICROSECOND_TO_TICKS(5U)
#define PWRUP_ACPU_CHN0_WAIT_TM		NANOSECOND_TO_TICKS(100U)
#define PWRUP_ACPU_CHN1_WAIT_TM		NANOSECOND_TO_TICKS(25U)
#define PWRUP_ACPU_CHN2_WAIT_TM		NANOSECOND_TO_TICKS(25U)
#define PWRUP_ACPU_CHN3_WAIT_TM		NANOSECOND_TO_TICKS(0U)
#define PWRDWN_ACPU_TO			MICROSECOND_TO_TICKS(5U)

#define ACPU_CTRL_CLK_PROP_TIME		((u32)2000)
#define RPU_CTRL_CLK_PROP_TIME		((u32)2000)
#define A78_CLUSTER_CONFIGURED		(0x1U)

#define ACPU_PACCEPT_TIMEOUT		(1000U)
#define RPU_PACTIVE_TIMEOUT		(10000000U)
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

typedef struct {
	u8 CmdLen; /** Total no of commands to configure this regulator */
	u8 CmdArr[MAX_I2C_COMMAND_LEN]; /** Array of i2c command bytes. For example, Len1,bytes, Len2, bytes, Len3,bytes etc */
} XPm_I2cCmd;

/**
 * The power node class.  This is the base class for all the power island and
 * power domain classes.
 */
typedef struct XPm_Power XPm_Power;
struct XPm_Power {
	XPm_Node Node; /**< Node: Node base class */
	XPm_Power *Parent; /**< Parent: Parent node in the power topology */
	u8 UseCount; /**< No. of devices currently using this power node */
	u8 WfParentUseCnt; /**< Pending use count of the parent */
	u16 PwrDnLatency; /**< Latency (in us) for transition to OFF state */
	u16 PwrUpLatency; /**< Latency (in us) for transition to ON state */
	XStatus (* HandleEvent)(XPm_Node *Node, u32 Event); /**< HandleEvent: Pointer to event handler */

	/* TODO: Remove below PSM variables */
	u32 PwrUpEnOffset; /**< PSM request power up interrupt enable register offset */
	u32 PwrDwnEnOffset; /**< PSM request power down interrupt enable register offset */
	u32 PwrUpMask; /**< PSM request power up interrupt mask */
	u32 PwrDwnMask; /**< PSM request power down interrupt mask */
	u32 PwrStatOffset; /**< PSM power state register offset */
	u32 PwrStatMask; /**< PSM power state mask */
};

/************************** Function Prototypes ******************************/
XPm_Power *XPmPower_GetById(u32 Id);
XStatus XPmPower_Init(XPm_Power *Power,
	u32 Id, u32 BaseAddress, XPm_Power *Parent);
XStatus XPmPower_AddParent(u32 Id, const u32 *Parents, u32 NumParents);
XStatus XPmPower_GetWakeupLatency(const u32 DeviceId, u32 *Latency);
XStatus XPmPower_GetState(const u32 DeviceId, u32 *const DeviceState);
XStatus XPm_DirectPwrDwn(const u32 DeviceId);

XPm_Power *XPmPower_GetByIndex(const u32 PwrIndex);
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XPM_POWER_H_ */
