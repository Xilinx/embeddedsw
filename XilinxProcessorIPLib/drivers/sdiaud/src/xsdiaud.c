/*******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xsdiaud.c
 * @addtogroup sdiaud Overview
 * @{
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar   02/14/18  Initial release.
 * 1.1   kar   04/25/18  Changed Set Clk Phase API's 2nd argument description.
 * 2.0   vve   09/27/18  Add 32 channel support
 *                       Add support for channel status extraction logic both
 *                       on embed and extract side.
 *                       Add APIs to detect group change, sample rate change,
 *                       active channel change
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
#define XSDIAUD_CHSTAT_NUMBER_OF_BYTES  24

/* Bit shift */
#define BIT(n)		             (1 << (n))
//!< Audio Embed total number of bytes in the 6 channel status registers

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

u8 XSdiAud_CountBits(u32 n)
{
	u8 count = 0;
	while(n) {
		n = n & (n-1);
		count++;
	}
	return count;
}
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
		RegValue |= XSDIAUD_CNTRL_EN_MASK;
		InstancePtr->IsStarted = (XIL_COMPONENT_IS_STARTED);
	} else {
		RegValue &= ~XSDIAUD_CNTRL_EN_MASK;
		InstancePtr->IsStarted = 0;
	}

	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_CNTRL_REG_OFFSET,
			RegValue);
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
void XSdiAud_GetChStat(XSdiAud *InstancePtr, u8 *ChStatBuf)
{
	int RegOffset = XSDIAUD_EXT_CH_STAT0_REG_OFFSET;
	u32 *pBuf32 = (u32 *) ChStatBuf;
	u8 NumBytes = XSDIAUD_CHSTAT_NUMBER_OF_BYTES;

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
 * This function reads the core version register and returns its value
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 *
 * @return core version register value is returned.
 *
 ******************************************************************************/
u32 XSdiAud_GetCoreVersion(XSdiAud *InstancePtr)
{
	u32 XSdiAud_CoreVersion;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	XSdiAud_CoreVersion = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_CORE_VERSION_REG_OFFSET);
	return XSdiAud_CoreVersion;
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
 * @param  XSdiAud_SRate is the sampling rate, it is enum XSdiAud_SampRate,
 *         it can be anyone of these
 *         000 - 48 KHz,
 *         001 - 44.1 KHz,
 *         010 - 32 KHz
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Emb_SetSmpRate(XSdiAud *InstancePtr, XSdiAud_SampRate XSdiAud_SRate)
{
	u32 XSdiAud_SR;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XSdiAud_SRate <= XSDIAUD_SMPLRATE_32);
	XSdiAud_SR = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET);
	XSdiAud_SR &= ~XSDIAUD_EMB_AUD_CNT_SR_MASK;
	XSdiAud_SR |= XSdiAud_SRate;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET, XSdiAud_SR);
}

/*****************************************************************************/
/**
 * This Audio Embed function sets the sample size in only SD Mode
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_SSize is the sample size, it is enum XSdiAud_SampSize,
 *         it can be 0 or 1.
 *         0 - 20 Bit
 *         1 - 24 Bit
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Emb_SetSmpSize(XSdiAud *InstancePtr, XSdiAud_SampSize XSdiAud_SSize)
{
	u32 XSdiAud_SS, XSdiAud_SSizeS;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XSdiAud_SSize <= 1);
	XSdiAud_SS = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET);
	XSdiAud_SS &= ~XSDIAUD_EMB_AUD_CNT_SS_MASK;
	XSdiAud_SSizeS = XSdiAud_SSize << XSDIAUD_EMB_AUD_CNT_SS_SHIFT;
	XSdiAud_SS |= XSdiAud_SSizeS;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET, XSdiAud_SS);
}

/*****************************************************************************/
/**
 * This Audio Embed function sets the asynchronous data flag
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_Async_data_flag is the async data flag, it is enum
 *         XSdiAud_Asx, it can be 0 or 1.
 *         0 - Synchronous audio
 *         1 - Asynchronous audio
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Emb_SetAsx(XSdiAud *InstancePtr, XSdiAud_Asx
			XSdiAud_Async_data_flag)
{
	u32 XSdiAud_Adf, XSdiAud_Async_data_flag_val;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	XSdiAud_Adf = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET);
	XSdiAud_Adf &= ~XSDIAUD_EMB_AUD_CNT_ASX_MASK;
	XSdiAud_Async_data_flag_val = XSdiAud_Async_data_flag <<
		XSDIAUD_EMB_AUD_CNT_ASX_SHIFT;
	XSdiAud_Adf |= XSdiAud_Async_data_flag_val;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET, XSdiAud_Adf);
}

/*****************************************************************************/
/**
 * This Audio function selects the aes channel pair
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_ACP is the async data flag, it is enum
 *         XSdiAud_AesChPair, it can be any value between 0 to 16.
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_SetAesChPair(XSdiAud *InstancePtr,
				XSdiAud_AesChPair XSdiAud_ACP)
{
	u32 XSdiAud_Acp, XSdiAud_Acp_val;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	XSdiAud_Acp = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET);
	XSdiAud_Acp &= ~XSDIAUD_EMB_AUD_CNT_AES_CH_PAIR_MASK;
	XSdiAud_Acp_val = XSdiAud_ACP << XSDIAUD_EMB_AUD_CNT_AES_CH_PAIR_SHIFT;
	XSdiAud_Acp |= XSdiAud_Acp_val;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET, XSdiAud_Acp);
}

/*****************************************************************************/
/**
 * This Audio Embed function sets the video properties of the image like
 * transport scan, transport rate and transport family.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_VP is the struct containing the video family, rate and
 *         video scan information.
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Emb_SetVidProps(XSdiAud *InstancePtr,
				XSdiAud_Emb_Vid_Props *XSdiAud_VP)
{
	u32 XSdiAud_Vf, XSdiAud_Vf_val;
	u32 XSdiAud_Vr, XSdiAud_Vr_val;
	u32 XSdiAud_Vs, XSdiAud_Vs_val;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	XSdiAud_Vf = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET);
	XSdiAud_Vf &= ~XSDIAUD_EMB_VID_CNT_FAMILY_MASK;
	XSdiAud_Vf_val = XSdiAud_VP->XSdiAud_TFamily <<
				XSDIAUD_EMB_VID_CNT_FAMILY_SHIFT;
	XSdiAud_Vf |= XSdiAud_Vf_val;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET, XSdiAud_Vf);
	XSdiAud_Vr = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET);
	XSdiAud_Vr &= ~XSDIAUD_EMB_VID_CNT_RATE_MASK;
	XSdiAud_Vr_val = XSdiAud_VP->XSdiAud_TRate <<
				XSDIAUD_EMB_VID_CNT_RATE_SHIFT;
	XSdiAud_Vr |= XSdiAud_Vr_val;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET, XSdiAud_Vr);
	XSdiAud_Vs = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET);
	XSdiAud_Vs &= ~XSDIAUD_EMB_VID_CNT_SCAN_MASK;
	XSdiAud_Vs_val = XSdiAud_VP->XSdiAud_TScan <<
			XSDIAUD_EMB_VID_CNT_SCAN_SHIFT;
	XSdiAud_Vs |= XSdiAud_Vs_val;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET, XSdiAud_Vs);
}

/*****************************************************************************/
/**
 * This Audio Embed function enables the external line number
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_En can be 0 or 1, 0 is to disable and 1 is to enable
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Emb_EnExtrnLine(XSdiAud *InstancePtr, u8 XSdiAud_En)
{
	u32 XSdiAud_EEL;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	XSdiAud_EEL = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET);
	XSdiAud_EEL &= ~XSDIAUD_EMB_VID_CNT_ELE_MASK;
	XSdiAud_EEL |= (XSdiAud_En << XSDIAUD_EMB_VID_CNT_ELE_SHIFT);
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EMB_VID_CNTRL_REG_OFFSET, XSdiAud_EEL);
}

/*****************************************************************************/
/**
 * This Audio Extract function sets the clock phase
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_SetClkP enables the use of clock phase data, 0 is to enable
 *         1 is to disable
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Ext_DisableClkPhase(XSdiAud *InstancePtr, u8 XSdiAud_SetClkP)
{
	u32 XSdiAud_SCP;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	XSdiAud_SCP = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET);
	XSdiAud_SCP &= ~XSDIAUD_EXT_AUD_CNT_CP_EN_MASK;
	XSdiAud_SCP |= (XSdiAud_SetClkP << XSDIAUD_EXT_AUD_CNT_CP_EN_SHIFT);
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET, XSdiAud_SCP);
}

/*****************************************************************************/
/**
 * This function detects the Audio groups and returns the groups which
 * are present
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  GrpSt is a pointer tp no. of active groups and also which groups are
 * 	   active.
 *
 * @return None.
 *
 ******************************************************************************/
void XSdiAud_GetActGrpStatus(XSdiAud *InstancePtr, XSdiAud_ActGrpSt *GrpSt)
{
	u32 XSdiAud_GP;
	u32 XSdiAud_GPE;
	u32 i;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	XSdiAud_GP = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_ACT_GRP_PRES_REG_OFFSET);
	XSdiAud_GPE = (XSdiAud_GP & XSDIAUD_GRP_PRESNT_MASK);
	GrpSt->NumGroups = XSdiAud_CountBits(XSdiAud_GPE);
	for (i = 0; i < MAX_AUDIO_GROUPS; i++) {
		GrpSt->GrpActive[i] = XSdiAud_GPE & BIT(i);
	}
}

/*****************************************************************************/
/**
 * This function sets the channel valid register. Based on the mask provided
 * the corresponding channels are embeddded/extracted onto sdi.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAudSetChMask is the 32 bit mask used to set the specific
 *         channels.
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_SetCh(XSdiAud *InstancePtr, u32 XSdiAudSetChMask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			 XSDIAUD_VALID_CH_REG_OFFSET, XSdiAudSetChMask);
}

/*****************************************************************************/
/**
 * This function mutes a specific channel based on the mask provided.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAudMuteChMask is a 32 bit mask used to mute specific channels.
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_MuteCh(XSdiAud *InstancePtr, u32 XSdiAudMuteChMask)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);

	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
				XSDIAUD_MUTE_CH_REG_OFFSET, XSdiAudMuteChMask);
}

/*****************************************************************************/
/**
* This function reads the FIFO overflow status. Each bit when read as 1,
* indicates that RX Sample FIFO for that group is overflowing.
*
* @param  InstancePtr is a pointer to the XSdiAud instance.
*
* @return Returns a u8 value, we can know the group overflowing using the bits
*         set in this value.
*
******************************************************************************/
u8 XSdiAud_Ext_GetFIFOOvFlwStatus(XSdiAud *InstancePtr)
{
    u8 XSdiAud_FIFOOvFlw;

    /* Verify arguments */
    Xil_AssertNonvoid(InstancePtr != NULL);

    XSdiAud_FIFOOvFlw = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EXT_FIFO_OVFLW_ST_REG_OFFSET);

    return XSdiAud_FIFOOvFlw;
}

/*****************************************************************************/
/**
* This function reads the active channel information decoded from audio control
* packet.
*
* @param  InstancePtr is a pointer to the XSdiAud instance.
* @param  ActChSt is the pointer to no. of active channels and the channels
* 	  active in each group.
*
* @return None.
*
******************************************************************************/
void XSdiAud_Ext_GetAcChStatus(XSdiAud *InstancePtr, XSdiAud_ActChSt *ActChSt)
{
    u32 XSdiAud_ActCh, i;

    /* Verify arguments */
    Xil_AssertVoid(InstancePtr != NULL);

    XSdiAud_ActCh = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_ACT_CH_STAT_REG_OFFSET);

    ActChSt->NumChannels = XSdiAud_CountBits(XSdiAud_ActCh);
    for (i = 0; i < MAX_AUDIO_GROUPS; i++) {
	ActChSt->GrpActCh[i] = XSdiAud_CountBits((XSdiAud_ActCh >> (4 * i)) & 0xF);
    }
}

/*****************************************************************************/
/**
* This function reads the sample rate information decoded from audio control
* packet (0 - 48 KHz, 1 - 44.1 KHz, 2 - 32 KHz, 3 - Reserved).
*
* @param  InstancePtr is a pointer to the XSdiAud instance.
* @param  SRSt is a pointer to the sample rate information of each channel pair.
*
* @return None.
*
******************************************************************************/
void XSdiAud_Ext_GetSRStatus(XSdiAud *InstancePtr, XSdiAud_SRSt *SRSt)
{
    u32 XSdiAud_SR;
    u32 i;

    /* Verify arguments */
    Xil_AssertVoid(InstancePtr != NULL);

    XSdiAud_SR = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_SR_STAT_REG_OFFSET);
    for (i = 0; i < MAX_AUDIO_CHANNELS / 2; i++) {
	SRSt->SRChPair[i] = (XSdiAud_SR >> (2 * i)) & 0x3;
    }
}

/*****************************************************************************/
/**
* This function reads the ASX information decoded from audio control packet.
* Bit 0  - Channel 1 & 2 Asx
* Bit 1  - Channel 3 & 4 Asx
* ------
* Bit 15 - Channel 31 & 32 Asx
*
* @param  InstancePtr is a pointer to the XSdiAud instance.
* @param  AsxSt is a pointer to the Asx status information of each channel pair.
*
* @return None.
*
******************************************************************************/
void XSdiAud_Ext_GetAsxStatus(XSdiAud *InstancePtr, XSdiAud_AsxSt *AsxSt)
{
    u32 XSdiAud_Asx, i;

    /* Verify arguments */
    Xil_AssertVoid(InstancePtr != NULL);

    XSdiAud_Asx = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_ASX_STAT_REG_OFFSET);

    for (i = 0; i < (MAX_AUDIO_CHANNELS / 2); i++ ) {
	AsxSt->AsxPair[i] = (XSdiAud_Asx >> i) & 0x1;
    }
}

/*****************************************************************************/
/**
 * This function is used to soft reset the XSdiAud registers.It resets all the
 * configuration registers.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 *
 * @return None.
 *
 ******************************************************************************/
void XSdiAud_ConfigReset(XSdiAud *InstancePtr)
{
	u32 XSdiAud_RstRVal;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	XSdiAud_RstRVal = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_SOFT_RST_REG_OFFSET);
	XSdiAud_RstRVal &= ~XSDIAUD_SOFT_RST_ACLK_MASK;
	XSdiAud_RstRVal |= XSDIAUD_SOFT_RST_ACLK_MASK;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_SOFT_RST_REG_OFFSET,
			XSdiAud_RstRVal);
}

/*****************************************************************************/
/**
 * This function is used to soft reset the XSdiAud core.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  RstCoreEnable is to enable or disable. When set to 1(i.e.enable),
 *         it resets the embedded core. When set to 0, it clears the reset
 *         (i.e.disable).
 *
 * @return None.
 *
 ******************************************************************************/
void XSdiAud_CoreReset(XSdiAud *InstancePtr, u8 RstCoreEnable)
{
	u32 XSdiAud_RstCVal;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	XSdiAud_RstCVal = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_SOFT_RST_REG_OFFSET);
	XSdiAud_RstCVal &= ~XSDIAUD_SOFT_RST_SCLK_MASK;
	if(RstCoreEnable) {
	XSdiAud_RstCVal |= XSDIAUD_SOFT_RST_SCLK_MASK;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_SOFT_RST_REG_OFFSET,
			XSdiAud_RstCVal);
	} else {
	XSdiAud_RstCVal |= RstCoreEnable;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
					XSDIAUD_SOFT_RST_REG_OFFSET,
					XSdiAud_RstCVal);
	}
}
/** @} */
