/*******************************************************************************
 *
 * Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xsdiaud.c
 * @addtogroup sdiaud_v1_0
 * @{
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar   02/06/18  Initial release.
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xsdiaud.h"
#include "xsdiaud_hw.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_types.h"
/************************** Constant Definitions *****************************/

/***************** Macros (In-line Functions) Definitions *********************/
#define XSDIAUD_SOFT_RESET_REGISTER_VALUE  0X03
//!< Soft Reset Register value to reset
#define XSDIAUD_MAX_SAMP_RATE_REGISTER_VALUE  2
//!< Audio Embed Sampling rate options can be 0 or 1 or 2
#define XSDIAUD_MAX_LINE_STD_REGISTER_VALUE  31
//!< Audio Embed Line standard register maximum value is 31

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function initializes the XSdiAud.
 * This function must be called prior to using the core.
 * Initialization of the XSdiAud includes
 * setting up the instance data, and ensuring the hardware is in a quiescent
 * state.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  CfgPtr points to the configuration structure associated with
 *         the XSdiAud.
 * @param  EffectiveAddr is the base address of the device. If address
 *         translation is being used, then this parameter must reflect the
 *         virtual base address. Otherwise, the physical address should be
 *         used.
 *
 * @return
 *   - XST_SUCCESS : if successful.
 *   - XST_FAILURE : if version mismatched.
 *
 * @note None.
 *
 ******************************************************************************/
int XSdiAud_CfgInitialize(XSdiAud *InstancePtr,
		XSdiAud_Config *CfgPtr,
		UINTPTR EffectiveAddr)
{
	u32 Status;
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != 0);

	/* Setup the instance */
	InstancePtr->Config = *CfgPtr;
	InstancePtr->Config.BaseAddress = EffectiveAddr;

	/* Run the self test. */
	Status = XSdiAud_SelfTest(InstancePtr);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Disable the core */
	XSdiAud_Enable(InstancePtr, FALSE);

	/* Set the flag to indicate the driver is ready */
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function enables/disables the XSdiAud.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  Enable specifies TRUE/FALSE value to either enable or disable
 *         the XSdiAud.
 *
 * @return None.
 *
 ******************************************************************************/
void XSdiAud_Enable(XSdiAud *InstancePtr, u8 Enable)
{
	Xil_AssertVoid(InstancePtr != NULL);
	u32 RegValue = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_CNTRL_REG_OFFSET);

	if (Enable) {
		RegValue |= XSDIAUD_EMB_CNTRL_EMBEN_MASK;
		InstancePtr->IsStarted = (XIL_COMPONENT_IS_STARTED);
	} else {
		RegValue &= ~XSDIAUD_EMB_CNTRL_EMBEN_MASK;
		InstancePtr->IsStarted = 0;
	}

	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_CNTRL_REG_OFFSET,
			RegValue);
}
/*****************************************************************************/
/**
 * This function is used to soft reset the XSdiAud Instance
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @return None.
 *
 ******************************************************************************/
void XSdiAud_SoftReset(XSdiAud *InstancePtr)

{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_SOFT_RST_REG_OFFSET,
			XSDIAUD_SOFT_RESET_REGISTER_VALUE);
}
/*****************************************************************************/
/**
 * This function reads all the Channel Status registers and writes to a buffer
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  ChStatBuf is a pointer to a buffer.
 *
 * @return None.
 *
 ******************************************************************************/
void XSdiAud_Ext_GetChStat(XSdiAud *InstancePtr, u8 *ChStatBuf)
{
	int RegOffset = XSDIAUD_EXT_CH_STAT0_REG_OFFSET;
	u32 *pBuf32 = (u32 *) ChStatBuf;
	u8 NumBytes = 24;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ChStatBuf != NULL);

	for (int i = 0; i < NumBytes; i += 4) {
		*pBuf32 = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
				RegOffset);
		pBuf32++;
		RegOffset += 4;
	}
}
/*****************************************************************************/
/**
 * This function reads the Interrupt Status register and returns its value
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 *
 * @return Interrupt status register value is returned.
 *
 ******************************************************************************/
u32 XSdiAud_GetIntStat(XSdiAud *InstancePtr)
{
	u32 XSdiAud_IntStat;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	XSdiAud_IntStat = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_INT_STS_REG_OFFSET);
	return XSdiAud_IntStat;
}
/*****************************************************************************/
/**
 * This Audio Embed function sets the sampling rate
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_SRate is the sampling rate, it can be anyone of these
 *         000 - 48 KHz, 001 - 44.1 KHz, 010 - 32 KHz
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Emb_SetSmpRate(XSdiAud *InstancePtr, u8 XSdiAud_SRate)
{
	u32 XSdiAud_SR;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XSdiAud_SRate <= XSDIAUD_MAX_SAMP_RATE_REGISTER_VALUE);
	XSdiAud_SR = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET);
	XSdiAud_SR &= ~XSDIAUD_EMB_AUD_CNT_SR_MASK;
	XSdiAud_SR |= XSdiAud_SRate;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET, XSdiAud_SR);
}
/*****************************************************************************/
/**
 * This Audio Embed function sets the sample size in SD Mode
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_SSize is the sample size it can be 0 or 1.
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Emb_SetSmpSize(XSdiAud *InstancePtr, u8 XSdiAud_SSize)
{
	u32 XSdiAud_SS;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XSdiAud_SSize <= 1);
	XSdiAud_SS = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET);
	XSdiAud_SS &= ~XSDIAUD_EMB_AUD_CNT_SS_MASK;
	XSdiAud_SS |= (XSdiAud_SSize << XSDIAUD_EMB_AUD_CNT_SS_SHIFT);
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET, XSdiAud_SS);
}
/*****************************************************************************/
/**
 * This Audio Embed function sets the line standard
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_LS is the line standard, it can be anyone of these
 *         00000 - SMPTE 260M 1035i 30 Hz
 *         00001 - SMPTE 295M 1080i 25 Hz
 *         00010 - SMPTE 274M 1080i or 1080sF 30 Hz
 *         00011 - SMPTE 274M 1080i or 1080sF 25 Hz
 *         00100 - SMPTE 274M 1080p 30 Hz
 *         00101 - SMPTE 274M 1080p 25 Hz
 *         00110 - SMPTE 274M 1080p 24 Hz
 *         00111 - SMPTE 296M 720p 60 Hz
 *         01000 - SMPTE 274M 1080sF 24 Hz
 *         01001 - SMPTE 296M 720p 50 Hz
 *         01010 - SMPTE 296M 720p 30 Hz
 *         01011 - SMPTE 296M 720p 25 Hz
 *         01100 - SMPTE 296M 720p 24 Hz
 *         01101 - SMPTE 274M 1080p 60 Hz
 *         01110 - SMPTE 274M 1080p 50 Hz
 *         10000 - NTSC
 *         10001 - PAL
 *         10010 - 2160 p23.98
 *         10011 - 2160 p24
 *         10100 - 2160 p25
 *         10101 - 2160 p29.97
 *         10110 - 2160 p30
 *         10110 - 2160 p47.95
 *         10111 - 2160 p48
 *         11000 - 2160 p50
 *         11001 - 2160 p59.94
 *         11010 - 2160 p60
 *         11111 – Others
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Emb_SetLineStd(XSdiAud *InstancePtr, u8 XSdiAud_LS)
{
	u32 XSdiAud_LinSt;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XSdiAud_LS <= XSDIAUD_MAX_LINE_STD_REGISTER_VALUE);
	XSdiAud_LinSt = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET);
	XSdiAud_LinSt &= ~XSDIAUD_EMB_VID_CNT_STD_MASK;
	XSdiAud_LinSt |= XSdiAud_LS;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET, XSdiAud_LinSt);
}
/*****************************************************************************/
/**
 * This Audio Embed function enables the external line number
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_EnDsb can be 0 or 1, 0 is to disable and 1 is to enable
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Emb_EnExtrnLine(XSdiAud *InstancePtr, u8 XSdiAud_EnDsb)
{
	u32 XSdiAud_EEL;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	XSdiAud_EEL = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET);
	XSdiAud_EEL &= ~XSDIAUD_EMB_VID_CNT_ELE_MASK;
	XSdiAud_EEL |= (XSdiAud_EnDsb << XSDIAUD_EMB_VID_CNT_ELE_SHIFT);
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET, XSdiAud_EEL);
}
/*****************************************************************************/
/**
 * This Audio Extract function sets the clock phase
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_SetClkP enables the use of clock phase data, 1 is to enable
 *         0 is to disable
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Ext_SetClkPhase(XSdiAud *InstancePtr, u8 XSdiAud_SetClkP)
{
	u32 XSdiAud_SCP;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	XSdiAud_SCP = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET);
	XSdiAud_SCP &= ~XSDIAUD_EXT_AUD_CNT_CP_EN_MASK;
	XSdiAud_SCP |= XSdiAud_SetClkP;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET, XSdiAud_SCP);
}
/*****************************************************************************/
/**
 * This function detects the Audio groups and returns the groups which
 * are present
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 *
 * @return Return type is enum XSdiAud_GrpsPrsnt, by this we can know the
 *         groups which are present
 *
 ******************************************************************************/
XSdiAud_GrpsPrsnt XSdiAud_DetAudGrp(XSdiAud *InstancePtr)
{
	XSdiAud_GrpsPrsnt XSdiAud_GP;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	XSdiAud_GP = (XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
		  XSDIAUD_GRP_PRES_REG_OFFSET)) & (XSDIAUD_EMB_GRP_PRESNT_MASK);
	return XSdiAud_GP;
}
/*****************************************************************************/
/**
 * This function sets the channel by taking the number of channels
 * and the start group number as the arguments.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAGrpNum is start group number, it is enum,
 *         its value can be 1 or 2 or 3 or 4.
 * @param  XSdiANumOfCh is number of channels, it is enum,
 *         its value can be any value from 1 to 16.
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_SetCh(XSdiAud *InstancePtr, XSdiAud_GrpNum XSdiAGrpNum,
		XSdiAud_NumOfCh XSdiANumOfCh)
{
	u32 i, j, k, l, m, n;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XSdiANumOfCh < InstancePtr->Config.MaxNumChannels);
	Xil_AssertVoid((XSdiANumOfCh%2) == 0);
	k = (XSdiANumOfCh)/4;
	l = (XSdiANumOfCh)%4;
	if (l == 0) {
		for (i = 0; i <= k; i++) {
		m = XSDIAUD_MUX1_CNTRL_REG_OFFSET + (4 * i);
			n = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress, m);
			n &= ~XSDIAUD_EMD_MUX_CNT_GS_MASK;
			n |= XSdiAGrpNum - 1;
			XSdiAud_WriteReg(InstancePtr->Config.BaseAddress, m, n);
			m = 0;
			n = 0;
		}
	} else {
		k = k + 1;
		for (j = 0; j <= k; j++) {
			m = XSDIAUD_MUX1_CNTRL_REG_OFFSET + 4 * j;
			n = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress, m);
			n &= ~XSDIAUD_EMD_MUX_CNT_GS_MASK;
			n |= XSdiAGrpNum - 1;
			XSdiAud_WriteReg(InstancePtr->Config.BaseAddress, m, n);
			m = 0;
			n = 0;
		}
	}
}
/*****************************************************************************/
/**
 * This function mutes the channel
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAGrpNum is start group number, it is enum,
 *         its value can be 1 or 2 or 3 or 4.
 * @param  XSdiANumOfCh is number of channels, it is enum,
 *         its value can be any value from 1 to 16.
 * @param  XSdiAChNum is channel number, it is enum,
 *         its value can be any value from 1 to 16.
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Ext_Mute(XSdiAud *InstancePtr, XSdiAud_GrpNum XSdiAGrpNum,
		XSdiAud_NumOfCh XSdiANumOfCh, XSdiAud_ChNum XSdiAChNum)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XSdiANumOfCh < InstancePtr->Config.MaxNumChannels);
	Xil_AssertVoid((XSdiANumOfCh%2) == 0);
	/* Code to mute the channel is to be added here */
}
/** @} */
