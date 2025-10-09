/**************************************************************************************************
* Copyright (c) 2023 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "xasufw_plmeventhandler.h"
#include "xasufw_ocphandler.h"
#include "xasufw_kat.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_Init(void);
static s32 XAsufw_ModulesInit(void);
static void XAsufw_StartKatTasks(void);
static void XAsufw_StartKeyTransferTasks(void);
static void XAsufw_SetAsufwPresentBit(void);

/************************************ Variable Definitions ***************************************/

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

	/** Initializes the debug log buffer for ASUFW. */
	XAsufw_InitDebugLogBuffer();

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

	/** Create and trigger KAT task. */
	XAsufw_StartKatTasks();

	/**
	 * Create and trigger task created for Key transfer and DME KEK derivation if PUF
	 * regeneration is successful.
	 */
	XAsufw_StartKeyTransferTasks();

	/**
	 * Set FW_Is_Present bit in ASU_GLOBAL GLOBAL_CNTRL register and RTCA address reserved for FW_IS_PRSNT.
	 * Clients need to check the bit in RTCA area before sending any requests to ASUFW.
	 */
	XAsufw_SetAsufwPresentBit();

	/**
	 * Call task dispatch loop to check and execute the tasks.
	 * When no tasks are in the task queue to be executed, enter sleep mode.
	 */
	XTask_DispatchLoop();

	/**
	 * Clear FW_Is_Present bit in ASU_GLOBAL GLOBAL_CNTRL register if code reaches here which
	 * it is not supposed to reach.
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
	XAsufw_ChannelConfigInit();

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

	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "Modules init done\r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function creates KAT tasks and triggers them, the task will be deleted after
 * 		the execution of the KATs.
 *
 *************************************************************************************************/
static void XAsufw_StartKatTasks(void)
{
	XTask_TaskNode *KatTask = XTask_Create(XTASK_PRIORITY_0, XAsufw_RunKatTaskHandler, NULL,
		0U);

	if (KatTask != NULL) {
		/* Self-reference for deletion. */
		KatTask->PrivData = KatTask;
		XTask_TriggerNow(KatTask);
	}
}

/*************************************************************************************************/
/**
 * @brief	This function creates key transfer tasks and triggers them, the task will be deleted
 * 		after the execution of the key transfer tasks.
 *
 *************************************************************************************************/
static void XAsufw_StartKeyTransferTasks(void)
{
	XTask_TaskNode *KeyTransferTask = XTask_Create(XTASK_PRIORITY_0,
		XAsufw_RunKeyTransferTaskHandler, NULL, 0U);

	if (KeyTransferTask != NULL) {
		/* Self-reference for deletion. */
		KeyTransferTask->PrivData = KeyTransferTask;
		XTask_TriggerNow(KeyTransferTask);
	}
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
/** @} */
