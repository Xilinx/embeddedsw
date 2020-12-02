/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xrfdc_hw.h
* @addtogroup rfdc_v8_1
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
* 2.3   sk     11/10/17 Corrected FIFO and DATA Interrupt masks.
* 2.4   sk     12/11/17 Added DDC and DUC support.
* 3.0   sg     13/01/18 Added PLL and external clock switch support
* 3.1   jm     01/24/18 Add Multi-tile sync support.
*       sk     02/27/18 Add API's to configure Multiband.
* 4.0   sk     04/09/18 Removed redundant inclusion of xparameters.h file.
* 5.0   sk     08/03/18 Fixed MISRAC warnings.
*       sk     08/24/18 Reorganize the code to improve readability and
*                       optimization.
* 5.1   cog    01/29/19 Added XRFdc_SetDither() and XRFdc_GetDither() APIs.
* 6.0   cog    02/17/19 New Interp/Decimation Mask.
*       cog    02/17/19 Added new Inverse-Sinc mask.
*       cog    02/17/19 Added new clock Distribution Defs.
*       cog    02/17/19 Added new intratile clock Defs.
*       cog    02/17/19 New Masks and offsets for XRFdc_GetPLLConfig() API.
*       cog    02/17/19 New Masks and offsets for  XRFdc_SetIMRPassMode() and
*                       XRFdc_SetIMRPassMode() APIs
*       cog    02/17/19 New Masks and offsets for XRFdc_SetDACMode() and
*                       XRFdc_GetDACMode() APIs
*       cog    02/17/19	New Masks and offsets for XRFdc_SetSignalDetector() and
*                       XRFdc_GetSignalDetector() APIs.
*       cog    02/17/19 New Masks and offsets for XRFdc_DisableCoefficientsOverride(),
*                       XRFdc_SetCalCoefficients and XRFdc_GetCalCoefficients APIs.
*       cog    02/19/19 New Masks and offsets for clock detection register.
*       cog    02/20/19 New Masks for ADC common mode over/under voltage interrupts.
*       cog    02/21/19 New Masks and offsets for XRFdc_SetCalFreeze() and
*                       XRFdc_GetCalFreeze() APIs.
*       cog    03/25/19 The new common mode over/under voltage interrupts mask
*                       bits were clashing with other interrupt bits.
*       cog    03/25/19 Added more calibration bypass masks.
* 7.0   cog    05/13/19 Formatting changes.
*       cog    05/13/19 Added common power up interrupt masks/shifts.
*       cog    07/16/19 Added XRFdc_SetDACOpCurr() API masks/shifts.
*       cog    07/18/19 Added XRFdc_S/GetDigitalStepAttenuator() API masks/shifts.
*       cog    07/26/19 Added XRFdc_S/GetLegacyCompatibilityMode() API shift.
*       cog    08/02/19 Formatting changes.
*       cog    09/01/19 Added offset for VOP control register.
*       cog    09/01/19 Added masks/shifts for DSA.
*       cog    09/18/19 Wider mask now needed for DAC Fabric Rate.
*       cog    09/18/19 Added mask for bypassing PLL output divider.
*       cog    10/02/19 Added mask for clock divider.
*       cog    10/02/19 Added mask for PLL output clock divider.
* 7.1   cog    11/15/19 Added offsets & masks for calibration mode support for
*                       Gen 3 devices.
*       cog    11/28/19 Added offset & shift for datapath modes.
*       cog    01/03/19 Change shift and mask for alternate bondout devices.
*       cog    01/23/20 Fixed shift and mask for GCB calibration override operations
*                       in Gen 3 Devices.
* 8.0   cog    02/10/20 Updated addtogroup.
*       cog    02/17/20 Added masks and shifts for tile/path enables.
*       cog    02/20/20 Added offsets, masks and shifts for FIFO delays.
*       cog    03/20/20 Added masks and shifts for power state mask.
*       cog    03/20/20 Added masks and shifts for datapath clock enables.
* 8.1   cog    06/24/20 Upversion.
*       cog    06/24/20 Masks, shifts and offsets for production Si VOP.
*       cog    06/24/20 Masks and offsets for MB config.
*       cog    06/24/20 Masks, shifts and offsets or observaion FIFO.
*       cog    06/24/20 Masks, shifts and offsets for TDD Mode registers.
*       cog    08/11/20 Refactor of clock distribution settings.
*       cog    08/28/20 Fixed bug in the ADC powerdown mode.
*       cog    09/08/20 The Four LSBs of the BLDR Bias Current should be the same as the
*                       four LSBs of the CS Gain.
*       cog    09/28/20 Added more DAC interrupts and fixed issue with
*                       GetEnabledInterrupts.
*       cog    10/14/20 Get I and Q data now supports warm bitstream swap.
*
*</pre>
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
#include "metal/io.h"
/************************** Constant Definitions *****************************/

/** @name Register Map
 *
 * Register offsets from the base address of an RFDC ADC and DAC device.
 * @{
 */

#define XRFDC_CLK_EN_OFFSET 0x000U /**< ADC Clock Enable Register */
#define XRFDC_ADC_DEBUG_RST_OFFSET 0x004U /**< ADC Debug Reset Register */
#define XRFDC_ADC_FABRIC_RATE_OFFSET 0x008U /**< ADC Fabric Rate Register */
#define XRFDC_ADC_FABRIC_RATE_OBS_OFFSET 0x050U /**< ADC Obs Fabric Rate Register */
#define XRFDC_ADC_FABRIC_RATE_TDD_OFFSET(X)                                                                            \
	((X == 0) ? XRFDC_ADC_FABRIC_RATE_OFFSET :                                                                     \
		    XRFDC_ADC_FABRIC_RATE_OBS_OFFSET) /**< ADC Fabric Rate (or OBS) Register TDD Selected */
#define XRFDC_DAC_FABRIC_RATE_OFFSET 0x008U /**< DAC Fabric Rate Register */
#define XRFDC_ADC_FABRIC_OFFSET 0x00CU /**< ADC Fabric Register */
#define XRFDC_ADC_FABRIC_OBS_OFFSET 0x054U /**< ADC Obs Fabric Register */
#define XRFDC_ADC_FABRIC_TDD_OFFSET(X)                                                                                 \
	((X == 0) ? XRFDC_ADC_FABRIC_OFFSET :                                                                          \
		    XRFDC_ADC_FABRIC_OBS_OFFSET) /**< ADC Fabric Register (or OBS) TDD Selected*/
#define XRFDC_ADC_FABRIC_ISR_OFFSET 0x010U /**< ADC Fabric ISR Register */
#define XRFDC_DAC_FIFO_START_OFFSET 0x010U /**< DAC FIFO Start Register */
#define XRFDC_DAC_FABRIC_ISR_OFFSET 0x014U /**< DAC Fabric ISR Register */
#define XRFDC_ADC_FABRIC_IMR_OFFSET 0x014U /**< ADC Fabric IMR Register */
#define XRFDC_DAC_FABRIC_IMR_OFFSET 0x018U /**< DAC Fabric IMR Register */
#define XRFDC_ADC_FABRIC_DBG_OFFSET 0x018U /**< ADC Fabric Debug Register */
#define XRFDC_ADC_FABRIC_DBG_OBS_OFFSET 0x058U /**< ADC Obs Fabric Debug Register */
#define XRFDC_ADC_FABRIC_DBG_TDD_OFFSET(X)                                                                             \
	((X == 0) ? XRFDC_ADC_FABRIC_DBG_OFFSET :                                                                      \
		    XRFDC_ADC_FABRIC_DBG_OBS_OFFSET) /**< ADC Fabric Debug (or OBS) Register TDD Selected */
#define XRFDC_ADC_UPDATE_DYN_OFFSET 0x01CU /**< ADC Update Dynamic Register */
#define XRFDC_DAC_UPDATE_DYN_OFFSET 0x020U /**< DAC Update Dynamic Register */
#define XRFDC_ADC_FIFO_LTNC_CRL_OFFSET 0x020U /**< ADC FIFO Latency Control Register */
#define XRFDC_ADC_FIFO_LTNC_CRL_OBS_OFFSET 0x064U /**< ADC Obs FIFO Latency Control Register */
#define XRFDC_ADC_FIFO_LTNC_CRL_TDD_OFFSET(X)                                                                          \
	((X == 0) ?                                                                                                    \
		 XRFDC_ADC_FIFO_LTNC_CRL_OFFSET :                                                                      \
		 XRFDC_ADC_FIFO_LTNC_CRL_OBS_OFFSET) /**< ADC FIFO Latency Control (or OBS) Register TDD Selected */
#define XRFDC_ADC_DEC_ISR_OFFSET 0x030U /**< ADC Decoder interface ISR Register */
#define XRFDC_DAC_DATAPATH_OFFSET 0x034U /**< ADC Decoder interface IMR Register */
#define XRFDC_ADC_DEC_IMR_OFFSET 0x034U /**< ADC Decoder interface IMR Register */
#define XRFDC_DATPATH_ISR_OFFSET 0x038U /**< ADC Data Path ISR Register */
#define XRFDC_DATPATH_IMR_OFFSET 0x03CU /**< ADC Data Path IMR Register */
#define XRFDC_ADC_DECI_CONFIG_OFFSET 0x040U /**< ADC Decimation Config Register */
#define XRFDC_ADC_DECI_CONFIG_OBS_OFFSET 0x048U /**< ADC Decimation Config Register */
#define XRFDC_ADC_DECI_CONFIG_TDD_OFFSET(X)                                                                            \
	((X == 0) ? XRFDC_ADC_DECI_CONFIG_OFFSET :                                                                     \
		    XRFDC_ADC_DECI_CONFIG_OBS_OFFSET) /**< ADC Decimation Config (or OBS) Register TDD Selected */
#define XRFDC_DAC_INTERP_CTRL_OFFSET 0x040U /**< DAC Interpolation Control Register */
#define XRFDC_ADC_DECI_MODE_OFFSET 0x044U /**< ADC Decimation mode Register */
#define XRFDC_ADC_DECI_MODE_OBS_OFFSET 0x04CU /**< ADC Obs Decimation mode Register */
#define XRFDC_ADC_DECI_MODE_TDD_OFFSET(X)                                                                              \
	((X == 0) ? XRFDC_ADC_DECI_MODE_OFFSET :                                                                       \
		    XRFDC_ADC_DECI_MODE_OBS_OFFSET) /**< ADC Decimation mode (or OBS) Register TDD Selected */
#define XRFDC_DAC_ITERP_DATA_OFFSET 0x044U /**< DAC interpolation data */
#define XRFDC_ADC_FABRIC_ISR_OBS_OFFSET 0x05CU /**< ADC Fabric ISR Observation Register */
#define XRFDC_ADC_FABRIC_IMR_OBS_OFFSET 0x060U /**< ADC Fabric ISR Observation Register */
#define XRFDC_DAC_TDD_MODE0_OFFSET 0x060U /**< DAC TDD Mode 0 Configuration*/
#define XRFDC_ADC_TDD_MODE0_OFFSET 0x068U /**< ADC TDD Mode 0 Configuration*/
#define XRFDC_TDD_MODE0_OFFSET(X)                                                                                      \
	((X == 0) ? XRFDC_ADC_TDD_MODE0_OFFSET : XRFDC_DAC_TDD_MODE0_OFFSET) /**< ADC TDD Mode 0 Configuration*/
#define XRFDC_ADC_MXR_CFG0_OFFSET 0x080U /**< ADC I channel mixer config Register */
#define XRFDC_ADC_MXR_CFG1_OFFSET 0x084U /**< ADC Q channel mixer config Register */
#define XRFDC_MXR_MODE_OFFSET 0x088U /**< ADC/DAC mixer mode Register */
#define XRFDC_NCO_UPDT_OFFSET 0x08CU /**< ADC/DAC NCO Update mode Register */
#define XRFDC_NCO_RST_OFFSET 0x090U /**< ADC/DAC NCO Phase Reset Register */
#define XRFDC_ADC_NCO_FQWD_UPP_OFFSET 0x094U /**< ADC NCO Frequency Word[47:32] Register */
#define XRFDC_ADC_NCO_FQWD_MID_OFFSET 0x098U /**< ADC NCO Frequency Word[31:16] Register */
#define XRFDC_ADC_NCO_FQWD_LOW_OFFSET 0x09CU /**< ADC NCO Frequency Word[15:0] Register */
#define XRFDC_NCO_PHASE_UPP_OFFSET 0x0A0U /**< ADC/DAC NCO Phase[17:16] Register */
#define XRFDC_NCO_PHASE_LOW_OFFSET 0x0A4U /**< ADC/DAC NCO Phase[15:0] Register */
#define XRFDC_ADC_NCO_PHASE_MOD_OFFSET 0x0A8U /**< ADC NCO Phase Mode Register */
#define XRFDC_QMC_UPDT_OFFSET 0x0C8U /**< ADC/DAC QMC Update Mode Register */
#define XRFDC_QMC_CFG_OFFSET 0x0CCU /**< ADC/DAC QMC Config Register */
#define XRFDC_QMC_OFF_OFFSET 0x0D0U /**< ADC/DAC QMC Offset Correction Register */
#define XRFDC_QMC_GAIN_OFFSET 0x0D4U /**< ADC/DAC QMC Gain Correction Register */
#define XRFDC_QMC_PHASE_OFFSET 0x0D8U /**< ADC/DAC QMC Phase Correction Register */
#define XRFDC_ADC_CRSE_DLY_UPDT_OFFSET 0x0DCU /**< ADC Coarse Delay Update Register */
#define XRFDC_DAC_CRSE_DLY_UPDT_OFFSET 0x0E0U /**< DAC Coarse Delay Update Register */
#define XRFDC_ADC_CRSE_DLY_CFG_OFFSET 0x0E0U /**< ADC Coarse delay Config Register */
#define XRFDC_DAC_CRSE_DLY_CFG_OFFSET 0x0DCU /**< DAC Coarse delay Config Register */
#define XRFDC_ADC_DAT_SCAL_CFG_OFFSET 0x0E4U /**< ADC Data Scaling Config Register */
#define XRFDC_ADC_SWITCH_MATRX_OFFSET 0x0E8U /**< ADC Switch Matrix Config Register */
#define XRFDC_ADC_TRSHD0_CFG_OFFSET 0x0ECU /**< ADC Threshold0 Config Register */
#define XRFDC_ADC_TRSHD0_AVG_UP_OFFSET 0x0F0U /**< ADC Threshold0 Average[31:16] Register */
#define XRFDC_ADC_TRSHD0_AVG_LO_OFFSET 0x0F4U /**< ADC Threshold0 Average[15:0] Register */
#define XRFDC_ADC_TRSHD0_UNDER_OFFSET 0x0F8U /**< ADC Threshold0 Under Threshold Register */
#define XRFDC_ADC_TRSHD0_OVER_OFFSET 0x0FCU /**< ADC Threshold0 Over Threshold Register */
#define XRFDC_ADC_TRSHD1_CFG_OFFSET 0x100U /**< ADC Threshold1 Config Register */
#define XRFDC_ADC_TRSHD1_AVG_UP_OFFSET 0x104U /**< ADC Threshold1 Average[31:16] Register */
#define XRFDC_ADC_TRSHD1_AVG_LO_OFFSET 0x108U /**< ADC Threshold1 Average[15:0] Register */
#define XRFDC_ADC_TRSHD1_UNDER_OFFSET 0x10CU /**< ADC Threshold1 Under Threshold Register */
#define XRFDC_ADC_TRSHD1_OVER_OFFSET 0x110U /**< ADC Threshold1 Over Threshold Register */
#define XRFDC_ADC_FEND_DAT_CRL_OFFSET 0x140U /**< ADC Front end Data Control Register */
#define XRFDC_ADC_TI_DCB_CRL0_OFFSET 0x144U /**< ADC Time Interleaved digital correction block gain control0 Register */
#define XRFDC_ADC_TI_DCB_CRL1_OFFSET 0x148U /**< ADC Time Interleaved digital correction block gain control1 Register */
#define XRFDC_ADC_TI_DCB_CRL2_OFFSET 0x14CU /**< ADC Time Interleaved digital correction block gain control2 Register */
#define XRFDC_ADC_TI_DCB_CRL3_OFFSET 0x150U /**< ADC Time Interleaved digital correction block gain control3 Register */
#define XRFDC_ADC_TI_TISK_CRL0_OFFSET 0x154U /**< ADC Time skew correction control bits0 Register */
#define XRFDC_DAC_MC_CFG0_OFFSET 0x1C4U /**< Static Configuration  data for DAC Analog */
#define XRFDC_ADC_TI_TISK_CRL1_OFFSET 0x158U /**< ADC Time skew correction control bits1 Register */
#define XRFDC_ADC_TI_TISK_CRL2_OFFSET 0x15CU /**< ADC Time skew correction control bits2 Register */
#define XRFDC_ADC_TI_TISK_CRL3_OFFSET 0x160U /**< ADC Time skew correction control bits3 Register */
#define XRFDC_ADC_TI_TISK_CRL4_OFFSET 0x164U /**< ADC Time skew correction control bits4 Register */
#define XRFDC_ADC_TI_TISK_CRL5_OFFSET 0x168U /**< ADC Time skew correction control bits5 Register (Gen 3 only) */
#define XRFDC_ADC_TI_TISK_DAC0_OFFSET 0x168U /**< ADC Time skew DAC cal code of subadc ch0 Register(Below Gen 3) */
#define XRFDC_ADC_TI_TISK_DAC1_OFFSET 0x16CU /**< ADC Time skew DAC cal code of subadc ch1 Register */
#define XRFDC_ADC_TI_TISK_DAC2_OFFSET 0x170U /**< ADC Time skew DAC cal code of subadc ch2 Register */
#define XRFDC_ADC_TI_TISK_DAC3_OFFSET 0x174U /**< ADC Time skew DAC cal code of subadc ch3 Register */
#define XRFDC_ADC_TI_TISK_DACP0_OFFSET 0x178U /**< ADC Time skew DAC cal code of subadc ch0 Register */
#define XRFDC_ADC_TI_TISK_DACP1_OFFSET 0x17CU /**< ADC Time skew DAC cal code of subadc ch1 Register */
#define XRFDC_ADC_TI_TISK_DACP2_OFFSET 0x180U /**< ADC Time skew DAC cal code of subadc ch2 Register */
#define XRFDC_ADC_TI_TISK_DACP3_OFFSET 0x184U /**< ADC Time skew DAC cal code of subadc ch3 Register */
#define XRFDC_DAC_VOP_CTRL_OFFSET 0x198U /**< DAC variable output power control Register */
#define XRFDC_ADC0_SUBDRP_ADDR_OFFSET 0x198U /**< subadc0, sub-drp address of target Register */
#define XRFDC_ADC0_SUBDRP_DAT_OFFSET 0x19CU /**< subadc0, sub-drp data of target Register */
#define XRFDC_ADC1_SUBDRP_ADDR_OFFSET 0x1A0U /**< subadc1, sub-drp address of target Register */
#define XRFDC_ADC1_SUBDRP_DAT_OFFSET 0x1A4U /**< subadc1, sub-drp data of target Register */
#define XRFDC_ADC2_SUBDRP_ADDR_OFFSET 0x1A8U /**< subadc2, sub-drp address of target Register */
#define XRFDC_ADC2_SUBDRP_DAT_OFFSET 0x1ACU /**< subadc2, sub-drp data of target Register */
#define XRFDC_ADC3_SUBDRP_ADDR_OFFSET 0x1B0U /**< subadc3, sub-drp address of target Register */
#define XRFDC_ADC3_SUBDRP_DAT_OFFSET 0x1B4U /**< subadc3, sub-drp data of target Register */
#define XRFDC_ADC_RX_MC_PWRDWN_OFFSET 0x1C0U /**< ADC Static configuration bits for ADC(RX) analog Register */
#define XRFDC_ADC_DAC_MC_CFG0_OFFSET 0x1C4U /**< ADC/DAC Static configuration bits for ADC/DAC analog Register */
#define XRFDC_ADC_DAC_MC_CFG1_OFFSET 0x1C8U /**< ADC/DAC Static configuration bits for ADC/DAC analog Register */
#define XRFDC_ADC_DAC_MC_CFG2_OFFSET 0x1CCU /**< ADC/DAC Static configuration bits for ADC/DAC analog Register */
#define XRFDC_DAC_MC_CFG3_OFFSET 0x1D0U /**< DAC Static configuration bits for DAC analog Register */
#define XRFDC_ADC_RXPR_MC_CFG0_OFFSET 0x1D0U /**< ADC RX Pair static Configuration Register */
#define XRFDC_ADC_RXPR_MC_CFG1_OFFSET 0x1D4U /**< ADC RX Pair static Configuration Register */
#define XRFDC_ADC_TI_DCBSTS0_BG_OFFSET 0x200U /**< ADC DCB Status0 BG Register */
#define XRFDC_ADC_TI_DCBSTS0_FG_OFFSET 0x204U /**< ADC DCB Status0 FG Register */
#define XRFDC_ADC_TI_DCBSTS1_BG_OFFSET 0x208U /**< ADC DCB Status1 BG Register */
#define XRFDC_ADC_TI_DCBSTS1_FG_OFFSET 0x20CU /**< ADC DCB Status1 FG Register */
#define XRFDC_ADC_TI_DCBSTS2_BG_OFFSET 0x210U /**< ADC DCB Status2 BG Register */
#define XRFDC_ADC_TI_DCBSTS2_FG_OFFSET 0x214U /**< ADC DCB Status2 FG Register */
#define XRFDC_ADC_TI_DCBSTS3_BG_OFFSET 0x218U /**< ADC DCB Status3 BG Register */
#define XRFDC_ADC_TI_DCBSTS3_FG_OFFSET 0x21CU /**< ADC DCB Status3 FG Register */
#define XRFDC_ADC_TI_DCBSTS4_MB_OFFSET 0x220U /**< ADC DCB Status4 MSB Register */
#define XRFDC_ADC_TI_DCBSTS4_LB_OFFSET 0x224U /**< ADC DCB Status4 LSB Register */
#define XRFDC_ADC_TI_DCBSTS5_MB_OFFSET 0x228U /**< ADC DCB Status5 MSB Register */
#define XRFDC_ADC_TI_DCBSTS5_LB_OFFSET 0x22CU /**< ADC DCB Status5 LSB Register */
#define XRFDC_ADC_TI_DCBSTS6_MB_OFFSET 0x230U /**< ADC DCB Status6 MSB Register */
#define XRFDC_ADC_TI_DCBSTS6_LB_OFFSET 0x234U /**< ADC DCB Status6 LSB Register */
#define XRFDC_ADC_TI_DCBSTS7_MB_OFFSET 0x238U /**< ADC DCB Status7 MSB Register */
#define XRFDC_ADC_TI_DCBSTS7_LB_OFFSET 0x23CU /**< ADC DCB Status7 LSB Register */
#define XRFDC_DSA_UPDT_OFFSET 0x254U /**< ADC DSA Update Trigger REgister */
#define XRFDC_ADC_FIFO_LTNCY_LB_OFFSET 0x280U /**< ADC FIFO Latency measurement LSB Register */
#define XRFDC_ADC_FIFO_LTNCY_MB_OFFSET 0x284U /**< ADC FIFO Latency measurement MSB Register */
#define XRFDC_DAC_DECODER_CTRL_OFFSET 0x180U /**< DAC Unary Decoder/ Randomizer settings */
#define XRFDC_DAC_DECODER_CLK_OFFSET 0x184U /**< Decoder Clock enable */
#define XRFDC_MB_CONFIG_OFFSET 0x308U /**< Multiband Config status */

#define XRFDC_ADC_SIG_DETECT_CTRL_OFFSET 0x114 /**< ADC Signal Detector Control */
#define XRFDC_ADC_SIG_DETECT_THRESHOLD0_LEVEL_OFFSET 0x118 /**< ADC Signal Detector Threshold 0 */
#define XRFDC_ADC_SIG_DETECT_THRESHOLD0_CNT_ON_OFFSET 0x11C /**< ADC Signal Detector Threshold 0 on Counter */
#define XRFDC_ADC_SIG_DETECT_THRESHOLD0_CNT_OFF_OFFSET 0x120 /**< ADC Signal Detector Threshold 0 off Counter */
#define XRFDC_ADC_SIG_DETECT_THRESHOLD1_LEVEL_OFFSET 0x124 /**< ADC Signal Detector Threshold 1 */
#define XRFDC_ADC_SIG_DETECT_THRESHOLD1_CNT_ON_OFFSET 0x128 /**< ADC Signal Detector Threshold 1 on Counter */
#define XRFDC_ADC_SIG_DETECT_THRESHOLD1_CNT_OFF_OFFSET 0x12C /**< ADC Signal Detector Threshold 1 off Counter */
#define XRFDC_ADC_SIG_DETECT_MAGN_OFFSET 0x130 /**< ADC Signal Detector Magintude */

#define XRFDC_HSCOM_CLK_DSTR_OFFSET 0x088U /**< Clock Distribution Register*/
#define XRFDC_HSCOM_CLK_DSTR_MASK 0xC788U /**< Clock Distribution Register*/
#define XRFDC_HSCOM_CLK_DSTR_MASK_ALT 0x1870U /**< Clock Distribution Register for Intratile*/
#define XRFDC_HSCOM_PWR_OFFSET 0x094 /**< Control register during power-up sequence */
#define XRFDC_HSCOM_CLK_DIV_OFFSET 0xB0 /**< Fabric clk out divider */
#define XRFDC_HSCOM_PWR_STATE_OFFSET 0xB4 /**< Check powerup state */
#define XRFDC_HSCOM_UPDT_DYN_OFFSET 0x0B8 /**< Trigger the update dynamic event */
#define XRFDC_HSCOM_EFUSE_2_OFFSET 0x144
#define XRFDC_DAC_INVSINC_OFFSET 0x0C0U /**< Invsinc control */
#define XRFDC_DAC_MB_CFG_OFFSET 0x0C4U /**< Multiband config */
#define XRFDC_MTS_SRDIST 0x1CA0U
#define XRFDC_MTS_SRCAP_T1 (0x24U << 2U)
#define XRFDC_MTS_SRCAP_PLL (0x0CU << 2U)
#define XRFDC_MTS_SRCAP_DIG (0x2CU << 2U)
#define XRFDC_MTS_SRDTC_T1 (0x27U << 2U)
#define XRFDC_MTS_SRDTC_PLL (0x26U << 2U)
#define XRFDC_MTS_SRFLAG (0x49U << 2U)
#define XRFDC_MTS_CLKSTAT (0x24U << 2U)
#define XRFDC_MTS_SRCOUNT_CTRL 0x004CU
#define XRFDC_MTS_SRCOUNT_VAL 0x0050U
#define XRFDC_MTS_SRFREQ_VAL 0x0054U
#define XRFDC_MTS_FIFO_CTRL_ADC 0x0010U
#define XRFDC_MTS_FIFO_CTRL_DAC 0x0014U
#define XRFDC_MTS_DELAY_CTRL 0x0028U
#define XRFDC_MTS_ADC_MARKER 0x0018U
#define XRFDC_MTS_ADC_MARKER_CNT 0x0010U
#define XRFDC_MTS_DAC_MARKER_CTRL 0x0048U
#define XRFDC_MTS_DAC_MARKER_CNT (0x92U << 2U)
#define XRFDC_MTS_DAC_MARKER_LOC (0x93U << 2U)
#define XRFDC_MTS_DAC_FIFO_MARKER_CTRL (0x94U << 2U)
#define XRFDC_MTS_DAC_FABRIC_OFFSET 0x0C

#define XRFDC_RESET_OFFSET 0x00U /**< Tile reset register */
#define XRFDC_RESTART_OFFSET 0x04U /**< Tile restart register */
#define XRFDC_RESTART_STATE_OFFSET 0x08U /**< Tile restart state register */
#define XRFDC_CURRENT_STATE_OFFSET 0x0CU /**< Current state register */
#define XRFDC_CLOCK_DETECT_OFFSET 0x80U /**< Clock detect register */
#define XRFDC_STATUS_OFFSET 0x228U /**< Common status register */
#define XRFDC_COMMON_INTR_STS 0x100U /**< Common Intr Status register */
#define XRFDC_COMMON_INTR_ENABLE 0x104U /**< Common Intr enable register */
#define XRFDC_INTR_STS 0x200U /**< Intr status register */
#define XRFDC_INTR_ENABLE 0x204U /**< Intr enable register */
#define XRFDC_CONV_INTR_STS(X) (0x208U + (X * 0x08U))
#define XRFDC_CONV_INTR_EN(X) (0x20CU + (X * 0x08U))
#define XRFDC_CONV_CAL_STGS(X) (0x234U + (X * 0x04U))
#define XRFDC_CONV_DSA_STGS(X) (0x244U + (X * 0x04U))
#define XRFDC_CAL_GCB_COEFF0_FAB(X) (0x280U + (X * 0x10U))
#define XRFDC_CAL_GCB_COEFF1_FAB(X) (0x284U + (X * 0x10U))
#define XRFDC_CAL_GCB_COEFF2_FAB(X) (0x288U + (X * 0x10U))
#define XRFDC_CAL_GCB_COEFF3_FAB(X) (0x28CU + (X * 0x10U))
#define XRFDC_TDD_CTRL_SLICE_OFFSET(X) (0x260 + (X * 0x04U)) /**< TDD control registers */
#define XRFDC_PLL_FREQ 0x300U /**< PLL output frequency (before divider) register */
#define XRFDC_PLL_FS 0x304U /**< Sampling rate register */
#define XRFDC_FIFO_ENABLE 0x230U /**< FIFO Enable and Disable */
#define XRFDC_PLL_SDM_CFG0 0x00U /**< PLL Configuration bits for sdm */
#define XRFDC_PLL_SDM_SEED0 0x18U /**< PLL Bits for sdm LSB */
#define XRFDC_PLL_SDM_SEED1 0x1CU /**< PLL Bits for sdm MSB */
#define XRFDC_PLL_VREG 0x44U /**< PLL bits for voltage regulator */
#define XRFDC_PLL_VCO0 0x54U /**< PLL bits for coltage controlled oscillator LSB */
#define XRFDC_PLL_VCO1 0x58U /**< PLL bits for coltage controlled oscillator MSB */
#define XRFDC_PLL_CRS1 0x28U /**< PLL bits for coarse frequency control LSB */
#define XRFDC_PLL_CRS2 0x2CU /**< PLL bits for coarse frequency control MSB */
#define XRFDC_PLL_DIVIDER0 0x30U /**< PLL Output Divider LSB register */
#define XRFDC_PLL_DIVIDER1 0x34U /**< PLL Output Divider MSB register */
#define XRFDC_PLL_SPARE0 0x38U /**< PLL spare inputs LSB */
#define XRFDC_PLL_SPARE1 0x3CU /**< PLL spare inputs MSB */
#define XRFDC_PLL_REFDIV 0x40U /**< PLL Reference Divider register */
#define XRFDC_PLL_VREG 0x44U /**< PLL voltage regulator */
#define XRFDC_PLL_CHARGEPUMP 0x48U /**< PLL bits for charge pumps */
#define XRFDC_PLL_LPF0 0x4CU /**< PLL bits for loop filters LSB */
#define XRFDC_PLL_LPF1 0x50U /**< PLL bits for loop filters MSB */
#define XRFDC_PLL_FPDIV 0x5CU /**< PLL Feedback Divider register */
#define XRFDC_CLK_NETWORK_CTRL0 0x8CU /**< Clock network control and trim register */
#define XRFDC_CLK_NETWORK_CTRL1 0x90U /**< Multi-tile sync and clock source control register */

#define XRFDC_HSCOM_NETWORK_CTRL1_MASK 0x02FU /**< Clock Network Register Mask for IntraTile*/
#define XRFDC_PLL_REFDIV_MASK 0x0E0U /**< PLL Reference Divider Register Mask for IntraTile */
#define XRFDC_PLL_DIVIDER0_ALT_MASK 0xC00U /**< PLL Output Divider Register Mask for IntraTile */
#define XRFDC_PLL_DIVIDER0_BYPPLL_MASK 0x800U /**< PLL Output Divider Register Mask for IntraTile */
#define XRFDC_PLL_DIVIDER0_BYPDIV_MASK 0x400U /**< PLL Output Divider Register Mask for IntraTile */

#define XRFDC_CAL_OCB1_OFFSET_COEFF0 0x200 /**< Foreground offset correction block */
#define XRFDC_CAL_OCB1_OFFSET_COEFF1 0x208 /**< Foreground offset correction block */
#define XRFDC_CAL_OCB1_OFFSET_COEFF2 0x210 /**< Foreground offset correction block */
#define XRFDC_CAL_OCB1_OFFSET_COEFF3 0x218 /**< Foreground offset correction block */
#define XRFDC_CAL_OCB2_OFFSET_COEFF0 0x204 /**< Background offset correction block */
#define XRFDC_CAL_OCB2_OFFSET_COEFF1 0x20C /**< Background offset correction block */
#define XRFDC_CAL_OCB2_OFFSET_COEFF2 0x214 /**< Background offset correction block */
#define XRFDC_CAL_OCB2_OFFSET_COEFF3 0x21C /**< Background offset correction block */
#define XRFDC_CAL_GCB_OFFSET_COEFF0 0x220 /**< Background gain correction block */
#define XRFDC_CAL_GCB_OFFSET_COEFF1 0x224 /**< Background gain correction block */
#define XRFDC_CAL_GCB_OFFSET_COEFF2 0x228 /**< Background gain correction block */
#define XRFDC_CAL_GCB_OFFSET_COEFF3 0x22C /**< Background gain correction block */
#define XRFDC_CAL_GCB_OFFSET_COEFF0_ALT 0x220 /**< Background gain correction block (below Gen 3) */
#define XRFDC_CAL_GCB_OFFSET_COEFF1_ALT 0x228 /**< Background gain correction block (below Gen 3) */
#define XRFDC_CAL_GCB_OFFSET_COEFF2_ALT 0x230 /**< Background gain correction block (below Gen 3) */
#define XRFDC_CAL_GCB_OFFSET_COEFF3_ALT 0x238 /**< Background gain correction block (below Gen 3) */
#define XRFDC_CAL_TSCB_OFFSET_COEFF0 0x170 /**< Background time skew correction block */
#define XRFDC_CAL_TSCB_OFFSET_COEFF1 0x174 /**< Background time skew correction block */
#define XRFDC_CAL_TSCB_OFFSET_COEFF2 0x178 /**< Background time skew correction block */
#define XRFDC_CAL_TSCB_OFFSET_COEFF3 0x17C /**< Background time skew correction block */
#define XRFDC_CAL_TSCB_OFFSET_COEFF4 0x180 /**< Background time skew correction block */
#define XRFDC_CAL_TSCB_OFFSET_COEFF5 0x184 /**< Background time skew correction block */
#define XRFDC_CAL_TSCB_OFFSET_COEFF6 0x188 /**< Background time skew correction block */
#define XRFDC_CAL_TSCB_OFFSET_COEFF7 0x18C /**< Background time skew correction block */
#define XRFDC_CAL_TSCB_OFFSET_COEFF0_ALT 0x168 /**< Background time skew correction block (below Gen 3) */
#define XRFDC_CAL_TSCB_OFFSET_COEFF1_ALT 0x16C /**< Background time skew correction block (below Gen 3) */
#define XRFDC_CAL_TSCB_OFFSET_COEFF2_ALT 0x170 /**< Background time skew correction block (below Gen 3) */
#define XRFDC_CAL_TSCB_OFFSET_COEFF3_ALT 0x174 /**< Background time skew correction block (below Gen 3) */
#define XRFDC_CAL_TSCB_OFFSET_COEFF4_ALT 0x178 /**< Background time skew correction block (below Gen 3) */
#define XRFDC_CAL_TSCB_OFFSET_COEFF5_ALT 0x17C /**< Background time skew correction block (below Gen 3) */
#define XRFDC_CAL_TSCB_OFFSET_COEFF6_ALT 0x180 /**< Background time skew correction block (below Gen 3) */
#define XRFDC_CAL_TSCB_OFFSET_COEFF7_ALT 0x184 /**< Background time skew correction block (below Gen 3) */

#define XRFDC_HSCOM_FIFO_START_OFFSET 0x0C0U /**< FIFO Start register tommon along tile */
#define XRFDC_HSCOM_FIFO_START_OBS_OFFSET 0x0BCU /**< FIFO Obs Start register common along tile */
#define XRFDC_HSCOM_FIFO_START_TDD_OFFSET(X)                                                                           \
	((X == 0) ?                                                                                                    \
		 XRFDC_HSCOM_FIFO_START_OFFSET :                                                                       \
		 XRFDC_HSCOM_FIFO_START_OBS_OFFSET) /**< FIFO Start (or OBS) register common along tile TDD Selected */

/* @} */

/** @name IP Register Map
 *
 * Register offsets from the base address of the IP.
 * @{
 */

#define XRFDC_TILES_ENABLED_OFFSET 0x00A0U /**< The tiles enabled in the design */
#define XRFDC_ADC_PATHS_ENABLED_OFFSET 0x00A4U /**< The ADC analogue/digital paths enabled in the design */
#define XRFDC_DAC_PATHS_ENABLED_OFFSET 0x00A8U /**< The DAC analogue/digital paths enabled in the design */
#define XRFDC_PATH_ENABLED_TILE_SHIFT 4U /**< A shift to get to the correct tile for the path */

/* @} */

/** @name Calibration Mode - Calibration mode registers
 *
 * This register contains bits for calibration modes
 * for ADC.
 * @{
 */

#define XRFDC_CAL_MODES_MASK 0x0003 /**< Calibration modes for Gen 3 mask*/

/* @} */
/** @name Calibration Coefficients - Calibration coefficients and disable registers
 *
 * This register contains bits for calibration coefficients
 * for ADC.
 * @{
 */

#define XRFDC_CAL_OCB_MASK 0xFFFFU /**< offsets coeff mask*/
#define XRFDC_CAL_GCB_MASK 0x0FFFU /**< gain coeff mask*/
#define XRFDC_CAL_GCB_FAB_MASK 0xFFF0U /**< gain coeff mask for IP Gen 2 or below*/
#define XRFDC_CAL_TSCB_MASK 0x01FFU /**< time skew coeff mask*/

#define XRFDC_CAL_GCB_FLSH_MASK 0x1000U /**< GCB accumulator flush mask*/
#define XRFDC_CAL_GCB_ACEN_MASK 0x0800U /**< GCB accumulator enable mask*/
#define XRFDC_CAL_GCB_ENFL_MASK 0x1800U /**< GCB accumulator enable mask*/

#define XRFDC_CAL_OCB_EN_MASK 0x0001U /**< offsets coeff override enable mask*/
#define XRFDC_CAL_GCB_EN_MASK 0x2000U /**< gain coeff override enable mask*/
#define XRFDC_CAL_TSCB_EN_MASK 0x8000U /**< time skew coeff override enable mask*/

#define XRFDC_CAL_OCB_EN_SHIFT 0U /**< offsets coeff shift*/
#define XRFDC_CAL_GCB_EN_SHIFT 13U /**< gain coeff shift*/
#define XRFDC_CAL_TSCB_EN_SHIFT 15U /**< time skew coeff shift*/
#define XRFDC_CAL_GCB_FLSH_SHIFT 12U /**< GCB accumulator flush shift*/
#define XRFDC_CAL_GCB_ACEN_SHIFT 11U /**< GCB accumulator enable shift*/

#define XRFDC_CAL_SLICE_SHIFT 16U /**<Coefficient shift for HSADCs*/

/* @} */
/** @name Calibration Coefficients - Calibration coefficients and disable registers
 *
 * This register contains bits for calibration coefficients
 * for ADC.
 * @{
 */

#define XRFDC_CAL_FREEZE_CAL_MASK 0x1U /**< Calibration freeze enable mask*/
#define XRFDC_CAL_FREEZE_STS_MASK 0x2U /**< Calibration freeze status mask*/
#define XRFDC_CAL_FREEZE_PIN_MASK 0x4U /**< Calibration freeze pin disable mask*/

#define XRFDC_CAL_FREEZE_CAL_SHIFT 0U /**< Calibration freeze enable shift*/
#define XRFDC_CAL_FREEZE_STS_SHIFT 1U /**< Calibration freeze status shift*/
#define XRFDC_CAL_FREEZE_PIN_SHIFT 2U /**< Calibration freeze pin disable shift*/

/* @} */

/** @name FIFO Enable - FIFO enable and disable register
 *
 * This register contains bits for FIFO enable and disable
 * for ADC and DAC.
 * @{
 */

#define XRFDC_FIFO_EN_MASK 0x00000001U /**< FIFO enable/disable mask*/
#define XRFDC_FIFO_EN_OBS_MASK 0x00000002U /**< FIFO OBS enable/disable mask*/
#define XRFDC_FIFO_EN_OBS_SHIFT 1U /**< FIFO OBS enable/disable  shift*/
#define XRFDC_RESTART_MASK 0x00000001U /**< Restart bit mask */

/* @} */

/** @name Clock Enable - FIFO Latency, fabric, DataPath,
 * 			full-rate, output register
 *
 * This register contains bits for various clock enable options of
 * the ADC. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_CLK_EN_CAL_MASK 0x00000001U /**< Enable Output Register clock */
#define XRFDC_CLK_EN_DIG_MASK 0x00000002U /**< Enable full-rate clock */
#define XRFDC_CLK_EN_DP_MASK 0x00000004U /**< Enable Data Path clock */
#define XRFDC_CLK_EN_FAB_MASK 0x00000008U /**< Enable fabric clock */
#define XRFDC_DAT_CLK_EN_MASK 0x0000000FU /**< Data Path Clk enable */
#define XRFDC_CLK_EN_LM_MASK 0x00000010U /**< Enable for FIFO Latency measurement clock */

/* @} */

/** @name Debug reset - FIFO Latency, fabric, DataPath,
 * 			full-rate, output register
 *
 * This register contains bits for various Debug reset options of
 * the ADC. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DBG_RST_CAL_MASK 0x00000001U /**< Reset clk_cal clock domain */
#define XRFDC_DBG_RST_DP_MASK 0x00000002U /**< Reset data path clock domain */
#define XRFDC_DBG_RST_FAB_MASK 0x00000004U /**< Reset clock fabric clock domain */
#define XRFDC_DBG_RST_DIG_MASK 0x00000008U /**< Reset clk_dig clock domain */
#define XRFDC_DBG_RST_DRP_CAL_MASK 0x00000010U /**< Reset subadc-drp register on clock cal */
#define XRFDC_DBG_RST_LM_MASK 0x00000020U /**< Reset FIFO Latency measurement clock domain */

/* @} */

/** @name Fabric rate - Fabric data rate for read and write
 *
 * This register contains bits for read and write fabric data
 * rate for ADC. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_ADC_FAB_RATE_WR_MASK 0x0000000FU /**< ADC FIFO Write Number of Words per clock */
#define XRFDC_DAC_FAB_RATE_WR_MASK 0x0000001FU /**< DAC FIFO Write Number of Words per clock */
#define XRFDC_ADC_FAB_RATE_RD_MASK 0x00000F00U /**< ADC FIFO Read Number of Words per clock */
#define XRFDC_DAC_FAB_RATE_RD_MASK 0x00001F00U /**< DAC FIFO Read Number of Words per clock */
#define XRFDC_FAB_RATE_RD_SHIFT 8U /**< Fabric Read shift */

/* @} */

/** @name Fabric Offset - FIFO de-skew
 *
 * This register contains bits of Fabric Offset.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FAB_RD_PTR_OFFST_MASK 0x0000003FU /**< FIFO read pointer offset for interface de-skew */

/* @} */

/** @name Fabric ISR - Interrupt status register for FIFO interface
 *
 * This register contains bits of margin-indicator and user-data overlap
 * (overflow/underflow). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FAB_ISR_USRDAT_OVR_MASK 0x00000001U /**< User-data overlap- data written faster than read (overflow) */
#define XRFDC_FAB_ISR_USRDAT_UND_MASK 0x00000002U /**< User-data overlap- data read faster than written (underflow) */
#define XRFDC_FAB_ISR_USRDAT_MASK 0x00000003U /**< User-data overlap Mask */
#define XRFDC_FAB_ISR_MARGIND_OVR_MASK 0x00000004U /**< Marginal-indicator overlap (overflow) */
#define XRFDC_FAB_ISR_MARGIND_UND_MASK 0x00000008U /**< Marginal-indicator overlap (underflow) */
/* @} */

/** @name Fabric IMR - Interrupt mask register for FIFO interface
 *
 * This register contains bits of margin-indicator and user-data overlap
 * (overflow/underflow). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FAB_IMR_USRDAT_OVR_MASK 0x00000001U /**< User-data overlap- data written faster than read (overflow) */
#define XRFDC_FAB_IMR_USRDAT_UND_MASK 0x00000002U /**< User-data overlap- data read faster than written (underflow) */
#define XRFDC_FAB_IMR_USRDAT_MASK 0x00000003U /**< User-data overlap Mask */
#define XRFDC_FAB_IMR_MARGIND_OVR_MASK 0x00000004U /**< Marginal-indicator overlap (overflow) */
#define XRFDC_FAB_IMR_MARGIND_UND_MASK 0x00000008U /**< Marginal-indicator overlap (underflow) */
/* @} */

/** @name Update Dynamic - Trigger a dynamic update event
 *
 * This register contains bits of update event for slice, nco, qmc
 * and coarse delay. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_UPDT_EVNT_MASK 0x0000000FU /**< Update event mask */
#define XRFDC_UPDT_EVNT_SLICE_MASK 0x00000001U /**< Trigger a slice update event apply to _DCONFIG reg */
#define XRFDC_UPDT_EVNT_NCO_MASK 0x00000002U /**< Trigger a update event apply to NCO_DCONFIG reg */
#define XRFDC_UPDT_EVNT_QMC_MASK 0x00000004U /**< Trigger a update event apply to QMC_DCONFIG reg */
#define XRFDC_ADC_UPDT_CRSE_DLY_MASK 0x00000008U /**< ADC Trigger a update event apply to Coarse delay_DCONFIG reg */
#define XRFDC_DAC_UPDT_CRSE_DLY_MASK 0x00000020U /**< DAC Trigger a update event apply to Coarse delay_DCONFIG reg */
/* @} */

/** @name FIFO Latency control - Config registers for FIFO Latency measurement
 *
 * This register contains bits of FIFO Latency ctrl for disable, restart and
 * set fifo latency measurement. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FIFO_LTNCY_PRD_MASK 0x00000007U /**< Set FIFO Latency measurement period */
#define XRFDC_FIFO_LTNCY_RESTRT_MASK 0x00000008U /**< Restart FIFO Latency measurement */
#define XRFDC_FIFO_LTNCY_DIS_MASK 0x000000010U /**< Disable FIFO Latency measurement */

/* @} */

/** @name Decode ISR - ISR for Decoder Interface
 *
 * This register contains bits of subadc 0,1,2 and 3 decoder overflow
 * and underflow range. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DEC_ISR_SUBADC_MASK 0x000000FFU /**< subadc decoder Mask */
#define XRFDC_DEC_ISR_SUBADC0_UND_MASK 0x00000001U /**< subadc0 decoder underflow range */
#define XRFDC_DEC_ISR_SUBADC0_OVR_MASK 0x00000002U /**< subadc0 decoder overflow range */
#define XRFDC_DEC_ISR_SUBADC1_UND_MASK 0x00000004U /**< subadc1 decoder underflow range */
#define XRFDC_DEC_ISR_SUBADC1_OVR_MASK 0x00000008U /**< subadc1 decoder overflow range */
#define XRFDC_DEC_ISR_SUBADC2_UND_MASK 0x00000010U /**< subadc2 decoder underflow range */
#define XRFDC_DEC_ISR_SUBADC2_OVR_MASK 0x00000020U /**< subadc2 decoder overflow range */
#define XRFDC_DEC_ISR_SUBADC3_UND_MASK 0x00000040U /**< subadc3 decoder underflow range */
#define XRFDC_DEC_ISR_SUBADC3_OVR_MASK 0x00000080U /**< subadc3 decoder overflow range */

/* @} */

/** @name Decode IMR - IMR for Decoder Interface
 *
 * This register contains bits of subadc 0,1,2 and 3 decoder overflow
 * and underflow range. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DEC_IMR_SUBADC0_UND_MASK 0x00000001U /**< subadc0 decoder underflow range */
#define XRFDC_DEC_IMR_SUBADC0_OVR_MASK 0x00000002U /**< subadc0 decoder overflow range */
#define XRFDC_DEC_IMR_SUBADC1_UND_MASK 0x00000004U /**< subadc1 decoder underflow range */
#define XRFDC_DEC_IMR_SUBADC1_OVR_MASK 0x00000008U /**< subadc1 decoder overflow range */
#define XRFDC_DEC_IMR_SUBADC2_UND_MASK 0x00000010U /**< subadc2 decoder underflow range */
#define XRFDC_DEC_IMR_SUBADC2_OVR_MASK 0x00000020U /**< subadc2 decoder overflow range */
#define XRFDC_DEC_IMR_SUBADC3_UND_MASK 0x00000040U /**< subadc3 decoder underflow range */
#define XRFDC_DEC_IMR_SUBADC3_OVR_MASK 0x00000080U /**< subadc3 decoder overflow range */
#define XRFDC_DEC_IMR_MASK 0x000000FFU

/* @} */

/** @name DataPath (DAC)- FIFO Latency, Image Reject Filter, Mode,
 *
 * This register contains bits for DataPath latency, Image Reject Filter
 * and the Mode for the DAC. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DATAPATH_MODE_MASK 0x00000003U /**< DataPath Mode */
#define XRFDC_DATAPATH_IMR_MASK 0x00000004U /**< IMR Mode */
#define XRFDC_DATAPATH_LATENCY_MASK 0x00000008U /**< DataPath Latency */

#define XRFDC_DATAPATH_IMR_SHIFT 2U /**< IMR Mode shift */

/* @} */

/** @name DataPath ISR - ISR for Data Path interface
 *
 * This register contains bits of QMC Gain/Phase overflow, offset overflow,
 * Decimation I-Path and Interpolation Q-Path overflow for stages 0,1,2.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_ADC_DAT_PATH_ISR_MASK 0x000000FFU /**< ADC Data Path Overflow */
#define XRFDC_DAC_DAT_PATH_ISR_MASK 0x0000FFFFU /**< DAC Data Path Overflow */
#define XRFDC_DAT_ISR_DECI_IPATH_MASK 0x00000007U /**< Decimation I-Path overflow for stages 0,1,2 */
#define XRFDC_DAT_ISR_INTR_QPATH_MASK 0x00000038U /**< Interpolation Q-Path overflow for stages 0,1,2 */
#define XRFDC_DAT_ISR_QMC_GAIN_MASK 0x00000040U /**< QMC Gain/Phase overflow */
#define XRFDC_DAT_ISR_QMC_OFFST_MASK 0x00000080U /**< QMC offset overflow */
#define XRFDC_DAC_DAT_ISR_INVSINC_MASK 0x00000100U /**< Inverse-Sinc offset overflow */

/* @} */

/** @name DataPath IMR - IMR for Data Path interface
 *
 * This register contains bits of QMC Gain/Phase overflow, offset overflow,
 * Decimation I-Path and Interpolation Q-Path overflow for stages 0,1,2.
 * Inverse sinc overflow, Datapath Scaling, Interpolation I and Q, IMR, and Even Nyquist Zone overflow
 * Mixer I and Q over/underflow.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DAT_IMR_DECI_IPATH_MASK 0x00000007U /**< Decimation I-Path overflow for stages 0,1,2 */
#define XRFDC_DAT_IMR_INTR_QPATH_MASK 0x00000038U /**< Interpolation Q-Path overflow for stages 0,1,2 */
#define XRFDC_DAT_IMR_QMC_GAIN_MASK 0x00000040U /**< QMC Gain/Phase overflow */
#define XRFDC_DAT_IMR_QMC_OFFST_MASK 0x00000080U /**< QMC offset overflow */
#define XRFDC_DAC_DAT_IMR_INV_SINC_MASK 0x00000100U /**< Inverse Sinc overflow */
#define XRFDC_DAC_DAT_IMR_MXR_HLF_I_MASK 0x00000200U /**< Over or under flow mixer (Mixer half I) */
#define XRFDC_DAC_DAT_IMR_MXR_HLF_Q_MASK 0x00000400U /**< Over or under flow mixer (Mixer half Q) */
#define XRFDC_DAC_DAT_IMR_DP_SCALE_MASK 0x00000800U /**< DataPath Scaling overflow */
#define XRFDC_DAC_DAT_IMR_INTR_IPATH3_MASK 0x00001000U /**< Interpolation I-Path overflow for stage 3 */
#define XRFDC_DAC_DAT_IMR_INTR_QPATH3_MASK 0x00002000U /**< Interpolation Q-Path overflow for stage 3 */
#define XRFDC_DAC_DAT_IMR_IMR_OV_MASK 0x00004000U /**< IMR overflow */
#define XRFDC_DAC_DAT_IMR_INV_SINC_EVEN_NYQ_MASK 0x00008000U /**< 2nd Nyquist Zone Inverse SINC overflow */
#define XRFDC_ADC_DAT_IMR_MASK 0x000000FFU /**< ADC DataPath mask */
#define XRFDC_DAC_DAT_IMR_MASK 0x0000FFFFU /**< DAC DataPath mask */

/* @} */

/** @name FIFO IMR - FIFO for Data Path interface
 *
 * This register contains bits of FIFO over/underflows
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FIFO_USRD_OF_MASK 0x00000001U /**< User data overflow */
#define XRFDC_FIFO_USRD_UF_MASK 0x00000002U /**< User data underflow */
#define XRFDC_FIFO_MRGN_OF_MASK 0x00000004U /**< Marginal overflow */
#define XRFDC_FIFO_MRGN_UF_MASK 0x00000008U /**< Marginal underflow */
#define XRFDC_FIFO_ACTL_OF_MASK 0x00000010U /**< DAC Actual overflow */
#define XRFDC_FIFO_ACTL_UF_MASK 0x00000020U /**< DAC Actual underflow */
#define XRFDC_DAC_FIFO_IMR_SUPP_MASK 0x00000030U /**< DAC FIFO Mask */
#define XRFDC_DAC_FIFO_IMR_MASK 0x0000003FU /**< DAC FIFO Mask */

/* @} */

/** @name Decimation Config - Decimation control
 *
 * This register contains bits to configure the decimation in terms of
 * the type of data. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DEC_CFG_MASK 0x00000003U /**< ChannelA (2GSPS real data from Mixer I output) */
#define XRFDC_DEC_CFG_CHA_MASK 0x00000000U /**< ChannelA(I) */
#define XRFDC_DEC_CFG_CHB_MASK 0x00000001U /**< ChannelB (2GSPS real data from Mixer Q output) */
#define XRFDC_DEC_CFG_IQ_MASK 0x00000002U /**< IQ-2GSPS */
#define XRFDC_DEC_CFG_4GSPS_MASK 0x00000003U /**< 4GSPS may be I or Q or Real depending on high level block config */

/* @} */

/** @name Decimation Mode - Decimation Rate
 *
 * This register contains bits to configures the decimation rate.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DEC_MOD_MASK 0x00000007U /**< Decimation mode Mask */
#define XRFDC_DEC_MOD_MASK_EXT 0x0000003FU /**< Decimation mode Mask */

/* @} */

/** @name Mixer config0 - Configure I channel coarse mixer mode of operation
 *
 * This register contains bits to set the output data sequence of
 * I channel. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_MIX_CFG0_MASK 0x00000FFFU /**< Mixer Config0 Mask */
#define XRFDC_MIX_I_DAT_WRD0_MASK 0x00000007U /**< Output data word[0] of I channel */
#define XRFDC_MIX_I_DAT_WRD1_MASK 0x00000038U /**< Output data word[1] of I channel */
#define XRFDC_MIX_I_DAT_WRD2_MASK 0x000001C0U /**< Output data word[2] of I channel */
#define XRFDC_MIX_I_DAT_WRD3_MASK 0x00000E00U /**< Output data word[3] of I channel */

/* @} */

/** @name Mixer config1 - Configure Q channel coarse mixer mode of operation
 *
 * This register contains bits to set the output data sequence of
 * Q channel. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_MIX_CFG1_MASK 0x00000FFFU /**< Mixer Config0 Mask */
#define XRFDC_MIX_Q_DAT_WRD0_MASK 0x00000007U /**< Output data word[0] of Q channel */
#define XRFDC_MIX_Q_DAT_WRD1_MASK 0x00000038U /**< Output data word[1] of Q channel */
#define XRFDC_MIX_Q_DAT_WRD2_MASK 0x000001C0U /**< Output data word[2] of Q channel */
#define XRFDC_MIX_Q_DAT_WRD3_MASK 0x00000E00U /**< Output data word[3] of Q channel */

/* @} */

/** @name Mixer mode - Configure mixer mode of operation
 *
 * This register contains bits to set NCO phases, NCO output scale
 * and fine mixer multipliers. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_EN_I_IQ_MASK 0x00000003U /**< Enable fine mixer multipliers on IQ i/p for I output */
#define XRFDC_EN_Q_IQ_MASK 0x0000000CU /**< Enable fine mixer multipliers on IQ i/p for Q output */
#define XRFDC_FINE_MIX_SCALE_MASK 0x00000010U /**< NCO output scale */
#define XRFDC_SEL_I_IQ_MASK 0x00000F00U /**< Select NCO phases for I output */
#define XRFDC_SEL_Q_IQ_MASK 0x0000F000U /**< Select NCO phases for Q output */
#define XRFDC_I_IQ_COS_MINSIN 0x00000C00U /**< Select NCO phases for I output */
#define XRFDC_Q_IQ_SIN_COS 0x00001000U /**< Select NCO phases for Q output */
#define XRFDC_MIXER_MODE_C2C_MASK 0x0000000FU /**< Mixer mode C2C Mask */
#define XRFDC_MIXER_MODE_R2C_MASK 0x00000005U /**< Mixer mode R2C Mask */
#define XRFDC_MIXER_MODE_C2R_MASK 0x00000003U /**< Mixer mode C2R Mask */
#define XRFDC_MIXER_MODE_OFF_MASK 0x00000000U /**< Mixer mode OFF Mask */
/* @} */

/** @name NCO update - NCO update mode
 *
 * This register contains bits to Select event source, delay and reset delay.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_UPDT_MODE_MASK 0x00000007U /**< NCO event source selection mask */
#define XRFDC_NCO_UPDT_MODE_GRP 0x00000000U /**< NCO event source selection is Group */
#define XRFDC_NCO_UPDT_MODE_SLICE 0x00000001U /**< NCO event source selection is slice */
#define XRFDC_NCO_UPDT_MODE_TILE 0x00000002U /**< NCO event source selection is tile */
#define XRFDC_NCO_UPDT_MODE_SYSREF 0x00000003U /**< NCO event source selection is Sysref */
#define XRFDC_NCO_UPDT_MODE_MARKER 0x00000004U /**< NCO event source selection is Marker */
#define XRFDC_NCO_UPDT_MODE_FABRIC 0x00000005U /**< NCO event source selection is fabric */
#define XRFDC_NCO_UPDT_DLY_MASK 0x00001FF8U /**< delay in clk_dp cycles in application of event after arrival */
#define XRFDC_NCO_UPDT_RST_DLY_MASK 0x0000D000U /**< optional delay on the NCO phase reset delay */

/* @} */

/** @name NCO Phase Reset - NCO Slice Phase Reset
 *
 * This register contains bits to reset the nco phase of the current
 * slice phase accumulator. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_PHASE_RST_MASK 0x00000001U /**< Reset NCO Phase of current slice */

/* @} */

/** @name DAC interpolation data
 *
 * This register contains bits for DAC interpolation data type
 * @{
 */

#define XRFDC_DAC_INTERP_DATA_MASK 0x00000001U /**< Data type mask */

/* @} */

/** @name NCO Freq Word[47:32] - NCO Phase increment(nco freq 48-bit)
 *
 * This register contains bits for frequency control word of the
 * NCO. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_FQWD_UPP_MASK 0x0000FFFFU /**< NCO Phase increment[47:32] */
#define XRFDC_NCO_FQWD_UPP_SHIFT 32U /**< Freq Word upper shift */

/* @} */

/** @name NCO Freq Word[31:16] - NCO Phase increment(nco freq 48-bit)
 *
 * This register contains bits for frequency control word of the
 * NCO. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_FQWD_MID_MASK 0x0000FFFFU /**< NCO Phase increment[31:16] */
#define XRFDC_NCO_FQWD_MID_SHIFT 16U /**< Freq Word Mid shift */

/* @} */

/** @name NCO Freq Word[15:0] - NCO Phase increment(nco freq 48-bit)
 *
 * This register contains bits for frequency control word of the
 * NCO. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_FQWD_LOW_MASK 0x0000FFFFU /**< NCO Phase increment[15:0] */
#define XRFDC_NCO_FQWD_MASK 0x0000FFFFFFFFFFFFU /**< NCO Freq offset[48:0] */

/* @} */

/** @name NCO Phase Offset[17:16] - NCO Phase offset
 *
 * This register contains bits to set NCO Phase offset(18-bit offset
 * added to the phase accumulator). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_PHASE_UPP_MASK 0x00000003U /**< NCO Phase offset[17:16] */
#define XRFDC_NCO_PHASE_UPP_SHIFT 16U /**< NCO phase upper shift */

/* @} */

/** @name NCO Phase Offset[15:0] - NCO Phase offset
 *
 * This register contains bits to set NCO Phase offset(18-bit offset
 * added to the phase accumulator). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_PHASE_LOW_MASK 0x0000FFFFU /**< NCO Phase offset[15:0] */
#define XRFDC_NCO_PHASE_MASK 0x0003FFFFU /**< NCO Phase offset[17:0] */

/* @} */

/** @name NCO Phase mode - NCO Control setting mode
 *
 * This register contains bits to set NCO mode of operation.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_NCO_PHASE_MOD_MASK 0x00000003U /**< NCO mode of operation  mask */
#define XRFDC_NCO_PHASE_MOD_4PHASE 0x00000003U /**< NCO output 4 successive phase */
#define XRFDC_NCO_PHASE_MOD_EVEN 0x00000001U /**< NCO output even phase */
#define XRFDC_NCO_PHASE_MODE_ODD 0x00000002U /**< NCO output odd phase */
/* @} */

/** @name QMC update - QMC update mode
 *
 * This register contains bits to Select event source and delay.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_QMC_UPDT_MODE_MASK 0x00000007U /**< QMC event source selection mask */
#define XRFDC_QMC_UPDT_MODE_GRP 0x00000000U /**< QMC event source selection is group */
#define XRFDC_QMC_UPDT_MODE_SLICE 0x00000001U /**< QMC event source selection is slice */
#define XRFDC_QMC_UPDT_MODE_TILE 0x00000002U /**< QMC event source selection is tile */
#define XRFDC_QMC_UPDT_MODE_SYSREF 0x00000003U /**< QMC event source selection is Sysref */
#define XRFDC_QMC_UPDT_MODE_MARKER 0x00000004U /**< QMC event source selection is Marker */
#define XRFDC_QMC_UPDT_MODE_FABRIC 0x00000005U /**< QMC event source selection is fabric */
#define XRFDC_QMC_UPDT_DLY_MASK 0x00001FF8U /**< delay in clk_dp cycles in application of event after arrival */

/* @} */

/** @name QMC Config - QMC Config register
 *
 * This register contains bits to enable QMC gain and QMC
 * Phase correction. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_QMC_CFG_EN_GAIN_MASK 0x00000001U /**< enable QMC gain correction mask */
#define XRFDC_QMC_CFG_EN_PHASE_MASK 0x00000002U /**< enable QMC Phase correction mask */
#define XRFDC_QMC_CFG_PHASE_SHIFT 1U /**< QMC config phase shift */

/* @} */

/** @name QMC Offset - QMC offset correction
 *
 * This register contains bits to set QMC offset correction
 * factor. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_QMC_OFFST_CRCTN_MASK 0x00000FFFU /**< QMC offset correction factor */

/* @} */

/** @name QMC Gain - QMC Gain correction
 *
 * This register contains bits to set QMC gain correction
 * factor. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_QMC_GAIN_CRCTN_MASK 0x00003FFFU /**< QMC gain correction factor */

/* @} */

/** @name QMC Phase - QMC Phase correction
 *
 * This register contains bits to set QMC phase correction
 * factor. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_QMC_PHASE_CRCTN_MASK 0x00000FFFU /**< QMC phase correction factor */

/* @} */

/** @name Coarse Delay Update - Coarse delay update mode.
 *
 * This register contains bits to Select event source and delay.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_CRSEDLY_UPDT_MODE_MASK 0x00000007U /**< Coarse delay event source selection mask */
#define XRFDC_CRSEDLY_UPDT_MODE_GRP 0x00000000U /**< Coarse delay event source selection is group */
#define XRFDC_CRSEDLY_UPDT_MODE_SLICE 0x00000001U /**< Coarse delay event source selection is slice */
#define XRFDC_CRSEDLY_UPDT_MODE_TILE 0x00000002U /**< Coarse delay event source selection is tile */
#define XRFDC_CRSEDLY_UPDT_MODE_SYSREF 0x00000003U /**< Coarse delay event source selection is sysref */
#define XRFDC_CRSEDLY_UPDT_MODE_MARKER 0x00000004U /**< Coarse delay event source selection is Marker */
#define XRFDC_CRSEDLY_UPDT_MODE_FABRIC 0x00000005U /**< Coarse delay event source selection is fabric */
#define XRFDC_CRSEDLY_UPDT_DLY_MASK 0x00001FF8U /**< delay in clk_dp cycles in application of event after arrival */

/* @} */

/** @name Coarse delay Config - Coarse delay select
 *
 * This register contains bits to select coarse delay.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_CRSE_DLY_CFG_MASK 0x00000007U /**< Coarse delay select */
#define XRFDC_CRSE_DLY_CFG_MASK_EXT 0x0000003FU /**< Extended coarse delay select*/

/* @} */

/** @name Data Scaling Config - Data Scaling enable
 *
 * This register contains bits to enable data scaling.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DAT_SCALE_CFG_MASK 0x00000001U /**< Enable data scaling */

/* @} */

/** @name Data Scaling Config - Data Scaling enable
 *
 * This register contains bits to enable data scaling.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_DAT_SCALE_CFG_MASK 0x00000001U /**< Enable data scaling */

/* @} */

/** @name Switch Matrix Config
 *
 * This register contains bits to control crossbar switch that select
 * data to mixer block. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SWITCH_MTRX_MASK 0x0000003FU /**< Switch matrix mask */
#define XRFDC_SEL_CB_TO_MIX1_MASK 0x00000003U /**< Control crossbar switch that select the data to mixer block mux1 */
#define XRFDC_SEL_CB_TO_MIX0_MASK 0x0000000CU /**< Control crossbar switch that select the data to mixer block mux0 */
#define XRFDC_SEL_CB_TO_QMC_MASK 0x00000010U /**< Control crossbar switch that select the data to QMC */
#define XRFDC_SEL_CB_TO_DECI_MASK 0x00000020U /**< Control crossbar switch that select the data to decimation filter */
#define XRFDC_SEL_CB_TO_MIX0_SHIFT 2U /**< Crossbar Mixer0 shift */

/* @} */

/** @name Threshold0 Config
 *
 * This register contains bits to select mode, clear mode and to
 * clear sticky bit. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD0_EN_MOD_MASK 0x00000003U /**< Enable Threshold0 block */
#define XRFDC_TRSHD0_CLR_MOD_MASK 0x00000004U /**< Clear mode */
#define XRFDC_TRSHD0_STIKY_CLR_MASK 0x00000008U /**< Clear sticky bit */

/* @} */

/** @name Threshold0 Average[31:16]
 *
 * This register contains bits to select Threshold0 under averaging.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD0_AVG_UPP_MASK 0x0000FFFFU /**< Threshold0 under Averaging[31:16] */
#define XRFDC_TRSHD0_AVG_UPP_SHIFT 16U /**< Threshold0 Avg upper shift */
/* @} */

/** @name Threshold0 Average[15:0]
 *
 * This register contains bits to select Threshold0 under averaging.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD0_AVG_LOW_MASK 0x0000FFFFU /**< Threshold0 under Averaging[15:0] */

/* @} */

/** @name Threshold0 Under threshold
 *
 * This register contains bits to select Threshold0 under threshold.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD0_UNDER_MASK 0x00007FFFU /**< Threshold0 under Threshold[14:0] */

/* @} */

/** @name Threshold0 Over threshold
 *
 * This register contains bits to select Threshold0 over threshold.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD0_OVER_MASK 0x00007FFFU /**< Threshold0 under Threshold[14:0] */

/* @} */

/** @name Threshold1 Config
 *
 * This register contains bits to select mode, clear mode and to
 * clear sticky bit. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD1_EN_MOD_MASK 0x00000003U /**< Enable Threshold1 block */
#define XRFDC_TRSHD1_CLR_MOD_MASK 0x00000004U /**< Clear mode */
#define XRFDC_TRSHD1_STIKY_CLR_MASK 0x00000008U /**< Clear sticky bit */

/* @} */

/** @name Threshold1 Average[31:16]
 *
 * This register contains bits to select Threshold1 under averaging.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD1_AVG_UPP_MASK 0x0000FFFFU /**< Threshold1 under Averaging[31:16] */
#define XRFDC_TRSHD1_AVG_UPP_SHIFT 16U /**< Threshold1 Avg upper shift */

/* @} */

/** @name Threshold1 Average[15:0]
 *
 * This register contains bits to select Threshold1 under averaging.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD1_AVG_LOW_MASK 0x0000FFFFU /**< Threshold1 under Averaging[15:0] */

/* @} */

/** @name Threshold1 Under threshold
 *
 * This register contains bits to select Threshold1 under threshold.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD1_UNDER_MASK 0x00007FFFU /**< Threshold1 under Threshold[14:0] */

/* @} */

/** @name Threshold1 Over threshold
 *
 * This register contains bits to select Threshold1 over threshold.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TRSHD1_OVER_MASK 0x00007FFFU /**< Threshold1 under Threshold[14:0] */

/* @} */

/** @name TDD Control
 *
 * This register contains bits to manage the TDD Control
 * @{
 */

#define XRFDC_TDD_CTRL_MASK 0x0000001FU /**< All TDD control bits */
#define XRFDC_TDD_CTRL_MODE01_MASK 0x00000003U /**< The TDD mode control bits */
#define XRFDC_TDD_CTRL_MODE0_MASK 0x00000001U /**< The TDD control bit for Mode 0 config */
#define XRFDC_TDD_CTRL_MODE1_MASK 0x00000002U /**< The TDD control bit for Mode 1 config (unused)*/
#define XRFDC_TDD_CTRL_OBS_EN_MASK 0x00000008U /**< The observation port enable*/
#define XRFDC_TDD_CTRL_RTP_MASK 0x00000004U /**< The IP RTS disable bit*/
#define XRFDC_TDD_CTRL_RTP_OBS_MASK 0x00000010U /**< The IP RTS disable bit for the observation channel*/
#define XRFDC_TDD_CTRL_MODE1_SHIFT 1U /**< The TDD control bit for Mode 1 config (unused)*/
#define XRFDC_TDD_CTRL_OBS_EN_SHIFT 3U /**< The observation port enable*/
#define XRFDC_TDD_CTRL_RTP_SHIFT 2U /**< The IP RTS disable bit*/
#define XRFDC_TDD_CTRL_RTP_OBS_SHIFT 4U /**< The IP RTS disable bit for the observation channel*/

/* @} */

/** @name TDD Configuration
 *
 * This register contains bits to manage the TDD Configuration
 * @{
 */

#define XRFDC_TDD_ADC_CFG_MASK 0x00007CFFU /**< All ADC TDD config bits */
#define XRFDC_TDD_DAC_CFG_MASK 0x00003FFFU /**< All DAC TDD config bits */
#define XRFDC_TDD_CFG_MASK(X) ((X == 0) ? XRFDC_TDD_ADC_CFG_MASK : XRFDC_TDD_DAC_CFG_MASK) /**< All TDD config bits */

/* @} */

/** @name FrontEnd Data Control
 *
 * This register contains bits to select raw data and cal coefficient to
 * be streamed to memory. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FEND_DAT_CTRL_MASK 0x000000FFU /**< raw data and cal coefficient to be streamed to memory */

/* @} */

/** @name TI Digital Correction Block control0
 *
 * This register contains bits for Time Interleaved digital correction
 * block gain and offset correction. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_CTRL0_MASK 0x0000FFFFU /**< TI  DCB gain and offset correction */
#define XRFDC_TI_DCB_MODE_MASK 0x00007800U /**< TI DCB Mode mask */

/* @} */

/** @name TI Digital Correction Block control1
 *
 * This register contains bits for Time Interleaved digital correction
 * block gain and offset correction. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_CTRL1_MASK 0x00001FFFU /**< TI  DCB gain and offset correction */

/* @} */

/** @name TI Digital Correction Block control2
 *
 * This register contains bits for Time Interleaved digital correction
 * block gain and offset correction. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_CTRL2_MASK 0x00001FFFU /**< TI  DCB gain and offset correction */

/* @} */

/** @name TI Time Skew control0
 *
 * This register contains bits for Time skew correction control bits0(enables,
 * mode, multiplier factors, debug). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_TISK_EN_MASK 0x00000001U /**< Block Enable */
#define XRFDC_TI_TISK_MODE_MASK 0x00000002U /**< Mode (2G/4G) */
#define XRFDC_TI_TISK_ZONE_MASK 0x00000004U /**< Specifies Nyquist zone */
#define XRFDC_TI_TISK_CHOP_EN_MASK 0x00000008U /**< enable chopping mode */
#define XRFDC_TI_TISK_MU_CM_MASK 0x000000F0U /**< Constant mu_cm multiplying common mode path */
#define XRFDC_TI_TISK_MU_DF_MASK 0x00000F00U /**< Constant mu_df multiplying differential path */
#define XRFDC_TI_TISK_DBG_CTRL_MASK 0x0000F000U /**< Debug control */
#define XRFDC_TI_TISK_DBG_UPDT_RT_MASK 0x00001000U /**< Debug update rate */
#define XRFDC_TI_TISK_DITH_DLY_MASK 0x0000E000U /**< Programmable delay on dither path to match data path */
#define XRFDC_TISK_ZONE_SHIFT 2U /**< Nyquist zone shift */

/* @} */

/** @name DAC MC Config0
 *
 * This register contains bits for enable/disable shadow logic , Nyquist zone
 * selection, enable full speed clock, Programmable delay.
 * @{
 */

#define XRFDC_MC_CFG0_MIX_MODE_MASK 0x00000002U /**< Enable 	Mixing mode */
#define XRFDC_MC_CFG0_MIX_MODE_SHIFT 1U /**< Mix mode shift */

/* @} */

/** @name TI Time Skew control0
 *
 * This register contains bits for Time skew correction control bits0(enables,
 * mode, multiplier factors, debug). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_EN_MASK 0x00000001U /**< Block Enable */
#define XRFDC_TISK_MODE_MASK 0x00000002U /**< Mode (2G/4G) */
#define XRFDC_TISK_ZONE_MASK 0x00000004U /**< Specifies Nyquist zone */
#define XRFDC_TISK_CHOP_EN_MASK 0x00000008U /**< enable chopping mode */
#define XRFDC_TISK_MU_CM_MASK 0x000000F0U /**< Constant mu_cm multiplying common mode path */
#define XRFDC_TISK_MU_DF_MASK 0x00000F00U /**< Constant mu_df multiplying differential path */
#define XRFDC_TISK_DBG_CTRL_MASK 0x0000F000U /**< Debug control */
#define XRFDC_TISK_DBG_UPDT_RT_MASK 0x00001000U /**< Debug update rate */
#define XRFDC_TISK_DITH_DLY_MASK 0x0000E000U /**< Programmable delay on dither path to match data path */

/* @} */

/** @name TI Time Skew control1
 *
 * This register contains bits for Time skew correction control bits1
 * (Deadzone Parameters). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DZ_MIN_VAL_MASK 0x000000FFU /**< Deadzone min */
#define XRFDC_TISK_DZ_MAX_VAL_MASK 0x0000FF00U /**< Deadzone max */

/* @} */

/** @name TI Time Skew control2
 *
 * This register contains bits for Time skew correction control bits2
 * (Filter parameters). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_MU0_MASK 0x0000000FU /**< Filter0 multiplying factor */
#define XRFDC_TISK_BYPASS0_MASK 0x00000080U /**< ByPass filter0 */
#define XRFDC_TISK_MU1_MASK 0x00000F00U /**< Filter1 multiplying factor */
#define XRFDC_TISK_BYPASS1_MASK 0x00008000U /**< Filter1 multiplying factor */

/* @} */

/** @name TI Time Skew control3
 *
 * This register contains bits for Time skew control settling time
 * following code update. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_SETTLE_MASK 0x000000FFU /**< Settling time following code update */

/* @} */

/** @name TI Time Skew control4
 *
 * This register contains bits for Time skew control setting time
 * following code update. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_CAL_PRI_MASK 0x00000001U /**< */
#define XRFDC_TISK_DITH_INV_MASK 0x00000FF0U /**< */

/* @} */

/** @name TI Time Skew DAC0
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch0. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DAC0_CODE_MASK 0x000000FFU /**< Code to correction DAC of subadc ch0 front end switch0 */
#define XRFDC_TISK_DAC0_OVRID_EN_MASK 0x00008000U /**< override enable */

/* @} */

/** @name TI Time Skew DAC1
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch1. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DAC1_CODE_MASK 0x000000FFU /**< Code to correction DAC of subadc ch1 front end switch0 */
#define XRFDC_TISK_DAC1_OVRID_EN_MASK 0x00008000U /**< override enable */

/* @} */

/** @name TI Time Skew DAC2
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch2. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DAC2_CODE_MASK 0x000000FFU /**< Code to correction DAC of subadc ch2 front end switch0 */
#define XRFDC_TISK_DAC2_OVRID_EN_MASK 0x00008000U /**< override enable */

/* @} */

/** @name TI Time Skew DAC3
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch3. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DAC3_CODE_MASK 0x000000FFU /**< Code to correction DAC of subadc ch3 front end switch0 */
#define XRFDC_TISK_DAC3_OVRID_EN_MASK 0x00008000U /**< override enable */

/* @} */

/** @name TI Time Skew DACP0
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch0. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DACP0_CODE_MASK 0x000000FFU /**< Code to correction DAC of subadc ch0 front end switch1 */
#define XRFDC_TISK_DACP0_OVRID_EN_MASK 0x00008000U /**< override enable */

/* @} */

/** @name TI Time Skew DACP1
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch1. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DACP1_CODE_MASK 0x000000FFU /**< Code to correction DAC of subadc ch1 front end switch1 */
#define XRFDC_TISK_DACP1_OVRID_EN_MASK 0x00008000U /**< override enable */

/* @} */

/** @name TI Time Skew DACP2
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch2. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DACP2_CODE_MASK 0x000000FFU /**< Code to correction DAC of subadc ch2 front end switch1 */
#define XRFDC_TISK_DACP2_OVRID_EN_MASK 0x00008000U /**< override enable */

/* @} */

/** @name TI Time Skew DACP3
 *
 * This register contains bits for Time skew DAC cal code of
 * subadc ch3. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TISK_DACP3_CODE_MASK 0x000000FFU /**< Code to correction DAC of subadc ch3 front end switch1 */
#define XRFDC_TISK_DACP3_OVRID_EN_MASK 0x00008000U /**< override enable */

/* @} */

/** @name SubDRP ADC0 address
 *
 * This register contains the sub-drp address of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC0_ADDR_MASK 0x000000FFU /**< sub-drp0 address */

/* @} */

/** @name SubDRP ADC0 Data
 *
 * This register contains the sub-drp data of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC0_DAT_MASK 0x0000FFFFU /**< sub-drp0 data for read or write transaction */

/* @} */

/** @name SubDRP ADC1 address
 *
 * This register contains the sub-drp address of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC1_ADDR_MASK 0x000000FFU /**< sub-drp1 address */

/* @} */

/** @name SubDRP ADC1 Data
 *
 * This register contains the sub-drp data of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC1_DAT_MASK 0x0000FFFFU /**< sub-drp1 data for read or write transaction */

/* @} */

/** @name SubDRP ADC2 address
 *
 * This register contains the sub-drp address of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC2_ADDR_MASK 0x000000FFU /**< sub-drp2 address */

/* @} */

/** @name SubDRP ADC2 Data
 *
 * This register contains the sub-drp data of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC2_DAT_MASK 0x0000FFFFU /**< sub-drp2 data for read or write transaction */

/* @} */

/** @name SubDRP ADC3 address
 *
 * This register contains the sub-drp address of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC3_ADDR_MASK 0x000000FFU /**< sub-drp3 address */

/* @} */

/** @name SubDRP ADC3 Data
 *
 * This register contains the sub-drp data of the target register.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_SUBDRP_ADC3_DAT_MASK 0x0000FFFFU /**< sub-drp3 data for read or write transaction */

/* @} */

/** @name RX MC PWRDWN
 *
 * This register contains the static configuration bits of ADC(RX) analog.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_RX_MC_PWRDWN_MASK 0x0000FFFFU /**< RX MC power down */

/* @} */

/** @name RX MC Config0
 *
 * This register contains the static configuration bits of ADC(RX) analog.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_RX_MC_CFG0_MASK 0x0000FFFFU /**< RX MC config0 */
#define XRFDC_RX_MC_CFG0_CM_MASK 0x00000040U /**< Coupling mode mask */
#define XRFDC_RX_MC_CFG0_IM3_DITH_MASK 0x00000020U /**< IM3 Dither Enable mode mask */
#define XRFDC_RX_MC_CFG0_IM3_DITH_SHIFT 5U /**< IM3 Dither Enable mode shift */

/* @} */

/** @name RX MC Config1
 *
 * This register contains the static configuration bits of ADC(RX) analog.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_RX_MC_CFG1_MASK 0x0000FFFFU /**< RX MC Config1 */

/* @} */

/** @name RX MC Config2
 *
 * This register contains the static configuration bits of ADC(RX) analog.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_RX_MC_CFG2_MASK 0x0000FFFFU /**< RX MC Config2 */

/* @} */

/** @name RX Pair MC Config0
 *
 * This register contains the RX Pair (RX0 and RX1 or RX2 and RX3)static
 * configuration bits of ADC(RX) analog. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_RX_PR_MC_CFG0_MASK 0x0000FFFFU /**< RX Pair MC Config0 */

/* @} */

/** @name RX Pair MC Config1
 *
 * This register contains the RX Pair (RX0 and RX1 or RX2 and RX3)static
 * configuration bits of ADC(RX) analog. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_RX_PR_MC_CFG1_MASK 0x0000FFFFU /**< RX Pair MC Config1 */

/* @} */

/** @name TI DCB Status0 BG
 *
 * This register contains the subadc ch0 ocb1 BG offset correction factor
 * value. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS0_BG_MASK 0x0000FFFFU /**< DCB Status0 BG */

/* @} */

/** @name TI DCB Status0 FG
 *
 * This register contains the subadc ch0 ocb2 FG offset correction factor
 * value(read and write). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS0_FG_MASK 0x0000FFFFU /**< DCB Status0 FG */

/* @} */

/** @name TI DCB Status1 BG
 *
 * This register contains the subadc ch1 ocb1 BG offset correction factor
 * value. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS1_BG_MASK 0x0000FFFFU /**< DCB Status1 BG */

/* @} */

/** @name TI DCB Status1 FG
 *
 * This register contains the subadc ch1 ocb2 FG offset correction factor
 * value(read and write). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS1_FG_MASK 0x0000FFFFU /**< DCB Status1 FG */

/* @} */

/** @name TI DCB Status2 BG
 *
 * This register contains the subadc ch2 ocb1 BG offset correction factor
 * value. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS2_BG_MASK 0x0000FFFFU /**< DCB Status2 BG */

/* @} */

/** @name TI DCB Status2 FG
 *
 * This register contains the subadc ch2 ocb2 FG offset correction factor
 * value(read and write). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS2_FG_MASK 0x0000FFFFU /**< DCB Status2 FG */

/* @} */

/** @name TI DCB Status3 BG
 *
 * This register contains the subadc ch3 ocb1 BG offset correction factor
 * value. Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS3_BG_MASK 0x0000FFFFU /**< DCB Status3 BG */

/* @} */

/** @name TI DCB Status3 FG
 *
 * This register contains the subadc ch3 ocb2 FG offset correction factor
 * value(read and write). Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS3_FG_MASK 0x0000FFFFU /**< DCB Status3 FG */

/* @} */

/** @name TI DCB Status4 MSB
 *
 * This register contains the DCB status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS4_MSB_MASK 0x0000FFFFU /**< read the status of gcb acc0 msb bits(subadc chan0) */

/* @} */

/** @name TI DCB Status4 LSB
 *
 * This register contains the DCB Status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS4_LSB_MASK 0x0000FFFFU /**< read the status of gcb acc0 lsb bits(subadc chan0) */

/* @} */

/** @name TI DCB Status5 MSB
 *
 * This register contains the DCB status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS5_MSB_MASK 0x0000FFFFU /**< read the status of gcb acc1 msb bits(subadc chan1) */

/* @} */

/** @name TI DCB Status5 LSB
 *
 * This register contains the DCB Status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS5_LSB_MASK 0x0000FFFFU /**< read the status of gcb acc1 lsb bits(subadc chan1) */

/* @} */

/** @name TI DCB Status6 MSB
 *
 * This register contains the DCB status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS6_MSB_MASK 0x0000FFFFU /**< read the status of gcb acc2 msb bits(subadc chan2) */

/* @} */

/** @name TI DCB Status6 LSB
 *
 * This register contains the DCB Status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS6_LSB_MASK 0x0000FFFFU /**< read the status of gcb acc2 lsb bits(subadc chan2) */

/* @} */

/** @name TI DCB Status7 MSB
 *
 * This register contains the DCB status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS7_MSB_MASK 0x0000FFFFU /**< read the status of gcb acc3 msb bits(subadc chan3) */

/* @} */

/** @name TI DCB Status7 LSB
 *
 * This register contains the DCB Status. Read/Write apart from the
 * reserved bits.
 * @{
 */

#define XRFDC_TI_DCB_STS7_LSB_MASK 0x0000FFFFU /**< read the status of gcb acc3 lsb bits(subadc chan3) */

/* @} */

/** @name PLL_REFDIV
 *
 * This register contains the bits for Reference Clock Divider
 * @{
 */

#define XRFDC_REFCLK_DIV_MASK 0x1FU
#define XRFDC_REFCLK_DIV_1_MASK 0x10U /**< Mask for Div1 */
#define XRFDC_REFCLK_DIV_2_MASK 0x0U /**< Mask for Div2 */
#define XRFDC_REFCLK_DIV_3_MASK 0x1U /**< Mask for Div3 */
#define XRFDC_REFCLK_DIV_4_MASK 0x2U /**< Mask for Div4 */

/* @} */

/** @name FIFO Latency
 *
 * This register contains bits for result, key and done flag.
 * Read/Write apart from the reserved bits.
 * @{
 */

#define XRFDC_FIFO_LTNCY_RES_MASK 0x00000FFFU /**< Latency measurement result */
#define XRFDC_FIFO_LTNCY_KEY_MASK 0x00004000U /**< Latency measurement result identification key */
#define XRFDC_FIFO_LTNCY_DONE_MASK 0x00008000U /**< Latency measurement done flag */

/* @} */

/** @name Decoder Control
 *
 * This register contains Unary Decoder/Randomizer settings to use.
 * @{
 */

#define XRFDC_DEC_CTRL_MODE_MASK 0x00000007U /**< Decoder mode */

/* @} */

/** @name HSCOM Power state mask
 *
 * This register contains HSCOM_PWR to check powerup_state.
 * @{
 */

#define XRFDC_HSCOM_PWR_STATE_MASK 0x0000FFFFU /**< powerup state mask */

/* @} */

/** @name Interpolation Control
 *
 * This register contains Interpolation filter modes.
 * @{
 */

#define XRFDC_INTERP_MODE_MASK 0x00000077U /**< Interp filter mask */
#define XRFDC_INTERP_MODE_I_MASK 0x00000007U /**< Interp filter I */
#define XRFDC_INTERP_MODE_Q_SHIFT 4U /**< Interp mode Q shift */
#define XRFDC_INTERP_MODE_MASK_EXT 0x00003F3FU /**< Interp filter mask */
#define XRFDC_INTERP_MODE_I_MASK_EXT 0x0000003FU /**< Interp filter I */
#define XRFDC_INTERP_MODE_Q_SHIFT_EXT 8U /**< Interp mode Q shift */

/* @} */

/** @name Tile enables register
 *
 * This register contains the bits that indicate
 * whether or not a tile is enabled (Read Only).
 * @{
 */

#define XRFDC_DAC_TILES_ENABLED_SHIFT 4U /**< Shift to the DAC tile bits */

/* @} */

/** @name Path enables register
 *
 * This register contains the bits that indicate
 * whether or not an analogue/digital is enabled (Read Only).
 * @{
 */

#define XRFDC_DIGITAL_PATH_ENABLED_SHIFT 16U /**< Shift to the digital path bits */

/* @} */

/** @name Tile Reset
 *
 * This register contains Tile reset bit.
 * @{
 */

#define XRFDC_TILE_RESET_MASK 0x00000001U /**< Tile reset mask */

/* @} */

/** @name Status register
 *
 * This register contains common status bits.
 * @{
 */

#define XRFDC_PWR_UP_STAT_MASK 0x00000004U /**< Power Up state mask */
#define XRFDC_PWR_UP_STAT_SHIFT 2U /**< PowerUp status shift */
#define XRFDC_PLL_LOCKED_MASK 0x00000008U /**< PLL Locked mask */
#define XRFDC_PLL_LOCKED_SHIFT 3U /**< PLL locked shift */

/* @} */

/** @name Restart State register
 *
 * This register contains Start and End state bits.
 * @{
 */

#define XRFDC_PWR_STATE_MASK 0x0000FFFFU /**< State mask */
#define XRFDC_RSR_START_SHIFT 8U /**< Start state shift */

/* @} */

/** @name Clock Detect register
 *
 * This register contains Start and End state bits.
 * @{
 */

#define XRFDC_CLOCK_DETECT_MASK 0x0000FFFFU /**< Clock detect mask */
#define XRFDC_CLOCK_DETECT_SRC_MASK 0x00005555U /**< Clock detect mask */

/* @} */

/** @name Common interrupt enable register
 *
 * This register contains bits to enable interrupt for
 * ADC and DAC tiles.
 * @{
 */

#define XRFDC_EN_INTR_DAC_TILE0_MASK 0x00000001U /**< DAC Tile0 	interrupt enable mask */
#define XRFDC_EN_INTR_DAC_TILE1_MASK 0x00000002U /**< DAC Tile1 	interrupt enable mask */
#define XRFDC_EN_INTR_DAC_TILE2_MASK 0x00000004U /**< DAC Tile2 	interrupt enable mask */
#define XRFDC_EN_INTR_DAC_TILE3_MASK 0x00000008U /**< DAC Tile3 	interrupt enable mask */
#define XRFDC_EN_INTR_ADC_TILE0_MASK 0x00000010U /**< ADC Tile0 	interrupt enable mask */
#define XRFDC_EN_INTR_ADC_TILE1_MASK 0x00000020U /**< ADC Tile1 	interrupt enable mask */
#define XRFDC_EN_INTR_ADC_TILE2_MASK 0x00000040U /**< ADC Tile2 	interrupt enable mask */
#define XRFDC_EN_INTR_ADC_TILE3_MASK 0x00000080U /**< ADC Tile3 	interrupt enable mask */
/* @} */

/** @name interrupt enable register
 *
 * This register contains bits to enable interrupt for blocks.
 * @{
 */

#define XRFDC_EN_INTR_SLICE_MASK 0x0000000FU /**< Slice intr mask */
#define XRFDC_EN_INTR_SLICE0_MASK 0x00000001U /**< slice0 	interrupt enable mask */
#define XRFDC_EN_INTR_SLICE1_MASK 0x00000002U /**< slice1 	interrupt enable mask */
#define XRFDC_EN_INTR_SLICE2_MASK 0x00000004U /**< slice2 	interrupt enable mask */
#define XRFDC_EN_INTR_SLICE3_MASK 0x00000008U /**< slice3 	interrupt enable mask */
#define XRFDC_INTR_COMMON_MASK 0x00000010U /**< Common interrupt enable mask */
/* @} */

/** @name Converter(X) interrupt register
 *
 * This register contains bits to enable different interrupts for block X.
 * @{
 */

#define XRFDC_INTR_OVR_RANGE_MASK 0x00000008U /**< Over Range 	interrupt mask */
#define XRFDC_INTR_OVR_VOLTAGE_MASK 0x00000004U /**< Over Voltage 	interrupt mask */
#define XRFDC_INTR_FIFO_OVR_MASK 0x00008000U /**< FIFO OF mask */
#define XRFDC_INTR_DAT_OVR_MASK 0x00004000U /**< Data OF mask */
#define XRFDC_INTR_CMODE_OVR_MASK 0x00040000U /**< Common mode OV mask */
#define XRFDC_INTR_CMODE_UNDR_MASK 0x00080000U /**< Common mode UV mask */
/* @} */

/** @name Multiband config register
 *
 * This register contains bits to configure multiband.
 * @{
 */

#define XRFDC_EN_MB_MASK 0x00000008U /**< multi-band adder mask */
#define XRFDC_EN_MB_SHIFT 3U /** <Enable Multiband shift */
#define XRFDC_DAC_MB_SEL_MASK 0x0003U /** <Local and remote select mask */
#define XRFDC_ALT_BOND_MASK 0x0200U /** <Alt bondout mask */
#define XRFDC_ALT_BOND_SHIFT 9U /** <Alt bondout shift */
#define XRFDC_ALT_BOND_CLKDP_MASK 0x4U /** <Alt bondout shift */
#define XRFDC_ALT_BOND_CLKDP_SHIFT 2U /** <Alt bondout shift */
#define XRFDC_MB_CONFIG_MASK 0x00000007U /** <Multiband Config mask */

/* @} */

/** @name Invsinc control register
 *
 * This register contains bits to configure Invsinc.
 * @{
 */
#define XRFDC_EN_INVSINC_MASK 0x00000001U /**< invsinc enable mask */
#define XRFDC_MODE_INVSINC_MASK 0x00000003U /**< invsinc mode mask */
/* @} */

/** @name OBS FIFO start register
 *
 * This register contains bits to configure Invsinc.
 * @{
 */
#define XRFDC_HSCOM_FIFO_START_OBS_EN_MASK 0x00000200U /**< invsinc enable mask */
#define XRFDC_HSCOM_FIFO_START_OBS_EN_SHIFT 9U /**< invsinc mode mask */
/* @} */

/** @name Signal Detector control register
 *
 * This register contains bits to configure Signal Detector.
 * @{
 */
#define XRFDC_ADC_SIG_DETECT_MASK 0xFF /**< signal detector mask */
#define XRFDC_ADC_SIG_DETECT_THRESH_MASK 0xFFFF /**< signal detector thresholds mask */
#define XRFDC_ADC_SIG_DETECT_THRESH_CNT_MASK 0xFFFF /**< signal detector thresholds counter mask */
#define XRFDC_ADC_SIG_DETECT_INTG_MASK 0x01 /**< leaky integrator enable mask */
#define XRFDC_ADC_SIG_DETECT_FLUSH_MASK 0x02 /**< leaky integrator flush mask */
#define XRFDC_ADC_SIG_DETECT_TCONST_MASK 0x1C /**< time constant mask */
#define XRFDC_ADC_SIG_DETECT_MODE_MASK 0x60 /**< mode mask */
#define XRFDC_ADC_SIG_DETECT_HYST_MASK 0x80 /**< hysteresis enable mask */
#define XRFDC_ADC_SIG_DETECT_INTG_SHIFT 0 /**< leaky integrator enable shift */
#define XRFDC_ADC_SIG_DETECT_FLUSH_SHIFT 1 /**< leaky integrator flush shift */
#define XRFDC_ADC_SIG_DETECT_TCONST_SHIFT 2 /**< time constant shift */
#define XRFDC_ADC_SIG_DETECT_MODE_WRITE_SHIFT 5 /**< mode shift fror writing */
#define XRFDC_ADC_SIG_DETECT_MODE_READ_SHIFT 6 /**< mode shift fror reading */
#define XRFDC_ADC_SIG_DETECT_HYST_SHIFT 7 /**< hysteresis enable shift */
/* @} */

/** @name CLK_DIV register
 *
 * This register contains the bits to control the clock
 * divider providing the clock fabric out.
 * @{
 */

#define XRFDC_FAB_CLK_DIV_MASK 0x0000000FU /**< clk div mask */
#define XRFDC_FAB_CLK_DIV_CAL_MASK 0x000000F0U /**< clk div cal mask */

/* @} */

/** @name Multiband Config
 *
 * This register contains bits to configure multiband for DAC.
 * @{
 */

#define XRFDC_MB_CFG_MASK 0x000001FFU /**< MB config mask */
#define XRFDC_MB_EN_4X_MASK 0x00000100U /**< Enable 4X MB mask */

/* @} */

/** @name Multi Tile Sync
 *
 * Multi-Tile Sync bit masks.
 * @{
 */

#define XRFDC_MTS_SRCAP_PLL_M 0x0100U
#define XRFDC_MTS_SRCAP_DIG_M 0x0100U
#define XRFDC_MTS_SRCAP_EN_TRX_M 0x0400U
#define XRFDC_MTS_SRCAP_INIT_M 0x8200U
#define XRFDC_MTS_SRCLR_T1_M 0x2000U
#define XRFDC_MTS_SRCLR_PLL_M 0x0200U
#define XRFDC_MTS_PLLEN_M 0x0001U
#define XRFDC_MTS_SRCOUNT_M 0x00FFU
#define XRFDC_MTS_DELAY_VAL_M 0x041FU
#define XRFDC_MTS_AMARK_CNT_M 0x00FFU
#define XRFDC_MTS_AMARK_LOC_M 0x0F0000U
#define XRFDC_MTS_AMARK_DONE_M 0x100000U

/* @} */

/** @name Output divider LSB register
 *
 * This register contains bits to configure output divisor
 * @{
 */

#define XRFDC_PLL_DIVIDER0_MASK 0x0CFFU
#define XRFDC_PLL_DIVIDER0_MODE_MASK 0x00C0U
#define XRFDC_PLL_DIVIDER0_BYP_OPDIV_MASK 0x0400U
#define XRFDC_PLL_DIVIDER0_BYP_PLL_MASK 0x0800U
#define XRFDC_PLL_DIVIDER0_VALUE_MASK 0x003FU
#define XRFDC_PLL_DIVIDER0_SHIFT 6U

/* @} */

/** @name Multi-tile sync and clock source control register
 *
 * This register contains bits to Multi-tile sync and clock source control
 * @{
 */
#define XRFDC_CLK_NETWORK_CTRL1_USE_PLL_MASK 0x1U /**< PLL clock mask */
#define XRFDC_CLK_NETWORK_CTRL1_USE_RX_MASK 0x2U /**< PLL clock mask */
#define XRFDC_CLK_NETWORK_CTRL1_REGS_MASK 0x3U /**< PLL clock mask */

/* @} */

/** @name PLL_CRS1 - PLL CRS1 register
 *
 * This register contains bits for VCO sel_auto, VCO band selection etc.,
 * @{
 */

#define XRFDC_PLL_CRS1_VCO_SEL_MASK 0x00008001U /**< VCO SEL Mask */
#define XRFDC_PLL_VCO_SEL_AUTO_MASK 0x00008000U /**< VCO Auto SEL Mask */

/* @} */

/** Register bits Shift, Width Masks
 *
 * @{
 */
#define XRFDC_DIGI_ANALOG_SHIFT4 4U
#define XRFDC_DIGI_ANALOG_SHIFT8 8U
#define XRFDC_DIGI_ANALOG_SHIFT12 12U

/* @} */

/** @name FIFO Delays
 *
 * This register contains bits for delaying the FIFOs.,
 * @{
 */

#define XRFDC_DAC_FIFO_DELAY_MASK 0x000000FFFU /**< DAC FIFO ReadPtr Delay */
#define XRFDC_ADC_FIFO_DELAY_MASK 0x0000001C0U /**< ADC FIFO ReadPtr Delay */
#define XRFDC_ADC_FIFO_DELAY_SHIFT 6U /**< ADC FIFO ReadPtr Shift */

/* @} */

#define XRFDC_IXR_FIFOUSRDAT_MASK 0x0000000FU
#define XRFDC_DAC_IXR_FIFOUSRDAT_SUPP_MASK 0x30000000U
#define XRFDC_DAC_IXR_FIFOUSRDAT_MASK 0x3000000FU
#define XRFDC_IXR_FIFOUSRDAT_OBS_MASK 0x0000F000U
#define XRFDC_IXR_FIFOUSRDAT_OF_MASK 0x00000001U
#define XRFDC_IXR_FIFOUSRDAT_UF_MASK 0x00000002U
#define XRFDC_IXR_FIFOMRGNIND_OF_MASK 0x00000004U
#define XRFDC_IXR_FIFOMRGNIND_UF_MASK 0x00000008U
#define XRFDC_DAC_IXR_FIFOACTIND_OF_MASK 0x20000000U
#define XRFDC_DAC_IXR_FIFOACTIND_UF_MASK 0x10000000U
#define XRFDC_ADC_IXR_DATAPATH_MASK 0x00000FF0U
#define XRFDC_ADC_IXR_DMON_STG_MASK 0x000003F0U
#define XRFDC_DAC_IXR_DATAPATH_MASK 0x000FFFF0U
#define XRFDC_DAC_IXR_INTP_STG_MASK 0x000003F0U
#define XRFDC_DAC_IXR_INTP_I_STG0_MASK 0x00000010U
#define XRFDC_DAC_IXR_INTP_I_STG1_MASK 0x00000020U
#define XRFDC_DAC_IXR_INTP_I_STG2_MASK 0x00000040U
#define XRFDC_DAC_IXR_INTP_Q_STG0_MASK 0x00000080U
#define XRFDC_DAC_IXR_INTP_Q_STG1_MASK 0x00000100U
#define XRFDC_DAC_IXR_INTP_Q_STG2_MASK 0x00000200U
#define XRFDC_ADC_IXR_DMON_I_STG0_MASK 0x00000010U
#define XRFDC_ADC_IXR_DMON_I_STG1_MASK 0x00000020U
#define XRFDC_ADC_IXR_DMON_I_STG2_MASK 0x00000040U
#define XRFDC_ADC_IXR_DMON_Q_STG0_MASK 0x00000080U
#define XRFDC_ADC_IXR_DMON_Q_STG1_MASK 0x00000100U
#define XRFDC_ADC_IXR_DMON_Q_STG2_MASK 0x00000200U
#define XRFDC_IXR_QMC_GAIN_PHASE_MASK 0x00000400U
#define XRFDC_IXR_QMC_OFFST_MASK 0x00000800U
#define XRFDC_DAC_IXR_INVSNC_OF_MASK 0x00001000U
#define XRFDC_DAC_IXR_MXR_HLF_I_MASK 0x00002000U
#define XRFDC_DAC_IXR_MXR_HLF_Q_MASK 0x00004000U
#define XRFDC_DAC_IXR_DP_SCALE_MASK 0x00008000U
#define XRFDC_DAC_IXR_INTP_I_STG3_MASK 0x00010000U
#define XRFDC_DAC_IXR_INTP_Q_STG3_MASK 0x00020000U
#define XRFDC_DAC_IXR_IMR_OV_MASK 0x00040000U
#define XRFDC_DAC_IXR_INV_SINC_EVEN_NYQ_MASK 0x00080000U
#define XRFDC_SUBADC_IXR_DCDR_MASK 0x00FF0000U
#define XRFDC_SUBADC0_IXR_DCDR_OF_MASK 0x00010000U
#define XRFDC_SUBADC0_IXR_DCDR_UF_MASK 0x00020000U
#define XRFDC_SUBADC1_IXR_DCDR_OF_MASK 0x00040000U
#define XRFDC_SUBADC1_IXR_DCDR_UF_MASK 0x00080000U
#define XRFDC_SUBADC2_IXR_DCDR_OF_MASK 0x00100000U
#define XRFDC_SUBADC2_IXR_DCDR_UF_MASK 0x00200000U
#define XRFDC_SUBADC3_IXR_DCDR_OF_MASK 0x00400000U
#define XRFDC_SUBADC3_IXR_DCDR_UF_MASK 0x00800000U
#define XRFDC_ADC_OVR_VOLTAGE_MASK 0x04000000U
#define XRFDC_COMMON_MASK 0x01000000U
#define XRFDC_ADC_OVR_RANGE_MASK 0x08000000U
#define XRFDC_ADC_CMODE_OVR_MASK 0x10000000U
#define XRFDC_ADC_CMODE_UNDR_MASK 0x20000000U
#define XRFDC_ADC_DAT_OVR_MASK 0x40000000U
#define XRFDC_DAT_OVR_MASK 0x40000000U
#define XRFDC_ADC_FIFO_OVR_MASK 0x80000000U
#define XRFDC_FIFO_OVR_MASK 0x80000000U
#define XRFDC_DAC_MC_CFG2_OPCSCAS_MASK 0x0000F8F8U
#define XRFDC_DAC_MC_CFG2_BLDGAIN_MASK 0x0000FFC0U
#define XRFDC_DAC_MC_CFG3_CSGAIN_MASK 0x0000FFC0U
#define XRFDC_DAC_MC_CFG2_OPCSCAS_20MA 0x00004858U
#define XRFDC_DAC_MC_CFG3_CSGAIN_20MA 0x000087C0U
#define XRFDC_DAC_MC_CFG2_OPCSCAS_32MA 0x0000A0D8U
#define XRFDC_DAC_MC_CFG3_CSGAIN_32MA 0x0000FFC0U
#define XRFDC_DAC_MC_CFG2_GEN1_COMP_MASK 0x0020U
#define XRFDC_DAC_MC_CFG3_OPT_LUT_MASK(X) ((X == 0) ? 0x03E0U : 0x03F0U)
#define XRFDC_DAC_MC_CFG3_OPT_MASK 0x001FU
#define XRFDC_DAC_MC_CFG3_UPDATE_MASK 0x0020U
#define XRFDC_DAC_MC_CFG0_CAS_BLDR_MASK 0xE000U
#define XRFDC_DAC_MC_CFG2_CAS_BIAS_MASK 0x001FU
#define XRFDC_ADC_DSA_RTS_PIN_MASK 0x0020U
#define XRFDC_ADC_DSA_CODE_MASK 0x001FU
#define XRFDC_ADC_DSA_UPDT_MASK 0x0001U

#define XRFDC_DAC_MC_CFG2_BLDGAIN_SHIFT 6U
#define XRFDC_DAC_MC_CFG3_CSGAIN_SHIFT 6U
#define XRFDC_DAC_MC_CFG3_OPT_LUT_SHIFT(X) ((X == 0) ? 5U : 4U)
#define XRFDC_ADC_OVR_VOL_RANGE_SHIFT 24U
#define XRFDC_ADC_DAT_FIFO_OVR_SHIFT 16U
#define XRFDC_DAT_FIFO_OVR_SHIFT 16U
#define XRFDC_ADC_SUBADC_DCDR_SHIFT 16U
#define XRFDC_IXR_FIFOUSRDAT_OBS_SHIFT 12U
#define XRFDC_DAC_IXR_FIFOUSRDAT_SUPP_SHIFT 24U
#define XRFDC_DATA_PATH_SHIFT 4U
#define XRFDC_ADC_CMODE_SHIFT 10U
#define XRFDC_COMMON_SHIFT 20U
#define XRFDC_DAC_MC_CFG2_GEN1_COMP_SHIFT 5U
#define XRFDC_ADC_DSA_RTS_PIN_SHIFT 5U

#define XRFDC_DAC_TILE_DRP_ADDR(X) (0x6000U + (X * 0x4000U))
#define XRFDC_DAC_TILE_CTRL_STATS_ADDR(X) (0x4000U + (X * 0x4000U))
#define XRFDC_ADC_TILE_DRP_ADDR(X) (0x16000U + (X * 0x4000U))
#define XRFDC_ADC_TILE_CTRL_STATS_ADDR(X) (0x14000U + (X * 0x4000U))
#define XRFDC_CTRL_STATS_OFFSET 0x0U
#define XRFDC_HSCOM_ADDR 0x1C00U
#define XRFDC_BLOCK_ADDR_OFFSET(X) (X * 0x400U)
#define XRFDC_TILE_DRP_OFFSET 0x2000U

/***************** Macros (Inline Functions) Definitions *********************/
#define XRFdc_In64 metal_io_read64
#define XRFdc_Out64 metal_io_write64

#define XRFdc_In32 metal_io_read32
#define XRFdc_Out32 metal_io_write32

#define XRFdc_In16 metal_io_read16
#define XRFdc_Out16 metal_io_write16

#define XRFdc_In8 metal_io_read8
#define XRFdc_Out8 metal_io_write8

/****************************************************************************/
/**
* Read a register.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddress contains the base address of the device.
* @param    RegOffset contains the offset from the 1st register of the
*           device to the target register.
*
* @return   The value read from the register.
*
* @note     C-Style signature:
*           u32 XRFdc_ReadReg64(XRFdc *InstancePtr. u32 BaseAddress, s32 RegOffset)
*
******************************************************************************/
#define XRFdc_ReadReg64(InstancePtr, BaseAddress, RegOffset)                                                           \
	XRFdc_In64(InstancePtr->io, ((u32)RegOffset + (u32)BaseAddress))

/***************************************************************************/
/**
* Write to a register.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddress contains the base address of the device.
* @param    RegOffset contains the offset from the 1st register of the
*           device to target register.
* @param    RegisterValue is the value to be written to the register.
*
* @note     None.
*
* @note     C-Style signature:
*	void XRFdc_WriteReg64(XRFdc *InstancePtr, u32 BaseAddress, s32 RegOffset,
*	u64 RegisterValue)
*
******************************************************************************/
#define XRFdc_WriteReg64(InstancePtr, BaseAddress, RegOffset, RegisterValue)                                           \
	XRFdc_Out64((InstancePtr->io), ((u32)RegOffset + (u32)BaseAddress), (u32)(RegisterValue))

/****************************************************************************/
/**
* Read a register.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddress contains the base address of the device.
* @param    RegOffset contains the offset from the 1st register of the
*           device to the target register.
*
* @return   The value read from the register.
*
* @note     C-Style signature:
*           u32 XRFdc_ReadReg(XRFdc *InstancePtr, u32 BaseAddress. int RegOffset)
*
******************************************************************************/
#define XRFdc_ReadReg(InstancePtr, BaseAddress, RegOffset)                                                             \
	XRFdc_In32((InstancePtr->io), ((u32)BaseAddress + (u32)RegOffset))

/***************************************************************************/
/**
* Write to a register.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddress contains the base address of the device.
* @param    RegOffset contains the offset from the 1st register of the
*           device to target register.
* @param    RegisterValue is the value to be written to the register.
*
* @note     None.
*
* @note     C-Style signature:
*           void XRFdc_WriteReg(XRFdc *InstancePtr, u32 BaseAddress, int RegOffset,
*           u32 RegisterValue)
*
******************************************************************************/
#define XRFdc_WriteReg(InstancePtr, BaseAddress, RegOffset, RegisterValue)                                             \
	XRFdc_Out32((InstancePtr->io), ((u32)RegOffset + (u32)BaseAddress), (u32)(RegisterValue))

/****************************************************************************/
/**
* Read a register.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddress contains the base address of the device.
* @param    RegOffset contains the offset from the 1st register of the
*           device to the target register.
*
* @return   The value read from the register.
*
* @note     C-Style signature:
*           u16 XRFdc_ReadReg16(XRFdc *InstancePtr, u32 BaseAddress. int RegOffset)
*
******************************************************************************/
#define XRFdc_ReadReg16(InstancePtr, BaseAddress, RegOffset)                                                           \
	XRFdc_In16((InstancePtr->io), ((u32)RegOffset + (u32)BaseAddress))

/***************************************************************************/
/**
* Write to a register.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddress contains the base address of the device.
* @param    RegOffset contains the offset from the 1st register of the
*           device to target register.
* @param    RegisterValue is the value to be written to the register.
*
* @note     None.
*
* @note     C-Style signature:
*	void XRFdc_WriteReg16(XRFdc *InstancePtr, u32 BaseAddress, int RegOffset,
*	u16 RegisterValue)
*
******************************************************************************/
#define XRFdc_WriteReg16(InstancePtr, BaseAddress, RegOffset, RegisterValue)                                           \
	XRFdc_Out16((InstancePtr->io), ((u32)RegOffset + (u32)BaseAddress), (u32)(RegisterValue))

/****************************************************************************/
/**
* Read a register.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddress contains the base address of the device.
* @param    RegOffset contains the offset from the 1st register of the
*           device to the target register.
*
* @return   The value read from the register.
*
* @note     C-Style signature:
*           u8 XRFdc_ReadReg8(XRFdc *InstancePtr, u32 BaseAddress. int RegOffset)
*
******************************************************************************/
#define XRFdc_ReadReg8(InstancePtr, BaseAddress, RegOffset)                                                            \
	XRFdc_In8((InstancePtr->io), ((u32)RegOffset + (u32)BaseAddress))

/***************************************************************************/
/**
* Write to a register.
*
* @param    InstancePtr is a pointer to the XRfdc instance.
* @param    BaseAddress contains the base address of the device.
* @param    RegOffset contains the offset from the 1st register of the
*           device to target register.
* @param    RegisterValue is the value to be written to the register.
*
* @note     None.
*
* @note     C-Style signature:
*	void XRFdc_WriteReg8(XRFdc *InstancePtr, u32 BaseAddress, int RegOffset,
*	u8 RegisterValue)
*
******************************************************************************/
#define XRFdc_WriteReg8(InstancePtr, BaseAddress, RegOffset, RegisterValue)                                            \
	XRFdc_Out8((InstancePtr->io), ((u32)RegOffset + (u32)BaseAddress), (u32)(RegisterValue))

#ifdef __cplusplus
}
#endif

#endif /* RFDC_HW_H_ */
/** @} */
