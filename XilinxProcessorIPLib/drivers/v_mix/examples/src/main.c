/*******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file main.c
*
* This file demonstrates the example usage of TPG IP available in catalogue
* Please refer v_tpg example design guide for details on HW setup
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   11/24/15   Initial Release

* </pre>
*
******************************************************************************/

#include "xparameters.h"
#include "platform.h"
#include "microblaze_sleep.h"
#include "xv_tpg.h"
#include "xv_mix_l2.h"
#include "xvtc.h"
#include "xintc.h"

#define NUM_TEST_MODES    (2)

/* Memory Layers for Mixer */
#define XVMIX_LAYER1_BASEADDR      (XPAR_MIG7SERIES_0_BASEADDR + (0x40000000))
#define XVMIX_LAYER2_BASEADDR      (XPAR_MIG7SERIES_0_BASEADDR + (0x41000000))
#define XVMIX_LAYER3_BASEADDR      (XPAR_MIG7SERIES_0_BASEADDR + (0x42000000))
#define XVMIX_LAYER4_BASEADDR      (XPAR_MIG7SERIES_0_BASEADDR + (0x43000000))
#define XVMIX_LAYER5_BASEADDR      (XPAR_MIG7SERIES_0_BASEADDR + (0x44000000))
#define XVMIX_LAYER6_BASEADDR      (XPAR_MIG7SERIES_0_BASEADDR + (0x45000000))
#define XVMIX_LAYER7_BASEADDR      (XPAR_MIG7SERIES_0_BASEADDR + (0x46000000))

XV_tpg	   tpg;
XV_Mix_l2  mix;
XVtc       vtc;
XIntc      intc;

XVidC_VideoStream VidStream;

u32 volatile *gpio_hlsIpReset;
u32 volatile *gpio_videoLockMonitor;

static int SetupInterrupts(void);
static void ConfigTpg(XVidC_VideoStream *StreamPtr);
static void ConfigMixer(XVidC_VideoStream *StreamPtr);
static void ConfigVtc(XVidC_VideoStream *StreamPtr);
static int RunMixerFeatureTests(XVidC_VideoStream *StreamPtr);

/*****************************************************************************/
/**
 * This function initializes and configures the system interrupt controller
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
static int SetupInterrupts(void)
{
  int Status;
  XIntc *IntcPtr = &intc;

  /* Initialize the Interrupt controller */
  Status = XIntc_Initialize(IntcPtr, XPAR_MICROBLAZE_SS_AXI_INTC_0_DEVICE_ID);
  if(Status != XST_SUCCESS) {
    xil_printf("ERR:: Interrupt controller device not found\r\n");
    return(XST_FAILURE);
  }

  /* Hook up interrupt service routine */
  Status = XIntc_Connect(IntcPtr,
		                 XPAR_MICROBLAZE_SS_AXI_INTC_0_V_MIX_0_INTERRUPT_INTR,
						 (XInterruptHandler)XVMix_InterruptHandler, &mix);
  if (Status != XST_SUCCESS) {
    xil_printf("ERR:: Mixer interrupt connect failed!\r\n");
    return XST_FAILURE;
  }

  /* Enable the interrupt vector at the interrupt controller */
  XIntc_Enable(IntcPtr, XPAR_MICROBLAZE_SS_AXI_INTC_0_V_MIX_0_INTERRUPT_INTR);

  /*
   * Start the interrupt controller such that interrupts are recognized
   * and handled by the processor
   */
  Status = XIntc_Start(IntcPtr, XIN_REAL_MODE);
  if (Status != XST_SUCCESS) {
    xil_printf("ERR:: Failed to start interrupt controller\r\n");
    return XST_FAILURE;
  }

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
 * This function initializes system wide peripherals.
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
int driverInit(void)
{
  int Status;
  XVtc_Config *vtc_Config;

  vtc_Config = XVtc_LookupConfig(XPAR_V_TC_0_DEVICE_ID);
  if(vtc_Config == NULL) {
	xil_printf("ERR:: VTC device not found\r\n");
	return(XST_FAILURE);
  }

  Status = XVtc_CfgInitialize(&vtc, vtc_Config, vtc_Config->BaseAddress);
  if(Status != XST_SUCCESS) {
	xil_printf("ERR:: VTC Initialization failed %d\r\n", Status);
	return(XST_FAILURE);
  }

  Status = XV_tpg_Initialize(&tpg, XPAR_V_TPG_0_DEVICE_ID);
  if(Status != XST_SUCCESS) {
    xil_printf("ERR:: TPG device not found\r\n");
    return(XST_FAILURE);
  }

  Status  = XVMix_Initialize(&mix, XPAR_V_MIX_0_DEVICE_ID);
  if(Status != XST_SUCCESS) {
    xil_printf("ERR:: Mixer device not found\r\n");
    return(XST_FAILURE);
  }

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
 * This function configures TPG for defined mode
 *
 * @return none
 *
 *****************************************************************************/
static void ConfigTpg(XVidC_VideoStream *StreamPtr)
{
  //Stop TPG
  XV_tpg_DisableAutoRestart(&tpg);

  XV_tpg_Set_height(&tpg, StreamPtr->Timing.VActive);
  XV_tpg_Set_width(&tpg, StreamPtr->Timing.HActive);
  XV_tpg_Set_colorFormat(&tpg, StreamPtr->ColorFormatId);
  XV_tpg_Set_bckgndId(&tpg, XTPG_BKGND_COLOR_BARS);
  XV_tpg_Set_ovrlayId(&tpg, 0);

  //Start TPG
  XV_tpg_EnableAutoRestart(&tpg);
  XV_tpg_Start(&tpg);
  xil_printf("INFO: TPG configured\r\n");
}

/*****************************************************************************/
/**
 * This function configures vtc for defined mode
 *
 * @return none
 *
 *****************************************************************************/
static void ConfigVtc(XVidC_VideoStream *StreamPtr)
{
  XVtc_Timing vtc_timing = {0};
  u16 PixelsPerClock = StreamPtr->PixPerClk;

  vtc_timing.HActiveVideo  = StreamPtr->Timing.HActive/PixelsPerClock;
  vtc_timing.HFrontPorch   = StreamPtr->Timing.HFrontPorch/PixelsPerClock;
  vtc_timing.HSyncWidth    = StreamPtr->Timing.HSyncWidth/PixelsPerClock;
  vtc_timing.HBackPorch    = StreamPtr->Timing.HBackPorch/PixelsPerClock;
  vtc_timing.HSyncPolarity = StreamPtr->Timing.HSyncPolarity;
  vtc_timing.VActiveVideo  = StreamPtr->Timing.VActive;
  vtc_timing.V0FrontPorch  = StreamPtr->Timing.F0PVFrontPorch;
  vtc_timing.V0SyncWidth   = StreamPtr->Timing.F0PVSyncWidth;
  vtc_timing.V0BackPorch   = StreamPtr->Timing.F0PVBackPorch;
  vtc_timing.VSyncPolarity = StreamPtr->Timing.VSyncPolarity;
  XVtc_SetGeneratorTiming(&vtc, &vtc_timing);
  XVtc_Enable(&vtc);
  XVtc_EnableGenerator(&vtc);
  XVtc_RegUpdateEnable(&vtc);
  xil_printf("INFO: VTC configured\r\n");
}

/*****************************************************************************/
/**
 * This function configures Mixer for defined mode
 *
 * @return none
 *
 *****************************************************************************/
static void ConfigMixer(XVidC_VideoStream *StreamPtr)
{
  XV_Mix_l2 *MixerPtr = &mix;

  /* Setup default config config after reset */

  /* Set Memory Layer Addresses */
  XVMix_SetLayerBufferAddr(MixerPtr, XVMIX_LAYER_1, XVMIX_LAYER1_BASEADDR);
  XVMix_SetLayerBufferAddr(MixerPtr, XVMIX_LAYER_2, XVMIX_LAYER2_BASEADDR);
  XVMix_SetLayerBufferAddr(MixerPtr, XVMIX_LAYER_3, XVMIX_LAYER3_BASEADDR);
  XVMix_SetLayerBufferAddr(MixerPtr, XVMIX_LAYER_4, XVMIX_LAYER4_BASEADDR);
  XVMix_SetLayerBufferAddr(MixerPtr, XVMIX_LAYER_5, XVMIX_LAYER5_BASEADDR);
  XVMix_SetLayerBufferAddr(MixerPtr, XVMIX_LAYER_6, XVMIX_LAYER6_BASEADDR);
  XVMix_SetLayerBufferAddr(MixerPtr, XVMIX_LAYER_7, XVMIX_LAYER7_BASEADDR);

  XVMix_SetBackgndColor(MixerPtr, XVMIX_BKGND_BLUE, StreamPtr->ColorDepth);

  XVMix_Stop(MixerPtr);
  XVMix_LayerDisable(MixerPtr, XVMIX_LAYER_STREAM);
  XVMix_SetVidStreamIn(MixerPtr, StreamPtr);
  XVMix_SetVidStreamOut(MixerPtr, StreamPtr);
  XVMix_SetLayerColorFormat(MixerPtr,
		                    XVMIX_LAYER_STREAM,
		                    StreamPtr->ColorFormatId);
  XVMix_SetResolution(MixerPtr,
		              StreamPtr->Timing.HActive,
		              StreamPtr->Timing.VActive);
  XVMix_LayerEnable(MixerPtr, XVMIX_LAYER_STREAM);
  XVMix_Start(MixerPtr);
  xil_printf("INFO: Mixer configured\r\n");
}

/*****************************************************************************/
/**
 * This function runs defined tests on Mixer core
 *
 * @return none
 *
 *****************************************************************************/
static int RunMixerFeatureTests(XVidC_VideoStream *StreamPtr)
{
  int layerIndex, Status;
  int ErrorCount = 0;
  XVidC_VideoWindow Win;
  u32 baseaddr;
  XV_Mix_l2 *MixerPtr = &mix;

  xil_printf("\r\n****Running Mixer Feature Tests****\r\n");
  /* Test 1: Layer 0 En/Dis
      - Disable layer 0
      - Check video lock
      - Enable layer 0
      - Check video lock
  */
  xil_printf("Disable Stream Layer: ");
  Status = XVMix_LayerDisable(MixerPtr, XVMIX_LAYER_STREAM);
  if(Status == XST_SUCCESS) {
     if(!(*gpio_videoLockMonitor)) {
         xil_printf("ERR:: Vidout not locked\r\n");
         ++ErrorCount;
     } else {
         xil_printf("Done\r\n");
     }
  } else {
         xil_printf("ERR:: Unable to disable stream layer\r\n");
         ++ErrorCount;
  }

  xil_printf("Enable  Stream Layer: ");
  Status = XVMix_LayerEnable(MixerPtr, XVMIX_LAYER_STREAM);
  if(Status == XST_SUCCESS) {
     if(!(*gpio_videoLockMonitor)) {
         xil_printf("ERR:: Vidout not locked\r\n");
         ++ErrorCount;
     } else {
         xil_printf("Done\r\n");
     }
  } else {
         xil_printf("ERR:: Unable to enable stream layer\r\n");
         ++ErrorCount;
  }

   /* Test 2: Memory layer En
      - Set layer window
      - Set layer Alpha, if available
      - Enable layer
      - Check video lock
      - Disable layer
      - Check video lock
  */
  for(layerIndex=XVMIX_LAYER_1; layerIndex<MixerPtr->Mix.Config.NumLayers; ++layerIndex) {
    xil_printf("\r\n--> Test Memory Layer %d <--\r\n", layerIndex);
    baseaddr = XVMIX_LAYER1_BASEADDR + ((layerIndex-1)*0x01000000);

    xil_printf("   Set Layer Buffer Addr: 0x%x\r\n", baseaddr);
    XVMix_SetLayerBufferAddr(MixerPtr,
					 layerIndex,
					 baseaddr);

    xil_printf("   Set Layer Window: ");
	Win.StartX = 10;
	Win.StartY = 10;
	Win.Width  = 256;
	Win.Height = 256;

	Win.StartX += layerIndex*50; //offset each layer by 50 pixels in horiz. dir
	Win.StartY += layerIndex*50; //offset each layer by 50 pixels in vert. dir
	Status = XVMix_SetLayerWindow(MixerPtr, layerIndex, &Win);
    if(Status != XST_SUCCESS) {
        xil_printf("ERR:: Unable to set window \r\n");
        ++ErrorCount;
    } else {
        xil_printf("Done\r\n");
    }

    xil_printf("   Set Layer Alpha to %d: ", XVMIX_ALPHA_MAX);
    if(XVMix_IsAlphaEnabled(MixerPtr, layerIndex)) {
	  Status = XVMix_SetLayerAlpha(MixerPtr, layerIndex, XVMIX_ALPHA_MAX);
      if(Status != XST_SUCCESS) {
        xil_printf("ERR:: Unable to set Alpha \r\n");
        ++ErrorCount;
      }
      else {
          xil_printf("Done\r\n");
      }
    } else {
        xil_printf("(Alpha Disabled in HW)\r\n");
    }

    xil_printf("   Enable Layer: ");
    Status = XVMix_LayerEnable(MixerPtr, layerIndex);
    if(Status != XST_SUCCESS) {
        xil_printf("ERR:: Failed to enable layer \r\n");
        ++ErrorCount;
    } else {
        xil_printf("Done\r\n");
    }

    //Check for vidout lock
    xil_printf("   Check Vidout State: ");
    if(!(*gpio_videoLockMonitor)) {
       xil_printf("ERR:: Not locked\r\n");
       ++ErrorCount;
    } else {
        xil_printf("Locked\r\n");
    }
  }

  /* Test 3: Memory layer Disable
      - Disable memory
      - Check for lock
  */
  xil_printf("\r\n");
  for(layerIndex=XVMIX_LAYER_1; layerIndex<MixerPtr->Mix.Config.NumLayers; ++layerIndex) {
    xil_printf("Disable Layer %d: ", layerIndex);
    XVMix_LayerDisable(MixerPtr, layerIndex);

    //Check for vidout lock
    if(!(*gpio_videoLockMonitor)) {
      xil_printf("ERR:: Vidout not locked\r\n");
      ++ErrorCount;
    } else {
      xil_printf("Done\r\n");
    }
  }

  return(ErrorCount);
}


/*****************************************************************************/
/**
 * This function toggles HW reset line for all IP's
 *
 * @return None
 *
 *****************************************************************************/
void resetIp(void)
{
  *gpio_hlsIpReset = 0; //reset IPs
  MB_Sleep(100);        //hold reset line
  *gpio_hlsIpReset = 1; //release reset
  MB_Sleep(200);        //wait
}

/***************************************************************************
*  This is the main loop of the application
***************************************************************************/
int main(void)
{
  int Status, index;
  int FailCount = 0;
  XVidC_VideoTiming const *TimingPtr;
  XVidC_VideoMode TestModes[NUM_TEST_MODES] =
  { XVIDC_VM_1080_60_P,
    XVIDC_VM_720_60_P
    //XVIDC_VM_UHD_30_P
  };

  init_platform();

  xil_printf("Start Mixer Example Design Test\r\n");

  /* Setup Reset line and video lock monitor */
  gpio_hlsIpReset = (u32*)XPAR_IP_RESET_HLS_BASEADDR;
  gpio_videoLockMonitor = (u32*)XPAR_VIDEO_LOCK_MONITOR_BASEADDR;

  //Release reset line
  *gpio_hlsIpReset = 1;

  /* Initialize IRQ */
  Status = SetupInterrupts();
  if (Status == XST_FAILURE) {
    return XST_FAILURE;
  }

  Status = driverInit();
  if(Status != XST_SUCCESS) {
	return(XST_FAILURE);
  }

  /* Enable exceptions. */
  Xil_ExceptionEnable();

  /* Setup a default stream */
  VidStream.PixPerClk     = tpg.Config.PixPerClk;
  VidStream.ColorFormatId = XVIDC_CSF_RGB;
  VidStream.ColorDepth    = XVIDC_BPC_8;
  VidStream.FrameRate     = XVIDC_FR_60HZ;

  resetIp();

  /* sanity check */
  if(*gpio_videoLockMonitor) {
		print("ERR:: Video should not be locked\r\n");
		return(XST_FAILURE);
  }

  for(index=0; index<NUM_TEST_MODES; ++index)
  {
    // Get mode to test
    VidStream.VmId = TestModes[index];

    // Get mode timing parameters
    TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
    VidStream.Timing = *TimingPtr;
    VidStream.FrameRate = XVidC_GetFrameRate(VidStream.VmId);

    xil_printf("\r\n*************************************\r\n");
    xil_printf("Test Input Stream: %s\r\n", XVidC_GetVideoModeStr(VidStream.VmId));
    xil_printf("*************************************\r\n");
    ConfigVtc(&VidStream);
    ConfigMixer(&VidStream);
    ConfigTpg(&VidStream);
    xil_printf("Wait for vid out lock: ");
    MB_Sleep(2000);

    if(!(*gpio_videoLockMonitor)) {
		xil_printf("Failed\r\n");
		++FailCount;
    } else {
	xil_printf("Locked\r\n");
        Status = RunMixerFeatureTests(&VidStream);
        if(Status != 0) { //problems encountered in feature test
		++FailCount;
        }
    }

    resetIp();
  }

  if(FailCount) {
    xil_printf("\r\n\r\nINFO: Test completed. %d/%d tests failed\r\n", FailCount, NUM_TEST_MODES);
  } else {
    xil_printf("\r\n\r\nINFO: Test completed successfully\r\n");
  }

  while(1);
}
