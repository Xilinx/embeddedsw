/******************************************************************************
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpsmfw_update.h"
#include "xil_util.h"
#include "xpsmfw_iomodule.h"
#include "xpsmfw_power.h"
#define PSM_IOMODULE_IRQ_PENDING_INTC_IRQ_MASK 0xFFFF0000U;
#define min(a,b)	(((a) < (b)) ? (a) : (b))

#define XPSM_RESET_VECTOR	(0xEBC00000U) /**< Reset vector */
extern u32 __update_mgr_b_start[];
extern u8 __update_mgr_a_fn_start[];
extern u8 __update_mgr_a_fn_end[];
static void XPsmFw_PsmUpdateMgr(void) __attribute__((section(".update_mgr_a")));

/****************************************************************************/
/**
 * @brief	Update PSM firmware during runtime
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
static void XPsmFw_PsmUpdateMgr(void) {

	/** NOTE: this function is special.
	 * It is relocated and called in 2 different contexts: old and new PSM code.
	 * The stack is wiped out in between; Therefore no local variables are allowed.
	*/

	/** Clear FW_IS_PRESENT bit from  PSM_GLOBAL_REG_GLOBAL_CNTRL register*/
	*((volatile u32 *)PSM_GLOBAL_REG_GLOBAL_CNTRL) = \
		((*((volatile u32 *)PSM_GLOBAL_REG_GLOBAL_CNTRL)) \
		& (~PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK));

	*((volatile u32 *)PSM_UPDATE_REG_STATE) = PSM_UPDATE_STATE_SHUTDOWN_DONE;
	/** Wait for Psm.elf loaded */
	mb_sleep();
	while (PSM_UPDATE_STATE_LOAD_ELF_DONE != *((volatile u32 *)PSM_UPDATE_REG_STATE));
	/** Reset PSM with new code */
	/** NOTE: We hardcoding asm here to make sure jumping to the reset vector
	 * Else, depending on comipiler optimization,
	 * it will set link register as in subroutine call.
	 * */
	__asm__ __volatile__ (
		"addik r5, r0, %0\n"	// Move the value XPSM_RESET_VECTOR into register r5
		"bra r5\n"	// Branch to the address in r5
		:: "i" (XPSM_RESET_VECTOR) : "r5"	// Tell the compiler that r5 is being modified
	);
}

/****************************************************************************/
/**
 * @brief	Get state of PSM update
 *
 * @return	Return state of PSM update
 *
 * @note	None
 *
 ****************************************************************************/
inline u32 XPsmFw_GetUpdateState(void){
	return Xil_In32(PSM_UPDATE_REG_STATE);
}

/****************************************************************************/
/**
 * @brief	Set state of PSM update
 *
 * @param State	PSM update state
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
inline void XPsmFw_SetUpdateState(u32 State){
	Xil_Out32(PSM_UPDATE_REG_STATE, State);
}

/****************************************************************************/
/**
 * @brief	Shutdown PSM
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsm_DoShutdown(){

	XStatus Status = XST_FAILURE;
	u32 UpdMgrSize = (u32)__update_mgr_a_fn_end - (u32)__update_mgr_a_fn_start;
	void (*XPsm_RelocatedFn)(void) =
			(void (*)(void))__update_mgr_b_start;

	XPsmFw_DisableInterruptSystem();
	/** Relocate PSM Update Manager to reserved safe location */
	Status = Xil_SMemCpy((u8 *)(UINTPTR)__update_mgr_b_start, UpdMgrSize,
		(const void *)&XPsmFw_PsmUpdateMgr, UpdMgrSize, UpdMgrSize);

	/** Store important dynamic data to safe place*/
	Status = XPsmFw_StoreData();
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error: Failed to store data\r\n");
		return Status;
	}
	/** Make sure all pending interrupts are not pending */
	u32 PendingIntcIrq = Xil_In32(PSM_IOMODULE_IRQ_PENDING) & PSM_IOMODULE_IRQ_PENDING_INTC_IRQ_MASK;
	if (PendingIntcIrq) {
		Status = XST_DEVICE_BUSY;
		goto done;
	}

	/* Disable exceptions */
	microblaze_disable_exceptions();
	microblaze_disable_interrupts();

	XPsmFw_StopIoModule();

	/* Jump to relocated PLM Update Manager */
	(void) XPsm_RelocatedFn();
	/** Never reach here */
done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Start shutdown of PSM
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
void XPsmFw_StartShutdown(void){
	XPsmFw_SetUpdateState(PSM_UPDATE_STATE_SHUTDOWN_START);
}

/****************************************************************************/
/**
 * @brief	Store dynamic data
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_StoreData(void){
	XStatus Status = XST_FAILURE;

	/** Save ApuCluster State */
	u8* ApuClusterState = XPsmFw_GetApuClusterStatePtr();
	u8 NumApuCluster = XPsmFw_GetNumApuCluster();
	u32 Position = 0U;
	/** Save Number of APU cluster */
	Status = Xil_SMemCpy((u8 *)XPSM_DATA_SAVED_START + Position, 1,\
		(const u8 *)(&NumApuCluster), 1, 1);
	Position += 1;
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error: Failed to save number of apu cluster\r\n");
		goto done;
	}

	/** Save Apu Cluster content */
	Status = Xil_SMemCpy((u8 *)XPSM_DATA_SAVED_START + Position, NumApuCluster, \
		(const u8 *)ApuClusterState, NumApuCluster, NumApuCluster);
	Position += NumApuCluster;
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error: Failed to save apu cluster state\r\n");
		goto done;
	}

	/** Save PsmtoPlmEvent */
	u32 PsmtoplmEventAddr = 0;
	XPsmFw_GetPsmToPlmEventAddr(&PsmtoplmEventAddr);
	/** First Save the size */
	u32 PsmToPlmEventSize = sizeof(struct PsmToPlmEvent_t);
	Status = Xil_SMemCpy((u8 *)XPSM_DATA_SAVED_START + Position, sizeof(PsmToPlmEventSize),
		(const u8 *)(&PsmToPlmEventSize), sizeof(PsmToPlmEventSize), sizeof(PsmToPlmEventSize));
	Position += sizeof(PsmToPlmEventSize);
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error: Failed to save PsmToPlmEvent size\r\n");
		goto done;
	}
	/** Second Save the content */
	Status = Xil_SMemCpy((u8 *)(XPSM_DATA_SAVED_START + Position), sizeof(struct PsmToPlmEvent_t),
		(const void *)PsmtoplmEventAddr, sizeof(struct PsmToPlmEvent_t), sizeof(struct PsmToPlmEvent_t));
	Position += sizeof(struct PsmToPlmEvent_t);
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "Error: Failed to save PsmToPlmEvent\r\n");
		goto done;
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Restore dynamic data
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_ReStoreData(void){
	XStatus Status = XST_FAILURE;

	/** Restore ApuCluster State */
	u8* ApuClusterState = XPsmFw_GetApuClusterStatePtr();
	u8 NumApuCluster = 0U;
	u32 Position = 0U;

	/** Restore Number of APU cluster */
	Status = Xil_SMemCpy((u8 *)(&NumApuCluster), 1, (const u8 *)XPSM_DATA_SAVED_START + Position, 1, 1);
	Position += 1;
	if (XST_SUCCESS != Status){
		XPsmFw_Printf(DEBUG_ERROR,"Error: during restoring Number of APU Cluster\n\r");
		goto done;
	}
	/** Check NumApuCluster if the same*/
	if (NumApuCluster != XPsmFw_GetNumApuCluster()) {
		XPsmFw_Printf(DEBUG_ERROR,"Error: NumApuCluster mismatched: exp: %d act:%d\n\r", NumApuCluster, XPsmFw_GetNumApuCluster());
		Status = XST_FAILURE;
		goto done;
	}

	/** Retore Apu Cluster State*/
	Status = Xil_SMemCpy((u8 *)ApuClusterState, NumApuCluster,\
		(const u8 *)XPSM_DATA_SAVED_START + Position, NumApuCluster, NumApuCluster);
	Position += NumApuCluster;
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR,"Error: in restoring APU clusters\n\r");
		goto done;
	}

	/** Restore PsmtoPlmEvent */
	/** First Restore the size */
	u32 PsmToPlmEventSize = 0U;
	u32 CopySize = 0U;
	Status = Xil_SMemCpy((u8 *)(&PsmToPlmEventSize), sizeof(PsmToPlmEventSize),
		(const u8 *)(XPSM_DATA_SAVED_START + Position), sizeof(PsmToPlmEventSize), sizeof(PsmToPlmEventSize));

	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR,"Error: in restoring APU clusters\n\r");
		goto done;
	}
	Position += sizeof(PsmToPlmEventSize);

	/** Update the copy size, it should be the smaller one */
	CopySize = min(PsmToPlmEventSize, sizeof(struct PsmToPlmEvent_t));

	/** Second Restore the content */
	u32 PsmtoplmEventAddr = 0;
	XPsmFw_GetPsmToPlmEventAddr(&PsmtoplmEventAddr);
	Status = Xil_SMemCpy((void *)PsmtoplmEventAddr, sizeof(struct PsmToPlmEvent_t),
		(const void *)(XPSM_DATA_SAVED_START + Position), PsmToPlmEventSize, CopySize);

	Position += PsmToPlmEventSize;

	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR,"Error: in restoring PsmToPlmEventSize\n\r");
		goto done;
	}

done:
	return Status;
}
