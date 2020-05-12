/*******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************/
/**
 *
 * @file xsdiaud_selftest.c
 * @addtogroup sdiaud_v2_1
 * @{
 * Contains an basic self-test API
 * @note None
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ----- ---------- -----------------------------------------------
 * 1.0    kar  02/14/18    Initial release.
 * 1.1    kar  04/02/18    Added new macros for UHD-SDI standard and channels.
 * 2.0    vve  09/27/18    Add 32 channel support
 *                         Add support for channel status extraction logic both
 *                         on embed and extract side.
 *                         Add APIs to detect group change, sample rate change,
 *                         active channel change
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xsdiaud.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xsdiaud_hw.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/***************** Macros (In-line Functions) Definitions *********************/

/*****************************************************************************/
/**
 *
 * This macro returns the XSdiAud operating mode.
 *
 * @param  InstancePtr is a pointer to the XSdiAud core instance.
 *
 * @return
 *   - TRUE  : Audio Embed
 *   - FALSE : Audio Extract
 *
 * @note C-style signature:
 *   u8 XSdiAud_IsEmbed(XSdiAud *InstancePtr)
 *
 *****************************************************************************/
#define XSdiAud_IsEmbed(InstancePtr) \
	(((XSdiAud_ReadReg((InstancePtr)->Config.BaseAddress,	\
	(XSDIAUD_GUI_PARAM_REG_OFFSET))	\
	& XSDIAUD_GUI_AUDF_MASK) >> XSDIAUD_GUI_AUDF_SHIFT) ? TRUE : FALSE)

/*****************************************************************************/
/**
 *
 * This macro returns the XSdiAud UHD-SDI standard.
 *
 * @param  InstancePtr is a pointer to the XSdiAud core instance.
 *
 * @return UHD-SDI standard
 *		0: 3G SDI
 *		1: 6G SDI
 *		2: 12G SDI 8DS
 *		3: 12G SDI 16DS
 *
 * @note C-style signature:
 *   u8 XSdiAud_GetSdiStd(XSdiAud *InstancePtr)
 *
 *****************************************************************************/
#define XSdiAud_GetSdiStd(InstancePtr) \
	((XSdiAud_ReadReg((InstancePtr)->Config.BaseAddress,	\
	(XSDIAUD_GUI_PARAM_REG_OFFSET)) \
	& XSDIAUD_GUI_STD_MASK) >> XSDIAUD_GUI_STD_SHIFT)

/*****************************************************************************/
/**
 *
 * This macro returns the XSdiAud Channel field of the GUI register
 *
 * @param  InstancePtr is a pointer to the XSdiAud core instance.
 *
 * @return Channel field of the GUI register i.e. a value in between 0 to 7
 *
 * @note C-style signature:
 *   u8 XSdiAud_GetCh(XSdiAud *InstancePtr)
 *
 *****************************************************************************/
#define XSdiAud_GetCh(InstancePtr) \
	((XSdiAud_ReadReg((InstancePtr)->Config.BaseAddress,	\
	(XSDIAUD_GUI_PARAM_REG_OFFSET))	\
	& XSDIAUD_GUI_CHAN_MASK) >> XSDIAUD_GUI_CHAN_SHIFT)

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof(x[0]))
#define XSDIAUD_DEFAULT_VALUE	0
#define XSDIAUD_VALID_CH_CNT	0xFFFFFFFF
#define XSDIAUD_CH_STS_REG_CNT	6
/*
 * Default register values after reset
 */

typedef struct {
	u32 RegOffset;
	u32 DefaultVal;
}XSdiAud_RegMap;

static XSdiAud_RegMap XSdiAud_Embed_RegMap[] = {
		{XSDIAUD_INT_EN_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_INT_STS_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_EMB_VID_CNTRL_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_AUD_CNTRL_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_VALID_CH_REG_OFFSET, XSDIAUD_VALID_CH_CNT},
		{XSDIAUD_MUTE_CH_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_ACT_GRP_PRES_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_ACT_CH_STAT_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_SR_STAT_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_ASX_STAT_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
};

static XSdiAud_RegMap XSdiAud_Extract_RegMap[] = {
		{XSDIAUD_INT_EN_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_INT_STS_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_AUD_CNTRL_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_VALID_CH_REG_OFFSET, XSDIAUD_VALID_CH_CNT},
		{XSDIAUD_MUTE_CH_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_ACT_GRP_PRES_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_EXT_FIFO_OVFLW_ST_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_ACT_CH_STAT_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_SR_STAT_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
		{XSDIAUD_ASX_STAT_REG_OFFSET, XSDIAUD_DEFAULT_VALUE},
};


#define XSDIAUD_EMBREG_CNT	ARRAY_SIZE(XSdiAud_Embed_RegMap)
#define XSDIAUD_EXTREG_CNT	ARRAY_SIZE(XSdiAud_Extract_RegMap)

/*****************************************************************************/
/**
 *
 * Runs a self-test on the driver/device. The self-test  reads the XSdi_Aud
 * registers and verifies the value.
 *
 * @param	InstancePtr is a pointer to the XSdiAud instance.
 *
 * @return
 *		- XST_SUCCESS if successful i.e. if the self test passes.
 *		- XST_FAILURE if unsuccessful i.e. if the self test fails
 *
 * @note	None.
 *
 *****************************************************************************/
int XSdiAud_SelfTest(XSdiAud *InstancePtr)
{
	int Status = XST_SUCCESS;
	u32 SdiAud_IsEmbed, SdiAud_NumCh, SdiAud_SdiStd, Data, i, reg, count;
	XSdiAud_RegMap *XSdiAud_Core_RegMap;

	/* verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	/* Read the SDI Audio Module control register to know the
	 * operating mode i.e. to know whether the core is configured
	 * as a Audio Embed or Audio Extract.
	 */
	SdiAud_IsEmbed = XSdiAud_IsEmbed(InstancePtr);
	if (SdiAud_IsEmbed != InstancePtr->Config.IsEmbed) {

	xil_printf("Core configuration (%d) doesn't match GUI value (%d).\r\n",
				SdiAud_IsEmbed, InstancePtr->Config.IsEmbed);
		return XST_FAILURE;
	}

	/* Read the SDI Audio Module control register to know the
	 * UHD-SDI Standard.
	 */
	SdiAud_SdiStd = XSdiAud_GetSdiStd(InstancePtr);
	if (SdiAud_SdiStd != InstancePtr->Config.LineRate) {

	xil_printf("Core configuration (%d) doesn't match GUI value (%d).\r\n",
				SdiAud_SdiStd, InstancePtr->Config.LineRate);
		return XST_FAILURE;
	}

	/* Read the SDI Audio Module control register to know the
	 * number of channels.
	 */
	SdiAud_NumCh = XSdiAud_GetCh(InstancePtr);
	switch (SdiAud_NumCh) {
	case 0:
		SdiAud_NumCh = XSDIAUD_2_CHANNELS;
		break;
	case 1:
		SdiAud_NumCh = XSDIAUD_4_CHANNELS;
		break;
	case 2:
		SdiAud_NumCh = XSDIAUD_6_CHANNELS;
		break;
	case 3:
		SdiAud_NumCh = XSDIAUD_8_CHANNELS;
		break;
	case 4:
		SdiAud_NumCh = XSDIAUD_10_CHANNELS;
		break;
	case 5:
		SdiAud_NumCh = XSDIAUD_12_CHANNELS;
		break;
	case 6:
		SdiAud_NumCh = XSDIAUD_14_CHANNELS;
		break;
	case 7:
		SdiAud_NumCh = XSDIAUD_16_CHANNELS;
		break;
	case 8:
		SdiAud_NumCh = XSDIAUD_32_CHANNELS;
		break;
	default:
		SdiAud_NumCh = XSDIAUD_32_CHANNELS;
		break;
	}

	if (SdiAud_NumCh != InstancePtr->Config.MaxNumChannels) {
		xil_printf("Core configuration (%d) doesn't match GUI value (%d).\r\n",
			SdiAud_NumCh, InstancePtr->Config.MaxNumChannels);
		return XST_FAILURE;
	}

	/*
	 * Reset the core, so that default values can be tested.
	 */
	XSdiAud_CoreReset(InstancePtr, TRUE);
	XSdiAud_CoreReset(InstancePtr, FALSE);
	XSdiAud_ConfigReset(InstancePtr);

	if (InstancePtr->Config.IsEmbed) {
		XSdiAud_Core_RegMap = XSdiAud_Embed_RegMap;
		count = XSDIAUD_EMBREG_CNT;
	} else {
		XSdiAud_Core_RegMap = XSdiAud_Extract_RegMap;
		count = XSDIAUD_EXTREG_CNT;
	}

	for (i = 0; i < count; i++)
	{
		Data = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
				XSdiAud_Core_RegMap[i].RegOffset);
		if (Data != XSdiAud_Core_RegMap[i].DefaultVal) {
			xil_printf("register doesn't hold reset value");
			return XST_FAILURE;
		}
	}

	/*
	 * verify channel status registers to be zero
	 */
	reg = XSDIAUD_EXT_CH_STAT0_REG_OFFSET;
	for (i = 0; i < XSDIAUD_CH_STS_REG_CNT; i++)
	{
		Data = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress, reg);
		if (Data) {
			xil_printf("channel status register doesn't hold reset value");
			return XST_FAILURE;
		}
		reg += 0x4;
	}

	return Status;
}
/** @} */
