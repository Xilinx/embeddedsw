/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_update.c
 *
 * This file contains the implementation of ASUFW update functionality.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   vm   03/16/26 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_update.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_init.h"
#include "xasufw_ipi.h"
#include "xasufw_hw.h"
#include "xasufw_memory.h"
#include "riscv_interface.h"
#include "xil_cache.h"
#include "xil_util.h"
#include "xil_types.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
static void XAsufw_DisableInterrupts(void);
static s32 XAsufw_ShutdownAsu(void);
static s32 XAsufw_InitDbRegion(void);
static u32 XAsufw_GetDbSize(void);
static s32 XAsufw_DsOps(u32 Op, u64 Addr, void *Data);
static XAsufw_DsEntry* XAsufw_GetDsEntry(XAsufw_DsEntry *DsList, u32 DsCnt, XAsufw_DsVer *DsVer);
static s32 XAsufw_StoreDataBackup(void);
static void XAsufw_AsuUpdateMgr(void) __attribute__((section(".update_mgr_a")));

/************************************ Variable Definitions ***************************************/
extern XAsufw_DsEntry __data_struct_start[]; /**< Data structure start address (linker-defined) */
extern XAsufw_DsEntry __data_struct_end[]; /**< Data structure end address (linker-defined) */
extern u32 __update_mgr_b_start[]; /**< Update manager B start address (linker-defined) */
extern u8 __update_mgr_a_fn_start[]; /**< Update manager A function start address (linker-defined) */
extern u8 __update_mgr_a_fn_end[]; /**< Update manager A function end address (linker-defined) */

static u32 DbStartAddr;	/**< Database start address */
static u32 DbEndAddr;	/**< Database end address */

#define XASUFW_DS_CNT	(u32)(__data_struct_end - __data_struct_start)	/**< Data structure count */

/*************************************************************************************************/
/**
 * @brief	This function initializes reserved DDR database region information.
 *
 * @return
 *	- XASUFW_SUCCESS, if reserved DDR region is valid.
 * 	- XASUFW_INVALID_RSVD_DDR_ADDR, if DDR reserved area address is invalid.
 *	- XASUFW_INSUFFICIENT_RSVD_DDR_SIZE, if DDR reserved area size is insufficient.
 *
 *************************************************************************************************/
static s32 XAsufw_InitDbRegion(void)
{
	s32 Status = XASUFW_FAILURE;
	u32 DdrRsvdAddr;
	u32 DdrRsvdSize;

	/** Get DDR reserved area configuration. */
	DdrRsvdAddr = XAsufw_ReadReg(XASUFW_RTCFG_RSVD_DDR_ADDR);
	DdrRsvdSize = XAsufw_ReadReg(XASUFW_RTCFG_RSVD_DDR_SIZE);

	/** Check if DDR reserved area is valid. */
	if ((DdrRsvdAddr == XASUFW_INVALID_RSVD_DDR_ADDR) ||
		(DdrRsvdSize == XASUFW_INVALID_RSVD_DDR_SIZE) ||
		(((u64)DdrRsvdAddr + DdrRsvdSize) > (u64)XASUFW_2GB_END_ADDR)) {
		Status = XASUFW_INVALID_RSVD_DDR_ADDR;
		goto END;
	}

	/** Check if DDR reserved area is sufficient. */
	if (DdrRsvdSize < XAsufw_GetDbSize()) {
		Status = XASUFW_INSUFFICIENT_RSVD_DDR_SIZE;
		goto END;
	}

	DbStartAddr = DdrRsvdAddr;
	DbEndAddr = DdrRsvdAddr + DdrRsvdSize - 1U;
	Status = XASUFW_SUCCESS;

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function disables interrupts by disabling IO Module interrupts,
 *		clearing IPI, and disabling processor exceptions.
 *
 *************************************************************************************************/
static void XAsufw_DisableInterrupts(void)
{
	/** Disable IO Module interrupts. */
	XAsufw_DisableInterruptSystem();

#if defined(XPAR_XIPIPSU_0_BASEADDR)
	/** Clear all pending IPI interrupts. */
	XAsufw_WriteReg(IPI_ASU_ISR, 0xFFFFFFFFU);
#endif

	/** Disable RISC-V interrupts. */
	riscv_disable_interrupts();

	/** Disable processor-level interrupts and exceptions. */
	Xil_ExceptionDisable();
}

/*************************************************************************************************/
/**
 * @brief	This function shuts down the ASU by disabling interrupts, relocating
 *		update manager, and calling it for runtime firmware update.
 *
 * @return
 * 	- This function never returns. In case if it reaches end due to any error during shutdown,
 *	it returns XASUFW_UPDATE_MGR_RELOCATION_FAIL.
 *
 *************************************************************************************************/
static s32 XAsufw_ShutdownAsu(void)
{
	s32 Status = XASUFW_FAILURE;
	u32 UpdMgrSize = (u32)__update_mgr_a_fn_end - (u32)__update_mgr_a_fn_start;
	void (*XAsufw_RelocatedFn)(void) = (void (*)(void))__update_mgr_b_start;

	/** Relocate ASUFW Update Manager to reserved safe location */
	Status = Xil_SMemCpy((u8 *)(UINTPTR)__update_mgr_b_start, UpdMgrSize,
		(const void *)&XAsufw_AsuUpdateMgr, UpdMgrSize, UpdMgrSize);
	if (Status != XASUFW_SUCCESS) {
		/** If relocation fails, set state to SHUTDOWN_DONE and return */
		XAsufw_SetUpdateState(XASUFW_UPDATE_STATE_SHUTDOWN_DONE);
		Status = XASUFW_UPDATE_MGR_RELOCATION_FAIL;
		goto END;
	}

	/** Set update state to INIT. */
	XAsufw_SetUpdateState(XASUFW_UPDATE_STATE_INIT);

	/** Disable event notifiers. */
	Status = XAsufw_EnableDisableEventNotifiers(XASUFW_REGISTER_NOTIFIER_DISABLE);
	if (XASUFW_SUCCESS != Status) {
		XAsufw_Printf(DEBUG_GENERAL, "ASUFW disable event notifiers failed. Error: 0x%x\r\n", Status);
		goto END;
	}

	/** Disable interrupts and clear IPI. */
	XAsufw_DisableInterrupts();

	/** Stop the IO Module. */
	XAsufw_StopIoModule();

	/** Jump to relocated ASUFW Update Manager */
	(void)XAsufw_RelocatedFn();
	/* Never reach here */

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function performs the ASUFW update operation.
 *
 * @return
 *	- XASUFW_SUCCESS, if ASUFW update completed successfully.
 * 	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_INVALID_RSVD_DDR_ADDR, if DDR reserved area address is invalid.
 *	- XASUFW_INSUFFICIENT_RSVD_DDR_SIZE, if DDR reserved area size is insufficient.
 *	- XASUFW_DATA_BACKUP_FAIL, if data backup operation fails.
 *	- XASUFW_INVALID_PARAM, if parameters are invalid.
 *
 *************************************************************************************************/
s32 XAsufw_PerformAsufwUpdate(void)
{
	s32 Status = XASUFW_FAILURE;

	/** Initialize reserved DDR region used for database backup. */
	Status = XAsufw_InitDbRegion();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	/** Store data backup before update. */
	Status = XAsufw_StoreDataBackup();
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_DATA_BACKUP_FAIL;
		goto END;
	}

	/** Shutdown ASU by disabling interrupts and setting update states. */
	Status = XAsufw_ShutdownAsu();

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function does operations like storing, restoring the Data
 *		Structure to Memory during ASUFW update.
 *
 * @param	Op	Type of operation to be performed on the data structure.
 * @param	Addr	Memory address to which data structure should be stored
 *		or restored from.
 * @param	Data	Data Structure Entry.
 *
 * @return
 *	- XASUFW_SUCCESS, in case of success.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_INVALID_PARAM, if invalid parameters or operation type.
 *	- XASUFW_MEM_COPY_FAIL, if memory copy operation fails.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if memory zeroize operation fails.
 *
 *************************************************************************************************/
static s32 XAsufw_DsOps(u32 Op, u64 Addr, void *Data)
{
	s32 Status = XASUFW_FAILURE;
	u32 Len = 0U;
	u32 VerPacked = 0;
	XAsufw_DsHdr *RestoreDsHdr = (XAsufw_DsHdr *)(UINTPTR)Addr;
	const XAsufw_DsEntry *DsEntry = (XAsufw_DsEntry *)Data;

	if (DsEntry == NULL) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	if (DsEntry->DsHdr.Len % XASUFW_WORD_LEN_IN_BYTES) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	if ((Addr + XASUFW_DS_HDR_SIZE + DsEntry->DsHdr.Len) > DbEndAddr) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	if (Op == XASUFW_STORE_DATABASE) {
		/**
		 * STORE operation: Backup data structure from firmware memory to DDR.
		 * Copy data structure content to DDR location after header space.
		 */
		Status = Xil_SMemCpy((void *)(UINTPTR)(Addr + XASUFW_DS_HDR_SIZE),
			DsEntry->DsHdr.Len, (void *)(UINTPTR)DsEntry->Addr,
			DsEntry->DsHdr.Len, DsEntry->DsHdr.Len);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}
		/**
		 * Write data structure version header to DDR.
		 * Pack version information (DsId, ModuleId, Version, LcVersion) into u32.
		 * Write packed version and length at the beginning of DDR location.
		 */
		VerPacked = XASUFW_PACK_DS_VER(DsEntry->DsHdr.Ver.DsId,
			DsEntry->DsHdr.Ver.ModuleId, DsEntry->DsHdr.Ver.Version,
			DsEntry->DsHdr.Ver.LcVersion);
		XAsufw_WriteReg((u32)Addr, VerPacked);
		XAsufw_WriteReg((u32)(Addr + XASUFW_WORD_LEN_IN_BYTES), DsEntry->DsHdr.Len);
	} else if (Op == XASUFW_RESTORE_DATABASE) {
		Len = DsEntry->DsHdr.Len;
		/**
		 * Handle version compatibility during restore.
		 * If new firmware's data structure is larger than backed up version,
		 * restore only the backed up size and zero-initialize new fields.
		 */
		if (DsEntry->DsHdr.Len > RestoreDsHdr->Len) {
			Len = RestoreDsHdr->Len;
			/** Zero-initialize additional fields that didn't exist in old firmware. */
			Status = Xil_SMemSet((void *)(UINTPTR)(DsEntry->Addr + Len),
					DsEntry->DsHdr.Len - Len, 0U,
					DsEntry->DsHdr.Len - Len);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_ZEROIZE_MEMSET_FAIL;
				goto END;
			}
		}
		/**
		 * RESTORE operation: Restore data structure from DDR backup to firmware memory.
		 * Copy backed up data from DDR to new firmware's data structure location.
		 */
		Status = Xil_SMemCpy((void *)(UINTPTR)DsEntry->Addr, Len,
			(void *)(UINTPTR)(Addr + XASUFW_DS_HDR_SIZE), Len, Len);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_MEM_COPY_FAIL;
			goto END;
		}
	} else {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function returns DsEntry found after searching in the provided DsList.
 *
 * @param	DsList	Data Structure List in which the DS is searched.
 * @param	DsCnt	Number of Data Structures present in the List.
 * @param	DsVer	Version information of the data structure.
 *
 * @return
 *	- Pointer to DsEntry in case of success.
 *	- NULL in case of failure.
 *
 *************************************************************************************************/
static XAsufw_DsEntry* XAsufw_GetDsEntry(XAsufw_DsEntry *DsList, u32 DsCnt,
				XAsufw_DsVer *DsVer)
{
	XAsufw_DsEntry *Result = NULL;
	u32 Index;

	if (DsList == NULL) {
		goto END;
	}

	for (Index = 0U; Index < DsCnt; Index++) {
		if ((DsList[Index].DsHdr.Ver.ModuleId == DsVer->ModuleId) &&
			(DsList[Index].DsHdr.Ver.DsId == DsVer->DsId)) {
			Result = &DsList[Index];
			break;
		}
	}

END:
	return Result;
}

/*************************************************************************************************/
/**
 * @brief	This function restores all the data structures after In Place ASUFW update.
 *
 * @return
 *	- XASUFW_SUCCESS, in case of success.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_DATA_RESTORE_FAIL, if data restore operation fails.
 *	- XASUFW_INVALID_PARAM, if parameters are invalid.
 *
 *************************************************************************************************/
s32 XAsufw_RestoreDataBackup(void)
{
	s32 Status = XASUFW_FAILURE;
	XAsufw_DsEntry *DsEntry = NULL;
	XAsufw_DsHdr *DsHdr = NULL;
	const XAsufw_DbHdr *DbHdr;
	u64 DsAddr;
	u64 EndAddr;

	/** Reinitialize database region after reboot before reading backup data. */
	Status = XAsufw_InitDbRegion();
	if (Status != XASUFW_SUCCESS) {
		goto END;
	}

	DbHdr = (XAsufw_DbHdr *)DbStartAddr;

	/** Validate database header version. */
	if (DbHdr->HdrVersion != XASUFW_UPDATE_DB_VERSION) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Validate database header size. */
	if (DbHdr->HdrSize != sizeof(XAsufw_DbHdr)) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Calculate data structure start and end addresses. */
	DsAddr = (u64)(DbStartAddr + (u32)DbHdr->HdrSize);
	EndAddr = DsAddr + (DbHdr->DbSize * XASUFW_WORD_LEN_IN_BYTES);

	/** Validate that end address is within database bounds. */
	if (EndAddr > DbEndAddr) {
		Status = XASUFW_INVALID_PARAM;
		goto END;
	}

	/** Iterate through all data structures and restore them. */
	while (DsAddr < EndAddr) {
		/** Find the corresponding data structure entry. */
		DsEntry = XAsufw_GetDsEntry(__data_struct_start, XASUFW_DS_CNT,
				(XAsufw_DsVer *)(UINTPTR)DsAddr);
		if (DsEntry != NULL) {
			/** Validate that handler function is available. */
			if (DsEntry->Handler == NULL) {
				Status = XASUFW_INVALID_PARAM;
				break;
			}
			/** Call handler to restore the data structure. */
			Status = DsEntry->Handler(XASUFW_RESTORE_DATABASE,
					DsAddr, DsEntry);
			if (Status != XASUFW_SUCCESS) {
				Status = XASUFW_DATA_RESTORE_FAIL;
				break;
			}
		}

		/** Move to next data structure. */
		DsHdr = (XAsufw_DsHdr *)(UINTPTR)DsAddr;
		DsAddr += XASUFW_DS_HDR_SIZE + DsHdr->Len;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function store all the data structures before InPlace
 *		ASUFW update.
 *
 * @return
 *	- XASUFW_SUCCESS, in case of success.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_ZEROIZE_MEMSET_FAIL, if memory zeroize operation fails.
 *	- XASUFW_DATA_BACKUP_FAIL, if data backup operation fails.
 *	- XASUFW_INVALID_PARAM, if parameters are invalid.
 *
 *************************************************************************************************/
static s32 XAsufw_StoreDataBackup(void)
{
	s32 Status = XASUFW_FAILURE;
	XAsufw_DsEntry *DsEntry = __data_struct_start;
	XAsufw_DbHdr * volatile DbHdr = (XAsufw_DbHdr *)DbStartAddr;
	u64 DsAddr;
	u32 DsCnt;
	u32 Index;

	/** Initialize database header with zeros. */
	Status = Xil_SMemSet((void *)DbHdr, sizeof(XAsufw_DbHdr), (int)0x0U,
			sizeof(XAsufw_DbHdr));
	if (Status != XASUFW_SUCCESS) {
		Status = XASUFW_ZEROIZE_MEMSET_FAIL;
		goto END;
	}

	/** Initialize database header fields. */
	DsCnt = XASUFW_DS_CNT;
	DbHdr->HdrVersion = XASUFW_UPDATE_DB_VERSION;
	DbHdr->HdrSize = (u8)sizeof(XAsufw_DbHdr);

	/** Calculate starting address for data structures. */
	DsAddr = (u64)(DbStartAddr + (u32)DbHdr->HdrSize);

	/** Iterate through all data structures and store them. */
	for (Index = 0; Index < DsCnt; Index++) {
		/** Validate that handler function is available. */
		if (DsEntry[Index].Handler == NULL) {
			Status = XASUFW_INVALID_PARAM;
			break;
		}
		/** Call handler to store the data structure. */
		Status = DsEntry[Index].Handler(XASUFW_STORE_DATABASE,
				DsAddr, &DsEntry[Index]);
		if (Status != XASUFW_SUCCESS) {
			Status = XASUFW_DATA_BACKUP_FAIL;
			break;
		}
		/** Move to next data structure address. */
		DsAddr += XASUFW_DS_HDR_SIZE + DsEntry[Index].DsHdr.Len;
	}

	/** Update database size if all data structures stored successfully. */
	if (Index == DsCnt) {
		DbHdr->DbSize = (u32)(DsAddr - (u64)DbStartAddr -
				(u32)DbHdr->HdrSize) / XASUFW_WORD_LEN_IN_BYTES;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function provides the ASUFW Database Size which has to be
 *		stored and restored during the update.
 *
 * @return
 *	- Size of ASUFW Database.
 *
 *************************************************************************************************/
static u32 XAsufw_GetDbSize(void)
{
	const XAsufw_DsEntry *DsEntry = __data_struct_start;
	u32 DsCnt = XASUFW_DS_CNT;
	u32 Index = 0U;
	u32 Size = sizeof(XAsufw_DbHdr);

	for (Index = 0U; Index < DsCnt; Index++) {
		Size += XASUFW_DS_HDR_SIZE + DsEntry[Index].DsHdr.Len;
	}

	return Size;
}

/*************************************************************************************************/
/**
 * @brief	ASUFW Update Manager function for runtime firmware update.
 *
 * @note	This function is special and is relocated and called in 2 different contexts:
 *		old and new ASUFW code. The stack is wiped out in between; therefore no local
 *		variables are allowed.
 *
 *************************************************************************************************/
static void XAsufw_AsuUpdateMgr(void)
{
	/** Clear FW_IS_PRESENT bit from ASU_GLOBAL_GLOBAL_CNTRL register */
	*((volatile u32 *)ASU_GLOBAL_GLOBAL_CNTRL) =
		((*((volatile u32 *)ASU_GLOBAL_GLOBAL_CNTRL))
		& (~ASU_GLOBAL_GLOBAL_CNTRL_FW_IS_PRESENT_MASK));

	/** Clear FW_IS_PRESENT bit from RTCA_EXEC_STATUS register */
	*((volatile u32 *)XASU_RTCA_EXEC_STATUS_ADDR) =
		((*((volatile u32 *)XASU_RTCA_EXEC_STATUS_ADDR)) &
		(~XASU_RTCA_FW_IS_PRESENT_STATUS_MASK));

	/** Set update state to SHUTDOWN_DONE */
	*((volatile u32 *)XASUFW_UPDATE_STATE_REG) = XASUFW_UPDATE_STATE_SHUTDOWN_DONE;

	/** Wait for ASUFW.elf to be loaded */
	while (XASUFW_UPDATE_STATE_LOAD_ELF_DONE != *((volatile u32 *)XASUFW_UPDATE_STATE_REG));

	/** Reset ASUFW with new code - Jump to reset vector */
	/**
	 * NOTE: We hardcode asm here to make sure jumping to the reset vector
	 * Else, depending on compiler optimization,
	 * it will set link register as in subroutine call.
	 */
	__asm__ __volatile__ (
		"li t0, %0\n"		/* Load the reset vector address into register t0. */
		"jr t0\n"		/* Jump to the address in t0. */
		:: "i" (XASUFW_RAM_START_ADDR) : "t0"	/* Tell the compiler that t0 is
							being modified. */
	);
}
/** @} */
