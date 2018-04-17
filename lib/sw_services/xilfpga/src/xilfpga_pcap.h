/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xilfpga_pcap.h
* @addtogroup xfpga_apis XilFPGA APIs
* @{
*
* The XILFPGA library provides the interface to the application to configure
* the programmable logic (PL) though the PS.
*
* - Supported Features:
*    - Full Bit-stream loading.
*    - Partial Bit-stream loading.
*    - Encrypted Bit-stream loading.
*    - Authenticated Bit-stream loading.
*    - Authenticated and Encrypted Bit-stream loading.
*    - Partial Bit-stream loading.
*
* #  Xilfpga_PL library Interface modules 	{#xilfpgapllib}
*	Xilfpga_PL library uses the below major components to configure the PL through PS.
*  - CSU DMA driver is used to transfer the actual Bit stream file for the PS to PL after PCAP initialization
*
*  - Xilsecure_library provides APIs to access secure hardware on the Zynq&reg; UltraScale+&tm; MPSoC devices. This library includes:
*	 - SHA-3 engine hash functions
*	 - AES for symmetric key encryption
*	 - RSA for authentication
*
* These algorithms are needed to support to load the Encrypted and Authenticated bit-streams into PL.
*
* @note XilFPGA library is capable of loading only .bin format files into PL. The library does not support other file formats. The current implementation supports only Full Bit-stream.
*
*
* ##   Initialization & Writing Bit-Stream  	{#xilinit}
*
* Use the u32 XFpga_PL_BitSream_Load(); function to initialize the driver and load the bit-stream.
*
* @{
* @cond xilfpga_internal
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   Nava   08/06/16 Initial release
* 1.1   Nava  16/11/16 Added PL power-up sequence.
* 2.0   Nava  10/1/17  Added Encrypted bitstream loading support.
* 2.0   Nava  16/02/17 Added Authenticated bitstream loading support.
* 2.1   Nava  06/05/17 Correct the check logic issues in
*                      XFpga_PL_BitStream_Load()
*                      to avoid the unwanted blocking conditions.
* 3.0   Nava  12/05/17 Added PL configuration registers readback support.
* 4.0   Nava  08/02/18 Added Authenticated and Encypted Bitstream loading support.
* 4.0   Nava  02/03/18 Added the legacy bit file loading feature support from U-boot.
*                      and improve the error handling support by returning the
*                      proper ERROR value upon error conditions.
* 4.1  Nava   27/03/18 For Secure Bitstream loading to avoid the Security violations
*                      Need to Re-validate the User Crypto flags with the Image
*                      Crypto operation by using the internal memory.To Fix this
*                      added a new API XFpga_ReValidateCryptoFlags().
* 4.1 Nava   16/04/18  Added partial bitstream loading support.
*
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XILFPGA_PCAP_H
#define XILFPGA_PCAP_H

/***************************** Include Files *********************************/

#include "xcsudma.h"
/************************** Constant Definitions *****************************/

#define PL_DONE_POLL_COUNT  30000U
#define PL_RESET_PERIOD_IN_US  1U



/* Dummy address to indicate that destination is PCAP */
#define XFPGA_DESTINATION_PCAP_ADDR    (0XFFFFFFFFU)
#define XFPGA_CSU_SSS_SRC_SRC_DMA	(0x5U)
#define XFPGA_CSU_SSS_SRC_DST_DMA	(0x30U)
#define XFPGA_CSU_SSS_DMA_TO_DMA	(0x50U)

/* Boot Header Image Offsets */
#define PARTATION_HEADER_OFFSET 	(0x9cU)
#define PARTATION_ATTRIBUTES_OFFSET	(0x24U)
#define BITSTREAM_PARTATION_OFFSET	(0x20U)
#define BITSTREAM_IV_OFFSET		(0xA0U)

/**
 * CSU Base Address
 */
#define CSU_BASEADDR      0XFFCA0000U

/**
 * Register: CSU_CSU_SSS_CFG
 */
#define CSU_CSU_SSS_CFG    ( ( CSU_BASEADDR ) + 0X00000008U )
#define CSU_CSU_SSS_CFG_PCAP_SSS_MASK    0X0000000FU
#define CSU_CSU_SSS_CFG_PCAP_SSS_SHIFT   0U

/**
 * Register: CSU_PCAP_STATUS
 */
#define CSU_PCAP_STATUS    ( ( CSU_BASEADDR ) + 0X00003010U )
#define CSU_PCAP_STATUS_PL_INIT_SHIFT   2U
#define CSU_PCAP_STATUS_PL_INIT_MASK    0X00000004U
#define CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK    0X00000001U
#define CSU_PCAP_STATUS_PL_DONE_MASK    0X00000008U

/* Register: CSU_PCAP_RESET */
#define CSU_PCAP_RESET    ( ( CSU_BASEADDR ) + 0X0000300CU )
#define CSU_PCAP_RESET_RESET_MASK    0X00000001U

/* Register: CSU_PCAP_CTRL */
#define CSU_PCAP_CTRL    ( ( CSU_BASEADDR ) + 0X00003008U )
#define CSU_PCAP_CTRL_PCAP_PR_MASK    0X00000001U

/**
 * Register: CSU_PCAP_RDWR
 */
#define CSU_PCAP_RDWR    ( ( CSU_BASEADDR ) + 0X00003004U )
#define CSU_PCAP_RDWR_PCAP_RDWR_B_SHIFT   0U

/* Register: CSU_PCAP_PROG */
#define CSU_PCAP_PROG    ( ( CSU_BASEADDR ) + 0X00003000U )
#define CSU_PCAP_PROG_PCFG_PROG_B_MASK    0X00000001U
#define CSU_PCAP_PROG_PCFG_PROG_B_SHIFT   0U

/* Register: PMU_GLOBAL for PL power-up */
#define PMU_GLOBAL_BASE			0xFFD80000U
#define PMU_GLOBAL_PWRUP_STATUS		(PMU_GLOBAL_BASE + 0x110U)
#define PMU_GLOBAL_PWRUP_EN		(PMU_GLOBAL_BASE + 0x118U)
#define PMU_GLOBAL_PWRUP_TRIG		(PMU_GLOBAL_BASE + 0x120U)
#define PMU_GLOBAL_PWR_PL_MASK		0x800000

#define PMU_GLOBAL_ISO_INT_EN		( PMU_GLOBAL_BASE + 0X318U )
#define PMU_GLOBAL_ISO_TRIG		( PMU_GLOBAL_BASE + 0X320U )
#define PMU_GLOBAL_ISO_STATUS		( PMU_GLOBAL_BASE + 0X310U )

#define GPIO_DIRM_5_EMIO		0xFF0A0344
#define GPIO_MASK_DATA_5_MSW	0xFF0A002C

/* Register: PCAP_CLK_CTRL Address */
#define PCAP_CLK_CTRL		0xFF5E00A4
#define PCAP_CLK_EN_MASK	0x01000000

/* AES KEY SRC Info */
#define XFPGA_KEY_SRC_EFUSE_RED		0xA5C3C5A3
#define XFPGA_KEY_SRC_BBRAM_RED		0x3A5C3C5A
#define XFPGA_KEY_SRC_EFUSE_BLK		0xA5C3C5A5
#define XFPGA_KEY_SRC_BH_BLACK		0xA35C7C53
#define XFPGA_KEY_SRC_EFUSE_GRY		0xA5C3C5A7
#define XFPGA_KEY_SRC_BH_GRY		0xA35C7CA5
#define XFPGA_KEY_SRC_KUP		0xA3A5C3C5

#define XFPGA_SUCCESS				(0x0U)
#define XFPGA_FAILURE				(0x1U)
#define XFPGA_ERROR_CSUDMA_INIT_FAIL		(0x2U)
#define XFPGA_ERROR_PL_POWER_UP 		(0x3U)
#define XFPGA_ERROR_PL_ISOLATION		(0x4U)
#define XPFGA_ERROR_PCAP_INIT			(0x5U)
#define XFPGA_ERROR_BITSTREAM_LOAD_FAIL 	(0x6U)
#define XFPGA_ERROR_CRYPTO_FLAGS		(0x7U)
#define XFPGA_ERROR_HDR_AUTH			(0X8U)
#define XFPGA_ENC_ISCOMPULSORY			(0x9U)
#define XFPGA_PARTITION_AUTH_FAILURE		(0xAU)
#define XFPGA_STRING_INVALID_ERROR		(0xBU)
#define XFPGA_ERROR_SECURE_CRYPTO_FLAGS		(0xCU)

/**************************** Type Definitions *******************************/
/***************** Macros (Inline Functions) Definitions *********************/
/** @endcond*/
/************************** Function Prototypes ******************************/
u32 XFpga_PL_BitSream_Load (UINTPTR WrAddr, UINTPTR KeyAddr, u32 flags);
u32 XFpga_PcapStatus(void);
u32 Xfpga_GetConfigReg(u32 ConfigReg, u32 *RegData);
/************************** Variable Definitions *****************************/

extern XCsuDma CsuDma;  /* CSU DMA instance */

#endif  /* XILFPGA_PCAP_H */
/** @} */
