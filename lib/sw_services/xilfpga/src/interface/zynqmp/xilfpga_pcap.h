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
 *****************************************************************************/
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
 *    - Full Bitstream loading.
 *    - Partial Bitstream loading.
 *    - Encrypted Bitstream loading.
 *    - Authenticated Bitstream loading.
 *    - Authenticated and Encrypted Bitstream loading.
 *    - Partial Bitstream loading.
 *
 * #  Xilfpga_PL library Interface modules	{#xilfpgapllib}
 *	Xilfpga_PL library uses the below major components to configure the PL
 *	through PS.
 *  - CSU DMA driver is used to transfer the actual Bit stream file for the
 *    PS to PL after PCAP initialization
 *
 *  - Xilsecure_library provides APIs to access secure hardware on the Zynq&reg;
 *    UltraScale+&tm; MPSoC devices. This library includes:
 *	 - SHA-3 engine hash functions
 *	 - AES for symmetric key encryption
 *	 - RSA for authentication
 *
 * These algorithms are needed to support to load the Encrypted and
 * Authenticated Bitstreams into PL.
 *
 * @note XilFPGA library is capable of loading only .bin format files into PL.
 * The library does not support other file formats.
 *
 *
 * ##   Initialization & Writing Bitstream	{#xilinit}
 *
 * Use the u32 XFpga_PL_BitSream_Load(); function to initialize the driver
 * and load the Bitstream.
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
 * 4.0   Nava  08/02/18 Added Authenticated and Encypted Bitstream
			loading support.
 * 4.0   Nava  02/03/18 Added the legacy bit file loading feature support
 *			from U-boot.and improve the error handling support
 *			by returning the proper ERROR value upon error
 *			conditions.
 * 4.1  Nava   27/03/18 For Secure Bitstream loading to avoid the Security
 *			violations Need to Re-validate the User Crypto flags
 *			with the Image Crypto operation by using the internal
 *			memory.To Fix this added a new API
 *			XFpga_ReValidateCryptoFlags().
 * 4.1 Nava   16/04/18  Added partial bitstream loading support.
 * 4.2 Nava   08/06/16  Refactor the xilfpga library to support
 *                      different PL programming Interfaces.
 * 4.2 adk    11/07/18  Added support for readback of PL configuration data.
 * 4.2 Nava   22/07/18 Added XFpga_SelectEndianess() new API to Support
 *                      programming the vivado generated .bit and .bin files
 * 4.2 adk   03/08/18 Added example for partial reconfiguration.
 * 4.2 Nava   16/08/18  Modified the PL data handling Logic to support
 *                      different PL programming interfaces.
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

#ifndef XILFPGA_PCAP_H
#define XILFPGA_PCAP_H

/***************************** Include Files *********************************/

#include "xcsudma.h"
#include "xsecure.h"
/************************** Constant Definitions *****************************/

#define PL_DONE_POLL_COUNT  30000U
#define PL_RESET_PERIOD_IN_US  1U



/* Dummy address to indicate that destination is PCAP */
#define XFPGA_DESTINATION_PCAP_ADDR    (0XFFFFFFFFU)
#define XFPGA_CSU_SSS_SRC_SRC_DMA	(0x5U)
#define XFPGA_CSU_SSS_SRC_DST_DMA	(0x30U)
#define XFPGA_CSU_SSS_DMA_TO_DMA	(0x50U)

/* Boot Header Image Offsets */
#define PARTATION_HEADER_OFFSET		(0x9cU)
#define PARTATION_ATTRIBUTES_OFFSET	(0x24U)
#define BITSTREAM_PARTATION_OFFSET	(0x20U)
#define BITSTREAM_IV_OFFSET		(0xA0U)

/**
 * CSU Base Address
 */
#define XILFPGA_CSU_BASEADDR      0XFFCA0000U

/**
 * Register: CSU_CSU_SSS_CFG
 */
#define CSU_CSU_SSS_CFG		((XILFPGA_CSU_BASEADDR) + 0X00000008U)
#define CSU_CSU_SSS_CFG_PCAP_SSS_MASK    0X0000000FU
#define CSU_CSU_SSS_CFG_PCAP_SSS_SHIFT   0U

/**
 * Register: CSU_PCAP_STATUS
 */
#define CSU_PCAP_STATUS    ((XILFPGA_CSU_BASEADDR) + 0X00003010U)
#define CSU_PCAP_STATUS_PL_INIT_SHIFT   2U
#define CSU_PCAP_STATUS_PL_INIT_MASK    0X00000004U
#define CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK    0X00000001U
#define CSU_PCAP_STATUS_PCAP_RD_IDLE_MASK    0X00000002U
#define CSU_PCAP_STATUS_PCAP_RD_SHIFT	1U
#define CSU_PCAP_STATUS_PL_DONE_MASK    0X00000008U

/* Register: CSU_PCAP_RESET */
#define CSU_PCAP_RESET    ((XILFPGA_CSU_BASEADDR) + 0X0000300CU)
#define CSU_PCAP_RESET_RESET_MASK    0X00000001U

/* Register: CSU_PCAP_CTRL */
#define CSU_PCAP_CTRL    ((XILFPGA_CSU_BASEADDR) + 0X00003008U)
#define CSU_PCAP_CTRL_PCAP_PR_MASK    0X00000001U

/**
 * Register: CSU_PCAP_RDWR
 */
#define CSU_PCAP_RDWR    ((XILFPGA_CSU_BASEADDR) + 0X00003004U)
#define CSU_PCAP_RDWR_PCAP_RDWR_B_SHIFT   0U

/* Register: CSU_PCAP_PROG */
#define CSU_PCAP_PROG    ((XILFPGA_CSU_BASEADDR) + 0X00003000U)
#define CSU_PCAP_PROG_PCFG_PROG_B_MASK    0X00000001U
#define CSU_PCAP_PROG_PCFG_PROG_B_SHIFT   0U

/* Register: PMU_GLOBAL for PL power-up */
#define PMU_GLOBAL_BASE			0xFFD80000U
#define PMU_GLOBAL_PWRUP_STATUS		(PMU_GLOBAL_BASE + 0x110U)
#define PMU_GLOBAL_PWRUP_EN		(PMU_GLOBAL_BASE + 0x118U)
#define PMU_GLOBAL_PWRUP_TRIG		(PMU_GLOBAL_BASE + 0x120U)
#define PMU_GLOBAL_PWR_PL_MASK		0x800000
#define PMU_GLOBAL_GEN_STORAGE5		(PMU_GLOBAL_BASE + 0x44U)

#define PMU_GLOBAL_ISO_INT_EN		(PMU_GLOBAL_BASE + 0X318U)
#define PMU_GLOBAL_ISO_TRIG		(PMU_GLOBAL_BASE + 0X320U)
#define PMU_GLOBAL_ISO_STATUS		(PMU_GLOBAL_BASE + 0X310U)

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

/* Error Codes */
#define XFPGA_ERROR_CSUDMA_INIT_FAIL		(0x2U)
#define XFPGA_ERROR_PL_POWER_UP			(0x3U)
#define XFPGA_ERROR_PL_ISOLATION		(0x4U)
#define XPFGA_ERROR_PCAP_INIT			(0x5U)
#define XFPGA_ERROR_BITSTREAM_LOAD_FAIL		(0x6U)
#define XFPGA_ERROR_CRYPTO_FLAGS		(0x7U)
#define XFPGA_ERROR_HDR_AUTH			(0X8U)
#define XFPGA_ENC_ISCOMPULSORY			(0x9U)
#define XFPGA_PARTITION_AUTH_FAILURE		(0xAU)
#define XFPGA_STRING_INVALID_ERROR		(0xBU)
#define XFPGA_ERROR_SECURE_CRYPTO_FLAGS		(0xCU)
#define XFPGA_ERROR_SECURE_MODE_EN		(0xDU)
#define XFPGA_HDR_NOAUTH_PART_AUTH		(0xEU)
#define XFPGA_DEC_WRONG_KEY_SOURCE		(0xFU)
#define XFPGA_ERROR_DDR_AUTH_VERIFY_SPK		(0x10U)
#define XFPGA_ERROR_DDR_AUTH_PARTITION		(0x11U)
#define XFPGA_ERROR_DDR_AUTH_WRITE_PL		(0x12U)
#define XFPGA_ERROR_OCM_AUTH_VERIFY_SPK		(0x13U)
#define XFPGA_ERROR_OCM_AUTH_PARTITION		(0x14U)
#define XFPGA_ERROR_OCM_REAUTH_WRITE_PL		(0x15U)
#define XFPGA_ERROR_PCAP_PL_DONE		(0x16U)
#define XFPGA_ERROR_AES_DECRYPT_PL		(0x17U)
#define XFPGA_ERROR_CSU_PCAP_TRANSFER		(0x18U)
#define XFPGA_ERROR_PLSTATE_UNKNOWN		(0x19U)
#define XFPGA_ERROR_BITSTREAM_FORMAT		(0x1AU)

/* PCAP Error Update Macro */
#define XFPGA_PCAP_ERR_MASK			(0xFF00U)
#define XFPGA_ERR_MODULE_MASK			(0xFFFF0000U)
#define XFPGA_PCAP_UPDATE_ERR(XfpgaPcapErr, ModuleErr)		\
		((ModuleErr << 16) & XFPGA_ERR_MODULE_MASK) + \
		((XfpgaPcapErr << 8) & XFPGA_PCAP_ERR_MASK)

#define XFPGA_STATE_MASK	0x00FF0000U
#define XFPGA_STATE_SHIFT	16
#define CFGREG_SRCDMA_OFFSET	0x8
#define CFGDATA_DSTDMA_OFFSET	0x1FC

/* FPGA Configuration Registers Offsets */
#define CRC		0  /* Status Register */
#define FAR		1  /* Frame Address Register */
#define FDRI		2  /* FDRI Register */
#define FDRO		3  /* FDRO Register */
#define CMD		4  /* Command Register */
#define CTL0		5  /* Control Register 0 */
#define MASK		6  /* MASK Register */
#define STAT		7  /* Status Register */
#define LOUT		8  /* LOUT Register */
#define COR0		9  /* Configuration Options Register 0 */
#define MFWR		10 /* MFWR Register */
#define CBC		11 /* CBC Register */
#define IDCODE		12 /* IDCODE Register */
#define AXSS		13 /* AXSS Register */
#define COR1		14 /* Configuration Options Register 1 */
#define WBSTAR		16 /* Warm Boot Start Address Register */
#define TIMER		17 /* Watchdog Timer Register */
#define BOOTSTS		22 /* Boot History Status Register */
#define CTL1		24 /* Control Register 1 */

/**************************** Type Definitions *******************************/
/**
 * Structure to store the PL Image details.
 * @BitstreamAddr	Linear memory Bitstream image base address.
 * @AddrPtr		Aes key address which is used for Decryption.
 * @ReadbackAddr	Address is the DMA buffer address to store the
 *			readback data.
 * @ConfigReg		Configuration register value to be returned
 * @NumFrames		The number of fpga configuration frames to read
 * @XSecure_ImageInfo	Used to store the secure image data.
 * @Flags		Flags are used to specify the type of Bitstream file.
 *			* BIT(0) - Bitstream type
 *                                     * 0 - Full Bitstream
 *                                     * 1 - Partial Bitstream
 *			* BIT(1) - Authentication using DDR
 *                                     * 1 - Enable
 *                                     * 0 - Disable
 *			* BIT(2) - Authentication using OCM
 *                                     * 1 - Enable
 *                                     * 0 - Disable
 *			* BIT(3) - User-key Encryption
 *                                     * 1 - Enable
 *                                     * 0 - Disable
 *			* BIT(4) - Device-key Encryption
 *                                     * 1 - Enable
 *                                     * 0 - Disable
 *
 */
typedef struct {
	UINTPTR BitstreamAddr;
	UINTPTR	AddrPtr;
	UINTPTR ReadbackAddr;
	u32 ConfigReg;
	u32 NumFrames;
	XSecure_ImageInfo SecureImageInfo;
	u32 Flags;
} XFpga_Info;

/************************** Variable Definitions *****************************/
extern XCsuDma CsuDma;  /* CSU DMA instance */

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
#endif  /* XILFPGA_PCAP_H */
/** @} */
