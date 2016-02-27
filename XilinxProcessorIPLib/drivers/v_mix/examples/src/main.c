/*******************************************************************************
 *
 * Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
*             02/05/16   Add Logo test
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
#define XVMIX_LAYER1_BASEADDR      (XPAR_MIG7SERIES_0_BASEADDR + (0x20000000))
#define XVMIX_LAYER_ADDR_OFFSET    (0x01000000U)

extern unsigned char Logo_R[];
extern unsigned char Logo_G[];
extern unsigned char Logo_B[];

XV_tpg	   tpg;
XV_Mix_l2  mix;
XVtc       vtc;
XIntc      intc;

XVidC_VideoStream VidStream;

u32 volatile *gpio_hlsIpReset;
u32 volatile *gpio_videoLockMonitor;

static int DriverInit(void);
static int SetupInterrupts(void);
static void ConfigTpg(XVidC_VideoStream *StreamPtr);
static void ConfigMixer(XVidC_VideoStream *StreamPtr);
static void ConfigVtc(XVidC_VideoStream *StreamPtr);
static int RunMixerFeatureTests(XVidC_VideoStream *StreamPtr);
static int CheckVidoutLock(void);

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
static int DriverInit(void)
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
  int NumLayers, index, Status;
  u32 MemAddr;

  /* Setup default config after reset */

  /* Set Memory Layer Addresses */
  NumLayers = XVMix_GetNumLayers(MixerPtr);
  MemAddr = XVMIX_LAYER1_BASEADDR;
  for(index = XVMIX_LAYER_1; index < NumLayers; ++index) {
	  Status = XVMix_SetLayerBufferAddr(MixerPtr, index, MemAddr);
	  if(Status != XST_SUCCESS) {
		  xil_printf("MIXER ERR:: Unable to set layer %d buffer addr to 0x%X\r\n",
				      index, MemAddr);
	  } else {
	      MemAddr += XVMIX_LAYER_ADDR_OFFSET;
	  }
  }

  if(XVMix_IsLogoEnabled(MixerPtr)) {
    XVidC_VideoWindow Win;

    Win.StartX = 64;
    Win.StartY = 64;
    Win.Width  = 64;
    Win.Height = 64;

    XVMix_LoadLogo(MixerPtr,
		           &Win,
	               Logo_R,
	               Logo_G,
	               Logo_B);
  }
  XVMix_SetBackgndColor(MixerPtr, XVMIX_BKGND_BLUE, StreamPtr->ColorDepth);

  XVMix_LayerDisable(MixerPtr, XVMIX_LAYER_MASTER);
  XVMix_SetVidStream(MixerPtr, StreamPtr);
  XVMix_LayerEnable(MixerPtr, XVMIX_LAYER_MASTER);
  XVMix_InterruptDisable(MixerPtr);
  XVMix_Start(MixerPtr);
  xil_printf("INFO: Mixer configured\r\n");
}

/*****************************************************************************/
/**
 * This function checks vidout lock status
 *
 * @return none
 *
 *****************************************************************************/
static int CheckVidoutLock(void)
{
  int Status = FALSE;

  if(*gpio_videoLockMonitor) {
      xil_printf("Locked\r\n");
      Status = TRUE;
  } else {
      xil_printf("<ERR:: Not Locked>\r\n\r\n");
  }
  return(Status);
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
  XVidC_ColorFormat Cfmt;
  u32 baseaddr, Stride;
  XV_Mix_l2 *MixerPtr = &mix;

  xil_printf("\r\n****Running Mixer Feature Tests****\r\n");
  /* Test 1: Master Layer Enable/Disable
      - Disable layer 0
      - Check video lock
      - Enable layer 0
      - Check video lock
  */
  xil_printf("Disable Master Layer: ");
  Status = XVMix_LayerDisable(MixerPtr, XVMIX_LAYER_MASTER);
  if(Status == XST_SUCCESS) {
	  ErrorCount += (!CheckVidoutLock() ? 1 : 0);
  } else {
      xil_printf("<ERR:: Command Failed>\r\n");
      ++ErrorCount;
  }

  xil_printf("Enable  Master Layer: ");
  Status = XVMix_LayerEnable(MixerPtr, XVMIX_LAYER_MASTER);
  if(Status == XST_SUCCESS) {
	  ErrorCount += (!CheckVidoutLock() ? 1 : 0);
  } else {
      xil_printf("<ERR:: Command Failed>\r\n");
      ++ErrorCount;
  }

   /* Test 2: Memory layer En
      - Set layer window
      - Set layer Alpha, if available
      - Enable layer
      - Check video lock
      - Set layer scaling, if available
      - Check video lock
      - Move layer window
      - Check video lock
      - Disable layer
      - Check video lock
  */
  for(layerIndex=XVMIX_LAYER_1; layerIndex<XVMix_GetNumLayers(MixerPtr); ++layerIndex) {
    xil_printf("\r\n--> Test Memory Layer %d <--\r\n", layerIndex);
    baseaddr = XVMix_GetLayerBufferAddr(MixerPtr, layerIndex);
    xil_printf("   Layer Buffer Addr: 0x%X\r\n", baseaddr);

	Win.StartX = 10;
	Win.StartY = 10;
	Win.Width  = 256;
	Win.Height = 256;

	Win.StartX += layerIndex*50; //offset each layer by 50 pixels in horiz. dir
	Win.StartY += layerIndex*50; //offset each layer by 50 pixels in vert. dir

	Cfmt = XVMix_GetLayerColorFormat(MixerPtr, layerIndex);
    xil_printf("   Layer Color Format: %s\r\n", XVidC_GetColorFormatStr(Cfmt));
	Stride = ((Cfmt == XVIDC_CSF_YCRCB_422) ? 2: 4); //BytesPerPixel
	Stride *= Win.Width;

    xil_printf("   Set Layer Window (%3d, %3d, %3d, %3d): ",
		    Win.StartX, Win.StartY, Win.Width, Win.Height);
	Status = XVMix_SetLayerWindow(MixerPtr, layerIndex, &Win, Stride);
    if(Status != XST_SUCCESS) {
        xil_printf("<ERR:: Command Failed>\r\n");
        ++ErrorCount;
    } else {
        xil_printf("Done\r\n");
    }

    xil_printf("   Set Layer Alpha to %d: ", XVMIX_ALPHA_MAX);
    if(XVMix_IsAlphaEnabled(MixerPtr, layerIndex)) {
	  Status = XVMix_SetLayerAlpha(MixerPtr, layerIndex, XVMIX_ALPHA_MAX);
      if(Status != XST_SUCCESS) {
        xil_printf("<ERR:: Command Failed>\r\n");
        ++ErrorCount;
      } else {
        xil_printf("Done\r\n");
      }
    } else {
        xil_printf("(Disabled in HW)\r\n");
    }

    xil_printf("   Enable Layer: ");
    Status = XVMix_LayerEnable(MixerPtr, layerIndex);
    if(Status != XST_SUCCESS) {
        xil_printf("<ERR:: Command Failed>\r\n");
        ++ErrorCount;
    } else {
        xil_printf("Done\r\n");
    }

    //Check for vidout lock
    xil_printf("   Check Vidout State: ");
	ErrorCount += (!CheckVidoutLock() ? 1 : 0);

    xil_printf("   Move window (x+10), (y+10): ");
    Status = XVMix_MoveLayerWindow(MixerPtr,
		                       layerIndex,
								   (Win.StartX+10),
								   (Win.StartY+10));
    if(Status != XST_SUCCESS) {
      xil_printf("<ERR:: Command Failed>\r\n");
      ++ErrorCount;
    } else {
      xil_printf("Done\r\n");
    }

    //Check for vidout lock
    xil_printf("   Check Vidout State: ");
	ErrorCount += (!CheckVidoutLock() ? 1 : 0);

    xil_printf("   Set Layer Scale Factor to 4x: ");
    if(XVMix_IsScalingEnabled(MixerPtr, layerIndex)) {
	  Status = XVMix_SetLayerScaleFactor(MixerPtr,
			                             layerIndex,
										 XVMIX_SCALE_FACTOR_4X);
      if(Status != XST_SUCCESS) {
        xil_printf("<ERR:: Command Failed>\r\n");
        ++ErrorCount;
      } else {
        xil_printf("Done\r\n");
        //Check for vidout lock
        xil_printf("   Check Vidout State: ");
	ErrorCount += (!CheckVidoutLock() ? 1 : 0);
      }
    } else {
        xil_printf("(Disabled in HW)\r\n");
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
	ErrorCount += (!CheckVidoutLock() ? 1 : 0);
  }

  /* Test 4: Logo Layer
   *   - Enable logo layer
   *   - Check for lock
   *   - Set Color Key
   *   - Move logo position
   *   - Check for lock
   *   - Disable logo layer
   *   - Check for lock
   */
  xil_printf("\r\n--> Test Logo Layer <--\r\n");
  if(XVMix_IsLogoEnabled(MixerPtr)) {
    xil_printf("   Enable Logo Layer: ");
    Status = XVMix_LayerEnable(MixerPtr, XVMIX_LAYER_LOGO);
    if(Status == XST_SUCCESS) {
	  ErrorCount += (!CheckVidoutLock() ? 1 : 0);
    } else {
      xil_printf("<ERR:: Command Failed>\r\n");
      ++ErrorCount;
    }
  } else {
	 xil_printf("  (Disabled in HW)\r\n");
	 return(ErrorCount);
  }

  {
	  XVMix_LogoColorKey Data ={{10,10,10},{40,40,40}};

	  xil_printf("   Set Logo Layer Color Key \r\n  "
			     "   Min(10,10,10)  Max(40,40,40): ");
	  if(XVMix_IsLogoColorKeyEnabled(MixerPtr)) {
        Status = XVMix_SetLogoColorKey(MixerPtr, Data);
        if(Status == XST_SUCCESS) {
            xil_printf("<ERR:: Command Failed>\r\n");
            ++ErrorCount;
        } else {
            xil_printf("Done\r\n");
        }
	  } else {
          xil_printf("(Disabled in HW)\r\n");
      }

	    xil_printf("   Move Logo window (100, 100): ");
	    Status = XVMix_MoveLayerWindow(MixerPtr,
			                       XVMIX_LAYER_LOGO,
									   100,
									   100);
	    if(Status != XST_SUCCESS) {
	      xil_printf("ERR:: Command Failed \r\n");
	      ++ErrorCount;
	    } else {
	      xil_printf("Done\r\n");
	    }

	    //Check for vidout lock
	    xil_printf("   Check Vidout State: ");
		ErrorCount += (!CheckVidoutLock() ? 1 : 0);
  }

  xil_printf("   Disable Logo Layer: ");
  Status = XVMix_LayerDisable(MixerPtr, XVMIX_LAYER_LOGO);
  if(Status == XST_SUCCESS) {
     ErrorCount += (!CheckVidoutLock() ? 1 : 0);
  } else {
     xil_printf("ERR:: Command Failed\r\n");
     ++ErrorCount;
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
  xil_printf("\r\nReset HLS IP \r\n");
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
  };

  init_platform();

  xil_printf("Start Mixer Example Design Test\r\n");

  /* Setup Reset line and video lock monitor */
  gpio_hlsIpReset = (u32*)XPAR_HLS_IP_RESET_BASEADDR;
  gpio_videoLockMonitor = (u32*)XPAR_VIDEO_LOCK_MONITOR_BASEADDR;

  //Release reset line
  *gpio_hlsIpReset = 1;

  /* Initialize IRQ */
  Status = SetupInterrupts();
  if (Status == XST_FAILURE) {
	xil_printf("ERR:: Interrupt Setup Failed\r\n");
	xil_printf("ERR:: Test could not be completed\r\n");
	while(1);
  }

  Status = DriverInit();
  if(Status != XST_SUCCESS) {
	xil_printf("ERR:: Driver Init. Failed\r\n");
	xil_printf("ERR:: Test could not be completed\r\n");
	while(1);
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
	xil_printf("ERR:: Video should not be locked\r\n");
	xil_printf("ERR:: Test could not be completed\r\n");
	while(1);
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
