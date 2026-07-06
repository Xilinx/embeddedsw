/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaudioformatter_example.c
 *
 * This is main file for the Audio Formatter example design.
 *
 * This file contains an example for using the audio formatter with I2S receiver
 * and I2S transmitter device using interrupt mode. This example assumes that
 * the interrupt controller is also present as a part of the system. The audio
 * is received from external device through i2s receiver and given to audio
 * formatter through axi stream interface, audio formatter writes the output to
 * memory through DMA transfer from where another instance of audio formatter
 * reads it and send to i2s transmitter for playback. In this example we wait
 * for IOC interrupts from the audio formatter S2MM and MM2S cores and on
 * receiving both the interrupts along with i2srx and i2stx interrupts we
 * stop audio formatter and print the test is successful.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xil_cache.h"
#include "xparameters.h"
#include "xaudioformatter.h"
#include "xi2srx.h"
#include "xi2stx.h"
#include "xil_printf.h"
#include "xinterrupt_wrap.h"

/************************** Constant Definitions ******************************/

#define XAUDIO_FORMATTER_SW_VER "v1.00"
#define I2S_RX_FS		48 /* kHz */
#define I2S_RX_MCLK		(384 * I2S_RX_FS)
#define I2S_RX_TIME_OUT 500000
#define I2S_TX_FS		48 /* kHz */
#define I2S_TX_MCLK		(384*I2S_TX_FS)
#define I2S_TX_TIME_OUT 500000
#define AF_FS		48 /* kHz */
#define AF_MCLK		(384 * AF_FS)
#define AF_S2MM_TIMEOUT 0x80000000
/************************** Variable Definitions ******************************/
XI2s_Rx I2sRxInstance;		/* Instance of the I2S receiver device */
XAudioFormatter AFInstance;
XI2s_Tx I2sTxInstance;		/* Instance of the I2s Transmitter device */
u32 I2sTxIntrReceived;
u32 I2sRxIntrReceived;
u32 S2MMAFIntrReceived;
u32 MM2SAFIntrReceived;
XAudioFormatterHwParams af_hw_params = {0x20000000, 2, BIT_DEPTH_24, 8, 64};

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 **
 ** This function setups the interrupt system so interrupts can occur for the
 ** audio formatter core.
 **
 ** @return
 **	- XST_SUCCESS if interrupt setup was successful.
 **	- A specific error code defined in "xstatus.h" if an error
 **	occurs.
 **
 ** @note	This function assumes a Microblaze or ARM system and no
 **	operating system is used.
 **
*******************************************************************************/

/*****************************************************************************/
/**
* This function is the handler which performs processing for the I2S
* receiver.
* It is called from an interrupt context when the I2S receiver receives a
* AES Block Complete Interrupt.
*
* This handler provides an example of how to handle I2S receiver interrupts
* but is application specific.
*
* @param	CallBackRef is a pointer to the callback function
*
* @return	None.
*
* @note	None.
*
******************************************************************************/
void I2sRxAesBlockCmplIntrHandler(void *CallBackRef)
{
	XI2s_Rx *InstancePtr = (XI2s_Rx *)CallBackRef;
	/* Set the interrupt received flag. */
	I2sRxIntrReceived = 1;
}

/*****************************************************************************/
/**
* This function is the handler which performs processing for the I2S
* receiver.
* It is called from an interrupt context when the I2S receiver receives a
* Underflow Interrupt.
*
* This handler provides an example of how to handle I2S receiver interrupts
* but is application specific.
*
* @param	CallBackRef is a pointer to the callback function
*
* @return	None.
*
* @note	None.
*
******************************************************************************/
void I2sRxOvrflwIntrHandler(void *CallBackRef)
{
	xil_printf("\nI2sRxOvrflw interrupt received.\r\n");
	XI2s_Rx *InstancePtr = (XI2s_Rx *)CallBackRef;
	/* Set the interrupt received flag. */
	I2sRxIntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function is the handler which performs processing for the I2s
 * Transmitter.
 * It is called from an interrupt context when the I2s Transmitter receives a
 * AES Block Complete Interrupt.
 *
 * This handler provides an example of how to handle I2s Transmitter interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note	None.
 *
******************************************************************************/
void I2sTxAesBlockCmplIntrHandler(void *CallBackRef)
{
	XI2s_Tx *InstancePtr = (XI2s_Tx *)CallBackRef;
	/* Set the interrupt received flag. */
	I2sTxIntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function is the handler which performs processing for the I2s
 * Transmitter.
 * It is called from an interrupt context when the I2s Transmitter receives a
 * AES Block Error Interrupt.
 *
 * This handler provides an example of how to handle I2s Transmitter interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note	None.
 *
******************************************************************************/
void I2sTxAesBlockErrIntrHandler(void *CallBackRef)
{
	XI2s_Tx *InstancePtr = (XI2s_Tx *)CallBackRef;
	/* Set the interrupt received flag. */
	I2sTxIntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function is the handler which performs processing for the I2s
 * Transmitter.
 * It is called from an interrupt context when the I2s Transmitter receives a
 * AES Channel Status Updated Interrupt.
 *
 * This handler provides an example of how to handle I2s Transmitter interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note	None.
 *
******************************************************************************/
void I2sTxAesGetChStsHandler(void *CallBackRef)
{
	XI2s_Tx *InstancePtr = (XI2s_Tx *)CallBackRef;
	/* Set the interrupt received flag. */
	I2sTxIntrReceived = 1;
}

/*****************************************************************************/
/**
 * This function is the handler which performs processing for the I2s
 * Transmitter.
 * It is called from an interrupt context when the I2s Transmitter receives a
 * Underflow Interrupt.
 *
 * This handler provides an example of how to handle I2s Transmitter interrupts
 * but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note	None.
 *
******************************************************************************/
void I2sTxUnderflowIntrHandler(void *CallBackRef)
{
	XI2s_Tx *InstancePtr = (XI2s_Tx *)CallBackRef;
	/* Set the interrupt received flag. */
	I2sTxIntrReceived = 1;
}

/*****************************************************************************/
/**
 *
 * This function is called from the interrupt handler of audio formatter core.
 * After the first S2MM interrupt is received the interrupt_flag is set here.
 *
 * @return
 *
 * @note	This function assumes a Microblaze or ARM system and no
 *	operating system is used.
 *
*******************************************************************************/
void *XS2MMAFCallback(void *data)
{
	XAudioFormatter *AFInstancePtr = (XAudioFormatter *)data;
	/* clear interrupt flag */
	S2MMAFIntrReceived = 1;
	AFInstancePtr->ChannelId = XAudioFormatter_S2MM;
	XAudioFormatterDMAStop(&AFInstance);
}

/*****************************************************************************/
/**
 *
 * This function is called from the interrupt handler of audio formatter core.
 * After the first MM2S interrupt is received the interrupt_flag is set here.
 *
 * @return
 *
 * @note	This function assumes a Microblaze or ARM system and no
 *	operating system is used.
 *
*******************************************************************************/
void *XMM2SAFCallback(void *data)
{
	XAudioFormatter *AFInstancePtr = (XAudioFormatter *)data;
	/* clear interrupt flag */
	MM2SAFIntrReceived = 1;
	AFInstancePtr->ChannelId = XAudioFormatter_MM2S;
	XAudioFormatterDMAStop(&AFInstance);
}

/*****************************************************************************/
/**
 * This function does the lookup and initialization of the I2S transmitter.
 *
 * @param	I2sTxInstancePtr is a pointer to the I2sTx driver instance
 *
 * @return	XST_SUCCESS if the call is successful, otherwise XST_FAILURE.
 *
 * @note	None.
 *
******************************************************************************/
u32 InitializeI2sTx(XI2s_Tx *I2sTxInstancePtr)
{
	/*
	 * Lookup and Initialize the I2S transmitter so that it's ready to use.
	 */
	u32 Status;
	XI2stx_Config *I2STxConfig;
	I2STxConfig = XI2s_Tx_LookupConfig(XPAR_XI2STX_0_BASEADDR);
	if (I2STxConfig == NULL) {
		return XST_FAILURE;
	}

	Status = XI2s_Tx_CfgInitialize(I2sTxInstancePtr, I2STxConfig,
		I2STxConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	XI2s_Tx_SetHandler(I2sTxInstancePtr, XI2S_TX_HANDLER_AES_BLKCMPLT,
			&I2sTxAesBlockCmplIntrHandler,
			(void *)I2sTxInstancePtr);
	XI2s_Tx_SetHandler(I2sTxInstancePtr, XI2S_TX_HANDLER_AES_BLKSYNCERR,
			&I2sTxAesBlockErrIntrHandler,
			(void *)I2sTxInstancePtr);
	XI2s_Tx_SetHandler(I2sTxInstancePtr, XI2S_TX_HANDLER_AES_CHSTSUPD,
			&I2sTxAesGetChStsHandler,
			(void *)I2sTxInstancePtr);
	XI2s_Tx_SetHandler(I2sTxInstancePtr, XI2S_TX_HANDLER_AUD_UNDRFLW,
			&I2sTxUnderflowIntrHandler,
			(void *)I2sTxInstancePtr);
	XI2s_Tx_SetSclkOutDiv(I2sTxInstancePtr, I2S_TX_MCLK, I2S_TX_FS);
	XI2s_Tx_SetChMux(I2sTxInstancePtr, 0, XI2S_TX_CHMUX_AXIS_01);
	XI2s_Tx_IntrEnable(I2sTxInstancePtr, XI2S_TX_GINTR_EN_MASK);
	XI2s_Tx_IntrEnable(I2sTxInstancePtr, XI2S_TX_INTR_AES_BLKCMPLT_MASK);
	XI2s_Tx_IntrEnable(I2sTxInstancePtr, XI2S_TX_INTR_AUDUNDRFLW_MASK);
	XI2s_Tx_Enable(I2sTxInstancePtr, 0x1);
	Status = XSetupInterruptSystem(I2sTxInstancePtr, &XI2s_Tx_IntrHandler,
				       I2sTxInstancePtr->Config.IntrId,
				       I2sTxInstancePtr->Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);

	if (Status == XST_FAILURE) {
		xil_printf("IRQ init failed.\n\r\r");
		return XST_FAILURE;
	}
}

/*****************************************************************************/
/**
 * This function does the lookup and initialization of the I2S receiver.
 *
 * @param	I2sRxInstancePtr is a pointer to the I2sRx driver instance
 *
 * @return	XST_SUCCESS if the call is successful, otherwise XST_FAILURE.
 *
 * @note	None.
 *
******************************************************************************/
u32 InitializeI2sRx(XI2s_Rx *I2sRxInstancePtr)
{
	/*
	 * Lookup and Initialize the I2S receiver so that it's ready to use.
	 */
	u32 Status;
	XI2srx_Config *I2SRxConfig;
	I2SRxConfig = XI2s_Rx_LookupConfig(XPAR_XI2SRX_0_BASEADDR);
	if (I2SRxConfig == NULL)
		return XST_FAILURE;

	Status = XI2s_Rx_CfgInitialize(&I2sRxInstance, I2SRxConfig,
			I2SRxConfig->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
	XI2s_Rx_SetHandler(I2sRxInstancePtr, XI2S_RX_HANDLER_AES_BLKCMPLT,
			&I2sRxAesBlockCmplIntrHandler,
			(void *)I2sRxInstancePtr);
	XI2s_Rx_SetHandler(I2sRxInstancePtr, XI2S_RX_HANDLER_AUD_OVRFLW,
			&I2sRxOvrflwIntrHandler, (void *)I2sRxInstancePtr);
	XI2s_Rx_SetSclkOutDiv(I2sRxInstancePtr, I2S_RX_MCLK, I2S_RX_FS);
	XI2s_Rx_SetChMux(I2sRxInstancePtr, 0x0, XI2S_RX_CHMUX_XI2S_01);
	XI2s_Rx_IntrEnable(I2sRxInstancePtr, XI2S_RX_GINTR_EN_MASK);
	XI2s_Rx_IntrEnable(I2sRxInstancePtr, XI2S_RX_INTR_AES_BLKCMPLT_MASK);
	XI2s_Rx_IntrEnable(I2sRxInstancePtr, XI2S_RX_INTR_AUDOVRFLW_MASK);
	XI2s_Rx_Enable(I2sRxInstancePtr, TRUE);
	Status = XSetupInterruptSystem(I2sRxInstancePtr, &XI2s_Rx_IntrHandler,
				       I2sRxInstancePtr->Config.IntrId,
				       I2sRxInstancePtr->Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);

	if (Status == XST_FAILURE) {
		xil_printf("IRQ init failed.\n\r\r");
		return XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************/
/**
 * This function does the lookup and initialization of the audio formatter.
 *
 * @param	AFInstancePtr is a pointer to audio formatter driver instance
 *
 * @return	XST_SUCCESS if the call is successful, otherwise XST_FAILURE.
 *
 * @note	None.
 *
******************************************************************************/
u32 InitializeAudioFormatter(XAudioFormatter *AFInstancePtr)
{
	u32 Status;
	u32 offset;
	XAudioFormatter_Config *AFConfig;
	/*
	 * Lookup and Initialize the audio formatter so that it's ready to use.
	 */
	Status = XAudioFormatter_Initialize(&AFInstance, XPAR_XAUDIO_FORMATTER_0_BASEADDR);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	if (AFInstancePtr->s2mm_presence == 1) {
		AFInstancePtr->ChannelId = XAudioFormatter_S2MM;
		XAudioFormatter_SetS2MMCallback(AFInstancePtr,
			XAudioFormatter_IOC_Handler, XS2MMAFCallback,
			(void *)AFInstancePtr);
		XAudioFormatter_InterruptEnable(AFInstancePtr,
			XAUD_CTRL_IOC_IRQ_MASK);
		XAudioFormatterSetS2MMTimeOut(AFInstancePtr, AF_S2MM_TIMEOUT);
		XAudioFormatterSetHwParams(AFInstancePtr, &af_hw_params);
		XAudioFormatterDMAStart(AFInstancePtr);
		Status = XSetupInterruptSystem(AFInstancePtr,
					       &XAudioFormatterS2MMIntrHandler,
					       AFInstancePtr->Config.IntrId,
					       AFInstancePtr->Config.IntrParent,
					       XINTERRUPT_DEFAULT_PRIORITY);
		if (Status == XST_FAILURE) {
			xil_printf("IRQ init failed.\n\r\r");
			return XST_FAILURE;
		}
	}
	if (AFInstancePtr->mm2s_presence == 1) {
		AFInstancePtr->ChannelId = XAudioFormatter_MM2S;
		XAudioFormatter_SetMM2SCallback(AFInstancePtr,
			XAudioFormatter_IOC_Handler, XMM2SAFCallback,
			(void *)AFInstancePtr);
		XAudioFormatter_InterruptEnable(AFInstancePtr,
			XAUD_CTRL_IOC_IRQ_MASK);
		XAudioFormatterSetFsMultiplier(AFInstancePtr, AF_MCLK, AF_FS);
		XAudioFormatterSetHwParams(AFInstancePtr, &af_hw_params);
		XAudioFormatterDMAStart(AFInstancePtr);
		Status = XSetupInterruptSystem(AFInstancePtr,
					       &XAudioFormatterS2MMIntrHandler,
					       AFInstancePtr->Config.IntrId,
					       AFInstancePtr->Config.IntrParent,
					       XINTERRUPT_DEFAULT_PRIORITY);
		if (Status == XST_FAILURE) {
			xil_printf("IRQ init failed.\n\r\r");
			return XST_FAILURE;
		}
	}
	return Status;
}

/*****************************************************************************/
/**
 * This function is the main function of the audio formatter example
 * using Interrupts.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate a
 *		Failure.
 *
 * @note	None.
 *
******************************************************************************/
int main(void)
{
	u32 Status;
	u32 RxStatus;
	u32 TxStatus;
	u32 IntrCount = 0;
	XI2s_Rx *I2sRxInstancePtr = &I2sRxInstance;
	XI2s_Tx *I2sTxInstancePtr = &I2sTxInstance;
	XAudioFormatter *AFInstancePtr = &AFInstance;

	xil_printf("\r\n-----------------------------------------------\r\n");
	xil_printf(" Xilinx Audio Formatter Example Design %s\r\n",
		XAUDIO_FORMATTER_SW_VER);
	xil_printf("	(c) 2018 by Xilinx Inc.\r\n");

	/* Initialize ICache */
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();

	/* Initialize DCache */
	Xil_DCacheInvalidate();
	Xil_DCacheEnable();

	/* Initialize IRQ */
	Status = InitializeAudioFormatter(AFInstancePtr);
	if (Status == XST_FAILURE) {
		xil_printf("\nAudio Formatter Init failed\n");
		return XST_FAILURE;
	}
	Status = InitializeI2sRx(I2sRxInstancePtr);
	if (Status == XST_FAILURE) {
		xil_printf("\nI2SRx Init failed\n");
		return XST_FAILURE;
	}
	Status = InitializeI2sTx(I2sTxInstancePtr);
	if (Status == XST_FAILURE) {
		xil_printf("\nI2STx Init failed\n");
		return XST_FAILURE;
	}
	RxStatus =  XST_FAILURE;
	while (IntrCount < I2S_RX_TIME_OUT) {
		if (I2sRxIntrReceived == 1 && S2MMAFIntrReceived == 1) {
			RxStatus =  XST_SUCCESS;
			break;
		}
		IntrCount++;
	}
	IntrCount = 0;
	TxStatus =  XST_FAILURE;
	while (IntrCount < I2S_TX_TIME_OUT) {
		if (I2sTxIntrReceived == 1 && MM2SAFIntrReceived == 1) {
			TxStatus =  XST_SUCCESS;
			break;
		}
		IntrCount++;
	}
	if (RxStatus == XST_SUCCESS && TxStatus == XST_SUCCESS)
		xil_printf("\nAudio Formatter Test successful\n");
	else
		xil_printf("\nAudio Formatter Test Failed\n");
	return Status;
}
