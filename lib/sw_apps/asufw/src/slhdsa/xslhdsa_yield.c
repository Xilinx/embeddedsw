/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xslhdsa_yield.c
*
* This file contains cooperative yield support for long-running SLH-DSA key generation
* and signing in ASUFW firmware.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   kk   09/09/26 Initial release
*
* </pre>
*
*************************************************************************************************/
/**
* @addtogroup xslhdsa_server_apis SLH-DSA Server APIs
* @{
*/
/*************************************** Include Files *******************************************/
#include "xil_sutil.h"
#include "xslhdsa.h"
#include "xasufw_util.h"
#include "xasufw_slhdsahandler.h"

#ifdef XASU_SLHDSA_ENABLE
/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XSlhDsa_YieldSliceReset(XSlhDsa_Instance *InstPtr);
static u32 *XSlhDsa_YieldUnitCounter(XSlhDsa_Instance *InstPtr, u32 Unit, u32 *BudgetPtr);

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
* @brief	This function starts a cooperative yield session for an IPI request.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	ReqId	Active request identifier.
*
*************************************************************************************************/
void XSlhDsa_YieldSessionBegin(XSlhDsa_Instance *InstPtr, u32 ReqId)
{
	/** Reject a NULL instance. */
	if (InstPtr == NULL) {
		goto END;
	}

	/**
	 * Start a new session when none is active or ReqId differs. Clear KeyGen/Sign stages
	 * and tree resume flags so a later request cannot resume another command's walk.
	 * The same ReqId on an active session is a yield resume: keep the checkpoints.
	 */
	if ((InstPtr->Yield.Session.Active == 0U) || (InstPtr->Yield.Session.ReqId != ReqId)) {
		InstPtr->Yield.Session.Active = 1U;
		InstPtr->Yield.Session.ReqId = ReqId;
		InstPtr->Yield.Session.Slices = 0U;
		InstPtr->Yield.Session.HashDmaResume = 0U;
		InstPtr->Yield.Session.HashWipeFail = 0U;
		InstPtr->Yield.Keygen.Stage = XSLHDSA_KEYGEN_STAGE_NONE;
		InstPtr->Yield.Sign.Stage = XSLHDSA_SIGN_STAGE_NONE;
		InstPtr->Yield.Xmss.Resume = 0U;
		InstPtr->Yield.Fors.TreeResume = 0U;
		InstPtr->Yield.Fors.EmitResume = 0U;
		InstPtr->Yield.Xmss.AuthPathResume = 0U;
		InstPtr->Yield.Xmss.EmitResume = 0U;
		InstPtr->Yield.Xmss.EmitWotsDone = 0U;
		InstPtr->Yield.Ht.Resume = 0U;
		InstPtr->Yield.Ht.EmitPending = 0U;
	}

	/** Start a new cooperative-yield time slice. */
	XSlhDsa_YieldSliceReset(InstPtr);

END:
	return;
}

/*************************************************************************************************/
/**
* @brief	This function ends a cooperative yield session, clears saved checkpoints and
*		zeroizes every secret the session cached across slices.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
*
* @return
*		- XASUFW_SUCCESS, if the session state is cleared and all secrets are zeroized.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr is NULL.
*		- Buffer-clear failure status via XAsufw_UpdateBufStatus, if any zeroize fails or
*		  if a PRF_msg or H_msg wipe failed during a SHA message DMA wait.
*
* @note
*	Every handler exit that is not XASUFW_CMD_IN_PROGRESS calls this function, including the
*	livelock-cap abort that never enters the driver. Zeroizing here rather than in the driver
*	entry points is what keeps key material from outliving an aborted command.
*
*************************************************************************************************/
s32 XSlhDsa_YieldSessionEnd(XSlhDsa_Instance *InstPtr)
{
	s32 Status = XASUFW_SUCCESS;
	u8 KeygenStage;
	u8 HashWipeFail;

	/** Reject a NULL instance. */
	if (InstPtr == NULL) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Drop any parked request, so a resume task which is already queued cannot re-arm a
	 * request this command has finished with.
	 */
	XAsufw_SlhdsaClearParkedRequest();

	/**
	 * Save the keygen stage before the checkpoints are dropped. An aborted keygen may still
	 * hold a PWCT signature; sign and verify leave this stage at NONE and skip that wipe.
	 */
	KeygenStage = InstPtr->Yield.Keygen.Stage;

	/**
	 * Save the deferred wipe failure before the checkpoints are dropped. PRF_msg and H_msg
	 * cannot report a failed wipe while they are waiting on a SHA message DMA, because the
	 * buffer-clear bits would stop the status comparing equal to XASUFW_CMD_IN_PROGRESS.
	 */
	HashWipeFail = InstPtr->Yield.Session.HashWipeFail;

	/** Clear the session identity and every KeyGen/Sign/tree resume checkpoint. */
	InstPtr->Yield.Session.Active = 0U;
	InstPtr->Yield.Session.ReqId = 0U;
	InstPtr->Yield.Session.Slices = 0U;
	InstPtr->Yield.Session.XmssLeaves = 0U;
	InstPtr->Yield.Session.ForsLeaves = 0U;
	InstPtr->Yield.Session.HashDmaResume = 0U;
	InstPtr->Yield.Session.HashWipeFail = 0U;
	InstPtr->Yield.Keygen.Stage = XSLHDSA_KEYGEN_STAGE_NONE;
	InstPtr->Yield.Sign.Stage = XSLHDSA_SIGN_STAGE_NONE;
	InstPtr->Yield.Xmss.Resume = 0U;
	InstPtr->Yield.Fors.TreeResume = 0U;
	InstPtr->Yield.Fors.EmitResume = 0U;
	InstPtr->Yield.Xmss.AuthPathResume = 0U;
	InstPtr->Yield.Xmss.EmitResume = 0U;
	InstPtr->Yield.Xmss.EmitWotsDone = 0U;
	InstPtr->Yield.Ht.Resume = 0U;
	InstPtr->Yield.Ht.EmitPending = 0U;

	/**
	 * Clear the staged key, the scratch buffers and the cached descriptors the instance
	 * reaches, which is also what hands the shared crypto pool back for the next command.
	 * Clears are folded into Status rather than aborting on the first failure, so the
	 * remaining secrets are still wiped and the failure is still reported.
	 */
	Status = XAsufw_UpdateBufStatus(Status, XSlhDsa_ClearSensitiveState(InstPtr));

	/**
	 * Wipe the signature scratch only when this session generated a key pair, so the PWCT
	 * signature made under the new key does not outlive it. A client signature left in the
	 * scratch is public. Sign and verify leave Keygen.Stage at NONE.
	 */
	if (KeygenStage != XSLHDSA_KEYGEN_STAGE_NONE) {
		Status = XAsufw_UpdateBufStatus(Status, XSlhDsa_ZeroizeSigScratch());
	}

	/**
	 * Zeroize the secrets parked on the yield context itself. The top XMSS root is the
	 * PK.root the hypertree commits to, and reusing a signing randomizer across two messages
	 * would leak FORS key material, so neither may survive the command on any terminating
	 * exit path.
	 */
	Status = XAsufw_UpdateBufStatus(Status,
			Xil_SecureZeroize(InstPtr->Yield.Sign.AddRand,
					  sizeof(InstPtr->Yield.Sign.AddRand)));
	Status = XAsufw_UpdateBufStatus(Status,
			Xil_SecureZeroize(InstPtr->Yield.Keygen.Root,
					  sizeof(InstPtr->Yield.Keygen.Root)));

	/** Report a PRF_msg or H_msg wipe that failed while a SHA message DMA was in flight. */
	if (HashWipeFail != 0U) {
		Status = XAsufw_UpdateBufStatus(Status, XASUFW_FAILURE);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function resolves the leaf counter and budget of a yield unit.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Unit	Yield unit (XSLHDSA_YIELD_UNIT_XMSS_LEAF or XSLHDSA_YIELD_UNIT_FORS_LEAF).
* @param	BudgetPtr	Filled with the leaves this unit may build per slice.
*
* @return	Pointer to the leaf counter of the unit, or NULL when Unit is unknown.
*
*************************************************************************************************/
static u32 *XSlhDsa_YieldUnitCounter(XSlhDsa_Instance *InstPtr, u32 Unit, u32 *BudgetPtr)
{
	u32 *CounterPtr;

	if (Unit == XSLHDSA_YIELD_UNIT_XMSS_LEAF) {
		CounterPtr = &InstPtr->Yield.Session.XmssLeaves;
		*BudgetPtr = InstPtr->Param.yield_xmss_leaves;
	} else if (Unit == XSLHDSA_YIELD_UNIT_FORS_LEAF) {
		CounterPtr = &InstPtr->Yield.Session.ForsLeaves;
		*BudgetPtr = InstPtr->Param.yield_fors_leaves;
	} else {
		CounterPtr = NULL;
	}

	return CounterPtr;
}

/*************************************************************************************************/
/**
* @brief	This function checks whether the slice has spent the leaf budget of a unit, without
*		claiming a leaf itself.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Unit	Yield unit whose budget gates the caller's loop.
*
* @return
*		- XASUFW_CMD_IN_PROGRESS when the slice has built its leaves for this unit.
*		- XASUFW_SUCCESS when the caller may continue.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr is NULL or Unit is unknown.
*
* @note	This is the checkpoint used by the auth-path and hypertree layer loops, which drive
*	the leaf loops rather than building leaves themselves.
*
*************************************************************************************************/
s32 XSlhDsa_YieldPoll(XSlhDsa_Instance *InstPtr, u32 Unit)
{
	s32 Status = XASUFW_SUCCESS;
	const u32 *CounterPtr;
	u32 Budget = 0U;

	/** Reject a NULL instance. */
	if (InstPtr == NULL) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** An inactive session is not a yield request; the caller may continue. */
	if (InstPtr->Yield.Session.Active == 0U) {
		goto END;
	}

	/** Resolve the leaf counter and per-slice budget of this unit. */
	CounterPtr = XSlhDsa_YieldUnitCounter(InstPtr, Unit, &Budget);
	if (CounterPtr == NULL) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * A slice which has not built a leaf yet must never yield here, otherwise a budget of
	 * zero would park the command forever without making progress.
	 */
	if ((*CounterPtr != 0U) && (*CounterPtr >= Budget)) {
		Status = XASUFW_CMD_IN_PROGRESS;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function claims one leaf of a unit for the current slice, or asks the caller
*		to yield when the slice has already built its leaves.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	Unit	Yield unit the caller is about to build a leaf for.
*
* @return
*		- XASUFW_CMD_IN_PROGRESS when the slice has built its leaves for this unit.
*		- XASUFW_SUCCESS when the caller may build the leaf.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr is NULL or Unit is unknown.
*
*************************************************************************************************/
s32 XSlhDsa_YieldPollLeaf(XSlhDsa_Instance *InstPtr, u32 Unit)
{
	s32 Status = XASUFW_SUCCESS;
	u32 *CounterPtr;
	u32 Budget = 0U;

	/** Reject a NULL instance. */
	if (InstPtr == NULL) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/** An inactive session is not a yield request; the caller may continue. */
	if (InstPtr->Yield.Session.Active == 0U) {
		goto END;
	}

	/** Resolve the leaf counter and per-slice budget of this unit. */
	CounterPtr = XSlhDsa_YieldUnitCounter(InstPtr, Unit, &Budget);
	if (CounterPtr == NULL) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Build at least one leaf per slice whatever the budget says, so an unbound parameter
	 * set cannot park the command forever without making progress.
	 */
	if ((*CounterPtr != 0U) && (*CounterPtr >= Budget)) {
		Status = XASUFW_CMD_IN_PROGRESS;
	} else {
		(*CounterPtr)++;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function re-queues the active IPI command after a cooperative yield slice.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
* @param	ReqBuf	Pointer to the active request buffer.
* @param	ReqId	Active request identifier (includes queue-task private data).
*
* @return
*		- XASUFW_SUCCESS, if the request was parked and its resume was scheduled.
*		- XASUFW_SLHDSA_INVALID_PARAM, if InstPtr or ReqBuf is NULL.
*		- XASUFW_TASK_INVALID_HANDLER, if the queue task for ReqId cannot be resolved or
*		  the resume task does not exist.
*
* @note	Parking and resuming live in the resident SLH-DSA handler, not here, because this file
*	is part of the SLH-DSA overlay image and the resume runs through a task which must stay
*	reachable whether or not the overlay is loaded.
*
*************************************************************************************************/
s32 XSlhDsa_YieldScheduleResume(XSlhDsa_Instance *InstPtr, const XAsu_ReqBuf *ReqBuf,
				u32 ReqId)
{
	s32 Status = XASUFW_FAILURE;

	/** Reject a NULL instance or request buffer. */
	if ((InstPtr == NULL) || (ReqBuf == NULL)) {
		Status = XASUFW_SLHDSA_INVALID_PARAM;
		goto END;
	}

	/**
	 * Park the request and hand its resume to the resident resume task. The caller has
	 * already committed to XASUFW_CMD_IN_PROGRESS, which holds every allocated resource until
	 * the command finally completes, so a park that cannot be carried out is reported rather
	 * than leaving the command waiting on a task that will never run.
	 */
	Status = XAsufw_SlhdsaParkRequest(ReqBuf, ReqId);

END:
	return Status;
}

/*************************************************************************************************/
/**
* @brief	This function starts a fresh yield slice by returning the full leaf budget.
*
* @param	InstPtr	Pointer to SLH-DSA instance.
*
*************************************************************************************************/
static void XSlhDsa_YieldSliceReset(XSlhDsa_Instance *InstPtr)
{
	/** Clear the leaves built, so the next slice may build a full budget of each unit. */
	if (InstPtr != NULL) {
		InstPtr->Yield.Session.XmssLeaves = 0U;
		InstPtr->Yield.Session.ForsLeaves = 0U;
	}
}

#endif /* XASU_SLHDSA_ENABLE */
/** @} */
