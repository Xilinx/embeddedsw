/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 * scd_memory_mode_ex.c: scenechange IP in memory mode test application.
 *
 * This application configures SceneChange IP to caluclate SAD values between
 * consecutive streams. The driver accepts the threshold values per stream and
 * calls the registered callback with SAD value and stream id.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who    Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  pv   10/10/18   Initial Release
 *			 Added flushing feature support for the driver.
 *			 it supports only for memory based scenechange IP.
 *			 flush bit should be set and held (until reset) by
 *			 software to flush pending transactions.IP is expecting
 *			 a hard reset, when flushing is done.(There is a flush
 *			 status bit and is asserted when the flush is done).
 * <pre>
 */
#include "xparameters.h"
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_io.h"
#include "xil_cache.h"
#include "sleep.h"
#include "xv_scenechange.h"
#ifndef SDT
#include "xscugic.h"
#else
#include "xinterrupt_wrap.h"
#endif

/*Reset all IPs in pipeline */
#define IP_RESET_MASK		0xF
/************** User Configurable Data for testing*******************************/
#define	SCD_STREAMS_ENABLE	1
/* users is changing the data in Layer 0 */
#define SCD_LAYER_0		0
#define MAX_PATTERNS		8

#if defined XPAR_PSU_ACPU_GIC_DEVICE_ID
#define PS_ACPU_GIC_DEVICE_ID XPAR_PSU_ACPU_GIC_DEVICE_ID
#elif defined XPAR_SCUGIC_0_DEVICE_ID
#define PS_ACPU_GIC_DEVICE_ID XPAR_SCUGIC_0_DEVICE_ID
#else
#warning No GIC Device ID found
#endif

#if defined XPAR_GPIO_0_BASEADDR
#define GPIO_BASE XPAR_GPIO_0_BASEADDR
#endif

/* Different patterns to write in the memory location for the streams */
volatile u32 local_mem[8] = {0xFF00FF, 0xFFFF00, 0xFF0000, 0x00FFFF,
	0xFFFF00, 0xFF00FF, 0x00FFFF, 0xFF0000};

/* Different memory location for the streams */
u64 memaddr[8] = {0x10000000, 0x20000000, 0x20400000, 0x20800000,
	0x30000000, 0x30400000, 0x30800000, 0x40000000};

/* below are the configurable parameters: user can give height and width
 * to 4320, 8192 and stride value, histbits, color format, threshold is
 * based on SceneChange IP PG*/
u32 scd_height[XV_SCD_IP_MAX_STREAMS]	= {720, 0, 0, 0, 0, 0, 0, 0};
u32 scd_width[XV_SCD_IP_MAX_STREAMS] 	= {1280, 0, 0, 0, 0, 0, 0, 0};
u32 scd_stride[XV_SCD_IP_MAX_STREAMS]	= {1280, 0, 0, 0, 0, 0, 0, 0};
/* user can configure scd_histbits : 2, 4, 8, 16, 32 and 64. Typical value:16*/
u32 scd_histbits[XV_SCD_IP_MAX_STREAMS]	= {16, 0, 0, 0, 0, 0, 0, 0};
u32 scd_clrfmt[XV_SCD_IP_MAX_STREAMS]	= {XV_SCD_HAS_Y8, 0, 0, 0, 0, 0, 0, 0};
u32 scd_threshold[XV_SCD_IP_MAX_STREAMS] = {1, 0, 0, 0, 0, 0, 0, 0};

/************************** Variable Definitions *****************************/

volatile u8 is_detected, sc_detected;
XScuGic    Intc;
XV_scenechange ScdPtr;

/************************** Function Prototypes ******************************/

void SceneChangeDetectedCallback(void *CallbackRef);

/************************** Function Definitions *****************************/

void ClearScreen(void)
{
	xil_printf("%c\[2J", 27);
	xil_printf("%c\033[0;0H", 27);
}

#ifndef SDT
static int SetupInterruptSystem(void)
{
	int Status;
	XScuGic *IntcInstPtr = &Intc;

	XScuGic_Config *IntcCfgPtr;
	IntcCfgPtr = XScuGic_LookupConfig(PS_ACPU_GIC_DEVICE_ID);
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

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XScuGic_InterruptHandler,
			(XScuGic *)IntcInstPtr);

	return XST_SUCCESS;
}
#endif

void SceneChangeDetectedCallback(void *CallbackRef)
{
	xil_printf("IN CB: Layer:%d SAD:%X\r\n", ScdPtr.ScdDetLayerId,
			ScdPtr.ScdLayerDetSAD);
	is_detected = 1;
}

int XV_SceneChange_init()
{
	XV_scenechange_Config *ScdConfig;
	int Status = 0;
	u32 streams = 0;

#ifndef SDT
	ScdConfig = XV_scenechange_LookupConfig(XPAR_XV_SCENECHANGE_0_DEVICE_ID);
#else
	ScdConfig = XV_scenechange_LookupConfig(XPAR_XV_SCENECHANGE_0_BASEADDR);
#endif
	if (ScdConfig == NULL)
		return XST_FAILURE;

	/* Initialize top level and all included sub-cores */
#ifndef SDT
	Status = XV_scenechange_Initialize(&ScdPtr, XPAR_XV_SCENECHANGE_0_DEVICE_ID);
#else
	Status = XV_scenechange_Initialize(&ScdPtr, XPAR_XV_SCENECHANGE_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	if (ScdPtr.ScdConfig->MemoryBased == XV_SCD_STREAM_MODE) {
		xil_printf("Application meant for memory mode\r\n");
		return XST_FAILURE;
	}

	XV_scenechange_Layer_stream_enable(&ScdPtr, SCD_STREAMS_ENABLE);

	for (streams = 0; streams < ScdPtr.ScdConfig->NumStreams; streams++) {
		if (SCD_STREAMS_ENABLE & (1 << streams)) {
			xil_printf("(%d) stream configuring...\r\n", streams);
			ScdPtr.LayerConfig[streams].Height	= scd_height[streams];
			ScdPtr.LayerConfig[streams].Width	= scd_width[streams];
			ScdPtr.LayerConfig[streams].Stride	= scd_stride[streams];
			ScdPtr.LayerConfig[streams].VFormat	= scd_clrfmt[streams];
			ScdPtr.LayerConfig[streams].SubSample	= scd_histbits[streams];
			ScdPtr.LayerConfig[streams].BufferAddr	= memaddr[streams];
			ScdPtr.LayerConfig[streams].Threshold	= scd_threshold[streams];
			Status = XV_scenechange_Layer_config(&ScdPtr, streams);
			if(Status == XST_FAILURE) {
				xil_printf("ERR:: Unable to configure SD Layer\r\n");
				return XST_FAILURE;
			}
			memset((char *)memaddr[streams], local_mem[streams],
					scd_height[streams] * scd_width[streams]);
		}
	}

#ifndef SDT
	Status = SetupInterruptSystem();
	if (Status != XST_SUCCESS) {
		xil_printf("SetupInterruptSystem() is Failed.\r\n");
		return XST_FAILURE;
	}
	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_SCENECHANGE_0_VEC_ID,
			(XInterruptHandler)XV_scenechange_InterruptHandler,
			(void *)&ScdPtr);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc, XPAR_FABRIC_V_SCENECHANGE_0_VEC_ID);
	} else {
		xil_printf("ERR:: Unable to register SD interrupt handler");
		return XST_FAILURE;
	}
#else
	Status = XSetupInterruptSystem(&ScdPtr,&XV_scenechange_InterruptHandler,
				       ScdPtr.ScdConfig->IntrId,
				       ScdPtr.ScdConfig->IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
	if (Status != XST_SUCCESS) {
		xil_printf("XSetupInterruptSystem() is Failed.\r\n");
		return XST_FAILURE;
	}
#endif

	XV_scenechange_SetCallback(&ScdPtr, SceneChangeDetectedCallback,
			(void *) &ScdPtr);

	XV_scenechange_InterruptGlobalEnable(&ScdPtr);
	XV_scenechange_EnableInterrupts(&ScdPtr);
	XV_scenechange_EnableAutoRestart(&ScdPtr);
	XV_scenechange_Start(&ScdPtr);

	return XST_SUCCESS;
}

void reset_pipe(void)
{

#if defined XPAR_GPIO_0_BASEADDR
	u32 count;
	*(u32 *)(GPIO_BASE) = 0xFF;
	for (count = 0; count <1000; count++);
	*(u32 *)(GPIO_BASE) = 0x0;
	for (count = 0; count <1000; count++);
	*(u32 *)(GPIO_BASE) = 0xFF;
	for (count = 0; count <1000; count++);
#else
	Xil_Out32(0xFF0A0018, 0xFFFF0000);
	Xil_Out32(0xFF0A02C4, 0xFFFFFFFF);
	Xil_Out32(0xFF0A02C8, 0xFFFFFFFF);
	Xil_Out32(0xFF0A004C, IP_RESET_MASK);
	Xil_Out32(0xFF0A004C, 0x00000000);
	Xil_Out32(0xFF0A004C, IP_RESET_MASK);
#endif

	xil_printf("Reset SCD - Done.\r\n");
}

int main()
{
	u8 state;

	ClearScreen();

	xil_printf("-----------------------------------------\r\n");
	xil_printf("---   SceneChange IP Application --------\r\n");
	xil_printf("---     (c) 2018 by Xilinx, Inc.     ----\r\n");
	xil_printf("----------------------------------------\r\n");
	xil_printf("      Build %s - %s \r\n", __DATE__, __TIME__);
	xil_printf("-----------------------------------------\r\n");

	Xil_DCacheDisable();
	Xil_ExceptionDisable();
	init_platform();

	reset_pipe();

	xil_printf("SceneChange initialization - Started\r\n");

	state = XV_SceneChange_init();
	if (state != XST_SUCCESS) {
		xil_printf("SceneChange_init Failed.\n");
		return XST_FAILURE;
	}
	Xil_ExceptionEnable();

	do {
		if(is_detected) {
			is_detected = 0;
			sc_detected++;
		}

		if(++state >= MAX_PATTERNS) {
			if(sc_detected)
				goto SCD_DONE;
			else
				goto SCD_FAILED;
		}

		/* pattern is changing on Layer-0 to test - used can configure*/
		memset((char *)memaddr[SCD_LAYER_0], local_mem[state],
				scd_height[SCD_LAYER_0] * scd_width[SCD_LAYER_0]);
		sleep(1);
	} while(1);

SCD_DONE:
	XV_scenechange_Stop(&ScdPtr);
	reset_pipe();
	XV_scenechange_WaitForIdle(&ScdPtr);
	xil_printf("SceneChange test : PASSED\r\n");
	return XST_SUCCESS;

SCD_FAILED:
	XV_scenechange_Stop(&ScdPtr);
	reset_pipe();
	XV_scenechange_WaitForIdle(&ScdPtr);
	xil_printf("SceneChange test : FAILURE\r\n");
	return  XST_FAILURE;
}
