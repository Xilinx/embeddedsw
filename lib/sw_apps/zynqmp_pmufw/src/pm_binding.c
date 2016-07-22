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
 */

/*********************************************************************
 * Implementations of the functions to be used for integrating power
 * management (PM) within PMU firmware.
 *********************************************************************/

#include "pm_binding.h"
#include "pm_api.h"
#include "pm_defs.h"
#include "pm_common.h"
#include "pm_proc.h"
#include "pm_core.h"
#include "pm_notifier.h"
#include "ipi_buffer.h"
#include "pm_power.h"
#include "pm_gic_proxy.h"

/*
 * Macro for all wake events in GPI1 that PM handles.
 * Used for initialization to avoid looping through all slaves array.
 */
#define PMU_IOMODULE_GPI1_WAKES_ALL_MASK \
		(PMU_IOMODULE_GPI1_GIC_WAKES_ALL_MASK | \
		PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK)

/* All WFI bitfields in GPI2 */
#define PMU_LOCAL_GPI2_ENABLE_ALL_PWRDN_REQ_MASK \
		(PMU_LOCAL_GPI2_ENABLE_ACPU0_PWRDWN_REQ_MASK | \
		PMU_LOCAL_GPI2_ENABLE_ACPU1_PWRDWN_REQ_MASK | \
		PMU_LOCAL_GPI2_ENABLE_ACPU2_PWRDWN_REQ_MASK | \
		PMU_LOCAL_GPI2_ENABLE_ACPU3_PWRDWN_REQ_MASK | \
		PMU_LOCAL_GPI2_ENABLE_R5_0_PWRDWN_REQ_MASK | \
		PMU_LOCAL_GPI2_ENABLE_R5_1_PWRDWN_REQ_MASK)

/* All GIC wakes in GPI1 */
#define PMU_IOMODULE_GPI1_GIC_WAKES_ALL_MASK \
		(PMU_IOMODULE_GPI1_ACPU_0_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_1_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_2_WAKE_MASK | \
		PMU_IOMODULE_GPI1_ACPU_3_WAKE_MASK | \
		PMU_IOMODULE_GPI1_R5_0_WAKE_MASK | \
		PMU_IOMODULE_GPI1_R5_1_WAKE_MASK)

/**
 * XPfw_PmInit() - initializes PM firmware
 *
 * @note    Call on startup after XPfw_PmSetConfiguration to initialize PM
 *          firmware. It is assumed that PMU firmware enables GPI1, GPI2, and
 *          IPI0 interrupts at the processor level, so PMU only masks/unmasks
 *          specific events.
 */
void XPfw_PmInit(void)
{
	PmDbg("Power Management Init\n");
	/* Disable all wake requests in GPI1 */
	DISABLE_WAKE(PMU_IOMODULE_GPI1_WAKES_ALL_MASK);
	/* Disable all wfi requests in GPI2 */
	DISABLE_WFI(PMU_LOCAL_GPI2_ENABLE_ALL_PWRDN_REQ_MASK);

	PmRequirementInit();
	/* Setup initial slaves for masters */
	PmSetupInitialMasterRequirements();
	/* Enable all IPI interrupts so masters' requests can be received */
	PmEnableAllMasterIpis();
}

/**
 * XPfw_PmIpiHandler() - Call from IPI interrupt handler to process PM API call
 * @isrMask IPI's ISR register value. Needed to determine buffer holding the
 *          payload
 * @isrClr  Pointer to a variable in which PM returns an ISR mask of the master
 *          whose request is handled (mask to write in ISR register to clear
 *          interrupt)
 * @apiId   PM API id that was read from master's IPI buffer and validated as
 *          existing
 *
 * @return  Status of the processing IPI
 *          - XST_INVALID_PARAM if isrMask parameter has invalid value
 *          - XST_SUCCESS otherwise
 *          - Note that if request is processed, firmware is not receiving any
 *            status of processing information. Processing status is returned to
 *            the master which initiated communication through IPI.
 *
 * @note    Call from IPI#0 interrupt routine. IPI's #0 interrupt can be used
 *          for some other purposes, not only for PM, and in #0 interrupt
 *          routine must :
 *          1. Read ISR register before calling this function
 *          2. Determine master requestor and accordingly read first 32b
 *             argument from master's IPI buffer. Determine is it PM API.
 *             If yes, this function is called.
 *          3. Write into ISR register after this function returns.
 *             Write to ISR clears interrupt in IPI peripheral. Interrupt must
 *             be cleared after this function returns to make PM API call
 *             atomic.
 */
int XPfw_PmIpiHandler(const u32 isrMask, const u32 apiId, u32* const isrClr)
{
	int status = XST_SUCCESS;
	u32 i;
	u32 payload[PAYLOAD_ELEM_CNT];
	u32 bufferBase;
	u32 offset = 0U;
	const PmMaster* master = PmGetMasterByIpiMask(isrMask);

	if ((NULL == isrClr) || (NULL == master)) {
		/* Never happens if IPI irq handler is implemented correctly */
		PmDbg("ERROR: IPI source not supported %lu\n", isrMask);
		status = XST_INVALID_PARAM;
		goto done;
	}

	/* Already have first argument (apiId) */
	payload[0] = apiId;
	bufferBase = master->buffer + IPI_BUFFER_REQ_OFFSET;
	for (i = 1U; i < PAYLOAD_ELEM_CNT; i++) {
		offset += PAYLOAD_ELEM_SIZE;
		payload[i] = XPfw_Read32(bufferBase + offset);
	}

	PmProcessRequest(master, payload);

	/*
	 * Master's bitfield in isr register will be cleared based on isrClr
	 * variable value (master's request is handled)
	 */
	*isrClr = master->ipiMask;

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
		PmDbg("ERROR: Unknown processor %lu\n", srcMask);
		status = XST_INVALID_PARAM;
		goto done;
	}

	status = PmProcFsm(proc, PM_PROC_EVENT_SLEEP);
	if ((XST_SUCCESS == status) &&
	    (true == PmMasterIsSuspended(proc->master))) {
		/*
		 * We've just powered down the last processor, now use
		 * opportunistic suspend to power down its parent(s)
		 */
		PmOpportunisticSuspend(proc->node.parent);
	}

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
	int status = XST_SUCCESS;

	if (PMU_IOMODULE_GPI1_GIC_WAKES_ALL_MASK & srcMask) {
		/* Processor GIC wake */
		PmProc* proc = PmGetProcByWakeStatus(srcMask);
		if (NULL != proc) {
			status = PmProcFsm(proc, PM_PROC_EVENT_WAKE);
		} else {
			status = XST_INVALID_PARAM;
		}
	} else if (PMU_LOCAL_GPI1_ENABLE_FPD_WAKE_GIC_PROX_MASK & srcMask) {
		status = PmMasterWake(&pmMasterApu_g);
	} else {
	}

	return status;
}

/**
 * XPfw_PmCheckIpiRequest() - Check whether the IPI interrupt is a PM call
 * @isrVal  IPI's ISR register value
 * @apiId   Pointer to a variable where api id can be returned
 *
 * @return  Check result
 *
 * @note    Call from IPI interrupt routine to check is interrupt a PM call.
 *          Function reads first argument of payload in IPI buffer of
 *          requestor master to determine whether first argument is within
 *          PM API regular ids.
 */
XPfw_PmIpiStatus XPfw_PmCheckIpiRequest(const u32 isrVal,
					u32* const apiId)
{
	XPfw_PmIpiStatus status;
	bool isValid;
	const PmMaster *master = PmGetMasterByIpiMask(isrVal);

	if (NULL == master) {
		/* IPI is not generated by one of the PM supported PUs */
		status = XPFW_PM_IPI_SRC_UNKNOWN;
		goto done;
	}

	/* Api id is first argument in payload */
	*apiId = XPfw_Read32(master->buffer);
	isValid = PmIsApiIdValid(*apiId);
	if (true == isValid) {
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
	pmPowerDomainFpd_g.node.currState = PM_PWR_STATE_ON;
}

/**
 * XPfw_DapRpuWakeEvent() - Inform PM about the RPU DAP wake event
 */
void XPfw_DapRpuWakeEvent(void)
{
	pmPowerIslandRpu_g.node.currState = PM_PWR_STATE_ON;
}
