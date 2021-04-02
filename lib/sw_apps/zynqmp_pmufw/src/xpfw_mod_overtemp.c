/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpfw_mod_overtemp.h"
#include "xpfw_config.h"

#ifdef ENABLE_MOD_OVERTEMP

#include "xpfw_default.h"
#include "xpfw_core.h"
#include "xpfw_module.h"
#include "xpfw_error_manager.h"
#include "pm_core.h"



/*
 * OVERTEMP_DEGC is decimal value of desired Over Temp limit value
 * in Celsius
 */
#ifndef OVERTEMP_DEGC
#define OVERTEMP_DEGC 90.0f
#endif

#ifndef ENABLE_EM
#error "ERROR: Over Temperature monitor requires EM to be enabled! Define ENABLE_EM"
#endif

#define AMS_PSSYSMON_CONFIG_REG1	0xFFA50904U
#define AMS_PSSYSMON_ALARM_OT_UPPER	0xFFA5094CU
#define AMS_PSSYSMON_ALARM_OT_LOWER	0xFFA5095CU

#define AMS_CONFIG_REG1_OT_DIS_MASK 0x1U


/*
 * Calculate the OT Register Value as per EQN 2-12 in UG580
 * LSB 4 bits should be equal to 0x3 to enable OT alarm
 */
#define OT_REG_VAL (((u32)(( (float)OVERTEMP_DEGC + 280.23087870f)* 4096.0f / 509.3140064f) <<4) | 0x3U)

static void InitOverTempAlarm(void)
{
	/* Clear OT alaram disable bit */
	XPfw_RMW32(AMS_PSSYSMON_CONFIG_REG1, AMS_CONFIG_REG1_OT_DIS_MASK, 0x0U);
	/* Disable hysteresis mode */
	XPfw_Write32(AMS_PSSYSMON_ALARM_OT_LOWER, 0x1U);
	/* Set OT alarm upper threshold */
	XPfw_Write32(AMS_PSSYSMON_ALARM_OT_UPPER, OT_REG_VAL);
}

/* Over Temperature Handler for LPD/FPD OT errors */
static void OverTempHandler(u8 ErrorId)
{
	XPfw_Printf(DEBUG_DETAILED,"Over-Temp: Powering Down Board due to ErrorID: %d\r\n", ErrorId);
	/* Turn Off board power */
	PmKillBoardPower();
}

static void OverTempCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	s32 Status;

	/* Register Over Temeperature handlers */
	Status = XPfw_EmSetAction(EM_ERR_ID_FPD_TEMP,
				EM_ACTION_CUSTOM,
				OverTempHandler);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Over-Temp: Failed to register FPD OT Handler\r\n");
	}
	Status = XPfw_EmSetAction(EM_ERR_ID_LPD_TEMP,
				EM_ACTION_CUSTOM,
				OverTempHandler);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Over-Temp: Failed to register LPD OT Handler\r\n");
	}
	/* Initialize SYSMON OT Alarm */
	InitOverTempAlarm();
}

void ModOverTempInit(void)
{
	const XPfw_Module_t *OverTempModPtr;
	OverTempModPtr = XPfw_CoreCreateMod();

	if (XST_SUCCESS != XPfw_CoreSetCfgHandler(OverTempModPtr, OverTempCfgInit)){
		XPfw_Printf(DEBUG_DETAILED,"Over-Temp: Set Cfg handler failed\r\n");
	}
}

#else /* ENABLE_MOD_OVERTEMP */
void ModOverTempInit(void) { }
#endif /* ENABLE_MOD_OVERTEMP */
