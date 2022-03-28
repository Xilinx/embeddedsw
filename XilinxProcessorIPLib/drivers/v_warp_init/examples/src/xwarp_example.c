/******************************************************************************
* Copyright (C) 2008 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xil_cache.h"
#include "xparameters.h"
#include "xscugic.h"
#include "xil_types.h"
#include "xil_io.h"
#include "sleep.h"
#include "xv_warp_filter_hw.h"
#include "xv_warp_filter_l2.h"
#include "xv_warp_init_l2.h"
#include "xwarp_input_configs.h"
#include "inputs.h"
#include "xgpio.h"

/************************** Local Constants *********************************/
XGpio              Gpio_WarpInit_resetn;
XGpio_Config       *Gpio_WarpInit_resetn_ConfigPtr;

XGpio              Gpio_Filter_resetn;
XGpio_Config       *Gpio_Filter_resetn_ConfigPtr;
/***************** Macros (Inline Functions) Definitions *********************/
#define XHls_IP_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XHls_IP_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))

/************************** Variable Definitions *****************************/
XScuGic					Intc;
XV_warp_filter			WarpInst;
XVWarpFilter_Desc				warp_descriptor;
XV_warp_init			WarpInitInst;
volatile u8 			frame_done_interrupt = 0;
volatile u32			frame_done_intr_count = 0;
volatile u8 			gen_remap_vec_int = 0;

warp_driver_Configs warp_drv_configs;

u32 *tmrctr_tcsr0_ptr =	(u32 *)0xA0000000;
u32 *tmrctr_tlr0_ptr =	(u32 *)0xA0000004;
u32 *tmrctr_tcr0_ptr =	(u32 *)0xA0000008;
u32 *tmrctr_tcr1_ptr =	(u32 *)0xA0000018;
u32 tmrctr_value, tmrctr_1_value;

/*****************************************************************************/
static void XBInterruptHandler(void *CallbackRef);
static int SetupInterruptSystem(void);
static void XV_Reset_Warp(void);
static void XWarpInit_callback(void *CallbackRef);
static void start_timer(void);
static void stop_timer(void);

void ResetWarpInit(void)
{
	XGpio_SetDataDirection(&Gpio_WarpInit_resetn, 1, 0);
	XGpio_DiscreteWrite(&Gpio_WarpInit_resetn, 1, 0);
	usleep(1000);

	XGpio_DiscreteWrite(&Gpio_WarpInit_resetn, 1, 1);
	usleep(1000);
}

void ResetFilter(void)
{
	XGpio_SetDataDirection(&Gpio_Filter_resetn, 1, 0);
	XGpio_DiscreteWrite(&Gpio_Filter_resetn, 1, 0);
	usleep(1000);

	XGpio_DiscreteWrite(&Gpio_Filter_resetn, 1, 1);
	usleep(1000);
}


/*****************************************************************************/
int main(void)
{
	u32 status;
	XVWarpFilter_InputConfigs		*filter_configs_ptr;
	u32 valid_seg;
	u32 lblock_count;
	u32 line_num;

	/* Initialize ICache */
	Xil_ICacheInvalidate();
	Xil_ICacheDisable();

	/* Initialize DCache */
	Xil_DCacheInvalidate();
	Xil_DCacheDisable();

	Xil_ExceptionDisable();

	xil_printf("\r\n-----------------------------------------------\r\n");
	xil_printf("	(c) 2020 by Xilinx Inc.\r\n");
	xil_printf("-----------------------------------------------\r\n");

	/**************V_WARP_REMAP_GEN*************************/
	/* Initialize GPIO for WarpInit Reset */
	Gpio_WarpInit_resetn_ConfigPtr =
		XGpio_LookupConfig(XPAR_AXI_GPIO_1_DEVICE_ID);

	if(Gpio_WarpInit_resetn_ConfigPtr == NULL) {
		Gpio_WarpInit_resetn.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	status = XGpio_CfgInitialize(&Gpio_WarpInit_resetn,
				Gpio_WarpInit_resetn_ConfigPtr,
				Gpio_WarpInit_resetn_ConfigPtr->BaseAddress);
	if(status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for Warp_Init Reset ");
		xil_printf("Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

	status = XV_warp_init_Initialize(&WarpInitInst,
			XPAR_V_WARP_INIT_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		xil_printf("Warp Initializer IP initialization failed.\n\r");
		return XST_FAILURE;
	}
	XVWarpInit_SetNumOfDescriptors(&WarpInitInst, 2);
	ResetWarpInit();


	/**************V_WARP_FILTER*************************/
	/* Initialize GPIO for WarpFilter Reset */
	Gpio_Filter_resetn_ConfigPtr =
		XGpio_LookupConfig(XPAR_AXI_GPIO_0_DEVICE_ID);

	if(Gpio_Filter_resetn_ConfigPtr == NULL) {
		Gpio_Filter_resetn.IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	status = XGpio_CfgInitialize(&Gpio_Filter_resetn,
				Gpio_Filter_resetn_ConfigPtr,
				Gpio_Filter_resetn_ConfigPtr->BaseAddress);
	if(status != XST_SUCCESS) {
		xil_printf("ERR:: GPIO for FILTER Reset ");
		xil_printf("Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

	status = XV_warp_filter_Initialize(&WarpInst, XPAR_V_WARP_FILTER_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		xil_printf("Warp Filter initialization failed.\n\r");
		return XST_FAILURE;
	}
	XVWarpFilter_SetNumOfDescriptors(&WarpInst, 2);
	ResetFilter();


	/* Initialize IRQ */
	status = SetupInterruptSystem();
	if (status == XST_FAILURE) {
		xil_printf("IRQ init failed.\n\r\r");
		return XST_FAILURE;
	}


	get_init_vect_input_configs(&warp_drv_configs, &input_configs);

	xil_printf("Configuring warp_init IP\n\r");
	XVWarpInit_ProgramDescriptor(&WarpInitInst, 0,
			&warp_drv_configs.initVectConfigs);
	XVWarpInit_EnableInterrupts(&WarpInitInst,
			XV_WARP_INIT_INTR_AP_DONE_MASK);

	xil_printf("Starting Remap_vector_generation IP\n\r");

	start_timer();

	/* Start the IP */
	XVWarpInit_start_with_desc(&WarpInitInst, 0);
	while (!gen_remap_vec_int) ;
	gen_remap_vec_int = 0;
	stop_timer();
	xil_printf("Got Remap vector generation Interrupt...Time taken = %u .....0\n\r",
			((tmrctr_1_value * (0xFFFFFFFF/322)) + tmrctr_value/322));

	/*Configure Warp inputs*/
	valid_seg = ((u32*)WarpInitInst.RemapVectorDesc_BaseAddr)[53];
    lblock_count = ((u32*)WarpInitInst.RemapVectorDesc_BaseAddr)[54];
    line_num = ((u32*)WarpInitInst.RemapVectorDesc_BaseAddr)[55];
    xil_printf("Valid_Segs = %d, lblk_count = %d, line_num = %d\n\r",
               valid_seg, lblock_count, line_num);

	filter_configs_ptr = &warp_drv_configs.filterConfigs;
	filter_configs_ptr->src_buf_addr = SRC_BUF_START_ADDR;
	filter_configs_ptr->dest_buf_addr = DST_BUF_START_ADDR;
	XVWarpFilter_ProgramDescriptor(&WarpInst, 0, filter_configs_ptr,
			valid_seg, lblock_count, line_num);

	/* Enable Interrupts */
	XVWarpFilter_InterruptEnable(&WarpInst, XV_WARP_FILTER_INTR_AP_DONE_MASK);

	start_timer();

	/* Start the IP */
	XVWarpFilter_Start(&WarpInst);

	while (1) {

		while (frame_done_interrupt != 1);
		frame_done_interrupt = 0;

		frame_done_intr_count++;
		if (frame_done_intr_count >= 1) {
			stop_timer();
			break;
		} else {
			/*Moving the dst frame address to next available mem location*/
			if (filter_configs_ptr->dest_buf_addr >= BUFF_QUEUE_SIZE) {
				filter_configs_ptr->dest_buf_addr = DST_BUF_START_ADDR;
			} else {
				filter_configs_ptr->dest_buf_addr += MAX_SIZE_OF_4K_FRAME_IN_MEM;
			}
			XVWarpFilter_update_src_frame_addr(&WarpInst, 0,
					filter_configs_ptr->src_buf_addr);
			XVWarpFilter_update_dst_frame_addr(&WarpInst, 0,
					filter_configs_ptr->dest_buf_addr);
			/*Restart the IP*/
			XVWarpFilter_Start(&WarpInst);
		}
	}

	xil_printf("Got Frame done Interrupt...Time taken = %u\n\r",
			((tmrctr_1_value * (0xFFFFFFFF/322)) + tmrctr_value/322));

	int out_crc = 0;
	unsigned char *dest = (unsigned char *)DST_BUF_START_ADDR;

	for (int i = 0; i < filter_configs_ptr->height * filter_configs_ptr->stride; i++)
		out_crc = out_crc ^ dest[i];

	if (out_crc == warp_drv_configs.golden_crc)
		xil_printf("\nTest Pass\n");

	XVWarpInit_ClearNumOfDescriptors(&WarpInitInst);
	XVWarpFilter_ClearNumOfDescriptors(&WarpInst);

	return 0;
}

/***********************Staic Funtction definitions***************************/
static void start_timer(void)
{
	*tmrctr_tcsr0_ptr = 0x820;
	usleep(10);
	*tmrctr_tcsr0_ptr = 0x880;
}

static void stop_timer(void)
{
	*tmrctr_tcsr0_ptr = 0x00;
	tmrctr_1_value = *tmrctr_tcr1_ptr;
	tmrctr_value = *tmrctr_tcr0_ptr;
}

static void XWarpInit_callback(void *CallbackRef)
{
	gen_remap_vec_int = 1;
}

/*****************************************************************************/
/**
 *
 ******************************************************************************/
static void XBInterruptHandler(void *CallbackRef)
{
	frame_done_interrupt = 1;
}


/*****************************************************************************/
/**
 *
 * This function setups the interrupt system so interrupts can occur for the
 * Warp core.
 *
 * @return
 *	- XST_SUCCESS if interrupt setup was successful.
 *	- A specific error code defined in "xstatus.h" if an error
 *	occurs.
 *
 * @note	This function assumes a Microblaze or ARM system and no
 *	operating system is used.
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
	Status = XScuGic_CfgInitialize(IntcInstPtr, IntcCfgPtr,
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
		(Xil_ExceptionHandler) XScuGic_InterruptHandler,
		(XScuGic *)IntcInstPtr);


	XVWarpInit_SetCallback(&WarpInitInst,
				(void *)XWarpInit_callback,
				(void *)&WarpInitInst);
	Status = XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_WARP_INIT_0_INTERRUPT_INTR,
			(XV_warp_init_Callback)XVWarpInit_IntrHandler,
			(void *)&WarpInitInst);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc,
				XPAR_FABRIC_V_WARP_INIT_0_INTERRUPT_INTR);
	} else {
		xil_printf("ERR:: Unable to register Remap vector generator interrupt handler");
		return XST_FAILURE;
	}

	XVWarpFilter_SetCallback(&WarpInst, XBInterruptHandler, &WarpInst);
	Status = XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_WARP_FILTER_0_INTERRUPT_INTR,
			(XV_warp_filter_Callback)XVWarpFilter_IntrHandler,
			(void *)&WarpInst);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc,
				XPAR_FABRIC_V_WARP_FILTER_0_INTERRUPT_INTR);
	} else {
		xil_printf("ERR:: Unable to register Warp Filter interrupt handler");
		return XST_FAILURE;
	}

	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

#if 0
static void XV_Reset_Warp(void)
{
	XHls_IP_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
        XGPIOPS_MASK_DATA_3_LSW_OFFSET, 0xFFFF0000);
	XHls_IP_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
        XGPIOPS_DIRM_3_OFFSET, 0xFFFFFFFF);
	XHls_IP_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
        XGPIOPS_OEN_3_OFFSET, 0xFFFFFFFF);
	XHls_IP_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
        XGPIOPS_DATA_3_OFFSET, 0x00000001);
	XHls_IP_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
        XGPIOPS_DATA_3_OFFSET, 0x00000000);
	XHls_IP_WriteReg(XPAR_PSU_GPIO_0_BASEADDR,
        XGPIOPS_DATA_3_OFFSET, 0x00000001);
}
#endif
