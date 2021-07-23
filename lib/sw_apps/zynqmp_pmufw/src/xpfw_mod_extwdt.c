/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpfw_mod_extwdt.h"
#include "xpfw_config.h"

#ifdef ENABLE_MOD_EXTWDT

#include "xpfw_default.h"
#include "xpfw_core.h"
#include "xpfw_module.h"


#ifndef EXTWDT_INTERVAL_MS
#define EXTWDT_INTERVAL_MS 1140U
#endif

#ifndef EXTWDT_MIO_PIN
#define EXTWDT_MIO_PIN 3U
#endif

#define EXTWDT_MIO_MASK ((u32)1U<<EXTWDT_MIO_PIN)


#ifndef ENABLE_SCHEDULER
#error "ERROR: External WDT feature requires scheduler to be enabled! Define ENABLE_SCHEDULER"
#endif

#ifdef ENABLE_RUNTIME_EXTWDT
static const XPfw_Module_t *ModPtr;
static u32 ExtWdtInterval = EXTWDT_INTERVAL_MS / 2U;
#endif

/* Toggle PMU GPO1 bit specified for EXTWDT */
static void ExtWdtToggle(void)
{
	u32 MioVal;
	/* Read o/p value from GPO1_READ register */
	MioVal = XPfw_Read32(PMU_LOCAL_GPO1_READ);
	/* Toggle GPO1 bit and write back the new o/p value */
	XPfw_Write32(PMU_IOMODULE_GPO1, (MioVal^(EXTWDT_MIO_MASK)));
}



#ifdef ENABLE_RUNTIME_EXTWDT
/****************************************************************************/
/**
 * @brief  Set external watchdog timer interval.
 *
 * @param  Interval The EXT WDT timer interval value in ms
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
void SetExtWdtInterval(u32 Interval)
{
	ExtWdtInterval = Interval;
}

/****************************************************************************/
/**
 * @brief  Get external watchdog timer interval.
 *
 * @param  None.
 *
 * @return EXT WDT interval value in ms
 *
 * @note   None.
 *
 ****************************************************************************/
u32 GetExtWdtInterval(void)
{
	return ExtWdtInterval;
}

/****************************************************************************/
/**
 * @brief  This function removes scheduled task for the EXTWDT.
 *
 * @param  None.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error
 * 	   code or a reason code
 *
 * @note   None.
 *
 ****************************************************************************/
s32 ExtWdtCfgDeInit(void)
{
	s32 Status = XST_FAILURE;
	u32 Interval = GetExtWdtInterval();

	/* De-Register scheduled task for External WDT service*/
	Status = XPfw_CoreRemoveTask(ModPtr,
				     Interval,
				     ExtWdtToggle);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"EXTWDT: Failed to De-Init EXT WDT Task\r\n");
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function schedule a task for the EXTWDT.
 *
 * @param  None.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error
 * 	   code or a reason code
 *
 * @note   None.
 *
 ****************************************************************************/
s32 ExtWdtCfgInit(void)
{
	s32 Status = XST_FAILURE;
	u32 Interval = GetExtWdtInterval();

	/* Register scheduler task for External WDT service*/
	Status = XPfw_CoreScheduleTask(ModPtr,
				       Interval,
				       ExtWdtToggle);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"EXTWDT: Failed to Init EXT WDT Task\r\n");
	}

	return Status;
}

/*
 * Create a Mod and assign the Handlers. We will call this function
 * from XPfw_UserStartup()
 */
void ModExtWdtInit(void)
{
	ModPtr = XPfw_CoreCreateMod();
}
#else
static void ExtWdtCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	s32 Status;

	/* Register scheduler task for External WDT service*/
	Status = XPfw_CoreScheduleTask(ModPtr,
				EXTWDT_INTERVAL_MS/2U,
				ExtWdtToggle);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED,"EXTWDT: Failed to Init EXT WDT Task\r\n");
	}

}



void ModExtWdtInit(void)
{
	const XPfw_Module_t *ExtWdtModPtr;
	ExtWdtModPtr = XPfw_CoreCreateMod();

	if (XST_SUCCESS != XPfw_CoreSetCfgHandler(ExtWdtModPtr, ExtWdtCfgInit)){
		XPfw_Printf(DEBUG_DETAILED,"EXTWDT: Set Cfg handler failed\r\n");
	}
}
#endif /* ENABLE_RUNTIME_EXTWDT */

#else /* ENABLE_MOD_EXTWDT */
void ModExtWdtInit(void) { }
#endif /* ENABLE_MOD_EXTWDT */
