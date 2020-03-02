/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaietile_error.c
* @{
*
* This file contains routines for error handling.
*
* There are three types of error events:
* * Errors without notification, by default when this error happens, no
*   notification is raised, user can register for
*   events hadler for this type of errors.
* * Errors for logging only, by default, when this error happens driver will
*   be notified by the interrupt, the driver will log this error and clear it.
* * Errors will notify users, by default, when this error happens, driver will
*   call the registered error handler, based on the return value from the error
*   handler, driver will kill the application or not. If there is no registered
*   application error handler, driver will kill the application.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   01/02/2020  Initial creation
* 1.1   Dishita 03/29/2020  Add support for clock gating
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#ifdef __linux__
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaietile_error.h"
#include "xaietile_event.h"
#include "xaietile_noc.h"
#include "xaietile_pl.h"

/***************************** Macro Definitions *****************************/
#define XAIETILE_ERROR_BROADCAST	0U /**< Broadcast signal to broadcast
					        errors */
#define XAIETILE_ERROR_SHIM_INTEVENT	0x10U /**< SHIM Internal event for errors */
#define XAIETILE_1ST_IRQ_IDS_MASK	0x3FU /**< 1st level interrupt output IRQ ids mask
						   Event 1st level interrupt controller
						   has its own irq IDs to the 2nd level
						   interrupt controller.
						   For column 0 to 43, the pattern is:
						   0 1 2 3 4 5 0 1, 0 1 2 3 4 5 0 1
						   for column 44 to 49, the pattern is:
						   0 1 2 3 4 5 0 1 2 3 4 5
						*/

#define XAIETILE_SHIM_INTERNAL
#define XAIETILE_CORE_ERROR_START	XAIETILE_EVENT_CORE_SRS_SATURATE
#define XAIETILE_CORE_ERROR_BCGROUP_MASK (\
	(1U << (XAIETILE_EVENT_CORE_TLAST_IN_WSS_WORDS_0_2 - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_PM_REG_ACCESS_FAILURE - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_STREAM_PKT_PARITY_ERROR - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_CONTROL_PKT_ERROR - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_AXI_MM_SLAVE_ERROR - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_INSTRUCTION_DECOMPRESSION_ERROR - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_DM_ADDRESS_OUT_OF_RANGE - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_PM_ECC_ERROR_2BIT - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_PM_ADDRESS_OUT_OF_RANGE - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_DM_ACCESS_TO_UNAVAILABLE - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_LOCK_ACCESS_TO_UNAVAILABLE - XAIETILE_CORE_ERROR_START)))

#define XAIETILE_CORE_ERROR_NO_BCGROUP_MASK (\
	(1U << (XAIETILE_EVENT_CORE_SRS_SATURATE - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_UPS_SATURATE - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_FP_OVERFLOW - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_FP_UNDERFLOW - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_FP_INVALID - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_FP_DIV_BY_ZERO - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_INSTR_WARNING - XAIETILE_CORE_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_CORE_INSTR_ERROR - XAIETILE_CORE_ERROR_START)))

#define XAIETILE_MEM_ERROR_START	XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED
#define XAIETILE_MEM_ERROR_BCGROUP_MASK (\
	(1U << (XAIETILE_EVENT_MEM_DM_ECC_ERROR_2BIT - XAIETILE_MEM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_2 - XAIETILE_MEM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_3 - XAIETILE_MEM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_4 - XAIETILE_MEM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_5 - XAIETILE_MEM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_6 - XAIETILE_MEM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_7 - XAIETILE_MEM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_MEM_DMA_S2MM_0_ERROR - XAIETILE_MEM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_MEM_DMA_S2MM_1_ERROR - XAIETILE_MEM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_MEM_DMA_MM2S_0_ERROR - XAIETILE_MEM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_MEM_DMA_MM2S_1_ERROR - XAIETILE_MEM_ERROR_START)))

#define XAIETILE_SHIM_ERROR_START	XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR
#define XAIETILE_SHIM_ERROR_BCGROUP_MASK (\
	(1U << (XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR - XAIETILE_SHIM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_SHIM_CONTROL_PKT_ERROR - XAIETILE_SHIM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_SHIM_AXI_MM_DECODE_NSU_ERROR_NOC - XAIETILE_SHIM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_NSU_ERROR_NOC - XAIETILE_SHIM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_SHIM_AXI_MM_UNSUPPORTED_TRAFFIC_NOC - XAIETILE_SHIM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_SHIM_AXI_MM_UNSECURE_ACCESS_IN_SECURE_MODE_NOC - XAIETILE_SHIM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_SHIM_AXI_MM_BYTE_STROBE_ERROR_NOC - XAIETILE_SHIM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_SHIM_DMA_S2MM_0_ERROR_NOC - XAIETILE_SHIM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_SHIM_DMA_S2MM_1_ERROR_NOC - XAIETILE_SHIM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_SHIM_DMA_MM2S_0_ERROR_NOC - XAIETILE_SHIM_ERROR_START)) | \
	(1U << (XAIETILE_EVENT_SHIM_DMA_MM2S_1_ERROR_NOC - XAIETILE_SHIM_ERROR_START)))

/************************** Variable Definitions *****************************/
const char *XAie_CoreErrorStr[] = {
	"Core SRS Saturate",
	"Core UPS Saturate",
	"Core FP Overflow",
	"Core FP Underflow",
	"Core FP Invalid",
	"Core FP DIV by zero",
	"Core TLAST in WSS words 0-2",
	"Core PM Reg access failure",
	"Core Stream PKT parity error",
	"Core Control PKT error",
	"Core AXI MM slave error",
	"Core Instruction decompression error",
	"Core DM address out of range",
	"Core PM ECC error scrub corrected",
	"Core PM ECC error scrub 2bit",
	"Core PM ECC error 1bit",
	"Core PM ECC error 2bit",
	"Core PM address out of range",
	"Core DM access to unavailable",
	"Core LOCK access to unavailable",
	"Core INSTR warning",
	"Core INSTR error",
};

const char *XAie_MemErrorStr[] = {
	"DM ECC error scrub corrected",
	"DM ECC error scrub 2bit",
	"DM ECC error 1bit",
	"DM ECC error 2bit",
	"DM parity error bank 2",
	"DM parity error bank 3",
	"DM parity error bank 4",
	"DM parity error bank 5",
	"DM parity error bank 6",
	"DM parity error bank 7",
	"DM DMA S2MM 0 error",
	"DM DMA S2MM 1 error",
	"DM DMA MM2S 0 error",
	"DM DMA MM2S 1 error",
};

const char *XAie_ShimErrorStr[] = {
	"Shim AXI MM slave tile error",
	"Shim Control PKT error",
	"Shim AXI MM decode NSU error NOC",
	"Shim AXI MM slave NSU error NOC",
	"Shim AXI MM unsupported traffic NOC",
	"Shim AXI MM unsecure access in secure mode NOC",
	"Shim AXI MM Byte strobe error NOC",
	"Shim DMA S2MM 0 error NOC",
	"Shim DMA S2MM 1 error NOC",
	"Shim DMA MM2S 0 error NOC",
	"Shim DMA MM2S 1 error NOC",
};

/************************** Internal Function Definitions ********************/
/*****************************************************************************/
/**
 * This API checks if the error needs logging by default.
 *
 * @param	Module - Module type
 *			XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_PL,
 *			XAIEGBL_MODULE_MEM
 * @param	Error - Error event id
 *			48 - 69 for Core module
 *			87 - 100 for Memory module
 *			62 - 72 for SHIM PL module
 *
 * @return	1 for logging required by default, 0 for no action required
 *		by default.
 * @note	As the function is called internally, it will not validate
 *		the module nor the errors assuming the caller will do the
 *		validation.
 *
 *		Used only within this file.
 *****************************************************************************/
static u32 XAieTile_ErrorsDefaultLog(u8 Module, u8 Error)
{
	if (Module == XAIEGBL_MODULE_CORE) {
		switch (Error) {
		case XAIETILE_EVENT_CORE_SRS_SATURATE:
		case XAIETILE_EVENT_CORE_UPS_SATURATE:
		case XAIETILE_EVENT_CORE_FP_OVERFLOW:
		case XAIETILE_EVENT_CORE_FP_UNDERFLOW:
		case XAIETILE_EVENT_CORE_FP_INVALID:
		case XAIETILE_EVENT_CORE_FP_DIV_BY_ZERO:
		case XAIETILE_EVENT_CORE_PM_ECC_ERROR_SCRUB_CORRECTED:
		case XAIETILE_EVENT_CORE_PM_ECC_ERROR_SCRUB_2BIT:
		case XAIETILE_EVENT_CORE_PM_ECC_ERROR_1BIT:
		case XAIETILE_EVENT_CORE_INSTR_WARNING:
		case XAIETILE_EVENT_CORE_INSTR_ERROR:
			return 0;
		default:
			return 1;
		}
	} else if (Module == XAIEGBL_MODULE_MEM) {
		switch (Error) {
		case XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED:
		case XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_2BIT:
		case XAIETILE_EVENT_MEM_DM_ECC_ERROR_1BIT:
			return 0;
		default:
			return 1;
		}
	} else {
		return 1;
	}
}

/*****************************************************************************/
/**
 * This API checks if the error needs to trap application by default.
 *
 * @param	Module - Module type
 *			XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_PL,
 *			XAIEGBL_MODULE_MEM
 * @param	Error - Error event id
 *			48 - 69 for Core module
 *			87 - 100 for Memory module
 *			62 - 72 for SHIM PL module
 *
 * @return	1 to trap application by default, 0 not to trap application
 *		by default.
 * @note	As the function is called internally, it will not validate
 *		the module nor the errors assuming the caller will do the
 *		validation.
 *
 *		Used only within this file.
 *****************************************************************************/
static u32 XAieTile_ErrorsDefaultTrap(u8 Module, u8 Error)
{
	if (XAieTile_ErrorsDefaultLog(Module, Error) == 0) {
		return 0;
	} else if (Module == XAIEGBL_MODULE_MEM) {
		switch (Error) {
		case XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_2:
		case XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_3:
		case XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_4:
		case XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_5:
		case XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_6:
		case XAIETILE_EVENT_MEM_DM_PARITY_ERROR_BANK_7:
			return 0;
		default:
			return 1;
		}
	} else {
		return 1;
	}
}

/*****************************************************************************/
/**
 * This API traps application due to error
 *
 * @param	Handler - Error handler
 *
 * @return	None.
 *
 * @note	On Linux, by default, AIE driver will signal SIGKILL to the
 *		thread which has registered to capture the signal. By default,
 *		the thread is the one which initializes the AIE instance.
 *		On baremetal, by default, AIE driver will exit.
 *
 *		Used only within this file.
 *****************************************************************************/
static void XAieTile_ErrorTrap(XAieGbl_ErrorHandler *Handler)
{
#ifdef __AIESIM__
	(void)Handler;
	exit(-1);
#elif defined __AIEBAREMTL__
	(void)Handler;
	exit(-1);
#else
	kill(Handler->Pid, SIGKILL);
#endif
}

/*****************************************************************************/
/**
 * This API checks if it needs to handle errors.
 *
 * @param	AieInst - pointer to AIE instance
 * @param	Loc - location of the tile
 * @param	Module - Module type
 *			XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_PL,
 *			XAIEGBL_MODULE_MEM
 * @param	Error - Error event id
 * @param	Arg - Argument for event handler
 *
 * @return	None.
 *
 * @note	There are three types of error events:
 *              * Errors without notification, by default when this error
 *                happens, no notification is raised, user can register for
 *                events hadler for this type of errors.
 *              * Errors for logging only, by default, when this error happens
 *                driver will be notified by the interrupt, the driver will
 *                log this error and clear it.
 *              * Errors will notify users, by default, when this error happens,
 *                driver will call the registered error handler, based on
 *                the return value from the error handler, driver will kill the
 *                application or not. If there is no registered application
 *                error handler, driver will kill the application.
 *
 *		Used only within this file.
 *****************************************************************************/
static void XAieTile_ErrorHandler(XAieGbl *AieInst, XAie_LocType Loc, u8 Module,
			     u8 Error, void *Arg)
{
	u8 ErrOff;
	XAieGbl_ErrorHandler *Handler;
	u32 *ErrsDefaultTrap;
	const char **ErrsStrs;

	(void)Arg;
	XAie_AssertVoid(AieInst != NULL);
	if (Module == XAIEGBL_MODULE_CORE) {
		ErrOff = Error - XAIETILE_EVENT_CORE_SRS_SATURATE;
		Handler = &AieInst->CoreErrHandlers[ErrOff];
		ErrsDefaultTrap = &(AieInst->CoreErrsDefaultTrap);
		ErrsStrs = XAie_CoreErrorStr;
	} else if (Module == XAIEGBL_MODULE_PL) {
		ErrOff = Error - XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR;
		Handler = &AieInst->ShimErrHandlers[ErrOff];
		ErrsDefaultTrap = &(AieInst->ShimErrsDefaultTrap);
		ErrsStrs = XAie_ShimErrorStr;
	} else {
		ErrOff = Error - XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED;
		Handler = &AieInst->MemErrHandlers[ErrOff];
		ErrsDefaultTrap = &(AieInst->MemErrsDefaultTrap);
		ErrsStrs = XAie_MemErrorStr;
	}
	XAieLib_log(XAIELIB_LOGERROR,
		    "Dev Error: (%d,%d),%s\n",
		    Loc.Col, Loc.Row, ErrsStrs[ErrOff]);
	if (Handler->Cb != NULL) {
		XAieGbl_ErrorHandleStatus Ret;

		Ret = Handler->Cb(AieInst, Loc, Module, Error, Handler->Arg);
		if (Ret == XAIETILE_ERROR_NOTHANDLED) {
			XAieTile_ErrorTrap(Handler);
		}
	} else {
		u32 Val, NumRows;
		XAieGbl_Tile *TilePtr;

		TilePtr = AieInst->Tiles;
		NumRows = AieInst->Config->NumRows;
		TilePtr += Loc.Col * (NumRows + 1) + Loc.Row;
		/* Check if it will need to trap application by default */
		if ((*ErrsDefaultTrap & (1 << ErrOff)) != 0) {
			/* Disable interrupt for this error and clean it*/
			if (Module == XAIEGBL_MODULE_CORE) {
				Val = XAieTile_CoreGroupEventGet(TilePtr, XAIETILE_GROUP_EVENT_CORE_ERROR0);
				Val &= ~(1 << ErrOff);
				XAieTile_CoreGroupEventSet(TilePtr, XAIETILE_GROUP_EVENT_CORE_ERROR0, Val);
			} else if (Module == XAIEGBL_MODULE_PL) {
				Val = XAieTile_PlGroupEventGet(TilePtr, XAIETILE_GROUP_EVENT_PL_ERRORS);
				Val &= ~(1 << ErrOff);
				XAieTile_PlGroupEventSet(TilePtr, XAIETILE_GROUP_EVENT_PL_ERRORS, Val);
			} else {
				Val = XAieTile_PlGroupEventGet(TilePtr, XAIETILE_GROUP_EVENT_MEM_ERROR);
				Val &= ~(1 << ErrOff);
				XAieTile_PlGroupEventSet(TilePtr, XAIETILE_GROUP_EVENT_MEM_ERROR, Val);
			}
			XAieTile_ErrorTrap(Handler);
		}
	}
}

/*****************************************************************************/
/**
 * This API sets user callback to the specified error.
 *
 * @param	AieInst - Pointer to the AIE device instance
 * @param	Module - Module type
 *			XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_PL,
 *			XAIEGBL_MODULE_MEM
 * @param	Error - Error event id
 *			48 - 69 for Core module
 *			87 - 100 for Memory module
 *			62 - 72 for SHIM PL module
 * @param	Cb - User error callback function pointer
 * @param	Arg - Argument which will be passed to callback function,
 *		      it can be NULL.
 *
 * @return	XAIE_SUCCESS if successful, else XAIE_FAILURE
 *
 * @note	Used only within this file.
 *****************************************************************************/
static int _XAieTile_ErrorRegisterNotification(XAieGbl *AieInst, u8 Module,
					       u8 Error,
					       XAieTile_ErrorCallBack Cb,
					       void *Arg)
{
	XAieGbl_ErrorHandler *Handler;
	XAieGbl_EventHandler *EvtHandler;
	u32 *ErrsDefaultTrap, *ErrsPollOnly;
	u8 ErrOff, ErrsStart, ErrsEnd;

	/* Setup the handler */
	XAie_AssertNonvoid(AieInst != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Config != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);
	XAie_AssertNonvoid(Module == XAIEGBL_MODULE_CORE ||
			   Module == XAIEGBL_MODULE_MEM ||
			   Module == XAIEGBL_MODULE_PL);

	if (Module == XAIEGBL_MODULE_CORE) {
		ErrsDefaultTrap = &(AieInst->CoreErrsDefaultTrap);
		ErrsPollOnly = &(AieInst->CoreErrsPollOnly);
		ErrsStart = XAIETILE_EVENT_CORE_SRS_SATURATE;
		ErrsEnd = XAIETILE_EVENT_CORE_INSTR_ERROR;
		EvtHandler = AieInst->CoreEvtHandlers;
		Handler = AieInst->CoreErrHandlers;
	} else if (Module == XAIEGBL_MODULE_MEM) {
		ErrsDefaultTrap = &(AieInst->MemErrsDefaultTrap);
		ErrsPollOnly = &(AieInst->MemErrsPollOnly);
		ErrsStart = XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED;
		ErrsEnd = XAIETILE_EVENT_MEM_DMA_MM2S_1_ERROR;
		Handler = AieInst->MemErrHandlers;
		EvtHandler = AieInst->MemEvtHandlers;
	} else {
		ErrsDefaultTrap = &(AieInst->ShimErrsDefaultTrap);
		ErrsPollOnly = &(AieInst->ShimErrsPollOnly);
		ErrsStart = XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR;
		ErrsEnd = XAIETILE_EVENT_SHIM_DMA_MM2S_1_ERROR_NOC;
		Handler = AieInst->ShimErrHandlers;
		EvtHandler = AieInst->ShimEvtHandlers;
	}
	XAie_AssertNonvoid((Error >= ErrsStart) && (Error <= ErrsEnd));
	ErrOff = Error - ErrsStart;
	if (Handler[ErrOff].Cb != XAIE_NULL) {
		if ((Cb != XAIE_NULL && Cb != Handler[ErrOff].Cb) ||
		    (Arg != XAIE_NULL && Arg != Handler[ErrOff].Arg)) {
			XAieLib_IntPrint("%s: failed,"
					 "event(%u) has been registered with"
					 "different callback."
					 "Please unregister first.\n",
					 __func__, Error);
			return XAIE_FAILURE;
		}
	}
	Handler[ErrOff].Cb = Cb;
	Handler[ErrOff].Arg = Arg;
	*ErrsDefaultTrap |= 1 << ErrOff;
	if (EvtHandler[Error].Cb == XAIE_NULL) {
		EvtHandler[Error].Cb = XAieTile_ErrorHandler;
		EvtHandler[Error].Arg = AieInst;
	}
	/* Enable notification if the error was polled only before */
	if ((*ErrsPollOnly & (1 << ErrOff)) != 0) {
		*ErrsPollOnly &= ~(1 << ErrOff);
		u32 NumCols, NumRows, ClockCntrlRegVal;
		XAieGbl_Tile *TilePtr;
		u64 RegAddr;

		NumCols = AieInst->Config->NumCols;
		NumRows = AieInst->Config->NumRows;
		/* Enable notification for the error */
		for(u32 col = 0; col < NumCols; col++){
			TilePtr = AieInst->Tiles;
			TilePtr += col * (NumRows + 1);
			if(Module == XAIEGBL_MODULE_PL){
			   XAieTile_PlGroupEventSet(TilePtr,
			   XAIETILE_GROUP_EVENT_PL_ERRORS,(~(*ErrsPollOnly)));
			}
			RegAddr = TilePtr->TileAddr;
			ClockCntrlRegVal = XAieGbl_Read32(RegAddr +
							XAIEGBL_PL_TILCLOCTRL);

			/* check if any tile in this col is used */
			if(ClockCntrlRegVal & XAIEGBL_PL_TILCLOCTRLMSK){
				for(u32 row = 1; row <= NumRows; row++){
					TilePtr = AieInst->Tiles;
					TilePtr += col * (NumRows + 1) + row;

					if (Module == XAIEGBL_MODULE_CORE) {
					    XAieTile_CoreGroupEventSet(TilePtr,
					    XAIETILE_GROUP_EVENT_CORE_ERROR0,
							   (~(*ErrsPollOnly)));
					}
					else if (Module == XAIEGBL_MODULE_MEM) {
					        XAieTile_MemGroupEventSet(TilePtr,
						XAIETILE_GROUP_EVENT_MEM_ERROR,
							   (~(*ErrsPollOnly)));
					}
					RegAddr = TilePtr->TileAddr;
					ClockCntrlRegVal = XAieGbl_Read32(
					     RegAddr + XAIEGBL_CORE_TILCLOCTRL);
					/* check if the tile above is gated */
					if(!(ClockCntrlRegVal &
					     XAIEGBL_CORE_TILCLOCTRL_NEXTILCLOENA_MASK))
						break;
				}
			}
		}
	}
	return XAIE_SUCCESS;
}

/************************** Function Definitions ********************/
/*****************************************************************************/
/**
 * This API sets user callback to the specified error.
 *
 * @param	AieInst - Pointer to the AIE device instance
 * @param	Module - Module type
 *			XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_PL,
 *			XAIEGBL_MODULE_MEM, XAIEGBL_MODULE_ALL
 *			or any OR value of CORE, PL and MEM.
 * @param	Error - Error event id
 *			0 for any errors which will generate interrupt.
 *			48 - 69 for Core module
 *			87 - 100 for Memory module
 *			62 - 72 for SHIM PL module
 * @param	Cb - User error callback function pointer
 *		     If callback is NULL, when this error happens, it
 *		     will kill the application.
 * @param	Arg - Argument which will be passed to callback function,
 *		      it can be NULL.
 *
 * @return	XAIE_SUCCESS if successful, else XAIE_FAILURE
 *
 * @note	None.
 *****************************************************************************/
int XAieTile_ErrorRegisterNotification(XAieGbl *AieInst, u8 Module, u8 Error,
				       XAieTile_ErrorCallBack Cb, void *Arg)
{
	u8 TmpModule, NumModules;

	XAie_AssertNonvoid(AieInst != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Config != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);

	/* Disable interrupt before setting the error handler */
	/* Setup the handler */

	/* If it is more that one module type is specified, the Error
	 * needs to be ALL. */
	TmpModule = Module;
	NumModules = 0;
	for (u8 m = 0; m < 3; m++) {
		if ((TmpModule & 0x1U) != 0) {
			NumModules++;
		}
		if ((NumModules > 1) && (Error != XAIETILE_ERROR_ALL)) {
			XAieLib_IntPrint("%s: failed: Multiple module types are specified,"
					 "but single error event is specified."
					 "Please specify ALL erros for multiple module types.\n",
					 __func__);
			return XAIE_FAILURE;
		}
		TmpModule >>= 1;
	}
	if ((Module == 0) || (Module > XAIEGBL_MODULE_ALL)) {
		XAieLib_IntPrint("%s: failed: Invalid Module type %u.\n",
				 __func__, Module);
		return XAIE_FAILURE;
	}
	for (u8 m = 0; m < 3; m++) {
		u8 MMask;
		u32 *ErrsPollOnly;
		u8 ErrsStart, ErrsEnd;

		MMask = 1 << m;
		if ((Module & MMask) == 0) {
			continue;
		}
		ErrsPollOnly = XAIE_NULL;
		ErrsStart = XAIETILE_ERROR_ALL;
		ErrsEnd = XAIETILE_ERROR_ALL;
		if (MMask == XAIEGBL_MODULE_CORE) {
			ErrsStart = XAIETILE_EVENT_CORE_SRS_SATURATE;
			ErrsEnd = XAIETILE_EVENT_CORE_INSTR_ERROR;
			if (Error == XAIETILE_ERROR_ALL) {
				ErrsPollOnly = &(AieInst->CoreErrsPollOnly);
			}
		} else if (MMask == XAIEGBL_MODULE_MEM) {
			ErrsStart = XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED;
			ErrsEnd = XAIETILE_EVENT_MEM_DMA_MM2S_1_ERROR;
			if (Error == XAIETILE_ERROR_ALL) {
				ErrsPollOnly = &(AieInst->MemErrsPollOnly);
			}
		} else if (MMask == XAIEGBL_MODULE_PL) {
			ErrsStart = XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR;
			ErrsEnd = XAIETILE_EVENT_SHIM_DMA_MM2S_1_ERROR_NOC;
			if (Error == XAIETILE_ERROR_ALL) {
				ErrsPollOnly = &(AieInst->ShimErrsPollOnly);
			}
		}
		XAie_AssertNonvoid(ErrsStart != XAIETILE_ERROR_ALL);
		XAie_AssertNonvoid(ErrsEnd != XAIETILE_ERROR_ALL);
		if (Error != XAIETILE_ERROR_ALL) {
			if (Error < ErrsStart || Error > ErrsEnd) {
				XAieLib_IntPrint("%s: failed, Invalid Error %u for Module %u.\n",
						 __func__, Error, Module);
				return XAIE_FAILURE;
			}
			return _XAieTile_ErrorRegisterNotification(AieInst,
								   MMask,
								   Error, Cb, Arg);
		} else {
			XAie_AssertNonvoid(ErrsPollOnly != XAIE_NULL);
			for (u8 e = ErrsStart; e <= ErrsEnd; e++) {
				int ret;

				if ((*ErrsPollOnly & (1 << (e - ErrsStart))) != 0) {
					continue;
				}
				ret = _XAieTile_ErrorRegisterNotification(AieInst,
							  MMask, e, Cb, Arg);
				if (ret != XAIE_SUCCESS) {
					return ret;
				}
			}
		}
	}
	return XAIE_SUCCESS;
}

/*****************************************************************************/
/**
 * This API sets user callback to the specified error.
 *
 * @param	AieInst - Pointer to the AIE device instance
 * @param	Module - Module type
 *			XAIEGBL_MODULE_CORE, XAIEGBL_MODULE_PL,
 *			XAIEGBL_MODULE_MEM
 * @param	Error - Error event id
 *			48 - 69 for Core module
 *			87 - 100 for Memory module
 *			62 - 72 for SHIM PL module
 * @param	Logging - Indicate if it needs to log. If it doesn't need
 *			  to log, when the error happens, there will be
 *			  no interrupt raised.
 *			  XAIE_ENABLE - need to log,
 *			  XAIE_DISABLE - no need to log
 *
 * @return	None.
 *
 * @note	None.
 *****************************************************************************/
void XAieTile_ErrorUnregisterNotification(XAieGbl *AieInst, u8 Module,
					  u8 Error, u8 Logging)
{
	XAieGbl_ErrorHandler *Handler;
	u32 *ErrsDefaultTrap, *ErrsPollOnly, ClockCntrlRegVal;
	u8 ErrOff, ErrorGroupUpdate;
	u64 RegAddr;

	XAie_AssertVoid(AieInst != XAIE_NULL);
	XAie_AssertVoid(AieInst->Config != XAIE_NULL);
	XAie_AssertVoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertVoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);

	/* Disable interrupt before setting the error handler */
	/* Get the handler */
	if (Module == XAIEGBL_MODULE_CORE) {
		XAie_AssertVoid(Error >= XAIETILE_EVENT_CORE_SRS_SATURATE &&
				Error <= XAIETILE_EVENT_CORE_INSTR_ERROR);
		ErrsDefaultTrap = &(AieInst->CoreErrsDefaultTrap);
		ErrsPollOnly = &(AieInst->CoreErrsPollOnly);
		ErrOff = Error - XAIETILE_EVENT_CORE_SRS_SATURATE;
		Handler = &AieInst->CoreErrHandlers[ErrOff];
	} else if (Module == XAIEGBL_MODULE_MEM) {
		XAie_AssertVoid(Error >= XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED &&
				Error <= XAIETILE_EVENT_MEM_DMA_MM2S_1_ERROR);
		ErrsDefaultTrap = &(AieInst->MemErrsDefaultTrap);
		ErrsPollOnly = &(AieInst->MemErrsPollOnly);
		ErrOff = Error - XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED;
		Handler = &AieInst->MemErrHandlers[ErrOff];
	} else {
		XAie_AssertVoid(Error >= XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR &&
				Error <= XAIETILE_EVENT_SHIM_DMA_MM2S_1_ERROR_NOC);
		ErrsDefaultTrap = &(AieInst->ShimErrsDefaultTrap);
		ErrsPollOnly = &(AieInst->ShimErrsPollOnly);
		ErrOff = Error - XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR;
		Handler = &AieInst->ShimErrHandlers[ErrOff];
	}
	*ErrsDefaultTrap &= ~(1U << ErrOff);
	Handler->Cb = NULL;
	Handler->Arg = NULL;
	ErrorGroupUpdate = 0;
	if (Logging == XAIE_DISABLE) {
		if ((*ErrsPollOnly & (1U << ErrOff)) == 0) {
			*ErrsPollOnly |= 1 << ErrOff;
			ErrorGroupUpdate = 1;
		}
	} else {
		if ((*ErrsPollOnly & (1 << ErrOff)) != 0) {
			*ErrsPollOnly &= ~(1U << ErrOff);
			ErrorGroupUpdate = 1;
		}
	}
	if (ErrorGroupUpdate == 1) {
		u32 NumCols, NumRows;
		XAieGbl_Tile *TilePtr;

		NumCols = AieInst->Config->NumCols;
		NumRows = AieInst->Config->NumRows;

		/* Enable notification for the error */
		for(u32 col = 0; col < NumCols; col++){
			TilePtr = AieInst->Tiles;
			TilePtr += col * (NumRows + 1);

			if (Module == XAIEGBL_MODULE_PL) {
				XAieTile_PlGroupEventSet(TilePtr,
					 XAIETILE_GROUP_EVENT_PL_ERRORS,
					   (~(*ErrsPollOnly)));
			}
			RegAddr = TilePtr->TileAddr;
			ClockCntrlRegVal = XAieGbl_Read32(RegAddr +
							XAIEGBL_PL_TILCLOCTRL);

			/* check if any tile in this col is used */
			if(ClockCntrlRegVal & XAIEGBL_PL_TILCLOCTRLMSK){
				for(u32 row = 1; row <= NumRows; row++){
					TilePtr = AieInst->Tiles;
					TilePtr += col * (NumRows + 1) + row;

					if (Module == XAIEGBL_MODULE_CORE) {
					    XAieTile_CoreGroupEventSet(TilePtr,
					    XAIETILE_GROUP_EVENT_CORE_ERROR0,
							(~(*ErrsPollOnly)));
					}
					else if (Module == XAIEGBL_MODULE_MEM) {
					         XAieTile_MemGroupEventSet(TilePtr,
						 XAIETILE_GROUP_EVENT_MEM_ERROR,
							  (~(*ErrsPollOnly)));
					}
					RegAddr = TilePtr->TileAddr;
					ClockCntrlRegVal = XAieGbl_Read32(
					     RegAddr + XAIEGBL_CORE_TILCLOCTRL);
					/* check if the tile above is gated */
					if(!(ClockCntrlRegVal &
					     XAIEGBL_CORE_TILCLOCTRL_NEXTILCLOENA_MASK))
						break;
				}
			}
		}
	}
}

/*****************************************************************************/
/**
 * This API initialize error default handlers.
 * When errors happens, the AIE driver will either take default action which
 * will trap the application or call the user registered callback.
 *
 * @param	AieInst - Pointer to the AIE device instance
 *
 * @return	None.
 *
 * @note	None.
 *****************************************************************************/
void XAieTile_ErrorsSetupDefaultHandler(XAieGbl *AieInst)
{
	u8 Err;

	XAie_AssertVoid(AieInst != XAIE_NULL);
	XAie_AssertVoid(AieInst->Config != XAIE_NULL);
	XAie_AssertVoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertVoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);

	memset(AieInst->CoreErrHandlers, 0, sizeof(AieInst->CoreErrHandlers));
	memset(AieInst->MemErrHandlers, 0, sizeof(AieInst->MemErrHandlers));
	memset(AieInst->ShimErrHandlers, 0, sizeof(AieInst->ShimErrHandlers));
	/* Clean up the error handlers */
#ifdef __linux__
	for (u32 i = 0; i < XAIEGBL_CORE_ERROR_NUM; i++) {
		AieInst->CoreErrHandlers[i].Pid = (int)getpid();
	}
	for (u32 i = 0; i < XAIEGBL_MEM_ERROR_NUM; i++) {
		AieInst->MemErrHandlers[i].Pid = (int)getpid();
	}
	for (u32 i = 0; i < XAIEGBL_PL_ERROR_NUM; i++) {
		AieInst->ShimErrHandlers[i].Pid = (int)getpid();
	}
#endif
	/* Check if the error needs to trap the app by default */
	AieInst->CoreErrsDefaultTrap = 0;
	for (Err = XAIETILE_EVENT_CORE_SRS_SATURATE;
	     Err <= XAIETILE_EVENT_CORE_INSTR_ERROR; Err++) {
		if (XAieTile_ErrorsDefaultTrap(XAIEGBL_MODULE_CORE, Err) == 1) {
			AieInst->CoreErrsDefaultTrap |=
				1 << (Err - XAIETILE_EVENT_CORE_SRS_SATURATE);
		}

	}
	AieInst->MemErrsDefaultTrap = 0;
	for (Err = XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED;
	     Err <= XAIETILE_EVENT_MEM_DMA_MM2S_1_ERROR; Err++) {
		if (XAieTile_ErrorsDefaultTrap(XAIEGBL_MODULE_MEM, Err) == 1) {
			AieInst->MemErrsDefaultTrap |=
				1 << (Err - XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED);
		}
	}
	AieInst->ShimErrsDefaultTrap = 0;
	for (Err = XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR;
	     Err <= XAIETILE_EVENT_SHIM_DMA_MM2S_1_ERROR_NOC; Err++) {
		if (XAieTile_ErrorsDefaultTrap(XAIEGBL_MODULE_PL, Err) == 1) {
			AieInst->ShimErrsDefaultTrap |=
				1 << (Err - XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR);
		}
	}
	/* Setup error handler for error events */
	AieInst->CoreErrsPollOnly = 0;
	for (Err = XAIETILE_EVENT_CORE_SRS_SATURATE;
	     Err <= XAIETILE_EVENT_CORE_INSTR_ERROR; Err++) {
		if (XAieTile_ErrorsDefaultLog(XAIEGBL_MODULE_CORE, Err) == 1) {
			AieInst->CoreEvtHandlers[Err].Cb = XAieTile_ErrorHandler;
		} else {
			AieInst->CoreErrsPollOnly |=
				1 << (Err - XAIETILE_EVENT_CORE_SRS_SATURATE);
		}
	}
	AieInst->MemErrsPollOnly = 0;
	for (Err = XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED;
	     Err <= XAIETILE_EVENT_MEM_DMA_MM2S_1_ERROR; Err++) {
		if (XAieTile_ErrorsDefaultLog(XAIEGBL_MODULE_MEM, Err) == 1) {
			AieInst->MemEvtHandlers[Err].Cb = XAieTile_ErrorHandler;
		} else {
			AieInst->MemErrsPollOnly |=
				1 << (Err - XAIETILE_EVENT_MEM_DM_ECC_ERROR_SCRUB_CORRECTED);
		}
	}
	AieInst->ShimErrsPollOnly = 0;
	for (Err = XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR;
	     Err <= XAIETILE_EVENT_SHIM_DMA_MM2S_1_ERROR_NOC; Err++) {
		if (XAieTile_ErrorsDefaultLog(XAIEGBL_MODULE_PL, Err) == 1) {
			AieInst->ShimEvtHandlers[Err].Cb = XAieTile_ErrorHandler;
		} else {
			AieInst->ShimErrsPollOnly |=
				1 << (Err - XAIETILE_EVENT_SHIM_AXI_MM_SLAVE_TILE_ERROR);
		}
	}
}

/*****************************************************************************/
/**
 * This API initialize error handling. It will setup the error broadcast network
 * so that if error is raised, interrupt will be raised, and the interrupt
 * will be captured by the AIE driver. When errors happens, the AIE driver will
 * either take default action which will trap the application or call the user
 * registered callback.
 *
 * @param	AieInst - Pointer to the AIE device instance
 *
 * @return	XAIE_SUCCESS if successful, else XAIE_FAILURE
 *
 * @note	None.
 *****************************************************************************/
int XAieTile_ErrorsHandlingInitialize(XAieGbl *AieInst)
{
	u32 NumCols, NumRows;
	XAieGbl_Tile *TilePtr;
#ifndef __AIESIM__
	u32 ClockCntrlRegVal;
	u64 RegAddr;
#endif

	XAie_AssertNonvoid(AieInst != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Config != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->Tiles != XAIE_NULL);
	XAie_AssertNonvoid(AieInst->IsReady == XAIE_COMPONENT_IS_READY);

	/* Setup error broadcast network */
	NumCols = AieInst->Config->NumCols;
	NumRows = AieInst->Config->NumRows;

	for(u32 col = 0; col < NumCols; col++){
		TilePtr = AieInst->Tiles;
		TilePtr += col * (NumRows + 1);
		/* Shim tile only need to setup error notification with
		 * 1st level interrupt controller.
		 */
		XAieTile_PlGroupEventSet(TilePtr,
					 XAIETILE_GROUP_EVENT_PL_ERRORS,
					 XAIETILE_SHIM_ERROR_BCGROUP_MASK);
		XAieTile_PlIntcL1IrqEventSet(TilePtr,
					     XAIETILE_ERROR_SHIM_INTEVENT,
					     XAIETILE_EVENT_SHIM_GROUP_ERRORS_,
					     XAIETILE_PL_BLOCK_SWITCHA);
#ifndef __AIESIM__
		/* check if any tile in this col is used */
		RegAddr = TilePtr->TileAddr;
		ClockCntrlRegVal = XAieGbl_Read32(RegAddr + XAIEGBL_PL_TILCLOCTRL);
		if(ClockCntrlRegVal & XAIEGBL_PL_TILCLOCTRLMSK){
#endif
			for(u32 row = 1; row <= NumRows; row++){
				TilePtr = AieInst->Tiles;
				TilePtr += col * (NumRows + 1) + row;
				/* Compute tile has core and memory module */
				/* Setup error group event to broadcast error */
				XAieTile_CoreGroupEventSet(TilePtr,
					XAIETILE_GROUP_EVENT_CORE_ERROR0,
					XAIETILE_CORE_ERROR_BCGROUP_MASK);
				XAieTile_MemGroupEventSet(TilePtr,
					XAIETILE_GROUP_EVENT_MEM_ERROR,
					XAIETILE_SHIM_ERROR_BCGROUP_MASK);
				XAieTileCore_EventBroadcast(TilePtr,
					XAIETILE_ERROR_BROADCAST,
					XAIETILE_EVENT_CORE_GROUP_ERRORS0);
				XAieTileMem_EventBroadcast(TilePtr,
					XAIETILE_ERROR_BROADCAST,
					XAIETILE_EVENT_MEM_GROUP_ERRORS);

#ifndef __AIESIM__
				RegAddr = TilePtr->TileAddr;
				ClockCntrlRegVal = XAieGbl_Read32(
					RegAddr + XAIEGBL_CORE_TILCLOCTRL);
				/* check if the tile above this tile is gated */
				if(!(ClockCntrlRegVal & XAIEGBL_CORE_TILCLOCTRL_NEXTILCLOENA_MASK))
					break;
#endif
			}
#ifndef __AIESIM__
		}
#endif
	}
	return XAIE_SUCCESS;
}
/** @} */
