/**************************************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
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
#include "xfih.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static s32 XAsufw_Init(void);
static s32 XAsufw_ModulesInit(void);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This is the main ASUFW function which will initialize the processor, starts timer,
 * 		initializes interrupts and calls XTask_DispatchLoop to execute the tasks based on
 *		priority.
 *
 * @return
 *		- This function never returns. In case if it reaches end due to any error during
 * 		  initialization, it returns XASUFW_FAILURE.
 *
 *************************************************************************************************/
int main(void)
{
	s32 Status = XASUFW_FAILURE;

	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "\r\nVersal Gen2 Application Security Unit Firmware\r\n");

	/** Initalize ASUFW. */
	Status = XAsufw_Init();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "ASUFW init failed. Error: 0x%x\r\n", Status);
		goto END;
	}

	/**
	 * Set FW_Is_Present bit in ASU_GLOBAL GLOBAL_CNTRL register.
	 * Clients need to check this bit before sending any requests to ASUFW.
	 */
	XAsufw_RMW(ASU_GLOBAL_GLOBAL_CNTRL, ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK,
		   ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK);

	/**
	 * Call task dispatch loop to check and execute the tasks.
	 * When no tasks are in the task queue to be executed, enter into sleep mode.
	 */
	XTask_DispatchLoop();

	/**
	 * Clear FW_Is_Present bit in ASU_GLOBAL GLOBAL_CNTRL register if code reaches here which
	 * it is not supposed to reach.
	 * TODO: Need to add code for clearing security critical data if any.
	 */
	XAsufw_RMW(ASU_GLOBAL_GLOBAL_CNTRL, ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK, 0x0U);
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
 *		- XASUFW_SUCCESS, On successful initialization.
 *		- XASUFW_FAILURE, if any other failure.
 *
 *************************************************************************************************/
static s32 XAsufw_Init(void)
{
	s32 Status = XASUFW_FAILURE;

	/** Initalize task queues. */
	XTask_Init();

	/** Initalize HW resource manager. */
	XAsufw_ResourceInit();

	/** Initalize ASUFW RTC area. */
	XAsufw_RtcaInit();

	/** Initalize PIT timer. */
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

	/** Initalize DMA. */
	Status = XAsufw_DmaInit();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "DMA init failed with error: 0x%x\r\n", Status);
		goto END;
	}

	/** ASU IPI initialization. */
	Status = XAsufw_IpiInit();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "IPI init failed with error: 0x%x\r\n", Status);
		goto END;
	}

	/** Communication channel and their shared memory initialization. */
	XAsufw_ChannelConfigInit();

	/** Initialize all ASUFW modules. */
	Status = XAsufw_ModulesInit();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes all modules supported by ASUFW.
 *
 * @return
 *		- XASUFW_SUCCESS, On successful initialization.
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

	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "Modules init done\r\n");

END:
	return Status;
}
/** @} */
