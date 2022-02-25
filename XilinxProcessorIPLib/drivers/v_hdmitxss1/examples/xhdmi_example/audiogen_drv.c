/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file audiogen_drv.c
 *
 * This file contains ...
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- --- ----------   -----------------------------------------------
 * X.XX  XX  YYYY/MM/DD   ...
 * 1.00  RHe 2019/05/14   Initial release
 * </pre>
 *
 ******************************************************************************/

#include "audiogen_drv.h"

#ifdef XPS_BOARD_VCK190
static u32 XhdmiAudGen_Mmcme5DividerEncoding(XhdmiAudioGen_MmcmDivType DivType,
		u16 Div);
static u32 XhdmiAudGen_Mmcme5CpResEncoding(u16 Mult);
static u32 XhdmiAudGen_Mmcme5LockReg1Reg2Encoding(u16 Mult);
#endif

typedef struct {
	AudioRate_t         SampleRate;
	XhdmiAudioGen_PLL_t	PLLSettings;
} XHDMI_SamplingFreq_PLLSettings;

/* MMCM PLL settings for sampling frequencies */
#ifndef XPS_BOARD_VCK190
const XHDMI_SamplingFreq_PLLSettings SampleRatePllSettingsTbl[] = {
    { XAUD_SRATE_32K,           { 2, 19,  0, 58,  0 }},
    { XAUD_SRATE_44K1,          { 2, 14,  0, 31,  0 }},
    { XAUD_SRATE_48K,           { 1, 14,  0, 57,  0 }},
    { XAUD_SRATE_88K2,          { 1, 14,  0, 31,  0 }},
    { XAUD_SRATE_96K,           { 3, 19,  0, 13,  0 }},
    { XAUD_SRATE_176K4,         { 3, 19,  0,  7,  0 }},
    { XAUD_SRATE_192K,          { 1, 10,  0, 10,  0 }},
	{ XAUD_NUM_SUPPORTED_SRATE, { 0,  0,  0,  0,  0 }},
};
#else
const XHDMI_SamplingFreq_PLLSettings SampleRatePllSettingsTbl[] = {
    { XAUD_SRATE_32K,           { 1, 38,  0, 232, 0 }},
    { XAUD_SRATE_44K1,          { 1, 28,  0, 124, 0 }},
    { XAUD_SRATE_48K,           { 1, 28,  0, 114, 0 }},
    { XAUD_SRATE_88K2,          { 1, 28,  0, 62,  0 }},
    { XAUD_SRATE_96K,           { 1, 28,  0, 57,  0 }},
    { XAUD_SRATE_176K4,         { 1, 28,  0, 31,  0 }},
    { XAUD_SRATE_192K,          { 1, 43,  0, 44,  0 }},
	{ XAUD_NUM_SUPPORTED_SRATE, { 0,  0,  0,  0,  0 }},
};
#endif
/* Original:
    { XAUD_SRATE_44K1,          { 2, 19,  0, 42,  0 }},
 */
/* Alternate PLL settings:
 *
 * KCU105:
    { XAUD_SRATE_32K,           { 1,  9,  5, 58,  6 }},
    { XAUD_SRATE_44K1,          { 2, 16,  3, 36,  2 }},
    { XAUD_SRATE_48K,           { 1, 11,  0, 44,  6 }},
    { XAUD_SRATE_88K2,          { 1, 14,  0, 31,  0 }},
    { XAUD_SRATE_96K,           { 1, 11,  0, 22,  3 }},
    { XAUD_SRATE_176K4,         { 1,  7,  0,  7,  6 }},
    { XAUD_SRATE_192K,          { 1,  7,  2,  7,  3 }},

 * KC705:
    { XAUD_SRATE_32K,           { 5, 43,  5, 53,  2 }},
    { XAUD_SRATE_44K1,          { 5, 62,  3, 55,  2 }},
    { XAUD_SRATE_48K,           { 5, 34,  2, 27,  7 }},
    { XAUD_SRATE_88K2,          { 5, 62,  3, 27,  5 }},
    { XAUD_SRATE_96K,           { 5, 43,  5, 17,  6 }},
    { XAUD_SRATE_176K4,         { 5, 53,  5, 11,  7 }},
    { XAUD_SRATE_192K,          { 1,  7,  2,  7,  3 }},
 */

/* Recommend N values for Audio Clock Regeneration */
const ACR_N_Table_t ACR_N_Table[] =
{
	/* TMDSClk    32k   44k1   48k   88k2    96k  176k4   192k */
	{        0, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 25200000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 27000000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 31500000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 33750000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 37800000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 40500000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 50400000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 54000000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 67500000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 74250000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 81000000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{ 92812500, { 8192, 6272, 12288, 12544, 24576, 25088, 49152}},
	{108000000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{111375000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{148500000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{185625000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{222750000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{297000000, { 3072, 4704,  5120,  9408, 10240, 18816, 20480}},
	{371250000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{445500000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{594000000, { 3072, 9408,  6144, 18816, 12288, 37632, 24576}}
};


const u32 XHdmi_ACR_GetNVal(u32 TMDSCharRate, AudioRate_t SRate)
{
  ACR_N_Table_t const *item;
  int i = 0;

  for(i = 0; i < sizeof(ACR_N_Table)/sizeof(ACR_N_Table_t); i++){
    item = &ACR_N_Table[i];
    if(item->TMDSCharRate == TMDSCharRate)
      return item->ACR_NVal[SRate];
  }

  /* If TMDS character rate could not be found return default values */
  item = &ACR_N_Table[0];

  return item->ACR_NVal[SRate];
}

/* Helper function for reversing the bit order */
u32 BitReverse(u32 x)
{
  x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
  x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
  x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
  x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
  return((x >> 16) | (x << 16));
}

static const u32 AudClkFrq[XAUD_NUM_SUPPORTED_SRATE] =
{
  16384000, /*512 x 32kHz */
  22579200, /*512 x 44.1kHz */
  24576000, /*512 x 48kHz */
  45158400, /*512 x 88.2kHz */
  49152000, /*512 x 96kHz */
  90316800, /*512 x 176.4kHz */
  98304000  /*512 x 192kHz */
};

int XhdmiAudGen_Init (XhdmiAudioGen_t *AudioGen, UINTPTR AudGen_Base,
                                UINTPTR ACRCtrl_Base, UINTPTR AudClk_Gen_Base)
{
  AudioGen->AudGenBase  = AudGen_Base;
  AudioGen->AudClkGenBase = AudClk_Gen_Base;
  AudioGen->ACRCtrlBase = ACRCtrl_Base;

  /* Enable the audio clock */
  XhdmiAudGen_SetAudClk(AudioGen, XAUD_SRATE_48K);

  /* Start the Audio Generator */
  XhdmiAudGen_Start(AudioGen, TRUE);

  /* Enable ACR */
  XhdmiACRCtrl_Enab(AudioGen, TRUE);

  return XST_SUCCESS;
}

int XhdmiAudGen_Reset(XhdmiAudioGen_t *AudioGen)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->AudGenBase, AUD_CTRL);

  data |= (1 << AUD_CTRL_REG_AUD_RST_SHIFT);
  XAudGen_WriteReg(AudioGen->AudGenBase, AUD_CTRL, data);

  return XST_SUCCESS;
}

int XhdmiAudGen_Start(XhdmiAudioGen_t *AudioGen, u8 setclr)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->AudGenBase, AUD_CTRL);

  if (setclr)
    data |= (1 << AUD_CTRL_REG_AUD_START_SHIFT);
  else
    data &= ~(AUD_CTRL_REG_AUD_START_MASK << AUD_CTRL_REG_AUD_START_SHIFT);

  XAudGen_WriteReg(AudioGen->AudGenBase, AUD_CTRL, data);

  return XST_SUCCESS;
}

int XhdmiAudGen_Drop(XhdmiAudioGen_t *AudioGen, u8 setclr)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->AudGenBase, AUD_CTRL);

  if (setclr)
    data |= (1 << AUD_CTRL_REG_AUD_DROP_SHIFT);
  else
    data &= ~(AUD_CTRL_REG_AUD_DROP_MASK << AUD_CTRL_REG_AUD_DROP_SHIFT);

  XAudGen_WriteReg(AudioGen->AudGenBase, AUD_CTRL, data);

  return XST_SUCCESS;
}

int XhdmiAudGen_UpdateConfig(XhdmiAudioGen_t *AudioGen)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->AudGenBase, AUD_CTRL);

  data |= (1 << AUD_CTRL_REG_AUD_UPDCFG_SHIFT);

  XAudGen_WriteReg(AudioGen->AudGenBase, AUD_CTRL, data);

  return XST_SUCCESS;
}

int XhdmiAudGen_SetSampleRate (XhdmiAudioGen_t *AudioGen,
                                    u32 TMDSCharRate, AudioRate_t SampleRate)
{
  int Result;
  u32 data;
  u8  NumEnabCh = XhdmiAudGen_GetEnabChannels(AudioGen);

  /* Disable all channels */
  XhdmiAudGen_SetEnabChannels(AudioGen, 0);

  /* Disable ACR generation */
  XhdmiACRCtrl_Enab(AudioGen, 0);

  /* Re-program audio clock */
  Result = XhdmiAudGen_SetAudClk(AudioGen, SampleRate);

  if (Result == XST_SUCCESS) {
    /* Write the recommended N value */
    data = XHdmi_ACR_GetNVal(TMDSCharRate, SampleRate);
    Result = XhdmiACRCtrl_SetNVal(AudioGen, data);
  }
  if (Result == XST_SUCCESS)
    /* Set Channel Status bits */
    Result = XhdmiAudGen_SetChSts(AudioGen, SampleRate);

  /* Set the sample rate for the audio generator */
  /* Read audio config reg */
  data = XAudGen_ReadReg(AudioGen->AudGenBase, AUD_CFG);
  /* Clear SAMPRATE field */
  data &= ~(AUD_CFG_REG_SAMPRATE_MASK << AUD_CFG_REG_SAMPRATE_SHIFT);
  /* Write SAMPRATE field */
  data |=
    ((SampleRate & AUD_CFG_REG_SAMPRATE_MASK) << AUD_CFG_REG_SAMPRATE_SHIFT);
  /* Write audio config reg */
  XAudGen_WriteReg(AudioGen->AudGenBase, AUD_CFG, data);

  if (Result == XST_SUCCESS)
    /* Reset the audio generator */
    Result = XhdmiAudGen_Reset(AudioGen);

  if (TMDSCharRate > 340000000) {
	  XhdmiACRCtrl_TMDSClkRatio(AudioGen, TRUE);
  } else {
	  XhdmiACRCtrl_TMDSClkRatio(AudioGen, FALSE);
  }

  if (Result == XST_SUCCESS)
  /* Re-enable ACR generation */
    Result = XhdmiACRCtrl_Enab(AudioGen, 1);

  if (Result == XST_SUCCESS)
  /* Re-enable the channels */
    Result = XhdmiAudGen_SetEnabChannels(AudioGen, NumEnabCh);

  if (Result == XST_SUCCESS)
    Result = XhdmiAudGen_UpdateConfig(AudioGen);

  return Result;
}

int XhdmiAudGen_SetAudClk (XhdmiAudioGen_t *AudioGen, AudioRate_t SampleRate)
{
  int Result;
  /* Assert the audio reset */
  XhdmiACRCtrl_AudioReset(AudioGen, TRUE);

  Result = XhdmiAudGen_SetAudClkParam(AudioGen, SampleRate);

  if (Result == XST_SUCCESS)
    Result = XhdmiAudGen_AudClkConfig(AudioGen);

  /* De-assert the audio reset */
  XhdmiACRCtrl_AudioReset(AudioGen, FALSE);

  return Result;
}

int XhdmiAudGen_SetAudClkParam(XhdmiAudioGen_t *AudioGen,
                                    AudioRate_t SampleRate)
{
  const XHDMI_SamplingFreq_PLLSettings* TblPtr = SampleRatePllSettingsTbl;

  while (TblPtr->SampleRate != XAUD_NUM_SUPPORTED_SRATE) {
    if (TblPtr->SampleRate == SampleRate) {
      memcpy(&AudioGen->AudClkPLL, &TblPtr->PLLSettings,
                   sizeof(AudioGen->AudClkPLL));
      return XST_SUCCESS;
    }
    TblPtr++;
  }
  return XST_FAILURE;
}

int XhdmiAudGen_AudClkConfig(XhdmiAudioGen_t *AudioGen)
{
  u32 dat = 0;
  u32 waitcount;
#ifdef XPS_BOARD_VCK190
  u32 regval;
  u32 regval2;
#else
  u32 fraction;
#endif

#ifndef XPS_BOARD_VCK190
  /* Set the DIVCLK_DIVIDE and CLKFBOUT_MULT parameters */
  fraction = AudioGen->AudClkPLL.Mult_Eights * 125;
  dat = ((AudioGen->AudClkPLL.Div) & 0xFF);
  dat |= ((u32)(AudioGen->AudClkPLL.Mult & 0xFF) << 8);
  dat |= (fraction &0xFFFF) << 16;
  *(u32*)(AudioGen->AudClkGenBase + 0x200) = dat; /* CLKCONFIG Reg 0 */

  dat = 0;

  /* Set the CLKOUT0_DIVIDE parameter */
  fraction = AudioGen->AudClkPLL.Clk0Div_Eights * 125;
  dat = ((AudioGen->AudClkPLL.Clk0Div) & 0xFF);
  dat |= (fraction &0xFFFF) << 8;
  *(u32*)(AudioGen->AudClkGenBase + 0x208) = dat; /* CLKCONFIG Reg 2 */

  /* Load the regs and start reconfiguration */
  *(u32*)(AudioGen->AudClkGenBase + 0x25C) = 0x7;
  /* De-assert LOAD and SEN */
  *(u32*)(AudioGen->AudClkGenBase + 0x25C) = 0x2;

  /* Wait for lock */
  waitcount = 0;
  while(waitcount < (AUDGEN_WAIT_CNT)) {
	usleep(100);
    dat = *(volatile u32*)(AudioGen->AudClkGenBase + 0x004);
    if(dat & 0x1) {
      return XST_SUCCESS;
    }
    waitcount++;
  }
  return XST_FAILURE;

#else
  /* Write CLKFBOUT_1 & CLKFBOUT_2 Values */
  regval = XhdmiAudGen_Mmcme5DividerEncoding(AUDGEN_MMCM_CLKFBOUT_MULT_F,
			  AudioGen->AudClkPLL.Mult);
  *(u32*)(AudioGen->AudClkGenBase + 0x330) = (u16)(regval & 0xFFFF);
  *(u32*)(AudioGen->AudClkGenBase + 0x334) = (u16)((regval >> 16) & 0xFFFF);

  /* Write DIVCLK_DIVIDE & DESKEW_2 Values */
  regval = XhdmiAudGen_Mmcme5DividerEncoding(AUDGEN_MMCM_DIVCLK_DIVIDE,
				AudioGen->AudClkPLL.Div) ;
  *(u32*)(AudioGen->AudClkGenBase + 0x384) = (u16)((regval >> 16) & 0xFFFF);
  *(u32*)(AudioGen->AudClkGenBase + 0x380) =
			  ((AudioGen->AudClkPLL.Div == 0) ? 0x0000 :
					  ((AudioGen->AudClkPLL.Div % 2) ? 0x0400 : 0x0000));

  /* Write CLKOUT0_1 & CLKOUT0_2 Values */
  regval = XhdmiAudGen_Mmcme5DividerEncoding(AUDGEN_MMCM_CLKOUT_DIVIDE,
					AudioGen->AudClkPLL.Clk0Div);
  *(u32*)(AudioGen->AudClkGenBase + 0x338) = (u16)(regval & 0xFFFF);
  *(u32*)(AudioGen->AudClkGenBase + 0x33C) = (u16)((regval >> 16) & 0xFFFF);

  /* Write CP & RES Values */
  regval = XhdmiAudGen_Mmcme5CpResEncoding(AudioGen->AudClkPLL.Mult);
  /* CP */
  regval2 = *(u32*)(AudioGen->AudClkGenBase + 0x378);
  regval2 &= ~(0xF);
  *(u32*)(AudioGen->AudClkGenBase + 0x378) = (u16)((regval & 0xF) | regval2);

  /* RES */
  regval2 = *(u32*)(AudioGen->AudClkGenBase + 0x3A8);
  regval2 &= ~(0x1E);
  *(u32*)(AudioGen->AudClkGenBase + 0x3A8) =
					  (u16)(((regval >> 15) & 0x1E) | regval2);

  /* Write Lock Reg1 & Reg2 Values */
  regval = XhdmiAudGen_Mmcme5LockReg1Reg2Encoding(AudioGen->AudClkPLL.Mult);
  /* LOCK_1 */
  regval2 = *(u32*)(AudioGen->AudClkGenBase + 0x39C);
  regval2 &= ~(0x8000);
  *(u32*)(AudioGen->AudClkGenBase + 0x39C) =
						(u16)((regval & 0x7FFF) | regval2);

  /* LOCK_2 */
  regval2 = *(u32*)(AudioGen->AudClkGenBase + 0x3A0);
  regval2 &= ~(0x8000);
  *(u32*)(AudioGen->AudClkGenBase + 0x3A0) =
						(u16)(((regval >> 16) & 0x7FFF) | regval2);

  *(u32*)(AudioGen->AudClkGenBase + 0x3F0) = 0x0000;
  *(u32*)(AudioGen->AudClkGenBase + 0x3FC) = 0x0001;

  /* Load the regs and start reconfiguration */
  *(u32*)(AudioGen->AudClkGenBase + 0x014) = 0x3;

  waitcount = 0;
  while(waitcount < (AUDGEN_WAIT_CNT)) {
	usleep(100);
    dat = *(volatile u32*)(AudioGen->AudClkGenBase + 0x004);
    if(dat & 0x1) {
      return XST_SUCCESS;
    }
    waitcount++;
  }
  return XST_FAILURE;
#endif

}

int XhdmiAudGen_GetAudClk (AudioRate_t SampleRate)
{
  return AudClkFrq[SampleRate];
}

int XhdmiAudGen_SetPattern (XhdmiAudioGen_t *AudioGen,
                                u8 ChannelID, AudioPattern_t Pattern)
{
  u32 data;

  if (ChannelID > 0 && ChannelID <= 8)
  {
    u32 ChannelRegOffset = ((ChannelID-1) * 0x10) + CH1_CTRL;

    /* Read channel reg */
    data = XAudGen_ReadReg(AudioGen->AudGenBase, ChannelRegOffset);
    /* Clear PATTERN field */
    data &= ~(CH_CTRL_REG_PATTERN_MASK << CH_CTRL_REG_PATTERN_SHIFT);
    /* Write PATTERN field */
    data |=
        ((Pattern & CH_CTRL_REG_PATTERN_MASK) << CH_CTRL_REG_PATTERN_SHIFT);
    /* Write channel reg */
    XAudGen_WriteReg(AudioGen->AudGenBase, ChannelRegOffset, data);

    return XST_SUCCESS;
  }
  else
    return XST_FAILURE;
}

int XhdmiAudGen_SetEnabChannels (XhdmiAudioGen_t *AudioGen, u8 NumChannels)
{
  u32 data;

  if (NumChannels <= 8)
  {
    /* Read audio config reg */
    data = XAudGen_ReadReg(AudioGen->AudGenBase, AUD_CFG);
    /* Clear NUMCHANS field */
    data &= ~(AUD_CFG_REG_NUMCH_MASK << AUD_CFG_REG_NUMCH_SHIFT);
    /* Write NUMCHANS field */
    data |=
        ((NumChannels & AUD_CFG_REG_NUMCH_MASK) << AUD_CFG_REG_NUMCH_SHIFT);
    /* Write audio config reg */
    XAudGen_WriteReg(AudioGen->AudGenBase, AUD_CFG, data);

    return XST_SUCCESS;
  }
  else
    return XST_FAILURE;
}

int XhdmiAudGen_GetEnabChannels (XhdmiAudioGen_t *AudioGen)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->AudGenBase, AUD_CFG);
  return (data >> AUD_CFG_REG_NUMCH_SHIFT) & AUD_CFG_REG_NUMCH_MASK;
}

int XhdmiAudGen_SetChSts(XhdmiAudioGen_t *AudioGen, AudioRate_t SampleRate)
{
  u32 data = 0;
  u8  tmp = 0;

  /* Byte 0 */
  tmp = 0x00;
  /* - Consumer use
   * - Normal audio
   * - Copy inhibited
   * - Pre-Emphasis: None
   * - Mode 0
   */
  data |= tmp;

  /* Byte 1 */
  tmp = 0x00; /* Category code: general */

  data |= ((u32)tmp << 8);

  /* Byte 2 */
  tmp = 0x00; /* - Source number: Do not take into account */
  /* - Channel number: Do not take into account */

  data |= ((u32)tmp << 16);

  /* Byte 3 */
  switch(SampleRate){
    case (XAUD_SRATE_32K)   : tmp = 0x3; break;
    case (XAUD_SRATE_44K1)  : tmp = 0x0; break;
    case (XAUD_SRATE_48K)   : tmp = 0x2; break;
    case (XAUD_SRATE_88K2)  : tmp = 0x8; break;
    case (XAUD_SRATE_96K)   : tmp = 0xA; break;
    case (XAUD_SRATE_176K4) : tmp = 0xC; break;
    case (XAUD_SRATE_192K)  : tmp = 0xE; break;
    /* Sampling frequency not indicated */
    default                 : tmp = 0x1; break;
  }
  /* - Clock Accuracy: Level 2 +/- 1000ppm */

  data |= ((u32)tmp << 24);

  /* Write status bytes 0 to 3 */
  /* Bits should be from LSB -> MSB */
  XAudGen_WriteReg(AudioGen->AudGenBase, CHSTS_0, BitReverse(data));

  data = 0;

  /* Byte 4 */
  tmp = 0x02; /* Sample word length: 16 bits */
  /* Original sampling frequency not indicated */

  data |= tmp;

  /* Byte 5 */
  tmp = 0x00; /* CGMS-A: Copying is permitted */

  data |= ((u32)tmp << 8);

  /* Write status bytes 4 to 5 */
  /* Bits should be from LSB -> MSB */
  XAudGen_WriteReg(AudioGen-> AudGenBase, CHSTS_1, BitReverse(data));

  return XST_SUCCESS;
}

int XhdmiACRCtrl_AudioReset (XhdmiAudioGen_t *AudioGen, u8 setclr)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->ACRCtrlBase, ACR_CTRL);
  if (setclr)
    data |= (1 << ACR_CTRL_AUD_RST_SHIFT);
  else
    data &= ~(ACR_CTRL_AUD_RST_MASK << ACR_CTRL_AUD_RST_SHIFT);

  XAudGen_WriteReg(AudioGen->ACRCtrlBase, ACR_CTRL, data);

  return XST_SUCCESS;
}

int XhdmiACRCtrl_Enab (XhdmiAudioGen_t *AudioGen, u8 setclr)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->ACRCtrlBase, ACR_CTRL);

  if (setclr)
    data |= (1 << ACR_CTRL_ENAB_ACR_SHIFT);
  else
    data &= ~(ACR_CTRL_ENAB_ACR_MASK << ACR_CTRL_ENAB_ACR_SHIFT);

  XAudGen_WriteReg(AudioGen->ACRCtrlBase, ACR_CTRL, data);

  return XST_SUCCESS;
}

int XhdmiACRCtrl_Sel (XhdmiAudioGen_t *AudioGen, u8 sel)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->ACRCtrlBase, ACR_CTRL);

  if (sel == ACR_SEL_GEN) /* Select the generated ACR values */
    data |= (1 << ACR_CTRL_SEL_ACR_SHIFT);
  else /* Select the input ACR values */
    data &= ~(ACR_CTRL_SEL_ACR_MASK << ACR_CTRL_SEL_ACR_SHIFT);

  XAudGen_WriteReg(AudioGen->ACRCtrlBase, ACR_CTRL, data);

  return XST_SUCCESS;
}

int XhdmiACRCtrl_TMDSClkRatio (XhdmiAudioGen_t *AudioGen, u8 setclr)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->ACRCtrlBase, ACR_CTRL);
  if (setclr)
    data |= (1 << ACR_CTRL_TMDSCLKRATIO_SHIFT);
  else
    data &= ~(ACR_CTRL_TMDSCLKRATIO_MASK << ACR_CTRL_TMDSCLKRATIO_SHIFT);

  XAudGen_WriteReg(AudioGen->ACRCtrlBase, ACR_CTRL, data);

  return XST_SUCCESS;
}

int XhdmiACRCtrl_SetNVal (XhdmiAudioGen_t *AudioGen, u32 NVal)
{
  XAudGen_WriteReg(AudioGen->ACRCtrlBase, ACR_N_OUT, NVal);
  return XST_SUCCESS;
}

int XhdmiACRCtrl_EnableCtsConv (XhdmiAudioGen_t *AudioGen, u8 setclr)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->ACRCtrlBase, ACR_CTRL);
  if (setclr)
    data |= (1 << ACR_CTRL_ENAB_ACR_SHIFT);
  else
    data &= ~(ACR_CTRL_ENAB_ACR_MASK << ACR_CTRL_ENAB_ACR_SHIFT);

  XAudGen_WriteReg(AudioGen->ACRCtrlBase, ACR_CTRL, data);

  return XST_SUCCESS;
}

int XhdmiACRCtrl_RxMode (XhdmiAudioGen_t *AudioGen, u8 setclr)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->ACRCtrlBase, ACR_CTRL);
  if (setclr)
    data |= (1 << ACR_CTRL_RX_MODE_SHIFT);
  else
    data &= ~(ACR_CTRL_RX_MODE_MASK << ACR_CTRL_RX_MODE_SHIFT);

  XAudGen_WriteReg(AudioGen->ACRCtrlBase, ACR_CTRL, data);

  return XST_SUCCESS;
}

int XhdmiACRCtrl_TxMode (XhdmiAudioGen_t *AudioGen, u8 setclr)
{
  u32 data;

  data = XAudGen_ReadReg(AudioGen->ACRCtrlBase, ACR_CTRL);
  if (setclr)
    data |= (1 << ACR_CTRL_TX_MODE_SHIFT);
  else
    data &= ~(ACR_CTRL_TX_MODE_MASK << ACR_CTRL_TX_MODE_SHIFT);

  XAudGen_WriteReg(AudioGen->ACRCtrlBase, ACR_CTRL, data);

  return XST_SUCCESS;
}

#ifdef XPS_BOARD_VCK190
/*****************************************************************************/
/**
* This function returns the DRP encoding of ClkFbOutMult optimized for:
* Phase = 0; Dutycycle = 0.5; No Fractional division
* The calculations are based on XAPP888
*
* @param	Div is the divider to be encoded
*
* @return
*		- Encoded Value for ClkReg1 [15: 0]
*       - Encoded Value for ClkReg2 [31:16]
*
* @note		None.
*
******************************************************************************/
u32 XhdmiAudGen_Mmcme5DividerEncoding(XhdmiAudioGen_MmcmDivType DivType,
		u16 Div)
{
	u32 DrpEnc;
	u32 ClkReg1;
    u32 ClkReg2;
    u8 HiTime, LoTime;
    u16 Divide = Div;

    if (DivType == AUDGEN_MMCM_CLKOUT_DIVIDE) {
	/* Div is an odd number */
		if (Div % 2) {
		Divide = (Div / 2);
		}
		/* Div is an even number */
		else {
		Divide = (Div / 2) + (Div % 2);
		}
    }

	HiTime = Divide / 2;
	LoTime = HiTime;

	ClkReg2 = LoTime & 0xFF;
	ClkReg2 |= (HiTime & 0xFF) << 8;

	if (DivType == AUDGEN_MMCM_CLKFBOUT_MULT_F) {
		ClkReg1 = (Divide % 2) ? 0x00001700 : 0x00001600;
	}
	else {
		/* Div is an odd number */
		if (Div % 2) {
			ClkReg1 = (Divide % 2) ? 0x0000BB00 : 0x0000BA00;
		}
		/* Div is an even number */
		else {
			ClkReg1 = (Divide % 2) ? 0x00001B00 : 0x00001A00;
		}
	}

    DrpEnc = (ClkReg2 << 16) | ClkReg1;

	return DrpEnc;
}

/*****************************************************************************/
/**
* This function returns the DRP encoding of CP and Res optimized for:
* Phase = 0; Dutycycle = 0.5; BW = low; No Fractional division
*
* @param	Mult is the divider to be encoded
*
* @return
*		- [3:0]   CP
*		- [20:17] RES
*
* @note		None.
*
******************************************************************************/
u32 XhdmiAudGen_Mmcme5CpResEncoding(u16 Mult)
{
	u32 DrpEnc;
	u16 cp;
	u16 res;

    switch (Mult) {
    case 4:
         cp = 5; res = 15;
         break;
    case 5:
         cp = 6; res = 15;
         break;
    case 6:
         cp = 7; res = 15;
         break;
    case 7:
         cp = 13; res = 15;
         break;
    case 8:
         cp = 14; res = 15;
         break;
    case 9:
         cp = 15; res = 15;
         break;
    case 10:
         cp = 14; res = 7;
         break;
    case 11:
         cp = 15; res = 7;
         break;
    case 12 ... 13:
         cp = 15; res = 11;
         break;
    case 14:
         cp = 15; res = 13;
         break;
    case 15:
         cp = 15; res = 3;
         break;
    case 16 ... 17:
         cp = 14; res = 5;
         break;
    case 18 ... 19:
         cp = 15; res = 5;
         break;
    case 20 ... 21:
         cp = 15; res = 9;
         break;
    case 22 ... 23:
         cp = 14; res = 14;
         break;
    case 24 ... 26:
         cp = 15; res = 14;
         break;
    case 27 ... 28:
         cp = 14; res = 1;
         break;
    case 29 ... 33:
         cp = 15; res = 1;
         break;
    case 34 ... 37:
         cp = 14; res = 6;
         break;
    case 38 ... 44:
         cp = 15; res = 6;
         break;
    case 45 ... 57:
         cp = 15; res = 10;
         break;
    case 58 ... 63:
         cp = 13; res = 12;
         break;
    case 64 ... 70:
         cp = 14; res = 12;
         break;
    case 71 ... 86:
         cp = 15; res = 12;
         break;
    case 87 ... 93:
         cp = 14; res = 2;
         break;
    case 94:
         cp = 5; res = 15;
         break;
    case 95:
         cp = 6; res = 15;
         break;
    case 96:
         cp = 7; res = 15;
         break;
    case 97:
         cp = 13; res = 15;
         break;
    case 98:
         cp = 14; res = 15;
         break;
    case 99:
         cp = 15; res = 15;
         break;
    case 100:
         cp = 14; res = 7;
         break;
    case 101:
         cp = 15; res = 7;
         break;
    case 102 ... 103:
         cp = 15; res = 11;
         break;
    case 104:
         cp = 15; res = 13;
         break;
    case 105:
         cp = 15; res = 3;
         break;
    case 106 ... 107:
         cp = 14; res = 5;
         break;
    case 108 ... 109:
         cp = 15; res = 5;
         break;
    case 110 ... 111:
         cp = 15; res = 9;
         break;
    case 112 ... 113:
         cp = 14; res = 14;
         break;
    case 114 ... 116:
         cp = 15; res = 14;
         break;
    case 117 ... 118:
         cp = 14; res = 1;
         break;
    case 119 ... 123:
         cp = 15; res = 1;
         break;
    case 124 ... 127:
         cp = 14; res = 6;
         break;
    case 128 ... 134:
         cp = 15; res = 6;
         break;
    case 135 ... 147:
         cp = 15; res = 10;
         break;
    case 148 ... 153:
         cp = 13; res = 12;
         break;
    case 154 ... 160:
         cp = 14; res = 12;
         break;
    case 161 ... 176:
         cp = 15; res = 12;
         break;
    case 177 ... 183:
         cp = 14; res = 2;
         break;
    case 184 ... 200:
         cp = 14; res = 4;
         break;
    case 201 ... 273:
         cp = 15; res = 4;
         break;
    case 274 ... 300:
         cp = 13; res = 8;
         break;
    case 301 ... 325:
         cp = 14; res = 8;
         break;
    case 326 ... 432:
         cp = 15; res = 8;
         break;
	 default:
         cp = 13; res = 8;
	     break;
	}

    /* Construct the return value */
    DrpEnc = ((res & 0xf) << 17) | ((cp & 0xf) | 0x160);

	return DrpEnc;
}

/*****************************************************************************/
/**
* This function returns the DRP encoding of Lock Reg1 & Reg2 optimized for:
* Phase = 0; Dutycycle = 0.5; BW = low; No Fractional division
*
* @param	Mult is the divider to be encoded
*
* @return
*		- [15:0]  Lock_1 Reg
*		- [31:16] Lock_2 Reg
*
* @note		None.
*
******************************************************************************/
u32 XhdmiAudGen_Mmcme5LockReg1Reg2Encoding(u16 Mult)
{
	u32 DrpEnc;
	u16 Lock_1;
	u16 Lock_2;
	u16 lock_ref_dly;
	u16 lock_fb_dly;
	u16 lock_cnt;
	u16 lock_sat_high = 9;

	switch (Mult) {
		case 4:
			lock_ref_dly = 4;
			lock_fb_dly = 4;
			lock_cnt = 1000;
			break;
		case 5:
			lock_ref_dly = 6;
			lock_fb_dly = 6;
			lock_cnt = 1000;
			break;
		case 6 ... 7:
			lock_ref_dly = 7;
			lock_fb_dly = 7;
			lock_cnt = 1000;
			break;
		case 8:
			lock_ref_dly = 9;
			lock_fb_dly = 9;
			lock_cnt = 1000;
			break;
		case 9 ... 10:
			lock_ref_dly = 10;
			lock_fb_dly = 10;
			lock_cnt = 1000;
			break;
		case 11:
			lock_ref_dly = 11;
			lock_fb_dly = 11;
			lock_cnt = 1000;
			break;
		case 12:
			lock_ref_dly = 13;
			lock_fb_dly = 13;
			lock_cnt = 1000;
			break;
		case 13 ... 14:
			lock_ref_dly = 14;
			lock_fb_dly = 14;
			lock_cnt = 1000;
			break;
		case 15:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 900;
			break;
		case 16 ... 17:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 825;
			break;
		case 18:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 750;
			break;
		case 19 ... 20:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 700;
			break;
		case 21:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 650;
			break;
		case 22 ... 23:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 625;
			break;
		case 24:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 575;
			break;
		case 25:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 550;
			break;
		case 26 ... 28:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 525;
			break;
		case 29 ... 30:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 475;
			break;
		case 31:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 450;
			break;
		case 32 ... 33:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 425;
			break;
		case 34 ... 36:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 400;
			break;
		case 37:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 375;
			break;
		case 38 ... 40:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 350;
			break;
		case 41 ... 43:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 325;
			break;
		case 44 ... 47:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 300;
			break;
		case 48 ... 51:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 275;
			break;
		case 52 ... 205:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 950;
			break;
		case 206 ... 432:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 925;
			break;
		default:
			lock_ref_dly = 16;
			lock_fb_dly = 16;
			lock_cnt = 250;
			break;
	}

	/* Construct Lock_1 Reg */
	Lock_1 = ((lock_fb_dly & 0x1F) << 10) | (lock_cnt & 0x3FF);

	/* Construct Lock_2 Reg */
	Lock_2 = ((lock_ref_dly & 0x1F) << 10) | (lock_sat_high & 0x3FF);

	/* Construct Return Value */
	DrpEnc = (Lock_2 << 16) | Lock_1;

	return DrpEnc;
}
#endif
