/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_memory.h
 * @addtogroup Overview
 * @{
 *
 * This file contains defines related to ASUFW internal memory.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   07/23/23 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASUFW_MEMORY_H
#define XASUFW_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

/************************************ Constant Definitions ***************************************/
/** ASUFW Run Time Configuration Area base address */
#define	XASUFW_RTCA_BASEADDR				(0xEBE40000U)

/** ASUFW Run Time Configuration Area related register defines */
#define XASUFW_RTCA_IDENTIFICATION_ADDR		(XASUFW_RTCA_BASEADDR + 0x0U)
#define XASUFW_RTCA_VERSION_ADDR			(XASUFW_RTCA_BASEADDR + 0x4U)
#define XASUFW_RTCA_SIZE_ADDR				(XASUFW_RTCA_BASEADDR + 0x8U)
#define XASUFW_RTCA_COMM_CHANNEL_INFO_ADDR	(XASUFW_RTCA_BASEADDR + 0x10U)

/** Default values of ASUFW Run Time Configuration Area registers */
#define XASUFW_RTCA_IDENTIFICATION_STRING	(0x41435452U)
#define XASUFW_RTCA_VERSION					(0x1U)
#define XASUFW_RTCA_SIZE					(0x1000U)

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_MEMORY_H */
/** @} */
