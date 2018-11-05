/******************************************************************************
*
* Copyright (C) 2015 - 18 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xfsbl_error.h
*
* This is the header file which contains error codes for the FSBL.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines
*            03/24/17 Added new error codes.
* 3.0   vns  09/08/17 Added error code for PPK revoke failure
* 4.0   vns  03/07/18 Added error codes for boot header authentication
*                     failure and for encryption compulsory
* 5.0   ka   04/10/18 Added error codes for user-efuse revocation
* 6.0   bkm  04/10/18 Added error codes for FMC_VADJ
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_ERROR_H
#define XFSBL_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/**
 * @name FSBL error codes description
 *
 * XXYY - Error code format
 *
 * YY - error code irrespective of stage
 *
 * XX -> x1 x2 x3 x4 x5 x6 x7 x8
 *
 * x1 -> FSBL / CSUROM error
 * x2x3 -> Stage at which error happened
 * 		   00 -> Error at stage 1
 * 		   01 -> Error at stage 2
 * 		   10 -> Error at stage 3
 * 		   11 -> Error at stage 4
 * x4x5x6 -> Error source for next 10 bits
 * 			 000 -> FSBL error code
 * 		     001 -> psu init failure
 * 		     010 -> psu postconfig failure
 * 		     011 -> Driver error code
 * x7x8 ->
 */

#define XFSBL_ERROR_STAGE_1		(0x0000U)
#define XFSBL_ERROR_STAGE_2		(u32)(0x2000U)
#define XFSBL_ERROR_STAGE_3		(u32)(0x4000U)
#define XFSBL_ERROR_STAGE_4		(u32)(0x6000U)

#define XFSBL_PSU_INIT_FAILED				(0x0800U)
#define XFSBL_PSU_INIT_COMPLETED			(0x1U)
#define XFSBL_PSU_POSTCONFIG_FAILED			(0x1000U)

#define XFSBL_SUCCESS					(u32)(0x0U)
#define XFSBL_STATUS_JTAG				(0x1U)
#define XFSBL_SUCCESS_NOT_PARTITION_OWNER		(0x2U)
#define XFSBL_STATUS_CONTINUE_PARTITION_LOAD	(0x3U)
#define XFSBL_STATUS_CONTINUE_OTHER_HANDOFF		(0x4U)
#define XFSBL_STATUS_SECONDARY_BOOT_MODE				(0x5U)

#define XFSBL_ERROR_UNSUPPORTED_BOOT_MODE		(0x6U)
#define XFSBL_WDT_INIT_FAILED				(0x7U)
#define XFSBL_INVALID_DEST_CPU			(0x8U)
#define XFSBL_ERROR_SYSTEM_WDT_RESET			(0x9U)

#define XFSBL_ERROR_QSPI_READ_ID			(0xAU)
#define XFSBL_ERROR_UNSUPPORTED_QSPI			(0xBU)
#define XFSBL_ERROR_QSPI_INIT				(0xCU)

#define XFSBL_ERROR_NO_OF_PARTITIONS			(0xDU)
#define XFSBL_ERROR_PPD		(0xEU)
#define XFSBL_ERROR_XIP_AUTH_ENC_PRESENT		(0xFU)
#define XFSBL_ERROR_XIP_EXEC_ADDRESS			(0x10U)
#define XFSBL_ERROR_PARTITION_LENGTH			(0x11U)
#define XFSBL_ERROR_INVALID_CHECKSUM_TYPE		(0x12U)
#define XFSBL_ERROR_INVALID_CPU_TYPE			(0x13U)
#define XFSBL_ERROR_LS_CPU_TYPE				(0x14U)
#define XFSBL_ERROR_INVALID_DEST_DEVICE			(0x15U)
#define XFSBL_ERROR_INVALID_LOAD_ADDRESS		(0x16U)
#define XFSBL_ERROR_PH_CHECKSUM_FAILED			(0x17U)
#define XFSBL_ERROR_PWR_UP_CPU				(0x18U)
#define XFSBL_ERROR_QSPI_LENGTH				(0x19U)
#define XFSBL_ERROR_INVALID_QSPI_CONNECTION		(s32)(0x1A)
#define XFSBL_ERROR_UNDEFINED_EXCEPTION			(0x1BU)
#define XFSBL_ERROR_SVC_EXCEPTION			(0x1CU)
#define XFSBL_ERROR_PREFETCH_ABORT_EXCEPTION		(0x1DU)
#define XFSBL_ERROR_DATA_ABORT_EXCEPTION		(0x1EU)
#define XFSBL_ERROR_IRQ_EXCEPTION			(0x1FU)
#define XFSBL_ERROR_FIQ_EXCEPTION			(0x20U)
#define XFSBL_ERROR_IHT_CHECKSUM			(0x21U)
#define XFSBL_ERROR_QSPI_READ				(0x22U)
#define XFSBL_ERROR_HANDOFF_CPUID			(0x23U)
#define XFSBL_ERROR_LOAD_ADDRESS			(0x24U)
#define XFSBL_ERROR_HOOK_BEFORE_BITSTREAM_DOWNLOAD	(0x25U)
#define XFSBL_ERROR_HOOK_AFTER_BITSTREAM_DOWNLOAD	(0x26U)
#define XFSBL_ERROR_HOOK_BEFORE_HANDOFF		        (0x27U)
#define XFSBL_ERROR_SD_INIT 			        (0x28U)
#define XFSBL_ERROR_SD_F_OPEN 			        (0x29U)
#define XFSBL_ERROR_SD_F_LSEEK			        (0x2AU)
#define XFSBL_ERROR_SD_F_READ			        (0x2BU)
#define XFSBL_ERROR_NAND_INIT			        (0x2CU)
#define XFSBL_ERROR_NAND_READ			        (0x2DU)
#define XFSBL_ERROR_ADDRESS				(0x2EU)
#define XFSBL_ERROR_SPK_RSA_DECRYPT			(0x2FU)
#define XFSBL_ERROR_SPK_SIGNATURE			(0x30U)
#define XFSBL_ERROR_PART_RSA_DECRYPT			(0x31U)
#define XFSBL_ERROR_PART_SIGNATURE			(0x32U)
#define XFSBL_ERROR_DDR_INIT_FAIL			(0x33U)

#define XFSBL_ERROR_CSUDMA_INIT_FAIL                (0x34U)
#define XFSBL_ERROR_DECRYPTION_IV_COPY_FAIL         (0x35U)
#define XFSBL_ERROR_DECRYPTION_FAIL                 (0x36U)
#define XFSBL_ERROR_BITSTREAM_LOAD_FAIL             (0x37U)
#define XFSBL_ERROR_BITSTREAM_GCM_TAG_MISMATCH      (0x38U)
#define XFSBL_ERROR_DECRYPTION_IMAGE_LENGTH_MISMATCH    (0x39U)
#define XFSBL_ERROR_BITSTREAM_DECRYPTION_FAIL       (0x3AU)
#define XFSBL_ERROR_SECURE_NOT_ENABLED              (0x3BU)

#define XFSBL_ERROR_PL_NOT_ENABLED                  (0x3DU)
#define XFSBL_ERROR_PL_POWER_UP						(0x3EU)
#define XFSBL_ERROR_A53_0_POWER_UP					(0x3FU)
#define XFSBL_ERROR_A53_1_POWER_UP					(0x40U)
#define XFSBL_ERROR_A53_2_POWER_UP					(0x41U)
#define XFSBL_ERROR_A53_3_POWER_UP					(0x42U)
#define XFSBL_ERROR_R5_0_POWER_UP					(0x43U)
#define XFSBL_ERROR_R5_1_POWER_UP					(0x44U)
#define XFSBL_ERROR_R5_L_POWER_UP					(0x45U)
#define XFSBL_ERROR_R5_0_TCM_POWER_UP				(0x46U)
#define XFSBL_ERROR_R5_1_TCM_POWER_UP				(0x47U)
#define XFSBL_ERROR_R5_L_TCM_POWER_UP				(0x48U)
#define XFSBL_ERROR_UNSUPPORTED_CLUSTER_ID			(0x49U)
#define XFSBL_ERROR_I2C_INIT						(0x4AU)
#define XFSBL_ERROR_I2C_WRITE						(0x4BU)
#define XFSBL_ERROR_DDR_ECC_INIT					(0x4CU)
#define XFSBL_ERROR_TCM_ECC_INIT					(0x4DU)
#define XFSBL_ERROR_UNSUPPORTED_HANDOFF				(0x4EU)
#define XFSBL_ERROR_PARTITION_CHECKSUM_FAILED		(0x4FU)
#define XFSBL_ERROR_PM_INIT					(0x50U)
#define XFSBL_ERROR_PROTECTION_CFG					(0x51U)
#define XFSBL_ERROR_UNAVAILABLE_CPU					(0x52U)
#define XFSBL_ERROR_GT_LANE_SELECTION				(0x53U)
#define XFSBL_ERROR_I2C_SET_SCLK                    (0x54U)
#define XFSBL_ERROR_PMU_GLOBAL_REQ_ISO				(0x55U)
#define XFSBL_ERROR_AES_INITIALIZE             		(0x56U)
#define XFSBL_ERROR_QSPI_MANUAL_START     			(0x57U)
#define XFSBL_ERROR_QSPI_PRESCALER_CLK              (0x58U)
#define XFSBL_ERROR_RSA_INITIALIZE                  (0x59U)
#define XFSBL_ERROR_CHUNK_HASH_COMPARISON			(0x60U)
#define XFSBL_ERROR_PROVIDED_BUF_HASH_STORE			(0x61U)
#define XFSBL_ERROR_USB_BOOT_WITH_NO_DDR			(0x62U)
#define XFSBL_ERROR_BLOCK_SIZE_SEC_BS				(0x63U)
#define XFSBL_ERROR_IMAGE_HEADER_ACOFFSET			(0x64U)
#define XFSBL_ERROR_PPK_VERIFICATION				(0x65U)
#define XFSBL_ERROR_SPKID_VERIFICATION				(0x66U)
#define XSFBL_ERROR_PPK_SELECT_ISREVOKED			(0x67U)
#define XFSBL_ERROR_BH_RSA_DECRYPT				(0x68U)
#define XFSBL_ERROR_BH_SIGNATURE				(0x69U)
#define XFSBL_ERROR_BH_AUTH_IS_NOTALLOWED			(0x70U)
#define XFSBL_ERROR_ENC_IS_MANDATORY				(0x71U)
#define XFSBL_ERROR_USER_EFUSE_ISREVOKED			(0x72U)
#define XFSBL_ERROR_OUT_OF_RANGE_USER_EFUSE			(0x73U)
#define XFSBL_ERROR_INVALID_EFUSE_SELECT			(0x74U)
#define XSFBL_ERROR_FMC_ENABLE						(0x75U)
#define XSFBL_EEPROM_PRESENT						(0x76U)
#define XFSBL_FAILURE					(0x3FFFFFFFU)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_ERROR_H */
