/**************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasu_perf.h
 *
 * This file Contains the client performance measurement function prototypes, defines and macros.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   am   03/05/25 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/
/**
 * @addtogroup xasu_perf_client_apis PERF Client APIs
 * @{
*/
#ifndef XASU_PERF_H_
#define XASU_PERF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#ifdef SDT
#include "xasu_bsp_config.h"
#endif

/** If performance measurement is enabled */
#ifdef XASU_PERF_MEASUREMENT_ENABLE

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_LPD_SYSTMR_CTRL_BASE_ADDRESS	(0xEA470000U)
	/**< Base address LPD time stamp generator. */
#define XASU_LPD_SYSTMR_CTRL_CNTCR_OFFSET	(0x0U)
	/**< Offset of the counter control register controls the counter increments. */
#define XASU_LPD_SYSTMR_CTRL_CNTCR_ENABLE	(0x1U)
	/**< Enable the counter control. */
#define XASU_LPD_SYSTMR_CTRL_CNTCVL_OFFSET	(0x8U)
	/**< Offset of the read or write of the lower 32 bits of the current counter value. */
#define XASU_LPD_SYSTMR_CTRL_CNTCVU_OFFSET	(0xCU)
	/**< Offset of the read or write of the upper 32 bits of the current counter value. */

/************************************ Function Prototypes ****************************************/
void XAsu_PerfInit(void);
void XAsu_StartTiming(void);
void XAsu_EndTiming(const char* Function_name);
#else
#define XAsu_PerfInit(void)
#define XAsu_StartTiming(void)
#define XAsu_EndTiming(Function_name)
#endif  /* XASU_PERF_MEASUREMENT_ENABLE */

#ifdef __cplusplus
}
#endif

#endif  /* XASU_PERF_H_ */
/** @} */