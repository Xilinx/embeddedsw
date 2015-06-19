/******************************************************************************
 *
 * (c) Copyright 2014 Xilinx, Inc. All rights reserved.
 *
 * This file contains confidential and proprietary information  of Xilinx, Inc.
 * and is protected under U.S. and  international copyright and other
 * intellectual property  laws.
 *
 * DISCLAIMER
 * This disclaimer is not a license and does not grant any  rights to the
 * materials distributed herewith. Except as  otherwise provided in a valid
 * license issued to you by  Xilinx, and to the maximum extent permitted by
 * applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND  WITH ALL
 * FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES  AND CONDITIONS, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING  BUT NOT LIMITED TO WARRANTIES OF
 * MERCHANTABILITY, NON-  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
 * and
 * (2) Xilinx shall not be liable (whether in contract or tort,  including
 * negligence, or under any other theory of liability) for any loss or damage
 * of any kind or nature  related to, arising under or in connection with these
 * materials, including for any direct, or any indirect,  special, incidental,
 * or consequential loss or damage  (including loss of data, profits,
 * goodwill, or any type of  loss or damage suffered as a result of any
 * action brought  by a third party) even if such damage or loss was
 * reasonably foreseeable or Xilinx had been advised of the  possibility
 * of the same.
 *
 * CRITICAL APPLICATIONS
 * Xilinx products are not designed or intended to be fail-  safe, or for use
 * in any application requiring fail-safe  performance, such as life-support
 * or safety devices or  systems, Class III medical devices, nuclear
 * facilities,  applications related to the deployment of airbags, or any
 * other applications that could lead to death, personal  injury, or severe
 * property or environmental damage  (individually and collectively,
 * "Critical  Applications"). Customer assumes the sole risk and  liability
 * of any use of Xilinx products in Critical  Applications, subject only to
 * applicable laws and  regulations governing limitations on product liability.
 *
 * THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS  PART
 * OF THIS FILE AT ALL TIMES.
 *
 ******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_hw.h
*
* This is the header file which contains definitions for the hardware
* interface of secure hardware devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ba   09/25/14 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XSECURE_HW_H
#define XSECURE_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xparameters.h"
#include "xil_types.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

#define XSECURE_CSU_REG_BASE_ADDR	(0xFFCA0000U)
					/**< CSU base address */
#define XSECURE_CSU_DMA_BASE		(0xFFC80000U)
					/**< CSUDMA base address */

#define XSECURE_CSU_SHA3_BASE	(XSECURE_CSU_REG_BASE_ADDR + 0x2000U)
					/**< SHA3 base address */
#define XSECURE_CSU_CTRL_REG	(XSECURE_CSU_REG_BASE_ADDR + 0x4U)
					/**< CSU control reg. */
#define XSECURE_CSU_SSS_BASE	(XSECURE_CSU_REG_BASE_ADDR + 0x8U)
					/**< CSU SSS base address */
#define XSECURE_CSU_AES_BASE	(XSECURE_CSU_REG_BASE_ADDR + 0x1000U)
					/**< CSU AES base address */
#define XSECURE_CSU_RSA_BASE	(0xFFCE0000U)
					/**< RSA reg. base address */
#define XSECURE_CSU_PCAP_STATUS (XSECURE_CSU_REG_BASE_ADDR + 0X00003010U)
					/**< CSU PCAP Status reg. */
#define XSECURE_CSU_PCAP_STATUS_PCAP_WR_IDLE_MASK    (0X00000001U)
					/**< PCAP Write Idle */

/** @name Register Map
 *
 * Register offsets for the SHA module.
 * @{
 */
#define XSECURE_CSU_SHA3_START_OFFSET	(0x00U) /**< SHA start message */
#define XSECURE_CSU_SHA3_RESET_OFFSET	(0x04U) /**< Reset Register */
#define XSECURE_CSU_SHA3_DONE_OFFSET	(0x08U) /**< SHA Done Register */

#define XSECURE_CSU_SHA3_DIGEST_0_OFFSET	(0x10U)
					/**< SHA3 Digest: Reg 0 */
#define XSECURE_CSU_SHA3_DIGEST_11_OFFSET	(0x34U)
					/**< SHA3 Digest: Last Register */
/* @} */

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
#define XSECURE_CSU_AES_KUP_WR_OFFSET	(0x1CU)
						/**< AES KUP Write Control */

#define XSECURE_CSU_AES_KUP_0_OFFSET	(0x20U)
						/**< AES Key Update 0 */
#define XSECURE_CSU_AES_KUP_1_OFFSET	(0x24U) /**< AES Key Update 1 */
#define XSECURE_CSU_AES_KUP_2_OFFSET	(0x28U) /**< AES Key Update 2 */
#define XSECURE_CSU_AES_KUP_3_OFFSET	(0x2CU) /**< AES Key Update 3 */
#define XSECURE_CSU_AES_KUP_4_OFFSET	(0x30U) /**< AES Key Update 4 */
#define XSECURE_CSU_AES_KUP_5_OFFSET	(0x34U) /**< AES Key Update 5 */
#define XSECURE_CSU_AES_KUP_6_OFFSET	(0x38U) /**< AES Key Update 6 */
#define XSECURE_CSU_AES_KUP_7_OFFSET	(0x3CU) /**< AES Key Update 7 */

#define XSECURE_CSU_AES_IV_0_OFFSET		(0x40U) /**< AES IV 0 */
#define XSECURE_CSU_AES_IV_1_OFFSET		(0x44U) /**< AES IV 1 */
#define XSECURE_CSU_AES_IV_2_OFFSET		(0x48U) /**< AES IV 2 */
#define XSECURE_CSU_AES_IV_3_OFFSET		(0x4CU) /**< AES IV 3 */
/* @} */


/** @name Register Map
 *
 * Register offsets for the RSA module.
 * @{
 */
#define XSECURE_CSU_RSA_WRITE_DATA_OFFSET	(0x00U)
						/**< RAM write data offset */
#define XSECURE_CSU_RSA_WRITE_ADDR_OFFSET	(0x04U)
						/**< RAM write address offset */
#define XSECURE_CSU_RSA_READ_DATA_OFFSET	(0x08U)
						/**< RAM data read offset */
#define XSECURE_CSU_RSA_READ_ADDR_OFFSET	(0x0CU)
						/**< RAM read offset */
#define XSECURE_CSU_RSA_CONTROL_OFFSET		(0x10U)
						/**< RSA Control Reg */

#define XSECURE_CSU_RSA_STATUS_OFFSET		(0x14U)
						/**< Status Register */

#define XSECURE_CSU_RSA_MINV0_OFFSET		(0x18U)
					/**< RSA MINV(Mod 32 Inverse) 0 */
#define XSECURE_CSU_RSA_MINV1_OFFSET		(0x1CU)
						/**< RSA MINV 1 */
#define XSECURE_CSU_RSA_MINV2_OFFSET		(0x20U) /**< RSA MINV 2 */
#define XSECURE_CSU_RSA_MINV3_OFFSET		(0x24U) /**< RSA MINV 3 */
#define XSECURE_CSU_RSA_ZERO_OFFSET		(0x28U) /**< RSA Zero offset */

#define XSECURE_CSU_RSA_WR_DATA_0_OFFSET	(0x2cU) /**< Write Data 0 */
#define XSECURE_CSU_RSA_WR_DATA_1_OFFSET	(0x30U) /**< Write Data 1 */
#define XSECURE_CSU_RSA_WR_DATA_2_OFFSET	(0x34U) /**< Write Data 2 */
#define XSECURE_CSU_RSA_WR_DATA_3_OFFSET	(0x38U) /**< Write Data 3 */
#define XSECURE_CSU_RSA_WR_DATA_4_OFFSET	(0x3cU) /**< Write Data 4 */
#define XSECURE_CSU_RSA_WR_DATA_5_OFFSET	(0x40U) /**< Write Data 5 */
#define XSECURE_CSU_RSA_WR_ADDR_OFFSET		(0x44U)
					/**< Write address in RSA RAM */

#define XSECURE_CSU_RSA_RD_DATA_0_OFFSET	(0x48U) /**< Read Data 0 */
#define XSECURE_CSU_RSA_RD_DATA_1_OFFSET	(0x4cU) /**< Read Data 1 */
#define XSECURE_CSU_RSA_RD_DATA_2_OFFSET	(0x50U) /**< Read Data 2 */
#define XSECURE_CSU_RSA_RD_DATA_3_OFFSET	(0x54U) /**< Read Data 3 */
#define XSECURE_CSU_RSA_RD_DATA_4_OFFSET	(0x58U) /**< Read Data 4 */
#define XSECURE_CSU_RSA_RD_DATA_5_OFFSET	(0x5cU) /**< Read Data 5 */
#define XSECURE_CSU_RSA_RD_ADDR_OFFSET		(0x60U)
						/**< Read address in RSA RAM */

/* @} */

/**************************** Type Definitions *******************************/

/* Definition for SSS reg Source bits. */
typedef enum
{
	XSECURE_CSU_SSS_SRC_PCAP	= 0x3U,	/**< SSS source is PCAP */
	XSECURE_CSU_SSS_SRC_SRC_DMA	= 0x5U,	/**< SSS source is DMA */
	XSECURE_CSU_SSS_SRC_AES		= 0xAU,	/**< SSS source is AES */
	XSECURE_CSU_SSS_SRC_PSTP	= 0xCU,	/**< SSS source is PSTP */
	XSECURE_CSU_SSS_SRC_NONE	= 0x0U,	/**< NO Source */
	XSECURE_CSU_SSS_SRC_MASK	= 0xFU	/**< Mask for SSS source */
}XSECURE_CSU_SSS_SRC;				/**< SSS source values */

/**
* Definition for SSS reg Destination bits.
*/
typedef enum
{
	XSECURE_CSU_SSS_PCAP_SHIFT = 0U,/**< Offset for destination PCAP */
	XSECURE_CSU_SSS_DMA_SHIFT = 4U,	/**< Offset for destination DMA */
	XSECURE_CSU_SSS_AES_SHIFT = 8U,	/**< Offset for destination AES */
	XSECURE_CSU_SSS_SHA_SHIFT = 12U,/**< Offset for destination SHA */
	XSECURE_CSU_SSS_PSTP_SHIFT = 16U/**< Offset for destination PSTP */
}XSECURE_CSU_SSS_DEST_SHIFT;		/**<.Offset for SSS destination.*/

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* Read a CSU register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of
*		the device.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XSecure_ReadReg(u32 BaseAddress, int RegOffset)
*
******************************************************************************/
#define XSecure_ReadReg(BaseAddress, RegOffset) \
				Xil_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
* Write a CSU register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of
*		the device.
* @param	RegisterValue is the value to be written to the register
*
* @return	None.
*
* @note		C-Style signature:
*			void XSecure_WriteReg(u32 BaseAddress, int RegOffset,
*			u16 RegisterValue)
*
******************************************************************************/
#define XSecure_WriteReg(BaseAddress, RegOffset, RegisterValue) \
			Xil_Out32((BaseAddress) + (RegOffset), (RegisterValue))

#define XSecure_In32(Addr)			Xil_In32(Addr)

#define XSecure_In64(Addr)			Xil_In64(Addr)

#define XSecure_Out32(Addr, Data)		Xil_Out32(Addr, Data)

#define XSecure_Out64(Addr, Data)		Xil_Out64(Addr, Data)

/**
* Definition for SSS inline functions
*/

static inline u32 XSecure_SssInputPcap(XSECURE_CSU_SSS_SRC Src)
{
    Src &= XSECURE_CSU_SSS_SRC_MASK;
    return (Src << XSECURE_CSU_SSS_PCAP_SHIFT);
}

/***************************************************************************/
/**
* Set the SSS configuration mask for a data transfer to DMA device
*
* @param	Src contains the bits for source device sending data to DMA.
*
* @return	None.
*
* @note		C-Style signature:
*			void XSecure_SssInputDstDma(XSECURE_CSU_SSS_SRC Src)
*
******************************************************************************/
static inline u32 XSecure_SssInputDstDma(XSECURE_CSU_SSS_SRC Src)
{
	Src &= XSECURE_CSU_SSS_SRC_MASK;
	return (Src << XSECURE_CSU_SSS_DMA_SHIFT);
}

/***************************************************************************/
/**
* Set the SSS configuration mask for a data transfer to AES device
*
* @param	Src contains the bits for AES source device
*
* @return	None.
*
* @note		C-Style signature:
*			void XSecure_SssInputAes(XSECURE_CSU_SSS_SRC Src)
*
******************************************************************************/
static inline u32 XSecure_SssInputAes(XSECURE_CSU_SSS_SRC Src)
{
	Src &= XSECURE_CSU_SSS_SRC_MASK;
	return (Src << XSECURE_CSU_SSS_AES_SHIFT);
}

/***************************************************************************/
/**
* Set the SSS configuration mask for a data transfer to SHA device
*
* @param	Src contains the bits for SHA source device
*
* @return	None.
*
* @note		C-Style signature:
*			void XSecure_SssInputSha3(XSECURE_CSU_SSS_SRC Src)
*
******************************************************************************/
static inline u32 XSecure_SssInputSha3(XSECURE_CSU_SSS_SRC Src)
{
	Src &= XSECURE_CSU_SSS_SRC_MASK;
	return (Src << XSECURE_CSU_SSS_SHA_SHIFT);
}

/***************************************************************************/
/**
* Set up the CSU Secure Stream Switch configuration
*
* @param	Cfg contains the 32 bit value to be written into SSS config
*			register
*
* @return	None.
*
* @note		C-Style signature:
*			void XSecure_SssSetup(u32 Cfg)
*
******************************************************************************/
static inline void XSecure_SssSetup(u32 Cfg)
{
	XSecure_Out32(XSECURE_CSU_SSS_BASE, Cfg);
}

/***************************************************************************/
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

/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif  /* XSECURE_HW_H */
