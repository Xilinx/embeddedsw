/******************************************************************************
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
#include "xpfw_default.h"
#include "pm_master.h"
#include "xpfw_ipi_manager.h"
#include "xpfw_platform.h"
#include "crl_apb.h"
#include "xpfw_error_manager.h"
#include "ipi.h"
#include "csu.h"
#include "pm_reset.h"
#include "xpfw_resets.h"


#define CSU_PCAP_STATUS	(CSU_BASE + 0x00003010U)
#define CSU_PCAP_STATUS_PL_DONE_MASK (1U<<3)

#define LPD_XPPU_CTRL_ADDRESS	0xFF980000U
#define LPD_XPPU_CTRL_EN_MASK	BIT(0)

#define RestartDebug(MSG, ...)	fw_printf("PMUFW: %s: " MSG, __func__, ##__VA_ARGS__)

#ifdef ENABLE_RECOVERY

/* Macros used to track the pahses in restart */
#define XPFW_RESTART_STATE_BOOT 0U
#define	XPFW_RESTART_STATE_INPROGRESS 1U
#define	XPFW_RESTART_STATE_DONE 2U

/* Enable APU restart only if PMU has access to FPD WDT(psu_wdt_1) */
#ifdef XPAR_PSU_WDT_1_DEVICE_ID
	#include "xwdtps.h"
	#define WDT_INSTANCE_COUNT	XPAR_XWDTPS_NUM_INSTANCES
	#define ENABLE_APU_RESTART
#else /* XPAR_PSU_WDT_1_DEVICE_ID */
	#error "ENABLE_RECOVERY is defined but psu_wdt_1 is not assigned to PMU"
#endif

#ifdef ENABLE_APU_RESTART

/* Check if a timeout value was provided in build flags else default to 60 secs */
#if (RECOVERY_TIMEOUT > 0U)
#define WDT_DEFAULT_TIMEOUT_SEC	RECOVERY_TIMEOUT
#else
#define WDT_DEFAULT_TIMEOUT_SEC	60U
#endif

/* Assign default values for WDT params assuming default config */
#define APU_WDT_BASE XPAR_PSU_WDT_1_BASEADDR
#define APU_WDT_CLOCK_FREQ_HZ	XPAR_PSU_WDT_1_WDT_CLK_FREQ_HZ

#define WDT_CRV_SHIFT 12U
#define WDT_PRESCALER 4096U

#define WDT_CLK_PER_SEC ((APU_WDT_CLOCK_FREQ_HZ) / (WDT_PRESCALER))

/* FPD WDT driver instance used within this file */
static XWdtPs FpdWdtInstance;

/* Data strcuture to track restart phases for a Master */
typedef struct XPfwRestartTracker {
	PmMaster *Master; /* Master whose restart cycle is being tracked */
	u32 RestartCount; /* Number of times a master has been restarted */
	u8 RestartState; /* Track different phases in restart cycle */
	u32 WdtBaseAddress; /* Base address for WDT assigend to this master */
	u8 WdtTimeout; /* Timeout value for WDT */
	u8 ErrorId; /* Error Id corresponding to the WDT */
	XWdtPs* WdtPtr; /* Pointer to WDT for this master */
} XPfwRestartTracker;

XPfwRestartTracker RstTrackerList[] ={
		{
			.Master = &pmMasterApu_g,
			.RestartCount = 0U,
			.RestartState = XPFW_RESTART_STATE_BOOT,
			.WdtBaseAddress = APU_WDT_BASE,
			.WdtTimeout= WDT_DEFAULT_TIMEOUT_SEC,
			.WdtPtr = &FpdWdtInstance,
		},
};

static XWdtPs_Config* GetWdtCfgPtr(u32 BaseAddress)
{
	u32 WdtIdx;
	XWdtPs_Config* WdtConfigPtr = NULL;

	/* Search and return Config pointer with given base address */
	for(WdtIdx=0; WdtIdx < WDT_INSTANCE_COUNT; WdtIdx++) {
		WdtConfigPtr = XWdtPs_LookupConfig(WdtIdx);
		if(WdtConfigPtr == NULL) {
			goto Done;
		}
		if(WdtConfigPtr->BaseAddress == APU_WDT_BASE) {
			break;
		}
	}

Done:
	return WdtConfigPtr;
}


static void WdtRestart(XWdtPs* WdtInstPtr, u32 Timeout)
{

	XWdtPs_DisableOutput(WdtInstPtr, XWDTPS_RESET_SIGNAL);
	XWdtPs_Stop(WdtInstPtr);
	/* Setting the divider value */
	XWdtPs_SetControlValue(WdtInstPtr, XWDTPS_CLK_PRESCALE,
			XWDTPS_CCR_PSCALE_4096);
	/* Set the Watchdog counter reset value */
	XWdtPs_SetControlValue(WdtInstPtr, XWDTPS_COUNTER_RESET,
			(Timeout*WDT_CLK_PER_SEC) >> WDT_CRV_SHIFT);
	/* Start the Watchdog timer */
	XWdtPs_Start(WdtInstPtr);
	XWdtPs_RestartWdt(WdtInstPtr);
	/* Enable reset output */
	XWdtPs_EnableOutput(WdtInstPtr, XWDTPS_RESET_SIGNAL);
}

/**
 * XPfw_RestartIsPlDone - check the status of PL DONE bit
 *
 * @return TRUE if its done else FALSE
 */
static bool XPfw_RestartIsPlDone()
{
	return ((XPfw_Read32(CSU_PCAP_STATUS) & CSU_PCAP_STATUS_PL_DONE_MASK) ==
							CSU_PCAP_STATUS_PL_DONE_MASK);
}

bool XPfw_RestartIsSubSysEnabled()
{
	return ((XPfw_Read32(LPD_XPPU_CTRL_ADDRESS) & LPD_XPPU_CTRL_EN_MASK) ==
						LPD_XPPU_CTRL_EN_MASK);
}
/* Send an IPI from PMU_IPI_1 to the master */
static void MasterIdle(PmMaster* Master)
{
	/* This is the first restart, send an IPI */
	Xil_Out32((IPI_BASEADDR + ((u32)0X00031000U)), Master->ipiMask);
}

void XPfw_RestartSystemLevel(void)
{
	bool IsPlUp = XPfw_RestartIsPlDone();
	if(IsPlUp) {
		RestartDebug("Ps Only Reset\r\n");
		XPfw_ResetPsOnly();
	}
	else {
		/* TODO: Req and wait for Ack from PL */
		RestartDebug("SRST\r\n");
		/* Bypass RPLL before SRST : Workaround for a bug in 1.0 Silicon */
		if (XPfw_PlatformGetPsVersion() == XPFW_PLATFORM_PS_V1) {
			XPfw_UtilRMW(CRL_APB_RPLL_CTRL, CRL_APB_RPLL_CTRL_BYPASS_MASK,
					 CRL_APB_RPLL_CTRL_BYPASS_MASK);
		}
		XPfw_RMW32(CRL_APB_RESET_CTRL,
			   CRL_APB_RESET_CTRL_SOFT_RESET_MASK,
			   CRL_APB_RESET_CTRL_SOFT_RESET_MASK);
	}
}

/**
 * @XPfw_RecoveryInit - Initialize WDTs and setup recovery
 *
 * @return XST_SUCCESS if all Restart Trackers were initialized
 *         successfully
 */
int XPfw_RecoveryInit(void)
{
	s32 Status = XST_FAILURE;
	u32 RstIdx;
	XWdtPs_Config *WdtConfigPtr;

	for (RstIdx = 0; RstIdx < ARRAY_SIZE(RstTrackerList); RstIdx++) {
		WdtConfigPtr = GetWdtCfgPtr(RstTrackerList[RstIdx].WdtBaseAddress);
		if (WdtConfigPtr == NULL) {
			Status = XST_FAILURE;
			break;
		}
		/* Initialize and capture the status */
		Status = XWdtPs_CfgInitialize(RstTrackerList[RstIdx].WdtPtr,
				WdtConfigPtr, WdtConfigPtr->BaseAddress);
		if (Status != XST_SUCCESS) {
			break;
		}
		/* Reset the WDT */
		PmResetAssertInt(PM_RESET_SWDT_CRF, PM_RESET_ACTION_PULSE);
		WdtRestart(RstTrackerList[RstIdx].WdtPtr, RstTrackerList[RstIdx].WdtTimeout);
	}
	return Status;
}

/**
 * XPfw_RecoveryHandler() - Handle WDT expiry
 *
 * @ErrorId is the ID corresponding to WDT that has expired
 */
void XPfw_RecoveryHandler(u8 ErrorId)
{
	u32 RstIdx;

	for (RstIdx = 0; RstIdx < ARRAY_SIZE(RstTrackerList); RstIdx++) {
		/* Currently we support only APU restart for FPD WDT timeout */
		if(ErrorId == EM_ERR_ID_FPD_SWDT &&
				RstTrackerList[RstIdx].Master->nid == NODE_APU) {
			if(RstTrackerList[RstIdx].RestartState != XPFW_RESTART_STATE_INPROGRESS ) {
				RestartDebug("Initiating APU sub-system restart\r\n");
				RstTrackerList[RstIdx].RestartState = XPFW_RESTART_STATE_INPROGRESS;
				RstTrackerList[RstIdx].RestartCount++;
				WdtRestart(RstTrackerList[RstIdx].WdtPtr, RstTrackerList[RstIdx].WdtTimeout);
				MasterIdle(RstTrackerList[RstIdx].Master);
			}
			else{
				RestartDebug("Escalating to system level reset\r\n");
				#ifdef ENABLE_ESCALATION
					XPfw_RestartSystemLevel();
				#else
					PmMasterRestart(RstTrackerList[RstIdx].Master);
				#endif /* ENABLE_ESCALATION */
			}
		}
	}
}

/**
 * XPfw_RecoveryAck - Acknowledge the reception of restart call from a master
 * @Master is the PM master from which restart call has been received
 *
 * @note: The restart stae corresponding to this master is set to DONE when
 * this function is called.
 */
void XPfw_RecoveryAck(PmMaster *Master)
{
	u32 RstIdx;
	for (RstIdx = 0; RstIdx < ARRAY_SIZE(RstTrackerList); RstIdx++) {
		/* Currently we support only APU restart */
		if(RstTrackerList[RstIdx].Master == Master) {
			RstTrackerList[RstIdx].RestartState = XPFW_RESTART_STATE_DONE;
			WdtRestart(RstTrackerList[RstIdx].WdtPtr, RstTrackerList[RstIdx].WdtTimeout);
		}
	}
}



#endif /* ENABLE_APU_RESTART */

#else /* ENABLE_RECOVERY */
void XPfw_RecoveryAck(PmMaster *Master)
{

}

void XPfw_RecoveryHandler(u8 ErrorId)
{

}

int XPfw_RecoveryInit(void)
{
	/* Recovery is not enabled. So return a failure code */
	return XST_FAILURE;
}
#endif /* ENABLE_RECOVERY */
