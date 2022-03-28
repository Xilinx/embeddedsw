/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.      All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/*
 * scd_stream_mode_ex.c: scenechange IP in stream mode test application.
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
#include "xscugic.h"
#include "xil_io.h"
#include "xil_cache.h"
#include "sleep.h"
#include "xvidc.h"
#include "xv_scenechange.h"
#include "xscugic.h"
#include "xv_tpg.h"
#include "xv_frmbufwr.h"
#include "xv_frmbufwr_l2.h"

/* Reset IPs in pipeline using IP_RESET_MASK*/
#define IP_RESET_MASK		0xF

/************** User Configurable Data for testing*******************************/
/* user can configure till 8 streams, each bit is designated for one stream
 * in integer in IP register*/
#define	SCD_STREAMS_ENABLE	1
/* Test formats for the TPG */
#define MAX_PATTERNS		8
/*Please refer framebuffer PG278 to get number of bytes per color and
 * color format representation. So, User can pass any format in
 * FB_COLOR_FORMAT, FB_CLRFMT_BYTES */
#define FB_COLOR_FORMAT		20
#define FB_CLRFMT_BYTES		3

/* User can pass any memory address */
u64 memaddr		= 0x10000000;
/* IP supports till 4320 as height*/
u32 scd_height		= 720;
/* IP supports till 8192 as width*/
u32 scd_width		= 1280;
u32 scd_stride		= 1280;
/* user can configure hist bits : 2, 4, 8, 16, 32 and 64. Typical value is 16*/
u32 scd_histbits	= 16;
/* Two color formats are supported by IP: XV_SCD_HAS_Y8, XV_SCD_HAS_Y10 */
u32 scd_clrfmt		= XV_SCD_HAS_Y8;
u32 scd_threshold	= 1;

/************************** Variable Definitions *****************************/

XV_tpg Tpg;
XV_tpg_Config *Tpg_ConfigPtr;
XTpg_PatternId Pattern; /**< Video pattern */
XV_FrmbufWr_l2     Frmbufwr;
XV_frmbufwr_Config *FrmBufWr_ConfigPtr;
XScuGic    Intc;
XV_scenechange ScdPtr;
volatile u8 is_detected, sc_detected;

/************************** Function Prototypes ******************************/

void SceneChangeDetectedCallback(void *CallbackRef);
void XV_ConfigTpg(XV_tpg *InstancePtr);
void ResetTpg(void);

/************************** Function Definitions *****************************/

void ClearScreen(void)
{
	xil_printf("%c\[2J", 27);
	xil_printf("%c\033[0;0H", 27);
}

static int SetupInterruptSystem(void)
{
	int Status;
	XScuGic *IntcInstPtr = &Intc;

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

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XScuGic_InterruptHandler,
			(XScuGic *)IntcInstPtr);

	return XST_SUCCESS;
}

void SceneChangeDetectedCallback(void *CallbackRef)
{
	xil_printf("IN CB: Layer:%d SAD:%X\r\n", ScdPtr.ScdDetLayerId,
			ScdPtr.ScdLayerDetSAD);
	is_detected = 1;
}

int XV_SceneChange_init(u16 DeviceId)
{
	XV_scenechange_Config *ScdConfig;
	int Status;
	u32 streams = SCD_STREAMS_ENABLE - 1;

	ScdConfig = XV_scenechange_LookupConfig(DeviceId);
	if (ScdConfig == NULL)
		return XST_FAILURE;

	/* Initialize top level and all included sub-cores */
	Status = XV_scenechange_Initialize(&ScdPtr, DeviceId);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	if (ScdPtr.ScdConfig->MemoryBased == XV_SCD_MEMORY_MODE) {
		xil_printf("Application meant for stream mode\r\n");
		return XST_FAILURE;
	}

	XV_scenechange_Layer_stream_enable(&ScdPtr, SCD_STREAMS_ENABLE);

	ScdPtr.LayerConfig[streams].Height	= scd_height;
	ScdPtr.LayerConfig[streams].Width	= scd_width;
	ScdPtr.LayerConfig[streams].Stride	= scd_stride;
	ScdPtr.LayerConfig[streams].VFormat	= scd_clrfmt;
	ScdPtr.LayerConfig[streams].SubSample	= scd_histbits;
	ScdPtr.LayerConfig[streams].BufferAddr	= memaddr;
	ScdPtr.LayerConfig[streams].Threshold	= scd_threshold;

	Status = XV_scenechange_Layer_config(&ScdPtr, streams);
	if(Status == XST_FAILURE) {
		xil_printf("ERR:: Unable to configure SD Layer\r\n");
		return XST_FAILURE;
	}

	XV_scenechange_SetCallback(&ScdPtr, SceneChangeDetectedCallback,
			(void *) &ScdPtr);

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

	XV_scenechange_InterruptGlobalEnable(&ScdPtr);
	XV_scenechange_EnableInterrupts(&ScdPtr);
	XV_scenechange_EnableAutoRestart(&ScdPtr);
	XV_scenechange_Start(&ScdPtr);

	return XST_SUCCESS;
}

void reset_pipe(void)
{
	Xil_Out32(0xFF0A0018, 0xFFFF0000);
	Xil_Out32(0xFF0A02C4, 0xFFFFFFFF);
	Xil_Out32(0xFF0A02C8, 0xFFFFFFFF);
	Xil_Out32(0xFF0A004C, IP_RESET_MASK);
	Xil_Out32(0xFF0A004C, 0x00000000);
	Xil_Out32(0xFF0A004C, IP_RESET_MASK);

	xil_printf("Reset SCD - Done.\r\n");
}

void XV_ConfigTpg(XV_tpg *InstancePtr) {
	XV_tpg *pTpg = InstancePtr;

	XV_tpg_DisableAutoRestart(pTpg);
	XV_tpg_Set_height(pTpg, scd_height);
	XV_tpg_Set_width(pTpg, scd_width);
	XV_tpg_Set_colorFormat(pTpg, XVIDC_CSF_YONLY);
	XV_tpg_Set_bckgndId(pTpg, Pattern);
	XV_tpg_Set_ovrlayId(pTpg, 0);
	XV_tpg_Set_enableInput(pTpg, 0);
	XV_tpg_Set_passthruStartX(pTpg, 0);
	XV_tpg_Set_passthruStartY(pTpg, 0);
	XV_tpg_Set_passthruEndX(pTpg, scd_width);
	XV_tpg_Set_passthruEndY(pTpg, scd_height);
	XV_tpg_EnableAutoRestart(pTpg);
	XV_tpg_Start(pTpg);
}

void FrameBuf_Config() {
	XV_frmbufwr_Set_HwReg_width(&Frmbufwr.FrmbufWr,
			scd_width);
	XV_frmbufwr_Set_HwReg_height(&Frmbufwr.FrmbufWr,
			scd_height);
	XV_frmbufwr_Set_HwReg_stride(&Frmbufwr.FrmbufWr,
			scd_width * FB_CLRFMT_BYTES);
	XV_frmbufwr_Set_HwReg_video_format(&Frmbufwr.FrmbufWr, FB_COLOR_FORMAT);
	XV_frmbufwr_Set_HwReg_frm_buffer_V(&Frmbufwr.FrmbufWr,
			memaddr);
	XV_frmbufwr_EnableAutoRestart(&Frmbufwr.FrmbufWr);
	XV_frmbufwr_Start(&Frmbufwr.FrmbufWr);
}

u32 scd_stream_mode_pipe_init(void)
{
	u8 Status;

	/* Initialize TPG IP */
	Tpg_ConfigPtr = XV_tpg_LookupConfig(XPAR_XV_TPG_0_DEVICE_ID);

	if (!Tpg_ConfigPtr) {
		Tpg.IsReady = 0;
		xil_printf("TPG - failed...\r\n");
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XV_tpg_CfgInitialize(&Tpg, Tpg_ConfigPtr,
			Tpg_ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: TPG Initialization failed %d\r\n", Status);
		return (XST_FAILURE);
	}
	/* Initialize FrameBufWr IP */
	FrmBufWr_ConfigPtr = XV_frmbufwr_LookupConfig(XPAR_V_FRMBUF_WR_0_DEVICE_ID);

	if (!FrmBufWr_ConfigPtr) {
		xil_printf("FBWR - failed...\r\n");
		return (XST_DEVICE_NOT_FOUND);
	}

	Status = XVFrmbufWr_Initialize(&Frmbufwr, XPAR_V_FRMBUF_WR_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("ERR:: FrmBufWr Initialization failed %d\r\n", Status);
		return (XST_FAILURE);
	}

	Status |= XScuGic_Connect(&Intc,
			XPAR_FABRIC_V_FRMBUF_WR_0_VEC_ID,
			(XInterruptHandler)XVFrmbufWr_InterruptHandler,
			(void *)&Frmbufwr);
	if (Status == XST_SUCCESS) {
		XScuGic_Enable(&Intc, XPAR_FABRIC_V_FRMBUF_WR_0_VEC_ID);
	} else {
		xil_printf("ERR:: Unable to register SD interrupt handler");
		return XST_FAILURE;
	}

	XV_frmbufwr_InterruptGlobalEnable(&Frmbufwr.FrmbufWr);
	XV_frmbufwr_InterruptEnable(&Frmbufwr.FrmbufWr, XVFRMBUFWR_IRQ_DONE_MASK);

	FrameBuf_Config();

	return XST_SUCCESS;
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

	state = SetupInterruptSystem();
	if (state != XST_SUCCESS) {
		xil_printf("SetupInterrupt() is Failed.\r\n");
		return XST_FAILURE;
	}

	reset_pipe();

	state = Pattern;
	state = scd_stream_mode_pipe_init();
	if (state != XST_SUCCESS) {
		xil_printf("scd_ stream mode pipe Failed.\n");
		return XST_FAILURE;
	}
	xil_printf("SceneChange initialization - Started\r\n");

	state = XV_SceneChange_init(XPAR_XV_SCENECHANGE_0_DEVICE_ID);
	if (state != XST_SUCCESS) {
		xil_printf("SceneChange_init Failed.\n");
		return XST_FAILURE;
	}
	Xil_ExceptionEnable();

	state = XTPG_BKGND_H_RAMP;
	XV_ConfigTpg(&Tpg);
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

		Pattern = state;
		XV_tpg_Set_bckgndId(&Tpg, Pattern);
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
