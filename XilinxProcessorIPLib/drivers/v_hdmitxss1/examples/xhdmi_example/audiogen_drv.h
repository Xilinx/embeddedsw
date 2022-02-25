/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file audiogen_drv.h
 *
 * This file contains ..
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

#ifndef AUDIOGEN_DRV_H_
#define AUDIOGEN_DRV_H_

#include "xvidc.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"
#include "sleep.h"

/* AUDGEN_WAIT_CNT will be assigned a dummy value when there is no TX Instance
 * as AUDGEN only exists for exdes which contains TX instance */
#if defined (XPAR_XV_HDMITXSS1_NUM_INSTANCES)
#define AUDGEN_WAIT_CNT (XPAR_XV_HDMITXSS1_0_AXI_LITE_FREQ_HZ / 500)
#else
#define AUDGEN_WAIT_CNT 1
#endif

typedef struct {
  u32 TMDSCharRate;
  u32 ACR_NVal[7];
} ACR_N_Table_t;

typedef enum {
  XAUD_SRATE_32K = 0,
  XAUD_SRATE_44K1,
  XAUD_SRATE_48K,
  XAUD_SRATE_88K2,
  XAUD_SRATE_96K,
  XAUD_SRATE_176K4,
  XAUD_SRATE_192K,
  XAUD_NUM_SUPPORTED_SRATE
} AudioRate_t;

typedef enum {
  XAUD_PAT_MUTE = 0,
  XAUD_PAT_SINE = 1,
  XAUD_PAT_PING = 2,
  XAUD_PAT_RAMP = 3,
  XAUD_NUM_SUPPORTED_PAT
} AudioPattern_t;

typedef struct {
  u8 Div;
  u8 Mult;
  u8 Mult_Eights;
  u8 Clk0Div;
  u8 Clk0Div_Eights;

} XhdmiAudioGen_PLL_t;

typedef struct {
  UINTPTR AudGenBase;
  UINTPTR ACRCtrlBase;
  UINTPTR AudClkGenBase;
  XhdmiAudioGen_PLL_t AudClkPLL;
} XhdmiAudioGen_t;

/* This typedef enumerates the different MMCM Dividers */
typedef enum {
  AUDGEN_MMCM_CLKFBOUT_MULT_F, /* M */
  AUDGEN_MMCM_DIVCLK_DIVIDE,   /* D */
  AUDGEN_MMCM_CLKOUT_DIVIDE    /* On */
} XhdmiAudioGen_MmcmDivType;

int XhdmiAudGen_Init(XhdmiAudioGen_t *AudioGen, UINTPTR AudGen_Base,
                          UINTPTR ACRCtrl_Base, UINTPTR AudClk_Gen_Base);
int XhdmiAudGen_Reset(XhdmiAudioGen_t *AudioGen);
int XhdmiAudGen_Start(XhdmiAudioGen_t *AudioGen, u8 setclr);
int XhdmiAudGen_Drop(XhdmiAudioGen_t *AudioGen, u8 setclr);
int XhdmiAudGen_SetSampleRate(XhdmiAudioGen_t *AudioGen,
                                   u32 TMDSCharRate, AudioRate_t SampleRate);
int XhdmiAudGen_SetPattern(XhdmiAudioGen_t *AudioGen,
                                u8 ChannelID, AudioPattern_t Pattern);
int XhdmiAudGen_SetEnabChannels(XhdmiAudioGen_t *AudioGen, u8 NumChannels);
int XhdmiAudGen_GetEnabChannels(XhdmiAudioGen_t *AudioGen);
int XhdmiAudGen_SetChSts(XhdmiAudioGen_t *AudioGen, AudioRate_t SampleRate);
int XhdmiAudGen_UpdateConfig(XhdmiAudioGen_t *AudioGen);

int XhdmiAudGen_SetAudClk(XhdmiAudioGen_t *AudioGen, AudioRate_t SampleRate);
int XhdmiAudGen_SetAudClkParam(XhdmiAudioGen_t *AudioGen,
                                    AudioRate_t SampleRate);
int XhdmiAudGen_AudClkConfig(XhdmiAudioGen_t *AudioGen);
int XhdmiAudGen_GetAudClk(AudioRate_t SampleRate);

int XhdmiACRCtrl_AudioReset(XhdmiAudioGen_t *AudioGen, u8 setclr);
int XhdmiACRCtrl_Enab(XhdmiAudioGen_t *AudioGen, u8 setclr);
int XhdmiACRCtrl_Sel(XhdmiAudioGen_t *AudioGen, u8 sel);
int XhdmiACRCtrl_TMDSClkRatio (XhdmiAudioGen_t *AudioGen, u8 setclr);
int XhdmiACRCtrl_SetNVal(XhdmiAudioGen_t *AudioGen, u32 NVal);
int XhdmiACRCtrl_EnableCtsConv (XhdmiAudioGen_t *AudioGen, u8 setclr);
int XhdmiACRCtrl_RxMode (XhdmiAudioGen_t *AudioGen, u8 setclr);
int XhdmiACRCtrl_TxMode (XhdmiAudioGen_t *AudioGen, u8 setclr);

#define XAudGen_In32   Xil_In32  /**< Input Operations */
#define XAudGen_Out32  Xil_Out32 /**< Output Operations */

/*****************************************************************************/
/**
 *
 * This function macro reads the given register.
 *
 * @param  BaseAddress is the base address of the Audio Generator core.
 * @param  RegOffset is the register offset of the register
 *
 * @return The 32-bit value of the register.
 *
 * @note   C-style signature:
 *   u32 XAudGen_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
 ******************************************************************************/
#define XAudGen_ReadReg(BaseAddress, RegOffset) \
    XAudGen_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
 *
 * Write the given register.
 *
 * @param  BaseAddress is the base address of the Audio Generator core.
 * @param  RegOffset is the register offset of the register
 * @param  Data is the 32-bit value to write to the register.
 *
 * @return None.
 *
 * @note   C-style signature:
 *   void XAudGen_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
 ******************************************************************************/
#define XAudGen_WriteReg(BaseAddress, RegOffset, Data)   \
    XAudGen_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))

#define AUD_CTRL  0x000	/* Audio Generator Control Register */#define AUD_CFG	  0x004	/* Audio Generator Configuration Register */#define CH1_CTRL  0x010	/* Audio Channel 1 Control Register */#define CH2_CTRL  0x020	/* Audio Channel 2 Control Register */#define CH3_CTRL  0x030	/* Audio Channel 3 Control Register */#define CH4_CTRL  0x040	/* Audio Channel 4 Control Register */#define CH5_CTRL  0x050	/* Audio Channel 5 Control Register */#define CH6_CTRL  0x060	/* Audio Channel 6 Control Register */#define CH7_CTRL  0x070	/* Audio Channel 7 Control Register */#define CH8_CTRL  0x080	/* Audio Channel 8 Control Register */#define CHSTS_0	  0x0A0	/* Audio Channel Status 0 Register */#define CHSTS_1	  0x0A4	/* Audio Channel Status 1 Register */#define CHSTS_2	  0x0A8	/* Audio Channel Status 2 Register */#define CHSTS_3	  0x0AC	/* Audio Channel Status 3 Register */#define CHSTS_4	  0x0B0	/* Audio Channel Status 4 Register */#define CHSTS_5	  0x0B4	/* Audio Channel Status 5 Register */

/* Audio Control Register */
#define AUD_CTRL_REG_AUD_RST_MASK     0x1
#define AUD_CTRL_REG_AUD_RST_SHIFT    0
#define AUD_CTRL_REG_AUD_START_MASK   0x1
#define AUD_CTRL_REG_AUD_START_SHIFT  1
#define AUD_CTRL_REG_AUD_UPDCFG_MASK  0x1
#define AUD_CTRL_REG_AUD_UPDCFG_SHIFT 2
#define AUD_CTRL_REG_AUD_DROP_MASK    0x1
#define AUD_CTRL_REG_AUD_DROP_SHIFT   3

/* Audio Configuration Register */
#define AUD_CFG_REG_SAMPRATE_MASK  0xF /* Sample rate */#define AUD_CFG_REG_SAMPRATE_SHIFT 0
#define AUD_CFG_REG_NUMCH_MASK     0xF /* Number of channels */#define AUD_CFG_REG_NUMCH_SHIFT    8

/* Channel Control Register */
#define CH_CTRL_REG_PATTERN_MASK  0x3 /* Audio pattern */#define CH_CTRL_REG_PATTERN_SHIFT 0
#define CH_CTRL_REG_PERIOD_MASK   0xF /* Reserved */#define CH_CTRL_REG_PERIOD_SHIFT  8

/* ACR Control registers */
#define ACR_VER      0x000
#define ACR_CTRL     0x004
#define ACR_CTS_IN   0x008
#define ACR_N_IN     0x00C
#define ACR_CTS_OUT  0x010
#define ACR_N_OUT    0x014

/* ACR Control Register */
#define ACR_CTRL_ENAB_ACR_MASK      0x1
#define ACR_CTRL_ENAB_ACR_SHIFT     0
#define ACR_CTRL_SEL_ACR_MASK       0x1
#define ACR_CTRL_SEL_ACR_SHIFT      1
#define ACR_CTRL_AUD_RST_MASK       0x1
#define ACR_CTRL_AUD_RST_SHIFT      2
#define ACR_CTRL_TMDSCLKRATIO_MASK  0x1
#define ACR_CTRL_TMDSCLKRATIO_SHIFT 3
#define ACR_CTRL_RX_MODE_MASK       0x1
#define ACR_CTRL_RX_MODE_SHIFT      4
#define ACR_CTRL_TX_MODE_MASK       0x1
#define ACR_CTRL_TX_MODE_SHIFT      5

#define ACR_SEL_GEN 1  /* Select generated ACR values */#define ACR_SEL_IN  0  /* Select input ACR values */

#define ACR_FRL_MODE 1    /* FRL Mode */
#define ACR_TMDS_MODE  0  /* TMDS Mode */
#endif /* AUDIOGEN_DRV_H_ */
