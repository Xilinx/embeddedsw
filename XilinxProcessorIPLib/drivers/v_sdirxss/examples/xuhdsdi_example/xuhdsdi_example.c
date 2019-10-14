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
 *****************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xsdi_example.c
 *
 * This file demonstrates how to use Xilinx SDI Subsystem for passthrough mode
 * on ZCU106 board. It takes SDI data input through SDI Rx Subsystem and passes
 * the same data as SDI Data out through SDI Tx Subsystem.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date       Changes
 * ---- ---- ---------- --------------------------------------------------
 * 1.0  PG   05/09/2017 Initial version
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "sleep.h"
#include "xiic.h"
#include "si5324drv.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_exception.h"
#include "xv_sdivid.h"
#include "xvidc.h"
#include "xgpio.h"
#include "xsdi_menu.h"
#include "xv_sdirxss.h"
#include "xv_sditxss.h"
#include "xscugic.h"
#ifdef XPAR_XSDIAUD_NUM_INSTANCES
#include "xspdif.h"
#include "xsdiaud_hw.h"
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
else
#include "xscugic.h"
#endif

/************************** Constant Definitions *****************************/
#define GPIO_0_TX_RST XPAR_GPIO_0_BASEADDR
#define GPIO_1_RX_RST XPAR_GPIO_1_BASEADDR

#ifdef XPAR_XSDIAUD_NUM_INSTANCES
#define SDI_AUD_EMBED   XPAR_V_UHDSDI_AUDIO_EMBED_1_BASEADDR
#define SDI_AUD_EXTRT   XPAR_V_UHDSDI_AUDIO_EXTRACT_0_BASEADDR
#endif

typedef u8 AddressType;
#define PAGE_SIZE   16

#define I2C_MUX_ADDR	0x74  /**< I2C Mux Address */
#define I2C_CLK_ADDR	0x69  /**< I2C Clk Address */
#define I2C_CLK_ADDR_570	0x5D  /**< I2C Clk Address for Si570*/

#define SDI_TX_SS_INTR_ID XPAR_FABRIC_V_SDITXSS_0_VEC_ID
#define SDI_RX_SS_INTR_ID XPAR_FABRIC_V_SDIRXSS_0_VEC_ID

#define IIC_INTR_ID XPAR_FABRIC_AXI_IIC_0_IIC2INTC_IRPT_INTR

#define FREQ_148_5_MHz	(148500000)
#define FREQ_297_MHz	(297000000)
#define FREQ_74_25_MHz	(74250000)
#define FREQ_27_MHz	(27000000)
#define FREQ_148_35_MHz	(148350000)

#ifdef XPAR_XSDIAUD_NUM_INSTANCES
#define SPDIF_0_DEVICE_ID XPAR_XSPDIF_0_DEVICE_ID
#define SPDIF_1_DEVICE_ID XPAR_XSPDIF_1_DEVICE_ID

#define SPDIF_0_INTERRUPT_ID XPAR_FABRIC_SPDIF_0_VEC_ID
#define SPDIF_1_INTERRUPT_ID XPAR_FABRIC_SPDIF_1_VEC_ID

#define SDIAUD_0_DEVICE_ID	XPAR_XSDIAUD_0_DEVICE_ID
#define SDIAUD_1_DEVICE_ID	XPAR_XSDIAUD_1_DEVICE_ID

#define SDIAUD_0_INTERRUPT_ID	XPAR_FABRIC_SDIAUD_0_VEC_ID
#define SDIAUD_1_INTERRUPT_ID	XPAR_FABRIC_SDIAUD_1_VEC_ID

#define XPAR_SDIAUD_0_BA	XPAR_V_UHDSDI_AUDIO_EXTRACT_0_BASEADDR
#define XPAR_SDIAUD_1_BA	XPAR_V_UHDSDI_AUDIO_EMBED_1_BASEADDR
#define XSDIAUD_QUAD_GROUP	4
#define ACR_BASEADDR XPAR_AES_TX_HIER_ACR_BASEADDR
#define AXIS_SWITCH_RX XPAR_AXIS_SWITCH_0_BASEADDR
#define AXIS_SWITCH_TX XPAR_AXIS_SWITCH_1_BASEADDR
#endif

/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/
void ClearScreen(void);
static int SetupInterruptSystem(void);
void GtReadyCallback(void *CallbackRef);
void RxStreamUpCallback(void *CallbackRef);
void RxStreamUp(void);
void RxStreamDownCallback(void *CallbackRef);
void Xil_AssertCallbackRoutine(u8 *File, s32 Line);
void StartTxAfterRx(void);
static int I2cMux(void);
static int I2cClk(u32 InFreq, u32 OutFreq);
static int I2cClk_SI5319(u32 InFreq, u32 OutFreq);
int Si570_SetClock(u32 IICBaseAddress, u8 IICAddress1, u32 RxRefClk);
void Info(void);
void DebugInfo(void);
#ifdef XPAR_XSDIAUD_NUM_INSTANCES
void SdiAud_Ext_IntrHandler(XSdiAud *InstancePtr);
void ConfigNandCTS(u8 IsFractional, XSdiVid_TransMode RxTransMode,
		XSdiAud_SampRate SampleRate);
#endif

/*
 * Write buffer for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];

u8 ReadBuffer[PAGE_SIZE];	/* Read buffer for reading a page. */

volatile u8 TransmitComplete;	/* Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/* Flag to check completion of Reception */

unsigned int IntrReceived;
u8 AudioStatusUpdate;

/************************** Variable Definitions *****************************/

static XScuGic Intc;

XGpio Gpio_AxisFifo_resetn;
XGpio_Config *Gpio_AxisFifo_resetn_ConfigPtr;

XGpio Gpio_si5324;
XGpio_Config *Gpio_si5324_ConfigPtr;

XV_SdiTxSs SdiTxSs;       /* SDI TX SS structure */
XV_SdiTxSs_Config *XV_SdiTxSs_ConfigPtr;

XV_SdiRxSs SdiRxSs;       /* SDI RX SS structure */
XV_SdiRxSs_Config *XV_SdiRxSs_ConfigPtr;

#ifdef XPAR_XSDIAUD_NUM_INSTANCES
XSpdif SpdifRx;   		/* Instance of the SPDIF RX device */
XSpdif SpdifTx;   		/* Instance of the SPDIF TX device */
#endif

#define UART_BASEADDR XPAR_XUARTPS_0_BASEADDR

u8 StartTxAfterRxFlag;
XSdi_Menu SdiMenu;			/**< Menu structure */
u32 Index, MaxIndex;
u8 PayloadStatus;


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function Clears Uart terminal screen.
 *
 * @return	None
 *
 * @note	None.
 *
 ******************************************************************************/
void ClearScreen(void)
{
	xil_printf("%c\[2J", 27);	/**< Clear Sreen */
	xil_printf("%c\033[0;0H", 27);	/**< Bring Cursor to 0,0 */
}

/*****************************************************************************/
/**
 *
 * This function setups the interrupt system so interrupts can occur for the
 * SDI cores. The function is application-specific since the actual system
 * may or may not have an interrupt controller. The SDI cores could be
 * directly connected to a processor without an interrupt controller.
 * The user should modify this function to fit the application.
 *
 * @return
 *		- XST_SUCCESS if interrupt setup was successful.
 *		- A specific error code defined in "xstatus.h" if an error
 *		occurs.
 *
 * @note	This function assumes a Microblaze or ARM system and no operating
 *		system is used.
 *
 ******************************************************************************/
static int SetupInterruptSystem(void)
{
	int Status;
	XScuGic *IntcInstPtr = &Intc;

	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
	XScuGic_Config *IntcCfgPtr;
	IntcCfgPtr = XScuGic_LookupConfig(XPAR_PSU_ACPU_GIC_DEVICE_ID);
	if (!IntcCfgPtr) {
		xil_printf("ERR:: Interrupt Controller not found");
		return XST_DEVICE_NOT_FOUND;
	}
	Status = XScuGic_CfgInitialize(IntcInstPtr,
					IntcCfgPtr,
					IntcCfgPtr->CpuBaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed!\r\n");
		return XST_FAILURE;
	}

	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				(XScuGic *)IntcInstPtr);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function setup SI5324 clock generator over IIC.
 *
 * @return	The number of bytes sent.
 *
 * @note	None.
 *
 ******************************************************************************/
static int I2cMux(void)
{
	u8 Buffer;
	int Status;

	xil_printf("Set i2c mux... ");

	Buffer = 0x18;
	Status = XIic_Send((XPAR_IIC_0_BASEADDR),
				(I2C_MUX_ADDR),
				(u8 *)&Buffer,
				1,
				(XIIC_STOP));
	xil_printf("done\n\r");

	return Status;
}

/*****************************************************************************/
/**
 *
 * This function setup SI5324 clock generator either in free or locked mode.
 *
 * @param	InFreq specifies an input frequency for the si5324.
 * @param	OutFreq specifies the output frequency of si5324.
 *
 * @return	'XST_FAILURE' if error in programming external clock
 *			else 'XST_SUCCESS' if success
 *
 * @note	None.
 *
 ******************************************************************************/
static int I2cClk(u32 InFreq, u32 OutFreq)
{
	int Status;

	/* Free running mode */
	if (!InFreq) {

		Status = Si5324_SetClock((XPAR_IIC_0_BASEADDR),
					(I2C_CLK_ADDR),
					(SI5324_CLKSRC_XTAL),
					(SI5324_XTAL_FREQ),
					OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming SI5324\n\r");
			return XST_FAILURE;
		}
	}

	/* Locked mode */
	else {
		Status = Si5324_SetClock((XPAR_IIC_0_BASEADDR),
					(I2C_CLK_ADDR),
					(SI5324_CLKSRC_CLK1),
					InFreq,
					OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming SI5324\n\r");
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function setup SI5319 clock generator either in free or locked mode.
 *
 * @param	InFreq specifies an input frequency for the si5319.
 * @param	OutFreq specifies the output frequency of si5319.
 *
 * @return	'XST_FAILURE' if error in programming external clock
 *			else 'XST_SUCCESS' if success
 *
 * @note	None.
 *
 ******************************************************************************/
static int I2cClk_SI5319(u32 InFreq, u32 OutFreq)
{
	int Status;

	/* Free running mode */
	if (!InFreq) {

		Status = Si5324_SetClock((0x800E0000),
					(0x68),
					(SI5324_CLKSRC_XTAL),
					(SI5324_XTAL_FREQ),
					OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming SI5319\n\r");
			return XST_FAILURE;
		} else {
			print("Success programming SI5319\n\r");
		}
	}

	/* Locked mode */
	else {
		Status = Si5324_SetClock((0x800E0000),
					(0x68),
					(SI5324_CLKSRC_CLK1),
					InFreq,
					OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming SI5319\n\r");
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function is called when the GT reset is done and it starts SDI Tx
 * Subsystem.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void GtReadyCallback(void *CallbackRef)
{
	/* Disable AXISFIFO */
	XGpio_DiscreteWrite(&Gpio_AxisFifo_resetn, 1, 0);

	/* Start SDI TX Subsystem */
	XV_SdiTxSs_StreamStart(&SdiTxSs);

	/* Enable AXIS FIFO */
	XGpio_DiscreteWrite(&Gpio_AxisFifo_resetn, 1, 1);

	/* Print stream information */
	XV_SdiTxSs_ReportStreamInfo(&SdiTxSs);
}

/*****************************************************************************/
/**
 *
 * This function is called when the RX Lock occurs, it calculates the frequency
 * wrt RxTransMode, programs SI5324 with the calculated frequency and then
 * enables Rx stream flow IPs.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void RxStreamUpCallback(void *CallbackRef)
{
	u32 Si5324_ClkIn;
	XSdiVid_TransMode RxTransMode;
	u8 IsFractional;
	XVidC_VideoStream *SdiRxSsVidStreamPtr;

	SdiRxSsVidStreamPtr = XV_SdiRxSs_GetVideoStream(&SdiRxSs, 0);

	IsFractional = XV_SdiRxSs_GetTransportBitRate(&SdiRxSs);

	RxTransMode =  XV_SdiRxSs_GetTransportMode(&SdiRxSs);


	switch (RxTransMode) {
		case XSDIVID_MODE_SD:
			if (IsFractional == 0) {
				Si5324_ClkIn = FREQ_27_MHz;

			} else if (IsFractional == 1) {
				Si5324_ClkIn = FREQ_27_MHz;
			}
			break;

		case XSDIVID_MODE_HD:
			if (IsFractional == 0) {
				Si5324_ClkIn = FREQ_74_25_MHz;
			} else if (IsFractional == 1) {
				Si5324_ClkIn = FREQ_74_25_MHz/1.001;
			}
			break;

		case XSDIVID_MODE_3GA:
		case XSDIVID_MODE_3GB:
		case XSDIVID_MODE_6G:
			if (IsFractional == 0) {
				Si5324_ClkIn = FREQ_148_5_MHz;
			} else if (IsFractional == 1) {
				Si5324_ClkIn = FREQ_148_5_MHz/1.001;
			}
			break;

		case XSDIVID_MODE_12G:
			if (IsFractional == 0) {
				Si5324_ClkIn = FREQ_297_MHz;
			} else if (IsFractional == 1) {
				Si5324_ClkIn = FREQ_297_MHz/1.001;
			}
			break;

		default:
			Si5324_ClkIn = FREQ_148_5_MHz;
			break;
	}

	xil_printf("INFO>> SDI Rx: Input Locked\r\n");

	/* The output clock frequency is set to 148.5MHz because the QPLL1's
	 * reference clock is set as 148.5MHz.
	 */
	if (IsFractional == 0) {
	I2cClk(Si5324_ClkIn, FREQ_148_5_MHz);
	} else if (IsFractional == 1) {
		I2cClk(Si5324_ClkIn, FREQ_148_5_MHz/1.001);
	}

	/* Waiting for SI5328 Lock */
	usleep(1000000);

	/* QPLL1 reset */
	Xil_Out32((UINTPTR)(GPIO_0_TX_RST), (u32)(0x00000001));
	usleep(1);
	Xil_Out32((UINTPTR)(GPIO_0_TX_RST), (u32)(0x00000000));

	StartTxAfterRxFlag = (TRUE);
	XV_SdiRxSs_StreamFlowEnable(&SdiRxSs);

	#ifdef XPAR_XSDIAUD_NUM_INSTANCES
	/* Enable SDI Audio Pass-Through or AES Audio Playback (from this ISR) */
	XSdiAud_IntrEnable(&SdiExtract, XSDIAUD_INT_EN_AUD_STAT_UPDATE_MASK);

	/* Enable AES Capture */
	if (XSDIAudioMode == XSDI_AES_CAPTURE_PLAYBACK)
	{
		XSdiEnableAESAudioCapture();
	}
	#endif
}

/*****************************************************************************/
/**
 *
 * This function is called when the RX looses lock, and it stops SDI Tx
 * Subsystem.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void RxStreamDownCallback(void *CallbackRef)
{
	xil_printf("INFO>> SDI Rx: Lock Lost\r\n");
	XV_SdiRxSs_ClearYCbCr444_RGB_10bit(&SdiRxSs);
	XV_SdiTxSs_ClearYCbCr444_RGB_10bit(&SdiTxSs);

	XV_SdiTxSs_Stop(&SdiTxSs);

#ifndef XPAR_XSDIAUD_NUM_INSTANCES
	XSdiVid_TransMode RxTransMode;
	RxTransMode =  XV_SdiRxSs_GetTransportMode(&SdiRxSs);
	if(RxTransMode == XSDIVID_MODE_12G)
	usleep(8000);
#endif

#ifdef XPAR_XSDIAUD_NUM_INSTANCES
	/* Disable Audio */
	if (XSDIAudioMode == XSDI_SDI_PASS_THROUGH) {
		XSdiDisableSDIAudioPassThrough();
	} else if (XSDIAudioMode == XSDI_AES_CAPTURE_PLAYBACK) {
		XSdiDisableAESAudioCapture();
		XSdiDisableAESAudioPlayback();
	}
#endif
}

/*****************************************************************************/
/**
 *
 * This function is called when the any exception occurs.
 *
 * @param	File is pointer to file name.
 * @param	Line is line number of the function caller.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void Xil_AssertCallbackRoutine(u8 *File, s32 Line)
{
	xil_printf("Assertion in File %s, on line %0d\n\r", File, Line);
}

/*****************************************************************************/
/**
 *
 * This function outputs the video information on TX and RX.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void Info(void)
{
	xil_printf("\n\r-----\n\r");
	xil_printf("Info\n\r");
	xil_printf("-----\n\r\n\r");

	XV_SdiTxSs_ReportInfo(&SdiTxSs);
	XV_SdiRxSs_ReportInfo(&SdiRxSs);
}

/*****************************************************************************/
/**
 *
 * This function outputs the debug information.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void DebugInfo(void)
{
	u32 VtcClkStable;
	xil_printf("\n\r-----\n\r");
	xil_printf("Info\n\r");
	xil_printf("-----\n\r\n\r");

	/* Read the current LOL status from SI5324 */
	VtcClkStable = XGpio_DiscreteRead(&Gpio_si5324, 1) ? 0 : 1;


	/* Reports Tx Debug information.
	 * A known issue on the VTC which requires both axilite and video clock to
	 * be stable. As a workaround, the clock's status is passed over to prevent
	 * accesses of the VTC when the clock is unstable.
	 * Inverts the LOL as a clock stable flag so VTC outputs info only when the
	 * clock is stable.
	 */
	XV_SdiTxSs_ReportDebugInfo(&SdiTxSs, VtcClkStable);

	/* Reports Rx Debug information */
	XV_SdiRxSs_ReportDebugInfo(&SdiRxSs);
}

/*****************************************************************************/
/**
 *
 * This function is called to start the TX stream after the RX stream
 * was up and running.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void StartTxAfterRx(void)
{
	/* clear flag */
	StartTxAfterRxFlag = (FALSE);
	XVidC_VideoStream *SdiRxSsVidStreamPtr;
	XVidC_VideoStream *SdiTxSsVidStreamPtr;
	XSdiVid_Transport *SdiRxSsTransportPtr;
	XSdiVid_Transport *SdiTxSsTransportPtr;
	u32 *SdiRxSsPayloadIdPtr;
	u32 *SdiTxSsPayloadIdPtr;

	SdiRxSsVidStreamPtr = XV_SdiRxSs_GetVideoStream(&SdiRxSs, 0);

	if (SdiRxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_444 ||
		    SdiRxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_RGB ||
		    SdiRxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_MEM_YUVX10) {
		XV_SdiRxSs_SetYCbCr444_RGB_10bit(&SdiRxSs);
		XV_SdiTxSs_SetYCbCr444_RGB_10bit(&SdiTxSs);
	} else if (SdiRxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCBCR_422){
		XV_SdiRxSs_ClearYCbCr444_RGB_10bit(&SdiRxSs);
		XV_SdiTxSs_ClearYCbCr444_RGB_10bit(&SdiTxSs);
	}

	SdiTxSsVidStreamPtr = XV_SdiTxSs_GetVideoStream(&SdiTxSs, 0);
	*SdiTxSsVidStreamPtr = *SdiRxSsVidStreamPtr;

	SdiRxSsTransportPtr = XV_SdiRxSs_GetTransport(&SdiRxSs);
	SdiTxSsTransportPtr = XV_SdiTxSs_GetTransport(&SdiTxSs);
	*SdiTxSsTransportPtr = *SdiRxSsTransportPtr;

	for (int StreamId = 0; StreamId < 8; StreamId++) {
		SdiRxSsPayloadIdPtr = XV_SdiRxSs_GetPayloadId(&SdiRxSs, StreamId);
		SdiTxSsPayloadIdPtr = XV_SdiTxSs_GetPayloadId(&SdiTxSs, StreamId);
		*SdiTxSsPayloadIdPtr = *SdiRxSsPayloadIdPtr;
	}

	XV_SdiTxSs_StreamConfig(&SdiTxSs);
}

#ifdef XPAR_XSDIAUD_NUM_INSTANCES


/*****************************************************************************/
/**
 *
 * This function disables the SDI Audio Pass-Through Path
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XSdiDisableSDIAudioPassThrough(void)
{
	/* Disable SDI Audio Extract */
	XSdiAud_Enable(&SdiExtract, 0);

	/* Clear all interrupts enables & pending interrupt status */
	XSdiAud_ConfigReset(&SdiExtract);

	/* Disable SDI Audio Embed */
	XSdiAud_Enable(&SdiEmbed, 0);

	/* Clear all interrupts enables & pending interrupt status */
	XSdiAud_ConfigReset(&SdiEmbed);
}

/*****************************************************************************/
/**
 *
 * This function disables the AES Audio Playback path
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XSdiDisableAESAudioPlayback(void)
{
	/* Disable SDI Audio Extract */
	XSdiAud_Enable(&SdiExtract, 0);

	/* Clear all interrupts enables & pending interrupt status */
	XSdiAud_ConfigReset(&SdiExtract);

	/* Disable ACR */
	Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x8), (u32)(0x0));

	/* Disable SPDIF TX */
	XSpdif_Enable(&SpdifTx, 0x0);
}

/*****************************************************************************/
/**
 *
 * This function disables the AES Audio Capture path
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XSdiDisableAESAudioCapture(void)
{
	/* Disable SPDIF RX */
	XSpdif_Enable(&SpdifRx, 0x0);

	/* Disable SDI Audio Embed */
	XSdiAud_Enable(&SdiEmbed, 0);

	/* Clear all interrupt enables & pending interrupt status */
	XSdiAud_ConfigReset(&SdiEmbed);
}

/*****************************************************************************/
/**
 *
 * This function enables the SDI Audio Pass-Through Path (Audio extracted
 * from SDI RX is embedded back on to SDI TX)
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XSdiEnableSDIAudioPassThrough(XSdiAud_SampRate SampleRate, u32 Asx)
{
	XSdiVid_Transport *RxTransport;
	XSdiAud_Emb_Vid_Props AudEmbVidProps;

	/* Enable SDI Audio Extract to Audio Buffer path */
	Xil_Out32((UINTPTR)(AXIS_SWITCH_RX + 0x40), (u32)(0x80000000));
	Xil_Out32((UINTPTR)(AXIS_SWITCH_RX + 0x44), (u32)(0x0));
	Xil_Out32((UINTPTR)(AXIS_SWITCH_RX), (u32)(0x2));

	/* Enable Audio Buffer to SDI Audio Embed path */
	Xil_Out32((UINTPTR)(AXIS_SWITCH_TX + 0x40), (u32)(0x1));
	Xil_Out32((UINTPTR)(AXIS_SWITCH_TX), (u32)(0x2));

	/* Enable 32-Channel SDI Audio Embed */
	RxTransport = XV_SdiRxSs_GetTransport(&SdiRxSs);
	AudEmbVidProps.XSdiAud_TFamily = RxTransport->TFamily;
	AudEmbVidProps.XSdiAud_TRate = RxTransport->TRate;
	AudEmbVidProps.XSdiAud_TScan = RxTransport->TScan;
	XSdiAud_Emb_SetVidProps(&SdiEmbed, &AudEmbVidProps);
	XSdiAud_Emb_SetAsx(&SdiEmbed, Asx);
	XSdiAud_Emb_SetSmpSize(&SdiEmbed, 1);
	XSdiAud_Emb_SetSmpRate(&SdiEmbed, SampleRate);
	XSdiAud_SetCh(&SdiEmbed, 0xFFFFFFFF);
	XSdiAud_Enable(&SdiEmbed, 1);

	/* Enable 32-Channel SDI Audio Extract */
	XSdiAud_SetCh(&SdiExtract, 0xFFFFFFFF);
	XSdiAud_Enable(&SdiExtract, 1);
}


/*****************************************************************************/
/**
 *
 * This function enables the AES Audio Playback Path (Audio extracted from SDI RX
 *  is played back on SPDIF TX)
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XSdiEnableAESAudioPlayback(XSdiAud_SampRate SampleRate)
{
	XSdiVid_TransMode RxTransMode;
	u8 IsFractional;

	/* Enable SDI Audio Extract to SPDIF TX path */
	Xil_Out32((UINTPTR)(AXIS_SWITCH_RX + 0x40), (u32)(0x0));
	Xil_Out32((UINTPTR)(AXIS_SWITCH_RX + 0x44), (u32)(0x80000000));
	Xil_Out32((UINTPTR)(AXIS_SWITCH_RX), (u32)(0x2));

	/* Configure ACR to generate FS rate clock */
	RxTransMode =  XV_SdiRxSs_GetTransportMode(&SdiRxSs);
	IsFractional = XV_SdiRxSs_GetTransportBitRate(&SdiRxSs);
	ConfigNandCTS(IsFractional, RxTransMode, SampleRate);

	Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x30), (u32)(0x00000100));
	Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x34), (u32)(0x0000003C));
	Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x38), (u32)(0x00000014));
	Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x3C), (u32)(0x00000C00));
	Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x40), (u32)(0x00000006));
	Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x20), (u32)(0x00000001));
	Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x70), (u32)(0x00000020));
	Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x8),  (u32)(0x00000001));

	/* Wait for clock generated by ACR to be stable */
	sleep(1);

	/* Program SI5319 to generate FS*512 clock (AES TX Clock) */
	if (SampleRate == XSDIAUD_SMPLRATE_48) {
		I2cClk_SI5319(48000, 48000*512);
	} else if (SampleRate == XSDIAUD_SMPLRATE_44) {
		I2cClk_SI5319(44100, 44100*512);
	} else if (SampleRate == XSDIAUD_SMPLRATE_32) {
		I2cClk_SI5319(32000, 32000*512);
	}

	/* Wait for SI5319 to Lock */
	sleep(1);

	/* Enable SPDIF TX */
	XSpdif_ResetFifo(&SpdifTx);
	XSpdif_SetClkConfig(&SpdifTx, XSPDIF_CLK_8);
	XSpdif_Enable(&SpdifTx, TRUE);

	/* Enable 2-Channel SDI Audio Extract */
	XSdiAud_SetCh(&SdiExtract, 0x3);
	XSdiAud_Enable(&SdiExtract, 1);

	/* Enable loop control in ACR after the whole pipe is up & running */
	sleep(1);
	Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x20), (u32)(0x00000005));
}


/*****************************************************************************/
/**
 *
 * This function configures the AES Audio Capture Path (Audio captured from
 * SPDIF RX is embedded on to SDI TX)
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XSdiEnableAESAudioCapture(void)
{
	XSdiVid_Transport *RxTransport;
	XSdiAud_Emb_Vid_Props AudEmbVidProps;

	/* Enable AES RX to SDI Audio Embed path */
	Xil_Out32((UINTPTR)(AXIS_SWITCH_TX + 0x40), (u32)(0x0));
	Xil_Out32((UINTPTR)(AXIS_SWITCH_TX + 0x44), (u32)(0x80000000));
	Xil_Out32((UINTPTR)(AXIS_SWITCH_TX), (u32)(0x2));

	/* Enable SPDIF RX */
	XSpdif_ResetFifo(&SpdifRx);
	XSpdif_Enable(&SpdifRx, TRUE);

	/* Enable interrupt to detect change in channel status (to re-config sample rate in audio embed*/
	XSdiAud_IntrEnable(&SdiEmbed, XSDIAUD_INT_ST_AES_CS_CHANGE_MASK);

	/* Enable 2-Channel SDI Audio Embed */
	RxTransport = XV_SdiRxSs_GetTransport(&SdiRxSs);
	AudEmbVidProps.XSdiAud_TFamily = RxTransport->TFamily;
	AudEmbVidProps.XSdiAud_TRate = RxTransport->TRate;
	AudEmbVidProps.XSdiAud_TScan = RxTransport->TScan;
	XSdiAud_Emb_SetVidProps(&SdiEmbed, &AudEmbVidProps);
	XSdiAud_Emb_SetAsx(&SdiEmbed, 1);
	XSdiAud_Emb_SetSmpSize(&SdiEmbed, 1);
	XSdiAud_Emb_SetSmpRate(&SdiEmbed, XSDIAUD_SMPLRATE_48);
	XSdiAud_SetCh(&SdiEmbed, 0x3);
	XSdiAud_Enable(&SdiEmbed, 1);
}

/*****************************************************************************/
/**
 *
 * This function prints the audio status information
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void XSdiAud_Report_Audio_Status(void)
{
	XSdiAud_ActGrpSt ActiveGroupStatus;
	XSdiAud_SRSt SampleRateStatus;
	XSdiAud_AsxSt AsxStatus;
	XSdiAud_ActChSt ChStatus;
	XSdiAud_SampRate SampleRate;
	u32 Asx;
	char *SR, *ASX;
	u32 i;
	u32 AnyGroupPresent;
	u32 FirstGroupPresent;

	xil_printf("\n------------\n\r");
	xil_printf("SDI Audio info\n\r");
	xil_printf("------------\n\r");
	if (XSDIAudioMode == XSDI_SDI_PASS_THROUGH) {
		xil_printf("Audio Mode: SDI Audio Pass-Through\n\r");
	} else if (XSDIAudioMode == XSDI_AES_CAPTURE_PLAYBACK) {
		xil_printf("Audio Mode: AES3 Audio Capture and Playback\n\r");
	}

	XSdiAud_GetActGrpStatus(&SdiExtract, &ActiveGroupStatus);
	XSdiAud_Ext_GetSRStatus(&SdiExtract, &SampleRateStatus);
	XSdiAud_Ext_GetAsxStatus(&SdiExtract, &AsxStatus);
	XSdiAud_Ext_GetAcChStatus(&SdiExtract, &ChStatus);
	xil_printf("\nSDI Audio Extract: Detected %d Audio Groups\n\r", ActiveGroupStatus.NumGroups);

	AnyGroupPresent = 0;
	FirstGroupPresent = 0;
	for (i = 0; i < MAX_AUDIO_GROUPS; i++) {
		if (ActiveGroupStatus.GrpActive[i]) {
			SampleRate = SampleRateStatus.SRChPair[2 * i];
			if (SampleRate == XSDIAUD_SMPLRATE_48) {
				SR = "48.0";
			} else if (SampleRate == XSDIAUD_SMPLRATE_44) {
				SR = "44.1";
			} else if (SampleRate == XSDIAUD_SMPLRATE_32) {
				SR = "32.0";
			}
			Asx = AsxStatus.AsxPair[2 * i];
			if (Asx == 0) {
				ASX = "Sync";
			} else if (Asx == 1) {
				ASX = "Async";
			}
			xil_printf("        G%d: %s KHz, %d Active Channels, %s Audio\n\r", i+1, SR, ChStatus.GrpActCh[i], ASX);
			AnyGroupPresent = 1;
			if (ActiveGroupStatus.GrpActive[0]) {
				FirstGroupPresent = 1;
			}
		}
	}

	if (XSDIAudioMode == XSDI_SDI_PASS_THROUGH) {
		if (AnyGroupPresent) {
			xil_printf("\nSDI Audio Embed: Embedding back all the incoming Audio Groups \n\r");
		} else {
			xil_printf("\nSDI Audio Embed: Audio not available to embed \n\r");
		}
	}

	if (XSDIAudioMode == XSDI_AES_CAPTURE_PLAYBACK)	{
		if (FirstGroupPresent) {
			xil_printf("\nSending out Group 1 (channel Pair 1) on AES3/SPDIF TX \n\r");
		} else {
			xil_printf("\nGroup 1 not available to send on AES3/SPDIF TX \n\r");
		}
	}
}


/*****************************************************************************/
/**
* This function is the callback function for the AES status change interrupt.
*
* @param  CallBackRef is a pointer to the callback function.
*
* @return None.
*
******************************************************************************/

void XSdiAud_AESStChangeHandler(void *CallBackRef)
{
	XSdiAud *InstancePtr = (XSdiAud *)CallBackRef;
	u32 Fs;

	/* Check the sampling frequency & take the appropriate action */
	Fs = XSpdif_GetFs(&SpdifRx, 200000000);

	if (Fs > 47000 && Fs < 53000) {
		XSdiAud_Emb_SetSmpRate(&SdiEmbed, XSDIAUD_SMPLRATE_48);
		xil_printf("\nDetected 48 KHz Audio on AES3/SPDIF RX. Embedding on to Group 1 \n\r");
	} else if (Fs > 43000 && Fs < 47000) {
		XSdiAud_Emb_SetSmpRate(&SdiEmbed, XSDIAUD_SMPLRATE_44);
		xil_printf("\nDetected 44.1 KHz Audio on AES3/SPDIF RX. Embedding on to Group 1 \n\r");
	} else if (Fs > 31000 && Fs < 35000) {
		XSdiAud_Emb_SetSmpRate(&SdiEmbed, XSDIAUD_SMPLRATE_32);
		xil_printf("\nDetected 32 KHz Audio on AES3/SPDIF RX. Embedding on to Group 1 \n\r");
	} else if (Fs > 87000 && Fs < 91000) {
		xil_printf("\nDetected 88.2 KHz Audio on AES3/SPDIF RX. Not supported by SDI Embed\n\r");
	} else if (Fs > 95000 && Fs < 99000) {
		xil_printf("\nDetected 96 KHz Audio on AES3/SPDIF RX. Not supported by SDI Embed\n\r");
	} else if (Fs > 191000 && Fs < 195000) {
		xil_printf("\nDetected 192 KHz Audio on AES3/SPDIF RX. Not supported by SDI Embed\n\r");
	} else {
		XSdiDisableAESAudioCapture();
		XSdiEnableAESAudioCapture();
	}
}

/*****************************************************************************/
/**
* This function is the callback function for the active group change interrupt.
*
* @param  CallBackRef is a pointer to the callback function.
*
* @return None.
*
******************************************************************************/

void XSdiAud_DetGroupChangeHandler(void *CallBackRef)
{
	XSdiAud *InstancePtr = (XSdiAud *)CallBackRef;
	if (InstancePtr->Config.IsEmbed == 1) {

		xil_printf("\n\nINFO>> SDI Audio Extract: Detected change in Audio Groups \n\r");

		/* Report Audio Status of the incoming SDI stream */
		XSdiAud_Report_Audio_Status();
	}
}

/*****************************************************************************/
/**
* This function is the callback function for the sample rate change interrupt.
*
* @param  CallBackRef is a pointer to the callback function.
*
* @return None.
*
******************************************************************************/

void XSdiAud_DetSRChangeHandler(void *CallBackRef)
{
	XSdiAud *InstancePtr = (XSdiAud *)CallBackRef;

	if (InstancePtr->Config.IsEmbed == 1) {

		xil_printf("\n\nINFO>> SDI Audio Extract: Detected change in Audio Sample Rate \n\r");

		/* Disable Audio */
		if (XSDIAudioMode == XSDI_SDI_PASS_THROUGH) {
			XSdiDisableSDIAudioPassThrough();
		} else if (XSDIAudioMode == XSDI_AES_CAPTURE_PLAYBACK) {
			XSdiDisableAESAudioPlayback();
		}

		/* Enable Audio Status Update Interrupt (to query audio properties before enabling the audio) */
		XSdiAud_IntrEnable(&SdiExtract, XSDIAUD_INT_EN_AUD_STAT_UPDATE_MASK);
	}
}

/*****************************************************************************/
/**
* This function is the callback function for the audio status change interrupt.
*
* @param  CallBackRef is a pointer to the callback function.
*
* @return None.
*
******************************************************************************/

void XSdiAud_DetAudStUpdateHanlder(void *CallBackRef)
{
	XSdiAud *InstancePtr = (XSdiAud *)CallBackRef;
	XSdiAud_ActGrpSt ActiveGroupStatus;
	XSdiAud_SRSt SampleRateStatus;
	XSdiAud_AsxSt AsxStatus;
	XSdiAud_SampRate SampleRate;
	u32 Asx;
	u32 AnyGroupPresent;
	u32 FirstGroupPresent;
	int i;

	AnyGroupPresent = 0;
	FirstGroupPresent = 0;
	if (InstancePtr->Config.IsEmbed) {
		/* Fetch Audio Sample Rate and ASX information from SDI Audio Extract */
		XSdiAud_GetActGrpStatus(&SdiExtract, &ActiveGroupStatus);
		XSdiAud_Ext_GetSRStatus(&SdiExtract, &SampleRateStatus);
		XSdiAud_Ext_GetAsxStatus(&SdiExtract, &AsxStatus);
		for (i = 0; i < MAX_AUDIO_GROUPS; i++) {
			if (ActiveGroupStatus.GrpActive[i]) {
				SampleRate = SampleRateStatus.SRChPair[2 * i];
				Asx = AsxStatus.AsxPair[2 * i];
				AnyGroupPresent = 1;
				if (ActiveGroupStatus.GrpActive[0]) {
					FirstGroupPresent = 1;
				}
				break;
			}
		}

		/* Enable SDI Audio Pass-Through or AES Audio Playback */
		if (((XSDIAudioMode == XSDI_SDI_PASS_THROUGH) && AnyGroupPresent) ||
				((XSDIAudioMode == XSDI_AES_CAPTURE_PLAYBACK) && FirstGroupPresent)) {
			/* Disable Audio Status Update Interrupt (else it occurs once for every frame) */
			XSdiAud_IntrDisable(&SdiExtract,XSDIAUD_INT_EN_AUD_STAT_UPDATE_MASK);

			/* Enable SDI Audio Pass-Through or AES Audio Playback */
			if (XSDIAudioMode == XSDI_SDI_PASS_THROUGH) {
				XSdiEnableSDIAudioPassThrough(SampleRate, Asx);
			} else if (XSDIAudioMode == XSDI_AES_CAPTURE_PLAYBACK) {
				XSdiEnableAESAudioPlayback(SampleRate);
			}

			/* Report Audio Status of the incoming SDI stream */
			XSdiAud_Report_Audio_Status();

			/* Enable Sample Rate Change Interrupt (Watch dog interrupt to
			 * re-configure the path with new sample rate) */
			XSdiAud_IntrEnable(&SdiExtract, XSDIAUD_INT_EN_SMP_RATE_CHANGE_MASK);

			/* Enable Audio Group Change Interrupt (To print the new status) */
			XSdiAud_IntrEnable(&SdiExtract, XSDIAUD_INT_EN_GRP_CHANGE_MASK);
		}
	}
}

/*****************************************************************************/
/**
 *
 * This function configures the N & CTS values in Audio Clock Recovery to
 * generate Audio Sample Rate clock from the Video Clock
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void ConfigNandCTS(u8 IsFractional, XSdiVid_TransMode RxTransMode,
		XSdiAud_SampRate SampleRate)
{
	if (IsFractional == 0) {
		switch (RxTransMode) {
			case XSDIVID_MODE_HD:
				if (SampleRate == XSDIAUD_SMPLRATE_48) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(6144));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(74250));
				} else if (SampleRate == XSDIAUD_SMPLRATE_44) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(6272));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(82500));
				} else if (SampleRate == XSDIAUD_SMPLRATE_32) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(4096));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(74250));
				}
				break;

			case XSDIVID_MODE_SD:
			case XSDIVID_MODE_3GA:
			case XSDIVID_MODE_3GB:
			case XSDIVID_MODE_6G:
				if (SampleRate == XSDIAUD_SMPLRATE_48) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(6144));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(148500));
				} else if (SampleRate == XSDIAUD_SMPLRATE_44) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(6272));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(165000));
				} else if (SampleRate == XSDIAUD_SMPLRATE_32) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(4096));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(148500));
				}
				break;

			case XSDIVID_MODE_12G:
				if (SampleRate == XSDIAUD_SMPLRATE_48) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(5120));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(247500));
				} else if (SampleRate == XSDIAUD_SMPLRATE_44) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(4704));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(247500));
				} else if (SampleRate == XSDIAUD_SMPLRATE_32) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(3072));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(222750));
				}
				break;

			default:
				Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(6144));
				Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(148500));
				break;
		}
	} else if (IsFractional == 1) {
		switch (RxTransMode) {
			case XSDIVID_MODE_HD:
				if (SampleRate == XSDIAUD_SMPLRATE_48) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(11648));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(140625));
				} else if (SampleRate == XSDIAUD_SMPLRATE_44) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(17836));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(234375));
				} else if (SampleRate == XSDIAUD_SMPLRATE_32) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(11648));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(210937));
				}
				break;

			case XSDIVID_MODE_SD:
			case XSDIVID_MODE_3GA:
			case XSDIVID_MODE_3GB:
			case XSDIVID_MODE_6G:
				if (SampleRate == XSDIAUD_SMPLRATE_48) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(5824));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(140625));
				} else if (SampleRate == XSDIAUD_SMPLRATE_44) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(8918));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(234375));
				} else if (SampleRate == XSDIAUD_SMPLRATE_32) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(11648));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(421875));
				}
				break;

			case XSDIVID_MODE_12G:
				if (SampleRate == XSDIAUD_SMPLRATE_48) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(5824));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(281250));
				} else if (SampleRate == XSDIAUD_SMPLRATE_44) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(4459));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(234375));
				} else if (SampleRate == XSDIAUD_SMPLRATE_32) {
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(5824));
					Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(421875));
				}
				break;

			default:
				Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x50), (u32)(5824));
				Xil_Out32((UINTPTR)(ACR_BASEADDR + 0x54), (u32)(140625));
				break;
		}
	}
}
#endif

/*****************************************************************************/
/**
 *
 * Main function to call example with SDI TX and SDI RX drivers.
 *
 * @return
 *		- XST_SUCCESS if SDI example was successfully.
 *		- XST_FAILURE if SDI example failed.
 *
 * @note	None.
 *
 ******************************************************************************/
int main(void)
{
	u32 Status;
#ifdef XPAR_XSDIAUD_NUM_INSTANCES
	XSpdif_Config *Config;
#endif

	/* Setting path for Si570 chip */
	I2cMux();

	/* si570 configuration of 148.5MHz */
	Si570_SetClock(XPAR_IIC_0_BASEADDR, I2C_CLK_ADDR_570, FREQ_148_5_MHz);


	/* Rx Reset Sequence */
	Xil_Out32((UINTPTR)(GPIO_1_RX_RST), (u32)(0x00000000));
	Xil_Out32((UINTPTR)(GPIO_1_RX_RST), (u32)(0x00000001));


	StartTxAfterRxFlag = (FALSE);

	Xil_ExceptionDisable();
	init_platform();

	ClearScreen();

	xil_printf("----------------------------------------\r\n");
	xil_printf("---     SDI Pass Through Example     ---\r\n");
	xil_printf("---     (c) 2018 by Xilinx, Inc.     ---\r\n");
	xil_printf("----------------------------------------\r\n");
	xil_printf("      Build %s - %s      \r\n", __DATE__, __TIME__);
	xil_printf("----------------------------------------\r\n");

	/* Initialize IRQ */
	Status = SetupInterruptSystem();
	if (Status == XST_FAILURE) {
		xil_printf("IRQ init failed.\n\r\r");
		return XST_FAILURE;
	}

	/* Initialize SDI TX Subsystem */
	XV_SdiTxSs_ConfigPtr = XV_SdiTxSs_LookupConfig(XPAR_XV_SDITXSS_0_DEVICE_ID);

	XV_SdiTxSs_ConfigPtr->BaseAddress = XPAR_XV_SDITXSS_0_BASEADDR;

	if (!XV_SdiTxSs_ConfigPtr) {
		SdiTxSs.IsReady = 0;
		return XST_DEVICE_NOT_FOUND;
	}

	/* Initialize top level and all included sub-cores */
	Status = XV_SdiTxSs_CfgInitialize(&SdiTxSs,
					XV_SdiTxSs_ConfigPtr,
					XV_SdiTxSs_ConfigPtr->BaseAddress);
#ifdef XPAR_XSDIAUD_NUM_INSTANCES
	XV_SdiTxSs_SetCoreSettings(&SdiTxSs, XV_SDITXSS_CORESELID_USEANCIN, 1);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: SDI TX Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Disable SDI Tx all Interrupts */
	XV_SdiTxSs_IntrDisable(&SdiTxSs, XV_SDITXSS_IER_ALLINTR_MASK);


	XV_SdiTxSs_SetCallback(&SdiTxSs,
				XV_SDITXSS_HANDLER_GTREADY,
				GtReadyCallback,
				(void *)&SdiTxSs);

	/* Enable SDI Tx GT reset done Interrupts */
	XV_SdiTxSs_IntrEnable(&SdiTxSs, XV_SDITXSS_IER_GTTX_RSTDONE_MASK);

	/* Initialize axis_fifo_gpio */
	Gpio_AxisFifo_resetn_ConfigPtr =
		XGpio_LookupConfig(XPAR_GPIO_2_DEVICE_ID);

	if (!Gpio_AxisFifo_resetn_ConfigPtr) {
		Gpio_AxisFifo_resetn.IsReady = 0;
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XGpio_CfgInitialize(&Gpio_AxisFifo_resetn,
					Gpio_AxisFifo_resetn_ConfigPtr,
					Gpio_AxisFifo_resetn_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for AxisFifo Reset ");
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Setting it as output */
	XGpio_SetDataDirection(&Gpio_AxisFifo_resetn, 1, 0);

	/* Initialize SI5324_lol_gpio */
	Gpio_si5324_ConfigPtr =
		XGpio_LookupConfig(XPAR_GPIO_3_DEVICE_ID);

	if (!Gpio_si5324_ConfigPtr) {
		Gpio_si5324.IsReady = 0;
		return XST_DEVICE_NOT_FOUND;
	}

	Status = XGpio_CfgInitialize(&Gpio_si5324,
					Gpio_si5324_ConfigPtr,
					Gpio_si5324_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for SI5324 LOL");
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Setting it as input */
	XGpio_SetDataDirection(&Gpio_si5324, 1, 1);

	/* Initialize SDI RX Subsystem */
	XV_SdiRxSs_ConfigPtr = XV_SdiRxSs_LookupConfig(XPAR_XV_SDIRX_0_DEVICE_ID);

	XV_SdiRxSs_ConfigPtr->BaseAddress = XPAR_XV_SDIRXSS_0_BASEADDR;

	if (!XV_SdiRxSs_ConfigPtr) {
		SdiRxSs.IsReady = 0;
		return XST_DEVICE_NOT_FOUND;
	}

	/* Initialize top level and all included sub-cores */
	Status = XV_SdiRxSs_CfgInitialize(&SdiRxSs,
					XV_SdiRxSs_ConfigPtr,
					XV_SdiRxSs_ConfigPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: SDI RX Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	/* Disable SDI Rx all Interrupts */
	XV_SdiRxSs_IntrDisable(&SdiRxSs, XV_SDIRXSS_IER_ALLINTR_MASK);

	XV_SdiRxSs_SetCallback(&SdiRxSs,
				XV_SDIRXSS_HANDLER_STREAM_UP,
				RxStreamUpCallback,
				(void *)&SdiRxSs);

	XV_SdiRxSs_SetCallback(&SdiRxSs,
				XV_SDIRXSS_HANDLER_STREAM_DOWN,
				RxStreamDownCallback,
				(void *)&SdiRxSs);

	/* Enable SDI Rx video lock and unlock Interrupts */
	XV_SdiRxSs_IntrEnable(&SdiRxSs, XV_SDIRXSS_IER_VIDEO_LOCK_MASK | XV_SDIRXSS_IER_VIDEO_UNLOCK_MASK);

#ifdef XPAR_XSDIAUD_NUM_INSTANCES
	AudioStatusUpdate = 0;
	XSDIAudioMode = XSDI_SDI_PASS_THROUGH;
	/*
	 * Lookup and Initialize the Spdif so that it's ready to use.
	 */

	Config = XSpdif_LookupConfig(SPDIF_0_DEVICE_ID);
	if (Config == NULL)
		 return XST_FAILURE;
	Status = XSpdif_CfgInitialize(&SpdifRx, Config,
				        Config->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	Config = XSpdif_LookupConfig(SPDIF_1_DEVICE_ID);
	if (Config == NULL)
		 return XST_FAILURE;
	Status = XSpdif_CfgInitialize(&SpdifTx, Config,
				        Config->BaseAddress);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	Status = XSdiAud_Initialize(&SdiEmbed, SDIAUD_1_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: SDI Embed IP Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	Status = XSdiAud_Initialize(&SdiExtract, SDIAUD_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: SDI Extract IP Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	xil_printf("SDI AUDIO EMBED AND EXTRACT DRIVER ready to use\r\n");

	/*Audio Extract Module Reset*/
	XSdiAud_ConfigReset(&SdiExtract);
	XSdiAud_CoreReset(&SdiExtract, 1);
	XSdiAud_CoreReset(&SdiExtract, 0);

	/*Audio Embed Module Reset*/
	XSdiAud_ConfigReset(&SdiEmbed);
	XSdiAud_CoreReset(&SdiEmbed, 1);
	XSdiAud_CoreReset(&SdiEmbed, 0);

	XSdiAud_SetHandler(&SdiExtract, XSDIAUD_HANDLER_CHSTAT_CHNG_DET,
			XSdiAud_DetAudStUpdateHanlder,
			(void *)&SdiExtract);
	XSdiAud_SetHandler(&SdiExtract, XSDIAUD_HANDLER_SAMPLE_RATE_CHNG_DET,
			XSdiAud_DetSRChangeHandler,
			(void *)&SdiExtract);
	XSdiAud_SetHandler(&SdiExtract, XSDIAUD_HANDLER_AUD_GRP_CHNG_DET,
			XSdiAud_DetGroupChangeHandler,
			(void *)&SdiExtract);
	XSdiAud_SetHandler(&SdiEmbed, XSDIAUD_HANDLER_AES_CS_CHANGE_DET,
			XSdiAud_AESStChangeHandler,
			(void *)&SdiEmbed);
#endif
	Status = 0;
	Status |= XScuGic_Connect(&Intc,
				SDI_RX_SS_INTR_ID,
				(XInterruptHandler)XV_SdiRxSS_SdiRxIntrHandler,
				(void *)&SdiRxSs);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc, SDI_RX_SS_INTR_ID);
	} else {
		xil_printf("ERR:: Unable to register SDI RX interrupt handler");
		xil_printf("SDI RX SS initialization error\n\r");
		return XST_FAILURE;
	}

	/*Register SDI TX SS Interrupt Handler with Interrupt Controller */
	Status = 0;
	Status |= XScuGic_Connect(&Intc,
				SDI_TX_SS_INTR_ID,
				(XInterruptHandler)XV_SdiTxSS_SdiTxIntrHandler,
				(void *)&SdiTxSs);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc, SDI_TX_SS_INTR_ID);
	} else {
		xil_printf("ERR:: Unable to register SDI TX interrupt handler");
		xil_printf("SDI TX SS initialization error\n\r");
		return XST_FAILURE;
	}

#ifdef XPAR_XSDIAUD_NUM_INSTANCES
	/* Register Spdif Rx Interrupt Handler with Interrupt Controller */
	Status = 0;
	Status |= XScuGic_Connect(&Intc,
				SPDIF_0_INTERRUPT_ID,
				(XInterruptHandler)XSpdif_IntrHandler,
				(void *)&SpdifRx);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc, SPDIF_0_INTERRUPT_ID);
		xil_printf("Successfully registered Spdif Rx interrupt handler\n\r");
	} else {
		xil_printf("ERR:: Unable to register Spdif Rx interrupt handler\n\r");
		return XST_FAILURE;
	}

	/* Register Spdif Tx Interrupt Handler with Interrupt Controller */
	Status = 0;
	Status |= XScuGic_Connect(&Intc,
				SPDIF_1_INTERRUPT_ID,
				(XInterruptHandler)XSpdif_IntrHandler,
				(void *)&SpdifTx);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc, SPDIF_1_INTERRUPT_ID);
		xil_printf("Successfully registered Spdif Tx interrupt handler\n\r");
	} else {
		xil_printf("ERR:: Unable to register Spdif Tx interrupt handler\n\r");
		return XST_FAILURE;
	}

	/* Register SDI AUdio Extract Interrupt Handler with Interrupt Controller */
	Status = 0;
	Status |= XScuGic_Connect(&Intc,
				SDIAUD_0_INTERRUPT_ID,
				(XInterruptHandler)XSdiAud_IntrHandler,
				(void *)&SdiExtract);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc, SDIAUD_0_INTERRUPT_ID);
		xil_printf("Successfully registered SDI Audio Extract interrupt handler\n\r");
	} else {
		xil_printf("ERR:: Unable to register SDI Audio Extract interrupt handler\n\r");
		return XST_FAILURE;
	}
	/* Register SDI AUdio Embed Interrupt Handler with Interrupt Controller */
	Status = 0;
	Status |= XScuGic_Connect(&Intc,
				SDIAUD_1_INTERRUPT_ID,
				(XInterruptHandler)XSdiAud_IntrHandler,
				(void *)&SdiEmbed);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc, SDIAUD_1_INTERRUPT_ID);
		xil_printf("Successfully registered SDI Audio Embed interrupt handler\n\r");
	} else {
		xil_printf("ERR:: Unable to register SDI Audio Embed interrupt handler\n\r");
		return XST_FAILURE;
	}
#endif
	/* Initialize menu */
	XSdi_MenuInitialize(&SdiMenu, UART_BASEADDR);

	/* Enable exceptions. */
	Xil_AssertSetCallback((Xil_AssertCallback) Xil_AssertCallbackRoutine);
	Xil_ExceptionEnable();

	while (1) {
		if (StartTxAfterRxFlag) {
				StartTxAfterRx();
		}

		XSdi_MenuProcess(&SdiMenu);
	}
	return XST_SUCCESS;
}
