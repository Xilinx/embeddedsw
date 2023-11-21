/******************************************************************************
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xparameters.h"
#ifndef SDT
#include "microblaze_sleep.h"
#else
#include "sleep.h"
#endif
#include "xv_tpg.h"
#include "xv_axi4s_remap.h"
#include "xvtc.h"
#include "xvidc.h"

XV_tpg_Config           *tpg_Config;
XV_tpg                  tpg;

XV_axi4s_remap_Config   *remap_Config;
XV_axi4s_remap          remap;

XVtc                    vtc;
XVtc_Config             *vtc_Config;
XVtc_Timing             vtc_timing;

u32 volatile            *gpio_hlsIpReset;
u32 volatile            *gpio_videoLockMonitor;

#ifdef XPAR_V_AXI4S_REMAP_0_S_AXI_CTRL_BASEADDR
#define XPAR_V_AXI4S_REMAP_0_BASEADDR XPAR_V_AXI4S_REMAP_0_S_AXI_CTRL_BASEADDR
#endif

#define VideoClockGen_WriteReg(RegOffset, Data) \
    Xil_Out32((XPAR_VIDEO_CLK_BASEADDR) + (RegOffset), (u32)(Data))
#define VideoClockGen_ReadReg(RegOffset) \
    Xil_In32((XPAR_VIDEO_CLK_BASEADDR) + (RegOffset))

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
    tpg_Config = XV_tpg_LookupConfig(XPAR_V_TPG_0_BASEADDR);
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
    remap_Config = XV_axi4s_remap_LookupConfig(XPAR_V_AXI4S_REMAP_0_DEVICE_ID);
#else
    remap_Config = XV_axi4s_remap_LookupConfig(XPAR_V_AXI4S_REMAP_0_BASEADDR);
#endif
    if(remap_Config == NULL)
    {
        xil_printf("ERROR:: AXI4S_REMAP device not found\r\n");
        return(XST_DEVICE_NOT_FOUND);
    }
    status = XV_axi4s_remap_CfgInitialize(&remap, remap_Config, remap_Config->BaseAddress);
    if(status != XST_SUCCESS)
    {
        xil_printf("ERROR:: AXI4S_REMAP Initialization failed %d\r\n", status);
        return(XST_FAILURE);
    }
    return(XST_SUCCESS);
}

void videoIpConfig(XVidC_VideoMode videoMode)
{
    XVidC_VideoTiming const *timing = XVidC_GetTimingInfo(videoMode);

    XV_axi4s_remap_Set_width(&remap, timing->HActive);
    XV_axi4s_remap_Set_height(&remap, timing->VActive);
    XV_axi4s_remap_Set_ColorFormat(&remap, 0);
    XV_axi4s_remap_Set_inPixClk(&remap, remap.Config.PixPerClkIn);
    XV_axi4s_remap_Set_outPixClk(&remap, remap.Config.PixPerClkOut);
    XV_axi4s_remap_Set_inHDMI420(&remap, 0);
    XV_axi4s_remap_Set_outHDMI420(&remap, 0);
    XV_axi4s_remap_Set_inPixDrop(&remap, 0);
    XV_axi4s_remap_Set_outPixRepeat(&remap, 0);
    xil_printf("INFO: width height %d %d\r\n", Xil_In32(XPAR_V_AXI4S_REMAP_0_BASEADDR+0x018), Xil_In32(XPAR_V_AXI4S_REMAP_0_BASEADDR+0x010));
    XV_axi4s_remap_WriteReg(remap_Config->BaseAddress, XV_AXI4S_REMAP_CTRL_ADDR_AP_CTRL, 0x81);
    xil_printf("INFO: Remapper started\r\n");

    XV_tpg_Set_height(&tpg, timing->VActive);
    XV_tpg_Set_width(&tpg, timing->HActive);
    XV_tpg_Set_colorFormat(&tpg, 0);
    XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_COLOR_BARS);
    XV_tpg_Set_ovrlayId(&tpg, 0);
    XV_tpg_WriteReg(tpg_Config->BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL, 0x81);
    xil_printf("INFO: TPG configured\r\n");

    u16 PixelsPerClk;
    PixelsPerClk = remap.Config.PixPerClkOut;
    //PixelsPerClk = (remap.Config.PixPerClkOut)*2;

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
    { {250, 500, 0  }, //1080p
      {125, 250, 500}, //4K30
      {0,   125, 250}  //4K60
    };
    const int ClkOut_Div[3][XVIDC_PPC_NUM_SUPPORTED] =
    { {6, 12, 25}, //1080p
      {3, 6 , 12}, //4K30
      {0, 3 , 6 }  //4K60
    };
/*
    const int ClkOut_Frac[3][XVIDC_PPC_NUM_SUPPORTED] =
    { {500, 0,   0  }, //1080p
      {250, 500, 0  }, //4K30
      {125, 250, 500}  //4K60
    };
    const int ClkOut_Div[3][XVIDC_PPC_NUM_SUPPORTED] =
    { {12, 25, 50}, //1080p
      {6,  12, 25}, //4K30
      {3,  6 , 12}  //4K60
    };
*/

    /* Validate IP Parameters */
    Xil_AssertNonvoid((remap.Config.PixPerClkOut == XVIDC_PPC_1) ||
                      (remap.Config.PixPerClkOut == XVIDC_PPC_2) ||
                      (remap.Config.PixPerClkOut == XVIDC_PPC_4));

    mode_index = ((videoMode ==  XVIDC_VM_1080_60_P) ? 0 :
                  (videoMode ==  XVIDC_VM_UHD_30_P)  ? 1 :
                  (videoMode ==  XVIDC_VM_UHD_60_P)  ? 2 : 3);

    if(mode_index > 2)
    {
      xil_printf("ERROR:: Video Mode %s not supported\r\n", XVidC_GetVideoModeStr(videoMode));
      return(XST_FAILURE);
    }

    //map PPC to array index
    PixelsPerClk = (remap.Config.PixPerClkOut>>1);
    CLKOUT0_FRAC   =  ClkOut_Frac[mode_index][PixelsPerClk];
    CLKOUT0_DIVIDE =  ClkOut_Div[mode_index][PixelsPerClk];

    clock_config_reg_0 = (1<<26) | (CLKFBOUT_FRAC<<16) | (CLKFBOUT_MULT<<8) | DIVCLK_DIVIDE;
    clock_config_reg_2 = (1<<18) | (CLKOUT0_FRAC<<8) | CLKOUT0_DIVIDE;

    VideoClockGen_WriteReg(0x200, clock_config_reg_0);
    VideoClockGen_WriteReg(0x208, clock_config_reg_2);

#ifndef SDT
    MB_Sleep(300);
#else
    msleep(300);
#endif

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
                xil_printf("ERROR:: Video Clock Generator failed lock\r\n");
                return(XST_FAILURE);
            }
        }
    }
    xil_printf("Video Clock Generator locked\r\n");

    return(XST_SUCCESS);

}

void resetIp(void)
{
    *gpio_hlsIpReset = 0; //reset IPs

#ifndef SDT
    MB_Sleep(300);
#else
    msleep(300);
#endif

    *gpio_hlsIpReset = 1; // release reset

#ifndef SDT
    MB_Sleep(300);
#else
    msleep(300);
#endif

}

int main()
{
    int status;
    XVidC_VideoMode TestMode;

    printf("Start test\r\n");

//    printf("hit any key\r\n");
//    inbyte();

    gpio_hlsIpReset = (u32*)XPAR_HLS_IP_RESET_BASEADDR;
    gpio_videoLockMonitor = (u32*)XPAR_VIDEO_LOCK_MONITOR_BASEADDR;

    status = driverInit();
    if(status != XST_SUCCESS) {
        return(XST_FAILURE);
    }

    resetIp();

    if(*gpio_videoLockMonitor) {
        xil_printf("ERROR:: Video should not be locked\r\n");
        return(XST_FAILURE);
    }


    TestMode = XVIDC_VM_1080_60_P;
    xil_printf("\r\nTest: %s\r\n", XVidC_GetVideoModeStr(TestMode));
    status = videoClockConfig(TestMode);
    if(status != XST_SUCCESS) {
        return(XST_FAILURE);
    }
    videoIpConfig(TestMode);

#ifndef SDT
    MB_Sleep(300);
#else
    msleep(300);
#endif

    if(!(*gpio_videoLockMonitor)) {
        xil_printf("ERROR:: Video Lock failed for 1080P60\r\n");
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

#ifndef SDT
    MB_Sleep(300);
#else
    msleep(300);
#endif

    if(!(*gpio_videoLockMonitor)) {
        xil_printf("ERROR:: Video Lock failed for 4KP30\r\n");
        return(XST_FAILURE);
    }
    else {
        xil_printf("4KP30 passed\r\n\r\n");
    }

    /* Run 4k60 Test if supported by HW
     * Check if IP is configured for 2/4 Pixels/Clock
     * Required to support 4K60
     */
    if((remap.Config.PixPerClkOut == XVIDC_PPC_2) ||
       (remap.Config.PixPerClkOut == XVIDC_PPC_4)) {

      resetIp();

      TestMode = XVIDC_VM_UHD_60_P;
      xil_printf("\r\nTest: %s\r\n", XVidC_GetVideoModeStr(TestMode));
      status = videoClockConfig(TestMode);
      if(status != XST_SUCCESS) {
        return(XST_FAILURE);
      }
      videoIpConfig(TestMode);

    #ifndef SDT
        MB_Sleep(300);
    #else
        msleep(300);
    #endif

      if(!(*gpio_videoLockMonitor)) {
        xil_printf("ERROR:: Video Lock failed for 4KP60\r\n");
        return(XST_FAILURE);
      }
      else {
        xil_printf("4KP60 passed\r\n\r\n");
      }
    }
    xil_printf("Test Completed Successfully\r\n");

    return 0;
}
