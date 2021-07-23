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
#ifdef ENABLE_RUNTIME_OVERTEMP
#define OT_REG_VAL(value) (((u32)(( (float)(value) + 280.23087870f)* 4096.0f / 509.3140064f) <<4) | 0x3U)
static u32 OverTempLimit = (u32)OVERTEMP_DEGC;
#else
#define OT_REG_VAL (((u32)(( (float)OVERTEMP_DEGC + 280.23087870f)* 4096.0f / 509.3140064f) <<4) | 0x3U)
#endif /* ENABLE_RUNTIME_OVERTEMP */

#ifndef ENABLE_RUNTIME_OVERTEMP
static void InitOverTempAlarm(void)
{
	/* Clear OT alaram disable bit */
	XPfw_RMW32(AMS_PSSYSMON_CONFIG_REG1, AMS_CONFIG_REG1_OT_DIS_MASK, 0x0U);
	/* Disable hysteresis mode */
	XPfw_Write32(AMS_PSSYSMON_ALARM_OT_LOWER, 0x1U);
	/* Set OT alarm upper threshold */
	XPfw_Write32(AMS_PSSYSMON_ALARM_OT_UPPER, OT_REG_VAL);
}
#endif /* ENABLE_RUNTIME_OVERTEMP */

/* Over Temperature Handler for LPD/FPD OT errors */
static void OverTempHandler(u8 ErrorId)
{
	XPfw_Printf(DEBUG_DETAILED,"Over-Temp: Powering Down Board due to ErrorID: %d\r\n", ErrorId);
	/* Turn Off board power */
	PmKillBoardPower();
}

#ifdef ENABLE_RUNTIME_OVERTEMP
/****************************************************************************/
/**
 * @brief  Set OT register value at runtime using an IOCTL calls.
 *
 * @param  DegCel The OT value in degree Celsius
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
void SetOverTempLimit(u32 DegCel)
{
	OverTempLimit = DegCel;
}

/****************************************************************************/
/**
 * @brief  Get OT register value.
 *
 * @param  None.
 *
 * @return OT reg value
 *
 * @note   None.
 *
 ****************************************************************************/
u32 GetOverTempLimit(void)
{
	return OverTempLimit;
}

static void DeInitOverTempAlarm(void)
{
	/* Set OT alarm disable bit to disable alarm */
	XPfw_RMW32(AMS_PSSYSMON_CONFIG_REG1, AMS_CONFIG_REG1_OT_DIS_MASK, 0x1U);
}

static void InitOverTempAlarm(void)
{
	u32 OtLimit = GetOverTempLimit();

	/* Clear OT alarm disable bit */
	XPfw_RMW32(AMS_PSSYSMON_CONFIG_REG1, AMS_CONFIG_REG1_OT_DIS_MASK, 0x0U);
	/* Disable hysteresis mode */
	XPfw_Write32(AMS_PSSYSMON_ALARM_OT_LOWER, 0x1U);
	/* Set OT alarm upper threshold */
	XPfw_Write32(AMS_PSSYSMON_ALARM_OT_UPPER, OT_REG_VAL(OtLimit));
}

/****************************************************************************/
/**
 * @brief  This function registers over temperature handler.
 *
 * @param  None.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error
 * 	   code or a reason code
 *
 * @note   None.
 *
 ****************************************************************************/
s32 OverTempCfgInit(void)
{
	s32 Status = XST_FAILURE;

	/* Register Over Temperature handler for FPD event */
	Status = XPfw_EmSetAction(EM_ERR_ID_FPD_TEMP,
				  EM_ACTION_CUSTOM,
				  OverTempHandler);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Over-Temp: Failed to register FPD OT Handler\r\n");
		goto done;
	}

	/* Register Over Temperature handler for LPD event */
	Status = XPfw_EmSetAction(EM_ERR_ID_LPD_TEMP,
				  EM_ACTION_CUSTOM,
				  OverTempHandler);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Over-Temp: Failed to register LPD OT Handler\r\n");
		goto done;
	}

	/* Initialize SYSMON OT Alarm */
	InitOverTempAlarm();

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief  This function de-registers over temperature handler.
 *
 * @param  None.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error
 * 	   code or a reason code
 *
 * @note   None.
 *
 ****************************************************************************/
s32 OverTempCfgDeInit(void)
{
	s32 Status = XST_FAILURE;

	/* De-Initialize SYSMON OT Alarm */
	DeInitOverTempAlarm();

	/* De-Register Over Temperature handler for LPD event */
	Status = XPfw_EmDisable(EM_ERR_ID_LPD_TEMP);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Over-Temp: Failed to de-register LPD OT Handler\r\n");
		goto done;
	}

	/* De-Register Over Temperature handler for FPD event */
	Status = XPfw_EmDisable(EM_ERR_ID_FPD_TEMP);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"Over-Temp: Failed to de-register FPD OT Handler\r\n");
		goto done;
	}

done:
	return Status;
}
#else
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
#endif /* ENABLE_RUNTIME_OVERTEMP */

#else /* ENABLE_MOD_OVERTEMP */
void ModOverTempInit(void) { }
#endif /* ENABLE_MOD_OVERTEMP */
