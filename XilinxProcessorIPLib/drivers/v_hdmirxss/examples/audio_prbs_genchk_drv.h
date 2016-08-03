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
 * @file audio_prbs_genchk_drv.h
 *
 * This file contains the definitions for the Audio PRBS generator and checker
 * used in the HDMI example design.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- --- ----------   -----------------------------------------------
 * X.XX  XX  YYYY/MM/DD   ...
 * 1.00  RHe 2015/05/20   First release
 * </pre>
 *
 ******************************************************************************/

#ifndef AUDIO_PRBS_GENCHK_DRV_H_
#define AUDIO_PRBS_GENCHK_DRV_H_

#include "xvidc.h"
#include "xil_types.h"
#include "xstatus.h"
#include "xil_io.h"

#include "audiogen_drv.h"

typedef struct {
  u32 PRBSGenChkBase;
} XAudioPRBSGenChk_t;

//typedef enum {
//  XAUD_RATE_32K = 0,
//  XAUD_RATE_44K1,
//  XAUD_RATE_48K,
//  XAUD_RATE_88K2,
//  XAUD_RATE_96K,
//  XAUD_RATE_176K4,
//  XAUD_RATE_192K,
//  XAUD_NUM_SUPPORTED_RATES
//} AudioRate_t;

int XAudioPRBSGenChk_Init(XAudioPRBSGenChk_t *InstancePtr, u32 PRBSGenChkBase);
int XAudioPRBSGenChk_Enable(XAudioPRBSGenChk_t *InstancePtr, u8 setclr);
u8  XAudioPRBSGenChk_IsEnabled(XAudioPRBSGenChk_t *InstancePtr);
int XAudioPRBSGenChk_SetAxiClkFrq(XAudioPRBSGenChk_t *InstancePtr, u32 ClkFrq);
int XAudioPRBSGenChk_SetAudCfg(XAudioPRBSGenChk_t *InstancePtr, AudioRate_t SampleRate);
int XAudioPRBSGenChk_SetChCfg(XAudioPRBSGenChk_t *InstancePtr, u8 ChannelCount);
int XAudioPRBSGenChk_SetSmpCntThres(XAudioPRBSGenChk_t *InstancePtr, u8 Threshold);
u32 XAudioPRBSGenChk_GetDataErrCnt(XAudioPRBSGenChk_t *InstancePtr);
u32 XAudioPRBSGenChk_GetSmpCntErrCnt(XAudioPRBSGenChk_t *InstancePtr);
u32 XAudioPRBSGenChk_GetSmpCntCapt(XAudioPRBSGenChk_t *InstancePtr, u8 ChannelID);
int XAudioPRBSGenChk_ClrErrCnt(XAudioPRBSGenChk_t *InstancePtr);

#define XAudio_PRBS_GenChk_In32   Xil_In32  /**< Input Operations */
#define XAudio_PRBS_GenChk_Out32  Xil_Out32 /**< Output Operations */

/*****************************************************************************/
/**
 *
 * This function macro reads the given register.
 *
 * @param  BaseAddress is the base address of the Audio PRBS Generator/Checker core.
 * @param  RegOffset is the register offset of the register
 *
 * @return The 32-bit value of the register.
 *
 * @note   C-style signature:
 *   u32 XAudio_PRBS_GenChk_ReadReg(u32 BaseAddress, u32 RegOffset)
 *
 ******************************************************************************/
#define XAudio_PRBS_GenChk_ReadReg(BaseAddress, RegOffset) \
    XAudio_PRBS_GenChk_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
 *
 * Write the given register.
 *
 * @param  BaseAddress is the base address of the Audio PRBS Generator/Checker core.
 * @param  RegOffset is the register offset of the register
 * @param  Data is the 32-bit value to write to the register.
 *
 * @return None.
 *
 * @note   C-style signature:
 *   void XAudio_PRBS_GenChk_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
 *
 ******************************************************************************/
#define XAudio_PRBS_GenChk_WriteReg(BaseAddress, RegOffset, Data)   \
    XAudio_PRBS_GenChk_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))

#define PRBS_VER            0x000 // Audio PRBS Generator/Checker Version Register#define PRBS_CTRL           0x004 // Audio PRBS Generator/Checker Control Register
#define PRBS_AXICLKFRQ      0x008 // AXI Clock Frequency Register#define PRBS_DAT_ERR_CNT    0x00C // Audio PRBS Data Checker Error Count Register
#define PRBS_SMPCNT_ERR_CNT 0x010 // Audio Sample Count Checker Error Count Register
#define PRBS_SMPCNT_CAPT_0  0x014 // Channel 1 Audio Sample Count Captured Register
#define PRBS_SMPCNT_CAPT_1  0x018 // Channel 2 Audio Sample Count Captured Register
#define PRBS_SMPCNT_CAPT_2  0x01C // Channel 3 Audio Sample Count Captured Register
#define PRBS_SMPCNT_CAPT_3  0x020 // Channel 4 Audio Sample Count Captured Register
#define PRBS_SMPCNT_CAPT_4  0x024 // Channel 5 Audio Sample Count Captured Register
#define PRBS_SMPCNT_CAPT_5  0x028 // Channel 6 Audio Sample Count Captured Register
#define PRBS_SMPCNT_CAPT_6  0x02C // Channel 7 Audio Sample Count Captured Register
#define PRBS_SMPCNT_CAPT_7  0x030 // Channel 8 Audio Sample Count Captured Register

// Audio PRBS Generator/Checker Control Register
#define PRBS_CTRL_REG_ENABLE_MASK       0x1
#define PRBS_CTRL_REG_ENABLE_SHIFT      0
#define PRBS_CTRL_REG_CHCFG_MASK        0x7
#define PRBS_CTRL_REG_CHCFG_SHIFT       1
#define PRBS_CTRL_REG_AUDCFG_MASK       0x7
#define PRBS_CTRL_REG_AUDCFG_SHIFT      4
#define PRBS_CTRL_REG_SMPCNTTHRES_MASK  0x1F
#define PRBS_CTRL_REG_SMPCNTTHRES_SHIFT 7

#define PRBS_ERR_CNT_REG_MASK     0xFFFFFFFF
#define PRBS_ERR_CNT_REG_SHIFT    0


#endif /* AUDIO_PRBS_GENCHK_DRV_H_ */
