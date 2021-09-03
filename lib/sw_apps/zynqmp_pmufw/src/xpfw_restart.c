/******************************************************************************
* Copyright (c) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
#include "pm_csudma.h"
#include "xpfw_aib.h"
#if defined(USE_DDR_FOR_APU_RESTART) && defined(ENABLE_SECURE)
#include "xsecure_sha.h"

static XSecure_Sha3 Sha3Instance;
#endif
FSBL_Store_Restore_Info_Struct FSBL_Store_Restore_Info = {0U};

#ifdef ENABLE_RECOVERY

#define XPFW_RESTART_SCOPE_REG		PMU_GLOBAL_GLOBAL_GEN_STORAGE4
#define XPFW_RESTART_SCOPE_SHIFT	(3U)
#define XPFW_RESTART_SCOPE_MASK		(0x3U << XPFW_RESTART_SCOPE_SHIFT)

#ifdef CHECK_HEALTHY_BOOT

#define XPFW_BOOT_HEALTH_STS		PMU_GLOBAL_GLOBAL_GEN_STORAGE4
#define XPFW_BOOT_HEALTH_GOOD		(0x1U)

#endif
/* Macros used to track the phases in restart */
#define XPFW_RESTART_STATE_BOOT 0U
#define	XPFW_RESTART_STATE_INPROGRESS 1U
#define	XPFW_RESTART_STATE_DONE 2U

/* Check if PMU has access to FPD WDT (psu_wdt_1) and LPD WDT (psu_wdt_0)*/
#if defined(XPMU_FPDWDT) && defined(XPMU_LPDWDT)
	#include "xwdtps.h"
	#define WDT_INSTANCE_COUNT	XPAR_XWDTPS_NUM_INSTANCES
#else /* XPMU_FPDWDT */
	#error "ENABLE_RECOVERY is defined but psu_wdt_0 & psu_wdt_1 is not assigned to PMU"
#endif

/* Check if PMU has access to TTC_9 */
#ifdef XPMU_XTTCPS_9
	#include "xttcps.h"
#else /* XPMU_XTTCPS_9 */
	#error "ENABLE_RECOVERY is defined but psu_tcc_9 is not assigned to PMU, APU recovery will not work"
#endif

/* Check if a timeout value was provided in build flags else default to 120 secs */
#if (RECOVERY_TIMEOUT > 0U)
#define WDT_DEFAULT_TIMEOUT_SEC	RECOVERY_TIMEOUT
#else
#define WDT_DEFAULT_TIMEOUT_SEC	60U
#endif

/* Assign default values for WDT params assuming default config */
#define WDT_CRV_SHIFT 12U
#define WDT_PRESCALER 4096U

#define WDT_CLK_PER_SEC ((XPMU_FPDWDT_WDT_CLK) / (WDT_PRESCALER))

#define TTC_PRESCALER			15U
#define TTC_COUNT_PER_SEC		(XPAR_XTTCPS_0_TTC_CLK_FREQ_HZ / 65535U)
#define TTC_DEFAULT_NOTIFY_TIMEOUT_SEC	0U

/* FPD WDT driver instance used within this file */
static XWdtPs WdtInstance;

/* TTC driver instance used within this file */
static XTtcPs FpdTtcInstance;

/* Data strcuture to track restart phases for a Master */
typedef struct XPfwRestartTracker {
	u32 WdtBaseAddress; /* Base address for WDT assigend to this master */
	u8 RestartState; /* Track different phases in restart cycle */
	u8 RestartScope; /* Restart scope upon WDT */
	u8 WdtTimeout; /* Timeout value for WDT */
	u8 ErrorId; /* Error Id corresponding to the WDT */
	u32 WdtResetId; /* WDT reset ID */
	u16 TtcDeviceId; /* TTC timer device ID */
	u8 TtcTimeout; /* Timeout to notify master for event */
	u32 TtcResetId; /* Reset line ID for TTC */
	PmMaster *Master; /* Master whose restart cycle is being tracked */
	XWdtPs* WdtPtr; /* Pointer to WDT for this master */
	XTtcPs *TtcPtr; /* Pointer to TTC for this master */
} XPfwRestartTracker;

static XPfwRestartTracker RstTrackerList[] ={
		{
			.Master = &pmMasterApu_g,
			.RestartState = XPFW_RESTART_STATE_BOOT,
			.WdtBaseAddress = XPMU_FPDWDT_BASEADDR,
			.WdtTimeout= WDT_DEFAULT_TIMEOUT_SEC,
			.WdtPtr = &WdtInstance,
			.WdtResetId = PM_RESET_SWDT_CRF,
			.TtcDeviceId = XPMU_XTTCPS_9,
			.TtcTimeout = TTC_DEFAULT_NOTIFY_TIMEOUT_SEC,
			.TtcPtr = &FpdTtcInstance,
			.TtcResetId = PM_RESET_TTC3,
#ifdef ENABLE_RECOVERY_RESET_SYSTEM
			.RestartScope = PMF_SHUTDOWN_SUBTYPE_SYSTEM,
#elif defined(ENABLE_RECOVERY_RESET_PS_ONLY)
			.RestartScope = PMF_SHUTDOWN_SUBTYPE_PS_ONLY,
#else
			.RestartScope = PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM,
#endif
		},
		{
				.Master = &pmMasterRpu0_g,
				.RestartState = XPFW_RESTART_STATE_BOOT,
				.WdtBaseAddress = XPMU_LPDWDT_BASEADDR,
				.WdtTimeout= WDT_DEFAULT_TIMEOUT_SEC,
				.WdtPtr = &WdtInstance,
				.WdtResetId = PM_RESET_SWDT_CRL,
				.TtcDeviceId = 0,
				.TtcTimeout = 0,
				.TtcPtr = NULL,
				.TtcResetId = 0,
	#ifdef ENABLE_RECOVERY_RESET_SYSTEM
				.RestartScope = PMF_SHUTDOWN_SUBTYPE_SYSTEM,
	#elif defined(ENABLE_RECOVERY_RESET_PS_ONLY)
				.RestartScope = PMF_SHUTDOWN_SUBTYPE_PS_ONLY,
	#else
				.RestartScope = PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM,
	#endif
		},
		{
				.Master = &pmMasterRpu_g,
				.RestartState = XPFW_RESTART_STATE_BOOT,
				.WdtBaseAddress = XPMU_LPDWDT_BASEADDR,
				.WdtTimeout= WDT_DEFAULT_TIMEOUT_SEC,
				.WdtPtr = &WdtInstance,
				.WdtResetId = PM_RESET_SWDT_CRL,
				.TtcDeviceId = 0,
				.TtcTimeout = 0,
				.TtcPtr = NULL,
				.TtcResetId = 0,
	#ifdef ENABLE_RECOVERY_RESET_SYSTEM
				.RestartScope = PMF_SHUTDOWN_SUBTYPE_SYSTEM,
	#elif defined(ENABLE_RECOVERY_RESET_PS_ONLY)
				.RestartScope = PMF_SHUTDOWN_SUBTYPE_PS_ONLY,
	#else
				.RestartScope = PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM,
	#endif
		},
};

static XWdtPs_Config* GetWdtCfgPtr(u32 BaseAddress)
{
	u32 WdtIdx;
	XWdtPs_Config* WdtConfigPtr = NULL;

	/* Search and return Config pointer with given base address */
	for(WdtIdx = 0U; WdtIdx < WDT_INSTANCE_COUNT; WdtIdx++) {
		WdtConfigPtr = XWdtPs_LookupConfig(WdtIdx);
		if (WdtConfigPtr == NULL) {
			goto Done;
		}
		if (BaseAddress == WdtConfigPtr->BaseAddress) {
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
	if (!TtcInstancePtr) {
		goto END;
	}

	/* Stop the timer */
	XTtcPs_Stop(TtcInstancePtr);

	/* Set Interval mode */
	XTtcPs_SetOptions(TtcInstancePtr, XTTCPS_OPTION_INTERVAL_MODE);
	XTtcPs_SetInterval(TtcInstancePtr, (PeriodInSec * TTC_COUNT_PER_SEC));
	XTtcPs_ResetCounterValue(TtcInstancePtr);
	XTtcPs_SetPrescaler(TtcInstancePtr, TTC_PRESCALER);
END:
	return;
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
	if (!TtcInstancePtr) {
		goto END;
	}

	/* Enable interrupt */
	XTtcPs_EnableInterrupts(TtcInstancePtr, XTTCPS_IXR_INTERVAL_MASK);

	/* Start the timer */
	XTtcPs_Start(TtcInstancePtr);
END:
	return;
}

/**
 * XPfw_TTCStop - Stop TTC timer
 * @TtcInstancePtr: Timer instance pointer
 *
 * Stop TTC timer and disable TTC interrupts.
 */
static void XPfw_TTCStop(XTtcPs *TtcInstancePtr)
{
	if (!TtcInstancePtr) {
		goto END;
	}

	/* Stop the timer */
	XTtcPs_Stop(TtcInstancePtr);

	/* Disable interrupt */
	XTtcPs_DisableInterrupts(TtcInstancePtr, XTTCPS_IXR_INTERVAL_MASK);
END:
	return;
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
	XPfw_RMW32(XPFW_BOOT_HEALTH_STS, XPFW_BOOT_HEALTH_GOOD, 0U);
}

#endif

/* Set up the restart scope */
static void SetRestartScope(XPfwRestartTracker *RestartTracker)
{
	/* Set up for master to read and send back the proper restart command */
	XPfw_RMW32(XPFW_RESTART_SCOPE_REG, XPFW_RESTART_SCOPE_MASK,
			   ((u32)RestartTracker->RestartScope << XPFW_RESTART_SCOPE_SHIFT));
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

#ifdef ENABLE_ESCALATION
/**
 * XPfw_RestartIsPlDone - check the status of PL DONE bit
 *
 * @return TRUE if its done else FALSE
 */
static bool XPfw_RestartIsPlDone(void)
{
	return ((XPfw_Read32(CSU_PCAP_STATUS_REG) &
		CSU_PCAP_STATUS_PL_DONE_MASK_VAL) == CSU_PCAP_STATUS_PL_DONE_MASK_VAL);
}

static void XPfw_RestartSystemLevel(void)
{
	bool IsPlUp = XPfw_RestartIsPlDone();
	if(IsPlUp) {
		XPfw_Printf(DEBUG_DETAILED,"Ps Only Reset\r\n");
		XPfw_ResetPsOnly();
	}
	else {
		XPfw_Printf(DEBUG_DETAILED,"SRST\r\n");
		XPfw_ResetSystem();
	}
}
#endif /* ENABLE_ESCALATION */

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
static s32 Xpfw_TTCInit(u16 TtcDeviceId, XTtcPs *TtcInstancePtr)
{
	XTtcPs_Config *timerConfig;
	s32 Status = XST_FAILURE;

	if (!TtcInstancePtr) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Look up the configuration based on the device identifier */
	timerConfig = XTtcPs_LookupConfig(TtcDeviceId);
	if (NULL == timerConfig) {
		Status = XST_FAILURE;
		goto END;
	}

	/* Initialize the device */
	Status = XTtcPs_CfgInitialize(TtcInstancePtr, timerConfig, timerConfig->BaseAddress);

END:
	return Status;
}

/**
 * Xpfw_GetRstTracker - Get WDT reset tracker
 *
 * @return XPfwRestartTracker in case of success else NULL
 */
static XPfwRestartTracker *Xpfw_GetRstTracker(void)
{
	XPfwRestartTracker *Handle = NULL;
	u32 RstIdx;
	u32 FsblProcInfo = XPfw_Read32(PMU_GLOBAL_GLOBAL_GEN_STORAGE5) & FSBL_STATE_PROC_INFO_MASK;

	for (RstIdx = 0; RstIdx < ARRAY_SIZE(RstTrackerList); RstIdx++) {
		if ((FSBL_RUNNING_ON_A53 == FsblProcInfo) &&
				(NODE_APU == RstTrackerList[RstIdx].Master->nid)) {
			XPfw_Printf(DEBUG_DETAILED,"APU\r\n");
			break;
		}

		if ((FSBL_RUNNING_ON_R5_0 == FsblProcInfo) &&
				(NODE_RPU_0 == RstTrackerList[RstIdx].Master->nid)) {
			XPfw_Printf(DEBUG_DETAILED,"RPU0\r\n");
			break;
		}

		if ((FSBL_RUNNING_ON_R5_L == FsblProcInfo) &&
				(NODE_RPU == RstTrackerList[RstIdx].Master->nid)) {
			XPfw_Printf(DEBUG_DETAILED,"RPU LS\r\n");
			break;
		}
	}

	if (ARRAY_SIZE(RstTrackerList) > RstIdx) {
		Handle = &RstTrackerList[RstIdx];
	}

	return Handle;
}

/**
 * @XPfw_RecoveryInit - Initialize WDTs and setup recovery
 *
 * @return XST_SUCCESS if all Restart Trackers were initialized
 *         successfully
 */
s32 XPfw_RecoveryInit(void)
{
	s32 Status = XST_FAILURE;
	XWdtPs_Config *WdtConfigPtr;
	XPfwRestartTracker *RstTracker = Xpfw_GetRstTracker();

	if (NULL == RstTracker) {
		XPfw_Printf(DEBUG_DETAILED,"ASSERT: No Valid Rst Tracker\r\n");
		goto END;
	}

	/*
	 * Reset TTC lines. TTC lines can be same for different TTC ID so
	 * reset them in advance to avoid reset after initialization.
	 */

	if (NULL != RstTracker->TtcPtr) {
		Status = PmResetAssertInt(RstTracker->TtcResetId,
							PM_RESET_ACTION_PULSE);
		if (XST_SUCCESS != Status) {
			goto END;
		}
	}

	WdtConfigPtr = GetWdtCfgPtr(RstTracker->WdtBaseAddress);
	if (NULL == WdtConfigPtr) {
		goto END;
	}
	/* Initialize and capture the status */
	Status = XWdtPs_CfgInitialize(RstTracker->WdtPtr,
						WdtConfigPtr, WdtConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	/* Reset the WDT */
	Status = PmResetAssertInt(RstTracker->WdtResetId, PM_RESET_ACTION_PULSE);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	WdtRestart(RstTracker->WdtPtr, RstTracker->WdtTimeout);

	if (NULL != RstTracker->TtcPtr) {
		Status = Xpfw_TTCInit(RstTracker->TtcDeviceId, RstTracker->TtcPtr);
		if (XST_SUCCESS != Status) {
			goto END;
		}

		XPfw_TimerSetIntervalMode(RstTracker->TtcPtr, RstTracker->TtcTimeout);
	}
END:
	return Status;
}

/**
 * XPfw_RecoveryHandler() - Handle WDT expiry
 *
 * @ErrorId is the ID corresponding to WDT that has expired
 */
void XPfw_RecoveryHandler(u8 ErrorId)
{
	XPfwRestartTracker *RstTracker = Xpfw_GetRstTracker();

	if (NULL == RstTracker) {
		XPfw_Printf(DEBUG_DETAILED,"ASSERT: No Valid Rst Tracker\r\n");
		goto END;
	}

#ifdef CHECK_HEALTHY_BOOT
	u32 DoSubSystemRestart = 0U;

	if (XPfw_GetBootHealthStatus()) {
		/*
		 * Do subsystem restart only if last boot was healthy
		 */
		DoSubSystemRestart=1;
	}
#endif

	if ((EM_ERR_ID_FPD_SWDT == ErrorId) &&
			(NODE_APU != RstTracker->Master->nid)) {
		XPfw_Printf(DEBUG_DETAILED,"ASSERT: Nothing to be done for FPD swdt\r\n");
		goto END;
	}

	if ((EM_ERR_ID_LPD_SWDT == ErrorId) &&
			((NODE_RPU_0 != RstTracker->Master->nid) &&
			(NODE_RPU != RstTracker->Master->nid))) {
		XPfw_Printf(DEBUG_DETAILED,"ASSERT: Nothing to be done for LPD swdt\r\n");
		goto END;
	}

#ifdef CHECK_HEALTHY_BOOT
	if ((XPFW_RESTART_STATE_INPROGRESS != RstTracker->RestartState) &&
			DoSubSystemRestart) {
#else
		if (XPFW_RESTART_STATE_INPROGRESS != RstTracker->RestartState) {
#endif
			XPfw_Printf(DEBUG_DETAILED,"Request Master to idle its cores\r\n");
			RstTracker->RestartState = XPFW_RESTART_STATE_INPROGRESS;
			WdtRestart(RstTracker->WdtPtr, RstTracker->WdtTimeout);


			switch (ErrorId) {
			case EM_ERR_ID_FPD_SWDT:
				/*
				 * Inform ATF to idle APU and issue the PmSystemShutdown for restart scope.
				 */
				SetRestartScope(RstTracker);
				MasterIdle(RstTracker->TtcPtr, RstTracker->TtcTimeout);
				break;
			case EM_ERR_ID_LPD_SWDT:

				/*
				 * Apply Isolation for RPU(M), Slave interface
				 * will be taken care while releasing tcm.
				 * todo: Store the AIB enum in the RstTraker's structure.
				 */

				XPfw_AibEnable(XPFW_AIB_RPU0_TO_LPD);

				/* Note: Need not to enable AIB over rpu1 even in case of
				 * lock-step, as R5-0 will go to sleep.
				 */

				RstTracker->Master->procs[0]->sleep();

				/*
				 * Remove the isolation
				 */
				XPfw_AibDisable(XPFW_AIB_RPU0_TO_LPD);

				if (PMF_SHUTDOWN_SUBTYPE_SUBSYSTEM ==
							RstTracker->RestartScope) {

					XPfw_Printf(DEBUG_DETAILED, "Restarting RPU from WDT\n\r");
					if (XST_SUCCESS != PmMasterRestart(RstTracker->Master)) {
						XPfw_Printf(DEBUG_DETAILED, "Master restart failed");
					}

				} else if (PMF_SHUTDOWN_SUBTYPE_PS_ONLY ==
							RstTracker->RestartScope) {
					XPfw_ResetPsOnly();
				} else if (PMF_SHUTDOWN_SUBTYPE_SYSTEM ==
							RstTracker->RestartScope) {
					XPfw_ResetSystem();
				}

				break;
			default:
				// Fatal: Never Happen
				break;
			}
		} else {
			XPfw_Printf(DEBUG_DETAILED,"Escalating to system level reset\r\n");
#ifdef ENABLE_ESCALATION
			XPfw_RestartSystemLevel();
#else
			/*
			 * Fixme: reset as per the restartScope, don't assume subsystem only.
			 */
			if (XST_SUCCESS != PmMasterRestart(RstTracker->Master)) {
				XPfw_Printf(DEBUG_DETAILED, "Master restart failed\r\n");
			}
#endif /* ENABLE_ESCALATION */
		}
END:
	XPfw_Printf(DEBUG_DETAILED,"Exit restart handler\r\n");
}

/**
 * XPfw_RecoveryAck - Acknowledge the reception of restart call from a master
 * @Master is the PM master from which restart call has been received
 *
 * @note: The restart state corresponding to this master is set to DONE when
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

#else /* ENABLE_RECOVERY */
void XPfw_RecoveryAck(PmMaster *Master) { }

void XPfw_RecoveryHandler(u8 ErrorId) { }

s32 XPfw_RecoveryInit(void)
{
	/* Recovery is not enabled. So return a failure code */
	return XST_FAILURE;
}

void XPfw_RecoveryStop(PmMaster *Master) { }

void XPfw_RecoveryRestart(PmMaster *Master) { }
#endif /* ENABLE_RECOVERY */

#if defined(USE_DDR_FOR_APU_RESTART) && defined(ENABLE_SECURE)
/**
 *
 * This function is used to store the FSBL image from OCM to
 * reserved location of DDR.
 *
 * @param       None
 *
 * @return      Returns the status
 *
 */
s32 XPfw_StoreFsblToDDR(void)
{
	u32 FsblStatus;
	s32 Status;

	Status = PmDmaInit();
	if (XST_SUCCESS != Status) {
		goto END;
	}

	FsblStatus = XPfw_Read32(PMU_GLOBAL_GLOBAL_GEN_STORAGE5);

	/* Check if FSBL is running on A53 and not encrypted, store it to DDR */
	if (FSBL_RUNNING_ON_A53 == (FsblStatus & FSBL_STATE_PROC_INFO_MASK)) {
		if (0x0U == (FsblStatus & FSBL_ENCRYPTION_STS_MASK)) {
			Status = XSecure_Sha3Initialize(&Sha3Instance, &CsuDma);
			if (XST_SUCCESS != Status) {
				goto END;
			}

			(void)memcpy((u32 *)FSBL_STORE_ADDR, (u32 *)FSBL_LOAD_ADDR,
					FSBL_IMAGE_SIZE);

			Status = (s32)XSecure_Sha3Digest(&Sha3Instance,
							(u8 *)FSBL_STORE_ADDR, FSBL_IMAGE_SIZE,
							(u8 *)FSBL_Store_Restore_Info.FSBLImageHash);
			if (XST_SUCCESS != Status) {
				XPfw_Printf(DEBUG_DETAILED, "FSBL image checksum calculation"
											"failed\r\n");
				goto END;
			}
			FSBL_Store_Restore_Info.OcmAndFsblInfo |= XPFW_FSBL_IS_COPIED;
			XPfw_Printf(DEBUG_DETAILED, "Copied FSBL image to DDR\r\n");
		} else {
			XPfw_Printf(DEBUG_DETAILED, "FSBL copy to DDR is skipped.\r\n"
					"Note: APU-only restart will not work if XilFPGA uses OCM "
					"for secure bit-stream loading.\r\n");
		}
	} else {
		XPfw_Printf(DEBUG_PRINT_ALWAYS, "FSBL is running on RPU. \r\n"
				"Warning: APU-only restart is not supported "
				"if FSBL boots on RPU.\r\n");
	}
END:
	return Status;
}

/**
 *
 * This function is used to load the FSBL image from reserved location of DDR
 * to OCM.
 *
 * @param       None
 *
 * @return      Returns the status
 *
 */
s32 XPfw_RestoreFsblToOCM(void)
{
	u32 Index;
	u32 HashCalculated[SHA3_HASH_LENGTH_IN_WORDS] = {0U};
	s32 Status = XST_SUCCESS;

	Status = (s32)XSecure_Sha3Digest(&Sha3Instance, (u8 *)FSBL_STORE_ADDR,
				FSBL_IMAGE_SIZE, (u8 *)HashCalculated);
	if (XST_SUCCESS != Status) {
		XPfw_Printf(DEBUG_DETAILED, "FSBL image checksum calculation"
									"failed\r\n");
		goto END;
	}

	for (Index = 0U; Index < SHA3_HASH_LENGTH_IN_WORDS; Index++) {
		if (FSBL_Store_Restore_Info.FSBLImageHash[Index] !=
				HashCalculated[Index]) {
			Status = XST_FAILURE;
			break;
		} else {
			Status = XST_SUCCESS;
		}
	}

	if (XST_SUCCESS == Status) {
		(void)memcpy((u32 *)FSBL_LOAD_ADDR, (u32 *)FSBL_STORE_ADDR,
				FSBL_IMAGE_SIZE);
		XPfw_Printf(DEBUG_DETAILED, "FSBL image hash checksum matched\r\n");
	} else {
		XPfw_Printf(DEBUG_DETAILED, "FSBL image hash checksum is not matching."
				" This could be due to FSBL image being corrupted. "
				"Unable to do APU-only restart\r\n");
	}

END:
	return Status;
}
#endif
