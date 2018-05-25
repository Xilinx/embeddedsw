/*
 * Copyright (C) 2014 - 2015 Xilinx, Inc.  All rights reserved.
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
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 */
#include "xpfw_config.h"
#ifdef ENABLE_PM

/*********************************************************************
 * Implementations of the functions to be used for integrating power
 * management (PM) within PMU firmware.
 *********************************************************************/

#include "pm_binding.h"
#include "pm_defs.h"
#include "pm_common.h"
#include "pm_proc.h"
#include "pm_core.h"
#include "pm_notifier.h"
#include "pm_power.h"
#include "pm_gic_proxy.h"
#include "pm_requirement.h"
#include "pm_extern.h"
#include "pm_usb.h"
#include "pm_hooks.h"

/* All GIC wakes in GPI1 */
#define PMU_IOMODULE_GPI1_GIC_WAKES_ALL_MASK \
		(PMU_IOMODULE_GPI1_ACPU_0_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_1_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_2_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_3_WAKE_MASK | \
		PMU_IOMODULE_GPI1_R5_0_WAKE_MASK | \
		PMU_IOMODULE_GPI1_R5_1_WAKE_MASK)

#define PMU_IOMODULE_GPI1_MIO_WAKE_ALL_MASK \
		(PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK | \
		PMU_IOMODULE_GPI1_MIO_WAKE_1_MASK | \
		PMU_IOMODULE_GPI1_MIO_WAKE_2_MASK | \
		PMU_IOMODULE_GPI1_MIO_WAKE_3_MASK | \
		PMU_IOMODULE_GPI1_MIO_WAKE_4_MASK | \
		PMU_IOMODULE_GPI1_MIO_WAKE_5_MASK)

/**
 * XPfw_PmInit() - initializes PM firmware
 *
 * @note	Call on startup to initialize PM firmware.
 */
void XPfw_PmInit(void)
{
#ifdef ENABLE_POS
	u32 bootType = PmHookGetBootType();

	/* Call user hook for Power Off Suspend initialization */
	PmHookInitPowerOffSuspend();
#else
	u32 bootType = PM_COLD_BOOT;
#endif

	PmInfo("Power Management Init\r\n");

	if (bootType == PM_COLD_BOOT) {
		PmMasterDefaultConfig();
		PmNodeConstruct();
	}
}

/**
 * XPfw_PmIpiHandler() - Call from IPI interrupt handler to process PM API call
 * @IsrMask IPI's ISR register value. Needed to determine the source master
 *
 * @Payload  Pointer to IPI Payload
 *
 * @Len Size of the payload in words
 *
 * @return  Status of the processing IPI
 *          - XST_INVALID_PARAM if input parameters have invalid value
 *          - XST_SUCCESS otherwise
 *          - Note that if request is processed, firmware is not receiving any
 *            status of processing information. Processing status is returned to
 *            the master which initiated communication through IPI.
 *
 */
int XPfw_PmIpiHandler(const u32 IsrMask, const u32* Payload, u8 Len)
{
	int status = XST_SUCCESS;
	PmMaster* master = PmGetMasterByIpiMask(IsrMask);

	if ((NULL == Payload) || (NULL == master) || (Len < PAYLOAD_ELEM_CNT)) {
		/* Never happens if IPI irq handler is implemented correctly */
		PmErr("Unknown IPI %lu\r\n", IsrMask);
		status = XST_INVALID_PARAM;
		goto done;
	}

	PmProcessRequest(master, Payload);

done:
	return status;
}

/**
 * XPfw_PmWfiHandler() - Call from GPI2 interrupt handler to process sleep req
 * @srcMask Value read from GPI2 register which determines master requestor
 *
 * @return  Status of triggering sleep for a processor (XST_INVALID_PARAM if
 *          processor cannot be determined by srcMask, status of performing
 *          sleep operation otherwise)
 *
 * @note    Call from GPI2 interrupt routine to process sleep request. Must not
 *          clear GPI2 interrupt before this function returns.
 */
int XPfw_PmWfiHandler(const u32 srcMask)
{
	int status;
	PmProc *proc = PmGetProcByWfiStatus(srcMask);

	if (NULL == proc) {
		PmErr("Unknown processor 0x%lx\r\n", srcMask);
		status = XST_INVALID_PARAM;
		goto done;
	}

	status = PmProcFsm(proc, PM_PROC_EVENT_SLEEP);

done:
	return status;
}

/**
 * XPfw_PmWakeHandler() - Call from GPI1 interrupt to process wake request
 * @srcMask     Value read from GPI1 register which determines interrupt source
 *
 * @return      Status of performing wake-up (XST_INVALID_PARAM if wake is a
 *              processor wake event but processor is not found, status of
 *              performing wake otherwise)
 *
 * @note    Call from GPI1 interrupt routine to process wake request. Must not
 *          clear GPI1 interrupt before this function returns.
 *          If the wake source is one of GIC wakes, source of the interrupt
 *          (peripheral that actually generated interrupt to GIC) cannot be
 *          determined, and target should be immediately woken-up (target is
 *          processor whose GIC wake bit is set in srcMask). If the wake is the
 *          FPD GIC Proxy interrupt, the APU needs to be woken up.
 */
int XPfw_PmWakeHandler(const u32 srcMask)
{
	int status = XST_INVALID_PARAM;

#if defined(PMU_MIO_INPUT_PIN) && (PMU_MIO_INPUT_PIN >= 0) \
				&& (PMU_MIO_INPUT_PIN <= 5)
	if ((PMU_IOMODULE_GPI1_MIO_WAKE_0_MASK << PMU_MIO_INPUT_PIN) == srcMask) {
		PmShutdownInterruptHandler();
		return XST_SUCCESS;
	}
#endif
	if (0U != (PMU_IOMODULE_GPI1_GIC_WAKES_ALL_MASK & srcMask))  {
		/* Processor GIC wake */
		PmProc* proc = PmProcGetByWakeMask(srcMask);
		if ((NULL != proc) && (NULL != proc->master)) {
			status = PmMasterWakeProc(proc);
		} else {
			status = XST_INVALID_PARAM;
		}
	} else if (0U != (PMU_IOMODULE_GPI1_FPD_WAKE_GIC_PROXY_MASK & srcMask)) {
		status = PmMasterWake(&pmMasterApu_g);
	} else if (0U != (PMU_IOMODULE_GPI1_MIO_WAKE_ALL_MASK & srcMask)) {
		status = PmExternWakeMasters();
	} else if (0U != (PMU_IOMODULE_GPI1_USB_0_WAKE_MASK & srcMask)) {
		status = PmWakeMasterBySlave(&pmSlaveUsb0_g.slv);
	} else if (0U != (PMU_IOMODULE_GPI1_USB_1_WAKE_MASK & srcMask)) {
		status = PmWakeMasterBySlave(&pmSlaveUsb1_g.slv);
	} else {
	}

	return status;
}

/**
 * XPfw_PmCheckIpiRequest() - Check whether the IPI interrupt is a PM call
 * @isrVal  IPI's ISR register value
 * @apiId   Pointer to a variable holding the api id (first word of message)
 *
 * @return  Check result
 *
 * @note    Call from IPI interrupt routine to check is interrupt a PM call.
 *          Function reads first argument of payload in IPI buffer of
 *          requestor master to determine whether first argument is within
 *          PM API regular ids.
 */
XPfw_PmIpiStatus XPfw_PmCheckIpiRequest(const u32 isrVal,
					const u32* apiId)
{
	XPfw_PmIpiStatus status;
	const PmMaster *master = PmGetMasterByIpiMask(isrVal);

	if (NULL == master) {
		/* IPI is not generated by one of the PM supported PUs */
		status = XPFW_PM_IPI_SRC_UNKNOWN;
		goto done;
	}

	/* Api id is first argument in payload */
	if ((*apiId >= PM_API_MIN) && (*apiId <= PM_API_MAX)) {
		/* Api id is within valid range */
		status = XPFW_PM_IPI_IS_PM_CALL;
	} else {
		/* This IPI was not a PM call */
		status = XPFW_PM_IPI_NOT_PM_CALL;
	}

done:
	return status;
}

/**
 * XPfw_DapFpdWakeEvent() - Inform PM about the FPD DAP wake event
 */
void XPfw_DapFpdWakeEvent(void)
{
	pmPowerDomainFpd_g.power.node.currState = PM_PWR_STATE_ON;
}

/**
 * XPfw_DapRpuWakeEvent() - Inform PM about the RPU DAP wake event
 */
void XPfw_DapRpuWakeEvent(void)
{
	pmPowerIslandRpu_g.power.node.currState = PM_PWR_STATE_ON;
}

#endif
