/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file mipi_cfg.c
*
* This file demonstrates how to use Xilinx MIPI CSI RX Subsystem, Demosaic
* GammaLUT,VPSS CSC, VPSS Scaler , Frame Buffer Read and Write
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include "sleep.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xcsiss.h"
#include "xvidc.h"
#include "xv_frmbufwr_l2.h"
#include "xv_frmbufrd_l2.h"
#include "xvprocss.h"
#include "xgpio_l.h"
#include "xiic.h"
#include "xscugic.h"
#include "xuartpsv.h"
#include "sensor_cfgs.h"

/***************** Macros (Inline Functions) Definitions *********************/
#define HDMI_H_RES		(1920)
#define HDMI_V_RES		(1080)

#define DSI_H_RES		(1920)
#define DSI_V_RES		(1200)

#define UART_BASEADDR	XPAR_XUARTPSV_0_BASEADDR

#define MIPICSI_BASEADDR XPAR_CSISS_0_BASEADDR
#define XCSIRXSS_DEVICE_ID	XPAR_CSISS_0_DEVICE_ID
#define GPIO_SENSOR		XPAR_GPIO_3_BASEADDR

#define GPIO_IP_RESET	 XPAR_AXI_GPIO_0_BASEADDR

#define VPROCSSCSC_BASE	XPAR_XVPROCSS_0_BASEADDR
#define DEMOSAIC_BASE	XPAR_XV_DEMOSAIC_0_S_AXI_CTRL_BASEADDR
#define VGAMMALUT_BASE	XPAR_XV_GAMMA_LUT_0_S_AXI_CTRL_BASEADDR


#define IIC_SENSOR_DEV_ID	XPAR_IIC_1_DEVICE_ID
#define XVPROCSS_DEVICE_ID	XPAR_XVPROCSS_1_DEVICE_ID

#define PSU_INTR_DEVICE_ID XPAR_PSV_ACPU_GIC_DEVICE_ID

// Group 0 84 +32 , Group1 - 92 +32
#define XPAR_INTC_0_V_SENSOR_IIC_0_VEC_ID  XPAR_FABRIC_IIC_1_VEC_ID
#define XPAR_INTC_0_CSIRXSS_0_VEC_ID XPAR_FABRIC_MIPICSISS_0_VEC_ID
#define XPAR_INTC_0_V_FRMBUF_WR_0_VEC_ID XPAR_FABRIC_V_FRMBUF_WR_0_VEC_ID
#define XPAR_INTC_0_V_FRMBUF_RD_0_VEC_ID XPAR_FABRIC_V_FRMBUF_RD_0_VEC_ID


#define DDR_BASEADDR XPAR_AXI_NOC_DDR_LOW_0_BASEADDR

#define BUFFER_BASEADDR0 (DDR_BASEADDR + (0x10000000))
#define BUFFER_BASEADDR1 (DDR_BASEADDR + (0x20000000))
#define BUFFER_BASEADDR2 (DDR_BASEADDR + (0x30000000))
#define BUFFER_BASEADDR3 (DDR_BASEADDR + (0x40000000))
#define BUFFER_BASEADDR4 (DDR_BASEADDR + (0x50000000))
#define CHROMA_ADDR_OFFSET   (0x01000000U)

#define PAGE_SIZE	16


/**************************** Type Definitions *******************************/
typedef u8 AddressType;

u8 SensorIicAddr; /* Variable for storing Eeprom IIC address */

#define SENSOR_ADDR         (0x34>>1)	/* for IMX274 Vision */

#define IIC_MUX_ADDRESS 		0x75
#define IIC_EEPROM_CHANNEL		0x01	/* 0x08 */

XIic IicSensor; /* The instance of the IIC device. */

volatile u8 TransmitComplete; /* Flag to check completion of Transmission */
volatile u8 ReceiveComplete; /* Flag to check completion of Reception */

u8 WriteBuffer[sizeof(AddressType) + PAGE_SIZE];
u8 ReadBuffer[PAGE_SIZE]; /* Read buffer for reading a page. */


extern XScuGic     Intc;

extern XPipeline_Cfg Pipeline_Cfg;
XCsiSs CsiRxSs;


XV_FrmbufWr_l2     frmbufwr;
XV_FrmbufRd_l2     frmbufrd;



XVidC_VideoStream  VidStream;
XVidC_ColorFormat  Cfmt;
XVprocSs scaler_new_inst;
XVidC_VideoStream  StreamOut;


u32 frame_array[5] = {BUFFER_BASEADDR0, BUFFER_BASEADDR1, BUFFER_BASEADDR2,
		              BUFFER_BASEADDR3, BUFFER_BASEADDR4};
u32 rd_ptr = 4 ;
u32 wr_ptr = 0 ;
u64 XVFRMBUFRD_BUFFER_BASEADDR;
u64 XVFRMBUFWR_BUFFER_BASEADDR;
u32 frmrd_start  = 0;
u32 frm_cnt = 0;
u32 frm_cnt1 = 0;


void start_hdmi(XVidC_VideoMode VideoMode);
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
	Xil_Out32(GPIO_IP_RESET, 0xFFFFFFFF);
	Xil_Out32(GPIO_IP_RESET, 0x00000000);
	Xil_Out32(GPIO_IP_RESET, 0xFFFFFFFF);
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
 * This function programs colour space converter with the given width and height
 *
 * @param	width is Hsize of a packet in pixels.
 * @param	height is number of lines of a packet.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void ConfigCSC(u32 width , u32 height)
{
	Xil_Out32((VPROCSSCSC_BASE + 0x0010), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x0018), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x0050), 0x1000);
	Xil_Out32((VPROCSSCSC_BASE + 0x0058), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x0060), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x0068), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x0070), 0x1000);
	Xil_Out32((VPROCSSCSC_BASE + 0x0078), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x0080), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x0088), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x0090), 0x1000);
	Xil_Out32((VPROCSSCSC_BASE + 0x0098), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x00a0), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x00a8), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x00b0), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x00b8), 0xff  );
	Xil_Out32((VPROCSSCSC_BASE + 0x0020), width );
	Xil_Out32((VPROCSSCSC_BASE + 0x0028), height );
	Xil_Out32((VPROCSSCSC_BASE + 0x0000), 0x81  );

}

/*****************************************************************************/
/**
 * This function programs colour space converter with the given width and height
 *
 * @param	width is Hsize of a packet in pixels.
 * @param	height is number of lines of a packet.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void ConfigDemosaic(u32 width , u32 height)
{
	Xil_Out32((DEMOSAIC_BASE + 0x10), width );
	Xil_Out32((DEMOSAIC_BASE + 0x18), height );
	Xil_Out32((DEMOSAIC_BASE + 0x20), 0x0   );
	Xil_Out32((DEMOSAIC_BASE + 0x28), 0x0   );
	Xil_Out32((DEMOSAIC_BASE + 0x00), 0x81   );

}

/*****************************************************************************/
/**
 * This function programs colour space converter with the given width and height
 *
 * @param	width is Hsize of a packet in pixels.
 * @param	height is number of lines of a packet.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void ConfigGammaLut(u32 width , u32 height)
{
	u32 count;
	Xil_Out32((VGAMMALUT_BASE + 0x10), width );
	Xil_Out32((VGAMMALUT_BASE + 0x18), height );
	Xil_Out32((VGAMMALUT_BASE + 0x20), 0x0   );

	for(count=0; count < 0x200; count += 2)
	{
		Xil_Out16((VGAMMALUT_BASE + 0x800 + count), count/2 );
	}

	for(count=0; count < 0x200; count += 2)
	{
		Xil_Out16((VGAMMALUT_BASE + 0x1000 + count), count/2 );
	}

	for(count=0; count < 0x200; count += 2)
	{
		Xil_Out16((VGAMMALUT_BASE + 0x1800 + count), count/2 );
	}

	Xil_Out32((VGAMMALUT_BASE + 0x00), 0x81   );
}


/*****************************************************************************/
/**
 * This function initializes and configures VProcSS IP for scalar mode with the
 * given input and output width and height values.
 *
 * @param	count is a flag value to initialize IP only once.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void InitVprocSs_Scaler(int count,int width,int height)
{
	XVprocSs_Config* p_vpss_cfg;
	int status;
	int widthIn, heightIn, widthOut, heightOut;

		widthOut = DSI_H_RES;
		heightOut = DSI_V_RES;
	usleep(1000);
	/* Local variables */
	XVidC_VideoMode resIdIn, resIdOut;
	XVidC_VideoStream StreamIn;

	if (Pipeline_Cfg.VideoMode == XVIDC_VM_1280x720_60_P) {
		widthIn = 1280;
		heightIn = 720;
		StreamIn.FrameRate = XVIDC_FR_60HZ;
	}

	if (Pipeline_Cfg.VideoMode == XVIDC_VM_1920x1080_30_P) {
		widthIn = 1920;
		heightIn = 1080;
		StreamIn.FrameRate = XVIDC_FR_60HZ;
	}

	if (Pipeline_Cfg.VideoMode == XVIDC_VM_1920x1080_60_P) {
		widthIn = 1920;
		heightIn = 1080;
		StreamIn.FrameRate = XVIDC_FR_60HZ;
	}

	if (Pipeline_Cfg.VideoMode == XVIDC_VM_3840x2160_30_P) {
		widthIn = 3840;
		heightIn = 2160;
		StreamIn.FrameRate = XVIDC_FR_60HZ;
	}

	if (Pipeline_Cfg.VideoMode == XVIDC_VM_3840x2160_60_P) {
		widthIn = 3840;
		heightIn = 2160;
		StreamIn.FrameRate = XVIDC_FR_60HZ;
	}

	if (count) {
		p_vpss_cfg = XVprocSs_LookupConfig(XVPROCSS_DEVICE_ID);
		if (p_vpss_cfg == NULL) {
			xil_printf("ERROR! Failed to find VPSS-based scaler.");
			xil_printf("\n\r");
			return;
		}

		status = XVprocSs_CfgInitialize(&scaler_new_inst, p_vpss_cfg,
				p_vpss_cfg->BaseAddress);
		if (status != XST_SUCCESS) {
			xil_printf("ERROR! Failed to initialize VPSS-based ");
			xil_printf("scaler.\n\r");
			return;
		}
	}

	XVprocSs_Stop(&scaler_new_inst);

	/* Get resolution ID from frame size */
	resIdIn = XVidC_GetVideoModeId(widthIn, heightIn, StreamIn.FrameRate,
			FALSE);

	/* Setup Video Processing Subsystem */
	StreamIn.VmId = resIdIn;
	StreamIn.Timing.HActive = widthIn;
	StreamIn.Timing.VActive = heightIn;
	StreamIn.ColorFormatId = XVIDC_CSF_RGB;
	StreamIn.ColorDepth = (XVidC_ColorDepth)scaler_new_inst.Config.ColorDepth;
	StreamIn.PixPerClk = (XVidC_PixelsPerClock)scaler_new_inst.Config.PixPerClock;
	StreamIn.IsInterlaced = 0;

	status = XVprocSs_SetVidStreamIn(&scaler_new_inst, &StreamIn);
	if (status != XST_SUCCESS) {
		xil_printf("Unable to set input video stream parameters \
				correctly\r\n");
		return;
	}

	/* Get resolution ID from frame size */
	resIdOut = XVidC_GetVideoModeId(widthOut, heightOut, XVIDC_FR_60HZ,
					FALSE);



	StreamOut.VmId = resIdOut;
	StreamOut.Timing.HActive = widthOut;
	StreamOut.Timing.VActive = heightOut;
	StreamOut.ColorFormatId = XVIDC_CSF_RGB;
	StreamOut.ColorDepth = (XVidC_ColorDepth)scaler_new_inst.Config.ColorDepth;
	StreamOut.PixPerClk=(XVidC_PixelsPerClock)scaler_new_inst.Config.PixPerClock;
	StreamOut.FrameRate = XVIDC_FR_60HZ;
	StreamOut.IsInterlaced = 0;

	/* Get resolution ID from frame size */
	resIdOut = XVidC_GetVideoModeId(widthOut, heightOut, XVIDC_FR_60HZ,
					FALSE);



	XVprocSs_SetVidStreamOut(&scaler_new_inst, &StreamOut);
	if (status != XST_SUCCESS) {
		xil_printf("Unable to set output video stream parameters correctly\r\n");
		return;
	}

	status = XVprocSs_SetSubsystemConfig(&scaler_new_inst);
	if (status != XST_SUCCESS) {
		xil_printf("XVprocSs_SetSubsystemConfig failed %d\r\n", status);
		XVprocSs_LogDisplay(&scaler_new_inst);
		return;
	}

}

/*****************************************************************************/
/**
 * This Send handler is called asynchronously from an interrupt
 * context and indicates that data in the specified buffer has been sent.
 *
 * @param	InstancePtr is not used, but contains a pointer to the IIC
 *		device driver instance which the handler is being called for.
 *
 * @return	None.
 *
 * @note	None.
 *
 ****************F*************************************************************/
static void SendHandler(XIic *InstancePtr) {
	TransmitComplete = 0;
}

/*****************************************************************************/
/**
 * This Receive handler is called asynchronously from an interrupt
 * context and indicates that data in the specified buffer has been Received.
 *
 * @param	InstancePtr is not used, but contains a pointer to the IIC
 *		device driver instance which the handler is being called for.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
static void ReceiveHandler(XIic *InstancePtr) {
	ReceiveComplete = 0;
}

/*****************************************************************************/
/**
 * This Status handler is called asynchronously from an interrupt
 * context and indicates the events that have occurred.
 *
 * @param	InstancePtr is a pointer to the IIC driver instance for which
 *		the handler is being called for.
 * @param	Event indicates the condition that has occurred.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
static void StatusHandler(XIic *InstancePtr, int Event) {

}

/*****************************************************************************/
/**
 * This function writes a buffer of data to the IIC serial sensor.
 *
 * @param	ByteCount is the number of bytes in the buffer to be written.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/

int SensorWriteData(u16 ByteCount) {
	int Status;

	/* Set the defaults. */
	TransmitComplete = 1;

	IicSensor.Stats.TxErrors = 0;

	/* Start the IIC device. */
	Status = XIic_Start(&IicSensor);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* Send the Data. */
	Status = XIic_MasterSend(&IicSensor, WriteBuffer, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/* Wait till the transmission is completed. */
	while ((TransmitComplete) || (XIic_IsIicBusy(&IicSensor) == TRUE)) {


		if (IicSensor.Stats.TxErrors != 0) {

			/* Enable the IIC device. */
			Status = XIic_Start(&IicSensor);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if (!XIic_IsIicBusy(&IicSensor)) {
				/* Send the Data. */
				Status = XIic_MasterSend(&IicSensor,
								WriteBuffer,
								ByteCount);

				if (Status == XST_SUCCESS) {
					IicSensor.Stats.TxErrors = 0;
				}
			}
		}
	}
	/* Stop the IIC device. */
	Status = XIic_Stop(&IicSensor);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function reads data from the IIC serial Camera Sensor into a specified
 * buffer.
 *
 * @param	BufferPtr contains the address of the data buffer to be filled.
 * @param	ByteCount contains the number of bytes in the buffer to be read.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/

int SensorReadData(u8 *BufferPtr, u16 ByteCount) {
	int Status;

	/* Set the Defaults. */
	ReceiveComplete = 1;

	Status = SensorWriteData(2);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Start the IIC device. */
	Status = XIic_Start(&IicSensor);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Receive the Data. */
	Status = XIic_MasterRecv(&IicSensor, BufferPtr, ByteCount);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Wait till all the data is received. */
	while ((ReceiveComplete) || (XIic_IsIicBusy(&IicSensor) == TRUE)) {

		usleep(10);
	}



	/* Stop the IIC device. */
	Status = XIic_Stop(&IicSensor);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function setup Camera sensor programming wrt resolution selected
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/
int SetupCameraSensor(void) {
	int Status;
	u32 Index, MaxIndex;
	SensorIicAddr = SENSOR_ADDR;
	struct regval_list *sensor_cfg = NULL;

	/* If no camera present then return */
	if (Pipeline_Cfg.CameraPresent == FALSE) {
		xil_printf("%s - No camera present\r\n", __func__);
		return XST_SUCCESS;
	}

	/* Validate Pipeline Configuration */
	if ((Pipeline_Cfg.VideoMode == XVIDC_VM_3840x2160_30_P)
			&& (Pipeline_Cfg.ActiveLanes != 4)) {
		xil_printf("4K supports only 4 Lane configuration\r\n");
		return XST_FAILURE;
	}

	if ((Pipeline_Cfg.VideoMode == XVIDC_VM_1920x1080_60_P)
			&& (Pipeline_Cfg.ActiveLanes == 1)) {
		xil_printf("1080p doesn't support 1 Lane configuration\r\n");
		return XST_FAILURE;
	}

	Status = XIic_SetAddress(&IicSensor, XII_ADDR_TO_SEND_TYPE,
					SensorIicAddr);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Select the sensor configuration based on resolution and lane */
	switch (Pipeline_Cfg.VideoMode) {

		case XVIDC_VM_1280x720_60_P:
			MaxIndex = length_imx274_config_720p_60fps_regs;
			sensor_cfg = imx274_config_720p_60fps_regs;
			break;

		case XVIDC_VM_1920x1080_30_P:
			MaxIndex = length_imx274_config_1080p_60fps_regs;
			sensor_cfg = imx274_config_1080p_60fps_regs;
			break;

		case XVIDC_VM_1920x1080_60_P:
			MaxIndex = length_imx274_config_1080p_60fps_regs;
			sensor_cfg = imx274_config_1080p_60fps_regs;
			break;

		case XVIDC_VM_3840x2160_30_P:
			MaxIndex = length_imx274_config_4K_30fps_regs;
			sensor_cfg = imx274_config_4K_30fps_regs;
			break;

		case XVIDC_VM_3840x2160_60_P:
			MaxIndex = length_imx274_config_4K_60fps_regs;
			sensor_cfg = imx274_config_4K_60fps_regs;
			break;


		default:
			return XST_FAILURE;
			break;

	}

	/* Program sensor */
	for (Index = 0; Index < (MaxIndex - 0); Index++) {

		WriteBuffer[0] = sensor_cfg[Index].Address >> 8;
		WriteBuffer[1] = sensor_cfg[Index].Address;
		WriteBuffer[2] = sensor_cfg[Index].Data;
		Status = SensorWriteData(3);
		if (Status == XST_SUCCESS) {
			ReadBuffer[0] = 0;
			Status = SensorReadData(ReadBuffer, 1);
			if(WriteBuffer[2] != ReadBuffer[0])
			 xil_printf("index %d Ref %d Read %d\n",Index,WriteBuffer[2],ReadBuffer[0]);
			usleep(100);

		} else {
			xil_printf("Error in Writing entry status = %x \r\n",
					Status);
			break;
		}
	}
	if (Index != (MaxIndex - 0)) {
		/* all registers are written into */
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function starts camera sensor to transmit captured video
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note		None.
 *
 *****************************************************************************/
int StartSensor(void)
{
	int Status;

	if (Pipeline_Cfg.CameraPresent == FALSE) {
		xil_printf("%s - No camera present\r\n", __func__);
		return XST_SUCCESS;
	}

	usleep(1000000);
	WriteBuffer[0] = 0x30;
	WriteBuffer[1] = 0x00;
	WriteBuffer[2] = 0x00;
	Status = SensorWriteData(3);
	usleep(1000000);
	WriteBuffer[0] = 0x30;
	WriteBuffer[1] = 0x3E;
	WriteBuffer[2] = 0x02;
	Status = SensorWriteData(3);
	usleep(1000000);
	WriteBuffer[0] = 0x30;
	WriteBuffer[1] = 0xF4;
	WriteBuffer[2] = 0x00;
	Status = SensorWriteData(3);
	usleep(1000000);
	WriteBuffer[0] = 0x30;
	WriteBuffer[1] = 0x18;
	WriteBuffer[2] = 0xA2;
	Status = SensorWriteData(3);

	if (Status != XST_SUCCESS) {
		xil_printf("Error: in Writing entry status = %x \r\n", Status);
		xil_printf("%s - Failed\r\n", __func__);
		return XST_FAILURE;
	}

	return Status;
}


/*****************************************************************************/
/**
 * This function initializes IIC controller and gets config parameters.
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/
int InitIIC(void) {
	int Status;
	XIic_Config *ConfigPtr; /* Pointer to configuration data */

	memset(&IicSensor, 0, sizeof(XIic));

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIic_LookupConfig(IIC_SENSOR_DEV_ID);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&IicSensor, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly.
	 */
	Status = XIic_SelfTest(&IicSensor);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return Status;
}
/*****************************************************************************/
/**
 * This function sets send, receive and error handlers for IIC interrupts.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void SetupIICIntrHandlers(void) {
	/*
	 * Set the Handlers for transmit and reception.
	 */
	XIic_SetSendHandler(&IicSensor, &IicSensor,
				(XIic_Handler) SendHandler);
	XIic_SetRecvHandler(&IicSensor, &IicSensor,
				(XIic_Handler) ReceiveHandler);
	XIic_SetStatusHandler(&IicSensor, &IicSensor,
				(XIic_StatusHandler) StatusHandler);

}
/*****************************************************************************/
/**
 * This function stops VProc_SS scalar IP.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void DisableScaler(void)
{
	XVprocSs_Stop(&scaler_new_inst);
}

/*****************************************************************************/
/**
 * This function initializes MIPI CSI2 RX SS and gets config parameters.
 *
 * @return	XST_SUCCESS if successful or else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/
u32 InitializeCsiRxSs(void)
{
	u32 Status = 0;
	XCsiSs_Config *CsiRxSsCfgPtr = NULL;

	CsiRxSsCfgPtr = XCsiSs_LookupConfig(XCSIRXSS_DEVICE_ID);
	if (!CsiRxSsCfgPtr) {
		xil_printf("CSI2RxSs LookupCfg failed\r\n");
		return XST_FAILURE;
	}

	Status = XCsiSs_CfgInitialize(&CsiRxSs, CsiRxSsCfgPtr,
			CsiRxSsCfgPtr->BaseAddr);

	if (Status != XST_SUCCESS) {
		xil_printf("CsiRxSs Cfg init failed - %x\r\n", Status);
		return Status;
	}

	return XST_SUCCESS;
}




/*****************************************************************************/
/**
 * This function sets colour depth value getting from MIPI CSI2 RX SS
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void SetColorDepth(void)
{

	print(TXT_GREEN);
	xil_printf("Setting Color Depth = %d bpc\r\n", CsiRxSs.Config.PixelFormat);
	print(TXT_RST);
	return;
}

/*****************************************************************************/
/**
 * This function enables MIPI CSI IP
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void EnableCSI(void)
{
	XCsiSs_Reset(&CsiRxSs);
	XCsiSs_Configure(&CsiRxSs, (Pipeline_Cfg.ActiveLanes), 0);
	XCsiSs_Activate(&CsiRxSs, XCSI_ENABLE);

	usleep(1000000);
}

/*****************************************************************************/
/**
 * This function disables MIPI CSI IP
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void DisableCSI(void)
{
	usleep(10000);
	XCsiSs_Reset(&CsiRxSs);
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
int SetupInterrupts(void) {
	int Status;

	XScuGic *IntcInstPtr = &Intc;


	/*
	 * Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */

	Status = XScuGic_Connect(IntcInstPtr, XPAR_INTC_0_V_SENSOR_IIC_0_VEC_ID,
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
	XScuGic_Enable(IntcInstPtr, XPAR_INTC_0_V_SENSOR_IIC_0_VEC_ID);
	XScuGic_Enable(IntcInstPtr, XPAR_INTC_0_V_FRMBUF_WR_0_VEC_ID);
	XScuGic_Enable(IntcInstPtr, XPAR_INTC_0_V_FRMBUF_RD_0_VEC_ID);


	return (XST_SUCCESS);
}


/*****************************************************************************/
/**
 * This function toggles HW reset line for all IP's
 *
 * @return None
 *
 *****************************************************************************/
void resetFrmbufIp(void)
{

  /* Stop Frame Buffer and wait for IDLE */
  XVFrmbufWr_Stop(&frmbufwr);
  XVFrmbufRd_Stop(&frmbufrd);

  xil_printf("\r\nReset HLS IP \r\n");

  usleep(1000);          //hold reset line

}

/*****************************************************************************/
/**
 * This function calculates the stride
 *
 * @returns stride in bytes
 *
 *****************************************************************************/
static u32 CalcStride(XVidC_ColorFormat Cfmt,
                      u16 AXIMMDataWidth,
                      XVidC_VideoStream *StreamPtr)
{
  u32 stride;
  int width = StreamPtr->Timing.HActive;
  u16 MMWidthBytes = AXIMMDataWidth/8;

  if ((Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)
      || (Cfmt == XVIDC_CSF_MEM_Y10)) {
    // 4 bytes per 3 pixels (Y_UV10, Y_UV10_420, Y10)
    stride = ((((width*4)/3)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
  }
  else if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
           || (Cfmt == XVIDC_CSF_MEM_Y8)) {
    // 1 byte per pixel (Y_UV8, Y_UV8_420, Y8)
    stride = ((width+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
  }
  else if ((Cfmt == XVIDC_CSF_MEM_RGB8) || (Cfmt == XVIDC_CSF_MEM_YUV8)
           || (Cfmt == XVIDC_CSF_MEM_BGR8)) {
    // 3 bytes per pixel (RGB8, YUV8, BGR8)
     stride = (((width*3)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
  }
  else {
    // 4 bytes per pixel
    stride = (((width*4)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
  }

  return(stride);
}
/*****************************************************************************/
/**
 * This function configures Frame Buffer for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
static int ConfigFrmbuf(u32 StrideInBytes,
                        XVidC_ColorFormat Cfmt,
                        XVidC_VideoStream *StreamPtr
						)
{
  int Status;


  XVFrmbufWr_WaitForIdle(&frmbufwr);
  XVFrmbufRd_WaitForIdle(&frmbufrd);


  XVFRMBUFWR_BUFFER_BASEADDR = frame_array[wr_ptr];
  XVFRMBUFRD_BUFFER_BASEADDR = frame_array[rd_ptr];
    /* Configure Frame Buffers */


    Status = XVFrmbufWr_SetMemFormat(&frmbufwr, StrideInBytes, Cfmt, StreamPtr);
    if(Status != XST_SUCCESS) {
      xil_printf("ERROR:: Unable to configure Frame Buffer Write\r\n");
      return(XST_FAILURE);
    }

    Status = XVFrmbufWr_SetBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR);
    if(Status != XST_SUCCESS) {
      xil_printf("ERROR:: Unable to configure Frame \
                                        Buffer Write buffer address\r\n");
      return(XST_FAILURE);
    }

    /* Set Chroma Buffer Address for semi-planar color formats */
    if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
        (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
      Status = XVFrmbufWr_SetChromaBufferAddr(&frmbufwr,
                               XVFRMBUFWR_BUFFER_BASEADDR+CHROMA_ADDR_OFFSET);
      if(Status != XST_SUCCESS) {
        xil_printf("ERROR::Unable to configure Frame Buffer \
                                           Write chroma buffer address\r\n");
        return(XST_FAILURE);
      }
    }

    /* Configure Frame Buffer Read*/
        Status = XVFrmbufRd_SetMemFormat(&frmbufrd, StrideInBytes,
                                                       Cfmt, StreamPtr);
        if(Status != XST_SUCCESS) {
          xil_printf("ERROR:: Unable to configure Frame Buffer Read\r\n");
          return(XST_FAILURE);
        }

        Status = XVFrmbufRd_SetBufferAddr(&frmbufrd,
                                      XVFRMBUFRD_BUFFER_BASEADDR);
        if(Status != XST_SUCCESS) {
          xil_printf("ERROR:: Unable to configure Frame \
                                         Buffer Read buffer address\r\n");
          return(XST_FAILURE);
        }


        /* Set Chroma Buffer Address for semi-planar color formats */
        if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) ||
            (Cfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
            (Cfmt == XVIDC_CSF_MEM_Y_UV10) ||
            (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
          Status = XVFrmbufRd_SetChromaBufferAddr(&frmbufrd,
                       XVFRMBUFRD_BUFFER_BASEADDR+CHROMA_ADDR_OFFSET);
          if(Status != XST_SUCCESS) {
            xil_printf("ERROR:: Unable to configure Frame \
                               Buffer Read chroma buffer address\r\n");
            return(XST_FAILURE);
          }
        }


  /* Enable Interrupt */
  XVFrmbufWr_InterruptEnable(&frmbufwr, XVFRMBUFWR_IRQ_DONE_MASK);
  XVFrmbufRd_InterruptEnable(&frmbufrd, XVFRMBUFRD_IRQ_DONE_MASK);
  xil_printf("INFO: FRMBUF configured\r\n");

  return(Status);
}

/*****************************************************************************/
/**
 *
 * This function is called when a Frame Buffer Write Done has occurred.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void FrmbufwrDoneCallback(void *CallbackRef) {
	 //xil_printf("  Wr Done  \r\n");
	int Status;

	 rd_ptr = wr_ptr;
	 if(wr_ptr == 0)
		 rd_ptr = 4 ;
	 else
		 rd_ptr = wr_ptr -1 ;

	 if(wr_ptr == 4) {

		  wr_ptr = 0;
	 }
	 else{
		 wr_ptr = wr_ptr + 1;
	 }

     XVFRMBUFRD_BUFFER_BASEADDR = frame_array[rd_ptr];
	 XVFRMBUFWR_BUFFER_BASEADDR = frame_array[wr_ptr];

	 Status = XVFrmbufWr_SetBufferAddr(&frmbufwr,
                                               XVFRMBUFWR_BUFFER_BASEADDR);
	   if(Status != XST_SUCCESS) {
	     xil_printf("ERROR:: Unable to configure Frame Buffer \
                                                 Write buffer address\r\n");
	   }

	   /* Set Chroma Buffer Address for semi-planar color formats */
	   if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) ||
               (Cfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
	       (Cfmt == XVIDC_CSF_MEM_Y_UV10) ||
               (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
	     Status = XVFrmbufWr_SetChromaBufferAddr(&frmbufwr,
                             XVFRMBUFWR_BUFFER_BASEADDR+CHROMA_ADDR_OFFSET);
	     if(Status != XST_SUCCESS) {
	       xil_printf("ERROR:: Unable to configure Frame Buffer \
                                           Write chroma buffer address\r\n");
	     }
	   }

	  Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
	   if(Status != XST_SUCCESS) {
	     xil_printf("ERROR:: Unable to configure Frame Buffer \
                                                   Read buffer address\r\n");
	   }

	   /* Set Chroma Buffer Address for semi-planar color formats */
	   if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) ||
               (Cfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
	       (Cfmt == XVIDC_CSF_MEM_Y_UV10) ||
               (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
	     Status = XVFrmbufRd_SetChromaBufferAddr(&frmbufrd,
                            XVFRMBUFRD_BUFFER_BASEADDR+CHROMA_ADDR_OFFSET);
	     if(Status != XST_SUCCESS) {
	       xil_printf("ERROR:: Unable to configure Frame \
                                      Buffer Read chroma buffer address\r\n");
	     }
	   }

	   if ( (frmrd_start == 0) && (wr_ptr == 2)){
		   XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);
		   XVFrmbufRd_Start(&frmbufrd);
		   frmrd_start = 1 ;
	   }
frm_cnt++;
}

/*****************************************************************************/
/**
 *
 * This function is called when a Frame Buffer Read Done has occurred.
 *
 * @param	CallbackRef is a callback function reference.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void FrmbufrdDoneCallback(void *CallbackRef) {
      frm_cnt1++;
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

int config_csi_cap_path(){


	u32 Status;
	/* Initialize Frame Buffer Write */
	 Status =  XVFrmbufWr_Initialize(&frmbufwr, XPAR_XV_FRMBUFWR_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "Frame Buffer Write Init failed status = %x.\r\n"
				 TXT_RST, Status);
		return XST_FAILURE;
	}


	/* Initialize Frame Buffer Read */
	Status =  XVFrmbufRd_Initialize(&frmbufrd, XPAR_XV_FRMBUFRD_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "Frame Buffer Read Init failed status = %x.\r\n"
				 TXT_RST, Status);
		return XST_FAILURE;
	}

	/* Initialize IIC */
	Status = InitIIC();
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "\n\rIIC Init Failed \n\r" TXT_RST);
		return XST_FAILURE;
	}
	xil_printf("\n *Init IIC is Done \n");


	/* Initialize CSIRXSS  */
	Status = InitializeCsiRxSs();
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "CSI Rx Ss Init failed status = %x.\r\n"
				 TXT_RST, Status);
		return XST_FAILURE;
	}

	/* MIPI colour depth in bits per clock */

	xil_printf("\r\n\r\n *Init CSI is Done \n");


	/* Initialize IRQ */
	Status = SetupInterrupts();
	if (Status == XST_FAILURE) {
		print(TXT_RED "IRQ init failed.\n\r" TXT_RST);
		return XST_FAILURE;
	}
	xil_printf("\r\n\r\n *IRQ Setup is Done \n");

	/* IIC interrupt handlers */
	SetupIICIntrHandlers();

	Status = XVFrmbufWr_SetCallback(&frmbufwr,
	                                    XVFRMBUFWR_HANDLER_DONE,
	                                    (void *)FrmbufwrDoneCallback,
	                                    (void *) &frmbufwr);
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "Frame Buffer Write Call back  failed status = %x.\r\n"
					TXT_RST, Status);
		return XST_FAILURE;
	}


	Status = XVFrmbufRd_SetCallback(&frmbufrd,
	                                XVFRMBUFRD_HANDLER_DONE,
	                                (void *)FrmbufrdDoneCallback,
	                                (void *) &frmbufrd);
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "Frame Buffer Read Call back  failed status = %x.\r\n"
				 TXT_RST, Status);
		return XST_FAILURE;
	}


	print("\r\n\r\n--------------------------------\r\n");

	return 0;

}

int start_csi_cap_pipe(XVidC_VideoMode VideoMode)
{
	u32 Status;
    int stride;
    XVidC_VideoTiming const *TimingPtr;
	int  widthIn, heightIn;
	int  widthOut, heightOut;
	/* Local variables */
	XVidC_VideoMode  resIdOut;



	/* Setup the default pipeline configuration parameters */
	/* Look for Default ColorDepth manual setting just after
	 * SetColorDepth
	 */

	Pipeline_Cfg.ActiveLanes = 4;
	Pipeline_Cfg.VideoSrc = XVIDSRC_SENSOR;

	Pipeline_Cfg.Live = TRUE;

	/* Vertical and Horizontal flip don't work */
	Pipeline_Cfg.Vflip = FALSE;
	Pipeline_Cfg.Hflip = FALSE;

	/* Video pipeline configuration from user */
	Pipeline_Cfg.CameraPresent = TRUE;
	Pipeline_Cfg.DSIDisplayPresent = TRUE;

	/* Default Resolution that to be displayed */
	Pipeline_Cfg.VideoMode = VideoMode ; //XVIDC_VM_1920x1080_60_P;

	/* Select the sensor configuration based on resolution and lane */
	switch (Pipeline_Cfg.VideoMode) {

		case XVIDC_VM_1280x720_60_P:
	        widthIn  = 1280;
	        heightIn = 720;
			break;

		case XVIDC_VM_1920x1080_30_P:
	        widthIn  = 1920;
	        heightIn = 1080;
			break;

		case XVIDC_VM_1920x1080_60_P:
	        widthIn  = 1920;
	        heightIn = 1080;
			break;

		case XVIDC_VM_3840x2160_30_P:
	        widthIn  = 3840;
	        heightIn = 2160;
			break;

		case XVIDC_VM_3840x2160_60_P:
	        widthIn  = 3840;
	        heightIn = 2160;
			break;
		default:
		    xil_printf("Invalid Input Selection ");
			return XST_FAILURE;
			break;

	}

	/* Programming Stream with fixed output */
	if(Pipeline_Cfg.VideoDestn == XVIDDES_DSI){
		/* Fixed output to DSI (1920x1200) */
		widthOut = DSI_H_RES;
		heightOut = DSI_V_RES;}
	else {
		widthOut = widthIn;
		heightOut = heightIn;
	}

    usleep(1000);
	Reset_IP_Pipe();


	resIdOut = XVidC_GetVideoModeId(widthIn, heightIn, XVIDC_FR_60HZ,
					FALSE);

	StreamOut.VmId = resIdOut;
	StreamOut.Timing.HActive = widthIn;
	StreamOut.Timing.VActive = heightIn;
	StreamOut.ColorFormatId = XVIDC_CSF_RGB;
	StreamOut.FrameRate = XVIDC_FR_60HZ;
	StreamOut.IsInterlaced = 0;


	/* Setup a default stream */
	StreamOut.ColorDepth = (XVidC_ColorDepth)frmbufwr.FrmbufWr.Config.MaxDataWidth;
	StreamOut.PixPerClk = (XVidC_PixelsPerClock)frmbufwr.FrmbufWr.Config.PixPerClk;

	VidStream.PixPerClk =(XVidC_PixelsPerClock)frmbufwr.FrmbufWr.Config.PixPerClk;
	VidStream.ColorDepth = (XVidC_ColorDepth)frmbufwr.FrmbufWr.Config.MaxDataWidth;
	Cfmt = XVIDC_CSF_MEM_RGB8 ;
	VidStream.ColorFormatId = StreamOut.ColorFormatId;
	VidStream.VmId = StreamOut.VmId;

	/* Get mode timing parameters */
	TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
	VidStream.Timing = *TimingPtr;
	VidStream.FrameRate = XVidC_GetFrameRate(VidStream.VmId);
	xil_printf("\r\n********************************************\r\n");
	xil_printf("Test Input Stream: %s (%s)\r\n",
	           XVidC_GetVideoModeStr(VidStream.VmId),
	           XVidC_GetColorFormatStr(Cfmt));
	xil_printf("********************************************\r\n");
	stride = CalcStride(Cfmt ,
	                         frmbufwr.FrmbufWr.Config.AXIMMDataWidth,
	                         &StreamOut);
	 xil_printf(" Stride is calculated %d \r\n",stride);
	ConfigFrmbuf(stride, Cfmt, &StreamOut);
	xil_printf(" Frame Buffer Setup is Done\r\n");


	/* Reset Camera Sensor module through GPIO */
	xil_printf("Disable CAM_RST of Sensor through GPIO\r\n");
	CamReset();
	xil_printf("\r\nSensor is Enabled\r\n");
	usleep(20000);

	XV_frmbufwr_EnableAutoRestart(&frmbufwr.FrmbufWr);
	XVFrmbufWr_Start(&frmbufwr);

	XV_frmbufrd_EnableAutoRestart(&frmbufrd.FrmbufRd);
	XVFrmbufRd_Start(&frmbufrd);


	ConfigCSC(widthIn, heightIn);
	ConfigGammaLut(widthIn, heightIn);
	ConfigDemosaic(widthIn, heightIn);
	if (Pipeline_Cfg.VideoDestn == XVIDDES_DSI) {
	InitVprocSs_Scaler(1,widthOut, heightOut);
	}
	EnableCSI();

	xil_printf("CSI is Enabled\r\n");

	/* Program Camera sensor */
	Status = SetupCameraSensor();
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "Failed to setup Camera sensor\r\n" TXT_RST);
		return XST_FAILURE;
	}
	xil_printf("Sensor setup is Done\r\n");

	/* Start HDMI */
	if (Pipeline_Cfg.VideoDestn == XVIDDES_HDMI) {
	  start_hdmi(Pipeline_Cfg.VideoMode);
	}
	  /* Start Camera Sensor to capture video */
      StartSensor();
	  xil_printf("Sensor is started \r\n");


	  xil_printf(TXT_RST);

      return 0;

}
