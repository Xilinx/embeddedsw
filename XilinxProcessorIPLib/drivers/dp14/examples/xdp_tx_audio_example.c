/*******************************************************************************
 *
 * Copyright (C) 2014 - 2016 Xilinx, Inc.  All rights reserved.
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
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdp_tx_audio_example.c
 *
 * Contains a design example using the XDp driver (operating in TX mode) to
 * train the main link and to display video. In this example application, the
 * sequence to enable audio is illustrated.
 *
 * @note	This example requires an audio source such as an S/PDIF instance
 *		to be part of the hardware system. See XAPP1178 for reference.
 * @note	This example requires that the audio enable configuration
 *		parameter for DisplayPort be turned on when creating the
 *		hardware design.
 * @note	For this example to output audio, the user will need to
 *		implement initialization of the system (Dptx_PlatformInit),
 *		configuration of the audio source (Dptx_ConfigureAudioSrc) and,
 *		depending on the hardware system, will need to implement sending
 *		of an info frame (Dptx_AudioSendInfoFrame). See XAPP1178 and the
 *		IP documentation for reference.
 * @note	For this example to display output, after training is complete,
 *		the user will need to implement configuration of the video
 *		stream source in order to provide the DisplayPort core with
 *		input (Dptx_StreamSrc* - called in xdp_tx_example_common.c). See
 *		XAPP1178 for reference.
 * @note	The functions Dptx_PlatformInit and Dptx_StreamSrc* are declared
 *		extern in xdp_tx_example_common.h and are left up to the user to
 *		implement. The functions Dptx_ConfigureAudioSrc and
 *		Dptx_AudioSendInfoFrame are present in this file and are also
 *		left for the user to implement.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.0   als  01/20/15 Initial creation.
 * 5.1   ms   01/23/17 Added xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings
 *                     are available in all examples. This is a fix for
 *                     CR-965028.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/

#include "xdp_tx_example_common.h"
#include "xil_printf.h"

/**************************** Function Prototypes *****************************/

u32 Dptx_AudioExample(XDp *InstancePtr, u16 DeviceId);
static void Dptx_AudioInit(XDp *InstancePtr);
static void Dptx_ConfigureAudioSrc(XDp *InstancePtr);
static void Dptx_AudioSendInfoFrame(XDp *InstancePtr);

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function is the main function of the XDp audio example.
 *
 * @param	None.
 *
 * @return
 *		- XST_SUCCESS if the audio example finished successfully.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
int main(void)
{
	u32 Status;

	/* Run the XDp audio example. */
	Status = Dptx_AudioExample(&DpInstance, DPTX_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("dp_tx_audio Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran dp_tx_audio Example\r\n");
	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * The main entry point for the audio example using the XDp driver. This
 * function will set up audio, initiate link training, and a video stream will
 * start being sent over the main link.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 * @param	DeviceId is the unique device ID of the DisplayPort TX core
 *		instance.
 *
 * @return
 *		- XST_SUCCESS if the system was set up correctly and link
 *		  training was successful.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 Dptx_AudioExample(XDp *InstancePtr, u16 DeviceId)
{
	u32 Status;

	/* Use single-stream transport (SST) mode for this example. Audio is
	 * not supported in multi-stream transport (MST) mode. */
	XDp_TxMstCfgModeDisable(InstancePtr);

	/* Do platform initialization here. This is hardware system specific -
	 * it is up to the user to implement this function. */
	Dptx_PlatformInit();
	/*******************/

	Status = Dptx_SetupExample(InstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Initialize DisplayPort audio. */
	Dptx_AudioInit(InstancePtr);

	XDp_TxEnableTrainAdaptive(InstancePtr, TRAIN_ADAPTIVE);
	XDp_TxSetHasRedriverInPath(InstancePtr, TRAIN_HAS_REDRIVER);

	/* A sink monitor must be connected at this point. See the polling or
	 * interrupt examples for how to wait for a connection event. */
	Status = Dptx_Run(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function will set up audio in the DisplayPort TX. The user will need
 * to implement configuration of the audio stream and, if needed, sending of
 * the info frame.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	The user needs to implement the Dptx_ConfigureAudioSrc and
 *		the Dptx_AudioSendInfoFrame functions to fulfill audio
 *		initialization.
 *
*******************************************************************************/
static void Dptx_AudioInit(XDp *InstancePtr)
{
	u32 Fs;
	u32 MAud;
	u32 NAud;
	u32 NumChs;

	/* Disable audio in the DisplayPort TX. This will also flush the buffers
	 * in the DisplayPort TX and set MUTE bit in VB-ID. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL, 0x0);

	/* Configure the audio source (the S/PDIF controller). It is up to the
	 * user to implement this function. */
	Dptx_ConfigureAudioSrc(InstancePtr);
	/*******************/

	/* Write audio info frame as per user requirements. This may be optional
	 * for some systems. 8 writes are required to register
	 * XDP_TX_AUDIO_INFO_DATA. It is up to the user to implement this
	 * function. */
	Dptx_AudioSendInfoFrame(InstancePtr);
	/*******************/

	Fs = 48; /* KHz (32 | 44.1 | 48) */
	if (InstancePtr->TxInstance.LinkConfig.LinkRate ==
						XDP_TX_LINK_BW_SET_540GBPS) {
		MAud = 512 * Fs;
	}
	else if (InstancePtr->TxInstance.LinkConfig.LinkRate ==
						XDP_TX_LINK_BW_SET_270GBPS) {
		MAud = 512 * Fs;
	}
	else if (InstancePtr->TxInstance.LinkConfig.LinkRate ==
						XDP_TX_LINK_BW_SET_162GBPS) {
		MAud = 512 * Fs;
	}

	/* Write the channel count. The value is (actual count - 1). */
	NumChs = 2;
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUDIO_CHANNELS,
								NumChs - 1);

	/* NAud = 540000 | 270000 | 162000 */
	NAud = 27 * InstancePtr->TxInstance.LinkConfig.LinkRate * 1000;

	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUDIO_MAUD, MAud);
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUDIO_NAUD, NAud);

	/* Enable audio in the DisplayPort TX. */
	XDp_WriteReg(InstancePtr->Config.BaseAddr, XDP_TX_AUDIO_CONTROL,
									0x1);
}

/******************************************************************************/
/**
 * This function needs to configure the audio source.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	The user needs to implement this. See XAPP1178 and the IP
 *		documentation for reference.
 *
*******************************************************************************/
static void Dptx_ConfigureAudioSrc(XDp *InstancePtr)
{
	xil_printf("Dptx_ConfigureAudioSrc: User defined function here.\n");
}

/******************************************************************************/
/**
 * This function needs to send an info frame as per user requirements.
 *
 * @param	InstancePtr is a pointer to the XDp instance.
 *
 * @return	None.
 *
 * @note	The user needs to implement this. See XAPP1178 and the IP
 *		documentation for reference.
 * @note	This may be optional for some systems.
 * @note	A sequence of 8 writes are required to register
 *		XDP_TX_AUDIO_INFO_DATA. See XAPP1178 and the IP documentation
 *		for reference.
 *
*******************************************************************************/
static void Dptx_AudioSendInfoFrame(XDp *InstancePtr)
{
	xil_printf("Dptx_AudioSendInfoFrame: User defined function here.\n");
}
