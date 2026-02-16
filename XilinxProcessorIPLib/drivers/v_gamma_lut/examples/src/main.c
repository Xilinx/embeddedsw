/******************************************************************************
* Copyright (C) 2008 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file main.c
 * @addtogroup v_gamma_lut Overview
 */

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "sleep.h"
#include "xv_gamma_lut.h"
#include "xv_tpg.h"
#include "xvtc.h"
#include "xvidc.h"

/***************** Macros (Inline Functions) Definitions *********************/
/** Writes a value to the Video Clock Generator register */
#define VideoClockGen_WriteReg(RegOffset, Data) \
    Xil_Out32((XPAR_VIDEO_CLK_WIZ_BASEADDR) + (RegOffset), (u32)(Data))

/** Reads a value from the Video Clock Generator register */
#define VideoClockGen_ReadReg(RegOffset) \
    Xil_In32((XPAR_VIDEO_CLK_WIZ_BASEADDR) + (RegOffset))


/************************** Variable Definitions *****************************/
XV_tpg_Config       *tpg_Config;
XV_tpg              tpg;

XV_gamma_lut_Config     *gamma_lut_Config;
XV_gamma_lut            gamma_lut;

XVtc                vtc;
XVtc_Config         *vtc_Config;
XVtc_Timing         vtc_timing;

u32 volatile        *gpio_hlsIpReset;
u32 volatile        *gpio_videoLockMonitor;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief Initializes video pipeline IP drivers.
 *
 * This function initializes the VTC (Video Timing Controller), TPG (Test
 * Pattern Generator), and Gamma LUT IP cores by looking up their
 * configurations and performing configuration initialization.
 *
 * @return XST_SUCCESS on successful initialization, XST_DEVICE_NOT_FOUND if
 *         any device is not found, or XST_FAILURE if initialization fails.
 *
 * @note This function supports both SDT and non-SDT build configurations.
 *
 *******************************************************************************/
int driverInit()
{
	int status;

#ifndef SDT
	vtc_Config = XVtc_LookupConfig(XPAR_V_TC_0_DEVICE_ID);
#else
    vtc_Config = XVtc_LookupConfig(XPAR_XVTC_0_BASEADDR);
#endif
	if(vtc_Config == NULL)
	{
		xil_printf("ERROR:: VTC device not found\r\n");
		return(XST_DEVICE_NOT_FOUND);
	}
	status = XVtc_CfgInitialize(&vtc, vtc_Config, vtc_Config->BaseAddress);
	if(status != XST_SUCCESS)
	{
		xil_printf("ERROR:: VTC Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

#ifndef SDT
	tpg_Config = XV_tpg_LookupConfig(XPAR_V_TPG_0_DEVICE_ID);
#else
    tpg_Config = XV_tpg_LookupConfig(XPAR_XV_TPG_0_BASEADDR);
#endif
	if(tpg_Config == NULL)
	{
		xil_printf("ERROR:: TPG device not found\r\n");
		return(XST_DEVICE_NOT_FOUND);
	}
	status = XV_tpg_CfgInitialize(&tpg, tpg_Config, tpg_Config->BaseAddress);
	if(status != XST_SUCCESS)
	{
		xil_printf("ERROR:: TPG Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

#ifndef SDT
	gamma_lut_Config = XV_gamma_lut_LookupConfig(XPAR_V_GAMMA_LUT_0_DEVICE_ID);
#else
    gamma_lut_Config = XV_gamma_lut_LookupConfig(XPAR_XV_GAMMA_LUT_0_BASEADDR);
#endif
	if(gamma_lut_Config == NULL)
	{
		xil_printf("ERROR:: Gamma LUT device not found\r\n");
		return(XST_DEVICE_NOT_FOUND);
	}
	status = XV_gamma_lut_CfgInitialize(&gamma_lut, gamma_lut_Config, gamma_lut_Config->BaseAddress);
	if(status != XST_SUCCESS)
	{
		xil_printf("ERROR:: Gamma LUT Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

	return(XST_SUCCESS);
}

/*****************************************************************************/
/**
 * @brief Configures video IP cores for the specified video mode.
 *
 * This function configures the TPG (Test Pattern Generator), Gamma LUT, and
 * VTC (Video Timing Controller) for the specified video resolution and timing.
 * It sets up the frame dimensions, color format, pattern type, and timing
 * parameters based on the requested video mode.
 *
 * @param videoMode The video mode identifier (e.g., 1080p60, 4K30, 4K60).
 *
 * @return None
 *
 * @note The TPG is configured to generate color bars pattern.
 *
 *******************************************************************************/
void videoIpConfig(XVidC_VideoMode videoMode)
{
	XVidC_VideoTiming const *timing = XVidC_GetTimingInfo(videoMode);
	u16 PixelsPerClk;

	XV_tpg_Set_height(&tpg, timing->VActive);
	XV_tpg_Set_width(&tpg, timing->HActive);
	XV_tpg_Set_colorFormat(&tpg, 0);
	XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_COLOR_BARS);
	XV_tpg_Set_ovrlayId(&tpg, 0);
	XV_tpg_WriteReg(tpg_Config->BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, 0x81);

	XV_gamma_lut_Set_HwReg_width(&gamma_lut, timing->HActive);
	XV_gamma_lut_Set_HwReg_height(&gamma_lut, timing->VActive);
	XV_gamma_lut_Set_HwReg_video_format(&gamma_lut, 0);

	XV_gamma_lut_EnableAutoRestart(&gamma_lut);
    XV_gamma_lut_Start(&gamma_lut);

	PixelsPerClk = tpg.Config.PixPerClk;

	vtc_timing.HActiveVideo  = timing->HActive/PixelsPerClk;
	vtc_timing.HFrontPorch   = timing->HFrontPorch/PixelsPerClk;
	vtc_timing.HSyncWidth    = timing->HSyncWidth/PixelsPerClk;
	vtc_timing.HBackPorch    = timing->HBackPorch/PixelsPerClk;
	vtc_timing.HSyncPolarity = timing->HSyncPolarity;
	vtc_timing.VActiveVideo  = timing->VActive;
	vtc_timing.V0FrontPorch  = timing->F0PVFrontPorch;
	vtc_timing.V0SyncWidth   = timing->F0PVSyncWidth;
	vtc_timing.V0BackPorch   = timing->F0PVBackPorch;
	vtc_timing.VSyncPolarity = timing->VSyncPolarity;
	XVtc_SetGeneratorTiming(&vtc, &vtc_timing);
	XVtc_Enable(&vtc);
	XVtc_EnableGenerator(&vtc);
	XVtc_RegUpdateEnable(&vtc);
}

/*****************************************************************************/
/**
 * @brief Configures the video clock generator for the specified video mode.
 *
 * This function programs the Video Clock Wizard IP to generate the appropriate
 * pixel clock frequency for the requested video mode. It configures the PLL
 * parameters (dividers, multipliers, and fractional values) based on the video
 * resolution and pixels-per-clock setting, then waits for PLL lock.
 *
 * @param videoMode The video mode identifier (e.g., 1080p60, 4K30, 4K60).
 *
 * @return XST_SUCCESS if clock configuration succeeds and PLL locks,
 *         XST_FAILURE if the video mode is not supported or PLL fails to lock.
 *
 * @note Supports 1080p60, 4K30, and 4K60 video modes with variable pixels-per-
 *       clock configurations (1, 2, 4, or 8 PPC).
 *
 *******************************************************************************/
int videoClockConfig(XVidC_VideoMode videoMode)
{
	u32 DIVCLK_DIVIDE = 4;
	u32 CLKFBOUT_MULT = 37;
	u32 CLKFBOUT_FRAC = 125;
	u32 CLKOUT0_DIVIDE;
	u32 CLKOUT0_FRAC;
	u32 clock_config_reg_0;
	u32 clock_config_reg_2;
	u32 timeout;
	u32 lock;
	u16 PixelsPerClk, mode_index;

	const int ClkOut_Frac[3][XVIDC_PPC_NUM_SUPPORTED] =
	{ {250, 500, 0  , 0}, //1080p
		{125, 250, 500, 0}, //4K30
		{0,   125, 250, 500}  //4K60
	};
	const int ClkOut_Div[3][XVIDC_PPC_NUM_SUPPORTED] =
	{ {6, 12, 25, 50}, //1080p
		{3, 6 , 12, 25}, //4K30
		{0, 3 , 6 , 12}  //4K60
	};

	/* Validate TPG Parameters */
	Xil_AssertNonvoid((tpg.Config.PixPerClk == XVIDC_PPC_1) ||
			(tpg.Config.PixPerClk == XVIDC_PPC_2) ||
			(tpg.Config.PixPerClk == XVIDC_PPC_4) ||
			(tpg.Config.PixPerClk == XVIDC_PPC_8));

	mode_index = ((videoMode ==  XVIDC_VM_1080_60_P) ? 0 :
			(videoMode ==  XVIDC_VM_UHD_30_P)  ? 1 :
			(videoMode ==  XVIDC_VM_UHD_60_P)  ? 2 : 3);

	if(mode_index > 2)
	{
		xil_printf("ERROR:: Video Mode %s not supported\r\n", XVidC_GetVideoModeStr(videoMode));
		return(XST_FAILURE);
	}

	//map PPC to array index
	PixelsPerClk = ((tpg.Config.PixPerClk == XVIDC_PPC_8)? 3 : tpg.Config.PixPerClk>>1);
	CLKOUT0_FRAC   =  ClkOut_Frac[mode_index][PixelsPerClk];
	CLKOUT0_DIVIDE =  ClkOut_Div[mode_index][PixelsPerClk];

	clock_config_reg_0 = (1<<26) | (CLKFBOUT_FRAC<<16) | (CLKFBOUT_MULT<<8) | DIVCLK_DIVIDE;
	clock_config_reg_2 = (1<<18) | (CLKOUT0_FRAC<<8) | CLKOUT0_DIVIDE;

	VideoClockGen_WriteReg(0x200, clock_config_reg_0);
	VideoClockGen_WriteReg(0x208, clock_config_reg_2);

	usleep(300000);

	lock = VideoClockGen_ReadReg(0x4) & 0x1;
	if(!lock) //check for lock
	{
		//Video Clock Generator not locked
		VideoClockGen_WriteReg(0x25C, 0x7);
		VideoClockGen_WriteReg(0x25C, 0x2);
		timeout = 100000;
		while(!lock)
		{
			lock = VideoClockGen_ReadReg(0x4) & 0x1;
			--timeout;
			if(!timeout)
			{
				print("ERROR:: Video Clock Generator failed lock\r\n");
				return(XST_FAILURE);
			}
		}
	}
	print("Video Clock Generator locked\r\n");

	return(XST_SUCCESS);

}

/*****************************************************************************/
/**
 * @brief Resets all video processing IP cores.
 *
 * This function performs a hardware reset of all video IP cores in the
 * pipeline by asserting the reset signal, waiting for stabilization, then
 * releasing the reset signal.
 *
 * @return None
 *
 * @note The reset is applied via GPIO and includes appropriate delays for
 *       hardware stabilization.
 *
 *******************************************************************************/
void resetIp(void)
{
	*gpio_hlsIpReset = 0; //reset IPs

	usleep(300000);

	*gpio_hlsIpReset = 1; // release reset

	usleep(300000);

}

/*****************************************************************************/
/**
 * @brief Main test application for Gamma LUT video pipeline.
 *
 * This function tests the video pipeline with Gamma LUT processing across
 * multiple video resolutions (1080p60, 4K30, 4K60) based on the hardware
 * capabilities. It initializes all IP drivers, configures the video clock
 * and IP cores for each test mode, and verifies video lock status.
 *
 * @return 0 (XST_SUCCESS) if all tests pass, XST_FAILURE if any test fails.
 *
 * @note 4K30 and 4K60 tests are conditionally executed based on the TPG's
 *       pixels-per-clock configuration capability.
 *
 *******************************************************************************/
int main()
{
	int status;
	XVidC_VideoMode TestMode;

	print("Start test\r\n");

	gpio_hlsIpReset = (u32*)XPAR_HLS_IP_RESET_BASEADDR;
	gpio_videoLockMonitor = (u32*)XPAR_VIDEO_LOCK_MONITOR_BASEADDR;

	status = driverInit();
	if(status != XST_SUCCESS) {
		return(XST_FAILURE);
	}

	resetIp();

	if(*gpio_videoLockMonitor) {
		print("ERROR:: Video should not be locked\r\n");
		return(XST_FAILURE);
	}


	TestMode = XVIDC_VM_1080_60_P;
	xil_printf("\r\nTest: %s\r\n", XVidC_GetVideoModeStr(TestMode));
	status = videoClockConfig(TestMode);
	if(status != XST_SUCCESS) {
		return(XST_FAILURE);
	}
	videoIpConfig(TestMode);

	usleep(300000);

	if(!(*gpio_videoLockMonitor)) {
		print("ERROR:: Video Lock failed for 1080P60\r\n");
		return(XST_FAILURE);
	}
	else {
		print("1080P60 passed\r\n");
	}

	if((tpg.Config.PixPerClk == XVIDC_PPC_4) ||
			(tpg.Config.PixPerClk == XVIDC_PPC_8)) {

		resetIp();

		TestMode = XVIDC_VM_UHD_30_P;
		xil_printf("\r\nTest: %s\r\n", XVidC_GetVideoModeStr(TestMode));
		status = videoClockConfig(TestMode);
		if(status != XST_SUCCESS){
			return(XST_FAILURE);
		}
		videoIpConfig(TestMode);

		usleep(300000);

		if(!(*gpio_videoLockMonitor)) {
			print("ERROR:: Video Lock failed for 4KP30\r\n");
			return(XST_FAILURE);
		}
		else {
			print("4KP30 passed\r\n\r\n");
		}
	}
	/* Run 4k60 Test if supported by HW
	 * Check if TPG is configured for 2/4/8 Pixels/Clock
	 * Required to support 4K60
	 */
	if((tpg.Config.PixPerClk == XVIDC_PPC_2) ||
			(tpg.Config.PixPerClk == XVIDC_PPC_4) ||
			(tpg.Config.PixPerClk == XVIDC_PPC_8)) {

		resetIp();

		TestMode = XVIDC_VM_UHD_60_P;
		xil_printf("\r\nTest: %s\r\n", XVidC_GetVideoModeStr(TestMode));
		status = videoClockConfig(TestMode);
		if(status != XST_SUCCESS) {
			return(XST_FAILURE);
		}
		videoIpConfig(TestMode);

		usleep(300000);

		if(!(*gpio_videoLockMonitor)) {
			print("ERROR:: Video Lock failed for 4KP60\r\n");
			return(XST_FAILURE);
		}
		else {
			print("4KP60 passed\r\n\r\n");
		}
	}
	print("Test Completed Successfully\r\n");

	return 0;
}
