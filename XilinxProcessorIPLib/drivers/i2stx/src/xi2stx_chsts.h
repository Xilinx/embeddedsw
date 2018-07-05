/******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc. All rights reserved.
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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 *****************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xi2stx_chsts.h
 * @addtogroup i2stx_1_0
 * @{
 *
 * Format status related offsets & masks definitions related to the
 * channel status format.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    11/16/17 Initial release.
 * </pre>
 *
 *****************************************************************************/

#ifndef XI2STX_CHSTS_H
#define XI2STX_CHSTS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xi2stx.h"

/************************** Constant Definitions *****************************/
/**
* @name AES Status and Register Masks and Shifts.For formats/line protocols
* check the AES Standard specifications document.
* @{
*/
#define XI2S_TX_AES_STS_USE_OF_CH_STS_BLK_SHIFT (0) /**< Use of Channel Status
						      Block bit shift */
#define XI2S_TX_AES_STS_USE_OF_CH_STS_BLK_MASK \
	(1 << XI2S_TX_AES_STS_USE_OF_CH_STS_BLK_SHIFT) /**< Use of Channel
							 Status Block mask */

#define XI2S_TX_AES_STS_LINEAR_PCM_ID_SHIFT (1) /**< Linear PCM
						  Identification bit shift */
#define XI2S_TX_AES_STS_LINEAR_PCM_ID_MASK \
	(1 << XI2S_TX_AES_STS_LINEAR_PCM_ID_SHIFT) /**< Linear PCM
						     Identification mask */

#define XI2S_TX_AES_STS_AUDIO_SIG_PRE_EMPH_SHIFT (2) /**< Audio signal pre-
						       emphasis bit shift */
#define XI2S_TX_AES_STS_AUDIO_SIG_PRE_EMPH_MASK \
	(0x7 << XI2S_TX_AES_STS_AUDIO_SIG_PRE_EMPH_SHIFT) /**< Audio signal
							    pre-emphasis mask */

#define XI2S_TX_AES_STS_LOCK_INDICATION_SHIFT (5) /**< lock indication
						    bit shift */
#define XI2S_TX_AES_STS_LOCK_INDICATION_MASK \
	(1 << XI2S_TX_AES_STS_LOCK_INDICATION_SHIFT) /**< Lock indication mask*/

#define XI2S_TX_AES_STS_SAMPLING_FREQ_E_SHIFT (6) /**< Sampling Frequency 0
						    bit shift */
#define XI2S_TX_AES_STS_SAMPLING_FREQ_E_MASK \
	(0x3 << XI2S_TX_AES_STS_SAMPLING_FREQ_E_SHIFT) /**< Sampling Frequency
							 0 mask */

#define XI2S_TX_AES_STS_CH_MODE_SHIFT (0) /**< Channel mode bit shift */
#define XI2S_TX_AES_STS_CH_MODE_MASK \
	(0xF << XI2S_TX_AES_STS_CH_MODE_SHIFT) /**< Channel mode mask */

#define XI2S_TX_AES_STS_USR_BITS_MGMT_SHIFT (4) /**< User Bits Management
						  bit shift */
#define XI2S_TX_AES_STS_USR_BITS_MGMT_MASK \
	(0xF << XI2S_TX_AES_STS_USR_BITS_MGMT_SHIFT) /**< User Bits
						       Management mask */

#define XI2S_TX_AES_STS_USEOF_AUX_SMPL_BITS_SHIFT (0) /**< Use of auxiliary
							sample bits
							bit shift */
#define XI2S_TX_AES_STS_USEOF_AUX_SMPL_BITS_MASK \
	(0x7 << XI2S_TX_AES_STS_USEOF_AUX_SMPL_BITS_SHIFT) /**< Use of
							     Auxiliary sample
							     bits mask */

#define XI2S_TX_AES_STS_SRC_WORD_LENGTH_SHIFT (3) /**< Source word
						    length bit shift */
#define XI2S_TX_AES_STS_SRC_WORD_LENGTH_MASK \
	(0x7 << XI2S_TX_AES_STS_SRC_WORD_LENGTH_SHIFT) /**< Source word
							 length mask */

#define XI2S_TX_AES_STS_INDICATE_ALIGN_LEVEL_SHIFT (6) /**< Indication of
							 Alignment level
							 bit shift */
#define XI2S_TX_AES_STS_INDICATE_ALIGN_LEVEL_MASK \
	(0x3 << XI2S_TX_AES_STS_INDICATE_ALIGN_LEVEL_SHIFT) /**< Indication of
							      Alignment level
							      mask */

#define XI2S_TX_AES_STS_CH_NUM0_SHIFT (0) /**< Channel Number (0) bit shift */
#define XI2S_TX_AES_STS_CH_NUM0_MASK \
	(0x7F << XI2S_TX_AES_STS_CH_NUM0_SHIFT) /**< Channel Number (0) mask */

#define XI2S_TX_AES_STS_MC_CH_MODE_SHIFT (7) /**< Multichannel mode bit shift */
#define XI2S_TX_AES_STS_MC_CH_MODE_MASK \
	(1 << XI2S_TX_AES_STS_MC_CH_MODE_SHIFT) /**< Multichannel mode mask */

#define XI2S_TX_AES_STS_CH_NUM1_SHIFT (0) /**< Channel Number (1) bit shift */
#define XI2S_TX_AES_STS_CH_NUM1_MASK \
	(0xF << XI2S_TX_AES_STS_CH_NUM1_SHIFT) /**< Channel Number (1) mask */

#define XI2S_TX_AES_STS_MC_CH_MODE_NUM_SHIFT (4) /**< Multichannel mode
						   number bit shift */
#define XI2S_TX_AES_STS_MC_CH_MODE_NUM_MASK \
	(0x7 << XI2S_TX_AES_STS_MC_CH_MODE_NUM_SHIFT) /**< Multichannel
							mode number mask */

#define XI2S_TX_AES_STS_DIGITAL_AUDIO_REF_SIG_SHIFT (0) /**< Digital Reference
							  Audio signal
							  bit shift */
#define XI2S_TX_AES_STS_DIGITAL_AUDIO_REF_SIG_MASK \
	(0x3 << XI2S_TX_AES_STS_DIGITAL_AUDIO_REF_SIG_SHIFT) /**< Digital
							       Reference Audio
							       signal mask */

#define XI2S_TX_AES_STS_RSVD_BUT_UNDEF0_SHIFT (2) /**< Reserved but undefined
						    (0) bit shift */
#define XI2S_TX_AES_STS_RSVD_BUT_UNDEF0_MASK \
	(1 << XI2S_TX_AES_STS_RSVD_BUT_UNDEF0_SHIFT) /**< Reserved but
						       undefined (0) mask */

#define XI2S_TX_AES_STS_SAMPLING_FREQ_Q_SHIFT (3) /**< Sampling Frequency
						    (1) bit shift */
#define XI2S_TX_AES_STS_SAMPLING_FREQ_Q_MASK \
	(0xF << XI2S_TX_AES_STS_SAMPLING_FREQ_Q_SHIFT) /**< Sampling Frequency
							 (1) mask */

#define XI2S_TX_AES_STS_SAMPLING_FREQ_SCALE_FLAG_SHIFT (7) /**< Sampling
							     Frequency scaling
							     flag bit shift */
#define XI2S_TX_AES_STS_SAMPLING_FREQ_SCALE_FLAG_MASK \
	(1 << XI2S_TX_AES_STS_SAMPLING_FREQ_SCALE_FLAG_SHIFT) /**< Sampling
							      Frequency scaling
							      flag mask */

#define XI2S_TX_AES_STS_RSVD_BUT_UNDEF1_SHIFT (0) /**< Reserved but undefined
						    (1) bit shift */
#define XI2S_TX_AES_STS_RSVD_BUT_UNDEF1_MASK \
	(0xFF << XI2S_TX_AES_STS_RSVD_BUT_UNDEF1_SHIFT) /**< Reserved but
							  undefined (1) mask */

#define XI2S_TX_AES_STS_ALPHANUM_CH_ORG_DATA_OFFSET (6) /**< Alphanumeric
							  channel origin data
							  register(s) offset */

#define XI2S_TX_AES_STS_ALPHANUM_CH_DEST_DATA_OFFSET (10) /**< Alphanumeric
							    channel destination
							    data bit shift */

#define XI2S_TX_AES_STS_LOCAL_SAMPLE_ADDRCODE_OFFSET (14) /**< Local sample
							    address code
							    register(s) offset*/

#define XI2S_TX_AES_STS_TIMEOFDAY_SAMPLE_ADDRCODE_OFFSET (18) /**< Time-of-day
								sample address
								code register(s)
							       	offset */

#define XI2S_TX_AES_STS_RELIABLE_FLAGS_OFFSET (22) /**< Reliability flags
						     bit shift */

#define XI2S_TX_AES_STS_CRC_CHAR_OFFSET (23) /**< Cyclic redundancy
					       check character bit shift */
/** @} */

/**************************** Type Definitions *******************************/
/** @} */

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XI2s_Tx_ReslveAesChStat(u8 I2stx_SrcBuf[]);

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

#endif /* XI2STX_CHSTS_H */
/** @} */
