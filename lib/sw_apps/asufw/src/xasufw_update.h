/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_update.h
 *
 * This file contains function declarations related to ASUFW update functionality.
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
#ifndef XASUFW_UPDATE_H_
#define XASUFW_UPDATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xasufw_hw.h"
#include "xasufw_update_ds.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_STORE_DATABASE		(0x1U)	/**< Store database operation */
#define XASUFW_RESTORE_DATABASE		(0x2U)	/**< Restore database operation */
#define XASUFW_UPDATE_DB_VERSION	(1U)	/**< Database version */
#define XASUFW_DS_HDR_SIZE		(sizeof(XAsufw_DsHdr))	/**< Data structure header size */

#define XASUFW_INVALID_RSVD_DDR_ADDR	(0xFFFFFFFFU)	/**< Invalid reserved DDR address */
#define XASUFW_INVALID_RSVD_DDR_SIZE	(0x0U)		/**< Invalid reserved DDR size */

/** ASUFW Update State Register. */
#define XASUFW_UPDATE_STATE_REG			ASU_GLOBAL_GLOBAL_GEN_STORAGE7

/**
 * ASUFW Update State Definitions.
 * NOTE: These state values are shared with PLM and must be kept in sync.
 * If modified here, update corresponding XPLMI_UPDATE_STATE_* defines in
 * lib/sw_services/xilplmi/src/versal_2ve_2vm/xplmi_asu_update.h
 */
#define XASUFW_UPDATE_STATE_INIT		(0x0U)	/**< Update state: Init. */
#define XASUFW_UPDATE_STATE_SHUTDOWN_START	(0x1U)	/**< Update state: Shutdown start. */
#define XASUFW_UPDATE_STATE_SHUTDOWN_DONE	(0x2U)	/**< Update state: Shutdown done. */
#define XASUFW_UPDATE_STATE_LOAD_ELF_DONE	(0x3U)	/**< Update state: Load ELF done. */
#define XASUFW_UPDATE_STATE_FINISHED		(0x4U)	/**< Update state: Finished. */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
/*************************************************************************************************/
/**
 * @brief	This function reads the current ASUFW update state.
 *
 * @return	Current ASUFW update state.
 *
 *************************************************************************************************/
static inline u32 XAsufw_GetUpdateState(void)
{
	return XAsufw_ReadReg(XASUFW_UPDATE_STATE_REG);
}

/*************************************************************************************************/
/**
 * @brief	This function sets the ASUFW update state to the provided value.
 *
 * @param	State	New ASUFW update state to be set.
 *
 *************************************************************************************************/
static inline void XAsufw_SetUpdateState(u32 State)
{
	XAsufw_WriteReg(XASUFW_UPDATE_STATE_REG, State);
}

/************************************ Function Prototypes ****************************************/
s32 XAsufw_PerformAsufwUpdate(void);
s32 XAsufw_RestoreDataBackup(void);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_UPDATE_H_ */
/** @} */
