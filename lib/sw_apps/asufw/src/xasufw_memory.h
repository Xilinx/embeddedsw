/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_memory.h
 *
 * This file contains defines related to ASUFW internal memory.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   07/23/23 Initial release
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
#ifndef XASUFW_MEMORY_H
#define XASUFW_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

/************************************ Constant Definitions ***************************************/
/*  ASUFW Run Time Configuration Area related register defines */
#define	XASUFW_RTCA_BASEADDR			(0xEBE40000U) /**< ASUFW RTCA BaseAddress */
#define XASUFW_RTCA_IDENTIFICATION_ADDR		(XASUFW_RTCA_BASEADDR + 0x0U)
							/**< RTCA identification address */
#define XASUFW_RTCA_VERSION_ADDR		(XASUFW_RTCA_BASEADDR + 0x4U)
							/**< RTCA version address */
#define XASUFW_RTCA_SIZE_ADDR			(XASUFW_RTCA_BASEADDR + 0x8U)
							/**< RTCA size address */
#define XASUFW_RTCA_COMM_CHANNEL_INFO_ADDR	(XASUFW_RTCA_BASEADDR + 0x10U)
							/**< RTCA channel information address */

/* Default values of ASUFW Run Time Configuration Area registers */
#define XASUFW_RTCA_IDENTIFICATION_STRING	(0x41435452U)
							/**< RTCA identification string */
#define XASUFW_RTCA_VERSION			(0x1U)
							/**< RTCA version */
#define XASUFW_RTCA_SIZE			(0x1000U)
							/**< RTCA size */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XASUFW_MEMORY_H */
/** @} */
