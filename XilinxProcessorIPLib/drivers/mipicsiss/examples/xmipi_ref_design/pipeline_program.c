/******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 * @file pipeline_program.c
 *
 * This file contains the video pipe line configuration, Sensor configuration
 * and its programming as per the resolution selected by user.
 * Please see pipeline_program.h for more details.
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

#include "stdlib.h"
#include "xparameters.h"
#include "sleep.h"
#include "xiic.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "sensor_cfgs.h"
#include "xscugic.h"

#include "xdsitxss.h"
#include "xgpio.h"
#include "xcsiss.h"
#include "xaxivdma.h"
#include <xvidc.h>
#include <xvprocss.h>

#define IIC_SENSOR_DEV_ID	XPAR_AXI_IIC_1_SENSOR_DEVICE_ID

#define VPROCSSCSC_BASE	XPAR_XVPROCSS_1_BASEADDR
#define DEMOSAIC_BASE	XPAR_XV_DEMOSAIC_0_S_AXI_CTRL_BASEADDR
#define VGAMMALUT_BASE	XPAR_XV_GAMMA_LUT_0_S_AXI_CTRL_BASEADDR

#define PAGE_SIZE	16

#define EEPROM_TEST_START_ADDRESS	128

#define FRAME_BASE	0x10000000
#define CSIFrame	0x20000000
#define ScalerFrame	0x30000000

#define XVPROCSS_DEVICE_ID	XPAR_DSI_DISPLAY_PATH_V_PROC_SS_0_DEVICE_ID
#define VDMA_NUM_FSTORES	XPAR_AXIVDMA_0_NUM_FSTORES
#define XDSITXSS_DEVICE_ID	XPAR_DSITXSS_0_DEVICE_ID
#define XDSITXSS_INTR_ID	XPAR_FABRIC_MIPI_DSI_TX_SUBSYSTEM_0_INTERRUPT_INTR
#define DSI_BYTES_PER_PIXEL	(3)
#define DSI_H_RES		(1920)
#define DSI_V_RES		(1200)
#define DSI_DISPLAY_HORI_VAL	(DSI_H_RES * DSI_BYTES_PER_PIXEL)
#define DSI_DISPLAY_VERT_VAL	(DSI_V_RES)
#define DSI_HBACK_PORCH			(0x39D)
#define DSI_HFRONT_PORCH		(0x00B9)
#define DSI_VSYNC_WIDTH			(0x05)
#define DSI_VBACK_PORCH			(0x04)
#define DSI_VFRONT_PORCH		(0x03)

#define ACTIVE_LANES_1	1
#define ACTIVE_LANES_2	2
#define ACTIVE_LANES_3	3
#define ACTIVE_LANES_4	4

XDsiTxSs DsiTxSs;

#define XGPIO_TREADY_DEVICE_ID	XPAR_GPIO_2_DEVICE_ID
XGpio Gpio_Tready;

#define XCSIRXSS_DEVICE_ID	XPAR_CSISS_0_DEVICE_ID
XCsiSs CsiRxSs;

#define XVDMACSI_DEVICE_ID	XPAR_PROCESSING_SS_AXI_VDMA_CSIRX2CFA_DEVICE_ID
XAxiVdma CsiVdma;

#define XVDMATPG_DEVICE_ID	XPAR_AXIVDMA_0_DEVICE_ID
XAxiVdma TpgVdma;

XVprocSs scaler_new_inst;

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

extern XPipeline_Cfg Pipeline_Cfg;
extern u8 TxRestartColorbar;

void ConfigDemosaicResolution(XVidC_VideoMode videomode);
void DisableDemosaicResolution();

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
 *****************************************************************************/
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
			MaxIndex = length_imx274_config_4K_30fps_regs;
			sensor_cfg = imx274_config_4K_30fps_regs;
			break;


		default:
			return XST_FAILURE;
			break;

	}

	/* Program sensor */
	for (Index = 0; Index < (MaxIndex - 1); Index++) {

		WriteBuffer[0] = sensor_cfg[Index].Address >> 8;
		WriteBuffer[1] = sensor_cfg[Index].Address;
		WriteBuffer[2] = sensor_cfg[Index].Data;

		Status = SensorWriteData(3);

		if (Status == XST_SUCCESS) {
			ReadBuffer[0] = 0;
			Status = SensorReadData(ReadBuffer, 1);

		} else {
			xil_printf("Error in Writing entry status = %x \r\n",
					Status);
			break;
		}
	}

	if (Index != (MaxIndex - 1)) {
		/* all registers are written into */
		return XST_FAILURE;
	}

	return XST_SUCCESS;
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
 * This function disables Demosaic, GammaLut and VProcSS IPs
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void DisableImageProcessingPipe(void)
{
	Xil_Out32((DEMOSAIC_BASE + 0x00), 0x0   );
	Xil_Out32((VGAMMALUT_BASE + 0x00), 0x0   );
	Xil_Out32((VPROCSSCSC_BASE + 0x00), 0x0  );

}

/*****************************************************************************/
/**
 * This function Initializes Image Processing blocks wrt to selected resolution
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void InitImageProcessingPipe(void)
{
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

		default:
			xil_printf("InitDemosaicGammaCSC - Invalid Video Mode");
			xil_printf("\n\r");
			return;
	}


	ConfigCSC(width, height);
	ConfigGammaLut(width, height);
	ConfigDemosaic(width, height);

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
	usleep(1000000);
	XCsiSs_Reset(&CsiRxSs);
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
 *
 * This function initializes the vdma IP for give configuration
 *
 * @param	VdmaInst is the pointer to XAxiVdma instance
 * @param	bpp is the bits per pixel
 * @param	FrameBase is the address of the frame buffer
 *
 * @return None.
 *
 * @note   None.
 *
 *****************************************************************************/
void InitVDMA(XAxiVdma *VdmaInst, u32 bpp, u32 FrameBase)
{
	u32 width, height;
	XAxiVdma_FrameCounter FrameCounter;
	XAxiVdma_DmaSetup DmaConfig;

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

		default:
			xil_printf("InitVDMA - Invalid Video Mode \r\n");
			return;
	}

	XAxiVdma_Reset(VdmaInst, XAXIVDMA_READ);
	while (XAxiVdma_ResetNotDone(VdmaInst, XAXIVDMA_READ));

	XAxiVdma_Reset(VdmaInst, XAXIVDMA_WRITE);
	while (XAxiVdma_ResetNotDone(VdmaInst, XAXIVDMA_WRITE));

	FrameCounter.ReadDelayTimerCount = 0;
	FrameCounter.ReadFrameCount = VDMA_NUM_FSTORES;
	FrameCounter.WriteDelayTimerCount = 0;
	FrameCounter.WriteFrameCount = VDMA_NUM_FSTORES;

	XAxiVdma_SetFrameCounter(VdmaInst, &FrameCounter);

	/* S2MM write channel config */
	/* Vertical size input */
	DmaConfig.VertSizeInput = height;
	/* Horizontal size input */
	DmaConfig.HoriSizeInput = (width * bpp);
	/* Stride */
	DmaConfig.Stride = (width * bpp);
	/* Frame Delay */
	DmaConfig.FrameDelay = 1;
	/* Circular Buffer Mode? */
	DmaConfig.EnableCircularBuf = 1;
	/* Gen-Lock Mode? */
	DmaConfig.EnableSync = 0;
	/* Master we synchronize with */
	DmaConfig.PointNum = 0;
	/* Frame Counter Enable */
	DmaConfig.EnableFrameCounter = 0;
	/* Fixed Frame Store Address index */
	DmaConfig.FixedFrameStoreAddr = 0;
	/* Gen-Lock Repeat? */
	DmaConfig.GenLockRepeat = 0;

	XAxiVdma_DmaConfig(VdmaInst, XAXIVDMA_WRITE, &DmaConfig);

	/* Start Addresses of Frame Store Buffers. */
	DmaConfig.FrameStoreStartAddr[0] = (u32) FrameBase;
	DmaConfig.FrameStoreStartAddr[1] = (u32) FrameBase
			+ (width * height * bpp);
	DmaConfig.FrameStoreStartAddr[2] = (u32) FrameBase
			+ (width * height * bpp * 2);
	XAxiVdma_DmaSetBufferAddr(VdmaInst, XAXIVDMA_WRITE,
					DmaConfig.FrameStoreStartAddr);

	/* MM2S Read channel config */
	DmaConfig.EnableSync = 1;

	XAxiVdma_DmaConfig(VdmaInst, XAXIVDMA_READ, &DmaConfig);

	XAxiVdma_DmaSetBufferAddr(VdmaInst, XAXIVDMA_READ,
					DmaConfig.FrameStoreStartAddr);

	XAxiVdma_GenLockSourceSelect(VdmaInst, XAXIVDMA_INTERNAL_GENLOCK,
					XAXIVDMA_READ);

	XAxiVdma_DmaStart(VdmaInst, XAXIVDMA_WRITE);

}

/*****************************************************************************/
/**
 * This function provides information to program vdma wrt BPC
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void InitCSC2TPG_Vdma(void)
{
	/* Need to Modify with respect to input BPP */
	InitVDMA(&TpgVdma, 3, FRAME_BASE);
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
void InitVprocSs_Scaler(int count)
{
	XVprocSs_Config* p_vpss_cfg;
	int status;
	int widthIn, heightIn, widthOut, heightOut;

	/* Fixed output to DSI */
	widthOut = DSI_H_RES;
	heightOut = DSI_V_RES;

	/* Local variables */
	XVidC_VideoMode resIdIn, resIdOut;
	XVidC_VideoStream StreamIn, StreamOut;

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
	StreamIn.ColorDepth = scaler_new_inst.Config.ColorDepth;
	StreamIn.PixPerClk = scaler_new_inst.Config.PixPerClock;
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

	if (resIdOut != XVIDC_VM_1920x1200_60_P) {
		xil_printf("resIdOut %d doesn't match XVIDC_VM_1920x1200_60_P \r\n", resIdOut);
	}

	StreamOut.VmId = resIdOut;
	StreamOut.Timing.HActive = widthOut;
	StreamOut.Timing.VActive = heightOut;
	StreamOut.ColorFormatId = XVIDC_CSF_RGB;
	StreamOut.ColorDepth = scaler_new_inst.Config.ColorDepth;
	StreamOut.PixPerClk = scaler_new_inst.Config.PixPerClock;
	StreamOut.FrameRate = XVIDC_FR_60HZ;
	StreamOut.IsInterlaced = 0;

	XVprocSs_SetVidStreamOut(&scaler_new_inst, &StreamOut);
	if (status != XST_SUCCESS) {
		xil_printf("Unable to set output video stream parameters correctly\r\n");
		return;
	}

	status = XVprocSs_SetSubsystemConfig(&scaler_new_inst);
	if (status != XST_SUCCESS) {
		xil_printf("XVprocSs_SetSubsystemConfig failed %d\r\n", status);
		return;
	}

}

/*****************************************************************************/
/**
 * This function resets VProcSS_scalar IP.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void ResetVprocSs_Scaler(void)
{
	XVprocSs_Reset(&scaler_new_inst);
}

/*****************************************************************************/
/**
 * This function enables MIPI DSI SS.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void EnableDSI(void)
{
	XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_ENABLE);
}

/*****************************************************************************/
/**
 * This function disables MIPI DSI SS.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void DisableDSI(void)
{
	u32 Status;

	XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_DISABLE);
	do {
		Status = XDsiTxSs_IsControllerReady(&DsiTxSs);
	} while (!Status);

}

/*****************************************************************************/
/**
 * This function programs MIPI DSI SS with the required timing paramters.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void InitDSI(void)
{
	XDsi_VideoTiming Timing = { 0 };

	/* Disable DSI core only. So removed DPHY register interface in design*/
	XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_DISABLE);

	XDsiTxSs_Reset(&DsiTxSs);

	if (!XDsiTxSs_IsControllerReady(&DsiTxSs)) {
		xil_printf("DSI Controller NOT Ready!!!!\r\n");
		return;
	}

	/* Set the DSI Timing registers */
	Timing.HActive = DSI_DISPLAY_HORI_VAL;
	Timing.VActive = DSI_DISPLAY_VERT_VAL;
	Timing.HBackPorch = DSI_HBACK_PORCH;
	Timing.HFrontPorch = DSI_HFRONT_PORCH;

	Timing.VSyncWidth = DSI_VSYNC_WIDTH;
	Timing.VBackPorch = DSI_VBACK_PORCH;
	Timing.VFrontPorch = DSI_VFRONT_PORCH;

	XDsiTxSs_SetCustomVideoInterfaceTiming(&DsiTxSs,
						XDSI_VM_NON_BURST_SYNC_EVENT,
						&Timing);

	usleep(1000000);
}

/*****************************************************************************/
/**
 * This function initializes MIPI DSI SS and gets config parameters.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
u32 SetupDSI(void)
{
	XDsiTxSs_Config *DsiTxSsCfgPtr = NULL;
	u32 Status;
	u32 PixelFmt;

	DsiTxSsCfgPtr = XDsiTxSs_LookupConfig(XDSITXSS_DEVICE_ID);

	if (!DsiTxSsCfgPtr) {
		xil_printf(TXT_RED "DSI Tx SS Device Id not found\r\n" TXT_RST);
		return XST_FAILURE;
	}

	Status = XDsiTxSs_CfgInitialize(&DsiTxSs, DsiTxSsCfgPtr,
			DsiTxSsCfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "DSI Tx Ss Cfg Init failed status = %d \
				\r\n" TXT_RST, Status);
		return Status;
	}

	PixelFmt = XDsiTxSs_GetPixelFormat(&DsiTxSs);

	if (PixelFmt != 0x3E) {
		xil_printf(TXT_RED "DSI Pixel format is not correct ");
		switch (PixelFmt) {
			case 0x0E:
				xil_printf("Packed RGB565");
				break;
			case 0x1E:
				xil_printf("Packed RGB666");
				break;
			case 0x2E:
				xil_printf("Loosely packed RGB666");
				break;
			case 0x3E:
				xil_printf("Packed RGB888");
				break;
			case 0x0B:
				xil_printf("Compressed Pixel Stream");
				break;
			default:
				xil_printf("Invalid data type");
		}
		xil_printf("\r\n");
		xil_printf("Expected is 0x3E for RGB888\r\n" TXT_RST);
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function programs GPIO to 0 to select tready from MIPI DSI SS.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void SelectDSIOuptut(void)
{
	XGpio_DiscreteWrite(&Gpio_Tready, 1, 0);
}

/*****************************************************************************/
/**
 * This function programs GPIO to '1' to select tready from HDMI.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void SelectHDMIOutput(void) {
	XGpio_DiscreteWrite(&Gpio_Tready, 1, 1);
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
 * This function stops VDMA IP transactions.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void DisableTPGVdma(void)
{
	XAxiVdma_DmaStop(&TpgVdma, XAXIVDMA_WRITE);
	XAxiVdma_DmaStop(&TpgVdma, XAXIVDMA_READ);
}

/*****************************************************************************/
/**
 * This function initializes GPIO IP for tready selection and gets config
 * parameters.
 *
 * @return	XST_SUCCESS if successful or else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/
u32 InitTreadyGpio(void)
{
	u32 Status = 0;
	XGpio_Config *GpioTreadyCfgPtr = NULL;

	GpioTreadyCfgPtr = XGpio_LookupConfig(XGPIO_TREADY_DEVICE_ID);

	if (!GpioTreadyCfgPtr) {
		xil_printf("Tready GPIO LookupCfg failed\r\n");
		return XST_FAILURE;
	}

	Status = XGpio_CfgInitialize(&Gpio_Tready, GpioTreadyCfgPtr,
			GpioTreadyCfgPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("TREADY GPIO cfg init failed - %x\r\n", Status);
		return Status;
	}

	return XST_SUCCESS;
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
 * This function returns selected colour depth of MIPI CSI2 RX SS.
 *
 * @param	CsiDataFormat is video data format
 *
 * @return	ColorDepth returns colour depth value.
 *
 * @note	None.
 *
 *****************************************************************************/
XVidC_ColorDepth GetColorDepth(u32 CsiDataFormat)
{
	XVidC_ColorDepth ColorDepth;

	switch (CsiDataFormat) {
		case XCSI_PXLFMT_RAW8:
			xil_printf("Color Depth = RAW8\r\n");
			ColorDepth = XVIDC_BPC_8;
			break;
		case XCSI_PXLFMT_RAW10:
			xil_printf("Color Depth = RAW10\r\n");
			ColorDepth = XVIDC_BPC_10;
			break;
		case XCSI_PXLFMT_RAW12:
			xil_printf("Color Depth = RAW12\r\n");
			ColorDepth = XVIDC_BPC_12;
			break;
		default:
			ColorDepth = XVIDC_BPC_UNKNOWN;
			break;
	}

	return ColorDepth;
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
	Pipeline_Cfg.ColorDepth = GetColorDepth(CsiRxSs.Config.PixelFormat);
	print(TXT_GREEN);
	xil_printf("Setting Color Depth = %d bpc\r\n", Pipeline_Cfg.ColorDepth);
	print(TXT_RST);
	return;
}

/*****************************************************************************/
/**
 * This function initializes Axi VDMA IP and gets config parameters.
 *
 * @return	XST_SUCCESS if successful or else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/
u32 InitializeVdma(void)
{
	u32 Status = 0;
	XAxiVdma_Config *VdmaCfgPtr = NULL;
	print(TXT_RED);


	VdmaCfgPtr = XAxiVdma_LookupConfig(XVDMATPG_DEVICE_ID);
	if (!VdmaCfgPtr) {
		xil_printf("TPG VDMA LookupCfg failed\r\n");
		return XST_FAILURE;
	}

	Status = XAxiVdma_CfgInitialize(&TpgVdma, VdmaCfgPtr,
					VdmaCfgPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("Vdma Cfg init failed - %x\r\n", Status);
		return Status;
	}

	print(TXT_RST);
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function prints the video pipeline information.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void PrintPipeConfig(void)
{
	xil_printf(TXT_YELLOW);
	xil_printf("-------------Current Pipe Configuration-------------\r\n");

	xil_printf("Color Depth	: ");
	switch (Pipeline_Cfg.ColorDepth) {
		case XVIDC_BPC_8:
			xil_printf("RAW8");
			break;
		case XVIDC_BPC_10:
			xil_printf("RAW10");
			break;
		case XVIDC_BPC_12:
			xil_printf("RAW12");
			break;
		default:
			xil_printf("Invalid");
			break;
	}
	xil_printf("\r\n");

	xil_printf("Source 		: %s\r\n",
			(Pipeline_Cfg.VideoSrc == XVIDSRC_SENSOR) ?
			"Sensor" : "Test Pattern Generator");
	xil_printf("Destination 	: %s\r\n",
			(Pipeline_Cfg.VideoDestn == XVIDDES_HDMI) ?
			"HDMI" : "DSI");

	xil_printf("Resolution	: ");
	switch (Pipeline_Cfg.VideoMode) {
		case XVIDC_VM_1280x720_60_P:
			xil_printf("1280x720@60");
			break;
		case XVIDC_VM_1920x1080_30_P:
			xil_printf("1920x1080@30");
			break;
		case XVIDC_VM_1920x1080_60_P:
			xil_printf("1920x1080@60");
			break;
		case XVIDC_VM_3840x2160_30_P:
			xil_printf("3840x2160@30");
			break;
		case XVIDC_VM_3840x2160_60_P:
			xil_printf("3840x2160@60");
			break;

		default:
			xil_printf("Invalid");
			break;
	}
	xil_printf("\r\n");

	xil_printf("Lanes		: ");
	switch (Pipeline_Cfg.ActiveLanes) {
		case 1:
			xil_printf("1");
			break;
		case 2:
			xil_printf("2");
			break;
		case 4:
			xil_printf("4");
			break;
		default:
			xil_printf("Invalid");
			break;
	}
	xil_printf(" Lanes\r\n");

	xil_printf("-----------------------------------------------------\r\n");
	xil_printf(TXT_RST);
	return;
}
