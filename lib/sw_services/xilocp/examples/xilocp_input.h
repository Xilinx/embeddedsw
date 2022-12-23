/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

 /*****************************************************************************/
 /**
 *
 * @file xilocp_input.h
 * This file contains macros which needs to configured by user for
 * xilocp_client_pcr_example.c and based on the options selected by user,
 * operations will be performed.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- -----  -------- ------------------------------------------------------
 * 1.1   am     12/21/22 Initial release
 *
 * </pre>
 *
 * User configurable parameters for OCP
 *------------------------------------------------------------------------------
 * #define XOCP_EXTEND_HASH
 *   "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
 * XOCP_EXTEND_HASH should be provided in string format. It should be
 * 48 characters long, which is used to extend hash for requesting ROM services.
 *
 *
 * XOCP_SELECT_PCR_NUM	(XOCP_PCR_0)
 * XOCP_SELECT_PCR_NUM can be configured as one of the seven provided PCR
 * number from XOcp_RomHwPcr enum in xocp_common.h file.
 *
 ******************************************************************************/
#ifndef XILOCP_INPUT_H
#define XILOCP_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/**
 * Hash to be extended for requesting ROM services.
 */
#define XOCP_EXTEND_HASH	\
	"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

/**
 * Following is the define to select PCR number by the user.
 */
#define XOCP_SELECT_PCR_NUM	(XOCP_PCR_2)

#ifdef __cplusplus
}
#endif

#endif /* XILOCP_INPUT_H_ */