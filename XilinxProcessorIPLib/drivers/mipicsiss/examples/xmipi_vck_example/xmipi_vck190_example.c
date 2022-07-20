 /******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file main.c
*
* This file demonstrates how to use Xilinx MIPI CSI RX Subsystem, Demosaic
* GammaLUT,VPSS CSC, VPSS Scaler , Frame Buffer Read and Write
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include "sleep.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xil_exception.h"
#include "xil_cache.h"
#include "xgpio_l.h"
#include "xiic.h"
#include "xscugic.h"
#include "xuartpsv.h"
#include "xvidc.h"
#include "platform.h"
#include "xgpio.h"
#include "xdsitxss.h"
#include "sensor_cfgs.h"
#include "xv_hdmitxss.h"

int config_hdmi();

int config_csi_cap_path();
int start_csi_cap_pipe(XVidC_VideoMode VideoMode);

XPipeline_Cfg Pipeline_Cfg;
XPipeline_Cfg New_Cfg;

XVidC_VideoMode VideoMode_CSI;
XVidC_VideoMode VideoMode_HDMI;

extern int StartSensor(void);
extern int SetupCameraSensor(void);
extern void Reset_IP_Pipe(void);
extern void InitVprocSs_Scaler(int count,int width,int height);
extern void DisableScaler(void);
extern void ConfigGammaLut(u32 width , u32 height);
extern void ConfigDemosaic(u32 width , u32 height);
extern void ConfigCSC(u32 width , u32 height);
extern void CamReset(void);
extern void EnableCSI(void);
extern void DisableCSI(void);
XScuGic     Intc;
extern u8                 TxBusy ;
extern u8                 TxRestartColorbar ;
extern u8   			  IsStreamUp;
extern u8                        SinkReady;

#define VPROCSSCSC_BASE	XPAR_XVPROCSS_1_BASEADDR
#define DEMOSAIC_BASE	XPAR_XV_DEMOSAIC_0_S_AXI_CTRL_BASEADDR
#define VGAMMALUT_BASE	XPAR_XV_GAMMA_LUT_0_S_AXI_CTRL_BASEADDR

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

extern	XV_HdmiTxSs  HdmiTxSs;

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
void InitImageProcessingPipe(u32 width, u32 height)
{
	ConfigCSC(width, height);
	ConfigGammaLut(width, height);
	ConfigDemosaic(width, height);

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

void Shutdown_DSI(void) {
	DisableDSI();
	usleep(20000);
	InitDSI();
	usleep(20000);
	xil_printf("DSI Turned Off...!!\r\n");
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
		xil_printf("DSI Tx SS Device Id not found\r\n");
		return XST_FAILURE;
	}

	Status = XDsiTxSs_CfgInitialize(&DsiTxSs, DsiTxSsCfgPtr,
			DsiTxSsCfgPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		xil_printf("DSI Tx Ss Cfg Init failed status = %d \
				\r\n",Status);
		return Status;
	}

	PixelFmt = XDsiTxSs_GetPixelFormat(&DsiTxSs);

	if (PixelFmt != 0x3E) {
		xil_printf("DSI Pixel format is not correct ");
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
		xil_printf("Expected is 0x3E for RGB888\r\n");
		return XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************/
/**
 * This function programs GPIO to '0' to select tready from DSI.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/
void SelectDSIOutput(void) {
	XGpio_DiscreteWrite(&Gpio_Tready, 1, 1);
}

/*****************************************************************************/
/**
 * This function programs GPIO to '0' to select tready from HDMI.
 *
 * @return	None.
 *
 * @note	None.
 *
 *****************************************************************************/

void SelectHDMIOutput(void) {
	XGpio_DiscreteWrite(&Gpio_Tready, 1, 0);
}


/*****************************************************************************/
/**
*
* This function setups the interrupt system so interrupts can occur for the
* IP cores. The function is application-specific since the actual system
* may or may not have an interrupt controller. The HDMI cores could be
* directly connected to a processor without an interrupt controller.
* The user should modify this function to fit the application.
*
* @param  None.
*
* @return
*   - XST_SUCCESS if interrupt setup was successful.
*   - A specific error code defined in "xstatus.h" if an error
*   occurs.
*
* @note   This function assumes a Microblaze system and no operating
*   system is used.
*
******************************************************************************/
int SetupInterruptSystem(void) {
	int Status;

	XScuGic *IntcInstPtr = &Intc;

	/*
	 * Initialize the interrupt controller driver so that it's ready to
	 * use, specify the device ID that was generated in xparameters.h
	 */
	XScuGic_Config *IntcCfgPtr;
	IntcCfgPtr = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
	if(IntcCfgPtr == NULL) {
		xil_printf("ERR:: Interrupt Controller not found");
		return (XST_DEVICE_NOT_FOUND);
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

	return (XST_SUCCESS);
}

void Xil_AssertCallbackRoutine(u8 *File, s32 Line) {
	xil_printf("Assertion in File %s, on line %0d\r\n", File, Line);
}


/*****************************************************************************/
/**
*
* Function to enable XPIO DCI
*
* @param  None.
*
* @return
*
* @note   None.
*
******************************************************************************/

void xpio_dci_fix()
{
   // XPIO_DCI_COMPONENT_9
   Xil_Out32(0xf651a00c, 0xf9e8d7c6);
   Xil_Out32(0xf651a000, 0x0fffffff);
   Xil_Out32(0xf651a004, 0x00000001);
   Xil_Out32(0xf651a04c, 0x107fc000);


   // XPIO_DCI_COMPONENT_8
   Xil_Out32(0xf64aa00c, 0xf9e8d7c6);
   Xil_Out32(0xf64aa000, 0x0fffffff);
   Xil_Out32(0xf64aa004, 0x00000001);
   Xil_Out32(0xf64aa04c, 0x107fc000);


   // XPIO_DCI_COMPONENT_7
   Xil_Out32(0xf644a00c, 0xf9e8d7c6);
   Xil_Out32(0xf644a000, 0x0fffffff);
   Xil_Out32(0xf644a004, 0x00000001);
   Xil_Out32(0xf644a04c, 0x107fc000);


   // XPIO_DCI_COMPONENT_6
   Xil_Out32(0xf63aa00c, 0xf9e8d7c6);
   Xil_Out32(0xf63aa000, 0x0fffffff);
   Xil_Out32(0xf63aa004, 0x00000001);
   Xil_Out32(0xf63aa04c, 0x107fc000);


   // XPIO_DCI_COMPONENT_5
   Xil_Out32(0xf633a00c, 0xf9e8d7c6);
   Xil_Out32(0xf633a000, 0x0fffffff);
   Xil_Out32(0xf633a004, 0x00000001);
   Xil_Out32(0xf633a04c, 0x107fc000);


   // XPIO_DCI_COMPONENT_4
   Xil_Out32(0xf62da00c, 0xf9e8d7c6);
   Xil_Out32(0xf62da000, 0x0fffffff);
   Xil_Out32(0xf62da004, 0x00000001);
   Xil_Out32(0xf62da04c, 0x107fc000);


   // XPIO_DCI_COMPONENT_3
   Xil_Out32(0xf623a00c, 0xf9e8d7c6);
   Xil_Out32(0xf623a000, 0x0fffffff);
   Xil_Out32(0xf623a004, 0x00000001);
   Xil_Out32(0xf623a04c, 0x107fc000);


   // XPIO_DCI_COMPONENT_2
   Xil_Out32(0xf61ca00c, 0xf9e8d7c6);
   Xil_Out32(0xf61ca000, 0x0fffffff);
   Xil_Out32(0xf61ca004, 0x00000001);
   Xil_Out32(0xf61ca04c, 0x107fc000);


   // XPIO_DCI_COMPONENT_11
   Xil_Out32(0xf661a00c, 0xf9e8d7c6);
   Xil_Out32(0xf661a000, 0x0fffffff);
   Xil_Out32(0xf661a004, 0x00000001);
   Xil_Out32(0xf661a04c, 0x107fc000);


   // XPIO_DCI_COMPONENT_10
   Xil_Out32(0xf65ba00c, 0xf9e8d7c6);
   Xil_Out32(0xf65ba000, 0x0fffffff);
   Xil_Out32(0xf65ba004, 0x00000001);
   Xil_Out32(0xf65ba04c, 0x107fc000);


   // XPIO_DCI_COMPONENT_1
   Xil_Out32(0xf616a00c, 0xf9e8d7c6);
   Xil_Out32(0xf616a000, 0x0fffffff);
   Xil_Out32(0xf616a004, 0x00000001);
   Xil_Out32(0xf616a04c, 0x107fc000);


   // XPIO_DCI_COMPONENT_0
   Xil_Out32(0xf609a00c, 0xf9e8d7c6);
   Xil_Out32(0xf609a000, 0x0fffffff);
   Xil_Out32(0xf609a004, 0x00000001);
   Xil_Out32(0xf609a04c, 0x107fc000);

}

/*****************************************************************************/
/**
*
* Function to enable XPIO equalization
*
* @param  None.
*
* @return
*
* @note   None.
*
******************************************************************************/

void xpio_equalization_fix()
{
   // Data 0 - BG25/BG24 XPIO_IOBPAIR_163
   Xil_Out32(0xf63a040c, 0xf9e8d7c6);
   Xil_Out32(0xf63a0400, 0x0fffffff);
   Xil_Out32(0xf63a0404, 0x00000001);
   Xil_Out32(0xf63a042c, 0x5d431ffe);

   // Data 1 - BC23/BD22 XPIO_IOBPAIR_171
   Xil_Out32(0xf63a320c, 0xf9e8d7c6);
   Xil_Out32(0xf63a3200, 0x0fffffff);
   Xil_Out32(0xf63a3204, 0x00000001);
   Xil_Out32(0xf63a322c, 0x5d431ffe);

   // Data 2 - BC22/BC21 XPIO_IOBPAIR_170
   Xil_Out32(0xf63a260c, 0xf9e8d7c6);
   Xil_Out32(0xf63a2600, 0x0fffffff);
   Xil_Out32(0xf63a2604, 0x00000001);
   Xil_Out32(0xf63a262c, 0x5d431ffe);

  // Data 3 - BC25/BD25 XPIO_IOBPAIR_173
   Xil_Out32(0xf63a360c, 0xf9e8d7c6);
   Xil_Out32(0xf63a3600, 0x0fffffff);
   Xil_Out32(0xf63a3604, 0x00000001);
   Xil_Out32(0xf63a362c, 0x5d431ffe);

  // CLK - BD23/BD24 XPIO_IOBPAIR_168
   Xil_Out32(0xf63a220c, 0xf9e8d7c6);
   Xil_Out32(0xf63a2200, 0x0fffffff);
   Xil_Out32(0xf63a2204, 0x00000001);
   Xil_Out32(0xf63a222c, 0x5d431ffe);

}

/*****************************************************************************/
/**
*
* Function to rnable MMCME5 Fabric Control in PCSR
*
* @param  None.
*
* @return
*
* @note   None.
*
******************************************************************************/
void Enable_mmcmfabric_control()
{

	//Enable MMCME5 Fabric Control in PCSR
	//X2Y0
	Xil_Out32(0xF618400C, 0xF9E8D7C6);
	Xil_Out32(0xF6184000, 0x200);
	Xil_Out32(0xF6184004, 0x200);
	Xil_Out32(0xF6184000, 0x1000);
	Xil_Out32(0xF6184004, 0x1000);
	Xil_Out32(0xF618400C, 0x0);
	//X3Y0
	Xil_Out32(0xF61E400C, 0xF9E8D7C6);
	Xil_Out32(0xF61E4000, 0x200);
	Xil_Out32(0xF61E4004, 0x200);
	Xil_Out32(0xF61E4000, 0x1000);
	Xil_Out32(0xF61E4004, 0x1000);
	Xil_Out32(0xF61E400C, 0x0);
	//X5Y0
	Xil_Out32(0xF62F400C, 0xF9E8D7C6);
	Xil_Out32(0xF62F4000, 0x200);
	Xil_Out32(0xF62F4004, 0x200);
	Xil_Out32(0xF62F4000, 0x1000);
	Xil_Out32(0xF62F4004, 0x1000);
	Xil_Out32(0xF62F400C, 0x0);
	//X9Y0
	Xil_Out32(0xF64C400C, 0xF9E8D7C6);
	Xil_Out32(0xF64C4000, 0x200);
	Xil_Out32(0xF64C4004, 0x200);
	Xil_Out32(0xF64C4000, 0x1000);
	Xil_Out32(0xF64C4004, 0x1000);
	Xil_Out32(0xF64C400C, 0x0);
	//X10Y0
	Xil_Out32(0xF653400C, 0xF9E8D7C6);
	Xil_Out32(0xF6534000, 0x200);
	Xil_Out32(0xF6534004, 0x200);
	Xil_Out32(0xF6534000, 0x1000);
	Xil_Out32(0xF6534004, 0x1000);
	Xil_Out32(0xF653400C, 0x0);
	//X11Y0
	Xil_Out32(0xF65D400C, 0xF9E8D7C6);
	Xil_Out32(0xF65D4000, 0x200);
	Xil_Out32(0xF65D4004, 0x200);
	Xil_Out32(0xF65D4000, 0x1000);
	Xil_Out32(0xF65D4004, 0x1000);
	Xil_Out32(0xF65D400C, 0x0);

}
/*****************************************************************************/
/**
*
* Function to rnable MMCME5 Fabric Control in PCSR
*
* @param  None.
*
* @return
*
* @note   None.
*
******************************************************************************/
void XMipi_DisplayMainMenu(void)
{

	xil_printf("\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("---   MAIN MENU   ---\r\n");
	xil_printf("---------------------\r\n");

	xil_printf("h - Select Display Device : HDMI\n\r");
	xil_printf("d - Select Display Device : DSI\n\r");
	xil_printf("r - Change the video resolution 2K/4K.\n\r");
do {

			u8 Response;


			Response = XUartPsv_RecvByte(XPAR_XUARTPSV_0_BASEADDR);

			XUartPsv_SendByte(XPAR_XUARTPSV_0_BASEADDR, Response);

			if ((Response == 'r')||(Response == 'R')){
					xil_printf("\r\n0 - 1920x1080p60");
					xil_printf(" => Configures Sensor for 1920x1080 60fps.\r\n");
					xil_printf("\r\n1 - 3840x2160p60");
					xil_printf(" => Configures Sensor for 3840x2160 60fps.\r\n");
					xil_printf("\r\n\r\n");
					xil_printf("\r\nEnter Selection ->\r\n ");
					Response = XUartPsv_RecvByte(XPAR_XUARTPSV_0_BASEADDR);
					XUartPsv_SendByte(XPAR_XUARTPSV_0_BASEADDR, Response);
					if ((Response == '0')) {
						xil_printf("\r\n 1920x1080p60 Resolution is Selected.\r\n");
						New_Cfg.VideoMode = XVIDC_VM_1920x1080_60_P;
						break;
					} else if ((Response == '1')) {
						New_Cfg.VideoMode = XVIDC_VM_3840x2160_60_P;
						xil_printf("\r\n 3840x2160p60 Resolution is Selected.\r\n");
						break;
					} else {
						New_Cfg.VideoMode = XVIDC_VM_1920x1080_60_P;
xil_printf("\r\n Wrong Input Selection, Default(1920x1080p60) is Selected\r\n");
						break;
					}
				} else if ((Response == 'h') ||(Response =='H')){
					xil_printf("\r\n HDMI Display is Selected.\r\n");
					New_Cfg.VideoDestn = XVIDDES_HDMI ;
					break;
				} else if((Response == 'd') || (Response == 'D')){
					xil_printf("\r\n DSI Display is Selected.\r\n");
					New_Cfg.VideoDestn = XVIDDES_DSI ;
					break;
				}else{
					New_Cfg.VideoDestn = XVIDDES_HDMI ;
		xil_printf("\r\n Wrong Input Selection, Default is Selected.\r\n");
					xil_printf("\r\n HDMI Display is Selected.\r\n");
          break;
				}
			} while (1);
}



int main() {

	u32 Status = XST_FAILURE;


	Pipeline_Cfg.ActiveLanes = 4;
	Pipeline_Cfg.VideoSrc = XVIDSRC_SENSOR;
	New_Cfg.VideoMode = XVIDC_VM_1920x1080_60_P;
	/* Default HDMI */
	Pipeline_Cfg.VideoDestn = XVIDDES_HDMI;
	New_Cfg.VideoDestn = XVIDDES_HDMI;
	Pipeline_Cfg.Live = TRUE;

	/* Vertical and Horizontal flip don't work */
	Pipeline_Cfg.Vflip = FALSE;
	Pipeline_Cfg.Hflip = FALSE;

	/* Video pipeline configuration from user */
	Pipeline_Cfg.CameraPresent = TRUE;
	Pipeline_Cfg.DSIDisplayPresent = TRUE;

	/* Default Resolution that to be displayed */
	Pipeline_Cfg.VideoMode = XVIDC_VM_1920x1080_60_P;
	int val = 0;

	xil_printf("\r\n\r\n");
	xil_printf("------------------------------------------\r\n");
	xil_printf("---  Versal MIPI CSI RX Design Example ---\r\n") ;
	xil_printf("---  (c) 2019 by Xilinx, Inc.      -------\r\n");
	xil_printf("------------------------------------------\r\n");
	xil_printf("Build %s - %s\r\n", __DATE__, __TIME__);
	xil_printf("------------------------------------------\r\n");

	xil_printf("Please answer the following questions about the hardware setup.");
	xil_printf("\r\n");


	/* Initialize platform */
	init_platform();
	/*  XPIO DCI Enable */
	xpio_dci_fix();

	/* Enable MMCME5 Fabric Control in
 *                 PCSR - Work around for HDMI ( SIEA Build Only) */
	Enable_mmcmfabric_control();


	XMipi_DisplayMainMenu();

	Pipeline_Cfg.VideoDestn = New_Cfg.VideoDestn ;
	Pipeline_Cfg.VideoMode = New_Cfg.VideoMode ;

	XGpio_Initialize(&Gpio_Tready,XGPIO_TREADY_DEVICE_ID);
	SelectHDMIOutput();

	Reset_IP_Pipe();
	if(Pipeline_Cfg.VideoDestn == XVIDDES_DSI){
	Status = SetupDSI();
		if (Status != XST_SUCCESS) {
		  xil_printf("SetupDSI failed status = %x.\r\n", Status);
		  return XST_FAILURE;
		}
	}
	/* Initialize IRQ */
	Status = SetupInterruptSystem();
	if (Status == XST_FAILURE) {
		xil_printf("IRQ Configuration failed.\r\n");
		return XST_FAILURE;
	}
	Status = config_csi_cap_path();
	if (Status == XST_FAILURE) {
		xil_printf("CSI Capture Pipe Configuration failed.\r\n");
		return XST_FAILURE;
	}
	if(Pipeline_Cfg.VideoDestn == XVIDDES_DSI){
	InitDSI();
	xil_printf("InitDSI Done \n\r");
	}
	Status = config_hdmi();
		if (Status == XST_FAILURE) {
			xil_printf("HDMI  TX Configuration failed.\r\n");
			return XST_FAILURE;
		}
		usleep(5000);
		xil_printf("HDMI  TX Configuration Done.\r\n");
		if(Pipeline_Cfg.VideoDestn == XVIDDES_DSI){
			SelectDSIOutput();
		}else {
			SelectHDMIOutput();
		}

	/* Enable exceptions. */
	Xil_AssertSetCallback((Xil_AssertCallback) Xil_AssertCallbackRoutine);
	Xil_ExceptionEnable();



	/* Start CSI PIPE */
	Status = start_csi_cap_pipe(Pipeline_Cfg.VideoMode);
	if (Status == XST_FAILURE) {
		xil_printf("CSI Cature Pipe Start failed.\r\n");
		return XST_FAILURE;
	}
	if(Pipeline_Cfg.VideoDestn == XVIDDES_DSI){
	EnableDSI();
	}

		/* Main loop */
			do {
				XMipi_DisplayMainMenu();
				Pipeline_Cfg.VideoDestn = New_Cfg.VideoDestn ;
				Pipeline_Cfg.VideoMode = New_Cfg.VideoMode ;
				if (Pipeline_Cfg.VideoDestn == XVIDDES_HDMI) {
					xil_printf("Set HDMI as destination \r\n");
					TxBusy            = (FALSE);
					TxRestartColorbar = (TRUE);
					config_csi_cap_path( );
					SelectHDMIOutput();
					start_csi_cap_pipe(Pipeline_Cfg.VideoMode);
				}

				if (Pipeline_Cfg.VideoDestn == XVIDDES_DSI) {
					xil_printf("Set DSI as destination \r\n");

					XGpio_Initialize(&Gpio_Tready,XGPIO_TREADY_DEVICE_ID);
					Reset_IP_Pipe();
					SetupDSI();
					TxRestartColorbar = (FALSE);
					TxBusy            = (TRUE);
					IsStreamUp        = (FALSE);
					config_csi_cap_path();
					SelectDSIOutput();
					InitDSI();

					start_csi_cap_pipe(Pipeline_Cfg.VideoMode);

					EnableDSI();
				}

            } while (1);

  return 0;
}
