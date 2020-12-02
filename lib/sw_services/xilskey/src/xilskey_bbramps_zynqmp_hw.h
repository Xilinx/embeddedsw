/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xilskey_bbramps_zynqmp_hw.h
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx ZynqMp BBRAM controller.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.0   vns     10/08/15 First release
* </pre>
*
******************************************************************************/

#ifndef __XSK_BBRAMPS_ZYNQMP_HW_H__
#define __XSK_BBRAMPS_ZYNQMP_HW_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xilskey_utils.h"
#include "xil_io.h"
#include "xilskey_bbram.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Bbram Base Address
 * @{
 */
#define XSK_ZYNQMP_BBRAM_BASEADDR	0xFFCD0000U /**< Bbram base address */
/*@}*/

/** @name Register: BbramSts
 * @{
 */
#define XSK_ZYNQMP_BBRAM_STS_OFFSET	0x00000000U	/**< Status
							  *  register offset */
#define XSK_ZYNQMP_BBRAM_STS_RSTVAL	0x00000000U	/**< Reset value */

#define XSK_ZYNQMP_BBRAM_STS_AES_CRC_PASS_SHIFT		9U
							/**< AES crc pass shift
							  */
#define XSK_ZYNQMP_BBRAM_STS_AES_CRC_PASS_WIDTH		1U
							/**< AES crc pass width
							  */
#define XSK_ZYNQMP_BBRAM_STS_AES_CRC_PASS_MASK		0x00000200U
							/**< AES crc pass
							  * mask */
#define XSK_ZYNQMP_BBRAM_STS_AES_CRC_PASS_DEFVAL	0x0U
							/**< AES crc pass
							  *  default value */

#define XSK_ZYNQMP_BBRAM_STS_AES_CRC_DONE_SHIFT		8U
							/**< AES CRC done
							  *  shift */
#define XSK_ZYNQMP_BBRAM_STS_AES_CRC_DONE_WIDTH		1U
							/**< AES CRC done
							  *  width */
#define XSK_ZYNQMP_BBRAM_STS_AES_CRC_DONE_MASK		0x00000100U
							/**< AES CRC done
							  *  mask */
#define XSK_ZYNQMP_BBRAM_STS_AES_CRC_DONE_DEFVAL	0x0U
							/**< AES CRC done
							  *  default value */

#define XSK_ZYNQMP_BBRAM_STS_ZEROIZED_SHIFT		4U
							/**< Bbram zeroised
							  *  shift */
#define XSK_ZYNQMP_BBRAM_STS_ZEROIZED_WIDTH		1U
							/**< Bbram zeroised
							  *  width */
#define XSK_ZYNQMP_BBRAM_STS_ZEROIZED_MASK		0x00000010U
							/**< Bbram zeroised
							  *  mask */
#define XSK_ZYNQMP_BBRAM_STS_ZEROIZED_DEFVAL		0x0U
							/**< Bbram zeroised
							  *  default value */

#define XSK_ZYNQMP_BBRAM_STS_PGM_MODE_SHIFT		0U
							/**< Bbram prgrmg mode
							  *  shift */
#define XSK_ZYNQMP_BBRAM_STS_PGM_MODE_WIDTH		1U
							/**< Bbram prgrmg mode
							  *  width */
#define XSK_ZYNQMP_BBRAM_STS_PGM_MODE_MASK		0x00000001U
							/**< Bbram prgrmg mode
							  *  mask */
#define XSK_ZYNQMP_BBRAM_STS_PGM_MODE_DEFVAL		0x0U
							/**< Bbram prgrmg mode
							  *  default value */
/*@}*/

/** @name Register: BbramCtrl
 * @{
 */
#define XSK_ZYNQMP_BBRAM_CTRL_OFFSET		0x00000004U
						/**< Control reg offset */
#define XSK_ZYNQMP_BBRAM_CTRL_RSTVAL		0x00000000U
						/**< Control reg reset value */
#define XSK_ZYNQMP_BBRAM_CTRL_ZEROIZE_SHIFT	0U
						/**< Control reg zeroise shift */
#define XSK_ZYNQMP_BBRAM_CTRL_ZEROIZE_WIDTH	1U
						/**< Control reg zeroise width */
#define XSK_ZYNQMP_BBRAM_CTRL_ZEROIZE_MASK	0x00000001U
						/**< Control reg zeroise mask */
#define XSK_ZYNQMP_BBRAM_CTRL_ZEROIZE_DEFVAL	0x0U
						/**< Control reg default value*/
/*@}*/

/** @name Register: BbramPgmMode
 * @{
 */
#define XSK_ZYNQMP_BBRAM_PGM_MODE_OFFSET	0x00000008U
						/**< Programming mode offset */
#define XSK_ZYNQMP_BBRAM_PGM_MODE_RSTVAL	0x00000000U
						/**< prgrmg mode reset value */
#define XSK_ZYNQMP_BBRAM_PGM_MODE_VAL_WIDTH	32U
						/**< prgrmg mode value width */
#define XSK_ZYNQMP_BBRAM_PGM_MODE_VAL_MASK	0xffffffffU
						/**< prgrmg mode value mask */
#define XSK_ZYNQMP_BBRAM_PGM_MODE_VAL_DEFVAL	0x0U
						/**< prgrmg mode default val */
#define XSK_ZYNQMP_BBRAM_PGM_MODE_SET_VAL	0x757BDF0D
						/**< prgrmg mode set value */
/*@}*/

/** @name Register: BbramAesCrc
 * @{
 */
#define XSK_ZYNQMP_BBRAM_AES_CRC_OFFSET		0x0000000CU
						/**< AES's CRC offset */
#define XSK_ZYNQMP_BBRAM_AES_CRC_RSTVAL		0x00000000U
						/**< AES's CRC reset val */
#define XSK_ZYNQMP_BBRAM_AES_CRC_VAL_SHIFT	0U
						/**< AES's CRC val shift */
#define XSK_ZYNQMP_BBRAM_AES_CRC_VAL_WIDTH	32U
						/**< AES's CRC val width */
#define XSK_ZYNQMP_BBRAM_AES_CRC_VAL_MASK	0xffffffffU
						/**< AES's CRC val mask */
#define XSK_ZYNQMP_BBRAM_AES_CRC_VAL_DEFVAL	0x0U
						/**< AES's CRC default val */
/*@}*/

/** @name Register: Bbram0
 * @{
 */
#define XSK_ZYNQMP_BBRAM_0_OFFSET	0x00000010U /**< Bbram0 offset */
#define XSK_ZYNQMP_BBRAM_0_RSTVAL	0x00000000U /**< Bbram0 rst value */
#define XSK_ZYNQMP_BBRAM_0_DATA_SHIFT		0U  /**< Bbram0 data shift */
#define XSK_ZYNQMP_BBRAM_0_DATA_WIDTH		32U /**< Bbram0 data width */
#define XSK_ZYNQMP_BBRAM_0_DATA_MASK	0xffffffffU /**< Bbram0 data mask */
#define XSK_ZYNQMP_BBRAM_0_DATA_DEFVAL		0x0U/**< Bbram0 def value */
/*@}*/

/** @name Register: Bbram1
 * @{
 */
#define XSK_ZYNQMP_BBRAM_1_OFFSET	0x00000014U /**< Bbram1 offset */
#define XSK_ZYNQMP_BBRAM_1_RSTVAL	0x00000000U /**< Bbram1 rst value */

#define XSK_ZYNQMP_BBRAM_1_DATA_SHIFT		0U /**< Bbram1 data shift */
#define XSK_ZYNQMP_BBRAM_1_DATA_WIDTH		32U /**< Bbram1 data width */
#define XSK_ZYNQMP_BBRAM_1_DATA_MASK	0xffffffffU /**< Bbram1 data mask */
#define XSK_ZYNQMP_BBRAM_1_DATA_DEFVAL		0x0U/**< Bbram1 def value */
/*@}*/

/** @name Register: Bbram2
 * @{
 */
#define XSK_ZYNQMP_BBRAM_2_OFFSET	0x00000018U /**< Bbram2 offset */
#define XSK_ZYNQMP_BBRAM_2_RSTVAL	0x00000000U /**< Bbram2 rst value */

#define XSK_ZYNQMP_BBRAM_2_DATA_SHIFT		0U  /**< Bbram2 data shift */
#define XSK_ZYNQMP_BBRAM_2_DATA_WIDTH		32U /**< Bbram2 data width */
#define XSK_ZYNQMP_BBRAM_2_DATA_MASK	0xffffffffU /**< Bbram2 data mask */
#define XSK_ZYNQMP_BBRAM_2_DATA_DEFVAL		0x0U/**< Bbram2 def value */
/*@}*/

/** @name Register: Bbram3
 * @{
 */
#define XSK_ZYNQMP_BBRAM_3_OFFSET	0x0000001CU/**< Bbram3 offset */
#define XSK_ZYNQMP_BBRAM_3_RSTVAL	0x00000000U/**< Bbram3 rst value */

#define XSK_ZYNQMP_BBRAM_3_DATA_SHIFT		0U /**< Bbram3 data shift */
#define XSK_ZYNQMP_BBRAM_3_DATA_WIDTH		32U/**< Bbram3 data width */
#define XSK_ZYNQMP_BBRAM_3_DATA_MASK	0xffffffffU/**< Bbram3 data mask */
#define XSK_ZYNQMP_BBRAM_3_DATA_DEFVAL		0x0U/**< Bbram3 def value */
/*@}*/

/** @name Register: Bbram4
 * @{
 */
#define XSK_ZYNQMP_BBRAM_4_OFFSET	0x00000020U/**< Bbram4 offset */
#define XSK_ZYNQMP_BBRAM_4_RSTVAL	0x00000000U/**< Bbram4 rst value */

#define XSK_ZYNQMP_BBRAM_4_DATA_SHIFT		0U /**< Bbram4 data shift */
#define XSK_ZYNQMP_BBRAM_4_DATA_WIDTH		32U/**< Bbram4 data width */
#define XSK_ZYNQMP_BBRAM_4_DATA_MASK	0xffffffffU/**< Bbram4 data mask */
#define XSK_ZYNQMP_BBRAM_4_DATA_DEFVAL		0x0U/**< Bbram4 def value */
/*@}*/

/** @name Register: Bbram5
 * @{
 */
#define XSK_ZYNQMP_BBRAM_5_OFFSET	0x00000024U/**< Bbram5 offset */
#define XSK_ZYNQMP_BBRAM_5_RSTVAL	0x00000000U/**< Bbram5 rst value */

#define XSK_ZYNQMP_BBRAM_5_DATA_SHIFT		0U /**< Bbram5 data shift */
#define XSK_ZYNQMP_BBRAM_5_DATA_WIDTH		32U/**< Bbram5 data width */
#define XSK_ZYNQMP_BBRAM_5_DATA_MASK	0xffffffffU/**< Bbram5 data mask */
#define XSK_ZYNQMP_BBRAM_5_DATA_DEFVAL		0x0U/**< Bbram5 def value */
/*@}*/

/** @name Register: Bbram6
 * @{
 */
#define XSK_ZYNQMP_BBRAM_6_OFFSET	0x00000028U/**< Bbram6 offset */
#define XSK_ZYNQMP_BBRAM_6_RSTVAL	0x00000000U/**< Bbram6 rst value */

#define XSK_ZYNQMP_BBRAM_6_DATA_SHIFT		0U /**< Bbram6 data shift */
#define XSK_ZYNQMP_BBRAM_6_DATA_WIDTH		32U/**< Bbram6 data width */
#define XSK_ZYNQMP_BBRAM_6_DATA_MASK	0xffffffffU/**< Bbram6 data mask */
#define XSK_ZYNQMP_BBRAM_6_DATA_DEFVAL		0x0U/**< Bbram6 def value */
/*@}*/

/** @name Register: Bbram7
 * @{
 */
#define XSK_ZYNQMP_BBRAM_7_OFFSET	0x0000002CU/**< Bbram7 offset */
#define XSK_ZYNQMP_BBRAM_7_RSTVAL	0x00000000U/**< Bbram7 rst value */

#define XSK_ZYNQMP_BBRAM_7_DATA_SHIFT		0U /**< Bbram7 data shift */
#define XSK_ZYNQMP_BBRAM_7_DATA_WIDTH		32U/**< Bbram7 data width */
#define XSK_ZYNQMP_BBRAM_7_DATA_MASK	0xffffffffU/**< Bbram7 data mask */
#define XSK_ZYNQMP_BBRAM_7_DATA_DEFVAL		0x0U /**< Bbram7 def value */
/*@}*/

/** @name Register: Bbram8
 * @{
 */
#define XSK_ZYNQMP_BBRAM_8_OFFSET	0x00000030U/**< Bbram8 offset */
#define XSK_ZYNQMP_BBRAM_8_RSTVAL	0x00000000U/**< Bbram8 rst value */

#define XSK_ZYNQMP_BBRAM_8_DATA_SHIFT		0U /**< Bbram8 data shift */
#define XSK_ZYNQMP_BBRAM_8_DATA_WIDTH		32U/**< Bbram8 data width */
#define XSK_ZYNQMP_BBRAM_8_DATA_MASK	0xffffffffU/**< Bbram8 data mask */
#define XSK_ZYNQMP_BBRAM_8_DATA_DEFVAL		0x0U/**< Bbram8 def value */
/*@}*/

/** @name Register: BbramSlverr
 * @{
 */
#define XSK_ZYNQMP_BBRAM_SLVERR_OFFSET	0x00000034U	/**< Slave error control
							  *  offset */
#define XSK_ZYNQMP_BBRAM_SLVERR_RSTVAL	0x00000000U	/**< Slave error reg
							  *  reset value */

#define XSK_ZYNQMP_BBRAM_SLVERR_EN_SHIFT	0x0U	/**< Slave error
							  *  enable shift */
#define XSK_ZYNQMP_BBRAM_SLVERR_EN_WIDTH	0x1U	/**< Slave error
							  *  enable width */
#define XSK_ZYNQMP_BBRAM_SLVERR_EN_MASK	0x00000001U	/**< Slave error
							  *  enable mask */
#define XSK_ZYNQMP_BBRAM_SLVERR_EN_DEFVAL	0x0U	/**< Slave error
							  *  enable def value*/
/*@}*/

/** @name Register: BbramIsr
 * @{
 */
#define XSK_ZYNQMP_BBRAM_ISR_OFFSET		0x00000038U
							/**< ISR offset */
#define XSK_ZYNQMP_BBRAM_ISR_RSTVAL		0x00000000U
							/**< ISR reset value */

#define XSK_ZYNQMP_BBRAM_ISR_APB_SLVERR_SHIFT		0x0U
							/**< ISR APB slave err
							  *  shift */
#define XSK_ZYNQMP_BBRAM_ISR_APB_SLVERR_WIDTH		0x1U
							/**< ISR APB slave err
							  *  width */
#define XSK_ZYNQMP_BBRAM_ISR_APB_SLVERR_MASK	0x00000001U
							/**< ISR APB slave err
							  *  maks */
#define XSK_ZYNQMP_BBRAM_ISR_APB_SLVERR_DEFVAL		0x0U
							/**< ISR APB slave err
							  *  def value */
/*@}*/

/** @name Register: BbramImr
 * @{
 */
#define XSK_ZYNQMP_BBRAM_IMR_OFFSET		0x0000003CU
							/**< IMR offset */
#define XSK_ZYNQMP_BBRAM_IMR_RSTVAL		0x00000001U
							/**< IMR reset value */
#define XSK_ZYNQMP_BBRAM_IMR_APB_SLVERR_SHIFT		0x0U
							/**< IMR APB slave err
							  *  shift */
#define XSK_ZYNQMP_BBRAM_IMR_APB_SLVERR_WIDTH		0x1U
							/**< IMR APB slave err
							  *  width */
#define XSK_ZYNQMP_BBRAM_IMR_APB_SLVERR_MASK	0x00000001U
							/**< IMR APB slave err
							  *  maks */
#define XSK_ZYNQMP_BBRAM_IMR_APB_SLVERR_DEFVAL		0x1U
							/**< IMR APB slave err
							  *  def value */
/*@}*/

/** @name Register: BbramIer
 * @{
 */
#define XSK_ZYNQMP_BBRAM_IER_OFFSET		0x00000040U
							/**< IER offset */
#define XSK_ZYNQMP_BBRAM_IER_RSTVAL		0x00000000U
							/**< IER reset value */
#define XSK_ZYNQMP_BBRAM_IER_APB_SLVERR_SHIFT		0U
							/**< IER APB slave err
							  *  shift */
#define XSK_ZYNQMP_BBRAM_IER_APB_SLVERR_WIDTH		1U
							/**< IER APB slave err
							  *  width */
#define XSK_ZYNQMP_BBRAM_IER_APB_SLVERR_MASK	0x00000001U
							/**< IER APB slave err
							  *  mask */
#define XSK_ZYNQMP_BBRAM_IER_APB_SLVERR_DEFVAL		0x0U
							/**< IER APB slave err
							  *  def value */
/*@}*/

/** @name Register: BbramIdr
 * @{
 */
#define XSK_ZYNQMP_BBRAM_IDR_OFFSET		0x00000044U
							/**< IDR offset */
#define XSK_ZYNQMP_BBRAM_IDR_RSTVAL		0x00000000U
							/**< IDR reset value */
#define XSK_ZYNQMP_BBRAM_IDR_APB_SLVERR_SHIFT		0x0U
							/**< IDR APB slave err
							  *  shift */
#define XSK_ZYNQMP_BBRAM_IDR_APB_SLVERR_WIDTH		0x1U
							/**< IDR APB slave err
							  *  width */
#define XSK_ZYNQMP_BBRAM_IDR_APB_SLVERR_MASK	0x00000001U
							/**< IDR APB slave err
							  *  mask */
#define XSK_ZYNQMP_BBRAM_IDR_APB_SLVERR_DEFVAL		0x0U
							/**< IDR APB slave err
							  *  def value */
/*@}*/

/***************** Macros (Inline Functions) Definitions ********************/

#ifdef __cplusplus
}
#endif

#endif /* __XSK_BBRAMPS_ZYNQMP_HW__ */
