/******************************************************************************
 *
 * (c) Copyright 2014 - 2015 Xilinx, Inc. All rights reserved.
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
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
******************************************************************************/
/*****************************************************************************/
/**
*
* @file periph.c
*
* This is top level resource file that will initialize all system level
* peripherals
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rc   07/07/14   First release
* 2.00  dmc  12/02/15   Removed UART driver instance
*            01/25/16   Remove inclusion xdebug.h and use of xdbg_printf()
*            01/25/16   Support a new GPIO instance to reset IP inside the VPSS
* </pre>
*
******************************************************************************/
#include "xparameters.h"
#include "periph.h"

/************************** Constant Definitions *****************************/

/** @name Reset Network
 *
 * @{
 * The following constants define various reset lines in the subsystem
 */
#define XPER_HLSIP_RESET       (0x00)
#define XPER_HLSIP_ENABLE      (0x01)
/*@}*/

/**************************** Type Definitions *******************************/

/**************************** Local Global *******************************/
/* Peripheral IP driver Instance */
XV_tpg Tpg;
XVtc Vtc;
XGpio VidLocMonitor;
XGpio HlsIpReset;

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Function Definition ******************************/

/*****************************************************************************/
/**
 * This function reports system wide common peripherals included in the design
 *
 * @param  InstancePtr is a pointer to the Subsystem instance to be
 *       worked on.
 *
 *****************************************************************************/
void XPeriph_ReportDeviceInfo(XPeriph *InstancePtr)
{
  u32 numInstances;

  xil_printf("\r\n  ->System Peripherals Included\r\n");

#if defined XPAR_XUARTLITE_NUM_INSTANCES
  numInstances = XPAR_XUARTLITE_NUM_INSTANCES;
  if(numInstances > 0)
  {
    xil_printf("    : %d UART-Lite core\r\n", numInstances);
  }
#endif

  numInstances = XPAR_XV_TPG_NUM_INSTANCES;
  if(numInstances > 0)
  {
     xil_printf("    : %d TPG\r\n", numInstances);
  }

  numInstances = XPAR_XVTC_NUM_INSTANCES;
  if(numInstances > 0)
  {
    xil_printf("    : %d VTC\r\n", numInstances);
  }

  numInstances = XPAR_XGPIO_NUM_INSTANCES;
  if(numInstances > 0)
  {
     xil_printf("    : %d GPIO\r\n", numInstances);
  }
}

/*****************************************************************************/
/**
 * This function initializes system wide common peripherals.
 *
 * @param  InstancePtr is a pointer to the Subsystem instance to be
 *       worked on.
 * @return XST_SUCCESS
 *
 *****************************************************************************/
int XPeriph_PowerOnInit(XPeriph *InstancePtr)
{
  int status = XST_FAILURE;
  XVtc_Config *VtcConfigPtr;
  XGpio_Config *GpioCfgPtr;

  Xil_AssertNonvoid(InstancePtr != NULL);

  //Bind the peripheral instance to ip drivers
  InstancePtr->TpgPtr   = &Tpg;
  InstancePtr->VtcPtr   = &Vtc;
  InstancePtr->VidLockMonitorPtr = &VidLocMonitor;

  //TPG
  status = XV_tpg_Initialize(InstancePtr->TpgPtr, XPAR_V_TPG_0_DEVICE_ID);
  if(status == XST_DEVICE_NOT_FOUND)
  {
    xil_printf("ERR:: TPG device not found\r\n");
    return(status);
  }

  //VTC
  VtcConfigPtr = XVtc_LookupConfig(XPAR_V_TC_0_DEVICE_ID);
  if(VtcConfigPtr == NULL)
  {
	xil_printf("ERR:: VTC device not found\r\n");
    return(XST_DEVICE_NOT_FOUND);
  }
  status = XVtc_CfgInitialize(InstancePtr->VtcPtr,
		                      VtcConfigPtr,
		                      VtcConfigPtr->BaseAddress);
  if(status != XST_SUCCESS)
  {
	  xil_printf("ERR:: VTC Initialization failed %d\r\n", status);
	  return(XST_FAILURE);
  }


  //Peripheral GPIOs
  //  Video Lock Monitor
  GpioCfgPtr = XGpio_LookupConfig(XPAR_VIDEO_LOCK_MONITOR_DEVICE_ID);
  if(GpioCfgPtr == NULL)
  {
	xil_printf("ERR:: Video Lock Monitor GPIO device not found\r\n");
    return(XST_DEVICE_NOT_FOUND);
  }
  status = XGpio_CfgInitialize(InstancePtr->VidLockMonitorPtr,
		                       GpioCfgPtr,
		                       GpioCfgPtr->BaseAddress);
  if(status != XST_SUCCESS)
  {
	  xil_printf("ERR:: Video Lock Monitor GPIO Initialization failed %d\r\n", status);
	  return(XST_FAILURE);
  }

// HLS IP Reset - done only in the single-IP VPSS cases
#ifdef XPAR_HLS_IP_RESET_DEVICE_ID
  GpioCfgPtr = XGpio_LookupConfig(XPAR_HLS_IP_RESET_DEVICE_ID);
  if(GpioCfgPtr == NULL)
  {
	xil_printf("ERR:: HLS IP Reset GPIO device not found\r\n");
    return(XST_DEVICE_NOT_FOUND);
  }

  InstancePtr->HlsIpResetPtr = &HlsIpReset;
  status = XGpio_CfgInitialize(InstancePtr->HlsIpResetPtr,
		                       GpioCfgPtr,
		                       GpioCfgPtr->BaseAddress);
  if(status != XST_SUCCESS)
  {
	  xil_printf("ERR:: HLS IP Reset GPIO Initialization failed %d\r\n", status);
	  return(XST_FAILURE);
  }

  /* Pulse resets and then leave them enabled */
  XPeriph_ResetHlsIp(InstancePtr);
#endif

  return(status);
}

/*****************************************************************************/
/**
 * This function resets the Hls IP block(s)
 *
 * @param  InstancePtr is a pointer to the peripheral instance
 *
 *****************************************************************************/
void XPeriph_ResetHlsIp(XPeriph *InstancePtr)
{
#ifdef XPAR_HLS_IP_RESET_DEVICE_ID
  XGpio_DiscreteWrite(InstancePtr->HlsIpResetPtr, XPER_GPIO_CHANNEL_1, XPER_HLSIP_RESET);
  usleep(10000);                                       //hold reset line
  XGpio_DiscreteWrite(InstancePtr->HlsIpResetPtr, XPER_GPIO_CHANNEL_1, XPER_HLSIP_ENABLE);
  usleep(10000);                                       //allow time for start
#endif
}

/*****************************************************************************/
/**
 * This function configures TPG to user defined parameters
 *
 * @param  InstancePtr is a pointer to the peripheral instance
 *
 *****************************************************************************/
void XPeriph_ConfigTpg(XPeriph *InstancePtr)
{
  XV_tpg *pTpg = InstancePtr->TpgPtr;

  //Stop TPG
  XV_tpg_DisableAutoRestart(pTpg);

  XV_tpg_Set_height(pTpg, InstancePtr->TpgConfig.Height);
  XV_tpg_Set_width(pTpg,  InstancePtr->TpgConfig.Width);
  XV_tpg_Set_colorFormat(pTpg, InstancePtr->TpgConfig.ColorFmt);
  XV_tpg_Set_bckgndId(pTpg, InstancePtr->TpgConfig.Pattern);
  XV_tpg_Set_ovrlayId(pTpg, 0);

  //Start TPG
  XV_tpg_EnableAutoRestart(pTpg);
  XV_tpg_Start(pTpg);
}

/*****************************************************************************/
/**
 * This function programs TPG to user defined resolution
 *
 * @param  InstancePtr is a pointer to the peripheral instance
 * @param  width is the new active width
 * @param  height is the new active height
 *****************************************************************************/
void XPeriph_SetTpgParams(XPeriph *InstancePtr,
		                  u16 width,
		                  u16 height,
			              XVidC_ColorFormat Cformat,
			              u16 Pattern,
			              u16 IsInterlaced)
{
  XPeriph_SetTPGWidth(InstancePtr,  width);
  XPeriph_SetTPGHeight(InstancePtr, height);
  XPeriph_SetTPGColorFormat(InstancePtr, Cformat);
  XPeriph_SetTPGPattern(InstancePtr, Pattern);
  XPeriph_SetTPGInterlacedMode(InstancePtr, IsInterlaced);
}

/*****************************************************************************/
/**
 * This function stops TPG IP
 *
 * @param  InstancePtr is a pointer to the peripheral instance
 *
 *****************************************************************************/
void XPeriph_DisableTpg(XPeriph *InstancePtr)
{
  //Stop TPG
  XV_tpg_DisableAutoRestart(InstancePtr->TpgPtr);
}

/*****************************************************************************/
/**
 * This function reports TPG Status
 *
 * @param  InstancePtr is a pointer to the peripheral instance
 *
 *****************************************************************************/
void XPeriph_TpgDbgReportStatus(XPeriph *InstancePtr)
{
  u32 done, idle, ready, ctrl;
  u32 width, height, bgid, cfmt;
  XV_tpg *pTpg = InstancePtr->TpgPtr;

  if(pTpg)
  {
    xil_printf("\r\n\r\n----->TPG STATUS<----\r\n");

	done  = XV_tpg_IsDone(pTpg);
	idle  = XV_tpg_IsIdle(pTpg);
	ready = XV_tpg_IsReady(pTpg);
	ctrl  = XV_tpg_ReadReg(pTpg->Config.BaseAddress, XV_TPG_CTRL_ADDR_AP_CTRL);

	width  = XV_tpg_Get_width(pTpg);
	height = XV_tpg_Get_height(pTpg);
	bgid   = XV_tpg_Get_bckgndId(pTpg);
	cfmt   = XV_tpg_Get_colorFormat(pTpg);

    xil_printf("IsDone:  %d\r\n", done);
    xil_printf("IsIdle:  %d\r\n", idle);
    xil_printf("IsReady: %d\r\n", ready);
    xil_printf("Ctrl:    0x%x\r\n\r\n", ctrl);

    xil_printf("Width:        %d\r\n",width);
    xil_printf("Height:       %d\r\n",height);
	xil_printf("Backgnd Id:   %d\r\n",bgid);
	xil_printf("Color Format: %d\r\n",cfmt);
  }
}

/*****************************************************************************/
/**
 * This function configures VTC to output parameters
 *
 * @param  InstancePtr is a pointer to the peripheral instance
 * @param  StreamPtr is a pointer output stream
 *
 *****************************************************************************/
void XPeriph_ConfigVtc(XPeriph *InstancePtr,
		               XVidC_VideoStream *StreamPtr,
		               u32 PixPerClk)
{
  XVtc_Polarity Polarity;
  XVtc_SourceSelect SourceSelect;
  XVtc_Timing VideoTiming;

  /* Disable Generator */
  XVtc_Reset(InstancePtr->VtcPtr);
  XVtc_DisableGenerator(InstancePtr->VtcPtr);
  XVtc_Disable(InstancePtr->VtcPtr);

  /* Set up source select
     1 = Generator registers, 0 = Detector registers
  **/
  memset((void *)&SourceSelect, 1, sizeof(SourceSelect));

  /* 1 = Generator registers, 0 = Detector registers */
  SourceSelect.VChromaSrc = 1;
  SourceSelect.VActiveSrc = 1;
  SourceSelect.VBackPorchSrc = 1;
  SourceSelect.VSyncSrc = 1;
  SourceSelect.VFrontPorchSrc = 1;
  SourceSelect.VTotalSrc = 1;
  SourceSelect.HActiveSrc = 1;
  SourceSelect.HBackPorchSrc = 1;
  SourceSelect.HSyncSrc = 1;
  SourceSelect.HFrontPorchSrc = 1;
  SourceSelect.HTotalSrc = 1;

//  XVtc_SetSource(InstancePtr->VtcPtr, &SourceSelect);
  // Note: Can not use SetSource function call because the XVtc_SourceSelect struct
  // does not have the interlace field. Workaround is to write it manually.
  XVtc_WriteReg(InstancePtr->VtcPtr->Config.BaseAddress, (XVTC_CTL_OFFSET), 0x07FFFF00);


  VideoTiming.HActiveVideo  = StreamPtr->Timing.HActive;
  VideoTiming.HFrontPorch   = StreamPtr->Timing.HFrontPorch;
  VideoTiming.HSyncWidth    = StreamPtr->Timing.HSyncWidth;
  VideoTiming.HBackPorch    = StreamPtr->Timing.HBackPorch;
  VideoTiming.HSyncPolarity = StreamPtr->Timing.HSyncPolarity;

  /* Vertical Timing */
  VideoTiming.VActiveVideo = StreamPtr->Timing.VActive;

  // The VTC has an offset issue.
  // This results into a wrong front porch and back porch value.
  // As a workaround the front porch and back porch need to be adjusted.
  VideoTiming.V0FrontPorch  = StreamPtr->Timing.F0PVFrontPorch;
  VideoTiming.V0BackPorch   = StreamPtr->Timing.F0PVBackPorch;
  VideoTiming.V0SyncWidth   = StreamPtr->Timing.F0PVSyncWidth;
  VideoTiming.V1FrontPorch  = StreamPtr->Timing.F1VFrontPorch;
  VideoTiming.V1SyncWidth   = StreamPtr->Timing.F1VSyncWidth;
  VideoTiming.V1BackPorch   = StreamPtr->Timing.F1VBackPorch;
  VideoTiming.VSyncPolarity = StreamPtr->Timing.VSyncPolarity;
  VideoTiming.Interlaced    = FALSE;

  // adjust Horizontal counts using PixPerClk
  VideoTiming.HActiveVideo  = VideoTiming.HActiveVideo/PixPerClk;
  VideoTiming.HFrontPorch   = VideoTiming.HFrontPorch/PixPerClk;
  VideoTiming.HBackPorch    = VideoTiming.HBackPorch/PixPerClk;
  VideoTiming.HSyncWidth    = VideoTiming.HSyncWidth/PixPerClk;

  XVtc_SetGeneratorTiming(InstancePtr->VtcPtr, &VideoTiming);

  /* Set up Polarity of all outputs */
  memset((void *)&Polarity, 0, sizeof(XVtc_Polarity));
  Polarity.ActiveChromaPol = 1;
  Polarity.ActiveVideoPol = 1;
//  Polarity.FieldIdPol = ((VideoTiming.Interlaced) ? 0 : 1);
  // Note: With this fix the polarity of field-id does not have to be switched.
  // I have found that the DELL monitor I am using does not care
  // about polarity for interlaced video. As a result setting polarity HIGH or
  // LOW for interlaced video does not make a difference.
  Polarity.FieldIdPol = 0;

  Polarity.VBlankPol = VideoTiming.VSyncPolarity;
  Polarity.VSyncPol  = VideoTiming.VSyncPolarity;
  Polarity.HBlankPol = VideoTiming.HSyncPolarity;
  Polarity.HSyncPol  = VideoTiming.HSyncPolarity;

  XVtc_SetPolarity(InstancePtr->VtcPtr, &Polarity);

  /* Enable generator module */
  XVtc_Enable(InstancePtr->VtcPtr);
  XVtc_EnableGenerator(InstancePtr->VtcPtr);
  XVtc_RegUpdateEnable(InstancePtr->VtcPtr);
}
