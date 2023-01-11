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
 *       am     01/10/23 Added nonce buffer macro for dme client support
 *
 * </pre>
 *
 * User configurable parameters for OCP
 *------------------------------------------------------------------------------
 * #define XOCP_EXTEND_HASH
 *   "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
 * XOCP_EXTEND_HASH should be provided in string format. It should be
 * 96 characters long, which is used to extend hash for requesting ROM services.
 *
 * #define XOCP_NONCE_BUFFER
 * 	      "0000000000000000000000000000000000000000000000000000000000000000"
 * XOCP_NONCE_BUFFER should be provided in string format. It should be
 * 64 characters long, which is used to generate the response to
 * DME challenge request.
 *
 * XOCP_SELECT_PCR_NUM	(XOCP_PCR_2)
 * XOCP_SELECT_PCR_NUM can be configured as one of the seven provided PCR
 * number from XOcp_RomHwPcr enum in xocp_common.h file.
 *
 * XOCP_READ_PCR_MASK	(0x00000004)
 * The lower 8 bits of XOCP_READ_PCR_MASK indicates 8 PCRs, user can set
 * the corresponding bit to read the specific PCR
 * Example: XOCP_READ_PCR_MASK (0x00000004) , 2nd bit of the Mask is set means
 * user want to read the PCR 2
 * XOCP_READ_PCR_MASK (0x0000000B), 2nd and 3rd bitsof the Mask are set, which
 * means user wants to read PCR 2 and PCR 3.
 * Default value is (0x00000004)
 *
 * XOCP_READ_NUM_OF_LOG_ENTRIES Number of PcrLog entries to read into buffer.
 * Default value is (0x00000001)
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
 * Nonce buffer
 */
#define XOCP_NONCE_BUFFER	\
	"0000000000000000000000000000000000000000000000000000000000000000"

/**
 * Following is the define to select PCR number by the user.
 */
#define XOCP_SELECT_PCR_NUM	(XOCP_PCR_2)

#define XOCP_READ_PCR_MASK	(0x00000004)

#define XOCP_READ_NUM_OF_LOG_ENTRIES	(0x00000001)

#ifdef __cplusplus
}
#endif

#endif /* XILOCP_INPUT_H_ */
