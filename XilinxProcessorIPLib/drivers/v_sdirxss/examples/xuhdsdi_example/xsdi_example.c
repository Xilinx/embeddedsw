/******************************************************************************
 *
 * Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
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
#include "xsdiaud.h"
#include "xsdiaud_hw.h"
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
else
#include "xscugic.h"
#endif

/************************** Constant Definitions *****************************/
#define GPIO_0_TX_RST XPAR_AXI_GPIO_0_BASEADDR
#define GPIO_1_RX_RST XPAR_AXI_GPIO_1_BASEADDR

#ifdef XPAR_XSDIAUD_NUM_INSTANCES
#define SDI_AUD_EMBED   XPAR_V_UHDSDI_AUDIO_EMBED_1_BASEADDR
#define SDI_AUD_EXTRT   XPAR_V_UHDSDI_AUDIO_EXTRACT_0_BASEADDR
#endif

typedef u8 AddressType;
#define PAGE_SIZE   16

#define I2C_MUX_ADDR	0x74  /**< I2C Mux Address */
#define I2C_CLK_ADDR	0x69  /**< I2C Clk Address */
#define I2C_CLK_ADDR_570	0x5D  /**< I2C Clk Address for Si570*/

#define SDI_TX_SS_INTR_ID XPAR_FABRIC_V_SMPTE_UHDSDI_TX_SS_SDI_TX_IRQ_INTR
#define SDI_RX_SS_INTR_ID XPAR_FABRIC_V_SMPTE_UHDSDI_RX_SS_SDI_RX_IRQ_INTR

#define IIC_INTR_ID XPAR_FABRIC_AXI_IIC_0_IIC2INTC_IRPT_INTR

#define FREQ_148_5_MHz	(148500000)
#define FREQ_297_MHz	(297000000)
#define FREQ_74_25_MHz	(74250000)
#define FREQ_27_MHz	(27000000)
#define FREQ_148_35_MHz	(148350000)

#ifdef XPAR_XSDIAUD_NUM_INSTANCES
#define SDIAUD_0_DEVICE_ID	XPAR_V_UHDSDI_AUDIO_EXTRACT_0_DEVICE_ID
#define SDIAUD_1_DEVICE_ID	XPAR_V_UHDSDI_AUDIO_EMBED_1_DEVICE_ID

#define SDIAUD_0_INTERRUPT_ID	XPAR_FABRIC_V_UHDSDI_AUDIO_EXTRACT_0_INTERRUPT_INTR
#define SDIAUD_1_INTERRUPT_ID	XPAR_FABRIC_V_UHDSDI_AUDIO_EMBED_1_INTERRUPT_INTR

#define XPAR_SDIAUD_0_BA	XPAR_V_UHDSDI_AUDIO_EXTRACT_0_BASEADDR
#define XPAR_SDIAUD_1_BA	XPAR_V_UHDSDI_AUDIO_EMBED_1_BASEADDR
#define XSDIAUD_QUAD_GROUP	4
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
int Si570_SetClock(u32 IICBaseAddress, u8 IICAddress1, u32 RxRefClk);
void Info(void);
void DebugInfo(void);
#ifdef XPAR_XSDIAUD_NUM_INSTANCES
void SdiAud_Ext_IntrHandler(XSdiAud *InstancePtr);
void SdiAudGrpChangeDetHandler(void *CallBackRef);
XSdiAud_NumOfCh XSdiAud_Ext_DetActCh(XSdiAud *InstancePtr);
#endif

/*
 * Write buffer for writing a page.
 */
u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];

u8 ReadBuffer[PAGE_SIZE];	/* Read buffer for reading a page. */

volatile u8 TransmitComplete;	/* Flag to check completion of Transmission */
volatile u8 ReceiveComplete;	/* Flag to check completion of Reception */

#ifdef XPAR_XSDIAUD_NUM_INSTANCES
XSdiAud SdiAudInstance;		/* Instance of the SdiAud device */
#endif

unsigned int IntrReceived;

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
XSdiAud SdiExtract;		/* Instance0 of the SdiAud device */
XSdiAud SdiEmbed;		/* Instance1 of the SdiAud device */
XSdiAud_GrpsPrsnt DetAudioGrps;
XSdiAud_NumOfCh AudNumOfCh;
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

#ifdef XPAR_XSDIAUD_NUM_INSTANCES
	/*Audio Extract Module Reset De-Assertion*/
	XSdiAud_ResetCoreEn(&SdiExtract, 0);
	/*Audio Embed Module Reset De-Assertion*/
	XSdiAud_ResetCoreEn(&SdiEmbed, 0);

    /*Audio Embed Module Enable*/
	XSdiAud_Enable(&SdiEmbed, 1);
    /*Audio Extract Module Enable*/
	XSdiAud_Enable(&SdiExtract, 1);
#endif
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
	XV_SdiTxSs_Stop(&SdiTxSs);

#ifdef XPAR_XSDIAUD_NUM_INSTANCES
    /*Audio Extract Module Disable*/
	XSdiAud_Enable(&SdiEmbed, 0);
	/*Audio Extract Module Disable*/
	XSdiAud_Enable(&SdiExtract, 0);

	/*Audio Extract Module Reset Assertion*/
	XSdiAud_ResetCoreEn(&SdiExtract, 1);
	/*Audio Embed Module Reset Assertion*/
	XSdiAud_ResetCoreEn(&SdiEmbed, 1);

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
 * This function is the handler which performs processing for the SDI extract
 * IP. It is called from an interrupt context when the SDI Extract IP receives
 * a interrupt for change in the presence of audio groups in the incoming
 * SDI stream.
 *
 * This handler provides an example of what needs to be done when gorup change
 * interrupt is detected in the received SDI audio stream by the SDI extract IP
 * block, but is application specific.
 *
 * @param	CallBackRef is a pointer to the callback function
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
void SdiAudGrpChangeDetHandler(void *CallBackRef)
{

	XSdiAud *InstancePtr = (XSdiAud *)CallBackRef;
	XSdiVid_TransMode RxTransModeAud;

	RxTransModeAud =  XV_SdiRxSs_GetTransportMode(&SdiRxSs);
	/* Set the interrupt received flag. */
	XSdiAud_IntrClr(&SdiExtract,XSDIAUD_INT_EN_GRP_CHG_MASK);

	/*Audio Embed Module Reset Assertion*/
	XSdiAud_ResetCoreEn(&SdiEmbed, 1);
	/*Audio Extract Module Reset Assertion*/
	XSdiAud_ResetCoreEn(&SdiExtract, 1);

	xil_printf("------------\n\r");
	xil_printf("SDI Audio Info\n\r");
	xil_printf("------------\n\r");

	/*Interrupt Mask Enable for Extract*/
	DetAudioGrps = XSdiAud_DetAudGrp(&SdiExtract);

	AudNumOfCh = XSdiAud_Ext_DetActCh(&SdiExtract);
	xil_printf("Number of Audio Channels = %d\r\n",AudNumOfCh);

	/* Audio Extract Function to set the Clock Phase in HD Mode */
	XSdiAud_Ext_SetClkPhase(&SdiExtract, 1);


	switch (DetAudioGrps)
	{
		case XSDIAUD_GROUP_0:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP1, AudNumOfCh);
			XSdiAud_SetCh(&SdiEmbed, XSDIAUD_GROUP1, AudNumOfCh);
			break;

		case XSDIAUD_GROUP_1:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP1, AudNumOfCh);
			XSdiAud_SetCh(&SdiEmbed, XSDIAUD_GROUP1, AudNumOfCh);
			break;

		case XSDIAUD_GROUP_2:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP2, AudNumOfCh);
			XSdiAud_SetCh(&SdiEmbed, XSDIAUD_GROUP2, AudNumOfCh);
			break;

		case XSDIAUD_GROUP_1_2:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP1, AudNumOfCh);
			XSdiAud_SetCh(&SdiEmbed, XSDIAUD_GROUP1, AudNumOfCh);
			break;

		case XSDIAUD_GROUP_3:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP3, AudNumOfCh);
			XSdiAud_SetCh(&SdiEmbed, XSDIAUD_GROUP3, AudNumOfCh);
			break;

		case XSDIAUD_GROUP_1_3:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP1, XSDIAUD_QUAD_GROUP);
			xil_printf("Only Group 1 is configured\r\n");
			xil_printf("Non-Sequential Groups are not Supported\r\n");
			break;

		case XSDIAUD_GROUP_2_3:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP2, AudNumOfCh);
			XSdiAud_SetCh(&SdiEmbed, XSDIAUD_GROUP2, AudNumOfCh);
			break;

		case XSDIAUD_GROUP_1_2_3:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP1, AudNumOfCh);
			XSdiAud_SetCh(&SdiEmbed, XSDIAUD_GROUP1, AudNumOfCh);
			break;

		case XSDIAUD_GROUP_4:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP4, AudNumOfCh);
			XSdiAud_SetCh(&SdiEmbed, XSDIAUD_GROUP4, AudNumOfCh);
			break;

		case XSDIAUD_GROUP_1_4:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP1, XSDIAUD_QUAD_GROUP);
			xil_printf("Only Group 1 is configured\r\n");
			xil_printf("Non-Sequential Groups are not Supported\r\n");
			break;

		case XSDIAUD_GROUP_2_4:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP2, XSDIAUD_QUAD_GROUP);
			xil_printf("Only Group 2 is configured\r\n");
			xil_printf("Non-Sequential Groups are not Supported\r\n");
			break;

		case XSDIAUD_GROUP_1_2_4:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP1, XSDIAUD_QUAD_GROUP);
			xil_printf("Only Group 1 is configured\r\n");
			xil_printf("Non-Sequential Groups are not Supported\r\n");
			break;

		case XSDIAUD_GROUP_3_4:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP3, AudNumOfCh);
			XSdiAud_SetCh(&SdiEmbed, XSDIAUD_GROUP3, AudNumOfCh);
			break;

		case XSDIAUD_GROUP_1_3_4:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP1, XSDIAUD_QUAD_GROUP);
			xil_printf("Only Group 1 is configured\r\n");
			xil_printf("Non-Sequential Groups are not Supported\r\n");
			break;

		case XSDIAUD_GROUP_2_3_4:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP2, AudNumOfCh);
			XSdiAud_SetCh(&SdiEmbed, XSDIAUD_GROUP2, AudNumOfCh);
			break;

		case XSDIAUD_GROUP_ALL:
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP1, AudNumOfCh);
			XSdiAud_SetCh(&SdiEmbed, XSDIAUD_GROUP1, AudNumOfCh);
			break;

		default:
			xil_printf("Invalid case: Groups are not configured\r\n");
			XSdiAud_SetCh(&SdiExtract, XSDIAUD_GROUP3, XSDIAUD_QUAD_GROUP);
			break;
	}

	/* Embed Module is Configured here*/
	/* Video Embed Function to set enable external line */
	XSdiAud_Emb_EnExtrnLine(&SdiEmbed, 1);

	/*Audio Embed Function to set the sampling rate */
	XSdiAud_Emb_SetSmpRate(&SdiEmbed, XSDIAUD_SAMPRATE0);

	if(RxTransModeAud == XSDIVID_MODE_SD) {
		XSdiAud_Emb_SetSmpSize(&SdiEmbed, XSDIAUD_SAMPSIZE1);
		XSdiAud_Emb_SetLineStd(&SdiEmbed, XSDIAUD_NTSC);
	}
	else {
		XSdiAud_Emb_SetSmpSize(&SdiEmbed, XSDIAUD_SAMPSIZE0);
		XSdiAud_Emb_SetLineStd(&SdiEmbed, XSDIAUD_SMPTE_274M_1080p_30Hz);
	}

		/*Audio Extract Module Reset De-Assertion*/
		XSdiAud_ResetCoreEn(&SdiExtract, 0);
		/*Audio Embed Module Reset De-Assertion*/
		XSdiAud_ResetCoreEn(&SdiEmbed, 0);

		/*Audio Extract Module Enable*/
		XSdiAud_Enable(&SdiExtract, 1);

		/*Audio Extract Module Enable*/
		XSdiAud_Enable(&SdiEmbed, 1);

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

	XV_SdiTxSs_ConfigPtr->BaseAddress = XPAR_V_SMPTE_UHDSDI_TX_SS_BASEADDR;

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
		XGpio_LookupConfig(XPAR_GPIO_REGISTER_AXIS_FIFO_GPIO_DEVICE_ID);

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
		XGpio_LookupConfig(XPAR_GPIO_REGISTER_SI5324_LOL_GPIO_DEVICE_ID);

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

	XV_SdiRxSs_ConfigPtr->BaseAddress = XPAR_V_SMPTE_UHDSDI_RX_SS_BASEADDR;

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
	Status = XSdiAud_Initialize(&SdiEmbed, XPAR_V_UHDSDI_AUDIO_EMBED_1_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: SDI Embed IP Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	Status = XSdiAud_Initialize(&SdiExtract, XPAR_V_UHDSDI_AUDIO_EXTRACT_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: SDI Extract IP Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	xil_printf("SDI AUDIO EMBED AND EXTRACT DRIVER ready to use\r\n");

	/*Audio Extract Module Reset*/
	XSdiAud_ResetCoreEn(&SdiExtract, 1);
	XSdiAud_ResetCoreEn(&SdiExtract, 0);
	XSdiAud_ResetReg(&SdiExtract);

	/*Audio Embed Module Reset*/
	XSdiAud_ResetCoreEn(&SdiEmbed, 1);
	XSdiAud_ResetCoreEn(&SdiEmbed, 0);
	XSdiAud_ResetReg(&SdiEmbed);

	XSdiAud_IntrEnable(&SdiExtract,XSDIAUD_INT_EN_GRP_CHG_MASK);
	XSdiAud_IntrEnable(&SdiExtract,XSDIAUD_EXT_INT_EN_PKT_CHG_MASK);
	XSdiAud_IntrEnable(&SdiExtract,XSDIAUD_EXT_INT_EN_STS_CHG_MASK);
	XSdiAud_IntrEnable(&SdiExtract,XSDIAUD_EXT_INT_EN_FIFO_OF_MASK);
	XSdiAud_IntrEnable(&SdiExtract,XSDIAUD_EXT_INT_EN_PERR_MASK);
	XSdiAud_IntrEnable(&SdiExtract,XSDIAUD_EXT_INT_EN_CERR_MASK);

	XSdiAud_IntrEnable(&SdiEmbed,XSDIAUD_INT_EN_GRP_CHG_MASK);

	XSdiAud_SetHandler(&SdiExtract, XSDIAUD_HANDLER_AUD_GRP_CHNG_DET,
	                                             SdiAudGrpChangeDetHandler,
	                                             (void *)&SdiExtract);
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
	/* Register SDI AUdio Extract Interrupt Handler with Interrupt Controller */
	Status = 0;
	Status |= XScuGic_Connect(&Intc,
				SDIAUD_0_INTERRUPT_ID,
				(XInterruptHandler)XSdiAud_IntrHandler,
				(void *)&SdiExtract);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc, SDIAUD_0_INTERRUPT_ID);
		xil_printf("Successfully registered SDI AUdio Extract interrupt handler");
	} else {
		xil_printf("ERR:: Unable to register SDI AUdio Extract interrupt handler");
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
#ifdef XPAR_XSDIAUD_NUM_INSTANCES
/*****************************************************************************/
/**
* This function detects the number of active channels in the incoming SDI stream
* and returns the number of active channels.
*
* @param  InstancePtr is a pointer to the XSdiAud instance.
*
* @return Return type is enum XSdiAud_NumOfCh, by this we can know the
*         number of channels which are present.
*
******************************************************************************/
XSdiAud_NumOfCh XSdiAud_Ext_DetActCh(XSdiAud *InstancePtr)
{
	XSdiAud_NumOfCh XSdiAud_ActCh;
	u32 XSdiAud_ActReg;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	XSdiAud_ActReg = XSdiAud_ReadReg(InstancePtr->Config.BaseAddress,
			XSDIAUD_EXT_CNTRL_PKTSTAT_REG_OFFSET);

	XSdiAud_ActReg = (XSdiAud_ActReg & XSDIAUD_EXT_PKTST_AC_MASK) >>
			XSDIAUD_EXT_PKTST_AC_SHIFT;

	switch(XSdiAud_ActReg)
	{
		case 0x0000:
			XSdiAud_ActCh = 0;
			break;
		case 0x000F:
			XSdiAud_ActCh = 4;
			break;
		case 0x00F0:
			XSdiAud_ActCh = 4;
			break;
		case 0x00FF:
			XSdiAud_ActCh = 8;
			break;
		case 0x0F00:
			XSdiAud_ActCh = 4;
			break;
		case 0x0F0F:
			XSdiAud_ActCh = 8;
			break;
		case 0x0FF0:
			XSdiAud_ActCh = 8;
			break;
		case 0x0FFF:
			XSdiAud_ActCh = 12;
			break;
		case 0xF000:
			XSdiAud_ActCh = 4;
			break;
		case 0xF00F:
			XSdiAud_ActCh = 8;
			break;
		case 0xF0F0:
			XSdiAud_ActCh = 8;
			break;
		case 0xF0FF:
			XSdiAud_ActCh = 12;
			break;
		case 0xFF00:
			XSdiAud_ActCh = 8;
			break;
		case 0xFF0F:
			XSdiAud_ActCh = 12;
			break;
		case 0xFFF0:
			XSdiAud_ActCh = 12;
			break;
		case 0xFFFF:
			XSdiAud_ActCh = 16;
			break;
		default:
			XSdiAud_ActCh = 4;
			xil_printf("Invalid Case: Num of Act Channels are not 4,8,12or16\r\n");
			break;
	}
	return XSdiAud_ActCh;
}
#endif
