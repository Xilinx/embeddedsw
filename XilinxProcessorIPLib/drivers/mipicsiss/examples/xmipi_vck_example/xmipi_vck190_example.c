/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
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

int config_hdmi();
void start_hdmi(XVidC_VideoMode VideoMode);
int config_csi_cap_path();
int start_csi_cap_pipe(XVidC_VideoMode VideoMode);

XScuGic     Intc;

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
int XMipi_DisplayMainMenu(void)
{
	int VideoMode_Select;
	xil_printf("\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("---   MAIN MENU   ---\r\n");
	xil_printf("---------------------\r\n");
	xil_printf("0 - 1920x1080p60\r\n");
xil_printf(" => Configures Sensor for 1920x1080 60fps.\r\n");
	xil_printf("1 - 3840x2160p60\r\n");
xil_printf(" => Configures Sensor for 3840x2160 60fps.\r\n");


	xil_printf("\r\n\r\n");
	xil_printf("Enter Selection -> ");

do {
			u8 Response;

			Response = XUartPsv_RecvByte(XPAR_XUARTPSV_0_BASEADDR);

			XUartPsv_SendByte(XPAR_XUARTPSV_0_BASEADDR, Response);


				if ((Response == '0')) {
					VideoMode_Select = 0 ;
					xil_printf("\r\n\r\n 1920x1080p60 is Selected.\r\n");
					break;
				} else if ((Response == '1')) {
                    VideoMode_Select = 1 ;
					xil_printf("\r\n\r\n 3840x2160p60 is Selected.\r\n");
					break;
				} else if ((Response != 0)) {
					VideoMode_Select = 0 ;
					xil_printf("\r\n\r\n Wrong Input \\
                        Selection,Default (1080p60) Output is Selected.\r\n");
					break;
				}
			} while (1);

	return (VideoMode_Select) ;

}
int main() {

	u32 Status = XST_FAILURE;
	XVidC_VideoMode VideoMode_CSI;
	XVidC_VideoMode VideoMode_HDMI;
	int val = 0;
	xil_printf("\r\n\r\n");
	xil_printf("------------------------------------------\r\n");
	xil_printf("---  Versal MIPI CSI RX Design Example ---\r\n") ;
	xil_printf("---  (c) 2019 by Xilinx, Inc.      -------\r\n");
	xil_printf("------------------------------------------\r\n");
	xil_printf("Build %s - %s\r\n", __DATE__, __TIME__);
	xil_printf("------------------------------------------\r\n");

	/* Initialize platform */
	init_platform();
	/*  XPIO DCI Enable */
	xpio_dci_fix();

	/*  XPIO equalization_fix */
//	xpio_equalization_fix();

	/* Enable MMCME5 Fabric Control in
 *                 PCSR - Work around for HDMI ( SIEA Build Only) */
	Enable_mmcmfabric_control();

	/* Initialize IRQ */
	Status = SetupInterruptSystem();
	if (Status == XST_FAILURE) {
		xil_printf("\r\n\r\n IRQ Configuration failed.\r\n\r\n");
		return XST_FAILURE;
	}


	val = XMipi_DisplayMainMenu();

	if(val == 0){
		 VideoMode_CSI =  XVIDC_VM_1920x1080_60_P ;
		 VideoMode_HDMI =  XVIDC_VM_1920x1080_60_P ;
	}
	else{
		 VideoMode_CSI =  XVIDC_VM_3840x2160_60_P ;
		 VideoMode_HDMI =  XVIDC_VM_1920x1080_60_P ;
	}

	Status = config_hdmi();
	if (Status == XST_FAILURE) {
		xil_printf("\r\n\r\n HDMI  TX Configuration failed.\r\n\r\n");
		return XST_FAILURE;
	}

	Status = config_csi_cap_path();
	if (Status == XST_FAILURE) {
		xil_printf("\r\n\r\n CSI Cature Pipe \\
                                      Configuration failed.\r\n\r\n");
		return XST_FAILURE;
	}

	/* Enable exceptions. */
	Xil_AssertSetCallback((Xil_AssertCallback) Xil_AssertCallbackRoutine);
	Xil_ExceptionEnable();

	/* Start HDMI */
    start_hdmi(VideoMode_HDMI);

	/* Start CSI PIPE */

	Status = start_csi_cap_pipe(VideoMode_CSI);
	if (Status == XST_FAILURE) {
		xil_printf("\r\n\r\n CSI Cature Pipe Start failed.\r\n\r\n");
		return XST_FAILURE;
	}
	return 0;
}
