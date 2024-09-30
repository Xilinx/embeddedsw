/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xsha_hw.h
 *
 * This header file contains macros for SHA HW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   06/14/24 Initial release
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XSHA_HW_H
#define XSHA_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

/************************************ Constant Definitions ***************************************/
/* Definitions for SHA2 and SHA3 driver */
#define XASU_XSHA_NUM_INSTANCES				2U /**< SHA number of instances */

/* Definitions for peripheral ASU_SHA2 */
#define XASU_XSHA_0_DEVICE_ID				0U /**< SHA2 Device ID */
#define XASU_XSHA_0_S_AXI_BASEADDR			0xEBF30000U /**< SHA2 base address */
#define XASU_XSHA_0_TYPE					2U /**< SHA2 type */

/* Definitions for peripheral ASU_SHA3 */
#define XASU_XSHA_1_DEVICE_ID				1U /**< SHA3 Device ID */
#define XASU_XSHA_1_S_AXI_BASEADDR			0xEBF40000U /**< SHA3 base address */
#define XASU_XSHA_1_TYPE					3U /**< SHA3 type */

/* Definitions for SHA registers */
#define	XASU_SHA_START_MASK					(0x1U)			/**< SHA start value */

#define XASU_SHA_RESET_OFFSET				(0x00000004U)	/**< SHA Reset register offset */
#define XASU_SHA_RESET_ASSERT_MASK			(0x1U)			/**< SHA Reset assert value */
#define XASU_SHA_RESET_DEASSERT_MASK		(0x0U)			/**< SHA Reset deassert value */

#define XASU_SHA_DONE_OFFSET				(0x00000008U)	/**< SHA Done register offset */
#define	XASU_SHA_DONE_MASK					(0x1U)			/**< SHA Done value */

#define XASU_SHA_NEXT_XOF_OFFSET			(0x0000000CU)	/**< SHA Next XOF register offset */
#define XASU_SHA_NEXT_XOF_ENABLE_MASK		(0x1U)			/**< SHA Next XOF enable mask */

#define XASU_SHA_DIGEST_0_OFFSET			(0x00000010U)	/**< SHA Digest 0 register offset */

#define XASU_SHA_MODE_OFFSET				(0x000000A0U)	/**< SHA Mode register offset */

#define XASU_SHA_AUTO_PADDING_OFFSET		(0x000000A4U)	/**< SHA AutoPadding register offset */
#define	XASU_SHA_AUTO_PADDING_ENABLE_MASK	(0x1U)			/**< SHA Auto Padding enable */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XSHA_HW_H */