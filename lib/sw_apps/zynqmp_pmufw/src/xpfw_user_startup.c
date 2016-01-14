/******************************************************************************
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
******************************************************************************/

#include "xpfw_config.h"

#include "xpfw_core.h"
#include "xpfw_events.h"
#include "xpfw_module.h"

#include "xpfw_user_startup.h"

#include "pm_binding.h"
#include "pm_api.h"
#include "ipi_buffer.h"
#include "pm_defs.h"

#include "xpfw_mod_dap.h"
#include "xpfw_mod_legacy.h"
#include "xpfw_mod_em.h"

#ifdef ENABLE_PM
static void PmIpiHandler(const XPfw_Module_t *ModPtr, u32 IpiNum, u32 SrcMask)
{
	u32 isrVal, isrClr, apiId;
	XPfw_PmIpiStatus ipiStatus;
	XStatus status;

	switch (IpiNum) {
	case 0:
		isrVal = XPfw_Read32(IPI_PMU_0_ISR);
		fw_printf("Received IPI Mask:0x%08lx\r\n", isrVal);
		ipiStatus = XPfw_PmCheckIpiRequest(isrVal, &apiId);
		if (XPFW_PM_IPI_IS_PM_CALL == ipiStatus) {
			/* Power management API processing */
			status = XPfw_PmIpiHandler(isrVal, apiId, &isrClr);
			if (XST_SUCCESS == status) {
				/* Clear only irq for handled PM request */
				XPfw_Write32(IPI_PMU_0_ISR, isrClr);
			}
		} else {
			status = XST_NO_FEATURE;
			fw_printf("MOD-%d: Non-PM IPI-%lu call received\r\n", ModPtr->ModId, IpiNum);
		}

		if (XST_SUCCESS != status) {
			/*
			 * Clear all irqs if something went wrong, to avoid
			 * system looping in interrupt handler because of error
			 */
			XPfw_Write32(IPI_PMU_0_ISR, isrVal);
			fw_printf("ERROR #%ld : IPI-%lu\r\n", status, IpiNum);
		}
		break;

	case 1:
		isrVal = XPfw_Read32(IPI_PMU_1_ISR);
		XPfw_Write32(IPI_PMU_1_ISR, isrVal);
		break;

	case 2:
		isrVal = XPfw_Read32(IPI_PMU_2_ISR);
		XPfw_Write32(IPI_PMU_2_ISR, isrVal);
		break;

	case 3:
		isrVal = XPfw_Read32(IPI_PMU_3_ISR);
		XPfw_Write32(IPI_PMU_3_ISR, isrVal);
		break;

	default:
		fw_printf("ERROR: Invalid IPI Number: %lu\r\n", IpiNum);
	}
}

static void PmEventHandler(const XPfw_Module_t *ModPtr, u32 EventId)
{
	u32 EvType, RegValue;
	EvType = XPfw_EventGetType(EventId);

	switch (EvType) {
	case XPFW_EV_TYPE_GPI1:
		RegValue = XPfw_EventGetRegMask(EventId);
		XPfw_PmWakeHandler(RegValue);
		break;
	case XPFW_EV_TYPE_GPI2:
		RegValue = XPfw_EventGetRegMask(EventId);
		XPfw_PmWfiHandler(RegValue);
		break;
	default:
		fw_printf("Unhandled PM Event: %lu\r\n", EventId);
		break;
	}
}

static void PmCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	/* Add Event Handlers for PM */
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_0_WAKE);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_1_WAKE);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_2_WAKE);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_3_WAKE);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_R5_0_WAKE);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_R5_1_WAKE);

	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_0);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_1);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_2);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_3);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_4);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_5);

	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_FPD_WAKE_GIC_PROXY);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_USB_0_WAKE);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_USB_1_WAKE);

	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_0_SLEEP);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_1_SLEEP);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_2_SLEEP);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_3_SLEEP);

	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_R5_0_SLEEP);
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_R5_1_SLEEP);

/*
 * FIXME: PM Init Disables the wakes/sleep interrupts
 * and enables them when required. For NOW, it works, but in future
 * all modules should use Register/DeRegister Event
 */
	XPfw_PmInit();
}
static void ModPmInit(void)
{
	const XPfw_Module_t *PmModPtr = XPfw_CoreCreateMod();

	(void)XPfw_CoreSetCfgHandler(PmModPtr, PmCfgInit);
	(void)XPfw_CoreSetEventHandler(PmModPtr, PmEventHandler);
	(void)XPfw_CoreSetIpiHandler(PmModPtr, PmIpiHandler, 0U);
}
#else /* ENABLE_PM */
static void ModPmInit(void) { }
#endif /* ENABLE_PM */

#ifdef ENABLE_RTC_TEST
static void RtcEventHandler(const XPfw_Module_t *ModPtr, u32 EventId)
{
	fw_printf("MOD%d:EVENTID: %d\r\n", ModPtr->ModId, EventId);
	//XPfw_CorePrintStats();
	if (XPFW_EV_RTC_SECONDS == EventId) {
			/* Ack the Int in RTC Module */
			Xil_Out32(RTC_RTC_INT_STATUS, 1U);
			fw_printf("RTC: %d \r\n", Xil_In32(RTC_CURRENT_TIME));
	}
}

static void RtcCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_RTC_SECONDS);
	/* Enable Seconds Alarm */
	Xil_Out32(RTC_RTC_INT_EN, 1U);
	Xil_Out32(RTC_RTC_INT_STATUS, 1U);
	fw_printf("RTC (MOD-%d): Initialized.\r\n", ModPtr->ModId);
}
static void ModRtcInit(void)
{
	const XPfw_Module_t *RtcModPtr = XPfw_CoreCreateMod();

	(void)XPfw_CoreSetCfgHandler(RtcModPtr, RtcCfgInit);
	(void)XPfw_CoreSetEventHandler(RtcModPtr, RtcEventHandler);
}
#else /* ENABLE_RTC_TEST */
static void ModRtcInit(void) { }
#endif /* ENABLE_RTC_TEST */

#ifdef ENABLE_SCHEDULER
static void PrintMsg1(void)
{
	fw_printf("Task#1\r\n");
}
static void PrintMsg2(void)
{
	fw_printf("Task#2\r\n");
}

static void SchCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	/* Task every 5 seconds - For our convenience in manual testing */
	fw_printf("Adding Task1 - Status: %ld\n", XPfw_CoreScheduleTask(ModPtr, 5000U, PrintMsg1));
	/* Every 10 seconds */
	fw_printf("Adding Task2 - Status:%ld\n", XPfw_CoreScheduleTask(ModPtr, 10000U, PrintMsg2));
}

static XStatus ModSchInit(void)
{
	const XPfw_Module_t *SchModPtr = XPfw_CoreCreateMod();

	return XPfw_CoreSetCfgHandler(SchModPtr, SchCfgInit);
}
#else /* ENABLE_SCHEDULER */
static void ModSchInit(void) { }
#endif /* ENABLE_SCHEDULER */

void XPfw_UserStartUp(void)
{
	ModRtcInit();
	ModEmInit();
	ModPmInit();
	(void)ModSchInit();
	ModDapInit();
	ModLegacyInit();
}
