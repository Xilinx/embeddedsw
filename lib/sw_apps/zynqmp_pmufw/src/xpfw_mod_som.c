/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpfw_mod_som.h"
#include "xpfw_config.h"

#ifdef ENABLE_MOD_SOM

#include "xpfw_default.h"
#include "xpfw_core.h"
#include "xpfw_module.h"
#include "xpfw_error_manager.h"



#ifdef SOM_EWDT_INTERVAL_MS

#ifndef ENABLE_SCHEDULER
#error "ERROR: SOM WDT feature requires scheduler to be enabled! Define ENABLE_SCHEDULER"
#endif

/* Toggle PMU GPO1[3] */
static void SomExtWdtToggle(void)
{
	u32 MioVal;
	/* Read o/p value from GPO1_READ register */
	MioVal = XPfw_Read32(PMU_LOCAL_GPO1_READ);
	/* Toggle GPO1[3] bit and write back the new o/p value */
	XPfw_Write32(PMU_IOMODULE_GPO1, (MioVal^PMU_IOMODULE_GPO1_MIO_3_MASK));
}


#endif /* SOM_EWDT_INTERVAL_MS */


/* SOM_OT_DEGC is decimal value of desired Over Temp value
 * in Celsius
 */
#ifdef SOM_OT_DEGC

#ifndef ENABLE_EM
#error "ERROR: SOM OT monitor requires EM to be enabled! Define ENABLE_EM"
#endif

#define AMS_PSSYSMON_CONFIG_REG1	0xFFA50904U
#define AMS_PSSYSMON_ALARM_OT_UPPER	0xFFA5094CU
#define AMS_PSSYSMON_ALARM_OT_LOWER	0xFFA5095CU

#define AMS_CONFIG_REG1_OT_DIS_MASK 0x1U


/* Calculate the OT Register Value as per EQN 2-10 in UG580*/
#define OT_REG_VAL (((u32)(( (float)SOM_OT_DEGC + 279.42657680f)* 4096.0f / 507.5921310f) <<4) | 0x3U)

static void InitOverTempAlarm(void)
{
	/* Clear OT alaram disable bit */
	XPfw_RMW32(AMS_PSSYSMON_CONFIG_REG1, AMS_CONFIG_REG1_OT_DIS_MASK, 0x0U);
	/* Disable hysteresis mode */
	XPfw_Write32(AMS_PSSYSMON_ALARM_OT_LOWER, 0x1U);
	/* Set OT alarm upper threshold */
	XPfw_Write32(AMS_PSSYSMON_ALARM_OT_UPPER, OT_REG_VAL);
}

/* Power Down SOM in response to LPD/FPD OT errors */
static void PowerDownSom(u8 ErrorId)
{
	XPfw_Printf(DEBUG_DETAILED,"SOM: Powering Down Board due to ErrorID: %d\r\n", ErrorId);
	/* TODO: Reuse PmKillBoardPower function in pm_core.c for this functionality */
#if defined(BOARD_SHUTDOWN_PIN) && defined(BOARD_SHUTDOWN_PIN_STATE)
	u32 reg = XPfw_Read32(PMU_LOCAL_GPO1_READ);
	u32 mask = PMU_IOMODULE_GPO1_MIO_0_MASK << BOARD_SHUTDOWN_PIN;
	u32 value = BOARD_SHUTDOWN_PIN_STATE << BOARD_SHUTDOWN_PIN;
	u32 mioPinOffset;

	mioPinOffset = IOU_SLCR_MIO_PIN_34_OFFSET + (BOARD_SHUTDOWN_PIN - 2U)*4U;

	reg = (reg & (~mask)) | (mask & value);
	XPfw_Write32(PMU_IOMODULE_GPO1, reg);
	/* Configure board shutdown pin to be controlled by the PMU */
	XPfw_RMW32((IOU_SLCR_BASE + mioPinOffset),
			0x000000FEU, 0x00000008U);
#endif
}

#endif /* SOM_OT_DEGC */

static void SomCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	s32 Status;
#ifdef SOM_EWDT_INTERVAL_MS
	/* Register scheduler task for External WDT service*/
	Status = XPfw_CoreScheduleTask(ModPtr,
				SOM_EWDT_INTERVAL_MS/2U,
				SomExtWdtToggle);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"SOM: Failed to Init EXT WDT Task\r\n");
	}
#endif /* SOM_EWDT_INTERVAL_MS */

#ifdef SOM_OT_DEGC
	/* Register Over Temeperature handlers */
	Status = XPfw_EmSetAction(EM_ERR_ID_FPD_TEMP,
				EM_ACTION_CUSTOM,
				PowerDownSom);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"SOM: Failed to register FPD OT Handler\r\n");
	}
	Status = XPfw_EmSetAction(EM_ERR_ID_LPD_TEMP,
				EM_ACTION_CUSTOM,
				PowerDownSom);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"SOM: Failed to register LPD OT Handler\r\n");
	}
	/* Initialize SYSMON OT Alarm */
	InitOverTempAlarm();
#endif /* SOM_OT_DEGC */
}

void ModSomInit(void)
{
	const XPfw_Module_t *SomModPtr;
	SomModPtr = XPfw_CoreCreateMod();

	if (XST_SUCCESS != XPfw_CoreSetCfgHandler(SomModPtr, SomCfgInit)){
		XPfw_Printf(DEBUG_DETAILED,"SOM: Set Cfg handler failed\r\n");
	}
}

#else /* ENABLE_MOD_SOM */
void ModSomInit(void) { }
#endif /* ENABLE_MOD_SOM */
