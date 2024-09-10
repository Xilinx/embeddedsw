/**************************************************************************************************
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_main.c
 * @addtogroup Overview
 * @{
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
 *
 * </pre>
 *
 *************************************************************************************************/

/*************************************** Include Files *******************************************/

#include "xasufw_resourcemanager.h"
#include "xasufw_init.h"
#include "xasufw_debug.h"
#include "xasufw_ipi.h"
#include "xasufw_sharedmem.h"
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
	Status = XAsufw_Init();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "ASUFW init failed. Error: 0x%x\r\n", Status);
		XFIH_GOTO(END);
	}

	/* Set FW_Is_Present bit before going to the task dispatch loop */
	XAsufw_RMW(ASU_GLOBAL_GLOBAL_CNTRL, ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK,
		   ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK);

	/**
	 * Call task dispatch loop to check and execute the tasks.
	 * When no tasks are in the task queue to be executed, enter into sleep mode.
	 */
	XTask_DispatchLoop();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes ASU microblaze by enabling exceptions, interrupts and
 * 		starting the timer.
 *
 * @return
 *		- On successful initialization, it returns XASUFW_SUCCESS.
 *		Otherwise, it returns unique error code received from the callee function.
 *
 *************************************************************************************************/
static s32 XAsufw_Init(void)
{
	s32 Status = XASUFW_FAILURE;

	XTask_Init();

	XAsufw_ResourceInit();

	XAsufw_RtcaInit();

	Status = XAsufw_StartTimer();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "Timer init failed\r\n");
		XFIH_GOTO(END);
	}

	/* Setup ASUFW interrupts and enable */
	Status = XAsufw_SetUpInterruptSystem();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "ASUFW interrupt setup failed. Error: 0x%x\r\n", Status);
		XFIH_GOTO(END);
	}

	Status = XAsufw_DmaInit();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "DMA init failed with error: 0x%x\r\n", Status);
		XFIH_GOTO(END);
	}

	/* ASU IPI initialization */
	Status = XAsufw_IpiInit();
	if (XASUFW_SUCCESS != Status) {
		XFIH_GOTO(END);
	}

	/* Communication channel and their shared memory initialization */
	Status = XAsufw_SharedMemoryInit();
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "Comm channel and shared memory initialization failed\r\n");
		XFIH_GOTO(END);
	}

	/* Initialize all ASUFW modules */
	Status = XAsufw_ModulesInit();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function initializes all modules supported by ASUFW.
 *
 * @return
 * 		- On successful initialization, it returns XASUFW_SUCCESS.
 *		Otherwise, it returns unique error code received from the caller function.
 *
 *************************************************************************************************/
static s32 XAsufw_ModulesInit(void)
{
	s32 Status = XASUFW_FAILURE;

	/* FIH initialization */
	XFih_Init();

	/* SHA2 module initialization */
	Status = XAsufw_Sha2Init();
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* SHA3 module initialization */
	Status = XAsufw_Sha3Init();
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* TRNG module initialization */
	Status = XAsufw_TrngInit();
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* RSA module initialization */
	Status = XAsufw_RsaInit();
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* ECC module initialization */
	Status = XAsufw_EccInit();
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	/* AES module initialization */
	Status = XAsufw_AesInit();
	if (Status != XASUFW_SUCCESS) {
		XFIH_GOTO(END);
	}

	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "Modules init done\r\n");

END:
	return Status;
}
/** @} */