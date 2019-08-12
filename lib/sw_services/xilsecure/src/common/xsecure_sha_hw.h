/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsecure_sha_hw.h
* This file contains SHA3 core hardware definitions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0   vns  03/09/19 Initial release
*
* </pre>
*
* @endcond
******************************************************************************/

#ifndef XSECURE_SHA_HW_H
#define XSECURE_SHA_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xsecure_utils.h"

/************************** Constant Definitions ****************************/
/**< SHA3 base address */
#ifndef XSECURE_VERSAL
#define XSECURE_CSU_SHA3_BASE	(XSECURE_CSU_REG_BASE_ADDR + 0x2000U)
				/**< Versal base address */
#else
#define XSECURE_CSU_SHA3_BASE	(0xF1210000U)
				/**< ZynqMP base address */
#endif


/** @name Register Map
 *
 * Register offsets for the SHA module.
 * @{
 */
#define XSECURE_CSU_SHA3_START_OFFSET	(0x00U) /**< SHA start message */
#define XSECURE_CSU_SHA3_RESET_OFFSET	(0x04U) /**< Reset Register */
#define XSECURE_CSU_SHA3_DONE_OFFSET	(0x08U) /**< SHA Done Register */

#define XSECURE_CSU_SHA3_DIGEST_0_OFFSET (0x10U)
					/**< SHA3 Digest: Reg 0 */
#define XSECURE_CSU_SHA3_DIGEST_11_OFFSET (0x3CU)
					/**< SHA3 Digest: Last Register */
/* @} */

#ifdef __cplusplus
extern "C" }
#endif

#endif /* XSECURE_SHA_HW_H */
