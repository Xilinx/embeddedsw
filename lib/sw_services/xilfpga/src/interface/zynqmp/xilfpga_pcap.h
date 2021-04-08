/******************************************************************************
* Copyright (c) 2018 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilfpga_pcap.h
 *
 * The XILFPGA library provides the interface to the application to configure
 * the programmable logic (PL) though the PS.
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date        Changes
 * ----- ----   -------- -------------------------------------------------------
 * 1.0   Nava   06/08/16 Initial release
 * 1.1   Nava   11/16/16 Added PL power-up sequence.
 * 2.0   Nava   01/10/17 Added Encrypted bitstream loading support.
 * 2.0   Nava   02/16/17 Added Authenticated bitstream loading support.
 * 2.1   Nava   05/06/17 Correct the check logic issues in
 *                       XFpga_PL_BitStream_Load()
 *                       to avoid the unwanted blocking conditions.
 * 3.0   Nava   05/12/17 Added PL configuration registers readback support.
 * 4.0   Nava   02/08/18 Added Authenticated and Encypted Bitstream
			 loading support.
 * 4.0   Nava   03/02/18 Added the legacy bit file loading feature support
 *			 from U-boot.and improve the error handling support
 *			 by returning the proper ERROR value upon error
 *			 conditions.
 * 4.1   Nava   03/27/18 For Secure Bitstream loading to avoid the Security
 *			 violations Need to Re-validate the User Crypto flags
 *			 with the Image Crypto operation by using the internal
 *			 memory.To Fix this added a new API
 *			 XFpga_ReValidateCryptoFlags().
 * 4.1   Nava   04/16/18 Added partial bitstream loading support.
 * 4.2   Nava   06/08/16 Refactor the xilfpga library to support
 *                       different PL programming Interfaces.
 * 4.2   adk    07/11/18 Added support for readback of PL configuration data.
 * 4.2   Nava   07/22/18 Added XFpga_SelectEndianess() new API to Support
 *                       programming the vivado generated .bit and .bin files
 * 4.2   adk    08/03/18 Added example for partial reconfiguration.
 * 4.2   Nava   08/16/18 Modified the PL data handling Logic to support
 *                       different PL programming interfaces.
 * 4.2   Nava   09/15/18 Fixed global function call-backs issue.
 * 5.0   Div    01/21/19 Fixed misra-c required standard violations.
 * 5.0   Nava   02/06/19 Remove redundant API's from the interface agnostic layer
 *                       and make the existing API's generic to support both
 *                       ZynqMP and versal platforms.
 * 5.0   Nava   02/26/19 Fix for power-up PL issue with pmufw.
 * 5.0   Nava   02/26/19 Update the data handling logic to avoid the code
 *                       duplication
 * 5.0   Nava   02/28/19 Handling all the 4 PS-PL resets irrespective of the
 *                       design configuration.
 * 5.0   Nava   03/21/19 Added Address alignment check. As CSUDMA expects word
 *		         aligned address. In case user passes an unaligned
 *		         address return error.
 * 5.0   sne    03/27/19 Fixed misra-c violations.
 * 5.0   Nava   04/23/19 Optimize the API's logic to avoid code duplication.
 * 5.2   Nava   12/18/19 Fix for security violation in the readback path.
 * 5.2   Nava   02/14/20 Added Bitstream loading support by using IPI services.
 * 5.3   Nava   06/16/20 Modified the date format from dd/mm to mm/dd.
 * 6.0   Nava   01/07/21 Fixed misra-c required standard violations.
 * 6.0   Nava   02/22/21 Fixed doxygen issues.
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

#ifndef XILFPGA_PCAP_H
#define XILFPGA_PCAP_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xcsudma.h"
#include "xsecure.h"
#include "xil_util.h"

/************************** Constant Definitions *****************************/
/**
 *@cond nocomments
 */
#define PL_DONE_POLL_COUNT  300000U
#define PL_RESET_PERIOD_IN_US  1U



/* Dummy address to indicate that destination is PCAP */
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
#define PCAP_STATUS_PCAP_WR_IDLE_MASK    0X00000001U
#define PCAP_STATUS_PCAP_RD_IDLE_MASK    0X00000002U
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
#define PMU_GLOBAL_PWR_PL_MASK		0x800000U
#define PMU_GLOBAL_GEN_STORAGE5		(PMU_GLOBAL_BASE + 0x44U)

#define PMU_GLOBAL_ISO_INT_EN		(PMU_GLOBAL_BASE + 0X318U)
#define PMU_GLOBAL_ISO_TRIG		(PMU_GLOBAL_BASE + 0X320U)
#define PMU_GLOBAL_ISO_STATUS		(PMU_GLOBAL_BASE + 0X310U)
#define PMU_GLOBAL_ISO_NONPCAP_MASK	0X00000004U

#define GPIO_DIRM_5_EMIO		0xFF0A0344U
#define GPIO_MASK_DATA_5_MSW	0xFF0A002CU
#define GPIO_PS_PL_DIRM_MASK	0xF0000000U
#define GPIO_LOW_DATA_MSW_VAL	0x0FFF0000U
#define GPIO_HIGH_DATA_MSW_VAL	0x0FFFF000U

/* Register: PCAP_CLK_CTRL Address */
#define PCAP_CLK_CTRL		0xFF5E00A4U
#define PCAP_CLK_EN_MASK	0x01000000U

/* AES KEY SRC Info */
#define XFPGA_KEY_SRC_EFUSE_RED		0xA5C3C5A3U
#define XFPGA_KEY_SRC_BBRAM_RED		0x3A5C3C5AU
#define XFPGA_KEY_SRC_EFUSE_BLK		0xA5C3C5A5U
#define XFPGA_KEY_SRC_BH_BLACK		0xA35C7C53U
#define XFPGA_KEY_SRC_EFUSE_GRY		0xA5C3C5A7U
#define XFPGA_KEY_SRC_BH_GRY		0xA35C7CA5U
#define XFPGA_KEY_SRC_KUP		0xA3A5C3C5U

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
#define XFPGA_ERROR_UNALIGN_ADDR		(0x1BU)
#define XFPGA_ERROR_AES_INIT			(0x1CU)
#define XFPGA_ERROR_EFUSE_CHECK 		(0x1DU)

/* PCAP Error Update Macro */
#define XFPGA_PCAP_ERR_MASK			(0xFF00U)
#define XFPGA_ERR_MODULE_MASK			(0xFFFF0000U)
#define XFPGA_PCAP_UPDATE_ERR(XfpgaPcapErr, ModuleErr)		\
		(((ModuleErr) << (u32)16U) & XFPGA_ERR_MODULE_MASK) + \
		(((XfpgaPcapErr) << (u32)8U) & XFPGA_PCAP_ERR_MASK)

#define XFPGA_STATE_MASK	0x00FF0000U
#define XFPGA_STATE_SHIFT	16U

/**************************** Type Definitions *******************************/
/**
 * Structure to store the PL encrypted Image details
 *
 * @param SecureAes Used to store AES initialization info.
 * @param NextBlkLen Used to store the next encrypted block size info.
 */
typedef struct {
	XSecure_Aes *SecureAes;	/* AES initialized structure */
	u32 NextBlkLen;		/* Not required for user, used
				 * for storing next block size
				 */
} XFpgaPs_PlEncryption;

/**
 * Structure to store Encryption parameters info
 *
 * @param PlEncrypt Used to store the PL encrypted Image details
 * @param SecureHdr Used to store the Secure header
 * @param Hdr Size of the secure header.
 * @param SssInstance Used to store the secure switch instance info
 */
typedef struct {
	XFpgaPs_PlEncryption PlEncrypt;	/* Encryption parameters */
	u8 SecureHdr[XSECURE_SECURE_HDR_SIZE + XSECURE_SECURE_GCM_TAG_SIZE];
	u32 Hdr;
	XSecure_Sss SssInstance;
} XFpgaPs_PlPartition;

/**
 * Structure to store the PL Image details.
 *
 * @param Secure_ImageInfo	Used to store the secure image data.
 * @param PlAesInfo used to store the encrypted image data.
 * @param Secure_Aes The AES-GCM driver instance data structure
 * @param TotalBitPartCount Used to store the number of Authenticated partitions info.
 * @param SecureOcmState Used to Preserve the initialization states for the OCM
 *                 use cases.
 * @param RemaningBytes used to preserve the remaining byte to process Authenticated
 *                bitstream Images.
 * @param AcPtr Used to Access the authenticate certificate buffer address
 * @param BitAddr Used to Access the Bitstream buffer Address.
 */
typedef struct {
	XSecure_ImageInfo SecureImageInfo;
	XFpgaPs_PlPartition PlAesInfo;
	XSecure_Aes Secure_Aes;
	u32 TotalBitPartCount;
	u32 SecureOcmState;
	u32 RemaningBytes;
	UINTPTR AcPtr;
	UINTPTR BitAddr;
} XFpga_Info;

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/*****************************************************************************/
/* @endcond */

#ifdef __cplusplus
}
#endif

#endif  /* XILFPGA_PCAP_H */
/** @} */
