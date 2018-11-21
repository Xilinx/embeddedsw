/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
	u32 Buf[XPFW_IPI_MAX_MSG_LEN] = {0U};

	if (IpiNum > 0U) {
		XPfw_Printf(DEBUG_ERROR,"EM: EM handles only IPI on PMU-0\r\n");
	} else {
		switch (Payload[EM_MOD_API_ID_OFFSET] & EM_API_ID_MASK) {
		case SET_EM_ACTION:
			ErrorId = (u8)Payload[EM_ERROR_ID_OFFSET];

			if ((SrcMask & ErrorTable[ErrorId].ChngPerm) == SrcMask) {
				RetVal = (u32)XPfw_EmSetAction(ErrorId, (u8)Payload[EM_ERROR_ACTION_OFFSET],
						ErrorTable[ErrorId].Handler);
			} else {
				RetVal = PERMISSION_DENIED;
			}
			if (RetVal != (u32)XST_SUCCESS) {
				XPfw_Printf(DEBUG_DETAILED, "Warning: EmIpiHandler: Failed "
						"to set action. Please check permissions \r\n");
			}
			Buf[0] = RetVal;
			if (XST_SUCCESS !=
					XPfw_IpiWriteResponse(ModPtr, SrcMask, &Buf[0], 1U)) {
				XPfw_Printf(DEBUG_DETAILED,"EM: IPI write resp failed\r\n");
			}
			break;

		case REMOVE_EM_ACTION:
			ErrorId = (u8)Payload[EM_ERROR_ID_OFFSET];

			if ((SrcMask & ErrorTable[ErrorId].ChngPerm) == SrcMask) {
				RetVal = (u32)XPfw_EmDisable(ErrorId);
			} else {
				RetVal = PERMISSION_DENIED;
			}

			if (RetVal != (u32)XST_SUCCESS) {
				XPfw_Printf(DEBUG_DETAILED,"Warning: EmIpiHandler: Failed"
						" to remove action. Please check permissions\r\n");
			}
			Buf[0] = RetVal;
			if (XST_SUCCESS !=
					XPfw_IpiWriteResponse(ModPtr, SrcMask, &Buf[0], 1U)) {
				XPfw_Printf(DEBUG_DETAILED,"EM: IPI write resp failed\r\n");
			}
			break;

		case SEND_ERRORS_OCCURRED:
			ErrorLog[PMU_BRAM_CE_LOG_OFFSET] = XPfw_Read32(PMU_LMB_BRAM_CE_CNT_REG);
			for(ErrorId = 0U; ErrorId < EM_ERROR_LOG_MAX; ErrorId++) {
				Buf[ErrorId] = ErrorLog[ErrorId];
			}
			if (XST_SUCCESS !=
					XPfw_IpiWriteResponse(ModPtr, SrcMask, &Buf[0],
							EM_ERROR_LOG_MAX)) {
				XPfw_Printf(DEBUG_DETAILED,"EM: IPI write resp failed\r\n");
			}
			break;

		default:
			XPfw_Printf(DEBUG_ERROR,"EM: Unsupported API ID received\r\n");
			Buf[0] = XST_FAILURE;
			if (XST_SUCCESS !=
					XPfw_IpiWriteResponse(ModPtr, SrcMask, &Buf[0], 1U)) {
				XPfw_Printf(DEBUG_DETAILED,"EM: IPI write resp failed\r\n");
			}
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
	if (XST_SUCCESS != XPfw_ResetRpu()) {
		XPfw_Printf(DEBUG_ERROR,"EM: Error in Reset RPU\r\n");
	}
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
void SwdtHandler(u8 ErrorId)
{
	XPfw_Printf(DEBUG_ERROR,"EM: Watchdog Timer Error (Error ID: %d)\r\n",
			ErrorId);
	XPfw_RecoveryHandler(ErrorId);
}

/* CfgInit Handler */
static void EmCfgInit(const XPfw_Module_t *ModPtr, const u32 *CfgData,
		u32 Len)
{
	u32 ErrId = 0U;

	/* Register for Error events from Core */
	if (XST_SUCCESS != XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ERROR_1)) {
		XPfw_Printf(DEBUG_DETAILED,"EM: Failed to register event ID: %d\r\n",
				XPFW_EV_ERROR_1);
	}
	if (XST_SUCCESS != XPfw_CoreRegisterEvent(ModPtr, XPFW_EV_ERROR_2)) {
		XPfw_Printf(DEBUG_DETAILED,"EM: Failed to register event ID: %d\r\n",
				XPFW_EV_ERROR_2);
	}

	/* Init the Error Manager */
	XPfw_EmInit();

	/* Set handlers for error manager */
	for (ErrId = 1U; ErrId < EM_ERR_ID_MAX; ErrId++)
	{
		if (ErrorTable[ErrId].Action != EM_ACTION_NONE) {
			if (XST_SUCCESS != XPfw_EmSetAction((u8)ErrId,
					ErrorTable[ErrId].Action, ErrorTable[ErrId].Handler)) {
				XPfw_Printf(DEBUG_DETAILED,"EM: Set action failed\r\n");
			}
		}
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

	if (XST_SUCCESS != XPfw_CoreSetCfgHandler(EmModPtr, EmCfgInit)) {
		XPfw_Printf(DEBUG_DETAILED,"EM: Set Cfg handler failed\r\n");
	} else if (XST_SUCCESS !=
			XPfw_CoreSetEventHandler(EmModPtr, EmEventHandler)) {
		XPfw_Printf(DEBUG_DETAILED,"EM: Set Event handler failed\r\n");
	} else if (XST_SUCCESS !=
			XPfw_CoreSetIpiHandler(EmModPtr, EmIpiHandler,
					(u16)EM_IPI_HANDLER_ID)) {
		XPfw_Printf(DEBUG_DETAILED,"EM: Set IPI handler failed\r\n");
	}
}

#else /* ENABLE_EM */
void ModEmInit(void) { }
void RpuLsHandler(u8 ErrorId) { }
void SwdtHandler(u8 ErrorId) { }
void NullHandler(u8 ErrorId) { }
#endif /* ENABLE_EM */
