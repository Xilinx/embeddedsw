/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xaudioformatter.c
* @addtogroup audio_formatter Overview
* @{
*
* This file contains the implementation of the interface functions for audio
* formater driver. Refer to the header file xaudioformatter.h for more detailed
* information.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaudioformatter.h"

/************************** Constant Definitions *****************************/
#define XAUD_CHSTAT_NUMBER_OF_BYTES  24
/************************** Function Prototypes ******************************/


/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This function soft resets the audio formatter DMA core.
*
* @param	InstancePtr is a pointer to the XAudioFormatter instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAudioFormatterDMAReset(XAudioFormatter *InstancePtr)
{
	u32 offset;
	u32 val;

	if (InstancePtr->ChannelId == XAudioFormatter_S2MM) {
		offset = XAUD_FORMATTER_S2MM_OFFSET;
	}
	if (InstancePtr->ChannelId == XAudioFormatter_MM2S) {
		offset = XAUD_FORMATTER_MM2S_OFFSET;
	}
	val = XAudioFormatter_ReadReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset);
	val |= XAUD_CTRL_RESET_MASK;
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset, val);
	while (XAudioFormatter_ReadReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset) & XAUD_CTRL_RESET_MASK);
}

/*****************************************************************************/
/**
*
* This function initializes the audio formatter core instance.
*
* @param	InstancePtr is a pointer to the XAudioFormatter instance.
* @param	CfgPtr is a reference to a structure containing information
*		about a specific XAudioFormatter instance.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*
* @note		None.
*
******************************************************************************/
u32 XAudioFormatter_CfgInitialize(XAudioFormatter *InstancePtr,
	XAudioFormatter_Config *CfgPtr)
{
	u32 val;
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);

	/* Setup the instance */
	InstancePtr->Config = *CfgPtr;
	InstancePtr->BaseAddress = CfgPtr->BaseAddress;
	val = XAudioFormatter_ReadReg(CfgPtr->BaseAddress,
		XAUD_FORMATTER_CORE_CONFIG);
	if (val & XAUD_CFG_MM2S_MASK) {
		InstancePtr->mm2s_presence = TRUE;
		InstancePtr->ChannelId = XAudioFormatter_MM2S;
		XAudioFormatterDMAReset(InstancePtr);
	}
	if (val & XAUD_CFG_S2MM_MASK) {
		InstancePtr->s2mm_presence = TRUE;
		InstancePtr->ChannelId = XAudioFormatter_S2MM;
		XAudioFormatterDMAReset(InstancePtr);
	}
	InstancePtr->IsReady = (u32)(XIL_COMPONENT_IS_READY);

	return XST_SUCCESS;

}

/*****************************************************************************/
/**
*
* This function clears the interrupts for audio formatter core.
*
* @param	InstancePtr is a pointer to the XAudioFormatter instance.
* @param	offset this is the offset to S2MM or MM2S.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAudioFormatter_InterruptClear(XAudioFormatter *InstancePtr, u32 mask)
{
	u32 offset;
	u32 val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->ChannelId == XAudioFormatter_S2MM) {
		offset = XAUD_FORMATTER_S2MM_OFFSET;
	}
	if (InstancePtr->ChannelId == XAudioFormatter_MM2S) {
		offset = XAUD_FORMATTER_MM2S_OFFSET;
	}
	val = XAudioFormatter_ReadReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_STS + offset);
	val |= mask;
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_STS + offset, val);
}

/*****************************************************************************/
/**
*
* This function enables the interrupts for audio formatter core.
*
* @param	InstancePtr is a pointer to the XAudioFormatter instance.
* @param	Mask is the interrupt mask.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAudioFormatter_InterruptEnable(XAudioFormatter *InstancePtr, u32 Mask)
{
	u32 val;
	u32 offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->ChannelId == XAudioFormatter_S2MM) {
		offset = XAUD_FORMATTER_S2MM_OFFSET;
	}
	if (InstancePtr->ChannelId == XAudioFormatter_MM2S) {
		offset = XAUD_FORMATTER_MM2S_OFFSET;
	}
	val = XAudioFormatter_ReadReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset);
	val |= Mask;
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset, val);
}

/*****************************************************************************/
/**
*
* This function disables the interrupts for audio formatter core.
*
* @param	InstancePtr is a pointer to the XAudioFormatter instance.
* @param	Mask is the interrupt mask.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAudioFormatter_InterruptDisable(XAudioFormatter *InstancePtr, u32 Mask)
{
	u32 val;
	u32 offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->ChannelId == XAudioFormatter_S2MM) {
		offset = XAUD_FORMATTER_S2MM_OFFSET;
	}
	if (InstancePtr->ChannelId == XAudioFormatter_MM2S) {
		offset = XAUD_FORMATTER_MM2S_OFFSET;
	}
	val = XAudioFormatter_ReadReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset);
	val &= ~Mask;
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset, val);
}

/*****************************************************************************/
/**
*
* This function starts the dma for audio formatter core.
*
* @param	InstancePtr is a pointer to the XAudioFormatter instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAudioFormatterDMAStart(XAudioFormatter *InstancePtr)
{
	u32 val;
	u32 offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->ChannelId == XAudioFormatter_S2MM) {
		offset = XAUD_FORMATTER_S2MM_OFFSET;
	}
	if (InstancePtr->ChannelId == XAudioFormatter_MM2S) {
		offset = XAUD_FORMATTER_MM2S_OFFSET;
	}
	val = XAudioFormatter_ReadReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset);
	val |= XAUD_CTRL_DMA_EN_MASK;
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset, val);
}

/*****************************************************************************/
/**
*
* This function stops the dma for audio formatter core.
*
* @param	InstancePtr is a pointer to the XAudioFormatter instance.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAudioFormatterDMAStop(XAudioFormatter *InstancePtr)
{
	u32 val;
	u32 offset;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->ChannelId == XAudioFormatter_S2MM) {
		offset = XAUD_FORMATTER_S2MM_OFFSET;
	}
	if (InstancePtr->ChannelId == XAudioFormatter_MM2S) {
		offset = XAUD_FORMATTER_MM2S_OFFSET;
	}
	val = XAudioFormatter_ReadReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset);
	val &= ~XAUD_CTRL_DMA_EN_MASK;
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset, val);
}

/*****************************************************************************/
/**
*
* This function sets the hw params for audio formatter core.
*
* @param	InstancePtr is a pointer to the XAudioFormatter instance.
* @param	hw_params is a pointer to the XAudioFormatterHwParams structure
*		which contains the no. of channels, bit width, no. of periods,
*		bytes per period, bytes per channel and buffer address.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XAudioFormatterSetHwParams(XAudioFormatter *InstancePtr,
	XAudioFormatterHwParams *hw_params)
{
	u32 val;
	u32 offset;
	u32 bytes_per_ch;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(hw_params != NULL);
	Xil_AssertVoid(hw_params->bits_per_sample <= BIT_DEPTH_32);
	Xil_AssertVoid(hw_params->periods >= XAUD_PERIODS_MIN &&
			hw_params->periods <= XAUD_PERIODS_MAX);
	Xil_AssertVoid(hw_params->bytes_per_period >= XAUD_PERIOD_BYTES_MIN &&
			hw_params->bytes_per_period <= XAUD_PERIOD_BYTES_MAX);
	if (InstancePtr->ChannelId == XAudioFormatter_S2MM) {
		Xil_AssertVoid(hw_params->active_ch >= XAUD_CHANNELS_MIN &&
			hw_params->active_ch <=
			InstancePtr->Config.MaxChannelsS2MM);
		offset = XAUD_FORMATTER_S2MM_OFFSET;
	}
	if (InstancePtr->ChannelId == XAudioFormatter_MM2S) {
		Xil_AssertVoid(hw_params->active_ch >= XAUD_CHANNELS_MIN &&
			hw_params->active_ch <=
			InstancePtr->Config.MaxChannelsMM2S);
		offset = XAUD_FORMATTER_MM2S_OFFSET;
	}
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_BUFF_ADDR_LSB + offset,
		(u32) hw_params->buf_addr);
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_BUFF_ADDR_MSB + offset,
		(u32) (hw_params->buf_addr >> 32));

	val = XAudioFormatter_ReadReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset);
	val &= ~(XAUD_CTRL_DATA_WIDTH_MASK << XAUD_CTRL_DATA_WIDTH_SHIFT);
	switch (hw_params->bits_per_sample) {
		case BIT_DEPTH_8:
			val |= (BIT_DEPTH_8 << XAUD_CTRL_DATA_WIDTH_SHIFT);
			break;
		case BIT_DEPTH_16:
			val |= (BIT_DEPTH_16 << XAUD_CTRL_DATA_WIDTH_SHIFT);
			break;
		case BIT_DEPTH_20:
			val |= (BIT_DEPTH_20 << XAUD_CTRL_DATA_WIDTH_SHIFT);
			break;
		case BIT_DEPTH_24:
			val |= (BIT_DEPTH_24 << XAUD_CTRL_DATA_WIDTH_SHIFT);
			break;
		case BIT_DEPTH_32:
			val |= (BIT_DEPTH_32 << XAUD_CTRL_DATA_WIDTH_SHIFT);
			break;
	}

	val |= hw_params->active_ch << XAUD_CTRL_ACTIVE_CH_SHIFT;
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_CTRL + offset, val);

	val = (hw_params->periods << XAUD_PERIOD_CFG_PERIODS_SHIFT)
		| hw_params->bytes_per_period;
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_PERIOD_CONFIG + offset, val);
	bytes_per_ch = hw_params->bytes_per_period / hw_params->active_ch;
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_BYTES_PER_CH + offset, bytes_per_ch);
}

/*****************************************************************************/
/**
 ** This function returns the No. of bytes of data read from memory..
 **
 ** @param  InstancePtr is a pointer to the Audio Formatter instance.
 **
 ** @return - No. of bytes of data read from memory.
 **
 ******************************************************************************/
u32 XAudioFormatterGetDMATransferCount(XAudioFormatter *InstancePtr)
{
	u32 offset;
	u32 val;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	if (InstancePtr->ChannelId == XAudioFormatter_S2MM) {
		offset = XAUD_FORMATTER_S2MM_OFFSET;
	}
	if (InstancePtr->ChannelId == XAudioFormatter_MM2S) {
		offset = XAUD_FORMATTER_MM2S_OFFSET;
	}
	val = XAudioFormatter_ReadReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_XFER_COUNT + offset);
	return val;
}

/*****************************************************************************/
/**
 ** This function sets the S2MM timeout.
 **
 ** @param  InstancePtr is a pointer to the Audio Formatter instance.
 ** @param  TimeOut is the Timeout value after which all the data is flushed to
 **         memory if there is no incoming data from any channel.
 **
 ** @return - None
 **
 ******************************************************************************/
void XAudioFormatterSetS2MMTimeOut(XAudioFormatter *InstancePtr, u32 TimeOut)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_S2MM_TIMEOUT, TimeOut);
}

/*****************************************************************************/
/**
 ** This function calculates the Fs multiplier value.
 **
 ** @param  InstancePtr is a pointer to the Audio Formatter instance.
 ** @param  MClk is the frequency of the MClk.
 ** @param  Fs is the sampling frequency of the system.
 **
 ** @return - None
 **
 ******************************************************************************/
void XAudioFormatterSetFsMultiplier(XAudioFormatter *InstancePtr, u32 Mclk,
	u32 Fs)
{
	u32 val;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((Mclk > 0) && (Fs > 0) && (Mclk > Fs));
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	val = Mclk / Fs;
	XAudioFormatter_WriteReg(InstancePtr->BaseAddress,
		XAUD_FORMATTER_FS_MULTIPLIER, val);
}

/*****************************************************************************/
/**
 ** This function reads all the Channel Status registers and writes to a buffer
 **
 ** @param  InstancePtr is a pointer to the Audio Formatter instance.
 ** @param  ChStatBuf is a pointer to a buffer.
 **
 ** @return None.
 **
 *******************************************************************************/
void XSdiAud_GetChStat(XAudioFormatter *InstancePtr, u8 *ChStatBuf)
{
	u32 offset;
	u32 RegOffset;
	u32 *pBuf32 = (u32 *) ChStatBuf;
	u8 NumBytes = XAUD_CHSTAT_NUMBER_OF_BYTES;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(ChStatBuf != NULL);

	if (InstancePtr->ChannelId == XAudioFormatter_S2MM) {
		offset = XAUD_FORMATTER_S2MM_OFFSET;
	}
	if (InstancePtr->ChannelId == XAudioFormatter_MM2S) {
		offset = XAUD_FORMATTER_MM2S_OFFSET;
	}
	RegOffset = XAUD_FORMATTER_CH_STS_START + offset;
	for (int i = 0; i < NumBytes; i += 4) {
		*pBuf32 = XAudioFormatter_ReadReg(
			InstancePtr->Config.BaseAddress, RegOffset);
		pBuf32++;
		RegOffset += 4;
	}
}
/** @} */
