/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_utils.c
* This file contains common functionalities required for xilsecure library
* like functions to read/write hardware registers, SSS switch configurations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.0   vns     03/09/19 Initial release
*       psl     03/26/19 Fixed MISRA-C violation
*       psl     04/05/19 Fixed IAR warnings.
* 4.1   psl     07/31/19 Fixed MISRA-C violation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsecure_utils.h"

/************************** Constant Definitions *****************************/
#ifdef XSECURE_VERSAL
/* XSecure_SssLookupTable[Input source][Resource] */
static const u8 XSecure_SssLookupTable
		[XSECURE_SSS_MAX_SRCS][XSECURE_SSS_MAX_SRCS] = {
	/*+----+-----+-----+-----+-----+-----+-----+--------+
	*|DMA0| DMA1| PTPI| AES | SHA | SBI | PZM |Invalid |
	*+----+-----+-----+-----+-----+-----+-----+--------+
	* 0x00 = INVALID value
	*/
	{0x0DU, 0x00U, 0x00U, 0x06U, 0x00U, 0x0BU, 0x03U, 0x00U}, /* DMA0 */
	{0x00U, 0x09U, 0x00U, 0x07U, 0x00U, 0x0EU, 0x04U, 0x00U}, /* DMA1 */
	{0x0DU, 0x0AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* PTPI */
	{0x0EU, 0x05U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* AES  */
	{0x0CU, 0x07U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SHA  */
	{0x05U, 0x0BU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* SBI  */
	{0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* PZI  */
	{0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, /* Invalid */
};

#else
static const u8 XSecure_SssLookupTable
			[XSECURE_SSS_MAX_SRCS][XSECURE_SSS_MAX_SRCS] = {
	/*+----+-----+-----+-----+---------+
	*|PCAP| DMA | AES | SHA | Invalid |
	*+----+-----+-----+-----+---------+
	* 0x00 = INVALID value
	*/
	{0x00U, 0x05U, 0x0AU, 0x00U, 0x00U}, /* PCAP */
	{0x03U, 0x05U, 0x0AU, 0x00U, 0x00U}, /* DMA */
	{0x00U, 0x05U, 0x00U, 0x00U, 0x00U}, /* AES */
	{0x00U, 0x05U, 0x00U, 0x00U, 0x00U}, /* SHA */
	{0x00U, 0x00U, 0x00U, 0x00U, 0x00U} /* Invalid */
};
#endif
/************************** Function Prototypes ******************************/

static void XSecure_SssDmaSrc(u16 DmaId, XSecure_SssSrc *Resource);
static u32 XSecure_SssCfg (XSecure_Sss *InstancePtr, XSecure_SssSrc Resource,
			XSecure_SssSrc InputSrc, XSecure_SssSrc OutputSrc);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief
 * This function takes the hardware core out of reset.
 *
 * @param	BaseAddress	Base address of the core.
 * @param	BaseAddress	Offset of the reset register.
 *
 * @return	None
 *
 *****************************************************************************/
void XSecure_ReleaseReset(u32 BaseAddress, u32 Offset)
{
	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_SET);
	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_UNSET);
}

/*****************************************************************************/
/**
 * @brief
 * This function places the hardware core into the reset.
 *
 * @param	BaseAddress	Base address of the core.
 * @param	BaseAddress	Offset of the reset register.
 *
 * @return	None
 *
 *****************************************************************************/
void XSecure_SetReset(u32 BaseAddress, u32 Offset)
{
	XSecure_WriteReg(BaseAddress, Offset, XSECURE_RESET_SET);
}

/*****************************************************************************/
/**
 * @brief
 * This function initializes the secure stream switch instance.
 *
 * @param	InstancePtr	Instance pointer to the XSecure_Sss.
 *
 *****************************************************************************/
void XSecure_SssInitialize (XSecure_Sss *InstancePtr)
{
	InstancePtr->Address = XSECURE_SSS_ADDRESS;

}

/*****************************************************************************/
/**
 * @brief
 * This function configures the secure stream switch for AES engine.
 *
 * @param	InstancePtr	Instance pointer to the XSecure_Sss
 * @param	InputSrc	Input DMA to be selected for AES engine.
 * @param	OutputSrc	Output DMA to be selected for AES engine.
 *
 * @return	- XST_SUCCESS - on successful configuration of the switch
 *
 * @note	InputSrc, OutputSrc are of type XSecure_SssSrc.
 *
 *****************************************************************************/
u32 XSecure_SssAes(XSecure_Sss *InstancePtr,
		XSecure_SssSrc InputSrc, XSecure_SssSrc OutputSrc)
{
	return XSecure_SssCfg(InstancePtr, XSECURE_SSS_AES,
			InputSrc, OutputSrc);
}

/*****************************************************************************/
/**
 * @brief
 * This function configures the secure stream switch for SHA hardware engine.
 *
 * @param	InstancePtr	Instance pointer to the XSecure_Sss
 * @param	DmaId		Device ID of DMA which is to be used as an
 *				input to the SHA engine.
 *
 * @return	- XST_SUCCESS - on successful configuration of the switch.
 *
 *****************************************************************************/
u32 XSecure_SssSha(XSecure_Sss *InstancePtr, u16 DmaId)
{
	XSecure_SssSrc InputSrc = (XSecure_SssSrc)0U;

	XSecure_SssDmaSrc(DmaId, &InputSrc);

	return XSecure_SssCfg(InstancePtr, XSECURE_SSS_SHA, InputSrc,
			XSECURE_SSS_INVALID);

}

/*****************************************************************************/
/**
 * @brief
 * This function configures secure stream switch to set DMA in loop back mode.
 *
 * @param	InstancePtr	Instance pointer to the XSecure_Sss
 * @param	DmaId		Device ID of DMA.
 *
 * @return	- XST_SUCCESS - on successful configuration of the switch.
 *
 *****************************************************************************/
u32 XSecure_SssDmaLoopBack(XSecure_Sss *InstancePtr, u16 DmaId)
{
	XSecure_SssSrc Resource = (XSecure_SssSrc)0U;

	XSecure_SssDmaSrc(DmaId, &Resource);

	return XSecure_SssCfg(InstancePtr, Resource, Resource,
				XSECURE_SSS_INVALID);
}

/*****************************************************************************/
/**
 * @brief
 * This function sets the DMA source of type XSecure_SssSrc based on the
 * provided DMA device ID.
 *
 * @param	InstancePtr	Instance pointer to the XSecure_Sss
 * @param	DmaId		Device ID of DMA.
 * @param	Resource	DMA source is updated into the pointer.
 *
 * @return	None.
 *
 *****************************************************************************/
static void XSecure_SssDmaSrc(u16 DmaId, XSecure_SssSrc *Resource)
{
	if (DmaId == 0U) {
		*Resource = XSECURE_SSS_DMA0;
	}
	else {
#ifdef XSECURE_VERSAL
		*Resource = XSECURE_SSS_DMA1;
#endif
	}

}

/*****************************************************************************/
/**
 * @brief
 * This function configures the secure stream switch.
 *
 * @param	InstancePtr	Instance pointer to the XSecure_Sss
 * @param	Resource	Resource for which input and output paths to be
 *				configured.
 * @param	InputSrc	Input source to be selected for the resource.
 * @param	OutputSrc	Output source to be selected for the resource.
 *
 * @return	- XST_SUCCESS - on successful configuration of the switch
 *
 * @note	Resource, InputSrc, OutputSrc are of type XSecure_SssSrc.
 *
 *****************************************************************************/
static u32 XSecure_SssCfg (XSecure_Sss *InstancePtr, XSecure_SssSrc Resource,
			XSecure_SssSrc InputSrc, XSecure_SssSrc OutputSrc)
{
	u32 InputSrcCfg = 0x00;
	u32 OutputSrcCfg = 0x00;
	u32 SssCfg = 0x00;

	/*
	 * Configure source of the input for given resource
	 * i.e Configure given InputSrc as a input for given Resource
	 */
	InputSrcCfg = (u32) XSecure_SssLookupTable [Resource][InputSrc] <<
		(XSECURE_SSS_CFG_LEN_IN_BITS * (u32)Resource);

	/*
	 * SSS allows configuring only input source for any Resources
	 * connected to it. So to define output source of given Resource,
	 *  configure given Resource as input to source mentioned by OutputSrc
	*/
	OutputSrcCfg = (u32) XSecure_SssLookupTable [OutputSrc][Resource] <<
			(XSECURE_SSS_CFG_LEN_IN_BITS * (u32)OutputSrc);

	SssCfg = InputSrcCfg | OutputSrcCfg;

	XSecure_Out32(InstancePtr->Address, SssCfg);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief
 * This function copies data from the specified source to destination.
 *
 *
 * @param	DestPtr		Pointer to the destination address.
 * @param	SrcPtr		Pointer to the source address.
 * @param	Size 		Data size to be copied.
 *
 * @return	Returns Status
 * 		- XST_SUCCESS on success
 * 		- XST_FAILURE on failure
 *
 *****************************************************************************/
void* XSecure_MemCpy(void * DestPtr, const void * SrcPtr, u32 Len)
{
	u8 *Dst = (u8 *)(UINTPTR)DestPtr;
	const u8 *Src = (const u8 *)SrcPtr;
	u32 Length = Len;

	/* Loop and copy.  */
	while (Length != 0U)
	{
		*Dst = *Src;
		Dst++;
		Src++;
		Length--;
	}

	return DestPtr;
}
