/*******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file main.c
* @addtogroup v_tpg Overview
*
* This file demonstrates the example usage of TPG IP available in catalogue
* Please refer v_tpg example design guide for details on HW setup
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vyc   09/11/15   Initial Release
* 1.10  rco   10/05/15   Update to support multiple PPC configurations
* 8.0   ms    01/23/17   Modified xil_printf statement in main function to
*                        ensure that "Successfully ran" and "Failed" strings
*                        are available in all examples. This is a fix for
*                        CR-965028.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "sleep.h"
#include "xv_tpg.h"
#include "xvtc.h"
#include "xvidc.h"


/***************** Macros (Inline Functions) Definitions *********************/

/**
 * Macro to write to Video Clock Generator register
 */
#define VideoClockGen_WriteReg(RegOffset, Data) \
    Xil_Out32((XPAR_VIDEO_CLK_WIZ_BASEADDR) + (RegOffset), (u32)(Data))

/**
 * Macro to read from Video Clock Generator register
 */
#define VideoClockGen_ReadReg(RegOffset) \
    Xil_In32((XPAR_VIDEO_CLK_WIZ_BASEADDR) + (RegOffset))

/************************** Variable Definitions *****************************/

XV_tpg_Config		*tpg_Config;
XV_tpg				tpg;

XV_tpg_Config		*tpg1_Config;
XV_tpg				tpg1;

XVtc				vtc;
XVtc_Config			*vtc_Config;
XVtc_Timing			vtc_timing;

u32 volatile		*gpio_hlsIpReset;
u32 volatile		*gpio_videoLockMonitor;

/*****************************************************************************/
/**
 * @brief Initialize VTC and TPG driver instances
 *
 * This function looks up the configuration for VTC and both TPG instances,
 * then initializes them with their respective base addresses.
 *
 * @return XST_SUCCESS if initialization succeeds
 *         XST_DEVICE_NOT_FOUND if device lookup fails
 *         XST_FAILURE if initialization fails
 *
 * @note This function must be called before using any VTC or TPG functionality
 *
 *******************************************************************************/
int driverInit()
{
	int status;

#ifndef SDT
	vtc_Config = XVtc_LookupConfig(XPAR_V_TC_0_DEVICE_ID);
#else
	vtc_Config = XVtc_LookupConfig(XPAR_V_TC_0_BASEADDR);
#endif
	if(vtc_Config == NULL)
	{
		xil_printf("ERR:: VTC device not found\r\n");
		return(XST_DEVICE_NOT_FOUND);
	}
	status = XVtc_CfgInitialize(&vtc, vtc_Config, vtc_Config->BaseAddress);
	if(status != XST_SUCCESS)
	{
		xil_printf("ERR:: VTC Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

#ifndef SDT
	tpg_Config = XV_tpg_LookupConfig(XPAR_V_TPG_0_DEVICE_ID);
#else
	tpg_Config = XV_tpg_LookupConfig(XPAR_V_TPG_0_BASEADDR);
#endif
	if(tpg_Config == NULL)
	{
		xil_printf("ERR:: TPG device not found\r\n");
		return(XST_DEVICE_NOT_FOUND);
	}
	status = XV_tpg_CfgInitialize(&tpg, tpg_Config, tpg_Config->BaseAddress);
	if(status != XST_SUCCESS)
	{
		xil_printf("ERR:: TPG Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

#ifndef SDT
	tpg1_Config = XV_tpg_LookupConfig(XPAR_V_TPG_1_DEVICE_ID);
#else
	tpg1_Config = XV_tpg_LookupConfig(XPAR_V_TPG_1_BASEADDR);
#endif
	if(tpg1_Config == NULL)
	{
		xil_printf("ERR:: TPG device not found\r\n");
		return(XST_DEVICE_NOT_FOUND);
	}
	status = XV_tpg_CfgInitialize(&tpg1, tpg1_Config, tpg1_Config->BaseAddress);
	if(status != XST_SUCCESS)
	{
		xil_printf("ERR:: TPG Initialization failed %d\r\n", status);
		return(XST_FAILURE);
	}

	return(XST_SUCCESS);
}

/*****************************************************************************/
/**
 * @brief Configure video IP cores for specified video mode
 *
 * This function configures both TPG instances and VTC with timing parameters
 * for the specified video mode. TPG0 generates color bars, TPG1 is configured
 * for passthrough mode, and VTC generates timing signals.
 *
 * @param  videoMode Video mode to configure (e.g., 1080p60, 4K30, 4K60)
 *
 * @return None
 *
 * @note Video clock must be configured before calling this function
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

	XV_tpg_Set_height(&tpg1, timing->VActive);
	XV_tpg_Set_width(&tpg1, timing->HActive);
	XV_tpg_Set_colorFormat(&tpg1, 0);
	XV_tpg_Set_bckgndId(&tpg1, XTPG_BKGND_COLOR_BARS);
	XV_tpg_Set_ovrlayId(&tpg1, 0);
	XV_tpg_Set_enableInput(&tpg1, 1);
	XV_tpg_Set_passthruStartX(&tpg1, 0);
	XV_tpg_Set_passthruStartY(&tpg1, 0);
	XV_tpg_Set_passthruEndX(&tpg1, timing->HActive);
	XV_tpg_Set_passthruEndY(&tpg1, timing->VActive);
	XV_tpg_WriteReg(tpg1_Config->BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, 0x81);

	PixelsPerClk = tpg1.Config.PixPerClk;

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
 * @brief Configure video clock generator for specified video mode
 *
 * This function programs the video clock wizard with appropriate PLL settings
 * to generate the required pixel clock for the specified video mode and
 * pixels-per-clock configuration.
 *
 * @param  videoMode Video mode that determines clock frequency requirements
 *
 * @return XST_SUCCESS if clock configuration and lock succeeds
 *         XST_FAILURE if video mode is unsupported or clock fails to lock
 *
 * @note Supports 1080p60, 4K30, and 4K60 modes with 1/2/4/8 pixels per clock
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
    Xil_AssertNonvoid((tpg1.Config.PixPerClk == XVIDC_PPC_1) ||
                      (tpg1.Config.PixPerClk == XVIDC_PPC_2) ||
					  (tpg1.Config.PixPerClk == XVIDC_PPC_4) ||
                      (tpg1.Config.PixPerClk == XVIDC_PPC_8));


    mode_index = ((videoMode ==  XVIDC_VM_1080_60_P) ? 0 :
                  (videoMode ==  XVIDC_VM_UHD_30_P)  ? 1 :
                  (videoMode ==  XVIDC_VM_UHD_60_P)  ? 2 : 3);

    if(mode_index > 2)
    {
      xil_printf("ERR:: Video Mode %s not supported\r\n", XVidC_GetVideoModeStr(videoMode));
      return(XST_FAILURE);
    }

    //map PPC to array index
    PixelsPerClk = ((tpg1.Config.PixPerClk == XVIDC_PPC_8)? 3 : tpg1.Config.PixPerClk>>1);
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
				xil_printf("ERR:: Video Clock Generator failed lock\r\n");
				return(XST_FAILURE);
			}
		}
	}
	xil_printf("Video Clock Generator locked\r\n");

	return(XST_SUCCESS);

}

/*****************************************************************************/
/**
 * @brief Reset video IP cores
 *
 * This function asserts and then releases reset for all HLS-based video IP
 * cores in the system using GPIO control.
 *
 * @return None
 *
 * @note Reset is held for 300ms before release, followed by 300ms settling time
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
 * @brief Main function to test TPG video pipeline
 *
 * This function tests the TPG video pipeline by sequentially running tests
 * for different video modes (1080p60, 4K30, and optionally 4K60). Each test
 * configures the clock, video IPs, and verifies video lock status.
 *
 * @return XST_SUCCESS if all tests pass
 *         XST_FAILURE if any test fails
 *
 * @note 4K60 test only runs if TPG is configured for 2/4/8 pixels per clock
 *
 *******************************************************************************/
int main()
{
	int status;
	XVidC_VideoMode TestMode;

	xil_printf("Start test\r\n");

	gpio_hlsIpReset = (u32*)XPAR_HLS_IP_RESET_BASEADDR;
	gpio_videoLockMonitor = (u32*)XPAR_VIDEO_LOCK_MONITOR_BASEADDR;

	status = driverInit();
	if(status != XST_SUCCESS) {
		return(XST_FAILURE);
	}

	resetIp();

	if(*gpio_videoLockMonitor) {
		xil_printf("ERR:: Video should not be locked\r\n");
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
		xil_printf("ERR:: Video Lock failed for 1080P60\r\n");
		return(XST_FAILURE);
	}
	else {
		xil_printf("1080P60 passed\r\n");
	}

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
		xil_printf("ERR:: Video Lock failed for 4KP30\r\n");
		return(XST_FAILURE);
	}
	else {
		xil_printf("4KP30 passed\r\n\r\n");
	}

    /* Run 4k60 Test if supported by HW
     * Check if TPG is configured for 2/4/8 Pixels/Clock
     * Required to support 4K60
     */
    if((tpg1.Config.PixPerClk == XVIDC_PPC_2) ||
       (tpg1.Config.PixPerClk == XVIDC_PPC_4) ||
       (tpg1.Config.PixPerClk == XVIDC_PPC_8)) {

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
		xil_printf("ERR:: Video Lock failed for 4KP60\r\n");
		return(XST_FAILURE);
	  }
	  else {
		xil_printf("4KP60 passed\r\n\r\n");
	  }
    }
	xil_printf("Successfully ran Example\r\n");

	return 0;
}
