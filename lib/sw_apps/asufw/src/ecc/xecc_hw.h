/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xecc_hw.h
 * @{
 *
 * This header file contains ECC core hardware register offsets of ASU.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date       Changes
 * ----- ------  ---------- -----------------------------------------------------------------------
 * 1.0   yog     06/19/2024 First Release
 *
 * </pre>
 *
 **************************************************************************************************/
#ifndef XECC_HW_H_
#define XECC_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/

/************************************ Constant Definitions ***************************************/
/* Definitions for driver ECC */
#define XASU_XECC_NUM_INSTANCES			(1U)

/* Definitions for peripheral ECC0 */
#define XASU_XECC_0_DEVICE_ID			(0U)
#define XASU_XECC_0_BASEADDR			(0xEBF00000U)

/**
 * @name  CTRL register
 * @{
 */
/**< Control register offset and definitions */
#define XECC_CTRL_OFFSET			(0x00000010U)
#define XECC_CTRL_START_MASK			(0x00000001U)
#define XECC_CTRL_OPCODE_MASK			(0x0000000EU)
#define XECC_CTRL_OPCODE_SHIFT			(0x00000001U)
#define XECC_CTRL_CURVE_MASK			(0x00000020U)
#define XECC_CTRL_CURVE_SHIFT			(0x00000005U)
#define XECC_CTRL_SUPPRESS_SCP2_MASK		(0x00000100U)
#define XECC_CTRL_SUPPRESS_SCP_MASK		(0x00000200U)
#define XECC_CTRL_SIGN_VERIFICATION_OP_CODE	(0x00000000U)
#define XECC_CTRL_PUB_KEY_VALIDATION_OP_CODE	(0x00000001U)
#define XECC_CTRL_PUB_KEY_GENERATION_OP_CODE	(0x00000002U)
#define XECC_CTRL_SIGN_GENERATION_OP_CODE	(0x00000003U)

/**
 * @name  Status register
 * @{
 */
/**< Status register offset and definitions */
#define XECC_STATUS_OFFSET			(0x00000014U)
#define XECC_STATUS_TERMINATION_CODE_MASK	(0x0000000FU)
#define XECC_STATUS_SCP_ENABLED_MASK		(0x00000020U)
#define XECC_STATUS_BUSY_MASK			(0x00000080U)

/**
 * @name  Reset register
 * @{
 */
/**< Reset register offset and definitions */
#define XECC_RESET_OFFSET			(0x00000040U) /**< Reset register offset */
#define XECC_RESET_MASK				(0x00000001U)

/**
 * @name  ISR register
 * @{
 */
/**< Interrupt Status register offset */
#define XECC_ISR_OFFSET				(0x00000048U)
#define XECC_ISR_DONE_MASK			(0x00000001U)

/**
 * @name  IER register
 * @{
 */
/**< Interrupt Enable register offset */
#define XECC_IER_OFFSET				(0x00000050U)
#define XECC_IER_DONE_MASK			(0x00000001U)

/**
 * @name  IDR register
 * @{
 */
/**< Interrupt Disable register offset */
#define XECC_IDR_OFFSET				(0x00000054U)
#define XECC_IDR_DONE_MASK			(0x00000001U)

/**
 * @name  CFG register
 * @{
 */
/**< ECC RAM data endianness configuration register offset */
#define XECC_CFG_OFFSET				(0x0000005CU)
#define XECC_CFG_WR_ENDIANNESS_MASK		(0x00000001U)
#define XECC_CFG_RD_ENDIANNESS_MASK		(0x00000002U)

#define XECC_MEM_SIGN_R_OFFSET			(0x00000200U)
#define XECC_MEM_SIGN_S_OFFSET			(0x00000230U)

#define XECC_MEM_GEN_SIGN_PVT_KEY_OFFSET	(0x00000200U)
#define XECC_MEM_GEN_KEY_PVT_KEY_OFFSET		(0x00000230U)
#define XECC_MEM_EPHEMERAL_KEY_OFFSET		(0x00000230U)
#define XECC_MEM_HASH_OFFSET			(0x00000260U)

#define XECC_MEM_PUB_KEY_X_OFFSET		(0x00000290U)
#define XECC_MEM_PUB_KEY_Y_OFFSET		(0x000002C0U)

#ifdef __cplusplus
}
#endif

#endif /* XECC_HW_H_ */