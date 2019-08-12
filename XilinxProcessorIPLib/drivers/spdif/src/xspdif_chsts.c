/******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc. All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
 *****************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xspdif_chsts.c
 * @addtogroup Spdif_v1_0
 * @{
 *
 * This file implements the channel status format related functions.For
 * formats/line protocols check the AES Standard specifications document.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar    01/25/18 Initial release.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xspdif_chsts.h"
#include "xspdif.h"
#include "xspdif_hw.h"
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
 * This function reads the array i.e. Spdif_SrcBuffer which has the values of
 * all the six Spdif AES status registers, extracts the required bits and
 * displays them.
 * Note: Call XSpdif_Rx_GetChStat API before calling XSpdif_Decode_ChStat API
 *
 * @param  Spdif_SrcBuf is an array that contains the values of all the
 *         the AES Status registers.It contains 24 bytes i.e. 192 status bits.
 *
 *****************************************************************************/
void XSpdif_Decode_ChStat(u8 Spdif_SrcBuf[24])
{
	u8 Spdif_useOf_ch_sts_blk; /**< use of channel status block */
	u8 Spdif_linear_pcm_id; /**< linear PCM identification */
	u8 Spdif_audio_sig_pre_emph; /**< audio signal Pre-emphasis */
	u8 Spdif_lock_indication; /**< lock indication */
	u8 Spdif_sampling_freq_e; /**< sampling frequency */
	u8 Spdif_ch_mode; /**< channel mode */
	u8 Spdif_usr_bits_mgmt; /**< user bits management */
	u8 Spdif_useOf_aux_smpl_bits; /**< use of auxiliary sample bits */
	u8 Spdif_src_word_length;	/**< source word length */
	u8 Spdif_indicate_align_level; 	/**< indication of alignment level */
	u8 Spdif_mc_ch_mode; /**< channel mode */
	u8 Spdif_byte3_mc0_ch_num; /**< Channel number 0 */
	u8 Spdif_byte3_mc1_ch_num;	/**< Channel number 1 */
	u8 Spdif_byte3_mc1_ch_mode_num; /**< multi channel1 mode number */
	u8 Spdif_digital_audio_ref_sig;	/**< digital audio reference signal */
	u8 Spdif_rsvd_but_undef0; /**< reserved but undefined */
	u8 Spdif_sampling_freq_q; /**< sampling frequency */
	u8 Spdif_sampling_freq_scale_flag; /**< sampling frequency
					     scaling flag */
	u8 Spdif_rsvd_but_undef1; /**< reserved but undefined */
	u32 Spdif_alphanum_ch_org_data; /**< Alphanumeric channel origin data */
	u32 Spdif_alphanum_ch_dest_data; /**< Alphanumeric channel destination
					   data*/
	u32 Spdif_local_sample_addrcode; /**< Local sample address code */
	u32 Spdif_timeOfDay_sample_addrcode;/**< Time-of-day sample
					      address code */
	u8 Spdif_reliable_flags;/**< Reliability flags */
	u8 Spdif_crc_char;/**< Cyclic redundancy check character */
	u8 Data;
	u32 Data32;

	Xil_AssertVoid(Spdif_SrcBuf != NULL);
	/* Read the value of Status register offset 0 from the Spdif_SrcBuf */
	Data = Spdif_SrcBuf[0];
	Spdif_useOf_ch_sts_blk =
		(Data & XSPDIF_AES_STS_USE_OF_CH_STS_BLK_MASK);
	xil_printf("Spdif_useOf_ch_sts_blk =0x%x \r\n", Spdif_useOf_ch_sts_blk);


	Spdif_linear_pcm_id =
		((Data & XSPDIF_AES_STS_LINEAR_PCM_ID_MASK) >>
		 XSPDIF_AES_STS_LINEAR_PCM_ID_SHIFT);
	xil_printf("Spdif_linear_pcm_id =0x%x \r\n", Spdif_linear_pcm_id);

	Spdif_audio_sig_pre_emph =
		((Data & XSPDIF_AES_STS_AUDIO_SIG_PRE_EMPH_MASK) >>
		 XSPDIF_AES_STS_AUDIO_SIG_PRE_EMPH_SHIFT);
	xil_printf("Spdif_audio_sig_pre_emph =0x%x \r\n",
			Spdif_audio_sig_pre_emph);

	Spdif_lock_indication =
		((Data & XSPDIF_AES_STS_LOCK_INDICATION_MASK) >>
		 XSPDIF_AES_STS_LOCK_INDICATION_SHIFT);
	xil_printf("Spdif_lock_indication =0x%x \r\n",Spdif_lock_indication );

	Spdif_sampling_freq_e =
		((Data & XSPDIF_AES_STS_SAMPLING_FREQ_E_MASK) >>
		 XSPDIF_AES_STS_SAMPLING_FREQ_E_SHIFT);
	xil_printf("Spdif_sampling_freq_e =0x%x \r\n",Spdif_sampling_freq_e );

	/* Read the value of Status register offset 1 from the Spdif_SrcBuf */
	Data = Spdif_SrcBuf[1];
	Spdif_ch_mode = (Data & XSPDIF_AES_STS_CH_MODE_MASK);
	xil_printf("Spdif_ch_mode =0x%x \r\n",Spdif_ch_mode );

	Spdif_usr_bits_mgmt =
		((Data & XSPDIF_AES_STS_USR_BITS_MGMT_MASK) >>
		 XSPDIF_AES_STS_USR_BITS_MGMT_SHIFT);
	xil_printf("Spdif_usr_bits_mgmt =0x%x \r\n", Spdif_usr_bits_mgmt);

	/* Read the value of Status register offset 2 from the Spdif_SrcBuf */
	Data = Spdif_SrcBuf[2];
	Spdif_useOf_aux_smpl_bits =
		(Data & XSPDIF_AES_STS_USEOF_AUX_SMPL_BITS_MASK);
	xil_printf("Spdif_useOf_aux_smpl_bits =0x%x \r\n",
			Spdif_useOf_aux_smpl_bits );

	Spdif_src_word_length =
		((Data & XSPDIF_AES_STS_SRC_WORD_LENGTH_MASK) >>
		 XSPDIF_AES_STS_SRC_WORD_LENGTH_SHIFT);
	xil_printf("Spdif_src_word_length =0x%x \r\n", Spdif_src_word_length);

	Spdif_indicate_align_level =
		((Data & XSPDIF_AES_STS_INDICATE_ALIGN_LEVEL_MASK) >>
		 XSPDIF_AES_STS_INDICATE_ALIGN_LEVEL_SHIFT);
	xil_printf("Spdif_indicate_align_level =0x%x \r\n",
			Spdif_indicate_align_level );

	/* Read the value of Status register offset 3 from the Spdif_SrcBuf */
	Data = Spdif_SrcBuf[3];
	Spdif_mc_ch_mode =
		((Data & XSPDIF_AES_STS_MC_CH_MODE_MASK) >>
		 XSPDIF_AES_STS_MC_CH_MODE_SHIFT);
	if (Spdif_mc_ch_mode == 0) {
		Spdif_byte3_mc0_ch_num =
			(Data & XSPDIF_AES_STS_CH_NUM0_MASK);
		xil_printf("Spdif_mc_ch_mode =0x%x \r\n",Spdif_mc_ch_mode );
		xil_printf("Spdif_byte3_mc0_ch_num =0x%x \r\n",
				Spdif_byte3_mc0_ch_num );

	} else if (Spdif_mc_ch_mode == 1) {
		Spdif_byte3_mc1_ch_num =
			(Data & XSPDIF_AES_STS_CH_NUM1_MASK);
		Spdif_byte3_mc1_ch_mode_num =
			((Data & XSPDIF_AES_STS_MC_CH_MODE_NUM_MASK) >>
			 XSPDIF_AES_STS_MC_CH_MODE_NUM_SHIFT);
		xil_printf("Spdif_mc_ch_mode =0x%x \r\n",Spdif_mc_ch_mode );
		xil_printf("Spdif_byte3_mc1_ch_num=0x%x \r\n",
				Spdif_byte3_mc1_ch_num);
		xil_printf("Spdif_byte3_mc1_ch_mode_num=0x%x \r\n",
				Spdif_byte3_mc1_ch_mode_num);
	}

	/* Read the value of Status register offset 4 from the Spdif_SrcBuf */
	Data = Spdif_SrcBuf[4];
	Spdif_digital_audio_ref_sig =
		((Data & XSPDIF_AES_STS_DIGITAL_AUDIO_REF_SIG_MASK) >>
		 XSPDIF_AES_STS_DIGITAL_AUDIO_REF_SIG_SHIFT);
	xil_printf("Spdif_digital_audio_ref_sig =0x%x \r\n",
			Spdif_digital_audio_ref_sig );

	Spdif_rsvd_but_undef0 =
		((Data & XSPDIF_AES_STS_RSVD_BUT_UNDEF0_MASK) >>
		 XSPDIF_AES_STS_RSVD_BUT_UNDEF0_SHIFT);
	xil_printf("Spdif_rsvd_but_undef0 =0x%x \r\n",Spdif_rsvd_but_undef0 );

	Spdif_sampling_freq_q =
		((Data & XSPDIF_AES_STS_SAMPLING_FREQ_Q_MASK) >>
		 XSPDIF_AES_STS_SAMPLING_FREQ_Q_SHIFT);
	xil_printf("Spdif_sampling_freq_q =0x%x \r\n",Spdif_sampling_freq_q );

	Spdif_sampling_freq_scale_flag =
		((Data & XSPDIF_AES_STS_SAMPLING_FREQ_SCALE_FLAG_MASK) >>
		 XSPDIF_AES_STS_SAMPLING_FREQ_SCALE_FLAG_SHIFT);
	xil_printf("Spdif_sampling_freq_scale_flag =0x%x \r\n",
			Spdif_sampling_freq_scale_flag);

	/* Read the value of Status register offset 5 from the Spdif_SrcBuf */
	Data = Spdif_SrcBuf[5];
	Spdif_rsvd_but_undef1 =
		(Data & XSPDIF_AES_STS_RSVD_BUT_UNDEF1_MASK);
	xil_printf("Spdif_rsvd_but_undef1 =0x%x \r\n",Spdif_rsvd_but_undef1 );

	/* Read the values of Status registers from offsets 6 - 9 from the
	 * Spdif_SrcBuf */
	Data32 = (u32)((Spdif_SrcBuf[9] << 24) | (Spdif_SrcBuf[8] << 16) |
			(Spdif_SrcBuf[7] << 8) | Spdif_SrcBuf[6]);
	Spdif_alphanum_ch_org_data = Data32;
	xil_printf("Spdif_alphanum_ch_org_data =0x%x \r\n",
			Spdif_alphanum_ch_org_data);

	/* Read the values of Status registers from offsets 10 - 13 from the
	 * Spdif_SrcBuf */
	Data32 = (u32)((Spdif_SrcBuf[13] << 24) | (Spdif_SrcBuf[12] << 16) |
			(Spdif_SrcBuf[11] << 8) | Spdif_SrcBuf[10]);
	Spdif_alphanum_ch_dest_data = Data32;
	xil_printf("Spdif_alphanum_ch_dest_data =0x%x \r\n",
			Spdif_alphanum_ch_dest_data);

	/* Read the values of Status registers from offsets 14 - 17 from the
	 * Spdif_SrcBuf */
	Data32 = (u32)((Spdif_SrcBuf[17] << 24) | (Spdif_SrcBuf[16] << 16) |
			(Spdif_SrcBuf[15] << 8) | Spdif_SrcBuf[14]);
	Spdif_local_sample_addrcode = Data32;
	xil_printf("Spdif_local_sample_addrcode =0x%x \r\n",
			Spdif_local_sample_addrcode );

	/* Read the values of Status registers from the offsets 18 - 21 from
	 * the Spdif_SrcBuf */
	Data32 = (u32)((Spdif_SrcBuf[21] << 24) | (Spdif_SrcBuf[20] << 16) |
			(Spdif_SrcBuf[19] << 8) | Spdif_SrcBuf[18]);
	Spdif_timeOfDay_sample_addrcode = Data32;
	xil_printf("Spdif_timeOfDay_sample_addrcode =0x%x \r\n",
			Spdif_timeOfDay_sample_addrcode );

	/* Read the value of the Status register offset 22 from
	 * the Spdif_SrcBuf */
	Data = Spdif_SrcBuf[XSPDIF_AES_STS_RELIABLE_FLAGS_OFFSET];
	Spdif_reliable_flags = Data;
	xil_printf("Spdif_reliable_flags =0x%x \r\n",Spdif_reliable_flags);

	/* Read the value of Status register offset 23 from the Spdif_SrcBuf */
	Data = Spdif_SrcBuf[XSPDIF_AES_STS_CRC_CHAR_OFFSET];
	Spdif_crc_char = Data;
	xil_printf("Spdif_crc_char =0x%x \r\n",Spdif_crc_char );
}
/** @} */

