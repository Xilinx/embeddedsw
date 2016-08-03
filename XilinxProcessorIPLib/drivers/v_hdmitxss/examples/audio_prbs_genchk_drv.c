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
 * @file audio_prbs_genchk_drv.c
 *
 * This file contains the driver implementation for the Audio PRBS generator
 * and checker used in the HDMI example design.
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

#include "audio_prbs_genchk_drv.h"

/*****************************************************************************/
/**
*
* This function initializes the Audio PRBS Generator/Checker.
* This function must be called prior to using the Generator/Checker.
*
* @param	InstancePtr is a pointer to the XAudioPRBSGenChk_t core instance.
* @param	PRBSGenChkBase is the base address for the core.
*
* @return
*  - XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
int XAudioPRBSGenChk_Init (XAudioPRBSGenChk_t *InstancePtr, u32 PRBSGenChkBase)
{
  InstancePtr->PRBSGenChkBase = PRBSGenChkBase;

  // Set the AXI clock frequency (Hz).
  // This is needed in order to generate a 1 sec reference pulse
  // for verifying the sample count counters.
  XAudioPRBSGenChk_SetAxiClkFrq(InstancePtr, 100000000);

  // Set the number of samples the sample count can deviate
  // from the expected count before an error is generated
  XAudioPRBSGenChk_SetSmpCntThres(InstancePtr, 5);

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function enables the Generator/Checker
*
* @param InstancePtr is a pointer to the XAudioPRBSGenChk_t core instance.
* @param setclr specifies TRUE/FALSE value to either assert or
*        release Generator/Checker reset.
*
* @return
*  - XST_SUCCESS if action was successful
*
* @note  None.
*
******************************************************************************/
int XAudioPRBSGenChk_Enable (XAudioPRBSGenChk_t *InstancePtr, u8 setclr)
{
  u32 data;

  data = XAudio_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL);

  if (setclr)
    data |= (1 << PRBS_CTRL_REG_ENABLE_SHIFT);
  else
    data &= ~(PRBS_CTRL_REG_ENABLE_MASK << PRBS_CTRL_REG_ENABLE_SHIFT);

  XAudio_PRBS_GenChk_WriteReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL, data);

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function returns the status of the Checker
*
* @param InstancePtr is a pointer to the XAudioPRBSGenChk_t core instance.
*
* @return
*  - TRUE  if checker is enabled
*  - FALSE if checker is disabled
*
* @note The checker is enabled after the enable bit is set and after a start
*       of audio block has been seen.
*
******************************************************************************/
u8 XAudioPRBSGenChk_IsEnabled (XAudioPRBSGenChk_t *InstancePtr)
{
  u32 data;

  data = XAudio_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL);

  if (data & (PRBS_CTRL_REG_ENABLE_MASK << PRBS_CTRL_REG_ENABLE_SHIFT))
    return TRUE;
  else
    return FALSE;
}

/*****************************************************************************/
/**
*
* This function sets the AXI clock frequency configuration
*
* @param InstancePtr is a pointer to the XAudioPRBSGenChk_t core instance.
* @param ClkFrq specifies the used clock frequency in Hz
*
* @return
*  - XST_SUCCESS if action was successful
*
* @note  None.
*
******************************************************************************/
int XAudioPRBSGenChk_SetAxiClkFrq(XAudioPRBSGenChk_t *InstancePtr, u32 ClkFrq)
{
  XAudio_PRBS_GenChk_WriteReg(InstancePtr->PRBSGenChkBase, PRBS_AXICLKFRQ, ClkFrq);

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the audio configuration for the Generator/Checker
*
* @param InstancePtr is a pointer to the XAudioPRBSGenChk_t core instance.
* @param SampleRate specifies the audio sample rate.
*
* @return
*  - XST_SUCCESS if action was successful
*
* @note  None.
*
******************************************************************************/
int XAudioPRBSGenChk_SetAudCfg (XAudioPRBSGenChk_t *InstancePtr, AudioRate_t SampleRate)
{
  u32 data;

  data = XAudio_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL);
  data &= ~(PRBS_CTRL_REG_AUDCFG_MASK << PRBS_CTRL_REG_AUDCFG_SHIFT);

  if (SampleRate > XAUD_SRATE_192K)
  {
    xil_printf("AudioPRBSGenChk: Invalid sample rate specified\n\r");
    return XST_FAILURE;
  }

  data |= ((SampleRate & PRBS_CTRL_REG_AUDCFG_MASK) << PRBS_CTRL_REG_AUDCFG_SHIFT);

  XAudio_PRBS_GenChk_WriteReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL, data);

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the audio channel configuration for the Generator/Checker
*
* @param InstancePtr is a pointer to the XAudioPRBSGenChk_t core instance.
* @param ChannelCount specifies the audio channel count.
*
* @return
*  - XST_SUCCESS if action was successful
*
* @note  None.
*
******************************************************************************/
int XAudioPRBSGenChk_SetChCfg(XAudioPRBSGenChk_t *InstancePtr, u8 ChannelCount)
{
  u32 data;

  data = XAudio_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL);
  data &= ~(PRBS_CTRL_REG_CHCFG_MASK << PRBS_CTRL_REG_CHCFG_SHIFT);

  if (ChannelCount > 8)
  {
    xil_printf("AudioPRBSGenChk: Invalid channel count specified\n\r");
    return XST_FAILURE;
  }

  data |= (((ChannelCount-1) & PRBS_CTRL_REG_CHCFG_MASK) << PRBS_CTRL_REG_CHCFG_SHIFT);

  XAudio_PRBS_GenChk_WriteReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL, data);

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets the threshold value for the sample count checker
*
* @param InstancePtr is a pointer to the XAudioPRBSGenChk_t core instance.
* @param Threshold specifies the number of audio samples the sample count can
*        deviate from the expected before generating a sample count error.
*
* @return
*  - XST_SUCCESS if action was successful
*
* @note  None.
*
******************************************************************************/
int XAudioPRBSGenChk_SetSmpCntThres(XAudioPRBSGenChk_t *InstancePtr, u8 Threshold)
{
  u32 data;

  data = XAudio_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL);
  data &= ~(PRBS_CTRL_REG_SMPCNTTHRES_MASK << PRBS_CTRL_REG_SMPCNTTHRES_SHIFT);

  if (Threshold > PRBS_CTRL_REG_SMPCNTTHRES_MASK)
  {
    xil_printf("AudioPRBSGenChk: Invalid sample count threshold specified\n\r");
    return XST_FAILURE;
  }

  data |= ((Threshold & PRBS_CTRL_REG_SMPCNTTHRES_MASK) << PRBS_CTRL_REG_SMPCNTTHRES_SHIFT);

  XAudio_PRBS_GenChk_WriteReg(InstancePtr->PRBSGenChkBase, PRBS_CTRL, data);

  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function returns the data error count from the checker
*
* @param InstancePtr is a pointer to the XAudioPRBSGenChk_t core instance..
*
* @return The number of data errors found
*
*
* @note  The count value does not wrap after reaching 0xFFFFFFFF.
*
******************************************************************************/
u32 XAudioPRBSGenChk_GetDataErrCnt (XAudioPRBSGenChk_t *InstancePtr)
{
  return XAudio_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, PRBS_DAT_ERR_CNT);
}

/*****************************************************************************/
/**
*
* This function returns the sample count error count from the checker
*
* @param InstancePtr is a pointer to the XAudioPRBSGenChk_t core instance..
*
* @return The number of sample count errors found
*
*
* @note  The count value does not wrap after reaching 0xFFFFFFFF.
*
******************************************************************************/
u32 XAudioPRBSGenChk_GetSmpCntErrCnt (XAudioPRBSGenChk_t *InstancePtr)
{
  return XAudio_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, PRBS_SMPCNT_ERR_CNT);
}


/*****************************************************************************/
/**
*
* This function returns the captured sample count for the last sample count
* error from the checker
*
* @param InstancePtr is a pointer to the XAudioPRBSGenChk_t core instance..
* @param ChannelID specifies the audio channel
*
* @return The captured sample counter value for the specified channel
*
*
* @note  Sample count capture values are initialized to 0xFFFFFFFF during reset.
*
******************************************************************************/
u32 XAudioPRBSGenChk_GetSmpCntCapt (XAudioPRBSGenChk_t *InstancePtr, u8 ChannelID)
{
  if (ChannelID > 8)
  {
    xil_printf("AudioPRBSGenChk: Invalid channel ID specified\n\r");
    return 0;
  }

  u32 RegOffset = ((ChannelID-1)*0x4)+PRBS_SMPCNT_CAPT_0;

  return XAudio_PRBS_GenChk_ReadReg(InstancePtr->PRBSGenChkBase, RegOffset);
}

/*****************************************************************************/
/**
*
* This function clears the error counts for the checker
*
* @param InstancePtr is a pointer to the XAudioPRBSGenChk_t core instance.
*
* @return
*  - XST_SUCCESS if action was successful
*
* @note
*
******************************************************************************/
int XAudioPRBSGenChk_ClrErrCnt (XAudioPRBSGenChk_t *InstancePtr)
{
  // Any write to the data error count register clears it.
  XAudio_PRBS_GenChk_WriteReg(InstancePtr->PRBSGenChkBase, PRBS_DAT_ERR_CNT, 0);

  return XST_SUCCESS;
}
