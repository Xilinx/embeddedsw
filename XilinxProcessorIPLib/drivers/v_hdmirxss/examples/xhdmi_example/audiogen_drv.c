/*
 * Copyright (c) 2016 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

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
 * 1.00  RHe 2014/12/00   First release
 * 1.1   RHe 2015/07/30   Updated ACR GetNVal to be dependent of the
 *                        TMDS character rate instead of the video mode.
 * 1.2   MMO 2017/09/05   Replace U32 with UINTPTR for 64 Bit Addressing Support
 * </pre>
 *
 ******************************************************************************/

#include "audiogen_drv.h"

// Recommend N values for Audio Clock Regeneration
const ACR_N_Table_t ACR_N_Table[] =
{
	// TMDSClk    32k   44k1   48k   88k2    96k  176k4   192k
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
	{297000000, { 4096, 6272,  6144, 12544, 12288, 25088, 24576}},
	{371250000, { 6144, 4704,  5120,  9408, 10240, 18816, 20480}},
	{445500000, { 4096, 4704,  5120,  9408, 10240, 18816, 20480}},
	{594000000, { 3072, 4704,  5120,  9408, 10240, 18816, 20480}}
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

  // If TMDS character rate could not be found return default values
  item = &ACR_N_Table[0];

  return item->ACR_NVal[SRate];
}

// Helper function for reversing the bit order
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
  16384000, //512 x 32kHz
  22579200, //512 x 44.1kHz
  24576000, //512 x 48kHz
  45158400, //512 x 88.2kHz
  49152000, //512 x 96kHz
  90316800, //512 x 176.4kHz
  98304000  //512 x 192kHz
};

int XhdmiAudGen_Init (XhdmiAudioGen_t *AudioGen, UINTPTR AudGen_Base,
                                  UINTPTR ACRCtrl_Base, UINTPTR AudClk_Gen_Base)
{
  AudioGen->AudGenBase  = AudGen_Base;
  AudioGen->AudClkGenBase = AudClk_Gen_Base;
  AudioGen->ACRCtrlBase = ACRCtrl_Base;

  // Enable the audio clock
  XhdmiAudGen_SetAudClk(AudioGen, XAUD_SRATE_48K);

  // Start the Audio Generator
  XhdmiAudGen_Start(AudioGen, TRUE);

  // Enable ACR
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

int XhdmiAudGen_SetSampleRate (XhdmiAudioGen_t *AudioGen, u32 TMDSCharRate, AudioRate_t SampleRate)
{
  u32 data;
  u8  NumEnabCh = XhdmiAudGen_GetEnabChannels(AudioGen);

  // Disable all channels
  XhdmiAudGen_SetEnabChannels(AudioGen, 0);

  // Disable ACR generation
  XhdmiACRCtrl_Enab(AudioGen, 0);

  // Re-program audio clock
  XhdmiAudGen_SetAudClk(AudioGen, SampleRate);

  // Write the recommended N value
  data = XHdmi_ACR_GetNVal(TMDSCharRate, SampleRate);
  XhdmiACRCtrl_SetNVal(AudioGen, data);

  // Set Channel Status bits
  XhdmiAudGen_SetChSts(AudioGen, SampleRate);

  // Set the sample rate for the audio generator
  data = XAudGen_ReadReg(AudioGen->AudGenBase, AUD_CFG); // Read audio config reg
  data &= ~(AUD_CFG_REG_SAMPRATE_MASK << AUD_CFG_REG_SAMPRATE_SHIFT); // Clear SAMPRATE field
  data |= ((SampleRate & AUD_CFG_REG_SAMPRATE_MASK) << AUD_CFG_REG_SAMPRATE_SHIFT); // Write SAMPRATE field
  XAudGen_WriteReg(AudioGen->AudGenBase, AUD_CFG, data); // Write audio config reg

  // Reset the audio generator
  XhdmiAudGen_Reset(AudioGen);

  // Re-enable ACR generation
  XhdmiACRCtrl_Enab(AudioGen, 1);

  // Re-enable the channels
  XhdmiAudGen_SetEnabChannels(AudioGen, NumEnabCh);

  //MB_Sleep(10);

  XhdmiAudGen_UpdateConfig(AudioGen);

  return XST_SUCCESS;
}

int XhdmiAudGen_SetAudClk (XhdmiAudioGen_t *AudioGen, AudioRate_t SampleRate)
{
  // Assert the audio reset
  XhdmiACRCtrl_AudioReset(AudioGen, TRUE);

  XhdmiAudGen_SetAudClkParam(AudioGen, SampleRate);

  XhdmiAudGen_AudClkConfig(AudioGen);

  // De-assert the audio reset
  XhdmiACRCtrl_AudioReset(AudioGen, FALSE);

  return XST_SUCCESS;
}

int XhdmiAudGen_SetAudClkParam(XhdmiAudioGen_t *AudioGen, AudioRate_t SampleRate)
{
  // The settings below are for an input clock frequency of 100 MHz.
  switch(SampleRate){
    case (XAUD_SRATE_32K) :
      AudioGen->AudClkPLL.Div = 2;
      AudioGen->AudClkPLL.Mult = 19;
      AudioGen->AudClkPLL.Clk0Div = 58;
      break;

    case (XAUD_SRATE_44K1) :
      AudioGen->AudClkPLL.Div = 2;
      AudioGen->AudClkPLL.Mult = 19;
      AudioGen->AudClkPLL.Clk0Div = 42;
      break;

    case (XAUD_SRATE_48K) :
      AudioGen->AudClkPLL.Div = 1;
      AudioGen->AudClkPLL.Mult = 14;
      AudioGen->AudClkPLL.Clk0Div = 57;
      break;

    case (XAUD_SRATE_88K2) :
      AudioGen->AudClkPLL.Div = 1;
      AudioGen->AudClkPLL.Mult = 14;
      AudioGen->AudClkPLL.Clk0Div = 31;
      break;

    case (XAUD_SRATE_96K) :
      AudioGen->AudClkPLL.Div = 3;
      AudioGen->AudClkPLL.Mult = 19;
      AudioGen->AudClkPLL.Clk0Div = 13;
      break;

    case (XAUD_SRATE_176K4) :
      AudioGen->AudClkPLL.Div = 3;
      AudioGen->AudClkPLL.Mult = 19;
      AudioGen->AudClkPLL.Clk0Div = 7;
      break;

    case (XAUD_SRATE_192K) :
      AudioGen->AudClkPLL.Div = 1;
      AudioGen->AudClkPLL.Mult = 10;
      AudioGen->AudClkPLL.Clk0Div = 10;
      break;

    default :
      return XST_FAILURE;
  }

  return XST_SUCCESS;
}

int XhdmiAudGen_AudClkConfig(XhdmiAudioGen_t *AudioGen)
{
  u32 dat = 0;

  // Set the DIVCLK_DIVIDE and CLKFBOUT_MULT parameters
  dat = ((AudioGen->AudClkPLL.Div) & 0xFF);
  dat |= ((u32)(AudioGen->AudClkPLL.Mult & 0xFF) << 8);

  *(u32*)(AudioGen->AudClkGenBase + 0x200) = dat; // CLKCONFIG Reg 0

  dat = 0;

  // Set the CLKOUT0_DIVIDE parameter
  dat = ((AudioGen->AudClkPLL.Clk0Div) & 0xFF);

  *(u32*)(AudioGen->AudClkGenBase + 0x208) = dat; // CLKCONFIG Reg 2

  *(u32*)(AudioGen->AudClkGenBase + 0x25C) = 0x7; // Load the regs and start reconfiguration
  *(u32*)(AudioGen->AudClkGenBase + 0x25C) = 0x2; // De-assert LOAD and SEN

  // Wait for lock
  while(1)
  {
	usleep(1000);

    dat = *(u32*)(AudioGen->AudClkGenBase + 0x004);

    if(dat & 0x1)
      break;
  }
  return XST_SUCCESS;
}

int XhdmiAudGen_GetAudClk (AudioRate_t SampleRate)
{
  return AudClkFrq[SampleRate];
}

int XhdmiAudGen_SetPattern (XhdmiAudioGen_t *AudioGen, u8 ChannelID, AudioPattern_t Pattern)
{
  u32 data;

  if (ChannelID > 0 && ChannelID <= 8)
  {
    u32 ChannelRegOffset = ((ChannelID-1) * 0x10) + CH1_CTRL;

    data = XAudGen_ReadReg(AudioGen->AudGenBase, ChannelRegOffset); // Read channel reg
    data &= ~(CH_CTRL_REG_PATTERN_MASK << CH_CTRL_REG_PATTERN_SHIFT); // Clear PATTERN field
    data |= ((Pattern & CH_CTRL_REG_PATTERN_MASK) << CH_CTRL_REG_PATTERN_SHIFT); // Write PATTERN field
    XAudGen_WriteReg(AudioGen->AudGenBase, ChannelRegOffset, data); // Write channel reg

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
    data = XAudGen_ReadReg(AudioGen->AudGenBase, AUD_CFG); // Read audio config reg
    data &= ~(AUD_CFG_REG_NUMCH_MASK << AUD_CFG_REG_NUMCH_SHIFT); // Clear NUMCHANS field
    data |= ((NumChannels & AUD_CFG_REG_NUMCH_MASK) << AUD_CFG_REG_NUMCH_SHIFT); // Write NUMCHANS field
    XAudGen_WriteReg(AudioGen->AudGenBase, AUD_CFG, data); // Write audio config reg

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

  // Byte 0
  tmp = 0x00; // - Consumer use
  // - Normal audio
  // - Copy inhibited
  // - Pre-Emphasis: None
  // - Mode 0
  data |= tmp;

  // Byte 1
  tmp = 0x00; // Category code: general

  data |= ((u32)tmp << 8);

  // Byte 2
  tmp = 0x00; // - Source number: Do not take into account
  // - Channel number: Do not take into account

  data |= ((u32)tmp << 16);

  // Byte 3
  switch(SampleRate){
    case (XAUD_SRATE_32K)   : tmp = 0x3; break;
    case (XAUD_SRATE_44K1)  : tmp = 0x0; break;
    case (XAUD_SRATE_48K)   : tmp = 0x2; break;
    case (XAUD_SRATE_88K2)  : tmp = 0x8; break;
    case (XAUD_SRATE_96K)   : tmp = 0xA; break;
    case (XAUD_SRATE_176K4) : tmp = 0xC; break;
    case (XAUD_SRATE_192K)  : tmp = 0xE; break;
    default                 : tmp = 0x1; break; // Sampling frequency not indicated
  }
  // - Clock Accuracy: Level 2 +/- 1000ppm

  data |= ((u32)tmp << 24);

  // Write status bytes 0 to 3
  // Bits should be from LSB -> MSB
  XAudGen_WriteReg(AudioGen->AudGenBase, CHSTS_0, BitReverse(data));

  data = 0;

  // Byte 4
  tmp = 0x02; // Sample word length: 16 bits
  // Original sampling frequency not indicated

  data |= tmp;

  // Byte 5
  tmp = 0x00; // CGMS-A: Copying is permitted

  data |= ((u32)tmp << 8);

  // Write status bytes 4 to 5
  // Bits should be from LSB -> MSB
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

  if (sel == ACR_SEL_GEN) // Select the generated ACR values
    data |= (1 << ACR_CTRL_SEL_ACR_SHIFT);
  else // Select the input ACR values
    data &= ~(ACR_CTRL_SEL_ACR_MASK << ACR_CTRL_SEL_ACR_SHIFT);

  XAudGen_WriteReg(AudioGen->ACRCtrlBase, ACR_CTRL, data);

  return XST_SUCCESS;
}

int XhdmiACRCtrl_SetNVal (XhdmiAudioGen_t *AudioGen, u32 NVal)
{
  XAudGen_WriteReg(AudioGen->ACRCtrlBase, ACR_N, NVal);
  return XST_SUCCESS;
}
