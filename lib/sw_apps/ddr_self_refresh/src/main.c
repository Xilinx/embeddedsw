/******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
 *
 ******************************************************************************/

/*
 * Zynq DDR Self-refresh
 *   This DDR self refresh application  provides a simple
 *   demonstration of how to enter to/exit from DDR self refresh mode.
 *   This application runs on R5 out of TCM.
 */

#include <sleep.h>
#include <xil_cache.h>
#include <xscugic.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_defs.h"

static XScuGic GicInst;
static XIpiPsu IpiInst;

#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define PMU_IPI_CHANNEL_ID	XPAR_XIPIPSU_0_DEVICE_ID

#define DDR_STATE_OFF	0U
#define DDR_STATE_RET	1U
#define DDR_STATE_ON	2U
#define DDR_STATE_MAX	3U

/**
 * IpiConfigure() - Call to configure IPI peripheral and enable its interrupt
 *		in both peripheral and at the GIC
 * @IpiInst	Ipi Data structure to initialize
 * @GicInst	GIC in which the reception of the interrupt should be enabled
 *
 * @return	Status of configuring the interrupt
 *
 * @note	This function does not enable any IPI interrupt, an interrupt is
 *		enabled when the associated callback is registered. The
 *		function can be called safely multiple times, the IPI will be
 *		effectively configured only the first time when a call is made.
 */
XStatus IpiConfigure(XScuGic *const GicInst, XIpiPsu *const IpiInst)
{
	XStatus status;
	XIpiPsu_Config *IpiCfgPtr;
	static bool initialized = false;

	/* If IPI is already initialized return success */
	if (initialized)
		return XST_SUCCESS;

	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(PMU_IPI_CHANNEL_ID);
	if (NULL == IpiCfgPtr) {
		status = XST_FAILURE;
		pm_print("%s ERROR in getting CfgPtr\n", __func__);
		return status;
	}

	/* Init with the Cfg Data */
	status = XIpiPsu_CfgInitialize(IpiInst, IpiCfgPtr,
				       IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != status) {
		pm_print("%s ERROR #%d in configuring IPI\n", __func__, status);
		return status;
	}

	initialized = true;

	return status;
}

/**
 * GicSetupInterruptSystem() - configure the system to receive peripheral
 *			       interrupt
 *
 * Does everything that is needed for enabling interrupts (gic setup,
 * Handler connecting, interrupt enabling on processor and gic level)
 *
 * @return:	status of operation success (XST_* from xstatus.h)
 */
static s32 GicSetupInterruptSystem(XScuGic *GicInst)
{
	s32 status;

	XScuGic_Config *GicCfgPtr = XScuGic_LookupConfig(INTC_DEVICE_ID);

	if (NULL == GicCfgPtr)
		return XST_FAILURE;

	status = XScuGic_CfgInitialize(GicInst, GicCfgPtr,
				       GicCfgPtr->CpuBaseAddress);
	if (XST_SUCCESS != status)
		return status;

	/*
	 * Connect the interrupt controller interrupt Handler to the
	 * hardware interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
				     GicInst);

	if (XST_SUCCESS != status)
		return status;

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/**
 * PmInit - Initialize GIC, IPIs, and Xilpm
 * @GicInst	Pointer to GIC driver instance
 * @XIpiPsu	Pointer to IPI driver instance
 *
 * @return	Status of PM initialization
 */
static XStatus PmInit(XScuGic *const GicInst, XIpiPsu *const IpiInst)
{
	XStatus status;

	status = GicSetupInterruptSystem(GicInst);
	if (XST_SUCCESS != status) {
		pm_print("GIC setup failed.\n");
		return status;
	}

	status  = IpiConfigure(GicInst, IpiInst);
	if (XST_SUCCESS != status) {
		pm_print("IPI configuration failed.\n");
		return status;
	}

	status = XPm_InitXilpm(IpiInst);
	if (XST_SUCCESS != status) {
		pm_print("Xilpm library initialization failed.\n");
		return status;
	}

	status = XPm_InitFinalize();
	if (XST_SUCCESS != status) {
		pm_print("Failed to finalize init\n");
		return status;
	}

	return status;
}


int main(void)
{
	XStatus status;
	XPm_NodeStatus nodestatus;

	/* Initialize GIC, IPIs, and Xilpm */
	status = PmInit(&GicInst, &IpiInst);
	if (XST_SUCCESS != status) {
		pm_print("PM initialization failed\n");
		return status;
	}

	/* Request DDR */
	status = XPm_RequestNode(NODE_DDR,PM_CAP_ACCESS, 100, 1);
	if (XST_SUCCESS != status) {
		pm_print("Failed to request DDR node\n");
		return status;
	}

	/*
	 * Turn off unnecessary processors
	 * All PM masters must call the PmInitFinalize() API when they have
	 * finished initializing all their slave nodes. Otherwise, the PMU
	 * will not power down any slave node. This can be avoided by powering
	 * down unused master.
	 */
	XPm_RequestWakeUp(NODE_RPU_1, 1, 0, REQUEST_ACK_NO);
	XPm_ForcePowerDown(NODE_RPU_1, REQUEST_ACK_NO);
	XPm_ForcePowerDown(NODE_APU, REQUEST_ACK_NO);

	Xil_DCacheDisable();

	pm_print("Put DDR in retention mode.\r\n");
	status = XPm_SetRequirement(NODE_DDR, PM_CAP_CONTEXT, 0,
				    REQUEST_ACK_NO);
	if (XST_SUCCESS != status) {
		pm_print("Failed to set DDR requirement\n");
	} else {
		status = XPm_GetNodeStatus(NODE_DDR, &nodestatus);
		if (XST_SUCCESS != status) {
			pm_print("Get Node status failed\n");
			return status;
		} else if (nodestatus.status != DDR_STATE_RET &&
			   nodestatus.usage > 1) {
			pm_print("Warning: DDR is being used by other master, Can't be put in Self-Refresh mode\n");
			return status;
		} else if (nodestatus.status == DDR_STATE_RET) {
			pm_print("DDR is in self-refresh mode\n");
		} else {
			pm_print("Unknown state\n");
		}
	}

	sleep(10);

	pm_print("Bring DDR out of retention mode.\r\n");
	status = XPm_SetRequirement(NODE_DDR, PM_CAP_ACCESS, 0, REQUEST_ACK_NO);
	if (XST_SUCCESS != status) {
		pm_print("Failed to set DDR requirement\n");
	} else {
		status = XPm_GetNodeStatus(NODE_DDR, &nodestatus);
		if (XST_SUCCESS != status) {
			pm_print("Get Node status failed\n");
			return status;
		} else if (nodestatus.status == DDR_STATE_RET) {
			pm_print("Failed to get DDR out of self-refresh mode\n");
			return status;
		} else if (nodestatus.status == DDR_STATE_ON) {
			pm_print("DDR is out of self-refresh mode\n");
		} else {
			pm_print("Unknown state\n");
		}
	}

	Xil_DCacheEnable();

	status = XPm_ReleaseNode(NODE_DDR);
	if (XST_SUCCESS != status) {
		pm_print("Failed to release DDR node\n");
		return status;
	}

	return status;
}
