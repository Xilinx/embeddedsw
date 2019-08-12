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
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_aes_hw.h
*
* This is the header file which contains ZynqMP AES core hardware definitions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 4.0   vns  03/11/19 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XSECURE_AES_HW_H
#define XSECURE_AES_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xparameters.h"
#include "xil_types.h"
#include "sleep.h"
#include "xsecure_utils.h"

/************************** Constant Definitions *****************************/

#define XSECURE_CSU_AES_BASE	(XSECURE_CSU_REG_BASE_ADDR + 0x1000U)
					/**< CSU AES base address */
#define XSECURE_CSU_PCAP_STATUS	(XSECURE_CSU_REG_BASE_ADDR + 0X00003010U)
					/**< CSU PCAP Status reg. */
#define XSECURE_CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK	(0X00000001U)
					/**< PCAP Write Idle */
/** @name Register Map
 *
 * Register offsets for the AES module.
 * @{
 */
#define XSECURE_CSU_AES_STS_OFFSET	(0x00U) /**< AES Status */
#define XSECURE_CSU_AES_KEY_SRC_OFFSET	(0x04U) /**< AES Key Source */
#define XSECURE_CSU_AES_KEY_LOAD_OFFSET	(0x08U) /**< AES Key Load Reg */
#define XSECURE_CSU_AES_START_MSG_OFFSET (0x0CU) /**< AES Start Message */
#define XSECURE_CSU_AES_RESET_OFFSET	(0x10U) /**< AES Reset Register */
#define XSECURE_CSU_AES_KEY_CLR_OFFSET	(0x14U) /**< AES Key Clear */
#define XSECURE_CSU_AES_CFG_OFFSET	(0x18U)/**< AES Operational Mode */
#define XSECURE_CSU_AES_KUP_WR_OFFSET	(0x1CU)/**< AES KUP Write Control */

#define XSECURE_CSU_AES_KUP_0_OFFSET	(0x20U)
						/**< AES Key Update 0 */
#define XSECURE_CSU_AES_KUP_1_OFFSET	(0x24U) /**< AES Key Update 1 */
#define XSECURE_CSU_AES_KUP_2_OFFSET	(0x28U) /**< AES Key Update 2 */
#define XSECURE_CSU_AES_KUP_3_OFFSET	(0x2CU) /**< AES Key Update 3 */
#define XSECURE_CSU_AES_KUP_4_OFFSET	(0x30U) /**< AES Key Update 4 */
#define XSECURE_CSU_AES_KUP_5_OFFSET	(0x34U) /**< AES Key Update 5 */
#define XSECURE_CSU_AES_KUP_6_OFFSET	(0x38U) /**< AES Key Update 6 */
#define XSECURE_CSU_AES_KUP_7_OFFSET	(0x3CU) /**< AES Key Update 7 */

#define XSECURE_CSU_AES_IV_0_OFFSET	(0x40U) /**< AES IV 0 */
#define XSECURE_CSU_AES_IV_1_OFFSET	(0x44U) /**< AES IV 1 */
#define XSECURE_CSU_AES_IV_2_OFFSET	(0x48U) /**< AES IV 2 */
#define XSECURE_CSU_AES_IV_3_OFFSET	(0x4CU) /**< AES IV 3 */
/* @} */

/**************************** Type Definitions *******************************/

/*****************************************************************************/
/**
* Wait for writes to PL and hence PCAP write cycle to complete
*
* @param	None.
*
* @return	None.
*
* @note		C-Style signature:
*			void XSecure_PcapWaitForDone(void)
*
******************************************************************************/
static inline void XSecure_PcapWaitForDone()
{
	while ((Xil_In32(XSECURE_CSU_PCAP_STATUS) &
			XSECURE_CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK) !=
			XSECURE_CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK);
}

/************************** Function Prototypes ******************************/


#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_AES_HW_H */
