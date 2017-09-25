/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* @file xrfdc_hw.h
* @addtogroup rfdc_v2_0
* @{
*
* This header file contains the identifiers and basic HW access driver
* functions (or  macros) that can be used to access the device. Other driver
* functions are defined in xrfdc.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---    -------- -----------------------------------------------
* 1.0   sk     05/16/17 Initial release
* 2.1   sk     09/15/17 Remove Libmetal library dependency for MB.
*       sk     09/21/17 Add support for Over voltage and Over
*                       Range interrupts.
* </pre>
*
******************************************************************************/

#ifndef RFDC_HW_H_
#define RFDC_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#ifdef __BAREMETAL__
#include "xil_io.h"
#endif
#include "xparameters.h"
#ifndef __MICROBLAZE__
#include "metal/io.h"
#endif
/************************** Constant Definitions *****************************/

/** @name Register Map
 *
 * Register offsets from the base address of an RFDC ADC and DAC device.
 * @{
 */

#define XRFDC_CLK_EN_OFFSET	0x000U	/**< ADC Clock Enable
							Register */
#define XRFDC_ADC_DEBUG_RST_OFFSET	0x004U	/**< ADC Debug Reset
							Register */
#define XRFDC_ADC_FABRIC_RATE_OFFSET	0x008U	/**< ADC Fabric Rate
							Register */
#define XRFDC_ADC_FABRIC_OFFSET	0x00CU	/**< ADC Fabric Register */
#define XRFDC_ADC_FABRIC_ISR_OFFSET	0x010U	/**< ADC Fabric ISR
							Register */
#define XRFDC_DAC_FABRIC_ISR_OFFSET	0x014U	/**< DAC Fabric ISR
							Register */
#define XRFDC_ADC_FABRIC_IMR_OFFSET	0x014U	/**< ADC Fabric IMR
							Register */
#define XRFDC_DAC_FABRIC_IMR_OFFSET	0x018U	/**< DAC Fabric IMR
							Register */
#define XRFDC_ADC_FABRIC_DBG_OFFSET	0x018U	/**< ADC Fabric Debug
							Register */
#define XRFDC_ADC_UPDATE_DYN_OFFSET	0x01CU	/**< ADC Update Dynamic
							Register */
#define XRFDC_DAC_UPDATE_DYN_OFFSET	0x020U	/**< DAC Update Dynamic
							Register */
#define XRFDC_ADC_FIFO_LTNC_CRL_OFFSET	0x020U	/**< ADC FIFO Latency
							Control Register */
#define XRFDC_ADC_DEC_ISR_OFFSET	0x030U	/**< ADC Decoder interface
							ISR Register */
#define XRFDC_ADC_DEC_IMR_OFFSET	0x034U	/**< ADC Decoder interface
							IMR Register */
#define XRFDC_DATPATH_ISR_OFFSET	0x038U	/**< ADC Data Path
							ISR Register */
#define XRFDC_DATPATH_IMR_OFFSET	0x03CU	/**< ADC Data Path
							IMR Register */
#define XRFDC_ADC_DECI_CONFIG_OFFSET	0x040U	/**< ADC Decimation
							Config Register */
#define XRFDC_DAC_INTERP_CTRL_OFFSET	0x040U	/**< DAC Interpolation
							Control Register */
#define XRFDC_ADC_DECI_MODE_OFFSET	0x044U	/**< ADC Decimation mode
							Register */
#define XRFDC_ADC_MXR_CFG0_OFFSET	0x080U	/**< ADC I channel mixer
							config Register */
#define XRFDC_ADC_MXR_CFG1_OFFSET	0x084U	/**< ADC Q channel mixer
							config Register */
#define XRFDC_MXR_MODE_OFFSET		0x088U	/**< ADC/DAC mixer mode
							Register */
#define XRFDC_NCO_UPDT_OFFSET	0x08CU	/**< ADC/DAC NCO Update
							mode Register */
#define XRFDC_NCO_RST_OFFSET	0x090U	/**< ADC/DAC NCO Phase
							Reset Register */
#define XRFDC_ADC_NCO_FQWD_UPP_OFFSET	0x094U	/**< ADC NCO Frequency
							Word[47:32] Register */
#define XRFDC_ADC_NCO_FQWD_MID_OFFSET	0x098U	/**< ADC NCO Frequency
							Word[31:16] Register */
#define XRFDC_ADC_NCO_FQWD_LOW_OFFSET	0x09CU	/**< ADC NCO Frequency
							Word[15:0] Register */
#define XRFDC_NCO_PHASE_UPP_OFFSET	0x0A0U	/**< ADC/DAC NCO Phase[17:16]
							Register */
#define XRFDC_NCO_PHASE_LOW_OFFSET	0x0A4U	/**< ADC/DAC NCO Phase[15:0]
							Register */
#define XRFDC_ADC_NCO_PHASE_MOD_OFFSET	0x0A8U	/**< ADC NCO Phase
							Mode Register */
#define XRFDC_QMC_UPDT_OFFSET	0x0C8U	/**< ADC/DAC QMC Update Mode
							Register */
#define XRFDC_QMC_CFG_OFFSET	0x0CCU	/**< ADC/DAC QMC Config
							Register */
#define XRFDC_QMC_OFF_OFFSET	0x0D0U	/**< ADC/DAC QMC Offset
							Correction Register */
#define XRFDC_QMC_GAIN_OFFSET	0x0D4U	/**< ADC/DAC QMC Gain
							Correction Register */
#define XRFDC_QMC_PHASE_OFFSET	0x0D8U	/**< ADC/DAC QMC Phase
							Correction Register */
#define XRFDC_ADC_CRSE_DLY_UPDT_OFFSET	0x0DCU	/**< ADC Coarse Delay
							Update Register */
#define XRFDC_DAC_CRSE_DLY_UPDT_OFFSET	0x0E0U	/**< DAC Coarse Delay
							Update Register */
#define XRFDC_ADC_CRSE_DLY_CFG_OFFSET	0x0E0U	/**< ADC Coarse delay
							Config Register */
#define XRFDC_DAC_CRSE_DLY_CFG_OFFSET	0x0DCU	/**< DAC Coarse delay
							Config Register */
#define XRFDC_ADC_DAT_SCAL_CFG_OFFSET	0x0E4U	/**< ADC Data Scaling
							Config Register */
#define XRFDC_ADC_SWITCH_MATRX_OFFSET	0x0E8U	/**< ADC Switch Matrix
							Config Register */
#define XRFDC_ADC_TRSHD0_CFG_OFFSET		0x0ECU	/**< ADC Threshold0
							Config Register */
#define XRFDC_ADC_TRSHD0_AVG_UP_OFFSET	0x0F0U	/**< ADC Threshold0
							Average[31:16] Register */
#define XRFDC_ADC_TRSHD0_AVG_LO_OFFSET	0x0F4U	/**< ADC Threshold0
							Average[15:0] Register */
#define XRFDC_ADC_TRSHD0_UNDER_OFFSET	0x0F8U	/**< ADC Threshold0
							Under Threshold Register */
#define XRFDC_ADC_TRSHD0_OVER_OFFSET	0x0FCU	/**< ADC Threshold0
							Over Threshold Register */
#define XRFDC_ADC_TRSHD1_CFG_OFFSET	0x100U	/**< ADC Threshold1
							Config Register */
#define XRFDC_ADC_TRSHD1_AVG_UP_OFFSET	0x104U	/**< ADC Threshold1
							Average[31:16] Register */
#define XRFDC_ADC_TRSHD1_AVG_LO_OFFSET	0x108U	/**< ADC Threshold1
							Average[15:0] Register */
#define XRFDC_ADC_TRSHD1_UNDER_OFFSET	0x10CU	/**< ADC Threshold1
							Under Threshold Register */
#define XRFDC_ADC_TRSHD1_OVER_OFFSET	0x110U	/**< ADC Threshold1
							Over Threshold Register */
#define XRFDC_ADC_FEND_DAT_CRL_OFFSET	0x140U	/**< ADC Front end
							Data Control Register */
#define XRFDC_ADC_TI_DCB_CRL0_OFFSET	0x144U	/**< ADC Time Interleaved
							digital correction block gain control0 Register */
#define XRFDC_ADC_TI_DCB_CRL1_OFFSET	0x148U	/**< ADC Time Interleaved
							digital correction block gain control1 Register */
#define XRFDC_ADC_TI_DCB_CRL2_OFFSET	0x14CU	/**< ADC Time Interleaved
							digital correction block gain control2 Register */
#define XRFDC_ADC_TI_DCB_CRL3_OFFSET	0x150U	/**< ADC Time Interleaved
							digital correction block gain control3 Register */
#define XRFDC_ADC_TI_TISK_CRL0_OFFSET	0x154U	/**< ADC Time skew correction
							control bits0 Register */
#define XRFDC_DAC_MC_CFG0_OFFSET		0x1C4U	/**< Static Configuration
							 data for DAC Analog */
#define XRFDC_ADC_TI_TISK_CRL1_OFFSET	0x158U	/**< ADC Time skew correction
							control bits1 Register */
#define XRFDC_ADC_TI_TISK_CRL2_OFFSET	0x15CU	/**< ADC Time skew correction
							control bits2 Register */
#define XRFDC_ADC_TI_TISK_CRL3_OFFSET	0x160U	/**< ADC Time skew correction
							control bits3 Register */
#define XRFDC_ADC_TI_TISK_CRL4_OFFSET	0x164U	/**< ADC Time skew correction
							control bits4 Register */
#define XRFDC_ADC_TI_TISK_DAC0_OFFSET	0x168U	/**< ADC Time skew DAC
							cal code of subadc ch0 Register */
#define XRFDC_ADC_TI_TISK_DAC1_OFFSET	0x16CU	/**< ADC Time skew DAC
							cal code of subadc ch1 Register */
#define XRFDC_ADC_TI_TISK_DAC2_OFFSET	0x170U	/**< ADC Time skew DAC
							cal code of subadc ch2 Register */
#define XRFDC_ADC_TI_TISK_DAC3_OFFSET	0x174U	/**< ADC Time skew DAC
							cal code of subadc ch3 Register */
#define XRFDC_ADC_TI_TISK_DACP0_OFFSET	0x178U	/**< ADC Time skew DAC
							cal code of subadc ch0 Register */
#define XRFDC_ADC_TI_TISK_DACP1_OFFSET	0x17CU	/**< ADC Time skew DAC
							cal code of subadc ch1 Register */
#define XRFDC_ADC_TI_TISK_DACP2_OFFSET	0x180U	/**< ADC Time skew DAC
							cal code of subadc ch2 Register */
#define XRFDC_ADC_TI_TISK_DACP3_OFFSET	0x184U	/**< ADC Time skew DAC
							cal code of subadc ch3 Register */
#define XRFDC_ADC0_SUBDRP_ADDR_OFFSET	0x198U	/**< subadc0, sub-drp address
							of target Register */
#define XRFDC_ADC0_SUBDRP_DAT_OFFSET	0x19CU	/**< subadc0, sub-drp data
							of target Register */
#define XRFDC_ADC1_SUBDRP_ADDR_OFFSET	0x1A0U	/**< subadc1, sub-drp address
							of target Register */
#define XRFDC_ADC1_SUBDRP_DAT_OFFSET	0x1A4U	/**< subadc1, sub-drp data
							of target Register */
#define XRFDC_ADC2_SUBDRP_ADDR_OFFSET	0x1A8U	/**< subadc2, sub-drp address
							of target Register */
#define XRFDC_ADC2_SUBDRP_DAT_OFFSET	0x1ACU	/**< subadc2, sub-drp data
							of target Register */
#define XRFDC_ADC3_SUBDRP_ADDR_OFFSET	0x1B0U	/**< subadc3, sub-drp address
							of target Register */
#define XRFDC_ADC3_SUBDRP_DAT_OFFSET	0x1B4U	/**< subadc3, sub-drp data
							of target Register */
#define XRFDC_ADC_RX_MC_PWRDWN_OFFSET	0x1C0U	/**< ADC Static configuration
							bits for ADC(RX) analog Register */
#define XRFDC_ADC_DAC_MC_CFG0_OFFSET		0x1C4U	/**< ADC/DAC Static
							configuration bits for ADC/DAC analog Register */
#define XRFDC_ADC_DAC_MC_CFG1_OFFSET		0x1C8U	/**< ADC/DAC Static
							configuration bits for ADC/DAC analog Register */
#define XRFDC_ADC_DAC_MC_CFG2_OFFSET		0x1CCU	/**< ADC/DAC Static
							configuration bits for ADC/DAC analog Register */
#define XRFDC_DAC_MC_CFG3_OFFSET		0x1D0U	/**< DAC Static
							configuration bits for DAC analog Register */
#define XRFDC_ADC_RXPR_MC_CFG0_OFFSET	0x1D0U	/**< ADC RX Pair static
							Configuration Register */
#define XRFDC_ADC_RXPR_MC_CFG1_OFFSET	0x1D4U	/**< ADC RX Pair static
							Configuration Register */
#define XRFDC_ADC_TI_DCBSTS0_BG_OFFSET	0x200U	/**< ADC DCB Status0
							BG Register */
#define XRFDC_ADC_TI_DCBSTS0_FG_OFFSET	0x204U	/**< ADC DCB Status0
							FG Register */
#define XRFDC_ADC_TI_DCBSTS1_BG_OFFSET	0x208U	/**< ADC DCB Status1
							BG Register */
#define XRFDC_ADC_TI_DCBSTS1_FG_OFFSET	0x20CU	/**< ADC DCB Status1
							FG Register */
#define XRFDC_ADC_TI_DCBSTS2_BG_OFFSET	0x210U	/**< ADC DCB Status2
							BG Register */
#define XRFDC_ADC_TI_DCBSTS2_FG_OFFSET	0x214U	/**< ADC DCB Status2
							FG Register */
#define XRFDC_ADC_TI_DCBSTS3_BG_OFFSET	0x218U	/**< ADC DCB Status3
							BG Register */
#define XRFDC_ADC_TI_DCBSTS3_FG_OFFSET	0x21CU	/**< ADC DCB Status3
							FG Register */
#define XRFDC_ADC_TI_DCBSTS4_MB_OFFSET	0x220U	/**< ADC DCB Status4
							MSB Register */
#define XRFDC_ADC_TI_DCBSTS4_LB_OFFSET	0x224U	/**< ADC DCB Status4
							LSB Register */
#define XRFDC_ADC_TI_DCBSTS5_MB_OFFSET	0x228U	/**< ADC DCB Status5
							MSB Register */
#define XRFDC_ADC_TI_DCBSTS5_LB_OFFSET	0x22CU	/**< ADC DCB Status5
							LSB Register */
#define XRFDC_ADC_TI_DCBSTS6_MB_OFFSET	0x230U	/**< ADC DCB Status6
							MSB Register */
#define XRFDC_ADC_TI_DCBSTS6_LB_OFFSET	0x234U	/**< ADC DCB Status6
							LSB Register */
#define XRFDC_ADC_TI_DCBSTS7_MB_OFFSET	0x238U	/**< ADC DCB Status7
							MSB Register */
#define XRFDC_ADC_TI_DCBSTS7_LB_OFFSET	0x23CU	/**< ADC DCB Status7
							LSB Register */
#define XRFDC_ADC_FIFO_LTNCY_LB_OFFSET	0x280U	/**< ADC FIFO Latency
							measurement LSB Register */
#define XRFDC_ADC_FIFO_LTNCY_MB_OFFSET	0x284U	/**< ADC FIFO Latency
							measurement MSB Register */
#define XRFDC_DAC_DECODER_CTRL_OFFSET	0x180U	/**< DAC Unary Decoder/
							Randomizer settings */
#define XRFDC_HSCOM_PWR_OFFSET		0x094	/**< Control register during
							power-up sequence */
#define XRFDC_HSCOM_UPDT_DYN_OFFSET		0x0B8	/**< Trigger the update
							dynamic event */

#define XRFDC_RESET_OFFSET		0x00U	/**< Tile reset register */
#define XRFDC_RESTART_OFFSET	0x04U	/**< Tile restart register */
#define XRFDC_RESTART_STATE_OFFSET	0x08U	/**< Tile restart state register */
#define XRFDC_CURRENT_STATE_OFFSET	0x0CU	/**< Current state register */
#define XRFDC_STATUS_OFFSET			0x228U	/**< Common status register */
#define XRFDC_COMMON_INTR_STS		0x100U	/**< Common Intr Status register */
#define XRFDC_COMMON_INTR_ENABLE	0x104U	/**< Common Intr enable register */
#define XRFDC_INTR_STS				0x200U	/**< Intr status register */
#define XRFDC_INTR_ENABLE			0x204U	/**< Intr enable register */
#define XRFDC_CONV_INTR_STS(X)		(0x208U + (X * 0x08))
#define XRFDC_CONV_INTR_EN(X)		(0x20CU + (X * 0x08))
#define XRFDC_FIFO_ENABLE			0x230U	/**< FIFO Enable and Disable */

/* @} */

/** @name FIFO Enable - FIFO enable and disable register
 *
 * This register contains bits for FIFO enable and disable
 * for ADC and DAC.
 * @{
 */

#define XRFDC_FIFO_EN_MASK	0x00000001U /**< FIFO enable/disable */

/* @} */

/** @name Clock Enable - FIFO Latency, fabric, DataPath,
 * 			full-rate, output register
 *
 * This register contains bits for various clock enable options of
 * the ADC. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_CLK_EN_CAL_MASK	0x00000001U /**< Enable Output
							Register clock */
#define XRFDC_CLK_EN_DIG_MASK	0x00000002U /**< Enable full-rate clock */
#define XRFDC_CLK_EN_DP_MASK	0x00000004U /**< Enable Data Path clock */
#define XRFDC_CLK_EN_FAB_MASK	0x00000008U /**< Enable fabric clock */
#define XRFDC_DAT_CLK_EN_MASK	0x0000000FU /**< Data Path Clk enable */
#define XRFDC_CLK_EN_LM_MASK	0x00000010U /**< Enable for FIFO
							Latency measurement clock */

/* @} */

/** @name Debug reset - FIFO Latency, fabric, DataPath,
 * 			full-rate, output register
 *
 * This register contains bits for various Debug reset options of
 * the ADC. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DBG_RST_CAL_MASK	0x00000001U /**< Reset clk_cal
							clock domain */
#define XRFDC_DBG_RST_DP_MASK	0x00000002U /**< Reset data path
							clock domain */
#define XRFDC_DBG_RST_FAB_MASK	0x00000004U /**< Reset clock fabric
							clock domain */
#define XRFDC_DBG_RST_DIG_MASK	0x00000008U /**< Reset clk_dig clock
							domain */
#define XRFDC_DBG_RST_DRP_CAL_MASK	0x00000010U /**< Reset subadc-drp
							register on clock cal */
#define XRFDC_DBG_RST_LM_MASK	0x00000020U /**< Reset FIFO Latency
							measurement clock domain */

/* @} */

/** @name Fabric rate - Fabric data rate for read and write
 *
 * This register contains bits for read and write fabric data
 * rate for ADC. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_ADC_FAB_RATE_WR_MASK	0x0000000FU /**< ADC FIFO Write Number
							of Words per clock */
#define XRFDC_DAC_FAB_RATE_WR_MASK	0x0000001FU /**< DAC FIFO Write Number
							of Words per clock */
#define XRFDC_FAB_RATE_RD_MASK	0x00000F00U /**< FIFO Read Number
							of words per clock */

/* @} */

/** @name Fabric Offset - FIFO de-skew
 *
 * This register contains bits of Fabric Offset.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FAB_RD_PTR_OFFST_MASK	0x0000003FU /**< FIFO read pointer
							offset for interface de-skew */

/* @} */

/** @name Fabric ISR - Interrupt status register for FIFO interface
 *
 * This register contains bits of margin-indicator and user-data overlap
 * (overflow/underflow). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FAB_ISR_USRDAT_OVR_MASK	0x00000001U /**< User-data overlap-
							data written faster than read (overflow) */
#define XRFDC_FAB_ISR_USRDAT_UND_MASK	0x00000002U /**< User-data overlap-
							data read faster than written (underflow) */
#define XRFDC_FAB_ISR_USRDAT_MASK	0x00000003U /**< User-data overlap Mask */
#define XRFDC_FAB_ISR_MARGIND_OVR_MASK	0x00000004U /**< Marginal-indicator
							overlap (overflow) */
#define XRFDC_FAB_ISR_MARGIND_UND_MASK	0x00000008U /**< Marginal-indicator
							overlap (underflow) */
/* @} */

/** @name Fabric IMR - Interrupt mask register for FIFO interface
 *
 * This register contains bits of margin-indicator and user-data overlap
 * (overflow/underflow). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FAB_IMR_USRDAT_OVR_MASK	0x00000001U /**< User-data overlap-
							data written faster than read (overflow) */
#define XRFDC_FAB_IMR_USRDAT_UND_MASK	0x00000002U /**< User-data overlap-
							data read faster than written (underflow) */
#define XRFDC_FAB_IMR_USRDAT_MASK	0x00000003U /**< User-data overlap Mask */
#define XRFDC_FAB_IMR_MARGIND_OVR_MASK	0x00000004U /**< Marginal-indicator
							overlap (overflow) */
#define XRFDC_FAB_IMR_MARGIND_UND_MASK	0x00000008U /**< Marginal-indicator
							overlap (underflow) */
/* @} */

/** @name Update Dynamic - Trigger a dynamic update event
 *
 * This register contains bits of update event for slice, nco, qmc
 * and coarse delay. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_UPDT_EVNT_MASK	0x0000000FU /**< Update event mask */
#define XRFDC_UPDT_EVNT_SLICE_MASK	0x00000001U /**< Trigger a slice update
							event apply to _DCONFIG reg */
#define XRFDC_UPDT_EVNT_NCO_MASK	0x00000002U /**< Trigger a update event
							apply to NCO_DCONFIG reg */
#define XRFDC_UPDT_EVNT_QMC_MASK	0x00000004U /**< Trigger a update event
							apply to QMC_DCONFIG reg */
#define XRFDC_ADC_UPDT_CRSE_DLY_MASK	0x00000008U /**< ADC Trigger a update event
							apply to Coarse delay_DCONFIG reg */
#define XRFDC_DAC_UPDT_CRSE_DLY_MASK	0x00000020U /**< DAC Trigger a update event
							apply to Coarse delay_DCONFIG reg */
/* @} */

/** @name FIFO Latency control - Config registers for FIFO Latency measurement
 *
 * This register contains bits of FIFO Latency ctrl for disable, restart and
 * set fifo latency measurement. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FIFO_LTNCY_PRD_MASK	0x00000007U /**< Set FIFO Latency
							measurement period */
#define XRFDC_FIFO_LTNCY_RESTRT_MASK	0x00000008U /**< Restart FIFO Latency
							measurement */
#define XRFDC_FIFO_LTNCY_DIS_MASK	0x000000010U /**< Disable FIFO Latency
							measurement */

/* @} */

/** @name Decode ISR - ISR for Decoder Interface
 *
 * This register contains bits of subadc 0,1,2 and 3 decoder overflow
 * and underflow range. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DEC_ISR_SUBADC_MASK		0x000000FFU /**< subadc decoder
							Mask */
#define XRFDC_DEC_ISR_SUBADC0_UND_MASK	0x00000001U /**< subadc0 decoder
							underflow range */
#define XRFDC_DEC_ISR_SUBADC0_OVR_MASK	0x00000002U /**< subadc0 decoder
							overflow range */
#define XRFDC_DEC_ISR_SUBADC1_UND_MASK	0x00000004U /**< subadc1 decoder
							underflow range */
#define XRFDC_DEC_ISR_SUBADC1_OVR_MASK	0x00000008U /**< subadc1 decoder
							overflow range */
#define XRFDC_DEC_ISR_SUBADC2_UND_MASK	0x00000010U /**< subadc2 decoder
							underflow range */
#define XRFDC_DEC_ISR_SUBADC2_OVR_MASK	0x00000020U /**< subadc2 decoder
							overflow range */
#define XRFDC_DEC_ISR_SUBADC3_UND_MASK	0x00000040U /**< subadc3 decoder
							underflow range */
#define XRFDC_DEC_ISR_SUBADC3_OVR_MASK	0x00000080U /**< subadc3 decoder
							overflow range */

/* @} */

/** @name Decode IMR - IMR for Decoder Interface
 *
 * This register contains bits of subadc 0,1,2 and 3 decoder overflow
 * and underflow range. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DEC_IMR_SUBADC0_UND_MASK	0x00000001U /**< subadc0 decoder
							underflow range */
#define XRFDC_DEC_IMR_SUBADC0_OVR_MASK	0x00000002U /**< subadc0 decoder
							overflow range */
#define XRFDC_DEC_IMR_SUBADC1_UND_MASK	0x00000004U /**< subadc1 decoder
							underflow range */
#define XRFDC_DEC_IMR_SUBADC1_OVR_MASK	0x00000008U /**< subadc1 decoder
							overflow range */
#define XRFDC_DEC_IMR_SUBADC2_UND_MASK	0x00000010U /**< subadc2 decoder
							underflow range */
#define XRFDC_DEC_IMR_SUBADC2_OVR_MASK	0x00000020U /**< subadc2 decoder
							overflow range */
#define XRFDC_DEC_IMR_SUBADC3_UND_MASK	0x00000040U /**< subadc3 decoder
							underflow range */
#define XRFDC_DEC_IMR_SUBADC3_OVR_MASK	0x00000080U /**< subadc3 decoder
							overflow range */

/* @} */

/** @name DataPath ISR - ISR for Data Path interface
 *
 * This register contains bits of QMC Gain/Phase overflow, offset overflow,
 * Decimation I-Path and Interpolation Q-Path overflow for stages 0,1,2.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_ADC_DAT_PATH_ISR_MASK	0x000000FFU /**< ADC Data Path Overflow */
#define XRFDC_DAC_DAT_PATH_ISR_MASK	0x000001FFU /**< DAC Data Path Overflow */
#define XRFDC_DAT_ISR_DECI_IPATH_MASK	0x00000007U /**< Decimation I-Path
							overflow for stages 0,1,2 */
#define XRFDC_DAT_ISR_INTR_QPATH_MASK	0x00000038U /**< Interpolation
							Q-Path overflow for stages 0,1,2 */
#define XRFDC_DAT_ISR_QMC_GAIN_MASK	0x00000040U /**< QMC Gain/Phase
							overflow */
#define XRFDC_DAT_ISR_QMC_OFFST_MASK	0x00000080U /**< QMC offset
							overflow */
#define XRFDC_DAC_DAT_ISR_INVSINC_MASK	0x00000100U /**< Inverse Sinc offset
							overflow */

/* @} */

/** @name DataPath IMR - IMR for Data Path interface
 *
 * This register contains bits of QMC Gain/Phase overflow, offset overflow,
 * Decimation I-Path and Interpolation Q-Path overflow for stages 0,1,2.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DAT_IMR_DECI_IPATH_MASK	0x00000007U /**< Decimation I-Path
							overflow for stages 0,1,2 */
#define XRFDC_DAT_IMR_INTR_QPATH_MASK	0x00000038U /**< Interpolation
							Q-Path overflow for stages 0,1,2 */
#define XRFDC_DAT_IMR_QMC_GAIN_MASK	0x00000040U /**< QMC Gain/Phase
							overflow */
#define XRFDC_DAT_IMR_QMC_OFFST_MASK	0x00000080U /**< QMC offset
							overflow */

/* @} */

/** @name Decimation Config - Decimation control
 *
 * This register contains bits to configure the decimation in terms of
 * the type of data. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DEC_CFG_MASK	0x00000003U /**< ChannelA (2GSPS real
							data from Mixer I output) */
#define XRFDC_DEC_CFG_CHB_MASK	0x00000001U /**< ChannelB (2GSPS real
							data from Mixer Q output) */
#define XRFDC_DEC_CFG_IQ_MASK	0x00000002U /**< IQ-2GSPS */
#define XRFDC_DEC_CFG_4GSPS_MASK	0x00000003U /**< 4GSPS may be I or Q
							or Real depending on high level block config */

/* @} */

/** @name Decimation Mode - Decimation Rate
 *
 * This register contains bits to configures the decimation rate.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DEC_MOD_MASK		0x00000007U	/**< Decimation mode Mask */
#define XRFDC_DEC_MOD_1X_MASK	0x00000001U /**< 1x (decimation bypass) */
#define XRFDC_DEC_MOD_2X_MASK	0x00000002U /**< 2x (decimation bypass) */
#define XRFDC_DEC_MOD_4X_MASK	0x00000003U /**< 4x (decimation bypass) */
#define XRFDC_DEC_MOD_8X_MASK	0x00000004U /**< 8x (decimation bypass) */
#define XRFDC_DEC_MOD_2X_BW_MASK	0x00000005U /**< 2x (med BW) */
#define XRFDC_DEC_MOD_4X_BW_MASK	0x00000006U /**< 4x (med BW) */
#define XRFDC_DEC_MOD_8X_BW_MASK	0x00000007U /**< 8x (med BW) */

/* @} */

/** @name Mixer config0 - Configure I channel coarse mixer mode of operation
 *
 * This register contains bits to set the output data sequence of
 * I channel. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_MIX_CFG0_MASK		0x00000FFFU	/**< Mixer Config0 Mask */
#define XRFDC_MIX_I_DAT_WRD0_MASK	0x00000007U /**< Output data word[0]
							of I channel */
#define XRFDC_MIX_I_DAT_WRD1_MASK	0x00000038U /**< Output data word[1]
							of I channel */
#define XRFDC_MIX_I_DAT_WRD2_MASK	0x000001C0U /**< Output data word[2]
							of I channel */
#define XRFDC_MIX_I_DAT_WRD3_MASK	0x00000E00U /**< Output data word[3]
							of I channel */

/* @} */

/** @name Mixer config1 - Configure Q channel coarse mixer mode of operation
 *
 * This register contains bits to set the output data sequence of
 * Q channel. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_MIX_CFG1_MASK		0x00000FFFU	/**< Mixer Config0 Mask */
#define XRFDC_MIX_Q_DAT_WRD0_MASK	0x00000007U /**< Output data word[0]
							of Q channel */
#define XRFDC_MIX_Q_DAT_WRD1_MASK	0x00000038U /**< Output data word[1]
							of Q channel */
#define XRFDC_MIX_Q_DAT_WRD2_MASK	0x000001C0U /**< Output data word[2]
							of Q channel */
#define XRFDC_MIX_Q_DAT_WRD3_MASK	0x00000E00U /**< Output data word[3]
							of Q channel */

/* @} */

/** @name Mixer mode - Configure mixer mode of operation
 *
 * This register contains bits to set NCO phases, NCO output scale
 * and fine mixer multipliers. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_EN_I_IQ_MASK			0x00000003U	/**< Enable fine mixer
							multipliers on IQ i/p for I output */
#define XRFDC_EN_Q_IQ_MASK			0x0000000CU	/**< Enable fine mixer
							multipliers on IQ i/p for Q output */
#define XRFDC_FINE_MIX_SCALE_MASK	0x00000010U	/**< NCO output scale */
#define XRFDC_SEL_I_IQ_MASK			0x00000F00U	/**< Select NCO phases
							for I output */
#define XRFDC_SEL_Q_IQ_MASK			0x0000F000U	/**< Select NCO phases
							for Q output */
#define XRFDC_I_IQ_COS_MINSIN	0x00000C00U	/**< Select NCO phases
							for I output */
#define XRFDC_Q_IQ_SIN_COS		0x00001000U	/**< Select NCO phases
							for Q output */

/* @} */

/** @name NCO update - NCO update mode
 *
 * This register contains bits to Select event source, delay and reset delay.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_UPDT_MODE_MASK	0x00000007U	/**< NCO event source
							selection mask */
#define XRFDC_NCO_UPDT_MODE_GRP		0x00000000U	/**< NCO event source
							selection is Group */
#define XRFDC_NCO_UPDT_MODE_SLICE	0x00000001U	/**< NCO event source
							selection is slice */
#define XRFDC_NCO_UPDT_MODE_TILE	0x00000002U	/**< NCO event source
							selection is tile */
#define XRFDC_NCO_UPDT_MODE_SYSREF	0x00000003U	/**< NCO event source
							selection is Sysref */
#define XRFDC_NCO_UPDT_MODE_MARKER	0x00000004U	/**< NCO event source
							selection is Marker */
#define XRFDC_NCO_UPDT_MODE_FABRIC	0x00000005U	/**< NCO event source
							selection is fabric */
#define XRFDC_NCO_UPDT_DLY_MASK		0x00001FF8U	/**< delay in clk_dp
							cycles in application of event after arrival */
#define XRFDC_NCO_UPDT_RST_DLY_MASK	0x0000D000U	/**< optional delay on
							the NCO phase reset delay */

/* @} */

/** @name NCO Phase Reset - NCO Slice Phase Reset
 *
 * This register contains bits to reset the nco phase of the current
 * slice phase accumulator. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_PHASE_RST_MASK	0x00000001U	/**< Reset NCO Phase
							of current slice */

/* @} */

/** @name NCO Freq Word[47:32] - NCO Phase increment(nco freq 48-bit)
 *
 * This register contains bits for frequency control word of the
 * NCO. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_FQWD_UPP_MASK		0x0000FFFFU	/**< NCO Phase
							increment[47:32] */

/* @} */

/** @name NCO Freq Word[31:16] - NCO Phase increment(nco freq 48-bit)
 *
 * This register contains bits for frequency control word of the
 * NCO. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_FQWD_MID_MASK		0x0000FFFFU	/**< NCO Phase
							increment[31:16] */

/* @} */

/** @name NCO Freq Word[15:0] - NCO Phase increment(nco freq 48-bit)
 *
 * This register contains bits for frequency control word of the
 * NCO. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_FQWD_LOW_MASK		0x0000FFFFU	/**< NCO Phase
							increment[15:0] */
#define XRFDC_NCO_FQWD_MASK			0x0000FFFFFFFFFFFFU	/**< NCO Freq
							offset[48:0] */

/* @} */

/** @name NCO Phase Offset[17:16] - NCO Phase offset
 *
 * This register contains bits to set NCO Phase offset(18-bit offset
 * added to the phase accumulator). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_PHASE_UPP_MASK		0x00000003U	/**< NCO Phase
							offset[17:16] */

/* @} */

/** @name NCO Phase Offset[15:0] - NCO Phase offset
 *
 * This register contains bits to set NCO Phase offset(18-bit offset
 * added to the phase accumulator). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_PHASE_LOW_MASK		0x0000FFFFU	/**< NCO Phase
							offset[15:0] */
#define XRFDC_NCO_PHASE_MASK			0x0003FFFFU	/**< NCO Phase
							offset[17:0] */

/* @} */

/** @name NCO Phase mode - NCO Control setting mode
 *
 * This register contains bits to set NCO mode of operation.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_PHASE_MOD_MASK		0x00000003U	/**< NCO mode
							of operation  mask */
#define XRFDC_NCO_PHASE_MOD_4PHASE		0x00000003U	/**< NCO output
							4 successive phase */
#define XRFDC_NCO_PHASE_MOD_EVEN		0x00000001U	/**< NCO output
							even phase */
#define XRFDC_NCO_PHASE_MODE_ODD		0x00000002U	/**< NCO output
							odd phase */
/* @} */

/** @name QMC update - QMC update mode
 *
 * This register contains bits to Select event source and delay.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_QMC_UPDT_MODE_MASK	0x00000007U	/**< QMC event source
							selection mask */
#define XRFDC_QMC_UPDT_MODE_GRP		0x00000000U	/**< QMC event source
							selection is group */
#define XRFDC_QMC_UPDT_MODE_SLICE	0x00000001U	/**< QMC event source
							selection is slice */
#define XRFDC_QMC_UPDT_MODE_TILE	0x00000002U	/**< QMC event source
							selection is tile */
#define XRFDC_QMC_UPDT_MODE_SYSREF	0x00000003U	/**< QMC event source
							selection is Sysref */
#define XRFDC_QMC_UPDT_MODE_MARKER	0x00000004U	/**< QMC event source
							selection is Marker */
#define XRFDC_QMC_UPDT_MODE_FABRIC	0x00000005U	/**< QMC event source
							selection is fabric */
#define XRFDC_QMC_UPDT_DLY_MASK		0x00001FF8U	/**< delay in clk_dp
							cycles in application of event after arrival */

/* @} */

/** @name QMC Config - QMC Config register
 *
 * This register contains bits to enable QMC gain and QMC
 * Phase correction. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_QMC_CFG_EN_GAIN_MASK	0x00000001U	/**< enable QMC gain
							correction mask */
#define XRFDC_QMC_CFG_EN_PHASE_MASK	0x00000002U	/**< enable QMC Phase
							correction mask */

/* @} */

/** @name QMC Offset - QMC offset correction
 *
 * This register contains bits to set QMC offset correction
 * factor. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_QMC_OFFST_CRCTN_MASK	0x00000FFFU	/**< QMC offset
							correction factor */

/* @} */

/** @name QMC Gain - QMC Gain correction
 *
 * This register contains bits to set QMC gain correction
 * factor. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_QMC_GAIN_CRCTN_MASK	0x00003FFFU	/**< QMC gain
							correction factor */

/* @} */

/** @name QMC Phase - QMC Phase correction
 *
 * This register contains bits to set QMC phase correction
 * factor. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_QMC_PHASE_CRCTN_MASK	0x00000FFFU	/**< QMC phase
							correction factor */

/* @} */

/** @name Coarse Delay Update - Coarse delay update mode.
 *
 * This register contains bits to Select event source and delay.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_CRSEDLY_UPDT_MODE_MASK	0x00000007U	/**< Coarse delay event
							source selection mask */
#define XRFDC_CRSEDLY_UPDT_MODE_GRP		0x00000000U	/**< Coarse delay event
							source selection is group */
#define XRFDC_CRSEDLY_UPDT_MODE_SLICE	0x00000001U/**< Coarse delay event
							source selection is slice */
#define XRFDC_CRSEDLY_UPDT_MODE_TILE	0x00000002U	/**< Coarse delay event
							source selection is tile */
#define XRFDC_CRSEDLY_UPDT_MODE_SYSREF	0x00000003U	/**< Coarse delay event
							source selection is sysref */
#define XRFDC_CRSEDLY_UPDT_MODE_MARKER	0x00000004U	/**< Coarse delay event
							source selection is Marker */
#define XRFDC_CRSEDLY_UPDT_MODE_FABRIC	0x00000005U	/**< Coarse delay event
							source selection is fabric */
#define XRFDC_CRSEDLY_UPDT_DLY_MASK		0x00001FF8U	/**< delay in clk_dp
							cycles in application of event after arrival */

/* @} */

/** @name Coarse delay Config - Coarse delay select
 *
 * This register contains bits to select coarse delay.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_CRSE_DLY_CFG_MASK		0x00000007U	/**< Coarse delay select */

/* @} */

/** @name Data Scaling Config - Data Scaling enable
 *
 * This register contains bits to enable data scaling.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DAT_SCALE_CFG_MASK		0x00000001U	/**< Enable data scaling */

/* @} */

/** @name Data Scaling Config - Data Scaling enable
 *
 * This register contains bits to enable data scaling.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DAT_SCALE_CFG_MASK		0x00000001U	/**< Enable data scaling */

/* @} */

/** @name Switch Matrix Config
 *
 * This register contains bits to control crossbar switch that select
 * data to mixer block. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SEL_CB_TO_MIX1_MASK		0x00000003U	/**< Control crossbar
							switch that select the data to mixer block mux1 */
#define XRFDC_SEL_CB_TO_MIX0_MASK		0x0000000CU	/**< Control crossbar
							switch that select the data to mixer block mux0 */
#define XRFDC_SEL_CB_TO_QMC_MASK		0x00000010U	/**< Control crossbar
							switch that select the data to QMC */
#define XRFDC_SEL_CB_TO_DECI_MASK		0x00000020U	/**< Control crossbar
							switch that select the data to decimation filter */

/* @} */

/** @name Threshold0 Config
 *
 * This register contains bits to select mode, clear mode and to
 * clear sticky bit. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD0_EN_MOD_MASK		0x00000003U	/**< Enable Threshold0
							block */
#define XRFDC_TRSHD0_CLR_MOD_MASK		0x00000004U	/**< Clear mode */
#define XRFDC_TRSHD0_STIKY_CLR_MASK		0x00000008U	/**< Clear sticky bit */

/* @} */

/** @name Threshold0 Average[31:16]
 *
 * This register contains bits to select Threshold0 under averaging.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD0_AVG_UPP_MASK		0x0000FFFFU	/**< Threshold0 under
							Averaging[31:16] */

/* @} */

/** @name Threshold0 Average[15:0]
 *
 * This register contains bits to select Threshold0 under averaging.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD0_AVG_LOW_MASK		0x0000FFFFU	/**< Threshold0 under
							Averaging[15:0] */

/* @} */

/** @name Threshold0 Under threshold
 *
 * This register contains bits to select Threshold0 under threshold.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD0_UNDER_MASK		0x00007FFFU	/**< Threshold0 under
							Threshold[14:0] */

/* @} */

/** @name Threshold0 Over threshold
 *
 * This register contains bits to select Threshold0 over threshold.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD0_OVER_MASK		0x00007FFFU	/**< Threshold0 under
							Threshold[14:0] */

/* @} */

/** @name Threshold1 Config
 *
 * This register contains bits to select mode, clear mode and to
 * clear sticky bit. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD1_EN_MOD_MASK		0x00000003U	/**< Enable Threshold1
							block */
#define XRFDC_TRSHD1_CLR_MOD_MASK		0x00000004U	/**< Clear mode */
#define XRFDC_TRSHD1_STIKY_CLR_MASK		0x00000008U	/**< Clear sticky bit */

/* @} */

/** @name Threshold1 Average[31:16]
 *
 * This register contains bits to select Threshold1 under averaging.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD1_AVG_UPP_MASK		0x0000FFFFU	/**< Threshold1 under
							Averaging[31:16] */

/* @} */

/** @name Threshold1 Average[15:0]
 *
 * This register contains bits to select Threshold1 under averaging.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD1_AVG_LOW_MASK		0x0000FFFFU	/**< Threshold1 under
							Averaging[15:0] */

/* @} */

/** @name Threshold1 Under threshold
 *
 * This register contains bits to select Threshold1 under threshold.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD1_UNDER_MASK		0x00007FFFU	/**< Threshold1 under
							Threshold[14:0] */

/* @} */

/** @name Threshold1 Over threshold
 *
 * This register contains bits to select Threshold1 over threshold.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD1_OVER_MASK		0x00007FFFU	/**< Threshold1 under
							Threshold[14:0] */

/* @} */

/** @name FrontEnd Data Control
 *
 * This register contains bits to select raw data and cal coefficient to
 * be streamed to memory. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FEND_DAT_CTRL_MASK	0x000000FFU	/**< raw data and cal
							coefficient to be streamed to memory */

/* @} */

/** @name TI Digital Correction Block control0
 *
 * This register contains bits for Time Interleaved digital correction
 * block gain and offset correction. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_CTRL0_MASK		0x0000FFFFU	/**< TI  DCB gain and
							offset correction */

/* @} */

/** @name TI Digital Correction Block control1
 *
 * This register contains bits for Time Interleaved digital correction
 * block gain and offset correction. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_CTRL1_MASK		0x00001FFFU	/**< TI  DCB gain and
							offset correction */

/* @} */

/** @name TI Digital Correction Block control2
 *
 * This register contains bits for Time Interleaved digital correction
 * block gain and offset correction. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_CTRL2_MASK		0x00001FFFU	/**< TI  DCB gain and
							offset correction */

/* @} */

/** @name TI Time Skew control0
 *
 * This register contains bits for Time skew correction control bits0(enables,
 * mode, multiplier factors, debug). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_TISK_EN_MASK		0x00000001U	/**< Block Enable */
#define XRFDC_TI_TISK_MODE_MASK		0x00000002U	/**< Mode (2G/4G) */
#define XRFDC_TI_TISK_ZONE_MASK		0x00000004U	/**< Specifies Nyquist zone */
#define XRFDC_TI_TISK_CHOP_EN_MASK	0x00000008U	/**< enable chopping mode */
#define XRFDC_TI_TISK_MU_CM_MASK	0x000000F0U	/**< Constant mu_cm multiplying
							common mode path */
#define XRFDC_TI_TISK_MU_DF_MASK	0x00000F00U	/**< Constant mu_df multiplying
							differential path */
#define XRFDC_TI_TISK_DBG_CTRL_MASK	0x0000F000U	/**< Debug control */
#define XRFDC_TI_TISK_DBG_UPDT_RT_MASK	0x00001000U	/**< Debug update rate */
#define XRFDC_TI_TISK_DITH_DLY_MASK	0x0000E000U	/**< Programmable delay on
							dither path to match data path */

/* @} */

/** @name DAC MC Config0
 *
 * This register contains bits for enable/disable shadow logic , Nyquist zone
 * selction, enable full speed clock, Programmable delay.
 * @{
 */

#define XRFDC_MC_CFG0_MIX_MODE_MASK		0x00000002U	/**< Enable
								Mixing mode */


/* @} */

/** @name TI Time Skew control0
 *
 * This register contains bits for Time skew correction control bits0(enables,
 * mode, multiplier factors, debug). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_EN_MASK		0x00000001U	/**< Block Enable */
#define XRFDC_TISK_MODE_MASK		0x00000002U	/**< Mode (2G/4G) */
#define XRFDC_TISK_ZONE_MASK		0x00000004U	/**< Specifies Nyquist zone */
#define XRFDC_TISK_CHOP_EN_MASK	0x00000008U	/**< enable chopping mode */
#define XRFDC_TISK_MU_CM_MASK	0x000000F0U	/**< Constant mu_cm multiplying
							common mode path */
#define XRFDC_TISK_MU_DF_MASK	0x00000F00U	/**< Constant mu_df multiplying
							differential path */
#define XRFDC_TISK_DBG_CTRL_MASK	0x0000F000U	/**< Debug control */
#define XRFDC_TISK_DBG_UPDT_RT_MASK	0x00001000U	/**< Debug update rate */
#define XRFDC_TISK_DITH_DLY_MASK	0x0000E000U	/**< Programmable delay on
							dither path to match data path */

/* @} */

/** @name TI Time Skew control1
 *
 * This register contains bits for Time skew correction control bits1
 * (Deadzone Parameters). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DZ_MIN_VAL_MASK		0x000000FFU	/**< Deadzone min */
#define XRFDC_TISK_DZ_MAX_VAL_MASK		0x0000FF00U	/**< Deadzone max */

/* @} */

/** @name TI Time Skew control2
 *
 * This register contains bits for Time skew correction control bits2
 * (Filter parameters). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_MU0_MASK			0x0000000FU	/**< Filter0 multiplying factor */
#define XRFDC_TISK_BYPASS0_MASK		0x00000080U	/**< ByPass filter0 */
#define XRFDC_TISK_MU1_MASK			0x00000F00U	/**< Filter1 multiplying factor */
#define XRFDC_TISK_BYPASS1_MASK		0x00008000U	/**< Filter1 multiplying factor */

/* @} */

/** @name TI Time Skew control3
 *
 * This register contains bits for Time skew control settling time
 * following code update. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_SETTLE_MASK		0x000000FFU	/**< Settling time following
							code update */

/* @} */

/** @name TI Time Skew control4
 *
 * This register contains bits for Time skew control setting time
 * following code update. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_CAL_PRI_MASK		0x00000001U	/**< */
#define XRFDC_TISK_DITH_INV_MASK	0x00000FF0U	/**< */

/* @} */

/** @name TI Time Skew DAC0
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch0. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DAC0_CODE_MASK		0x000000FFU	/**< Code to correction
							DAC of subadc ch0 front end switch0 */
#define XRFDC_TISK_DAC0_OVRID_EN_MASK	0x00008000U	/**< override enable */

/* @} */

/** @name TI Time Skew DAC1
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch1. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DAC1_CODE_MASK		0x000000FFU	/**< Code to correction
							DAC of subadc ch1 front end switch0 */
#define XRFDC_TISK_DAC1_OVRID_EN_MASK	0x00008000U	/**< override enable */

/* @} */

/** @name TI Time Skew DAC2
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch2. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DAC2_CODE_MASK		0x000000FFU	/**< Code to correction
							DAC of subadc ch2 front end switch0 */
#define XRFDC_TISK_DAC2_OVRID_EN_MASK	0x00008000U	/**< override enable */

/* @} */

/** @name TI Time Skew DAC3
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch3. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DAC3_CODE_MASK		0x000000FFU	/**< Code to correction
							DAC of subadc ch3 front end switch0 */
#define XRFDC_TISK_DAC3_OVRID_EN_MASK	0x00008000U	/**< override enable */

/* @} */

/** @name TI Time Skew DACP0
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch0. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DACP0_CODE_MASK		0x000000FFU	/**< Code to correction
							DAC of subadc ch0 front end switch1 */
#define XRFDC_TISK_DACP0_OVRID_EN_MASK	0x00008000U	/**< override enable */

/* @} */

/** @name TI Time Skew DACP1
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch1. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DACP1_CODE_MASK		0x000000FFU	/**< Code to correction
							DAC of subadc ch1 front end switch1 */
#define XRFDC_TISK_DACP1_OVRID_EN_MASK	0x00008000U	/**< override enable */

/* @} */

/** @name TI Time Skew DACP2
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch2. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DACP2_CODE_MASK		0x000000FFU	/**< Code to correction
							DAC of subadc ch2 front end switch1 */
#define XRFDC_TISK_DACP2_OVRID_EN_MASK	0x00008000U	/**< override enable */

/* @} */

/** @name TI Time Skew DACP3
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch3. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DACP3_CODE_MASK		0x000000FFU	/**< Code to correction
							DAC of subadc ch3 front end switch1 */
#define XRFDC_TISK_DACP3_OVRID_EN_MASK	0x00008000U	/**< override enable */

/* @} */

/** @name SubDRP ADC0 address
 *
 * This register contains the sub-drp address of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC0_ADDR_MASK		0x000000FFU	/**< sub-drp0 address */

/* @} */

/** @name SubDRP ADC0 Data
 *
 * This register contains the sub-drp data of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC0_DAT_MASK		0x0000FFFFU	/**< sub-drp0 data
							for read or write transaction */

/* @} */

/** @name SubDRP ADC1 address
 *
 * This register contains the sub-drp address of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC1_ADDR_MASK		0x000000FFU	/**< sub-drp1 address */

/* @} */

/** @name SubDRP ADC1 Data
 *
 * This register contains the sub-drp data of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC1_DAT_MASK		0x0000FFFFU	/**< sub-drp1 data
							for read or write transaction */

/* @} */

/** @name SubDRP ADC2 address
 *
 * This register contains the sub-drp address of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC2_ADDR_MASK		0x000000FFU	/**< sub-drp2 address */

/* @} */

/** @name SubDRP ADC2 Data
 *
 * This register contains the sub-drp data of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC2_DAT_MASK		0x0000FFFFU	/**< sub-drp2 data
							for read or write transaction */

/* @} */

/** @name SubDRP ADC3 address
 *
 * This register contains the sub-drp address of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC3_ADDR_MASK		0x000000FFU	/**< sub-drp3 address */

/* @} */

/** @name SubDRP ADC3 Data
 *
 * This register contains the sub-drp data of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC3_DAT_MASK		0x0000FFFFU	/**< sub-drp3 data
							for read or write transaction */

/* @} */

/** @name RX MC PWRDWN
 *
 * This register contains the static configuration bits of ADC(RX) analog.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_RX_MC_PWRDWN_MASK		0x0000FFFFU	/**< RX MC power down */

/* @} */

/** @name RX MC Config0
 *
 * This register contains the static configuration bits of ADC(RX) analog.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_RX_MC_CFG0_MASK		0x0000FFFFU	/**< RX MC config0 */

/* @} */

/** @name RX MC Config1
 *
 * This register contains the static configuration bits of ADC(RX) analog.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_RX_MC_CFG1_MASK		0x0000FFFFU	/**< RX MC Config1 */

/* @} */

/** @name RX MC Config2
 *
 * This register contains the static configuration bits of ADC(RX) analog.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_RX_MC_CFG2_MASK		0x0000FFFFU	/**< RX MC Config2 */

/* @} */

/** @name RX Pair MC Config0
 *
 * This register contains the RX Pair (RX0 and RX1 or RX2 and RX3)static
 * configuration bits of ADC(RX) analog. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_RX_PR_MC_CFG0_MASK		0x0000FFFFU	/**< RX Pair MC Config0 */

/* @} */

/** @name RX Pair MC Config1
 *
 * This register contains the RX Pair (RX0 and RX1 or RX2 and RX3)static
 * configuration bits of ADC(RX) analog. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_RX_PR_MC_CFG1_MASK		0x0000FFFFU	/**< RX Pair MC Config1 */

/* @} */

/** @name TI DCB Status0 BG
 *
 * This register contains the subadc ch0 ocb1 BG offset correction factor
 * value. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS0_BG_MASK		0x0000FFFFU	/**< DCB Status0 BG */

/* @} */

/** @name TI DCB Status0 FG
 *
 * This register contains the subadc ch0 ocb2 FG offset correction factor
 * value(read and write). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS0_FG_MASK		0x0000FFFFU	/**< DCB Status0 FG */

/* @} */

/** @name TI DCB Status1 BG
 *
 * This register contains the subadc ch1 ocb1 BG offset correction factor
 * value. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS1_BG_MASK		0x0000FFFFU	/**< DCB Status1 BG */

/* @} */

/** @name TI DCB Status1 FG
 *
 * This register contains the subadc ch1 ocb2 FG offset correction factor
 * value(read and write). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS1_FG_MASK		0x0000FFFFU	/**< DCB Status1 FG */

/* @} */

/** @name TI DCB Status2 BG
 *
 * This register contains the subadc ch2 ocb1 BG offset correction factor
 * value. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS2_BG_MASK		0x0000FFFFU	/**< DCB Status2 BG */

/* @} */

/** @name TI DCB Status2 FG
 *
 * This register contains the subadc ch2 ocb2 FG offset correction factor
 * value(read and write). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS2_FG_MASK		0x0000FFFFU	/**< DCB Status2 FG */

/* @} */

/** @name TI DCB Status3 BG
 *
 * This register contains the subadc ch3 ocb1 BG offset correction factor
 * value. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS3_BG_MASK		0x0000FFFFU	/**< DCB Status3 BG */

/* @} */

/** @name TI DCB Status3 FG
 *
 * This register contains the subadc ch3 ocb2 FG offset correction factor
 * value(read and write). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS3_FG_MASK		0x0000FFFFU	/**< DCB Status3 FG */

/* @} */

/** @name TI DCB Status4 MSB
 *
 * This register contains the DCB status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS4_MSB_MASK		0x0000FFFFU	/**< read the status of
							gcb acc0 msb bits(subadc chan0) */

/* @} */

/** @name TI DCB Status4 LSB
 *
 * This register contains the DCB Status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS4_LSB_MASK		0x0000FFFFU	/**< read the status of
							gcb acc0 lsb bits(subadc chan0) */

/* @} */

/** @name TI DCB Status5 MSB
 *
 * This register contains the DCB status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS5_MSB_MASK		0x0000FFFFU	/**< read the status of
							gcb acc1 msb bits(subadc chan1) */

/* @} */

/** @name TI DCB Status5 LSB
 *
 * This register contains the DCB Status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS5_LSB_MASK		0x0000FFFFU	/**< read the status of
							gcb acc1 lsb bits(subadc chan1) */

/* @} */

/** @name TI DCB Status6 MSB
 *
 * This register contains the DCB status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS6_MSB_MASK		0x0000FFFFU	/**< read the status of
							gcb acc2 msb bits(subadc chan2) */

/* @} */

/** @name TI DCB Status6 LSB
 *
 * This register contains the DCB Status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS6_LSB_MASK		0x0000FFFFU	/**< read the status of
							gcb acc2 lsb bits(subadc chan2) */

/* @} */

/** @name TI DCB Status7 MSB
 *
 * This register contains the DCB status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS7_MSB_MASK		0x0000FFFFU	/**< read the status of
							gcb acc3 msb bits(subadc chan3) */

/* @} */

/** @name TI DCB Status7 LSB
 *
 * This register contains the DCB Status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS7_LSB_MASK		0x0000FFFFU	/**< read the status of
							gcb acc3 lsb bits(subadc chan3) */

/* @} */

/** @name FIFO Latency
 *
 * This register contains bits for result, key and done flag.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FIFO_LTNCY_RES_MASK	0x00000FFFU	/**< Latency
							measurement result */
#define XRFDC_FIFO_LTNCY_KEY_MASK		0x00004000U	/**< Latency
							measurement result identification key */
#define XRFDC_FIFO_LTNCY_DONE_MASK		0x00008000U	/**< Latency
							measurement done flag */

/* @} */

/** @name Decoder Control
 *
 * This register contains Unary Decoder/Randomizer settings to use.
 * @{
 */

#define XRFDC_DEC_CTRL_MODE_MASK		0x00000007U	/**< Decoder mode */

/* @} */

/** @name HSCOM Power state mask
 *
 * This register contains HSCOM_PWR to check powerup_state.
 * @{
 */

#define XRFDC_HSCOM_PWR_STATE_MASK		0x0000FFFFU	/**< powerup state mask */

/* @} */

/** @name Interpolation Control
 *
 * This register contains Interpolation filter modes.
 * @{
 */

#define XRFDC_INTERP_MODE_MASK		0x00000007U	/**< Interpolation filter
								mode mask */

/* @} */

/** @name Tile Reset
 *
 * This register contains Tile reset bit.
 * @{
 */

#define XRFDC_TILE_RESET_MASK		0x00000001U	/**< Tile reset mask */

/* @} */

/** @name Status register
 *
 * This register contains common status bits.
 * @{
 */

#define XRFDC_PWR_UP_STAT_MASK		0x00000004U	/**< Power Up state mask */
#define XRFDC_PLL_LOCKED_MASK		0x00000008U	/**< PLL Locked mask */

/* @} */

/** @name Restart State register
 *
 * This register contains Start and End state bits.
 * @{
 */

#define XRFDC_PWR_STATE_MASK		0x0000FFFFU	/**< State mask */

/* @} */

/** @name Common interrupt enable register
 *
 * This register contains bits to enable interrupt for
 * ADC and DAC tiles.
 * @{
 */

#define XRFDC_EN_INTR_DAC_TILE0_MASK		0x00000001U	/**< DAC Tile0
								interrupt enable mask */
#define XRFDC_EN_INTR_DAC_TILE1_MASK		0x00000002U	/**< DAC Tile1
								interrupt enable mask */
#define XRFDC_EN_INTR_DAC_TILE2_MASK		0x00000004U	/**< DAC Tile2
								interrupt enable mask */
#define XRFDC_EN_INTR_DAC_TILE3_MASK		0x00000008U	/**< DAC Tile3
								interrupt enable mask */
#define XRFDC_EN_INTR_ADC_TILE0_MASK		0x00000010U	/**< ADC Tile0
								interrupt enable mask */
#define XRFDC_EN_INTR_ADC_TILE1_MASK		0x00000020U	/**< ADC Tile1
								interrupt enable mask */
#define XRFDC_EN_INTR_ADC_TILE2_MASK		0x00000040U	/**< ADC Tile2
								interrupt enable mask */
#define XRFDC_EN_INTR_ADC_TILE3_MASK		0x00000080U	/**< ADC Tile3
								interrupt enable mask */
/* @} */


/** @name interrupt enable register
 *
 * This register contains bits to enable interrupt for blocks.
 * @{
 */

#define XRFDC_EN_INTR_SLICE0_MASK		0x00000001U	/**< slice0
								interrupt enable mask */
#define XRFDC_EN_INTR_SLICE1_MASK		0x00000002U	/**< slice1
								interrupt enable mask */
#define XRFDC_EN_INTR_SLICE2_MASK		0x00000004U	/**< slice2
								interrupt enable mask */
#define XRFDC_EN_INTR_SLICE3_MASK		0x00000008U	/**< slice3
								interrupt enable mask */
/* @} */

/** @name Converter(X) interrupt register
 *
 * This register contains bits to enable different interrupts for block X.
 * @{
 */

#define XRFDC_INTR_OVR_RANGE_MASK		0x00000008U	/**< Over Range
								interrupt mask */
#define XRFDC_INTR_OVR_VOLTAGE_MASK		0x00000004U	/**< Over Voltage
								interrupt mask */
/* @} */

#define XRFDC_IXR_FIFOUSRDAT_MASK			0x0000000FU
#define XRFDC_IXR_FIFOUSRDAT_OF_MASK		0x00000001U
#define XRFDC_IXR_FIFOUSRDAT_UF_MASK 		0x00000002U
#define XRFDC_IXR_FIFOMRGNIND_OF_MASK 		0x00000004U
#define XRFDC_IXR_FIFOMRGNIND_UF_MASK 		0x00000008U
#define XRFDC_ADC_IXR_DATAPATH_MASK			0x00000FF0U
#define XRFDC_ADC_IXR_DMON_STG_MASK			0x000003F0U
#define XRFDC_DAC_IXR_DATAPATH_MASK			0x00001FF0U
#define XRFDC_DAC_IXR_INTP_STG_MASK 		0x000003F0U
#define XRFDC_DAC_IXR_INTP_I_STG0_MASK 		0x00000010U
#define XRFDC_DAC_IXR_INTP_I_STG1_MASK 		0x00000020U
#define XRFDC_DAC_IXR_INTP_I_STG2_MASK 		0x00000040U
#define XRFDC_DAC_IXR_INTP_Q_STG0_MASK 		0x00000080U
#define XRFDC_DAC_IXR_INTP_Q_STG1_MASK 		0x00000100U
#define XRFDC_DAC_IXR_INTP_Q_STG2_MASK 		0x00000200U
#define XRFDC_ADC_IXR_DMON_I_STG0_MASK 		0x00000010U
#define XRFDC_ADC_IXR_DMON_I_STG1_MASK 		0x00000020U
#define XRFDC_ADC_IXR_DMON_I_STG2_MASK 		0x00000040U
#define XRFDC_ADC_IXR_DMON_Q_STG0_MASK 		0x00000080U
#define XRFDC_ADC_IXR_DMON_Q_STG1_MASK 		0x00000100U
#define XRFDC_ADC_IXR_DMON_Q_STG2_MASK 		0x00000200U
#define XRFDC_IXR_QMC_GAIN_PHASE_MASK 		0x00000400U
#define XRFDC_IXR_QMC_OFFST_MASK 			0x00000800U
#define XRFDC_DAC_IXR_INVSNC_OF_MASK 		0x00001000U
#define XRFDC_SUBADC_IXR_DCDR_MASK 			0x00FF0000U
#define XRFDC_SUBADC0_IXR_DCDR_OF_MASK 		0x00010000U
#define XRFDC_SUBADC0_IXR_DCDR_UF_MASK 		0x00020000U
#define XRFDC_SUBADC1_IXR_DCDR_OF_MASK 		0x00040000U
#define XRFDC_SUBADC1_IXR_DCDR_UF_MASK 		0x00080000U
#define XRFDC_SUBADC2_IXR_DCDR_OF_MASK 		0x00100000U
#define XRFDC_SUBADC2_IXR_DCDR_UF_MASK 		0x00200000U
#define XRFDC_SUBADC3_IXR_DCDR_OF_MASK 		0x00400000U
#define XRFDC_SUBADC3_IXR_DCDR_UF_MASK 		0x00800000U
#define XRFDC_ADC_OVR_VOLTAGE_MASK			0x04000000U
#define XRFDC_ADC_OVR_RANGE_MASK			0x08000000U
#define XRFDC_DAC_MC_CFG2_OPCSCAS_MASK		0x0000F8F8U
#define XRFDC_DAC_MC_CFG3_CSGAIN_MASK		0x0000FFC0U
#define XRFDC_DAC_MC_CFG2_OPCSCAS_20MA		0x00004858U
#define XRFDC_DAC_MC_CFG3_CSGAIN_20MA		0x000087C0U
#define XRFDC_DAC_MC_CFG2_OPCSCAS_32MA		0x0000A0D8U
#define XRFDC_DAC_MC_CFG3_CSGAIN_32MA		0x0000FFC0U

#define XRFDC_DAC_TILE_DRP_ADDR(X)			(0x6000 + (X * 0x4000))
#define XRFDC_DAC_TILE_CTRL_STATS_ADDR(X)	(0x4000 + (X * 0x4000))
#define XRFDC_ADC_TILE_DRP_ADDR(X)			(0x16000 + (X * 0x4000))
#define XRFDC_ADC_TILE_CTRL_STATS_ADDR(X)	(0x14000 + (X * 0x4000))
#define XRFDC_CTRL_STATS_OFFSET		0x0
#define XRFDC_HSCOM_ADDR	0x1C00
#define XRFDC_BLOCK_ADDR_OFFSET(X)	(X * 0x400)

/***************** Macros (Inline Functions) Definitions *********************/
#ifdef __MICROBLAZE__
#define XRFdc_In64 Xil_In64
#define XRFdc_Out64 Xil_Out64

#define XRFdc_In32 Xil_In32
#define XRFdc_Out32 Xil_Out32

#define XRFdc_In16 Xil_In16
#define XRFdc_Out16 Xil_Out16

#define XRFdc_In8 Xil_In8
#define XRFdc_Out8 Xil_Out8
#else
#define XRFdc_In64 metal_io_read64
#define XRFdc_Out64 metal_io_write64

#define XRFdc_In32 metal_io_read32
#define XRFdc_Out32 metal_io_write32

#define XRFdc_In16 metal_io_read16
#define XRFdc_Out16 metal_io_write16

#define XRFdc_In8 metal_io_read8
#define XRFdc_Out8 metal_io_write8
#endif

/****************************************************************************/
/**
* Read a register.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to the target register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XRFdc_ReadReg(XRFdc *InstancePtr. u32 BaseAddress, s32 RegOffset)
*
******************************************************************************/
#ifdef __MICROBLAZE__
#define XRFdc_ReadReg64(InstancePtr, BaseAddress, RegOffset) \
	XRFdc_In64(InstancePtr->BaseAddr + BaseAddress + RegOffset)
#else
#define XRFdc_ReadReg64(InstancePtr, BaseAddress, RegOffset) \
	XRFdc_In64(InstancePtr->io, (RegOffset + BaseAddress))
#endif

/***************************************************************************/
/**
* Write to a register.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*			device to target register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XRFdc_WriteReg(XRFdc *InstancePtr, u32 BaseAddress, s32 RegOffset,
*		u64 RegisterValue)
*
******************************************************************************/
#ifdef __MICROBLAZE__
#define XRFdc_WriteReg64(InstancePtr, BaseAddress, RegOffset, RegisterValue) \
	XRFdc_Out64((InstancePtr->BaseAddr + BaseAddress) + (RegOffset), \
		(RegisterValue))
#else
#define XRFdc_WriteReg64(InstancePtr, BaseAddress, RegOffset, RegisterValue) \
	XRFdc_Out64((InstancePtr->io), (RegOffset + BaseAddress), \
		(RegisterValue))
#endif

/****************************************************************************/
/**
* Read a register.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to the target register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XRFdc_ReadReg(XRFdc *InstancePtr, u32 BaseAddress. int RegOffset)
*
******************************************************************************/
#ifdef __MICROBLAZE__
#define XRFdc_ReadReg(InstancePtr, BaseAddress, RegOffset) \
	XRFdc_In32((InstancePtr->BaseAddr + BaseAddress) + (RegOffset))
#else
#define XRFdc_ReadReg(InstancePtr, BaseAddress, RegOffset) \
	XRFdc_In32((InstancePtr->io), (BaseAddress + RegOffset))
#endif

/***************************************************************************/
/**
* Write to a register.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to target register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XRFdc_WriteReg(XRFdc *InstancePtr, u32 BaseAddress, int RegOffset,
*		u32 RegisterValue)
*
******************************************************************************/
#ifdef __MICROBLAZE__
#define XRFdc_WriteReg(InstancePtr, BaseAddress, RegOffset, RegisterValue) \
	XRFdc_Out32((InstancePtr->BaseAddr + BaseAddress) + (RegOffset), (RegisterValue))
#else
#define XRFdc_WriteReg(InstancePtr, BaseAddress, RegOffset, RegisterValue) \
	XRFdc_Out32((InstancePtr->io), (RegOffset + BaseAddress), (RegisterValue))
#endif

/****************************************************************************/
/**
* Read a register.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to the target register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u16 XRFdc_ReadReg(XRFdc *InstancePtr, u32 BaseAddress. int RegOffset)
*
******************************************************************************/
#ifdef __MICROBLAZE__
#define XRFdc_ReadReg16(InstancePtr, BaseAddress, RegOffset) \
	XRFdc_In16((InstancePtr->BaseAddr + BaseAddress) + (RegOffset))
#else
#define XRFdc_ReadReg16(InstancePtr, BaseAddress, RegOffset) \
	XRFdc_In16((InstancePtr->io), (RegOffset + BaseAddress))
#endif

/***************************************************************************/
/**
* Write to a register.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to target register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XRFdc_WriteReg(XRFdc *InstancePtr, u32 BaseAddress, int RegOffset,
*		u16 RegisterValue)
*
******************************************************************************/
#ifdef __MICROBLAZE__
#define XRFdc_WriteReg16(InstancePtr, BaseAddress, RegOffset, RegisterValue) \
	XRFdc_Out16((InstancePtr->BaseAddr + BaseAddress) + (RegOffset), (RegisterValue))
#else
#define XRFdc_WriteReg16(InstancePtr, BaseAddress, RegOffset, RegisterValue) \
	XRFdc_Out16((InstancePtr->io), (RegOffset + BaseAddress), (RegisterValue))
#endif

/****************************************************************************/
/**
* Read a register.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to the target register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u8 XRFdc_ReadReg(XRFdc *InstancePtr, u32 BaseAddress. int RegOffset)
*
******************************************************************************/
#ifdef __MICROBLAZE__
#define XRFdc_ReadReg8(InstancePtr, BaseAddress, RegOffset) \
	XRFdc_In8((InstancePtr->BaseAddr + BaseAddress) + (RegOffset))
#else
#define XRFdc_ReadReg8(InstancePtr, BaseAddress, RegOffset) \
	XRFdc_In8((InstancePtr->io), (RegOffset + BaseAddress))
#endif

/***************************************************************************/
/**
* Write to a register.
*
* @param	InstancePtr is a pointer to the XRfdc instance.
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to target register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XRFdc_WriteReg(XRFdc *InstancePtr, u32 BaseAddress, int RegOffset,
*		u8 RegisterValue)
*
******************************************************************************/
#ifdef __MICROBLAZE__
#define XRFdc_WriteReg8(InstancePtr, BaseAddress, RegOffset, RegisterValue) \
	XRFdc_Out8((InstancePtr->BaseAddr + BaseAddress) + (RegOffset), (RegisterValue))
#else
#define XRFdc_WriteReg8(InstancePtr, BaseAddress, RegOffset, RegisterValue) \
	XRFdc_Out8((InstancePtr->io), (RegOffset + BaseAddress), (RegisterValue))
#endif

#ifdef __cplusplus
}
#endif

#endif /* RFDC_HW_H_ */
/** @} */
