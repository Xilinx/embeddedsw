/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_npi.c
* @{
*
* This file contains routines for operations for accessing AI engine NPI.
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"
#include "xaie_npi.h"
#include "xaiegbl.h"

/************************** Constant Definitions *****************************/
/****************************** Type Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XAie_NpiMod _XAieNpiMod;
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the function to get the NPI module based on the device version
*
* @param	DevInst : Device Instance pointer
*
* @return	NPI module pointer for success, NULL for failure.
*
* @note		None.
*
*******************************************************************************/
static XAie_NpiMod *_XAie_NpiGetMod(XAie_DevInst *DevInst)
{
	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("failed to get NPI module, invalid dev instance.\n");
		return NULL;
	}

	if (DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE) {
		return &_XAieNpiMod;
	}

	XAIE_ERROR("failed to get NPI module, invalid dev version.\n");
	return NULL;
}

/*****************************************************************************/
/**
*
* This is function to set NPI lock
*
* @param	DevInst : AI engine device pointer
* @param	Lock : XAIE_ENABLE to lock, XAIE_DISABLE to unlock
*
* @return	None
*
* @note		This function will not check if NPI module is available as
*		it expects the caller function will do the checking.
*******************************************************************************/
static void _XAie_NpiSetLock(XAie_DevInst *DevInst, u8 Lock)
{
	XAie_NpiMod *NpiMod;
	XAie_BackendNpiWrReq Req;
	u32 LockVal;

	NpiMod = _XAie_NpiGetMod(DevInst);

	if (Lock == XAIE_DISABLE) {
		LockVal = NpiMod->PcsrUnlockCode;
	} else {
		LockVal = 0;
	}

	Req = _XAie_SetBackendNpiWrReq(NpiMod->PcsrLockOff, LockVal);
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIWR32, &Req);
}

/*****************************************************************************/
/**
*
* This is function to mask write to PCSR register
*
* @param	DevInst : AI engine device pointer
* @param	RegVal : Value to write to PCSR register
* @param	Mask : Mask to write to PCSR register
*
* @return	None
*
* @note		Sequence to write PCSR control register is as follows:
*		* unlock the PCSR register
*		* enable PCSR mask from mask register
*		* set the value to PCSR control register
*		* disable PCSR mask from mask register
*		* lock the PCSR register
*******************************************************************************/
static void _XAie_NpiWritePcsr(XAie_DevInst *DevInst, u32 RegVal, u32 Mask)
{
	XAie_NpiMod *NpiMod;
	XAie_BackendNpiWrReq Req;

	NpiMod = _XAie_NpiGetMod(DevInst);
	if (NpiMod == NULL) {
		return;
	}

	_XAie_NpiSetLock(DevInst, XAIE_DISABLE);

	Req = _XAie_SetBackendNpiWrReq(NpiMod->PcsrMaskOff, Mask);
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIWR32, &Req);

	Req = _XAie_SetBackendNpiWrReq(NpiMod->PcsrCntrOff, RegVal);
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIWR32, &Req);

	Req = _XAie_SetBackendNpiWrReq(NpiMod->PcsrMaskOff, 0);
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIWR32, &Req);

	_XAie_NpiSetLock(DevInst, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This is the NPI function to set the SHIM set assert
*
* @param	DevInst : AI engine device pointer
* @param	RstEnable : XAIE_ENABLE to assert reset, and XAIE_DISABLE to
*			    deassert reset.
*
* @return	None
*
* @note		None.
*
*******************************************************************************/
void _XAie_NpiSetShimReset(XAie_DevInst *DevInst, u8 RstEnable)
{
	u32 RegVal, Mask;
	XAie_NpiMod *NpiMod;

	NpiMod = _XAie_NpiGetMod(DevInst);
	if (NpiMod == NULL) {
		return;
	}

	Mask = NpiMod->ShimReset.Mask;
	RegVal = XAie_SetField(RstEnable, NpiMod->ShimReset.Lsb, Mask);

	_XAie_NpiWritePcsr(DevInst, RegVal, Mask);
}

/*****************************************************************************/
/**
*
* This is the NPI function to set the AI engine protect register configuration
*
* @param	DevInst : AI engine partition device pointer
* @param	Req : Request to set the protected registers
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		None.
*
*******************************************************************************/
AieRC _XAie_NpiSetProtectedRegEnable(XAie_DevInst *DevInst,
				    XAie_NpiProtRegReq *Req)
{
	u32 RegVal;
	XAie_NpiMod *NpiMod;
	XAie_BackendNpiWrReq IOReq;
	AieRC RC;

	NpiMod = _XAie_NpiGetMod(DevInst);
	if (NpiMod == NULL) {
		return XAIE_INVALID_ARGS;
	}

	RC = NpiMod->SetProtectedRegField(DevInst, Req, &RegVal);
	if (RC != XAIE_OK) {
		return RC;
	}

	IOReq = _XAie_SetBackendNpiWrReq(NpiMod->ProtRegOff, RegVal);

	_XAie_NpiSetLock(DevInst, XAIE_DISABLE);
	RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIWR32, &IOReq);
	_XAie_NpiSetLock(DevInst, XAIE_ENABLE);

	return RC;
}

/*****************************************************************************/
/**
*
* This NPI function enables or disables AI-Engine NPI interrupts to PS GIC.
*
* @param	DevInst : AI engine partition device pointer
* @param	Ops: XAIE_ENABLE or XAIE_DISABLE to enable/disable
*		     interrupt.
* @param	NpiIrqID: NPI IRQ ID.
* @param	AieIrqID: AIE IRQ ID.
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		None.
*
*******************************************************************************/
AieRC _XAie_NpiIrqConfig(XAie_DevInst *DevInst, u8 Ops, u8 NpiIrqID,
				u8 AieIrqID)
{
	u32 RegOff;
	XAie_NpiMod *NpiMod;
	XAie_BackendNpiWrReq IOReq;
	AieRC RC;

	NpiMod = _XAie_NpiGetMod(DevInst);
	if (NpiMod == NULL) {
		return XAIE_INVALID_ARGS;
	}

	/*
	 * For generation of devices which don't support this feature, silently
	 * return XAIE_OK as these registers are inactive.
	 */
	if (NpiMod->NpiIrqNum == XAIE_FEATURE_UNAVAILABLE) {
		return XAIE_OK;
	}

	if (Ops > XAIE_ENABLE) {
		XAIE_ERROR("Invalid NPI IRQ operations\n");
		return XAIE_INVALID_ARGS;
	}

	if (NpiIrqID >= NpiMod->NpiIrqNum || AieIrqID >= NpiMod->AieIrqNum) {
		XAIE_ERROR("Invalid AIE or NPI IRQ ID\n");
		return XAIE_INVALID_ARGS;
	}

	if (Ops == XAIE_ENABLE) {
		RegOff = NpiMod->BaseIrqRegOff +
				(NpiIrqID + 1) * NpiMod->IrqEnableOff;
	} else {
		RegOff = NpiMod->BaseIrqRegOff +
				(NpiIrqID + 1) * NpiMod->IrqDisableOff;
	}

	IOReq = _XAie_SetBackendNpiWrReq(RegOff, 1U << AieIrqID);

	_XAie_NpiSetLock(DevInst, XAIE_DISABLE);
	RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIWR32, &IOReq);
	_XAie_NpiSetLock(DevInst, XAIE_ENABLE);

	return RC;
}

/*****************************************************************************/
/**
*
* This NPI function enables AI-Engine NPI interrupts to PS GIC.
*
* @param	DevInst : AI engine partition device pointer
* @param	NpiIrqID: NPI IRQ ID.
* @param	AieIrqID: AIE IRQ ID.
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		None.
*
*******************************************************************************/
AieRC _XAie_NpiIrqEnable(XAie_DevInst *DevInst, u8 NpiIrqID, u8 AieIrqID)
{
	return _XAie_NpiIrqConfig(DevInst, XAIE_ENABLE, NpiIrqID, AieIrqID);
}

/*****************************************************************************/
/**
*
* This NPI function disables AI-Engine NPI interrupts to PS GIC.
*
* @param	DevInst : AI engine partition device pointer
* @param	NpiIrqID: NPI IRQ ID.
* @param	AieIrqID: AIE IRQ ID.
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		None.
*
*******************************************************************************/
AieRC _XAie_NpiIrqDisable(XAie_DevInst *DevInst, u8 NpiIrqID, u8 AieIrqID)
{
	return _XAie_NpiIrqConfig(DevInst, XAIE_DISABLE, NpiIrqID, AieIrqID);
}

/** @} */
