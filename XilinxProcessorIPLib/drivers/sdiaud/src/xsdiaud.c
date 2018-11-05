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
 * @addtogroup sdiaud_v1_1
 * @{
 * <pre>
 *
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar   02/14/18  Initial release.
 * 1.1   kar   04/25/18  Changed Set Clk Phase API's 2nd argument description.
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
//!< Audio Embed total number of bytes in the 6 channel status registers

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
void XSdiAud_Ext_GetChStat(XSdiAud *InstancePtr, u8 *ChStatBuf)
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
	Xil_AssertVoid(XSdiAud_SRate <= XSDIAUD_SAMPRATE2);
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
 * This Audio Embed function sets the line standard
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_LS is the line standard, it is enum XSdiAud_LineStnd.
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Emb_SetLineStd(XSdiAud *InstancePtr, XSdiAud_LineStnd XSdiAud_LS)
{
	u32 XSdiAud_LinSt;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
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
 *         groups which are present.
 *
 ******************************************************************************/
XSdiAud_GrpsPrsnt XSdiAud_DetAudGrp(XSdiAud *InstancePtr)
{
	u32 XSdiAud_GP;
	XSdiAud_GrpsPrsnt XSdiAud_GPE;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	XSdiAud_GP = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_GRP_PRES_REG_OFFSET);
	XSdiAud_GPE = XSdiAud_GP & XSDIAUD_GRP_PRESNT_MASK;
	switch (XSdiAud_GPE) {
	case XSDIAUD_GROUP_0:
		xil_printf("No Groups in the incoming SDI Stream\r\n");
		break;

	case XSDIAUD_GROUP_1:
		xil_printf("Incoming SDI Stream has Group 1\r\n");
		break;
	case XSDIAUD_GROUP_2:
		xil_printf("Incoming SDI Stream has Group 2\r\n");
		break;

	case XSDIAUD_GROUP_1_2:
		xil_printf("Incoming SDI Stream has Groups 1 & 2\r\n");
		break;

	case XSDIAUD_GROUP_3:
		xil_printf("Incoming SDI Stream has Group 3\r\n");
		break;

	case XSDIAUD_GROUP_1_3:
		xil_printf("Incoming SDI Stream has Groups 1 & 3\r\n");
		break;

	case XSDIAUD_GROUP_2_3:
		xil_printf("Incoming SDI Stream has Groups 2 & 3\r\n");
		break;

	case XSDIAUD_GROUP_1_2_3:
		xil_printf("Incoming SDI Stream has Groups 1,2& 3\r\n");
		break;

	case XSDIAUD_GROUP_4:
		xil_printf("Incoming SDI Stream has Groups 4\r\n");
		break;

	case XSDIAUD_GROUP_1_4:
		xil_printf("Incoming SDI Stream has Groups 1 & 4\r\n");
		break;

	case XSDIAUD_GROUP_2_4:
		xil_printf("Incoming SDI Stream has Groups 2 & 4\r\n");
		break;

	case XSDIAUD_GROUP_1_2_4:
		xil_printf("Incoming SDI Stream has Groups 1,2& 4\r\n");
		break;

	case XSDIAUD_GROUP_3_4:
		xil_printf("Incoming SDI Stream has groups 3 & 4\r\n");
		break;

	case XSDIAUD_GROUP_1_3_4:
		xil_printf("Incoming SDI Stream has Groups 1,3& 4\r\n");
		break;

	case XSDIAUD_GROUP_2_3_4:
		xil_printf("Incoming SDI Stream has Groups 2,3& 4\r\n");
		break;

	case XSDIAUD_GROUP_ALL:
		xil_printf("Incoming SDI Stream has all 4 Groups\r\n");
		break;

	default:
		xil_printf("Invalid case\r\n");
		break;
	}
	return XSdiAud_GPE;
}

/*****************************************************************************/
/**
 * This function sets the channel count by taking the number of channels
 * and the start group number as the arguments.
 * This API writes to the Mux or DeMux control registers, the number of
 * Mux or DeMux control registers written, depends on the number of channels.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiStrtGrpNum is start group number, it is enum XSdiAud_GrpNum,
 *         its value can be 1 or 2 or 3 or 4.
 * @param  XSdiANumOfCh is number of channels, it is enum XSdiAud_NumOfCh,
 *         its value can be any value from 1 to 16.
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_SetCh(XSdiAud *InstancePtr, XSdiAud_GrpNum XSdiStrtGrpNum,
		XSdiAud_NumOfCh XSdiANumOfCh)
{
	u32 Mod0_LoopCount, ModNot0_LoopCount, XSdiANumOfCh_div4,
	    XSdiANumOfCh_mod4, MuxOrDmux_Offset, MuxOrDmux_RegVal,
	    XSdiStrtGrpNumInc;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XSdiANumOfCh <= InstancePtr->Config.MaxNumChannels);
	Xil_AssertVoid((XSdiANumOfCh%2) == 0);
	Xil_AssertVoid(((XSdiANumOfCh <= XSDIAUD_4_CHANNELS) &&
				(XSdiStrtGrpNum <= XSDIAUD_GROUP4)) ||
			((XSdiANumOfCh <= XSDIAUD_8_CHANNELS) &&
			 (XSdiStrtGrpNum <= XSDIAUD_GROUP3)) ||
			((XSdiANumOfCh <= XSDIAUD_12_CHANNELS) &&
			 (XSdiStrtGrpNum <= XSDIAUD_GROUP2)) ||
			((XSdiANumOfCh <= XSDIAUD_16_CHANNELS) &&
			 (XSdiStrtGrpNum == XSDIAUD_GROUP1)));

	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			 XSDIAUD_AXIS_CHCOUNT_REG_OFFSET, XSdiANumOfCh);
	InstancePtr->StrtGrpNum =  XSdiStrtGrpNum;
	XSdiANumOfCh_div4 = (XSdiANumOfCh)/4;
	XSdiANumOfCh_mod4 = (XSdiANumOfCh)%4;
	InstancePtr->NumOfCh = XSdiANumOfCh;
	XSdiStrtGrpNumInc = XSdiStrtGrpNum - 1;
	if (XSdiANumOfCh_mod4 == 0) {
		for (Mod0_LoopCount = 0; Mod0_LoopCount < XSdiANumOfCh_div4; Mod0_LoopCount++) {
			MuxOrDmux_Offset = XSDIAUD_MUX1_OR_DMUX1_CNTRL_REG_OFFSET + (4 * Mod0_LoopCount);
			MuxOrDmux_RegVal = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress, MuxOrDmux_Offset);
			MuxOrDmux_RegVal &= ~XSDIAUD_EMD_MUX_CNT_GS_MASK;
			MuxOrDmux_RegVal |= XSdiStrtGrpNumInc;
			XSdiStrtGrpNumInc = XSdiStrtGrpNumInc + 1;
			XSdiAud_WriteReg(InstancePtr->Config.BaseAddress, MuxOrDmux_Offset, MuxOrDmux_RegVal);
			MuxOrDmux_Offset = 0;
			MuxOrDmux_RegVal = 0;
		}
	} else {

		for (ModNot0_LoopCount = 0; ModNot0_LoopCount <= XSdiANumOfCh_div4; ModNot0_LoopCount++) {
			MuxOrDmux_Offset = XSDIAUD_MUX1_OR_DMUX1_CNTRL_REG_OFFSET + 4 * ModNot0_LoopCount;
			MuxOrDmux_RegVal = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress, MuxOrDmux_Offset);
			MuxOrDmux_RegVal &= ~XSDIAUD_EMD_MUX_CNT_GS_MASK;
			MuxOrDmux_RegVal |= XSdiStrtGrpNumInc;
			XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			MuxOrDmux_Offset, MuxOrDmux_RegVal);
			XSdiStrtGrpNumInc = XSdiStrtGrpNumInc + 1;
			MuxOrDmux_Offset = 0;
			MuxOrDmux_RegVal = 0;
		}
	}
}

/*****************************************************************************/
/**
 * This function mutes a specific channel in a specific group.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAGrpNum is the group number, it is enum XSdiAud_GrpNum,
 *         its value can be 1 or 2 or 3 or 4.
 * @param  XSdiAChNum is channel number, it is enum XSdiAud_GrpXChNum,
 *         its value can be 1 or 2 or 3 or 4.
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Ext_Mute(XSdiAud *InstancePtr, XSdiAud_GrpNum XSdiAGrpNum,
		XSdiAud_GrpXChNum XSdiAChNum)
{
	u32 MuxOrDmux_Offset, MuxOrDmux_RegVal, NumOfCh_mod4, NumOfCh_div4;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XSdiAGrpNum >= InstancePtr->StrtGrpNum);
	NumOfCh_mod4 = (InstancePtr->NumOfCh) % 4;
	Xil_AssertVoid(((XSdiAChNum <= XSDIAUD_GROUPX_CHANNEL4) &&
				(NumOfCh_mod4 == 0)) ||
				((XSdiAChNum <= XSDIAUD_GROUPX_CHANNEL2) &&
				(NumOfCh_mod4 != 0)));
	NumOfCh_div4 = (InstancePtr->NumOfCh)/4;
	Xil_AssertVoid(((XSdiAGrpNum <= NumOfCh_div4) && (NumOfCh_mod4 == 0)) ||
				((XSdiAGrpNum <= (NumOfCh_div4 + 1)) &&
				(NumOfCh_mod4 != 0)));

	MuxOrDmux_Offset = XSDIAUD_DMUX1_CNTRL_REG_OFFSET +
			(4 * (XSdiAGrpNum - (InstancePtr->StrtGrpNum)));
	MuxOrDmux_RegVal = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			MuxOrDmux_Offset);
	switch (XSdiAChNum) {
	case 1:
		MuxOrDmux_RegVal &= ~XSDIAUD_EXT_DMUX_MUTE1_MASK;
		MuxOrDmux_RegVal |= XSDIAUD_EXT_DMUX_MUTE1_MASK;
		break;

	case 2:
		MuxOrDmux_RegVal &= ~XSDIAUD_EXT_DMUX_MUTE2_MASK;
		MuxOrDmux_RegVal |= XSDIAUD_EXT_DMUX_MUTE2_MASK;
		break;

	case 3:
		MuxOrDmux_RegVal &= ~XSDIAUD_EXT_DMUX_MUTE3_MASK;
		MuxOrDmux_RegVal |= XSDIAUD_EXT_DMUX_MUTE3_MASK;
		break;

	case 4:
		MuxOrDmux_RegVal &= ~XSDIAUD_EXT_DMUX_MUTE4_MASK;
		MuxOrDmux_RegVal |= XSDIAUD_EXT_DMUX_MUTE4_MASK;
		break;
	}
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress, MuxOrDmux_Offset,
		    MuxOrDmux_RegVal);
}

/*****************************************************************************/
/**
* This function reads the control packet status register and returns the 16 bit
* active channel field related to the 4 groups.
*
* @param  InstancePtr is a pointer to the XSdiAud instance.
*
* @return Active channel field (type u32) is returned.
*
******************************************************************************/
u32 XSdiAud_Ext_GetActCh(XSdiAud *InstancePtr)
{
    u32 XSdiAud_ActReg;

    /* Verify arguments */
    Xil_AssertNonvoid(InstancePtr != NULL);

    XSdiAud_ActReg = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
                                           XSDIAUD_EXT_CNTRL_PKTSTAT_REG_OFFSET);
    XSdiAud_ActReg = (XSdiAud_ActReg & XSDIAUD_EXT_PKTST_AC_MASK) >> XSDIAUD_EXT_PKTST_AC_SHIFT;

    return XSdiAud_ActReg;
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
void XSdiAud_ResetReg(XSdiAud *InstancePtr)
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
void XSdiAud_ResetCoreEn(XSdiAud *InstancePtr, u8 RstCoreEnable)
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

/*****************************************************************************/
/**
 * This Audio Embed function controls the rate at which audio samples are
 * inserted on to the SDI stream.
 *
 * @param  InstancePtr is a pointer to the XSdiAud instance.
 * @param  XSdiAud_RCE can be 0 or 1.
 *         0 - Audio samples are inserted when they are available.
 *         1 - Number of audio samples per video line are limited based on video
 *             resolution, frame rate and audio sample rate.
 *
 * @return none
 *
 ******************************************************************************/
void XSdiAud_Emb_RateCntrlEn(XSdiAud *InstancePtr, u8 XSdiAud_RCE)
{
	u32 XSdiAud_RateCntrlEn, XSdiAud_RC;
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(XSdiAud_RCE <= 1);
	XSdiAud_RateCntrlEn = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET);
	XSdiAud_RateCntrlEn &= ~XSDIAUD_EMB_AUD_CNT_RCE_MASK;
	XSdiAud_RC = XSdiAud_RCE << XSDIAUD_EMB_AUD_CNT_RCE_SHIFT;
	XSdiAud_RateCntrlEn |= XSdiAud_RC;
	XSdiAud_WriteReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_AUD_CNTRL_REG_OFFSET, XSdiAud_RateCntrlEn);
}
/** @} */
