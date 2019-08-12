/******************************************************************************
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
******************************************************************************/
#include "xpfw_config.h"

#include "xpfw_core.h"
#include "xpfw_events.h"
#include "xpfw_module.h"

#include "pm_binding.h"
#include "pm_defs.h"

#include "xpfw_ipi_manager.h"
#include "xpfw_mod_pm.h"


#ifdef ENABLE_PM

#define PM_MAX_MSG_LEN XPFW_IPI_MAX_MSG_LEN
const XPfw_Module_t *PmModPtr;

static void PmIpiHandler(const XPfw_Module_t *ModPtr, u32 IpiNum, u32 SrcMask, const u32* Payload, u8 Len)
{
	XPfw_PmIpiStatus ipiStatus;
	u32 isrVal;

	switch (IpiNum) {
	case 0U:
		ipiStatus = XPfw_PmCheckIpiRequest(SrcMask, &Payload[0]);

		if (XPFW_PM_IPI_IS_PM_CALL == ipiStatus) {
			/* Power management API processing */
			(void)XPfw_PmIpiHandler(SrcMask, &Payload[0], Len);
		} else {
			XPfw_Printf(DEBUG_DETAILED,"MOD-%d: Non-PM IPI-%lu call received"
					"\r\n", ModPtr->ModId, IpiNum);
		}
		break;

	case 1U:
		isrVal = XPfw_Read32(IPI_PMU_1_ISR);
		XPfw_Write32(IPI_PMU_1_ISR, isrVal);
		break;

	case 2U:
		isrVal = XPfw_Read32(IPI_PMU_2_ISR);
		XPfw_Write32(IPI_PMU_2_ISR, isrVal);
		break;

	case 3U:
		isrVal = XPfw_Read32(IPI_PMU_3_ISR);
		XPfw_Write32(IPI_PMU_3_ISR, isrVal);
		break;

	default:
		XPfw_Printf(DEBUG_ERROR,"ERROR: Invalid IPI Number: %lu\r\n", IpiNum);
		break;
	}
}

static void PmEventHandler(const XPfw_Module_t *ModPtr, u32 EventId)
{
	u32 EvType, RegValue;
	EvType = XPfw_EventGetType(EventId);

	switch (EvType) {
	case XPFW_EV_TYPE_GPI1:
		RegValue = XPfw_EventGetRegMask(EventId);
		if (XPfw_PmWakeHandler(RegValue) != XST_SUCCESS) {
			XPfw_Printf(DEBUG_DETAILED, "ERROR: Unknown processor %lu\r\n",
					RegValue);
		}
		break;
	case XPFW_EV_TYPE_GPI2:
		RegValue = XPfw_EventGetRegMask(EventId);
		(void)XPfw_PmWfiHandler(RegValue);
		break;
	default:
		XPfw_Printf(DEBUG_ERROR,"Unhandled PM Event: %lu\r\n", EventId);
		break;
	}
}

static void PmCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData, u32 Len)
{
	/* Add Event Handlers for PM */
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_0_WAKE) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_ACPU_0_WAKE)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_1_WAKE) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_ACPU_1_WAKE)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_2_WAKE) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_ACPU_2_WAKE)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_3_WAKE) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_ACPU_3_WAKE)
	}

	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_R5_0_WAKE) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_R5_0_WAKE)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_R5_1_WAKE) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_R5_1_WAKE)
	}

	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_0) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_MIO_WAKE_0)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_1) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_MIO_WAKE_1)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_2) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_MIO_WAKE_2)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_3) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_MIO_WAKE_3)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_4) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_MIO_WAKE_4)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_MIO_WAKE_5) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_MIO_WAKE_5)
	}

	if (XPfw_CoreRegisterEvent(ModPtr,
	XPFW_EV_FPD_WAKE_GIC_PROXY) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_FPD_WAKE_GIC_PROXY)
	}

	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_USB_0_WAKE) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_USB_0_WAKE)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_USB_1_WAKE) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_USB_1_WAKE)
	}

	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_0_SLEEP) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_ACPU_0_SLEEP)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_1_SLEEP) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_ACPU_1_SLEEP)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_2_SLEEP) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_ACPU_2_SLEEP)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ACPU_3_SLEEP) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_ACPU_3_SLEEP)
	}

	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_R5_0_SLEEP) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_R5_0_SLEEP)
	}
	if (XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_R5_1_SLEEP) != XST_SUCCESS) {
		XPfw_Printf(DEBUG_DETAILED,
				"Warning: PmCfgInit: Failed to register event ID:"
						" %d\r\n", XPFW_EV_R5_1_SLEEP)
	}

	XPfw_PmInit();

}
void ModPmInit(void)
{
	PmModPtr = XPfw_CoreCreateMod();

	(void)XPfw_CoreSetCfgHandler(PmModPtr, PmCfgInit);
	(void)XPfw_CoreSetEventHandler(PmModPtr, PmEventHandler);
	(void)XPfw_CoreSetIpiHandler(PmModPtr, PmIpiHandler, 0U);
}
#else /* ENABLE_PM */
void ModPmInit(void) { }
#endif /* ENABLE_PM */
