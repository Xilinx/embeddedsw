/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
#ifndef SDT
#include "xscugic.h"
#else
#include "xinterrupt_wrap.h"
#endif

#include "xdsitxss.h"
#include "xgpio.h"
#include "xcsiss.h"
#include <xvidc.h>
#include <xvprocss.h>
#include "xv_frmbufwr_l2.h"
#include "xv_frmbufrd_l2.h"

#ifndef SDT
#define IIC_SENSOR_DEV_ID	XPAR_AXI_IIC_1_SENSOR_DEVICE_ID
#else
#define IIC_SENSOR_BASE	XPAR_XIIC_0_BASEADDR
#endif

#define VPROCSSCSC_BASE	XPAR_XVPROCSS_1_BASEADDR
#ifndef SDT
#define DEMOSAIC_BASE	XPAR_XV_DEMOSAIC_0_S_AXI_CTRL_BASEADDR
#define VGAMMALUT_BASE	XPAR_XV_GAMMA_LUT_0_S_AXI_CTRL_BASEADDR
#else
#define DEMOSAIC_BASE	XPAR_XV_DEMOSAIC_0_BASEADDR
#define VGAMMALUT_BASE	XPAR_XV_GAMMA_LUT_0_BASEADDR
#endif
#ifndef SDT
#define XCSIRXSS_DEVICE_ID	XPAR_CSISS_0_DEVICE_ID
#define XGPIO_STREAM_MUX_GPIO_DEVICE_ID	XPAR_GPIO_5_DEVICE_ID
#else
#define XCSIRXSS_BASE	XPAR_XMIPICSISS_0_BASEADDR
#define XGPIO_STREAM_MUX_GPIO_BASE	XPAR_XGPIO_5_BASEADDR
#endif

#define PAGE_SIZE	16

#define EEPROM_TEST_START_ADDRESS	128

#define FRAME_BASE	0x10000000
#define CSIFrame	0x20000000
#define ScalerFrame	0x30000000

#ifndef SDT
#define XVPROCSS_DEVICE_ID	XPAR_XVPROCSS_0_DEVICE_ID
#define XDSITXSS_DEVICE_ID	XPAR_DSITXSS_0_DEVICE_ID
#define XDSITXSS_INTR_ID	XPAR_FABRIC_MIPI_DSI_TX_SUBSYSTEM_0_INTERRUPT_INTR
#else
#define XVPROCSS_BASE	XPAR_XVPROCSS_0_BASEADDR
#define XDSITXSS_BASE	XPAR_XDSITXSS_0_BASEADDR
#endif

#define DSI_BYTES_PER_PIXEL	(3)
#define DSI_H_RES		(1920)
#define DSI_V_RES		(1200)
#define HDMI_H_RES		(1920)
#define HDMI_V_RES		(1080)
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

XGpio Gpio_Stream_Mux;

XCsiSs CsiRxSs;

XVprocSs scaler_new_inst;

XVidC_VideoMode VideoMode;
XVidC_VideoStream  VidStream;
XVidC_ColorFormat  Cfmt;
XVidC_VideoStream  StreamOut;


XV_FrmbufWr_l2     frmbufwr;
XV_FrmbufRd_l2     frmbufrd;

/**************************** Type Definitions *******************************/
typedef u8 AddressType;

u8 SensorIicAddr; /* Variable for storing Eeprom IIC address */

#define SENSOR_ADDR         (0x34>>1)	/* for IMX274 Vision */

#define IIC_MUX_ADDRESS		0x75
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

#define DDR_BASEADDR 0x10000000

#define BUFFER_BASEADDR0 (DDR_BASEADDR + (0x10000000))
#define BUFFER_BASEADDR1 (DDR_BASEADDR + (0x20000000))
#define BUFFER_BASEADDR2 (DDR_BASEADDR + (0x30000000))
#define BUFFER_BASEADDR3 (DDR_BASEADDR + (0x40000000))
#define BUFFER_BASEADDR4 (DDR_BASEADDR + (0x50000000))
#define CHROMA_ADDR_OFFSET   (0x01000000U)

u32 frame_array[5] = {BUFFER_BASEADDR0, BUFFER_BASEADDR1, BUFFER_BASEADDR2,
		              BUFFER_BASEADDR3, BUFFER_BASEADDR4};
u32 rd_ptr = 4 ;
u32 wr_ptr = 0 ;
u64 XVFRMBUFRD_BUFFER_BASEADDR;
u64 XVFRMBUFWR_BUFFER_BASEADDR;
u32 frmrd_start  = 0;
u32 frm_cnt = 0;
u32 frm_cnt1 = 0;

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


  return(Status);
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
#ifndef SDT
	ConfigPtr = XIic_LookupConfig(IIC_SENSOR_DEV_ID);
#else
	ConfigPtr = XIic_LookupConfig(IIC_SENSOR_BASE);
#endif
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&IicSensor, ConfigPtr,
					ConfigPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifdef SDT
	Status = XSetupInterruptSystem(&IicSensor,&XIic_InterruptHandler,
			       ConfigPtr->IntrId,
			       ConfigPtr->IntrParent,
			       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status == XST_FAILURE) {
		xil_printf("ERROR:: Iic Interrupt Setup Failed\r\n");
		xil_printf("ERROR:: Test could not be completed\r\n");
		return(1);
	}
#endif
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


int start_csi_cap_pipe(XVidC_VideoMode VideoMode)
{

    int stride;
    XVidC_VideoTiming const *TimingPtr;
	int  widthIn, heightIn;
	/* Local variables */
	XVidC_VideoMode  resIdOut;


	/* Default Resolution that to be displayed */
	Pipeline_Cfg.VideoMode = VideoMode ;

	/* Select the sensor configuration based on resolution and lane */
	switch (Pipeline_Cfg.VideoMode) {

		case XVIDC_VM_1920x1080_30_P:
		case XVIDC_VM_1920x1080_60_P:
	        widthIn  = 1920;
	        heightIn = 1080;
			break;

		case XVIDC_VM_3840x2160_30_P:
		case XVIDC_VM_3840x2160_60_P:
	        widthIn  = 3840;
	        heightIn = 2160;
			break;

		case XVIDC_VM_1280x720_60_P:
			widthIn  = 1280;
			heightIn = 720;
			break;

		default:
		    xil_printf("Invalid Input Selection ");
			return XST_FAILURE;
			break;

	}


    usleep(1000);

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


	XV_frmbufwr_EnableAutoRestart(&frmbufwr.FrmbufWr);
	XVFrmbufWr_Start(&frmbufwr);


	xil_printf(TXT_RST);

      return 0;

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
#ifndef SDT
	Status =  XVFrmbufWr_Initialize(&frmbufwr, XPAR_XV_FRMBUFWR_0_DEVICE_ID);
#else
	 Status =  XVFrmbufWr_Initialize(&frmbufwr, XPAR_XV_FRMBUF_WR_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "Frame Buffer Write Init failed status = %x.\r\n"
				 TXT_RST, Status);
		return XST_FAILURE;
	}


	/* Initialize Frame Buffer Read */
#ifndef SDT
	Status =  XVFrmbufRd_Initialize(&frmbufrd, XPAR_XV_FRMBUFRD_0_DEVICE_ID);
#else
	Status =  XVFrmbufRd_Initialize(&frmbufrd, XPAR_XV_FRMBUF_RD_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf(TXT_RED "Frame Buffer Read Init failed status = %x.\r\n"
				 TXT_RST, Status);
		return XST_FAILURE;
	}


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
#ifndef SDT
		p_vpss_cfg = XVprocSs_LookupConfig(XVPROCSS_DEVICE_ID);
#else
		p_vpss_cfg = XVprocSs_LookupConfig(XVPROCSS_BASE);
#endif
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

	XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_DSI, XDSITXSS_ENABLE);
	XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_PHY, XDSITXSS_ENABLE);
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

	XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_DSI, XDSITXSS_DISABLE);
	XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_PHY, XDSITXSS_DISABLE);
	usleep(100000);
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
	u32 Status;
	XDsi_VideoTiming Timing = { 0 };

	/* Disable DSI core only. So removed DPHY register interface in design*/
	Status = XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_DSI, XDSITXSS_DISABLE);
	Status = XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_PHY, XDSITXSS_DISABLE);


	XDsiTxSs_Reset(&DsiTxSs);

        usleep(100000);
	Status = XDsiTxSs_Activate(&DsiTxSs, XDSITXSS_PHY, XDSITXSS_ENABLE);

	do {
		Status = XDsiTxSs_IsControllerReady(&DsiTxSs);
	} while (!Status);

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

#ifndef SDT
	DsiTxSsCfgPtr = XDsiTxSs_LookupConfig(XDSITXSS_DEVICE_ID);
#else
	DsiTxSsCfgPtr = XDsiTxSs_LookupConfig(XDSITXSS_BASE);
#endif
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
 * This function programs GPIO to 1 to select MIPI DSI SS Stream Path in AXI-
 * Stream switch.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void SelectDSIOutput(void)
{
	XGpio_DiscreteWrite(&Gpio_Stream_Mux, 1, 1);
}

/*****************************************************************************/
/**
 * This function programs GPIO to '0' to select HDMI Stream path in AXI-Stream
 * switch.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void SelectHDMIOutput(void) {
	XGpio_DiscreteWrite(&Gpio_Stream_Mux, 1, 0);
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
 * This function initializes GPIO IP for Stream Switch selection and gets config
 * parameters.
 *
 * @return	XST_SUCCESS if successful or else XST_FAILURE.
 *
 * @note	None.
 *
 *****************************************************************************/
u32 InitStreamMuxGpio(void)
{
	u32 Status = 0;
	XGpio_Config *GpioStreamMuxCfgPtr = NULL;

#ifndef SDT
	GpioStreamMuxCfgPtr = XGpio_LookupConfig(XGPIO_STREAM_MUX_GPIO_DEVICE_ID);
#else
	GpioStreamMuxCfgPtr = XGpio_LookupConfig(XGPIO_STREAM_MUX_GPIO_BASE);
#endif
	if (!GpioStreamMuxCfgPtr) {
		xil_printf("Stream Mux GPIO LookupCfg failed\r\n");
		return XST_FAILURE;
	}

	Status = XGpio_CfgInitialize(&Gpio_Stream_Mux, GpioStreamMuxCfgPtr,
			GpioStreamMuxCfgPtr->BaseAddress);

	if (Status != XST_SUCCESS) {
		xil_printf("Stream Mux GPIO cfg init failed - %x\r\n", Status);
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

#ifndef SDT
	CsiRxSsCfgPtr = XCsiSs_LookupConfig(XCSIRXSS_DEVICE_ID);
#else
	CsiRxSsCfgPtr = XCsiSs_LookupConfig(XCSIRXSS_BASE);
#endif
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
