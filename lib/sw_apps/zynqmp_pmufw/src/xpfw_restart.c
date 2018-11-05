/******************************************************************************
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
#include "xpfw_restart.h"

#define CSU_PCAP_STATUS	(CSU_BASE + 0x00003010U)
#define CSU_PCAP_STATUS_PL_DONE_MASK (1U<<3)

#define LPD_XPPU_CTRL_ADDRESS	0xFF980000U
#define LPD_XPPU_CTRL_EN_MASK	BIT(0U)

#define RestartDebug(DebugType, MSG, ...)	\
	XPfw_Printf((DebugType), "%s" MSG, __func__, ##__VA_ARGS__)

#ifdef ENABLE_RECOVERY

#ifdef CHECK_HEALTHY_BOOT

#define XPFW_BOOT_HEALTH_STS		PMU_GLOBAL_GLOBAL_GEN_STORAGE4
#define XPFW_BOOT_HEALTH_GOOD		(0x1U)

#endif
/* Macros used to track the phases in restart */
#define XPFW_RESTART_STATE_BOOT 0U
#define	XPFW_RESTART_STATE_INPROGRESS 1U
#define	XPFW_RESTART_STATE_DONE 2U

/* Check if PMU has access to FPD WDT (psu_wdt_1) */
#ifdef XPAR_PSU_WDT_1_DEVICE_ID
	#include "xwdtps.h"
	#define WDT_INSTANCE_COUNT	XPAR_XWDTPS_NUM_INSTANCES
#else /* XPAR_PSU_WDT_1_DEVICE_ID */
	#error "ENABLE_RECOVERY is defined but psu_wdt_1 is not assigned to PMU"
#endif

/* Check if PMU has access to TTC_9 */
#ifdef XPAR_PSU_TTC_9_DEVICE_ID
	#include "xttcps.h"
#else /* XPAR_PSU_TTC_9_DEVICE_ID */
	#error "ENABLE_RECOVERY is defined but psu_tcc_9 is not assigned to PMU"
#endif

/* Enable APU restart only if PMU has access to FPD WDT(psu_wdt_1) and TTC9 */
#if defined(XPAR_PSU_WDT_1_DEVICE_ID) && defined(XPAR_PSU_TTC_9_DEVICE_ID)
	#define ENABLE_APU_RESTART
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

#define TTC_PRESCALER			15
#define TTC_COUNT_PER_SEC		(XPAR_XTTCPS_0_TTC_CLK_FREQ_HZ / 65535)
#define TTC_DEFAULT_NOTIFY_TIMEOUT_SEC	0U

/* FPD WDT driver instance used within this file */
static XWdtPs FpdWdtInstance;

/* TTC driver instance used within this file */
static XTtcPs FpdTtcInstance;

/* Data strcuture to track restart phases for a Master */
typedef struct XPfwRestartTracker {
	PmMaster *Master; /* Master whose restart cycle is being tracked */
	u32 RestartCount; /* Number of times a master has been restarted */
	u8 RestartState; /* Track different phases in restart cycle */
	u32 WdtBaseAddress; /* Base address for WDT assigend to this master */
	u8 WdtTimeout; /* Timeout value for WDT */
	u8 ErrorId; /* Error Id corresponding to the WDT */
	XWdtPs* WdtPtr; /* Pointer to WDT for this master */
	u16 TtcDeviceId; /* TTC timer device ID */
	XTtcPs *TtcPtr; /* Pointer to TTC for this master */
	u8 TtcTimeout; /* Timeout to notify master for event */
	u32 TtcResetId; /* Reset line ID for TTC */
} XPfwRestartTracker;

static XPfwRestartTracker RstTrackerList[] ={
		{
			.Master = &pmMasterApu_g,
			.RestartCount = 0U,
			.RestartState = XPFW_RESTART_STATE_BOOT,
			.WdtBaseAddress = APU_WDT_BASE,
			.WdtTimeout= WDT_DEFAULT_TIMEOUT_SEC,
			.WdtPtr = &FpdWdtInstance,
			.TtcDeviceId = XPAR_PSU_TTC_9_DEVICE_ID,
			.TtcTimeout = TTC_DEFAULT_NOTIFY_TIMEOUT_SEC,
			.TtcPtr = &FpdTtcInstance,
			.TtcResetId = PM_RESET_TTC3,
		},
};

static XWdtPs_Config* GetWdtCfgPtr(u32 BaseAddress)
{
	u32 WdtIdx;
	XWdtPs_Config* WdtConfigPtr = NULL;

	/* Search and return Config pointer with given base address */
	for(WdtIdx = 0U; WdtIdx < WDT_INSTANCE_COUNT; WdtIdx++) {
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


static void WdtRestart(XWdtPs* WdtInstptr, u32 Timeout)
{

	XWdtPs_DisableOutput(WdtInstptr, XWDTPS_RESET_SIGNAL);
	XWdtPs_Stop(WdtInstptr);
	/* Setting the divider value */
	XWdtPs_SetControlValue(WdtInstptr, XWDTPS_CLK_PRESCALE,
			XWDTPS_CCR_PSCALE_4096);
	/* Set the Watchdog counter reset value */
	XWdtPs_SetControlValue(WdtInstptr, XWDTPS_COUNTER_RESET,
			(Timeout*WDT_CLK_PER_SEC) >> WDT_CRV_SHIFT);
	/* Start the Watchdog timer */
	XWdtPs_Start(WdtInstptr);
	XWdtPs_RestartWdt(WdtInstptr);
	/* Enable reset output */
	XWdtPs_EnableOutput(WdtInstptr, XWDTPS_RESET_SIGNAL);
}

static void WdtStop(XWdtPs* WdtInstPtr)
{
	/* Disable WDT restart output and stop WDT */
	XWdtPs_DisableOutput(WdtInstPtr, XWDTPS_RESET_SIGNAL);

	/* Stop the timer */
	XWdtPs_Stop(WdtInstPtr);
}

/**
 * XPfw_TimerSetIntervalMode - Set interval mode of TTC
 * @TtcInstancePtr: Timer instance pointer
 * @PeriodInSec: Timer interval in seconds
 *
 * Set timer mode to interval mode and set interval to specified
 * value.
 */
static void XPfw_TimerSetIntervalMode(XTtcPs *TtcInstancePtr, u32 PeriodInSec)
{
	/* Stop the timer */
	XTtcPs_Stop(TtcInstancePtr);

	/* Set Interval mode */
	XTtcPs_SetOptions(TtcInstancePtr, XTTCPS_OPTION_INTERVAL_MODE);
	XTtcPs_SetInterval(TtcInstancePtr, (PeriodInSec * TTC_COUNT_PER_SEC));
	XTtcPs_ResetCounterValue(TtcInstancePtr);
	XTtcPs_SetPrescaler(TtcInstancePtr, TTC_PRESCALER);
}

/**
 * XPfw_TTCStart - Start TTC timer
 * @TtcInstancePtr: Timer instance pointer
 * @Timeout: Timeout in seconds
 *
 * Start TTC timer and enable TTC interrupts. TTC configurations should
 * be done before this.
 */
static void XPfw_TTCStart(XTtcPs *TtcInstancePtr, u32 Timeout)
{
	/* Enable interrupt */
	XTtcPs_EnableInterrupts(TtcInstancePtr, XTTCPS_IXR_INTERVAL_MASK);

	/* Start the timer */
	XTtcPs_Start(TtcInstancePtr);
}

/**
 * XPfw_TTCStop - Stop TTC timer
 * @TtcInstancePtr: Timer instance pointer
 *
 * Stop TTC timer and disable TTC interrupts.
 */
static void XPfw_TTCStop(XTtcPs *TtcInstancePtr)
{
	/* Stop the timer */
	XTtcPs_Stop(TtcInstancePtr);

	/* Disable interrupt */
	XTtcPs_DisableInterrupts(TtcInstancePtr, XTTCPS_IXR_INTERVAL_MASK);
}

#ifdef CHECK_HEALTHY_BOOT

/**
 * Get the healthy bit state.
 */
u32 XPfw_GetBootHealthStatus(void)
{
	return !(!(XPfw_Read32(XPFW_BOOT_HEALTH_STS) & XPFW_BOOT_HEALTH_GOOD));
}

/**
 * Clear APU healthy bit
 */
void XPfw_ClearBootHealthStatus(void)
{
	XPfw_RMW32(XPFW_BOOT_HEALTH_STS, XPFW_BOOT_HEALTH_GOOD, 0);
}

#endif
/**
 * XPfw_RestartIsPlDone - check the status of PL DONE bit
 *
 * @return TRUE if its done else FALSE
 */
static bool XPfw_RestartIsPlDone(void)
{
	return ((XPfw_Read32(CSU_PCAP_STATUS) & CSU_PCAP_STATUS_PL_DONE_MASK) ==
							CSU_PCAP_STATUS_PL_DONE_MASK);
}

/**
 * MasterIdle - Notify master to execute idle sequence
 * @TtcInstancePtr: Timer instance pointer
 * @Timeout: Timeout in seconds
 *
 * On receiving WDT event, PMU calls this function to start TTC timer
 * to notify master about WDT event.
 */
static void MasterIdle(XTtcPs *TtcInstancePtr, u32 Timeout)
{
	/* This is the first restart, send a TTC event */
	XPfw_TTCStart(TtcInstancePtr, Timeout);
}

static void XPfw_RestartSystemLevel(void)
{
	bool IsPlUp = XPfw_RestartIsPlDone();
	if(IsPlUp) {
		RestartDebug(DEBUG_DETAILED,"Ps Only Reset\r\n");
		XPfw_ResetPsOnly();
	}
	else {
		RestartDebug(DEBUG_DETAILED,"SRST\r\n");
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
 * Xpfw_TTCInit - Initialize TTC timer
 * @TtcDeviceId: TTC timer device ID
 * @TtcInstancePtr: Timer instance pointer
 *
 * Lookup TTC configurations based on TTC device ID and initialize
 * TTC based on configurations.
 *
 * @return XST_SUCCESS in case of success else proper error code
 */
static int Xpfw_TTCInit(u16 TtcDeviceId, XTtcPs *TtcInstancePtr)
{
	XTtcPs_Config *timerConfig;
	s32 Status = XST_FAILURE;

	/* Look up the configuration based on the device identifier */
	timerConfig = XTtcPs_LookupConfig(TtcDeviceId);
	if (NULL == timerConfig) {
		return XST_FAILURE;
	}

	/* Initialize the device */
	Status = XTtcPs_CfgInitialize(TtcInstancePtr, timerConfig, timerConfig->BaseAddress);
	if (XST_SUCCESS != Status) {
		return Status;
	}

	return Status;
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

	/*
	 * Reset TTC lines. TTC lines can be same for different TTC ID so
	 * reset them in advance to avoid reset after initialization.
	 */
	for (RstIdx = 0; RstIdx < ARRAY_SIZE(RstTrackerList); RstIdx++) {
		(void)PmResetAssertInt(RstTrackerList[RstIdx].TtcResetId,
				PM_RESET_ACTION_PULSE);
	}

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
		(void)PmResetAssertInt(PM_RESET_SWDT_CRF, PM_RESET_ACTION_PULSE);
		WdtRestart(RstTrackerList[RstIdx].WdtPtr, RstTrackerList[RstIdx].WdtTimeout);

		Status = Xpfw_TTCInit(RstTrackerList[RstIdx].TtcDeviceId,
				RstTrackerList[RstIdx].TtcPtr);
		if (Status != XST_SUCCESS) {
			break;
		}
		XPfw_TimerSetIntervalMode(RstTrackerList[RstIdx].TtcPtr,
				RstTrackerList[RstIdx].TtcTimeout);
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
#ifdef CHECK_HEALTHY_BOOT
	u32 DoSubSystemRestart = 0;

	if(XPfw_GetBootHealthStatus())
	{
		/*
		 * Do subsystem restart only if last boot was healthy
		 */
		DoSubSystemRestart=1;
	}
#endif

	for (RstIdx = 0U; RstIdx < ARRAY_SIZE(RstTrackerList); RstIdx++) {
		/* Currently we support only APU restart for FPD WDT timeout */
		if(ErrorId == EM_ERR_ID_FPD_SWDT &&
				RstTrackerList[RstIdx].Master->nid == NODE_APU) {
#ifdef CHECK_HEALTHY_BOOT
			if (RstTrackerList[RstIdx].RestartState != XPFW_RESTART_STATE_INPROGRESS &&
					DoSubSystemRestart) {
#else
			if(RstTrackerList[RstIdx].RestartState != XPFW_RESTART_STATE_INPROGRESS ) {
#endif
				RestartDebug(DEBUG_DETAILED,"Initiating APU sub-system restart\r\n");
				RstTrackerList[RstIdx].RestartState = XPFW_RESTART_STATE_INPROGRESS;
				RstTrackerList[RstIdx].RestartCount++;
				WdtRestart(RstTrackerList[RstIdx].WdtPtr, RstTrackerList[RstIdx].WdtTimeout);
				MasterIdle(RstTrackerList[RstIdx].TtcPtr,
					RstTrackerList[RstIdx].TtcTimeout);
			}
			else{
				RestartDebug(DEBUG_DETAILED,"Escalating to system level reset\r\n");
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
	for (RstIdx = 0U; RstIdx < ARRAY_SIZE(RstTrackerList); RstIdx++) {
		/* Currently we support only APU restart */
		if(RstTrackerList[RstIdx].Master == Master) {
			RstTrackerList[RstIdx].RestartState = XPFW_RESTART_STATE_DONE;
			XPfw_TTCStop(RstTrackerList[RstIdx].TtcPtr);
			WdtRestart(RstTrackerList[RstIdx].WdtPtr, RstTrackerList[RstIdx].WdtTimeout);
#ifdef CHECK_HEALTHY_BOOT
			/*
			 * clear the healthy status of the boot.
			 * This has to be set by the targeted application on boot.
			 */
			XPfw_ClearBootHealthStatus();
#endif
		}
	}
}

/**
 * XPfw_RecoveryStop() - Disable recovery logic
 * @Master: PM master who wants to stop WDT recovery
 *
 * Stop recovery logic by stopping WDT timer and stopping
 * TTC timer in case it is running.
 */
void XPfw_RecoveryStop(PmMaster *Master)
{
	u32 RstIdx;

	for (RstIdx = 0; RstIdx < ARRAY_SIZE(RstTrackerList); RstIdx++) {
		if (RstTrackerList[RstIdx].Master == Master) {
			WdtStop(RstTrackerList[RstIdx].WdtPtr);
			XPfw_TTCStop(RstTrackerList[RstIdx].TtcPtr);
		}
	}
}

/**
 * XPfw_RecoveryRestart() - Restart recovery logic
 * @Master: PM master who wants to enable WDT recovery
 *
 * Restart recovery logic by restarting WDT timer and stopping
 * TTC timer in case it is running.
 */
void XPfw_RecoveryRestart(PmMaster *Master)
{
	u32 RstIdx;

	for (RstIdx = 0; RstIdx < ARRAY_SIZE(RstTrackerList); RstIdx++) {
		if (RstTrackerList[RstIdx].Master == Master) {
			WdtRestart(RstTrackerList[RstIdx].WdtPtr, RstTrackerList[RstIdx].WdtTimeout);
			XPfw_TTCStop(RstTrackerList[RstIdx].TtcPtr);
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

void XPfw_RecoveryStop(PmMaster *Master)
{

}

void XPfw_RecoveryRestart(PmMaster *Master)
{

}
#endif /* ENABLE_RECOVERY */
