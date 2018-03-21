/******************************************************************************
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
#include "xpfw_module.h"
#include "xpfw_error_manager.h"
#include "xpfw_resets.h"
#include "xpfw_events.h"
#include "xpfw_core.h"
#include "xpfw_rom_interface.h"
#include "xpfw_xpu.h"
#include "xpfw_restart.h"
#include "xpfw_mod_em.h"
#include "pmu_lmb_bram.h"

#ifdef ENABLE_EM
const XPfw_Module_t *EmModPtr;

static void EmIpiHandler(const XPfw_Module_t *ModPtr, u32 IpiNum, u32 SrcMask, const u32* Payload, u8 Len)
{
	u32 RetVal = XST_FAILURE;
	u8 ErrorId = 0U;

	if (IpiNum > 0) {
		XPfw_Printf(DEBUG_ERROR,"EM: EM handles only IPI on PMU-0\r\n");
	} else {
		switch (Payload[EM_MOD_API_ID_OFFSET] & EM_API_ID_MASK) {
		case SET_EM_ACTION:
			ErrorId = (u8)Payload[EM_ERROR_ID_OFFSET];
			RetVal = XPfw_EmSetAction(ErrorId, (u8)Payload[EM_ERROR_ACTION_OFFSET],
					ErrorTable[ErrorId].Handler);

			if (RetVal != XST_SUCCESS) {
				XPfw_Printf(DEBUG_DETAILED, "Warning: EmIpiHandler: Failed "
						"to set action \r\n");
			}
			XPfw_IpiWriteResponse(ModPtr, SrcMask, &RetVal, 1);
			break;

		case REMOVE_EM_ACTION:
			RetVal = XPfw_EmDisable((u8)Payload[EM_ERROR_ID_OFFSET]);

			if (RetVal != XST_SUCCESS) {
				XPfw_Printf(DEBUG_DETAILED,"Warning: EmIpiHandler: Failed"
						" to remove action\r\n");
			}
			XPfw_IpiWriteResponse(ModPtr, SrcMask, &RetVal, 1);
			break;

		case SEND_ERRORS_OCCURRED:
			ErrorLog[PMU_BRAM_CE_LOG_OFFSET] = XPfw_Read32(PMU_LMB_BRAM_CE_CNT_REG);

			XPfw_IpiWriteResponse(ModPtr, SrcMask, ErrorLog, EM_ERROR_LOG_MAX);
			break;

		default:
			XPfw_Printf(DEBUG_ERROR,"EM: Unsupported API ID received\r\n");
			XPfw_IpiWriteResponse(ModPtr, SrcMask, &RetVal, 1);
			break;
		}
	}
}
/**
 * Example Error handler for RPU lockstep error
 *
 * This handler should be called when an R5 lockstep error occurs
 * and it resets the RPU gracefully
 */

void RpuLsHandler(u8 ErrorId)
{
	XPfw_Printf(DEBUG_ERROR,"EM: RPU Lock-Step Error Occurred "
			"(Error ID: %d)\r\n", ErrorId);
	XPfw_Printf(DEBUG_ERROR,"EM: Initiating RPU Reset \r\n");
	(void)XPfw_ResetRpu();
}

/**
 * NullHandler() - Null handler for errors which doesn't have a handler defined
 * @ErrorId   ID of the error
 */
void NullHandler(u8 ErrorId)
{
	XPfw_Printf(DEBUG_ERROR,"EM: Error %d occurred\r\n",ErrorId);
}

/**
 * FpdSwdtHandler() - Error handler for FPD system watchdog timer error
 * @ErrorId   ID of the error
 *
 * @note      Called when an error from watchdog timer in the FPD subsystem
 *            occurs and it resets the FPD. APU resumes from HIVEC after reset
 */
void FpdSwdtHandler(u8 ErrorId)
{
	XPfw_Printf(DEBUG_ERROR,"EM: FPD Watchdog Timer Error (Error ID: %d)\r\n",
			ErrorId);
	XPfw_RecoveryHandler(ErrorId);
}

/****************************************************************************/
/**
 * @brief  This scheduler task checks for psu init completion and sets
 *         error action for PLL lock errors
 *
 * @param  None.
 *
 * @return None.
 *
 * @note   None.
 *
 ****************************************************************************/
static void CheckPsuInitCompletion(void)
{
	s32 Status;
	u32 PsuInitStatus = XPfw_Read32(PMU_GLOBAL_GLOBAL_GEN_STORAGE5);

	if (PsuInitStatus == PSU_INIT_COMPLETION) {

		/* Clear previous PLL lock errors if any */
		XPfw_Write32(PMU_GLOBAL_ERROR_STATUS_2, PMU_GLOBAL_ERROR_STATUS_2_PLL_LOCK_MASK);

		/* Set PS Error Out action for PLL lock errors */
		XPfw_EmSetAction(EM_ERR_ID_PLL_LOCK, EM_ACTION_PSERR, NULL);

		Status = XPfw_CoreRemoveTask(EmModPtr, CHECK_PSU_INIT_CONFIG,
				CheckPsuInitCompletion);
		if (XST_FAILURE == Status) {
			XPfw_Printf(DEBUG_ERROR,"EM (MOD-%d):Removing EM config task "
					"failed.", EmModPtr->ModId);
		}
	}
}

/* CfgInit Handler */
static void EmCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData,
		u32 Len)
{
	u32 ErrId = 0U;
	s32 Status;

	/* Register for Error events from Core */
	(void) XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ERROR_1);
	(void) XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ERROR_2);

	/* Init the Error Manager */
	XPfw_EmInit();

	/* Set handlers for error manager */
	for (ErrId = 1U; ErrId < EM_ERR_ID_MAX; ErrId++)
	{
		if (ErrorTable[ErrId].Action != EM_ACTION_NONE) {
			XPfw_EmSetAction(ErrId, ErrorTable[ErrId].Action,
					ErrorTable[ErrId].Handler);
		}
	}

	/*
	 * Schedule a task to check for psu_init completion to enable
	 * PLL lock errors
	 */
	Status = XPfw_CoreScheduleTask(EmModPtr, CHECK_PSU_INIT_CONFIG,
			CheckPsuInitCompletion);
	if (XST_FAILURE == Status) {
		XPfw_Printf(DEBUG_ERROR,"EM (MOD-%d):Scheduling EM Cfg task failed.",
				ModPtr->ModId);
	}


	if (XPfw_RecoveryInit() == XST_SUCCESS) {
		/* This is to enable FPD WDT and enable recovery mechanism when
		* ENABLE_RECOVERY flag is defined.
		*/
	}

	/* Enable the interrupts at XMPU/XPPU block level */
	XPfw_XpuIntrInit();

	XPfw_Printf(DEBUG_DETAILED,"EM Module (MOD-%d): Initialized.\r\n",
			ModPtr->ModId);
}

/**
 * Events handler receives error events from core and routes them to
 * Error Manager for processing
 */
static void EmEventHandler(const XPfw_Module_t *ModPtr, u32 EventId)
{
	switch (EventId) {
	case XPFW_EV_ERROR_1:
		if (XPfw_EmProcessError(EM_ERR_TYPE_1) != XST_SUCCESS) {
			XPfw_Printf(DEBUG_DETAILED,
					"Warning: EmEventHandler: Failed to process error type:"
							" %d\r\n", EM_ERR_TYPE_1)
		}
		break;
	case XPFW_EV_ERROR_2:
		if (XPfw_EmProcessError(EM_ERR_TYPE_2) != XST_SUCCESS) {
			XPfw_Printf(DEBUG_DETAILED,
					"Warning: EmEventHandler: Failed to process error type:"
							" %d\r\n", EM_ERR_TYPE_2)
		}
		break;
	default:
		XPfw_Printf(DEBUG_ERROR,"EM:Unhandled Event(ID:%lu)\r\n", EventId);
		break;
	}
}

/*
 * Create a Mod and assign the Handlers. We will call this function
 * from XPfw_UserStartup()
 */
void ModEmInit(void)
{
	EmModPtr = XPfw_CoreCreateMod();

	(void) XPfw_CoreSetCfgHandler(EmModPtr, EmCfgInit);
	(void) XPfw_CoreSetEventHandler(EmModPtr, EmEventHandler);
	(void)XPfw_CoreSetIpiHandler(EmModPtr, EmIpiHandler, (u16)EM_IPI_HANDLER_ID);
}

#else /* ENABLE_EM */
void ModEmInit(void) { }
void RpuLsHandler(u8 ErrorId) { }
void FpdSwdtHandler(u8 ErrorId) { }
void NullHandler(u8 ErrorId) { }
#endif /* ENABLE_EM */
