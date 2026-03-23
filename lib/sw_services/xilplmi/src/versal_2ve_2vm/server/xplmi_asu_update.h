/***************************************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/


/**************************************************************************************************/
/**
*
* @file xplmi_asu_update.h
*
* This file contains declarations for ASU in-place update functionality specific to versal_2ve_2vm.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ==============================================================================
* 1.0   vm   03/16/26 Initial release
*
* </pre>
*
***************************************************************************************************/

#ifndef XPLMI_ASU_UPDATE_H
#define XPLMI_ASU_UPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files ********************************************/
#include "xplmi_cmd.h"
#include "xil_types.h"
#include "xplmi_update.h"

/************************************ Constant Definitions ****************************************/
/**
 * ASUFW Update State Definitions.
 * NOTE: These state values are shared with ASUFW and must be kept in sync.
 * If modified here, update corresponding XASUFW_UPDATE_STATE_* defines in
 * lib/sw_apps/asufw/src/xasufw_update.h
 */
#define XPLMI_UPDATE_STATE_INIT			(0x0U)	/**< Update state: Init. */
#define XPLMI_UPDATE_STATE_SHUTDOWN_START	(0x1U)	/**< Update state: Shutdown start. */
#define XPLMI_UPDATE_STATE_SHUTDOWN_DONE	(0x2U)	/**< Update state: Shutdown done. */
#define XPLMI_UPDATE_STATE_LOAD_ELF_DONE	(0x3U)	/**< Update state: Load ELF done. */
#define XPLMI_UPDATE_STATE_FINISHED		(0x4U)	/**< Update state: Finished. */

/* ASU Update State Register and Magic Values */
#define XPLMI_ASU_UPDATE_STATE_REG		(0xEBF8003CU) /**< ASU_GLOBAL_GEN_STORAGE7 register
								address */

/* ASU shutdown monitoring parameters */
#define XPLMI_ASU_SHUTDOWN_POLL_INTERVAL_US	(100000U)  /**< Poll interval: 100ms */

/* ASU reset timeout */
#define XPLMI_ASU_RST_TIMEOUT			(150000U)  /**< Timeout to wait for ASU firmware
							     after reset: 150ms */

/* ASU register addresses */
#define XPLMI_ASU_GLOBAL_BASEADDR		(0xEBF80000U) /**< ASU GLOBAL base address */

/* ASU RTCA register */
#define XPLMI_ASU_RTCA_BASEADDR			(0xEBE40000U) /**< ASU RTCA base address */
#define XPLMI_ASU_RTCA_EXEC_STATUS		(XPLMI_ASU_RTCA_BASEADDR + 0x174U)
						/**< ASU RTCA exec status (0xEBE40174) */
#define XPLMI_ASU_RTCA_FW_IS_PRESENT_MASK	(0x00000002U) /**< ASU FW_IS_PRESENT bit in
							     RTCA (bit 1) */

/* ASU register bit definitions */
#define XPLMI_ASU_MB_SOFT_RST			(XPLMI_ASU_GLOBAL_BASEADDR + 0x10054U)
						/**< ASU MicroBlaze soft reset register */
#define XPLMI_ASU_MB_SOFT_RST_ASSERT_MASK	(0x1U) /**< ASU MB soft reset assert mask */
#define XPLMI_ASU_MB_SOFT_RST_DEASSERT_MASK	(0x0U) /**< ASU MB soft reset deassert mask */
#define XPLMI_ASU_RESET_SETTLE_TIME_US		(1000U) /**< 1ms wait for reset to take effect
							     per HW spec */

/************************************** Type Definitions ******************************************/
typedef int (*XPlmi_CheckAsuPresenceInPdi_t)(u32 PdiAddr);
typedef int (*XPlmi_LoadAsuImage_t)(void);

/******************************** Macros (Inline Functions) Definitions ***************************/

/************************************ Function Prototypes *****************************************/
u8 XPlmi_AsuFwIsPresent(void);
int XPlmi_AsuUpdate(XPlmi_Cmd *Cmd);
int XPlmi_AsuUpdateInit(XPlmi_IsPdiAddrLookup_t IsPdiAddrLookupHandler,
		XPlmi_CheckAsuPresenceInPdi_t CheckAsuPresenceHandler,
		XPlmi_LoadAsuImage_t LoadAsuImageHandler);
u32 XPlmi_GetAsuUpdatePdiAddr(void);

#ifdef __cplusplus
}
#endif

#endif /* XPLMI_ASU_UPDATE_H */
