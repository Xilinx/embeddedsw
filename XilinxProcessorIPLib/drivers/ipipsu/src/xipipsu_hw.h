/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xipipsu_hw.h
* @addtogroup ipipsu_v2_9
* @{
*
* This file contains macro definitions for low level HW related params
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.0   mjr  03/15/15 First release
* 2.1   kvn  05/05/16 Modified code for MISRA-C:2012 Compliance
 * 2.5  sdd 12/17/18  Add the cpp extern macro.
*
* </pre>
*
******************************************************************************/
#ifndef XIPIPSU_HW_H_	/**< prevent circular inclusions */
#define XIPIPSU_HW_H_	/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/
/* Message RAM related params */
#if defined (versal)
#define XIPIPSU_MSG_RAM_BASE 0xFF3F0000U  /**< IPI Message RAM base address */
#else
#define XIPIPSU_MSG_RAM_BASE 0xFF990000U  /**< IPI Message RAM base address */
#endif
#define XIPIPSU_MSG_BUF_SIZE 8U	/**< Size in Words */
#define XIPIPSU_MAX_BUFF_INDEX	7U /**< Maximum Buffer Index */

/* EIGHT pairs of TWO buffers(msg+resp) of THIRTY TWO bytes each */
#define XIPIPSU_BUFFER_OFFSET_GROUP	(8U * 2U * 32U) /**< Buffer offset for group */
#define XIPIPSU_BUFFER_OFFSET_TARGET (32U * 2U) /**< Buffer offset for target */
#define XIPIPSU_BUFFER_OFFSET_RESPONSE		(32U) /**< Buffer offset for response */

/* Number of IPI slots enabled on the device */
#define XIPIPSU_MAX_TARGETS	XPAR_XIPIPSU_NUM_TARGETS /**< Maximum number of targets */

/* Register Offsets for each member  of IPI Register Set */
#define XIPIPSU_TRIG_OFFSET 0x00U /**< Offset for Trigger register */
#define XIPIPSU_OBS_OFFSET 0x04U  /**< Offset for Observation register */
#define XIPIPSU_ISR_OFFSET 0x10U  /**< Offset for ISR register */
#define XIPIPSU_IMR_OFFSET 0x14U  /**< Offset for Interrupt Mask Register */
#define XIPIPSU_IER_OFFSET 0x18U  /**< Offset for Interrupt Enable Register */
#define XIPIPSU_IDR_OFFSET 0x1CU  /**< Offset for Interrupt Disable Register */

#if defined (versal)
#define XIPIPSU_ISR_BASE	0xFF300010U /**< Versal ISR base address */
#define XIPIPSU_ECC_UE_MASK	0x40U  /**< Uncorrecteble Error mask */
#endif
/* MASK of all valid IPI bits in above registers */
#if defined (versal)
#define XIPIPSU_ALL_MASK	0x000003FFU /**< All valid bit mask */
#else
#define XIPIPSU_ALL_MASK	0x0F0F0301U /**< All valid bit mask */
#endif

#ifdef __cplusplus
}
#endif

#endif /* XIPIPSU_HW_H_ */
/** @} */
