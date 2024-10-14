/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_ipi_manager.h
*
* This file contains definitions for IPI manager
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPSMFW_IPI_MANAGER_H_
#define XPSMFW_IPI_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xparameters.h"
#include "xpsmfw_default.h"

#if defined(XPAR_XIPIPSU_0_DEVICE_ID) || defined(XPAR_XIPIPSU_0_BASEADDR)
#include "xipipsu.h"

#define XPSMFW_IPI_MASK_COUNT 	XIPIPSU_MAX_TARGETS
#define XPSMFW_IPI_MAX_MSG_LEN XIPIPSU_MAX_MSG_LEN

#endif /* XPAR_XIPIPSU_0_DEVICE_ID */

#define XPSMFW_IPI_TIMEOUT	(~0U)
#define PAYLOAD_ARG_CNT		(8U)
#define RESPONSE_ARG_CNT	(8U)

#ifdef XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK
#define IPI_PSM_IER_PMC_MASK	  XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK
#else
#define IPI_PSM_IER_PMC_MASK 0U
#endif

/**
 * Initialize the IPI driver instance
 * This should be called in the core init
 */
XStatus XPsmfw_IpiManagerInit(void);

/****************************************************************************/
/**
 * @brief	Interrupt handler for IPI
 *
 * @param SrcMask	Source mask
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE or error code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_DispatchIpiHandler(u32 SrcMask);

/****************************************************************************/
/**
 * @brief	Sends IPI response to the target.
 *
 * @param IpiMask	IPI interrupt mask of target
 * @param Payload	IPI response
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 *		or a reason code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_IpiSendResponse(u32 IpiMask, u32 *Payload);

/****************************************************************************/
/**
 * @brief	Triggers IPI to the target.
 *
 * @param IpiMask	IPI interrupt mask of target
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 *		or a reason code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_IpiTrigger(u32 IpiMask);

#ifdef __cplusplus
}
#endif

#endif /* XPSMFW_IPI_MANAGER_H_ */
