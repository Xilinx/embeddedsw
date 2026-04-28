/**************************************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_main.c
 *
 * This is the main file which contains code for the ASUFW
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   10/11/23 Initial release
 * 1.1   ma   01/02/24 Call IPI init and commands init functions in XAsufw_Init
 * 1.2   vns  02/20/24 Call Resource Manager init function
 *       ma   03/16/24 Added error codes at required places
 *       ma   03/23/24 Call DMA init function
 *       ma   04/18/24 Call modules initialization functions
 *       ma   05/14/24 Initialize SHA2 and SHA3 modules
 *       ma   05/20/24 Initialize TRNG module
 *       ma   07/08/24 Removed generic module
 *       ma   07/23/24 Added RTCA initialization related code
 *       ma   07/30/24 Set FW_Is_Present bit in GLOBAL_CNTRL register after ASUFW initialization
 *                     is complete
 *       ss   08/20/24 Initialize RSA module
 *       yog  08/21/24 Initialize ECC module
 *       am   08/01/24 Added AES module initialization
 *       yog  08/25/24 Initialize FIH
 *       ss   09/26/24 Fixed doxygen comments
 *       yog  01/02/25 Initialize HMAC module
 *       ma   01/15/25 Initialize KDF module
 *       ma   02/21/25 Initialize error management functionality
 *       yog  02/24/25 Initialize ECIES module
 *       am   04/04/25 Added PMC key transfer support
 *       rmv  08/11/25 Initialize PLM event module
 *       rmv  08/11/25 Initialize OCP module
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/

#include "xasufw_resourcemanager.h"
#include "xasufw_init.h"
#include "xasufw_debug.h"
#include "xasufw_ipi.h"
#include "xasufw_queuescheduler.h"
#include "xasufw_status.h"
#include "xtask.h"
#include "xasufw_dma.h"
#include "xasufw_sha2handler.h"
#include "xasufw_sha3handler.h"
#include "xasufw_trnghandler.h"
#include "xasufw_hw.h"
#include "xasufw_rsahandler.h"
#include "xasufw_ecchandler.h"
#include "xasufw_aeshandler.h"
#include "xasufw_hmachandler.h"
#include "xasufw_kdfhandler.h"
#include "xasufw_ecieshandler.h"
#include "xasufw_keywraphandler.h"
#include "xfih.h"
#include "xasufw_error_manager.h"
#include "xasufw_config.h"
#include "xasufw_perf.h"
#include "xasufw_plmeventhandler.h"
#include "xasufw_ocphandler.h"
#include "xasufw_kat.h"
#include "xasufw_keymanagerhandler.h"
#include "xasufw_lmshandler.h"
#include "xocp.h"
#include "xil_error_node.h"
#include "xrsa.h"
#include "xasufw_update.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
/*
 * Lowest user-allowed GCC constructor priority (0-100 are reserved). Ensures
 * sbss is cleared before any default-priority constructor (e.g. xtimerinit())
 * on the in-place update path, where startup code does not re-zero sbss.
 */
#define XASUFW_CLEAR_SBSS_CTOR_PRIORITY		(101)

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_Init(void);
static s32 XAsufw_ModulesInit(void);
static void XAsufw_SetAsufwPresentBit(void);
static void XAsufw_ClearSbss(void) __attribute__((constructor(XASUFW_CLEAR_SBSS_CTOR_PRIORITY)));
#ifdef XASU_OCP_ENABLE
static s32 XAsufw_RunOcpKeyGeneration(void);
#endif

/************************************ Variable Definitions ***************************************/
/* Symbols provided by the linker script demarcating the sbss section. */
extern u8 __sbss_start[];
extern u8 __sbss_end[];


/*************************************************************************************************/
/**
 * @brief	This is the main ASUFW function which will initialize the processor, starts timer,
 * 		initializes interrupts, initializes modules and calls XTask_DispatchLoop to execute the
 * 		tasks based on priority.
 *
 * @return
 *		- This function never returns. In case if it reaches end due to any error during
 * 		  initialization, it returns XASUFW_FAILURE.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XASUFW_FAILURE;
	XFih_Var FihVar = XFih_VolatileAssignXfihVar(XFIH_FAILURE);

	/** Restore data backup if update state indicates load ELF done. */
	if (XAsufw_GetUpdateState() == XASUFW_UPDATE_STATE_LOAD_ELF_DONE) {
		Status = XAsufw_RestoreDataBackup();
		if (Status != XASUFW_SUCCESS) {
			XAsufw_Printf(DEBUG_GENERAL, "\r\n ASUFW restore data backup failed. Error: 0x%0x\r\n", Status);
		}
	}

	/** Validate debug log buffer information. */
	XAsufw_ValidateDebugLogBufferInfo();

	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "\r\nXilinx Versal_2VE_2VM Application Security Unit Firmware\r\n");
	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "Release %d.%d", SDK_RELEASE_YEAR, SDK_RELEASE_QUARTER);
	XAsufw_Printf(DEBUG_PRINT_ALWAYS, " %s - %s\r\n", __DATE__, __TIME__);

	/** Initialize ASUFW. */
	Status = XAsufw_Init();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "ASUFW init failed. Error: 0x%x\r\n", Status);
		goto END;
	}

	/** Initialize error manager functionality. */
	Status = XAsufw_ErrorManagerInit();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "Error management init failed. Error: 0x%x\r\n", Status);
		goto END;
	}

	/** Update access permissions for all modules. */
	Status = XAsufw_UpdateAccessPermissions();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "ASUFW update access permissions failed. Error: 0x%x\r\n",
					Status);
		goto END;
	}

	/**
	 * Check if crypto is disabled via eFuse and report non-fatal error to PLM if disabled
	 * and exit ASUFW execution.
	 */
	if (((Xil_In32(EFUSE_CACHE_IP_DISABLE_0_ADDR) & EFUSE_CACHE_IP_DISABLE_0_PS_CRYPTO_DIS_MASK))
				== EFUSE_CACHE_IP_DISABLE_0_PS_CRYPTO_DIS_MASK) {
		XAsufw_Printf(DEBUG_GENERAL, "Crypto is disabled via eFuse.\r\n");
		XAsufw_SendErrorToPlm(XASUFW_NON_FATAL_ERROR, (s32)XASUFW_CRYPTO_DISABLED);
		goto END;
	}
	/**
	 * Set FW_Is_Present bit in ASU_GLOBAL GLOBAL_CNTRL register and RTCA address reserved for FW_IS_PRSNT.
	 * Clients need to check the bit in RTCA area before sending any requests to ASUFW.
	 */
	XAsufw_SetAsufwPresentBit();

	/** Pause performance monitoring during KAT execution. */
	XASUFW_PERF_SET_MONITORING_STATE(XASUFW_PERF_MON_PAUSED);

	/** Run KATs for all crypto modules. */
	XAsufw_RunCryptoKats();

	/** Resume performance monitoring after KATs complete. */
	XASUFW_PERF_SET_MONITORING_STATE(XASUFW_PERF_MON_RESUMED);

	/** Run key transfer and PUF regeneration. */
	XFIH_CALL(XAsufw_PmcKeyTransfer, FihVar, Status);
	if (Status != XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "PMC key transfer failed. Error: 0x%x\r\n", Status);
	}

	/** Enable event notifiers. */
	Status = XAsufw_EnableDisableEventNotifiers(XASUFW_REGISTER_NOTIFIER_ENABLE);
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "ASUFW enable event notifiers failed. Error: 0x%x\r\n", Status);
	}

#ifdef XASU_OCP_ENABLE
	/**
	 * Generate device keys for OCP functionalities. In case of failure in generating device
	 * keys, continue booting and supporting other requests.
	 */
	Status = XAsufw_RunOcpKeyGeneration();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "OCP key generation failed. Error: 0x%x\r\n", Status);
	}
#endif

	/**
	 * Start background RSA key pair generation if ASU vault was created successfully.
	 * Keys are generated and stored in ASU vault. Key generation runs via the task
	 * scheduler without blocking the main firmware operations.
	 * In case of failure, continue booting.
	 */
#ifdef XASU_KEYMANAGER_ENABLE
	if (XKeyManager_IsAsuVaultCreated() == XASU_STATUS_PASS) {
		Status = XRsa_AddKeyPairGenToScheduler();
		if (XASUFW_SUCCESS != Status) {
			XAsufw_Printf(DEBUG_GENERAL, "RSA key pair gen scheduler init failed. Error: 0x%x\r\n",
				      Status);
		}
	}
#endif

	/** Check if this is a post-update boot and trigger pending requests. */
	if (XAsufw_GetUpdateState() == XASUFW_UPDATE_STATE_LOAD_ELF_DONE) {
		XAsufw_PostUpdateTriggerPendingRequests();
		XAsufw_SetUpdateState(XASUFW_UPDATE_STATE_FINISHED);
	}

	/**
	 * Call task dispatch loop to check and execute the tasks.
	 * When no tasks are in the task queue to be executed, enter sleep mode.
	 */
	XTask_DispatchLoop();

	/**
	 * Clear FW_Is_Present bit in ASU_GLOBAL GLOBAL_CNTRL register if code reaches here which
	 * it is not supposed to reach.
	 */
	/*
	 * TODO: Need to add code for clearing security critical data if any.
	 */
	XAsufw_RMW(ASU_GLOBAL_GLOBAL_CNTRL, ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK, 0x0U);
	XAsufw_WriteReg(XASU_RTCA_EXEC_STATUS_ADDR, 0x0U);
	while (1U);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs ASU system initialization. It does the following as part of
 * initialization:
 *        - Start Microblaze PIT timer
 *        - Set up interrupts
 *        - Initialize DMAs
 *        - Initialize IPI and enable interrupts
 *        - Initialize IPI shared memory
 *        - Initialize all the modules of ASUFW
 *
 * @return
 *		- XASUFW_SUCCESS, if ASUFW initialization is successful.
 *		- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
static s32 XAsufw_Init(void)
{
	s32 Status = XASUFW_FAILURE;

	/** Enable RAM ECC controllers. */
	XAsufw_RamEccInit();

	/** Initialize task queues. */
	XTask_Init();

	/** Initialize HW resource manager. */
	Status = XAsufw_ResourceInit();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "Resource init failed with error: 0x%x\r\n", Status);
		goto END;
	}

	/** Initialize ASUFW RTC area. */
	XAsufw_RtcaInit();

	/** Initialize PIT timer. */
	Status = XAsufw_StartTimer();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "Timer init failed with error: 0x%x\r\n", Status);
		goto END;
	}

	/** Setup ASUFW interrupts and enable. */
	Status = XAsufw_SetUpInterruptSystem();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "ASUFW interrupt setup failed with error: 0x%x\r\n", Status);
		goto END;
	}

	/** Initialize both ASU DMA0 and ASU DMA1. */
	Status = XAsufw_DmaInit();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "DMA init failed with error: 0x%x\r\n", Status);
		goto END;
	}

	/** Initialize ASU IPI. */
	Status = XAsufw_IpiInit();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "IPI init failed with error: 0x%x\r\n", Status);
		goto END;
	}

	/** Initialize communication channel and their shared memory. */
	Status = XAsufw_ChannelConfigInit();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "Channel config init failed with error: 0x%x\r\n",
			      Status);
		goto END;
	}

	/** Initialize all ASUFW modules. */
	Status = XAsufw_ModulesInit();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "ASUFW module initialization failed with error: 0x%x\r\n", Status);
		goto END;
	}

	XAsufw_UpdateModulesInfo();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes all modules supported by ASUFW.
 *
 * @return
 *		- XASUFW_SUCCESS, if ASUFW modules initialization is successful.
 *		- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
static s32 XAsufw_ModulesInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/** FIH library initialization. */
	XFih_Init();

	/** SHA2 module initialization. */
	Status = XAsufw_Sha2Init();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** SHA3 module initialization. */
	Status = XAsufw_Sha3Init();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** TRNG module initialization. */
	Status = XAsufw_TrngInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** RSA module initialization. */
	Status = XAsufw_RsaInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** ECC module initialization. */
	Status = XAsufw_EccInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** AES module initialization. */
	Status = XAsufw_AesInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

#ifdef XASU_HMAC_ENABLE
	/** HMAC module initialization. */
	Status = XAsufw_HmacInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}
#endif

#ifdef XASU_KDF_ENABLE
	/** KDF module initialization. */
	Status = XAsufw_KdfInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}
#endif

#ifdef XASU_ECIES_ENABLE
	/** ECIES module initialization. */
	Status = XAsufw_EciesInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}
#endif

#ifdef XASU_KEYWRAP_ENABLE
	/** Key wrap unwrap module initialization. */
	Status = XAsufw_KeyWrapInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}
#endif

#ifdef XASU_LMS_ENABLE
	/** LMS module initialization. */
	Status = XAsufw_LmsInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}
#endif

	/** PLM event handler initialization. */
	Status = XAsufw_PlmInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

#ifdef XASU_OCP_ENABLE
	/** PLM event handler initialization. */
	Status = XAsufw_OcpInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}
#endif

#ifdef XASU_KEYMANAGER_ENABLE
	/** Key manager module initialization. */
	Status = XAsufw_KeyManagerInit();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}
#endif

	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "Modules init done\r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sets the FW PRESENT status in ASU GLOBAL register and RTCA address.
 *
 *************************************************************************************************/
static void XAsufw_SetAsufwPresentBit(void)
{
	XAsufw_RMW(ASU_GLOBAL_GLOBAL_CNTRL, ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK,
		   ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK);
	XAsufw_RMW(XASU_RTCA_EXEC_STATUS_ADDR, XASU_RTCA_FW_IS_PRESENT_STATUS_MASK,
			XASU_RTCA_FW_IS_PRESENT_STATUS_VALUE);
}

/*************************************************************************************************/
/**
 * @brief	This function clears the sbss section.
 *
 * @note	Registered as a GCC constructor with priority 101 so that it runs before
 *		the default-priority xtimerinit() constructor in xiltimer.c. This ensures
 *		that on the in-place update path (where startup code does not re-zero sbss),
 *		sbss is cleared first and then xtimerinit() (and other constructors) can
 *		re-initialize their state cleanly.
 *
 *************************************************************************************************/
static void __attribute__((constructor(XASUFW_CLEAR_SBSS_CTOR_PRIORITY))) XAsufw_ClearSbss(void)
{
	s32 Status = XASUFW_FAILURE;
	u32 SbssLen;

	/*
	 * If the linker-provided symbols are inverted or equal, there is no
	 * sbss to clear. Treat this as a no-op rather than computing a
	 * negative/underflowed length.
	 */
	if ((UINTPTR)__sbss_end <= (UINTPTR)__sbss_start) {
		return;
	}

	SbssLen = (u32)((UINTPTR)__sbss_end - (UINTPTR)__sbss_start);

	Status = Xil_SMemSet(__sbss_start, SbssLen, 0U, SbssLen);
	if (Status != XASUFW_SUCCESS) {
		XAsufw_Printf(DEBUG_PRINT_ALWAYS, "Clearing sbss failed. Status = 0x%08x\r\n",
			      (u32)Status);
	}
}

#ifdef XASU_OCP_ENABLE
/*************************************************************************************************/
/**
 * @brief	This function generates device keys during ASUFW boot time.
 *
 * @return
 *	- XASUFW_SUCCESS, if device keys are generated successfully.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_OCP_GET_EVENT_MASK_FAILED, if getting OCP event is failed.
 *
 *************************************************************************************************/
static s32 XAsufw_RunOcpKeyGeneration(void)
{
	CREATE_VOLATILE(Status, XASUFW_FAILURE);
	u32 EventMask = 0U;
	XAsufw_Dma *XOcpAsuDmaPtr = XAsufw_GetDmaInstance(ASUDMA_0_DEVICE_ID);

	/** Get OCP event mask from PLM. */
	Status = XOcp_GetOcpEventMaskFromPlm(&EventMask);
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_OCP_GET_EVENT_MASK_FAILED;
		goto END;
	}

	/** Generate device keys based on OCP event mask. */
	ASSIGN_VOLATILE(Status, XASUFW_FAILURE);
	Status = XOcp_GenerateDeviceKeys(XOcpAsuDmaPtr, EventMask);

END:
	return Status;
}
#endif
/** @} */
