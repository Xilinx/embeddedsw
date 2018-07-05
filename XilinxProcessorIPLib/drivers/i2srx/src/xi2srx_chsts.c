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
 * @file xi2srx_chsts.c
 * @addtogroup i2srx_v1_0
 * @{
 *
 * This file implements the channel status format related functions.
 * For formats/line protocols check the AES standard specifications document.
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date      Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0    kar   01/25/18  Initial release.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xi2srx_chsts.h"
#include "xi2srx.h"
#include "xi2srx_hw.h"
#include "xi2srx_debug.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * This function reads the source buffer and writes to a destination buffer.
 * Before calling this API user application should write 192 bits i.e.
 *         24 bytes to the array I2srx_SrcBuf.
 * @param  I2srx_SrcBuf is the source buffer which has 192 channel
 *         status bits.
 * @param  I2srx_DstBuf is the destination buffer to store the
 *         24 bytes.
 *
 * @return void.
 *
 *****************************************************************************/
void XI2s_Rx_SetAesChStat(u32 I2srx_SrcBuf[], u8 I2srx_DstBuf[])
{
	u8 I2srx_useOf_ch_sts_blk; /**< use of channel status block */
	u8 I2srx_linear_pcm_id; /**< linear PCM identification */
	u8 I2srx_audio_sig_pre_emph; /**< audio signal Pre-emphasis */
	u8 I2srx_lock_indication; /**< lock indication */
	u8 I2srx_sampling_freq_e; /**< sampling frequency */
	u8 I2srx_ch_mode; /**< channel mode */
	u8 I2srx_usr_bits_mgmt; /**< user bits management */
	u8 I2srx_useOf_aux_smpl_bits; /**< use of auxiliary sample bits */
	u8 I2srx_src_word_length;	/**< source word length */
	u8 I2srx_indicate_align_level; 	/**< indication of alignment level */
	u8 I2srx_mc_ch_mode; /**< channel mode */
	u8 I2srx_byte3_mc0_ch_num; /**< Channel number 0 */
	u8 I2srx_byte3_mc1_ch_num;	/**< Channel number 1 */
	u8 I2srx_byte3_mc1_ch_mode_num; /**< multi channel1 mode number */
	u8 I2srx_digital_audio_ref_sig;	/**< digital audio reference signal */
	u8 I2srx_rsvd_but_undef0; /**< reserved but undefined */
	u8 I2srx_sampling_freq_q; /**< sampling frequency */
	u8 I2srx_sampling_freq_scale_flag; /**< sampling frequency
					     scaling flag */
	u8 I2srx_rsvd_but_undef1; /**< reserved but undefined */
	u32 I2srx_alphanum_ch_org_data; /**< Alphanumeric channel origin data */
	u32 I2srx_alphanum_ch_dest_data; /**< Alphanumeric channel destination
					   data*/
	u32 I2srx_local_sample_addrcode; /**< Local sample address code */
	u32 I2srx_timeOfDay_sample_addrcode;/**< Time-of-day sample
					      address code */
	u8 I2srx_reliable_flags;/**< Reliability flags */
	u8 I2srx_crc_char;/**< Cyclic redundancy check character */
	u32 Data;
	Xil_AssertVoid(I2srx_SrcBuf != NULL);

	I2srx_useOf_ch_sts_blk = I2srx_SrcBuf[0];
	I2srx_linear_pcm_id = I2srx_SrcBuf[1];
	I2srx_audio_sig_pre_emph = I2srx_SrcBuf[2];
	I2srx_lock_indication = I2srx_SrcBuf[3];
	I2srx_sampling_freq_e = I2srx_SrcBuf[4];
	I2srx_ch_mode = I2srx_SrcBuf[5];
	I2srx_usr_bits_mgmt = I2srx_SrcBuf[6];
	I2srx_useOf_aux_smpl_bits = I2srx_SrcBuf[7];
	I2srx_src_word_length= I2srx_SrcBuf[8];
	I2srx_indicate_align_level= I2srx_SrcBuf[9];
	I2srx_mc_ch_mode =0 ;
	I2srx_byte3_mc0_ch_num= I2srx_SrcBuf[10];
	I2srx_byte3_mc1_ch_num= I2srx_SrcBuf[11];
	I2srx_byte3_mc1_ch_mode_num= I2srx_SrcBuf[12];
	I2srx_digital_audio_ref_sig= I2srx_SrcBuf[13];
	I2srx_rsvd_but_undef0= I2srx_SrcBuf[14];
	I2srx_sampling_freq_q= I2srx_SrcBuf[15];
	I2srx_sampling_freq_scale_flag= I2srx_SrcBuf[16];
	I2srx_rsvd_but_undef1= I2srx_SrcBuf[17];
	I2srx_alphanum_ch_org_data= I2srx_SrcBuf[18];
	I2srx_alphanum_ch_dest_data= I2srx_SrcBuf[19];
	I2srx_local_sample_addrcode= I2srx_SrcBuf[20];
	I2srx_timeOfDay_sample_addrcode= I2srx_SrcBuf[21];
	I2srx_reliable_flags= I2srx_SrcBuf[22];
	I2srx_crc_char= I2srx_SrcBuf[23];

	Data = (I2srx_useOf_ch_sts_blk)|
		((I2srx_linear_pcm_id) <<
		 XI2S_RX_AES_STS_LINEAR_PCM_ID_SHIFT)|
		((I2srx_audio_sig_pre_emph) <<
		 XI2S_RX_AES_STS_AUDIO_SIG_PRE_EMPH_SHIFT)|
		((I2srx_lock_indication) <<
		 XI2S_RX_AES_STS_LOCK_INDICATION_SHIFT)|
		((I2srx_sampling_freq_e) <<
		 XI2S_RX_AES_STS_SAMPLING_FREQ_E_SHIFT);
	I2srx_DstBuf[0] = Data;

	Data = (I2srx_ch_mode)|
		((I2srx_usr_bits_mgmt) <<
		 XI2S_RX_AES_STS_USR_BITS_MGMT_SHIFT);
	I2srx_DstBuf[1] = Data;

	Data = (I2srx_useOf_aux_smpl_bits)|
		((I2srx_src_word_length) <<
		 XI2S_RX_AES_STS_SRC_WORD_LENGTH_SHIFT) |
		((I2srx_indicate_align_level) <<
		 XI2S_RX_AES_STS_INDICATE_ALIGN_LEVEL_SHIFT);
	I2srx_DstBuf[2] = Data;
	if (I2srx_mc_ch_mode == 0) {
		Data = (I2srx_byte3_mc0_ch_num);
		I2srx_DstBuf[3] = Data;
	}
	if (I2srx_mc_ch_mode == 1) {
		Data = (I2srx_mc_ch_mode << (XI2S_RX_AES_STS_MC_CH_MODE_SHIFT))
		       	| (I2srx_byte3_mc1_ch_num) |
			((I2srx_byte3_mc1_ch_mode_num) <<
			 (XI2S_RX_AES_STS_MC_CH_MODE_NUM_SHIFT));
		I2srx_DstBuf[3] = Data;
	}

	Data = (I2srx_digital_audio_ref_sig) |
		((I2srx_rsvd_but_undef0) <<
		 XI2S_RX_AES_STS_RSVD_BUT_UNDEF0_SHIFT)|
		((I2srx_sampling_freq_q) <<
		 XI2S_RX_AES_STS_SAMPLING_FREQ_Q_SHIFT)|
		((I2srx_sampling_freq_scale_flag) <<
		 XI2S_RX_AES_STS_SAMPLING_FREQ_SCALE_FLAG_SHIFT);
	I2srx_DstBuf[4] = Data;

	Data =  (I2srx_rsvd_but_undef1);
	I2srx_DstBuf[5] = Data;

	Data = (I2srx_alphanum_ch_org_data) &
		(XI2S_RX_AES_STS_ALPHANUM_CH_ORG_DATA_BYTEO_MASK);
	I2srx_DstBuf[6] = Data;

	Data = ((I2srx_alphanum_ch_org_data) &
			(XI2S_RX_AES_STS_ALPHANUM_CH_ORG_DATA_BYTE1_MASK)) >>
		XI2S_RX_AES_STS_ALPHANUM_CH_ORG_DATA_BYTE1_SHIFT;
	I2srx_DstBuf[7] = Data;

	Data =  ((I2srx_alphanum_ch_org_data) &
			(XI2S_RX_AES_STS_ALPHANUM_CH_ORG_DATA_BYTE2_MASK)) >>
		XI2S_RX_AES_STS_ALPHANUM_CH_ORG_DATA_BYTE2_SHIFT;
	I2srx_DstBuf[8] = Data;

	Data = ((I2srx_alphanum_ch_org_data) &
			(XI2S_RX_AES_STS_ALPHANUM_CH_ORG_DATA_BYTE3_MASK))>>
		XI2S_RX_AES_STS_ALPHANUM_CH_ORG_DATA_BYTE3_SHIFT;
	I2srx_DstBuf[9] = Data;

	Data = (I2srx_alphanum_ch_dest_data) &
		(XI2S_RX_AES_STS_ALPHANUM_CH_DEST_DATA_BYTEO_MASK);
	I2srx_DstBuf[10] = Data;

	Data = ((I2srx_alphanum_ch_dest_data) &
			(XI2S_RX_AES_STS_ALPHANUM_CH_DEST_DATA_BYTE1_MASK)) >>
		(XI2S_RX_AES_STS_ALPHANUM_CH_DEST_DATA_BYTE1_SHIFT);
	I2srx_DstBuf[11] = Data;
	Data = ((I2srx_alphanum_ch_dest_data) &
			(XI2S_RX_AES_STS_ALPHANUM_CH_DEST_DATA_BYTE2_MASK)) >>
		(XI2S_RX_AES_STS_ALPHANUM_CH_DEST_DATA_BYTE2_SHIFT);
	I2srx_DstBuf[12] = Data;

	Data = ((I2srx_alphanum_ch_dest_data) &
			(XI2S_RX_AES_STS_ALPHANUM_CH_DEST_DATA_BYTE3_MASK)) >>
		(XI2S_RX_AES_STS_ALPHANUM_CH_DEST_DATA_BYTE3_SHIFT);
	I2srx_DstBuf[13] = Data;
	Data = ((I2srx_local_sample_addrcode) &
			(XI2S_RX_AES_STS_LOCAL_SAMPLE_ADDRESS_CODE_BYTEO_MASK));
	I2srx_DstBuf[14] = Data;

	Data = ((I2srx_local_sample_addrcode) &
			(XI2S_RX_AES_STS_LOCAL_SAMPLE_ADDRESS_CODE_BYTE1_MASK))
		>> (XI2S_RX_AES_STS_LOCAL_SAMPLE_ADDRESS_CODE_BYTE1_SHIFT);
	I2srx_DstBuf[15] = Data;

	Data = ((I2srx_local_sample_addrcode) &
			(XI2S_RX_AES_STS_LOCAL_SAMPLE_ADDRESS_CODE_BYTE2_MASK))
	       	>> (XI2S_RX_AES_STS_LOCAL_SAMPLE_ADDRESS_CODE_BYTE2_SHIFT);
	I2srx_DstBuf[16] = Data;

	Data = ((I2srx_local_sample_addrcode) &
			(XI2S_RX_AES_STS_LOCAL_SAMPLE_ADDRESS_CODE_BYTE3_MASK))
		>> (XI2S_RX_AES_STS_LOCAL_SAMPLE_ADDRESS_CODE_BYTE3_SHIFT);
	I2srx_DstBuf[17] = Data;

	Data = ((I2srx_timeOfDay_sample_addrcode) &
		(XI2S_RX_AES_STS_TIME_OF_DAY_SAMPLE_ADDRESS_CODE_BYTEO_MASK));
	I2srx_DstBuf[18] = Data;

	Data = ((I2srx_timeOfDay_sample_addrcode) &
		(XI2S_RX_AES_STS_TIME_OF_DAY_SAMPLE_ADDRESS_CODE_BYTE1_MASK))>>
		(XI2S_RX_AES_STS_TIME_OF_DAY_SAMPLE_ADDRESS_CODE_BYTE1_SHIFT);
	I2srx_DstBuf[19] = Data;

	Data = ((I2srx_timeOfDay_sample_addrcode) &
		(XI2S_RX_AES_STS_TIME_OF_DAY_SAMPLE_ADDRESS_CODE_BYTE2_MASK)) >>
		(XI2S_RX_AES_STS_TIME_OF_DAY_SAMPLE_ADDRESS_CODE_BYTE2_SHIFT);
	I2srx_DstBuf[20] = Data;

	Data = ((I2srx_timeOfDay_sample_addrcode) &
		(XI2S_RX_AES_STS_TIME_OF_DAY_SAMPLE_ADDRESS_CODE_BYTE3_MASK)) >>
		(XI2S_RX_AES_STS_TIME_OF_DAY_SAMPLE_ADDRESS_CODE_BYTE3_SHIFT);
	I2srx_DstBuf[21] = Data;

	Data = I2srx_reliable_flags;
	I2srx_DstBuf[22] = Data;

	Data = I2srx_crc_char;
	I2srx_DstBuf[23] = Data;
}
/** @} */
