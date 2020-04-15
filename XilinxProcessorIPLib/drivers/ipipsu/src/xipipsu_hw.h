/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/**
*
* @file xipipsu_hw.h
* @addtogroup ipipsu_v2_6
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
#ifndef XIPIPSU_HW_H_	/* prevent circular inclusions */
#define XIPIPSU_HW_H_	/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/
/* Message RAM related params */
#if defined (versal)
#define XIPIPSU_MSG_RAM_BASE 0xFF3F0000U
#else
#define XIPIPSU_MSG_RAM_BASE 0xFF990000U
#endif
#define XIPIPSU_MSG_BUF_SIZE 8U	/* Size in Words */
#define XIPIPSU_MAX_BUFF_INDEX	7U

/* EIGHT pairs of TWO buffers(msg+resp) of THIRTY TWO bytes each */
#define XIPIPSU_BUFFER_OFFSET_GROUP	(8U * 2U * 32U)
#define XIPIPSU_BUFFER_OFFSET_TARGET (32U * 2U)
#define XIPIPSU_BUFFER_OFFSET_RESPONSE		(32U)

/* Number of IPI slots enabled on the device */
#define XIPIPSU_MAX_TARGETS	XPAR_XIPIPSU_NUM_TARGETS

/* Register Offsets for each member  of IPI Register Set */
#define XIPIPSU_TRIG_OFFSET 0x00U
#define XIPIPSU_OBS_OFFSET 0x04U
#define XIPIPSU_ISR_OFFSET 0x10U
#define XIPIPSU_IMR_OFFSET 0x14U
#define XIPIPSU_IER_OFFSET 0x18U
#define XIPIPSU_IDR_OFFSET 0x1CU

/* MASK of all valid IPI bits in above registers */
#if defined (versal)
#define XIPIPSU_ALL_MASK	0x000003FFU
#else
#define XIPIPSU_ALL_MASK	0x0F0F0301U
#endif

#ifdef __cplusplus
}
#endif

#endif /* XIPIPSU_HW_H_ */
/** @} */
