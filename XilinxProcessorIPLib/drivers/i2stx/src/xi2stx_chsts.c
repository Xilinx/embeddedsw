/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xi2stx_chsts.c
 * @addtogroup i2stx_v2_1
 * @{
 *
 * This file implements the channel status format related functions.For
 * formats/line protocols check the AES Standard specifications document.
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    11/16/17 Initial release.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xi2stx_chsts.h"
#include "xi2stx.h"
#include "xi2stx_hw.h"
#include "xi2stx_debug.h"
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
 * This function reads the array I2stx_SrcBuf which has the values of all the
 * I2S Transmitter AES status registers, extracts the required bits and
 * prints them. Before calling this API, Call API XI2s_Tx_GetAesChStatus.
 *
 * @param  I2stx_SrcBuf is an array that contains the values of all the
 *         the AES Status registers.
 *
 *****************************************************************************/
void XI2s_Tx_ReslveAesChStat(u8 I2stx_SrcBuf[])
{
	u8 I2stx_useOf_ch_sts_blk; /**< use of channel status block */
	u8 I2stx_linear_pcm_id; /**< linear PCM identification */
	u8 I2stx_audio_sig_pre_emph; /**< audio signal Pre-emphasis */
	u8 I2stx_lock_indication; /**< lock indication */
	u8 I2stx_sampling_freq_e; /**< sampling frequency */
	u8 I2stx_ch_mode; /**< channel mode */
	u8 I2stx_usr_bits_mgmt; /**< user bits management */
	u8 I2stx_useOf_aux_smpl_bits; /**< use of auxiliary sample bits */
	u8 I2stx_src_word_length;	/**< source word length */
	u8 I2stx_indicate_align_level; 	/**< indication of alignment level */
	u8 I2stx_mc_ch_mode; /**< channel mode */
	u8 I2stx_byte3_mc0_ch_num; /**< Channel number 0 */
	u8 I2stx_byte3_mc1_ch_num;	/**< Channel number 1 */
	u8 I2stx_byte3_mc1_ch_mode_num; /**< multi channel1 mode number */
	u8 I2stx_digital_audio_ref_sig;	/**< digital audio reference signal */
	u8 I2stx_rsvd_but_undef0; /**< reserved but undefined */
	u8 I2stx_sampling_freq_q; /**< sampling frequency */
	u8 I2stx_sampling_freq_scale_flag; /**< sampling frequency
					     scaling flag */
	u8 I2stx_rsvd_but_undef1; /**< reserved but undefined */
	u32 I2stx_alphanum_ch_org_data; /**< Alphanumeric channel origin data */
	u32 I2stx_alphanum_ch_dest_data; /**< Alphanumeric channel destination
					   data*/
	u32 I2stx_local_sample_addrcode; /**< Local sample address code */
	u32 I2stx_timeOfDay_sample_addrcode;/**< Time-of-day sample
					      address code */
	u8 I2stx_reliable_flags;/**< Reliability flags */
	u8 I2stx_crc_char;/**< Cyclic redundancy check character */
	u8 Data;
	u32 Data32;

	Xil_AssertVoid(I2stx_SrcBuf != NULL);
	/* Read the value of Status register offset 0 from the SrcBuffer */
	Data = I2stx_SrcBuf[0];
	I2stx_useOf_ch_sts_blk =
		(Data & XI2S_TX_AES_STS_USE_OF_CH_STS_BLK_MASK);
	xil_printf("I2stx_useOf_ch_sts_blk =0x%x \r\n", I2stx_useOf_ch_sts_blk);

	I2stx_linear_pcm_id =
		((Data & XI2S_TX_AES_STS_LINEAR_PCM_ID_MASK) >>
		 XI2S_TX_AES_STS_LINEAR_PCM_ID_SHIFT);
	xil_printf("I2stx_linear_pcm_id =0x%x \r\n", I2stx_linear_pcm_id);

	I2stx_digital_audio_ref_sig =
		((Data & XI2S_TX_AES_STS_AUDIO_SIG_PRE_EMPH_MASK) >>
		 XI2S_TX_AES_STS_AUDIO_SIG_PRE_EMPH_SHIFT);
	xil_printf("I2stx_digital_audio_ref_sig =0x%x \r\n",
		       	I2stx_digital_audio_ref_sig);

	I2stx_lock_indication =
		((Data & XI2S_TX_AES_STS_LOCK_INDICATION_MASK) >>
		 XI2S_TX_AES_STS_LOCK_INDICATION_SHIFT);
	xil_printf("I2stx_lock_indication =0x%x \r\n",I2stx_lock_indication );

	I2stx_sampling_freq_e =
		((Data & XI2S_TX_AES_STS_SAMPLING_FREQ_E_MASK) >>
		 XI2S_TX_AES_STS_SAMPLING_FREQ_E_SHIFT);
	xil_printf("I2stx_sampling_freq_e =0x%x \r\n",I2stx_sampling_freq_e );

	/* Read the Status register offset 1 from the SrcBuffer */
	Data = I2stx_SrcBuf[1];
	I2stx_ch_mode = (Data & XI2S_TX_AES_STS_CH_MODE_MASK);
	xil_printf("I2stx_ch_mode =0x%x \r\n",I2stx_ch_mode );

	I2stx_usr_bits_mgmt =
		((Data & XI2S_TX_AES_STS_USR_BITS_MGMT_MASK) >>
		 XI2S_TX_AES_STS_USR_BITS_MGMT_SHIFT);
	xil_printf("I2stx_usr_bits_mgmt =0x%x \r\n", I2stx_usr_bits_mgmt);

	/* Read the Status register offset 2 from the SrcBuffer */
	Data = I2stx_SrcBuf[2];
	I2stx_useOf_aux_smpl_bits =
		(Data & XI2S_TX_AES_STS_USEOF_AUX_SMPL_BITS_MASK);
	xil_printf("I2stx_useOf_aux_smpl_bits =0x%x \r\n",
			I2stx_useOf_aux_smpl_bits );

	I2stx_src_word_length =
		((Data & XI2S_TX_AES_STS_SRC_WORD_LENGTH_MASK) >>
		 XI2S_TX_AES_STS_SRC_WORD_LENGTH_SHIFT);
	xil_printf("I2stx_src_word_length =0x%x \r\n", I2stx_src_word_length);

	I2stx_indicate_align_level =
		((Data & XI2S_TX_AES_STS_INDICATE_ALIGN_LEVEL_MASK) >>
		 XI2S_TX_AES_STS_INDICATE_ALIGN_LEVEL_SHIFT);
	xil_printf("I2stx_indicate_align_level =0x%x \r\n",
			I2stx_indicate_align_level );

	/* Read the Status register offset 3 from the SrcBuffer */
	Data = I2stx_SrcBuf[3];
	I2stx_mc_ch_mode =
		((Data & XI2S_TX_AES_STS_MC_CH_MODE_MASK) >>
		 XI2S_TX_AES_STS_MC_CH_MODE_SHIFT);
	if (I2stx_mc_ch_mode == 0) {
		I2stx_byte3_mc0_ch_num =
			(Data & XI2S_TX_AES_STS_CH_NUM0_MASK);
		xil_printf("I2stx_mc_ch_mode =0x%x \r\n",I2stx_mc_ch_mode );
		xil_printf("I2stx_byte3_mc0_ch_num =0x%x \r\n",
				I2stx_byte3_mc0_ch_num );

	} else if (I2stx_mc_ch_mode == 1) {
		I2stx_byte3_mc1_ch_num =
			(Data & XI2S_TX_AES_STS_CH_NUM1_MASK);
		I2stx_byte3_mc1_ch_mode_num =
			((Data & XI2S_TX_AES_STS_MC_CH_MODE_NUM_MASK) >>
			 XI2S_TX_AES_STS_MC_CH_MODE_NUM_SHIFT);
		xil_printf("I2stx_mc_ch_mode =0x%x \r\n",I2stx_mc_ch_mode );
		xil_printf("I2stx_byte3_mc1_ch_num=0x%x \r\n",
				I2stx_byte3_mc1_ch_num);
		xil_printf("I2stx_byte3_mc1_ch_mode_num=0x%x \r\n",
			       	I2stx_byte3_mc1_ch_mode_num);
	}

	/* Read the Status register offset 4 from the SrcBuffer */
	Data = I2stx_SrcBuf[4];
	I2stx_audio_sig_pre_emph =
		((Data & XI2S_TX_AES_STS_DIGITAL_AUDIO_REF_SIG_MASK) >>
		 XI2S_TX_AES_STS_DIGITAL_AUDIO_REF_SIG_SHIFT);
	xil_printf("I2stx_audio_sig_pre_emph =0x%x \r\n",
			I2stx_audio_sig_pre_emph );

	I2stx_rsvd_but_undef0 =
		((Data & XI2S_TX_AES_STS_RSVD_BUT_UNDEF0_MASK) >>
		 XI2S_TX_AES_STS_RSVD_BUT_UNDEF0_SHIFT);
	xil_printf("I2stx_rsvd_but_undef0 =0x%x \r\n",I2stx_rsvd_but_undef0 );

	I2stx_sampling_freq_q =
		((Data & XI2S_TX_AES_STS_SAMPLING_FREQ_Q_MASK) >>
		 XI2S_TX_AES_STS_SAMPLING_FREQ_Q_SHIFT);
	xil_printf("I2stx_sampling_freq_q =0x%x \r\n",I2stx_sampling_freq_q );

	I2stx_sampling_freq_scale_flag =
		((Data & XI2S_TX_AES_STS_SAMPLING_FREQ_SCALE_FLAG_MASK) >>
		 XI2S_TX_AES_STS_SAMPLING_FREQ_SCALE_FLAG_SHIFT);
	xil_printf("I2stx_sampling_freq_scale_flag =0x%x \r\n",
			I2stx_sampling_freq_scale_flag);

	/* Read the Status register offset 5 from the SrcBuffer */
	Data = I2stx_SrcBuf[5];
	I2stx_rsvd_but_undef1 =
		(Data & XI2S_TX_AES_STS_RSVD_BUT_UNDEF1_MASK);
	xil_printf("I2stx_rsvd_but_undef1 =0x%x \r\n",I2stx_rsvd_but_undef1 );

	/* Read the Status registers from offset 6 - 9 from the SrcBuffer */
	Data32 = (u32)((I2stx_SrcBuf[9] << 24) | (I2stx_SrcBuf[8] << 16) |
			(I2stx_SrcBuf[7] << 8) | I2stx_SrcBuf[6]);
	I2stx_alphanum_ch_org_data = Data32;
	xil_printf("I2stx_alphanum_ch_org_data =0x%x \r\n",
		       	I2stx_alphanum_ch_org_data);

	/* Read the Status registers from offset 10 - 13 from the SrcBuffer */
	Data32 = (u32)((I2stx_SrcBuf[13] << 24) | (I2stx_SrcBuf[12] << 16) |
			(I2stx_SrcBuf[11] << 8) | I2stx_SrcBuf[10]);
	I2stx_alphanum_ch_dest_data = Data32;
	xil_printf("I2stx_alphanum_ch_dest_data =0x%x \r\n",
			I2stx_alphanum_ch_dest_data);

	/* Read the Status registers from offset 14 - 17 from the SrcBuffer */
	Data32 = (u32)((I2stx_SrcBuf[17] << 24) | (I2stx_SrcBuf[16] << 16) |
			(I2stx_SrcBuf[15] << 8) | I2stx_SrcBuf[14]);
	I2stx_local_sample_addrcode = Data32;
	xil_printf("I2stx_local_sample_addrcode =0x%x \r\n",
			I2stx_local_sample_addrcode );

	/* Read the Status registers from offset 18 - 21 from the SrcBuffer */
	Data32 = (u32)((I2stx_SrcBuf[21] << 24) | (I2stx_SrcBuf[20] << 16) |
			(I2stx_SrcBuf[19] << 8) | I2stx_SrcBuf[18]);
	I2stx_timeOfDay_sample_addrcode = Data32;
	xil_printf("I2stx_timeOfDay_sample_addrcode =0x%x \r\n",
			I2stx_timeOfDay_sample_addrcode );

	/* Read the Status register offset 22 from the SrcBuffer */
	Data = I2stx_SrcBuf[XI2S_TX_AES_STS_RELIABLE_FLAGS_OFFSET];
	I2stx_reliable_flags = Data;
	xil_printf("I2stx_reliable_flags =0x%x \r\n",I2stx_reliable_flags);

	/* Read the Status register offset 23 from the SrcBuffer */
	Data = I2stx_SrcBuf[XI2S_TX_AES_STS_CRC_CHAR_OFFSET];
	I2stx_crc_char = Data;
	xil_printf("I2stx_crc_char =0x%x \r\n",I2stx_crc_char );
}
/** @} */
