/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 *****************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xmipi_example.c
 *
 * This file demonstrates the Xilinx MIPI CSI2 Rx Subsystem and MIPI DSI2 Tx
 * Subsystem. The video pipeline is created by connecting an IMX274 Camera
 * sensor to the MIPI CSI2 Rx Subsystem. The sensor is programmed to generate
 * RAW10 type de bayered data as per the pipeline configuration. The raw pixels
 * are fed to Xilinx Demosaic, Gamma lut and v_proc_ss IPs to convert pixel
 * to RGB format. The RGB pixels are then sent across to a data Video
 * Test Pattern Generator. In a pass through mode, the camera data is passed
 * to an AXI Stream broadcaster. This sends across video stream to along
 * HDMI Tx Subsystem and a Video Processing Subsystem configured as Scalar.
 * The output of the scalar is connected to the DSI2 Tx Subsystem. The DSI2
 * output is connected to AUO Asus Display panel with 1920x1200 fixed resolution
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.00  pg    12/07/17 Initial release.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xiic.h"
#include "xil_io.h"
#include "xuartps.h"
#include "xil_types.h"
#include "xil_exception.h"
#include "string.h"
#include "si5324drv.h"
#include "xvidc.h"
#include "xvidc_edid.h"
#include "dp159.h"
#include "sleep.h"
#include "xv_hdmitxss.h"
#include "xvphy.h"
#include "xv_tpg.h"
#include "xgpio.h"
#include "xscugic.h"
#include "xvprocss.h"
#include "sensor_cfgs.h"
#include "xmipi_menu.h"
#include "pipeline_program.h"
#include "xv_frmbufwr_l2.h"
#include "xv_frmbufrd_l2.h"


/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/
#define LOOPBACK_MODE_EN	0
#define XPAR_CPU_CORE_CLOCK_FREQ_HZ	100000000
#define UART_BASEADDR	XPAR_XUARTPS_0_BASEADDR
#define I2C_MUX_ADDR	0x74  /**< I2C Mux Address */
#define I2C_CLK_ADDR	0x68  /**< I2C Clk Address */

#define IIC_SENSOR_INTR_ID	XPAR_FABRIC_AXI_IIC_1_SENSOR_IIC2INTC_IRPT_INTR

#define HDMI_TX_SS_DEVICE_ID	XPAR_XV_HDMITX_0_DEVICE_ID
#define HDMI_TXSS_INTR_ID	XPAR_FABRIC_V_HDMITXSS_0_VEC_ID
#define HDMI_TX_SS_INTR_ID 	HDMI_TXSS_INTR_ID

#define VPHY_DEV_ID	XPAR_VPHY_0_DEVICE_ID
#define VPHY_INTRID XPAR_FABRIC_VPHY_0_VEC_ID

#define VID_PHY_DEVICE_ID	VPHY_DEV_ID
#define VID_PHY_INTR_ID		VPHY_INTRID

#define GPIO_TPG_RESET_DEVICE_ID	XPAR_GPIO_3_DEVICE_ID

#define V_TPG_DEVICE_ID		XPAR_XV_TPG_0_DEVICE_ID

#define GPIO_SENSOR		XPAR_AXI_GPIO_0_SENSOR_BASEADDR
#define GPIO_IP_RESET	XPAR_GPIO_3_BASEADDR
#define GPIO_IP_RESET1   XPAR_GPIO_4_BASEADDR
#define GPIO_IP_RESET2   XPAR_GPIO_2_BASEADDR

#ifdef XPAR_PSU_ACPU_GIC_DEVICE_ID
#define PSU_INTR_DEVICE_ID	XPAR_PSU_ACPU_GIC_DEVICE_ID
#endif

#ifdef XPAR_PSU_RCPU_GIC_DEVICE_ID
#define PSU_INTR_DEVICE_ID	XPAR_PSU_RCPU_GIC_DEVICE_ID
#endif


#define XPAR_INTC_0_V_FRMBUF_WR_0_VEC_ID XPAR_FABRIC_V_FRMBUF_WR_0_VEC_ID
#define XPAR_INTC_0_V_FRMBUF_RD_0_VEC_ID XPAR_FABRIC_V_FRMBUF_RD_0_VEC_ID

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

int I2cMux(void);
int I2cClk(u32 InFreq, u32 OutFreq);

void EnableColorBar(XVphy *VphyPtr, XV_HdmiTxSs *HdmiTxSsPtr,
			XVidC_VideoMode VideoMode,
			XVidC_ColorFormat ColorFormat,
			XVidC_ColorDepth Bpc);

extern u32 InitStreamMuxGpio(void);

void Info(void);

void CloneTxEdid(void);

void SendVSInfoframe(void);

/************************** Variable Definitions *****************************/

XVphy Vphy; /* VPHY structure */

XV_HdmiTxSs HdmiTxSs; /* HDMI TX SS structure */
XV_HdmiTxSs_Config *XV_HdmiTxSs_ConfigPtr;

XScuGic Intc;

XGpio Gpio_Tpg_resetn;
XGpio_Config *Gpio_Tpg_resetn_ConfigPtr;

XV_tpg Tpg;
XV_tpg_Config *Tpg_ConfigPtr;
XTpg_PatternId Pattern; /**< Video pattern */



u8 IsPassThrough; /**< Demo mode 0-colorbar 1-pass through */
u8 StartTxAfterRxFlag;
u8 TxBusy;         /* TX busy flag is set while the TX is initialized */
u8 TxRestartColorbar; 	/* TX restart flag is set when the TX cable has
			 * been reconnected and the TX colorbar was showing.
			 */
u32 Index;
XMipi_Menu HdmiMenu;      /* Menu structure */

XPipeline_Cfg Pipeline_Cfg;
XPipeline_Cfg New_Cfg;

extern XV_FrmbufWr_l2     frmbufwr;
extern XV_FrmbufRd_l2     frmbufrd;

extern XIic IicIoExpander;
extern XIic IicSensor; /* The instance of the IIC device. */

extern XVprocSs scaler_new_inst;


/************************** Function Definitions *****************************/

void XV_ConfigTpg(XV_tpg *InstancePtr);
void ResetTpg(void);

extern void EnableDSI();
extern void Shutdown_DSI();
extern void Reconfigure_DSI();
extern void Reconfigure_HDMI(void);
extern void SelectDSIOutput(void);
extern void SelectHDMIOutput(void);
extern void config_csi_cap_path();
/*****************************************************************************/
/**
 * This function clones the EDID of the connected sink device to the HDMI RX
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void CloneTxEdid(void) {
	print("\r\nEdid Cloning no possible with HDMI RX SS.\n\r");
}

/*****************************************************************************/
/**
 *
 * This function generates video pattern.
 *
 * @param	InstancePtr TPG Instance Pointer.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void XV_ConfigTpg(XV_tpg *InstancePtr) {
	XV_tpg *pTpg = InstancePtr;

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);

	u32 width, height;

	switch (Pipeline_Cfg.VideoMode) {
		case XVIDC_VM_3840x2160_30_P:
		case XVIDC_VM_3840x2160_60_P:
			width = 3840;
			height = 2160;
			break;
		case XVIDC_VM_1920x1080_30_P:
		case XVIDC_VM_1920x1080_60_P:
			width = 1920;
			height = 1080;
			break;
		case XVIDC_VM_1280x720_60_P:
			width = 1280;
			height = 720;
			break;
		case XVIDC_VM_640x480_60_P:
			width = 640;
			height = 480;
			break;

		default:
			xil_printf("XV_ConfigTpg - Invalid Video Mode \r\n");
			return;
	}

	/* Stop TPG */
	XV_tpg_DisableAutoRestart(pTpg);

	if (HdmiTxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_420) {
		width = HdmiTxSsVidStreamPtr->Timing.HActive / 2;
		height = HdmiTxSsVidStreamPtr->Timing.VActive;

		XV_tpg_Set_height(pTpg, height);
		XV_tpg_Set_width(pTpg, width);
		XV_tpg_Set_colorFormat(pTpg, XVIDC_CSF_RGB);
		XV_tpg_Set_bckgndId(pTpg, Pattern);
		XV_tpg_Set_ovrlayId(pTpg, 0);
	} else {
		XV_tpg_Set_height(pTpg, height);
		XV_tpg_Set_width(pTpg, width);
		XV_tpg_Set_colorFormat(pTpg, XVIDC_CSF_RGB);
		XV_tpg_Set_bckgndId(pTpg, Pattern);
		XV_tpg_Set_ovrlayId(pTpg, 0);
	}

	/*
	 * Enable/Disable pass through mode based on whether sensor
	 * or TPG is selected
	 */
	if (New_Cfg.VideoSrc == XVIDSRC_SENSOR) {

		if (Pipeline_Cfg.CameraPresent)
			XV_tpg_Set_enableInput(pTpg, 1);
		else
			XV_tpg_Set_enableInput(pTpg, 0);
	} else {
		XV_tpg_Set_enableInput(pTpg, 0);
	}

	XV_tpg_Set_passthruStartX(pTpg, 0);
	XV_tpg_Set_passthruStartY(pTpg, 0);
	XV_tpg_Set_passthruEndX(pTpg, width);
	XV_tpg_Set_passthruEndY(pTpg, height);

	/* Start TPG */
	XV_tpg_EnableAutoRestart(pTpg);
	XV_tpg_Start(pTpg);
}

/*****************************************************************************/
/**
 *
 * This function resets TPG IP.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void ResetTpg(void) {
	XGpio_SetDataDirection(&Gpio_Tpg_resetn, 1, 0);
	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, 0);
	usleep(1000);
	XGpio_DiscreteWrite(&Gpio_Tpg_resetn, 1, 1);
	usleep(1000);
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
 *****************************************************************************/
int I2cMux(void) {
	u8 Buffer;
	int Status;

	/* Select SI5324 clock generator */
	Buffer = 0x80;
	Status = XIic_Send((XPAR_IIC_1_BASEADDR), (I2C_MUX_ADDR),
				(u8 *) &Buffer, 1, (XIIC_STOP));

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
 * @return	Zero if error in programming external clock ele '1' if success
 *
 * @note	None.
 *
 *****************************************************************************/
int I2cClk(u32 InFreq, u32 OutFreq) {
	int Status;

	/* Free running mode */
	if (InFreq == 0) {

		Status = Si5324_SetClock((XPAR_IIC_1_BASEADDR),
						(I2C_CLK_ADDR),
						(SI5324_CLKSRC_XTAL),
						(SI5324_XTAL_FREQ),
						OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming free mode SI5324\n\r");
			return 0;
		}
	}

	/* Locked mode */
	else {
		Status = Si5324_SetClock((XPAR_IIC_1_BASEADDR),
						(I2C_CLK_ADDR),
						(SI5324_CLKSRC_CLK1),
						InFreq,
						OutFreq);

		if (Status != (SI5324_SUCCESS)) {
			print("Error programming locked mode SI5324\n\r");
			return 0;
		}
	}

	return 1;
}

/*****************************************************************************/
/**
 *
 * This function reports the stream mode
 *
 * @param	HdmiTxSsPtr is a pointer to the XV_HdmiTxSs instance.
 * @param	IsPassThrough is a flag to represent passthrough or colorbar mode.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void ReportStreamMode(XV_HdmiTxSs *HdmiTxSsPtr, u8 IsPassThrough) {
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(HdmiTxSsPtr);

	if (IsPassThrough) {
		print("--------\n\rPass-Through :\n\r");
	} else {
		print("--------\n\rColorbar :\n\r");
	}

	XVidC_ReportStreamInfo(HdmiTxSsVidStreamPtr);
	print("--------\n\r");
}

/*****************************************************************************/
/**
 *
 * This function outputs the video timing , Audio, Link Status, HDMI RX state
 * of HDMI RX core. In addition, it also prints information about HDMI TX, and
 * HDMI GT cores.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void Info(void) {
	print("\n\r-----\n\r");
	print("Info\n\r");
	print("-----\n\r\n\r");
	print("GT status\n\r");
	print("---------\n\r");
	xil_printf("TX reference clock frequency %0d Hz\n\r",
			XVphy_ClkDetGetRefClkFreqHz(&Vphy, XVPHY_DIR_TX));
	xil_printf("RX reference clock frequency %0d Hz\n\r",
			XVphy_ClkDetGetRefClkFreqHz(&Vphy, XVPHY_DIR_RX));
	if (Vphy.Config.DruIsPresent == (TRUE)) {
		xil_printf("DRU reference clock frequency %0d Hz\n\r",
				XVphy_DruGetRefClkFreqHz(&Vphy));
	}
	XVphy_HdmiDebugInfo(&Vphy, 0, XVPHY_CHANNEL_ID_CH1);

}

/*****************************************************************************/
/**
 *
 * This function is called when a TX connect event has occurred.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void TxConnectCallback(void *CallbackRef) {
	XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *) CallbackRef;

	/* Pass-through */
	if (IsPassThrough) {
		StartTxAfterRxFlag = (TRUE);  /* Restart stream */
	}

	/* Colorbar */
	else {
#if(LOOPBACK_MODE_EN != 1)
		TxRestartColorbar = (TRUE);   /* Restart stream */
		TxBusy = (FALSE);
#endif
	}

	if (HdmiTxSsPtr->IsStreamConnected == (FALSE)) {
		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_TX, (FALSE));
	} else {
		/* Check HDMI sink version */
		XV_HdmiTxSs_DetectHdmi20(HdmiTxSsPtr);
		XVphy_IBufDsEnable(&Vphy, 0, XVPHY_DIR_TX, (TRUE));
	}
}

/*****************************************************************************/
/**
 *
 * This function is called when the GT TX reference input clock has changed.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void VphyHdmiTxInitCallback(void *CallbackRef) {
	XV_HdmiTxSs_RefClockChangeInit(&HdmiTxSs);
}

/*****************************************************************************/
/**
 *
 * This function is called when the GT TX has been initialized
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void VphyHdmiTxReadyCallback(void *CallbackRef) {
}

/*****************************************************************************/
/**
 *
 * This function is called when a TX vsync has occurred.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void TxVsCallback(void *CallbackRef) {
}

/*****************************************************************************/
/**
 *
 * This function is called when the TX stream is up.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void TxStreamUpCallback(void *CallbackRef) {
	xil_printf("TX stream is up\n\r");
	XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *) CallbackRef;
	XVphy_PllType TxPllType;
	u64 TxLineRate;

	TxPllType = XVphy_GetPllType(&Vphy, 0, XVPHY_DIR_TX, XVPHY_CHANNEL_ID_CH1);
	if ((TxPllType == XVPHY_PLL_TYPE_CPLL)) {
		TxLineRate = Vphy.Quads[0].Plls[0].LineRateHz;
	} else if ((TxPllType == XVPHY_PLL_TYPE_QPLL0)) {
		TxLineRate = Vphy.Quads[0].Plls[XVPHY_CHANNEL_ID_CMN0
			- XVPHY_CHANNEL_ID_CH1].LineRateHz;
	} else {
		TxLineRate = Vphy.Quads[0].Plls[XVPHY_CHANNEL_ID_CMN1
			- XVPHY_CHANNEL_ID_CH1].LineRateHz;
	}

	i2c_dp159(&Vphy, 0, TxLineRate);

	/* Enable TX TMDS clock*/
	XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_TX, (TRUE));

	/* Copy Sampling Rate */
	XV_HdmiTxSs_SetSamplingRate(HdmiTxSsPtr, Vphy.HdmiTxSampleRate);
	ReportStreamMode(HdmiTxSsPtr, IsPassThrough);

	/* Clear TX busy flag */
	TxBusy = (FALSE);

}

/*****************************************************************************/
/**
 *
 * This function is called when the TX stream is down.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void TxStreamDownCallback(void *CallbackRef) {

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
 *****************************************************************************/
void StartTxAfterRx(void) {

	/* clear start */
	StartTxAfterRxFlag = (FALSE);

	/* Disable TX TDMS clock */
	XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_TX, (FALSE));

	XV_HdmiTxSs_StreamStart(&HdmiTxSs);

	/* Enable RX clock forwarding */
	XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_RX, (TRUE));

	/* Program external clock generator in locked mode */
	/* Only when the GT TX and RX are not coupled */
	if (!XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)) {
		I2cClk(Vphy.HdmiRxRefClkHz, Vphy.HdmiTxRefClkHz);
	}

	/* Video Pattern Generator */
	ResetTpg();
	XV_ConfigTpg(&Tpg);

	/* Enable TX TMDS Clock in bonded mode */
	if (XVphy_IsBonded(&Vphy, 0, XVPHY_CHANNEL_ID_CH1)) {
		XVphy_Clkout1OBufTdsEnable(&Vphy, XVPHY_DIR_TX, (TRUE));
	}
}

/*****************************************************************************/
/**
 *
 * This function setups the interrupt system.
 *
 * @return	XST_SUCCESS if interrupt setup was successful else error code
 *
 * @note	None.
 *
 *****************************************************************************/
int SetupInterruptSystem(void) {
	int Status;

	XScuGic *IntcInstPtr = &Intc;

	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */

	XScuGic_Config *IntcCfgPtr;
	IntcCfgPtr = XScuGic_LookupConfig(PSU_INTR_DEVICE_ID);
	if (IntcCfgPtr == NULL) {
		print("ERR:: Interrupt Controller not found");
		return (XST_DEVICE_NOT_FOUND);
	}
	Status = XScuGic_CfgInitialize(IntcInstPtr, IntcCfgPtr,
			IntcCfgPtr->CpuBaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("Intc initialization failed!\r\n");
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */

	Status = XScuGic_Connect(IntcInstPtr, IIC_SENSOR_INTR_ID,
				(XInterruptHandler) XIic_InterruptHandler,
				(void *) &IicSensor);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XScuGic_Connect(IntcInstPtr, XPAR_INTC_0_V_FRMBUF_WR_0_VEC_ID,
					(XInterruptHandler) XVFrmbufWr_InterruptHandler,
					(void *) &frmbufwr);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XScuGic_Connect(IntcInstPtr, XPAR_INTC_0_V_FRMBUF_RD_0_VEC_ID,
					(XInterruptHandler) XVFrmbufRd_InterruptHandler,
					(void *) &frmbufrd);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable IO expander and sensor IIC interrupts */
	XScuGic_Enable(IntcInstPtr, IIC_SENSOR_INTR_ID);
	XScuGic_Enable(IntcInstPtr, XPAR_INTC_0_V_FRMBUF_WR_0_VEC_ID);
	XScuGic_Enable(IntcInstPtr, XPAR_INTC_0_V_FRMBUF_RD_0_VEC_ID);
	Xil_ExceptionInit();

	/*Register the interrupt controller handler with the exception table.*/
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			(XScuGic *) IntcInstPtr);

	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
 *
 * This function enables the ColorBar
 *
 * @param	VphyPtr is a pointer to the VPHY core instance.
 * @param	HdmiTxSsPtr is a pointer to the XV_HdmiTxSs instance.
 * @param	VideoMode is the resolution that is to be displayed
 * @param	ColorFormat that can be RGB888 or YUV422 or YUV420
 * @param	Bpc is the Bits per color that can be 8 or 10 or 12
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void EnableColorBar(XVphy *VphyPtr, XV_HdmiTxSs *HdmiTxSsPtr,
		XVidC_VideoMode VideoMode, XVidC_ColorFormat ColorFormat,
		XVidC_ColorDepth Bpc) {

	u32 TmdsClock = 0;
	u32 Result;
	u32 PixelClock;

	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(HdmiTxSsPtr);

	/*
	 * If the RX is master,
	 * then the TX has to follow the RX reference clock
	 * In this case the TX only color bar can't be displayed
	 */
	if (XVphy_IsBonded(VphyPtr, 0, XVPHY_CHANNEL_ID_CH1)) {
xil_printf("Both GT RX and GT TX are clocked by the RX reference clock.\n\r");
xil_printf("Please connect a source to the RX input\n\r");
	}

	/* Independent TX reference clock */
	else {
		/* Check if the TX isn't busy already */
		if (!TxBusy) {
			/* Set TX busy flag */
			TxBusy = (TRUE);

			if (VideoMode < XVIDC_VM_NUM_SUPPORTED) {
				xil_printf("Starting colorbar\n\r");
				IsPassThrough = 0;

				/* Disable TX TDMS clock */
				XVphy_Clkout1OBufTdsEnable(VphyPtr,
								XVPHY_DIR_TX,
								(FALSE));

				/* Get pixel clock */
				PixelClock = XVidC_GetPixelClockHzByVmId(VideoMode);

				/*
				 * In YUV420 the pixel clock is actually the
				 * half of the reported pixel clock
				 */
				if (ColorFormat == XVIDC_CSF_YCRCB_420) {
					PixelClock = PixelClock / 2;
				}
			}

			TmdsClock = XV_HdmiTxSs_SetStream(HdmiTxSsPtr,
								VideoMode,
								ColorFormat,
								Bpc, NULL);

			/* Set TX reference clock */
			VphyPtr->HdmiTxRefClkHz = TmdsClock;

			/* Set GT TX parameters */
			Result = XVphy_SetHdmiTxParam(VphyPtr, 0,
							XVPHY_CHANNEL_ID_CHA,
							HdmiTxSsVidStreamPtr->PixPerClk,
							HdmiTxSsVidStreamPtr->ColorDepth,
							HdmiTxSsVidStreamPtr->ColorFormatId);

			if (Result == (XST_FAILURE)) {
				xil_printf("Unable to set requested TX video resolution.\n\r");
				xil_printf("Returning to previously TX video resolution.\n\r");
			}

			/* Disable RX clock forwarding */
			XVphy_Clkout1OBufTdsEnable(VphyPtr, XVPHY_DIR_RX,
							(FALSE));

			/* Program external clock generator in free running mode */
			I2cClk(0, VphyPtr->HdmiTxRefClkHz);
		}
	}
}

/*****************************************************************************/
/**
* This function asserts a callback error.
*
* @param	File is current file name.
* @param	Line is line number of the asserted callback.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Xil_AssertCallbackRoutine(u8 *File, s32 Line) {
	xil_printf("Assertion in File %s, on line %0d\n\r", File, Line);
}

/*****************************************************************************/
/**
* This function resets IMX274 camera sensor.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void CamReset(void)
{
	Xil_Out32(GPIO_SENSOR, 0x07);
	Xil_Out32(GPIO_SENSOR, 0x06);
	Xil_Out32(GPIO_SENSOR, 0x07);
}

/*****************************************************************************/
/**
* This function resets image processing pipe.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void Reset_IP_Pipe(void)
{

	Xil_Out32(GPIO_IP_RESET1, 0x00);
	Xil_Out32(GPIO_IP_RESET2, 0x00);
	usleep(1000);
	Xil_Out32(GPIO_IP_RESET1, 0x01);
	Xil_Out32(GPIO_IP_RESET2, 0x03);

}

/*****************************************************************************/
/**
 *
 * Main function to initialize the video pipleline and process user input
 *
 * @return	XST_SUCCESS if MIPI example was successful else XST_FAILURE
 *
 * @note	None.
 *
 *****************************************************************************/
int main(void)
{
	u8 Response;
	u32 Status;
	XVphy_Config *XVphyCfgPtr;
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;

	/* Setup the default pipeline configuration parameters */
	/* Look for Default ColorDepth manual setting just after
	 * SetColorDepth
	 */

	Pipeline_Cfg.ActiveLanes = 4;
	Pipeline_Cfg.VideoSrc = XVIDSRC_SENSOR;

	/* Default DSI */
	Pipeline_Cfg.VideoDestn = XVIDDES_DSI;

	Pipeline_Cfg.Live = TRUE;

	/* Vertical and Horizontal flip don't work */
	Pipeline_Cfg.Vflip = FALSE;
	Pipeline_Cfg.Hflip = FALSE;

	/* Video pipeline configuration from user */
	Pipeline_Cfg.CameraPresent = TRUE;
	Pipeline_Cfg.DSIDisplayPresent = TRUE;

	/* Default Resolution that to be displayed */
	Pipeline_Cfg.VideoMode = XVIDC_VM_3840x2160_30_P;

	Xil_DCacheDisable();

	xil_printf("\n\r\n\r");
	xil_printf(TXT_GREEN);
	xil_printf("--------------------------------------------------\r\n");
	xil_printf("------  MIPI Reference Pipeline Design  ----------\r\n");
	xil_printf("---------  (c) 2017 by Xilinx, Inc.  -------------\r\n");
	xil_printf("--------------------------------------------------\r\n");
	xil_printf(TXT_RST);

	xil_printf(TXT_YELLOW);
	xil_printf("--------------------------------------------------\r\n");
	xil_printf("Build %s - %s\r\n", __DATE__, __TIME__);
	xil_printf("--------------------------------------------------\r\n");
	xil_printf(TXT_RST);

xil_printf("Please answer the following questions about the hardware setup.");
xil_printf("\r\n");

	do {
		xil_printf("Is the camera sensor connected? (Y/N)\r\n");

		Response = XUartPs_RecvByte(UART_BASEADDR);

		XUartPs_SendByte(UART_BASEADDR, Response);

		if ((Response == 'Y') || (Response == 'y')) {
			Pipeline_Cfg.CameraPresent = TRUE;
			break;
		} else if ((Response == 'N') || (Response == 'n')) {
			Pipeline_Cfg.CameraPresent = FALSE;
			break;
		}

	} while (1);

	if (Pipeline_Cfg.CameraPresent)
		print(TXT_GREEN);
	else
		print(TXT_RED);

	xil_printf("\r\nCamera sensor is set as %s\r\n",
			(Pipeline_Cfg.CameraPresent) ? "Connected" : "Disconnected");
	print(TXT_RST);

	if (!Pipeline_Cfg.CameraPresent) {
		Pipeline_Cfg.VideoSrc = XVIDSRC_TPG;
		xil_printf("Setting TPG as source in absence of Camera sensor.\r\n");
	}

	do {
		xil_printf("Is the DSI Display panel connected? (Y/N)\r\n");

		Response = XUartPs_RecvByte(UART_BASEADDR);

		XUartPs_SendByte(UART_BASEADDR, Response);

		if ((Response == 'Y') || (Response == 'y')) {
			Pipeline_Cfg.DSIDisplayPresent = TRUE;
			break;
		} else if ((Response == 'N') || (Response == 'n')) {
			Pipeline_Cfg.DSIDisplayPresent = FALSE;
			break;
		}

	} while (1);

	if (Pipeline_Cfg.DSIDisplayPresent)
		xil_printf(TXT_GREEN);
	else
		xil_printf(TXT_RED);

	xil_printf("\r\nDSI Display panel is set as %s\r\n",
			(Pipeline_Cfg.DSIDisplayPresent) ? "Connected" : "Disconnected");
	xil_printf(TXT_RST);

	StartTxAfterRxFlag = (FALSE);
	TxBusy = (FALSE);
	TxRestartColorbar = (FALSE);

	/* Start in color bar */
	IsPassThrough = 0;

	/* Initialize platform */
	init_platform();

	/* Initialize external clock generator for HDMI through I2C*/
	Si5324_Init(XPAR_IIC_1_BASEADDR, I2C_CLK_ADDR);

	/* Initialize IIC */
	Status = InitIIC();
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "\n\rIIC Init Failed \n\r" TXT_RST);
		return XST_FAILURE;
	}

	/* Initialize IRQ */
	Status = SetupInterruptSystem();
	if (Status == XST_FAILURE) {
		print(TXT_RED "IRQ init failed.\n\r" TXT_RST);
		return XST_FAILURE;
	}

	/* IIC interrupt handlers */
	SetupIICIntrHandlers();

	/* Reset Demosaic, Gamma_Lut and CSC IPs */
	Reset_IP_Pipe();

	/* Initialize DSI IP */
	Status = SetupDSI();
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "SetupDSI failed status = %x.\r\n" TXT_RST,
				 Status);
		return XST_FAILURE;
	}

	/* Initialize VProcSS Scalar IP */
	InitVprocSs_Scaler(1);
	xil_printf("\r\nInitVprocSs_Scaler Done \n\r");
	InitDSI();
	xil_printf("\r\nInitDSI Done \n\r");

	/* Initialize GPIO IP for Strem Switch Mux signal*/
	Status = InitStreamMuxGpio();
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "Tready GPIO Init failed status = %x.\r\n"
				 TXT_RST, Status);
		return XST_FAILURE;
	}

	/* Initialize CSIRXSS  */
	Status = InitializeCsiRxSs();
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "CSI Rx Ss Init failed status = %x.\r\n"
				 TXT_RST, Status);
		return XST_FAILURE;
	}

	config_csi_cap_path();



	/* MIPI colour depth in bits per clock */
	SetColorDepth();

	/*
	 * Initialize HDMI TX Subsystem
	 */
	XV_HdmiTxSs_ConfigPtr = XV_HdmiTxSs_LookupConfig(HDMI_TX_SS_DEVICE_ID);

	if (XV_HdmiTxSs_ConfigPtr == NULL) {
		HdmiTxSs.IsReady = 0;
	}

	/* Initialize top level and all included sub-cores */
	Status = XV_HdmiTxSs_CfgInitialize(&HdmiTxSs, XV_HdmiTxSs_ConfigPtr,
			XV_HdmiTxSs_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "ERR: HDMI TX Subsystem Initialization "
					"failed %d\r\n" TXT_RST, Status);
	}

	/* Register HDMI TX SS Interrupt Handler with Interrupt Controller */
	Status |= XScuGic_Connect(&Intc,
			HDMI_TX_SS_INTR_ID,
			(XInterruptHandler) XV_HdmiTxSS_HdmiTxIntrHandler,
			(void *) &HdmiTxSs);

	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc, HDMI_TX_SS_INTR_ID);
	} else {
		xil_printf("ERR: Unable to register HDMI TX interrupt handler");
		print("HDMI TX SS initialization error\n\r");
		return XST_FAILURE;
	}

	/* HDMI TX SS callback setup */
	XV_HdmiTxSs_SetCallback(&HdmiTxSs, XV_HDMITXSS_HANDLER_CONNECT,
			TxConnectCallback, (void *) &HdmiTxSs);
	XV_HdmiTxSs_SetCallback(&HdmiTxSs, XV_HDMITXSS_HANDLER_VS, TxVsCallback,
			(void *) &HdmiTxSs);

	XV_HdmiTxSs_SetCallback(&HdmiTxSs, XV_HDMITXSS_HANDLER_STREAM_UP,
			TxStreamUpCallback, (void *) &HdmiTxSs);

	/*
	 * Initialize Video PHY
	 * The GT needs to be initialized after the HDMI RX and TX.
	 * The reason for this is the GtRxInitStartCallback
	 * calls the RX stream down callback.
	 */

	XVphyCfgPtr = XVphy_LookupConfig(VID_PHY_DEVICE_ID);
	if (XVphyCfgPtr == NULL) {
		print("Video PHY device not found\n\r\r");
		return XST_FAILURE;
	}

	/* Register VPHY Interrupt Handler */
	Status = XScuGic_Connect(&Intc,	VID_PHY_INTR_ID,
				(XInterruptHandler) XVphy_InterruptHandler,
				(void *) &Vphy);

	if (Status != XST_SUCCESS) {
		print("HDMI VPHY Interrupt Vec ID not found!\n\r");
		return XST_FAILURE;
	}

	/* Initialize HDMI VPHY */
	Status = XVphy_Hdmi_CfgInitialize(&Vphy, 0, XVphyCfgPtr);
	if (Status != XST_SUCCESS) {
		print("HDMI VPHY initialization error\n\r");
		return XST_FAILURE;
	}

	/* Enable VPHY Interrupt */
	XScuGic_Enable(&Intc, VID_PHY_INTR_ID);

	/* VPHY callback setup */
	XVphy_SetHdmiCallback(&Vphy, XVPHY_HDMI_HANDLER_TXINIT,
			VphyHdmiTxInitCallback, (void *) &Vphy);
	XVphy_SetHdmiCallback(&Vphy, XVPHY_HDMI_HANDLER_TXREADY,
			VphyHdmiTxReadyCallback, (void *) &Vphy);

	/* Initialize GPIO for Tpg Reset */
	Gpio_Tpg_resetn_ConfigPtr = XGpio_LookupConfig(GPIO_TPG_RESET_DEVICE_ID);

	if (Gpio_Tpg_resetn_ConfigPtr == NULL) {
		Gpio_Tpg_resetn.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XGpio_CfgInitialize(&Gpio_Tpg_resetn,
					Gpio_Tpg_resetn_ConfigPtr,
					Gpio_Tpg_resetn_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for TPG Reset ");
		xil_printf("Initialization failed %d\r\n", Status);
		return (XST_FAILURE);
	}

	/* Reset TPG IP */
	ResetTpg();

	/* Initialize TPG IP */
	Tpg_ConfigPtr = XV_tpg_LookupConfig(V_TPG_DEVICE_ID);

	if (Tpg_ConfigPtr == NULL) {
		Tpg.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XV_tpg_CfgInitialize(&Tpg, Tpg_ConfigPtr,
			Tpg_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: TPG Initialization failed %d\r\n", Status);
		return (XST_FAILURE);
	}

	print("---------------------------------\r\n");

	/* Enable exceptions. */
	Xil_AssertSetCallback((Xil_AssertCallback) Xil_AssertCallbackRoutine);
	Xil_ExceptionEnable();

	/* Reset Camera Sensor module through GPIO */
	xil_printf("Disable CAM_RST of Sensor through GPIO\r\n");
	CamReset();
	xil_printf("Sensor is  Enabled\r\n");


	/* Program Camera sensor */
		Status = SetupCameraSensor();
		if (Status != XST_SUCCESS) {
			xil_printf("Failed to setup Camera sensor\r\n");
			return XST_FAILURE;
		}


	start_csi_cap_pipe(Pipeline_Cfg.VideoMode);

	InitImageProcessingPipe();

	/* Set colorbar pattern */
	ResetTpg();
	Pattern = XTPG_BKGND_COLOR_BARS;
	XV_ConfigTpg(&Tpg);
	/*If HDMI is disconnected then make DSI the default video destination*/
	if (HdmiTxSs.IsStreamConnected == (FALSE)) {
		print(TXT_RED);
		print("HDMI is disconnected.\r\n");
		print(TXT_RST);
		if (Pipeline_Cfg.DSIDisplayPresent)
			Pipeline_Cfg.VideoDestn = XVIDDES_DSI;
	} else {
		if (Pipeline_Cfg.VideoDestn == XVIDDES_HDMI) {
			EnableColorBar(&Vphy, &HdmiTxSs, Pipeline_Cfg.VideoMode,
					XVIDC_CSF_RGB, Pipeline_Cfg.ColorDepth);
		}
	}

	/*If DSI Display is not present, set HDMI as default video destination*/
	if (!Pipeline_Cfg.DSIDisplayPresent) {
		print("Setting HDMI as default destination as DSI Display panel"
				" is absent\r\n");
		Pipeline_Cfg.VideoDestn = XVIDDES_HDMI;
	}

	/* Default DSI as display */
	if (Pipeline_Cfg.VideoDestn == XVIDDES_DSI)
	{
		xil_printf("\n\rEnabling DSI Tready ... ");
		SelectDSIOutput();
		xil_printf("Enabled \n\r ");
	}
	else
	{
		xil_printf("\n\rEnabling HDMI Tready ... ");
		SelectHDMIOutput();
		xil_printf("Enabled \n\r ");
	}

	/* Start Camera Sensor to capture video */
	StartSensor();

	/* Initialize menu */
	XMipi_MenuInitialize(&HdmiMenu, UART_BASEADDR);


	/* Enable DSI IP */
	EnableDSI();



	New_Cfg = Pipeline_Cfg;

	/* Print the Pipe line configuration */
	PrintPipeConfig();

	/* Main loop */
	do {

		/* Switch to DSI on HDMI monitor removal */
		if ((HdmiTxSs.IsStreamConnected == (FALSE)) &&
				(Pipeline_Cfg.DSIDisplayPresent)) {
			New_Cfg.VideoDestn = XVIDDES_DSI;
		}

		/* if camera is not present, tpg is source */
		if (Pipeline_Cfg.CameraPresent == FALSE) {
			New_Cfg.VideoSrc = XVIDSRC_TPG;
		}

		if (New_Cfg.VideoSrc != Pipeline_Cfg.VideoSrc) {
			usleep(1000000);

			if (New_Cfg.VideoSrc == XVIDSRC_SENSOR) {
				xil_printf("Set Sensor as source\r\n");
				Pipeline_Cfg.VideoSrc = XVIDSRC_SENSOR;
			}

			if (New_Cfg.VideoSrc == XVIDSRC_TPG) {
				xil_printf("Set TPG as source\r\n");
				Pipeline_Cfg.VideoSrc = XVIDSRC_TPG;
			}

			/* Reset TPG IP */
			ResetTpg();
			/* Configure TPG IP */
			XV_ConfigTpg(&Tpg);

			PrintPipeConfig();
		}

		if (New_Cfg.VideoDestn != Pipeline_Cfg.VideoDestn) {

			if (New_Cfg.VideoDestn == XVIDDES_HDMI) {
				xil_printf("Set HDMI as destination \r\n");
				Pipeline_Cfg.VideoDestn = XVIDDES_HDMI;

				/* Make Video Scaler TREADY High */
				Shutdown_DSI();
				SelectHDMIOutput();
				Reconfigure_HDMI();
			}

			if (New_Cfg.VideoDestn == XVIDDES_DSI) {
				xil_printf("Set DSI as destination \r\n");
				Pipeline_Cfg.VideoDestn = XVIDDES_DSI;

				/* Make TREADY of HDMI as high */
				SelectDSIOutput();
				Reconfigure_DSI();
			}

			PrintPipeConfig();
		}

		if (TxRestartColorbar) {
			TxRestartColorbar = (FALSE);    /* Clear flag */
			HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(&HdmiTxSs);
			if (HdmiTxSs.IsStreamConnected == (TRUE)) {
				if (Pipeline_Cfg.VideoDestn == XVIDDES_HDMI) {
					EnableColorBar(&Vphy, &HdmiTxSs,
							Pipeline_Cfg.VideoMode,
							HdmiTxSsVidStreamPtr->ColorFormatId,
							Pipeline_Cfg.ColorDepth);
				}
			}
		}

		/* HDMI menu */
		XMipi_MenuProcess(&HdmiMenu);

	} while (1);

	return 0;
}
