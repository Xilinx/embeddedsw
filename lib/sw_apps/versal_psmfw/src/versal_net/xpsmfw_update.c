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
static int XPsmFw_PsmUpdateMgr(void) __attribute__((section(".update_mgr_a")));

static int XPsmFw_PsmUpdateMgr(void) {
	XStatus Status = XST_FAILURE;
	u32 PsmUpdateState = PSM_UPDATE_STATE_SHUTDOWN_DONE;
	void (*XPsm_ResetVector)(void) = (void (*)(void))XPSM_RESET_VECTOR;
	{
		u32 l_Val;
		u32 RegAddress = PSM_GLOBAL_REG_GLOBAL_CNTRL;
		u32 Mask = PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK;
		u32 Value = 0;
		l_Val = *((volatile u32 *)RegAddress);
		l_Val = (l_Val & (~Mask)) | (Mask & Value);
		*((volatile u32 *)RegAddress) =  l_Val;
	}
	*((volatile u32 *)PSM_UPDATE_REG_STATE) = PSM_UPDATE_STATE_SHUTDOWN_DONE;
	/** Wait for Psm.elf loaded */
	/** Note: we have to use Xil_In32 macro here not function call to XPsmFw_GetUpdateState
	 * because of this function is relocated.
	 **/
	mb_sleep();
	while (PSM_UPDATE_STATE_LOAD_ELF_DONE != PsmUpdateState) {
		PsmUpdateState = *((volatile u32 *)PSM_UPDATE_REG_STATE);
	}
	/** Reset PSM with new code */
	XPsm_ResetVector();
	/** Never reach here. */
	Status = XST_SUCCESS;
	return Status;
}

inline u32 XPsmFw_GetUpdateState(void){
	return Xil_In32(PSM_UPDATE_REG_STATE);
}
inline void XPsmFw_SetUpdateState(u32 State){
	Xil_Out32(PSM_UPDATE_REG_STATE, State);
}

XStatus XPsm_DoShutdown(){

	XStatus Status = XST_FAILURE;
	u32 UpdMgrSize = (u32)__update_mgr_a_fn_end - (u32)__update_mgr_a_fn_start;
	int (*XPsm_RelocatedFn)(void) =
			(int (*)(void))__update_mgr_b_start;

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
	Status = XPsm_RelocatedFn();
	/** Never reach here */
done:
	return Status;
}

void XPsmFw_StartShutdown(void){
	XPsmFw_SetUpdateState(PSM_UPDATE_STATE_SHUTDOWN_START);
}

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
