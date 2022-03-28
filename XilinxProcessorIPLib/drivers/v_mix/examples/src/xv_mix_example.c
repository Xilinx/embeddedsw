/*******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file main.c
*
* This file demonstrates the example usage of Mixer IP available in catalogue
* Please refer v_mix example design guide for details on HW setup
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   11/24/15   Initial Release
*             02/05/16   Add Logo test
*             03/08/16   Add configuration table for all layers, replace
*                        delay with polling with time out
*             03/14/16   Fix bug startx not multiple of pixels/clk for window
*                        move
*             08/05/16   Add Logo Pixel Alpha test
* 2.00  vyc   10/04/17   Add second buffer pointer for semi-planar formats
* 2.10  vyc   04/04/18   Add support for streaming layers using Frame Buffer
*                        Read for streaming input
*                        Add support for ZCU102, ZCU104, ZCU106
* 4.00  vyc   04/04/18   Add support for streaming layers using Frame Buffer
* 5.00  pv    19/04/18   Added flushing feature support in driver.
*			 flush bit should be set and held (until reset) by
*			 software to flush pending transactions.IP is expecting
*			 a hard reset, when flushing is done.(There is a flush
*			 status bit and is asserted when the flush is done).
* 6.00  pg    01/10/20   Add Colorimetry feature.
*                        Program CSC coefficient registers to do color conversion
*                        from YUV to RGB and RGB to YUV.
* </pre>
*
******************************************************************************/

#include "xparameters.h"
#include "platform.h"
#include "sleep.h"
#include "xv_tpg.h"
#include "xv_mix_l2.h"
#include "xvidc.h"
#include "xvtc.h"
#if defined (__MICROBLAZE__)
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#include "xgpio.h"

#if defined(__MICROBLAZE__)
#define DDR_BASEADDR XPAR_MIG7SERIES_0_BASEADDR
#else
#define DDR_BASEADDR XPAR_DDR_MEM_BASEADDR
#endif

#if defined XPAR_PSU_ACPU_GIC_DEVICE_ID
#define PS_ACPU_GIC_DEVICE_ID XPAR_PSU_ACPU_GIC_DEVICE_ID
#elif defined XPAR_SCUGIC_0_DEVICE_ID
#define PS_ACPU_GIC_DEVICE_ID XPAR_SCUGIC_0_DEVICE_ID
#else
#warning No GIC Device ID found
#endif

#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
#include "xv_frmbufrd_l2.h"
extern XV_frmbufrd_Config XV_frmbufrd_ConfigTable[];
#define XVFRMBUFRD_BUFFER_BASEADDR (DDR_BASEADDR + (0x20000000))
#define XVFRMBUFRD_CHROMA_ADDR_OFFSET   (0x01000000U)
#endif

#define NUM_TEST_MODES    (2)

/* Memory Layers for Mixer */
#define XVMIX_LAYER1_BASEADDR      (DDR_BASEADDR + (0x21000000))
#define XVMIX_LAYER_ADDR_OFFSET    (0x01000000U)
#define XVMIX_CHROMA_ADDR_OFFSET   (0x01000000U)

#define VIDEO_MONITOR_LOCK_TIMEOUT (2000000)

extern unsigned char Logo_R[];
extern unsigned char Logo_G[];
extern unsigned char Logo_B[];
extern unsigned char Logo_A[];

XV_tpg     tpg;
XV_Mix_l2  mix;
XVtc       vtc;
#if defined (__MICROBLAZE__)
XIntc      intc;
#else
XScuGic    intc;
#endif
XGpio      vmon;

XVidC_VideoStream VidStream;
XVidC_ColorStd colorStandard;
XVidC_ColorRange colorRange;
u8 colorDepth;

u32 volatile *gpio_hlsIpReset;

#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
typedef struct {
  XV_FrmbufRd_l2 Inst;
  u32            DeviceId;
} FrmbufInst;

FrmbufInst FBLayer[XPAR_XV_FRMBUFRD_NUM_INSTANCES];
#endif

static const XVidC_VideoWindow MixLayerConfig[8] =
{// X   Y     W    H
  {12,  10,  128, 128}, //Layer 1
  {200, 10,  128, 128}, //Layer 2
  {400, 10,  128, 128}, //Layer 3
  {600, 10,  128, 128}, //Layer 4
  {800, 100, 128, 128}, //Layer 5
  {12,  100, 128, 128}, //Layer 6
  {200, 100, 128, 128}, //Layer 7
  {200, 300, 128, 128}  //Layer 8
};

/*****************************************************************************/
/**
 * This macro reads GPIO to check video lock status
 *
 * @param  GpioPtr is pointer to the gpio Instance
 * @return T/F
 *
 *****************************************************************************/
#define XVMonitor_IsVideoLocked(GpioPtr)   (XGpio_DiscreteRead(GpioPtr, 1))


void resetIp(void);
static int DriverInit(void);
static int SetupInterrupts(void);
static void ConfigTpg(XVidC_VideoStream *StreamPtr);
static void ConfigMixer(XVidC_VideoStream *StreamPtr);
#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
static XVidC_ColorFormat FindMemFormat(XVidC_ColorFormat StreamFmt);
static u32 CalcStride(XVidC_ColorFormat Cfmt,
                      u16 AXIMMDataWidth,
                      XVidC_VideoStream *StreamPtr);
static int ConfigFrmbuf(XV_FrmbufRd_l2 *LayerFrmbuf,
                        u32 StrideInBytes,
                        XVidC_ColorFormat Cfmt,
                        XVidC_VideoStream *LayerStreamPtr);
#endif
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
#if defined(__MICROBLAZE__)
  int Status;
  XIntc *IntcPtr = &intc;

  /* Initialize the Interrupt controller */
  Status = XIntc_Initialize(IntcPtr,
                            XPAR_PROCESSOR_SS_PROCESSOR_AXI_INTC_DEVICE_ID);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Interrupt controller device not found\r\n");
    return(XST_FAILURE);
  }

  /* Hook up interrupt service routine */
  Status = XIntc_Connect(IntcPtr,
                         XPAR_PROCESSOR_SS_PROCESSOR_AXI_INTC_V_MIX_0_INTERRUPT_INTR,
                         (XInterruptHandler)XVMix_InterruptHandler,
                         &mix);
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: Mixer interrupt connect failed!\r\n");
    return XST_FAILURE;
  }

  /* Enable the interrupt vector at the interrupt controller */
  XIntc_Enable(IntcPtr,
               XPAR_PROCESSOR_SS_PROCESSOR_AXI_INTC_V_MIX_0_INTERRUPT_INTR);

  /*
   * Start the interrupt controller such that interrupts are recognized
   * and handled by the processor
   */
  Status = XIntc_Start(IntcPtr, XIN_REAL_MODE);
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: Failed to start interrupt controller\r\n");
    return XST_FAILURE;
  }

#else
  int Status;
  XScuGic *IntcPtr = &intc;

  /* Initialize the Interrupt controller */
  XScuGic_Config *IntcCfgPtr;
  IntcCfgPtr = XScuGic_LookupConfig(PS_ACPU_GIC_DEVICE_ID);
  if(IntcCfgPtr == NULL)
  {
    print("ERR:: Interrupt Controller not found");
    return (XST_DEVICE_NOT_FOUND);
  }
  Status = XScuGic_CfgInitialize(IntcPtr,
                                 IntcCfgPtr,
                                 IntcCfgPtr->CpuBaseAddress);
  if (Status != XST_SUCCESS) {
    xil_printf("Intc initialization failed!\r\n");
    return XST_FAILURE;
  }

  /* Hook up interrupt service routine */
  Status |= XScuGic_Connect(IntcPtr,
                            XPAR_FABRIC_V_MIX_0_INTERRUPT_INTR,
                            (XInterruptHandler)XVMix_InterruptHandler,
                            (void *)&mix);
  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                               (Xil_ExceptionHandler) XScuGic_InterruptHandler,
                               IntcPtr);
  if (Status != XST_SUCCESS) {
    xil_printf("ERR:: Mixer interrupt connect failed!\r\n");
    return XST_FAILURE;
  }

  /* Enable the interrupt vector at the interrupt controller */
  XScuGic_Enable(IntcPtr, XPAR_FABRIC_V_MIX_0_INTERRUPT_INTR);

#endif

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
  XGpio_Config *GpioCfgPtr;

  vtc_Config = XVtc_LookupConfig(XPAR_V_TC_0_DEVICE_ID);
  if(vtc_Config == NULL) {
    xil_printf("ERROR:: VTC device not found\r\n");
    return(XST_FAILURE);
  }

  Status = XVtc_CfgInitialize(&vtc, vtc_Config, vtc_Config->BaseAddress);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: VTC Initialization failed %d\r\n", Status);
    return(XST_FAILURE);
  }

  Status = XV_tpg_Initialize(&tpg, XPAR_V_TPG_0_DEVICE_ID);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: TPG device not found\r\n");
    return(XST_FAILURE);
  }

#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
  for(int count=0; count < XPAR_XV_FRMBUFRD_NUM_INSTANCES; ++count)
  {
    FBLayer[count].DeviceId = XV_frmbufrd_ConfigTable[count].DeviceId;
    Status = XVFrmbufRd_Initialize(&FBLayer[count].Inst, FBLayer[count].DeviceId);
    if(Status != XST_SUCCESS) {
        xil_printf("ERROR:: Frame Buffer Read initialization failed\r\n");
    return(XST_FAILURE);
    }
  }
#endif

  Status  = XVMix_Initialize(&mix, XPAR_V_MIX_0_DEVICE_ID);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Mixer device not found\r\n");
    return(XST_FAILURE);
  }

  //Video Lock Monitor
  GpioCfgPtr = XGpio_LookupConfig(XPAR_VIDEO_LOCK_MONITOR_DEVICE_ID);
  if(GpioCfgPtr == NULL) {
    xil_printf("ERROR:: Video Lock Monitor GPIO device not found\r\n");
    return(XST_FAILURE);
  }

  Status = XGpio_CfgInitialize(&vmon,
                               GpioCfgPtr,
                               GpioCfgPtr->BaseAddress);
  if(Status != XST_SUCCESS)  {
    xil_printf("ERROR:: Video Lock Monitor GPIO Initialization failed %d\r\n", Status);
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

#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
/*****************************************************************************/
/**
 * This function calculates the stride
 *
 * @returns stride in bytes
 *
 *****************************************************************************/
static u32 CalcStride(XVidC_ColorFormat Cfmt,
                      u16 AXIMMDataWidth,
                      XVidC_VideoStream *StreamPtr)
{
  u32 stride;
  int width = StreamPtr->Timing.HActive;
  u16 MMWidthBytes = AXIMMDataWidth/8;

  if ((Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)
      || (Cfmt == XVIDC_CSF_MEM_Y10)) {
    // 4 bytes per 3 pixels (Y_UV10, Y_UV10_420, Y10)
    stride = ((((width*4)/3)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
  }
  else if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420)
           || (Cfmt == XVIDC_CSF_MEM_Y8)) {
    // 1 byte per pixel (Y_UV8, Y_UV8_420, Y8)
    stride = ((width+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
  }
  else if ((Cfmt == XVIDC_CSF_MEM_RGB8) || (Cfmt == XVIDC_CSF_MEM_YUV8)
           || (Cfmt == XVIDC_CSF_MEM_BGR8)) {
    // 3 bytes per pixel (RGB8, YUV8, BGR8)
     stride = (((width*3)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
  }
  else {
    // 4 bytes per pixel
    stride = (((width*4)+MMWidthBytes-1)/MMWidthBytes)*MMWidthBytes;
  }

  return(stride);
}

/*****************************************************************************/
/**
 * This function finds a compatible memory format for the frame buffer
 *
 * @returns video color format
 *
 *****************************************************************************/
static XVidC_ColorFormat FindMemFormat(XVidC_ColorFormat StreamFmt)
{
  XVidC_ColorFormat Cfmt;

  switch(StreamFmt) {
    case XVIDC_CSF_RGB :
        Cfmt  = XVIDC_CSF_MEM_RGB8;
        break;
    case XVIDC_CSF_YCRCB_444 :
        Cfmt  = XVIDC_CSF_MEM_YUV8;
        break;
    case XVIDC_CSF_YCRCB_422 :
        Cfmt  = XVIDC_CSF_MEM_Y_UV8;
        break;
    case XVIDC_CSF_YCRCB_420 :
        Cfmt  = XVIDC_CSF_MEM_Y_UV8_420;
        break;
    case XVIDC_CSF_RGBA :
        Cfmt  = XVIDC_CSF_MEM_RGBA8;
        break;
    case XVIDC_CSF_YCRCBA_444 :
        Cfmt  = XVIDC_CSF_MEM_YUVA8;
        break;
    default :
        Cfmt  = XVIDC_CSF_MEM_RGB8;
        break;
  }

  return(Cfmt);
}

/*****************************************************************************/
/**
 * This function configures Frame Buffer for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
static int ConfigFrmbuf(XV_FrmbufRd_l2 *LayerFrmbuf,
                        u32 StrideInBytes,
                        XVidC_ColorFormat Cfmt,
                        XVidC_VideoStream *StreamPtr)
{
  int Status;
  u32 IrqMask;

  /* Configure Frame Buffers */
  Status = XVFrmbufRd_SetMemFormat(LayerFrmbuf, StrideInBytes, Cfmt, StreamPtr);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Unable to configure Frame Buffer Read\r\n");
    return(XST_FAILURE);
  }

  Status = XVFrmbufRd_SetBufferAddr(LayerFrmbuf, XVFRMBUFRD_BUFFER_BASEADDR);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Unable to configure Frame Buffer Read buffer address\r\n");
    return(XST_FAILURE);
  }

  /* Set Chroma Buffer Address for semi-planar color formats */
  if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
      (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
    Status = XVFrmbufRd_SetChromaBufferAddr(LayerFrmbuf, XVFRMBUFRD_BUFFER_BASEADDR+XVFRMBUFRD_CHROMA_ADDR_OFFSET);
    if(Status != XST_SUCCESS) {
      xil_printf("ERROR:: Unable to configure Frame Buffer Read chroma buffer address\r\n");
      return(XST_FAILURE);
    }
  }

  /* Start Frame Buffers */
  IrqMask = XVFRMBUFRD_IRQ_DONE_MASK | XVFRMBUFRD_IRQ_READY_MASK;
  XVFrmbufRd_InterruptDisable(LayerFrmbuf, IrqMask);
  XVFrmbufRd_Start(LayerFrmbuf);

  xil_printf("INFO: FRMBUF configured\r\n");
  return(Status);
}
#endif

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
  XVidC_ColorFormat Cfmt;

  /* Setup default config after reset */
  XVMix_LayerDisable(MixerPtr, XVMIX_LAYER_MASTER);
  XVMix_SetVidStream(MixerPtr, StreamPtr);

  /* Set Memory Layer Addresses */
  NumLayers = XVMix_GetNumLayers(MixerPtr);
  MemAddr = XVMIX_LAYER1_BASEADDR;
  for(index = XVMIX_LAYER_1; index < NumLayers; ++index) {
      XVMix_GetLayerColorFormat(MixerPtr, index, &Cfmt);
      if (!(XVMix_IsLayerInterfaceStream(MixerPtr, index))) {
          Status = XVMix_SetLayerBufferAddr(MixerPtr, index, MemAddr);
          if(Status != XST_SUCCESS) {
              xil_printf("MIXER ERROR:: Unable to set layer %d buffer addr to 0x%X\r\n",
                          index, MemAddr);
          } else {
              if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
                  (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420)) {
                  MemAddr += XVMIX_CHROMA_ADDR_OFFSET;
                  Status = XVMix_SetLayerChromaBufferAddr(MixerPtr, index, MemAddr);
                  if (Status != XST_SUCCESS) {
                      xil_printf("MIXER ERROR:: Unable to set layer %d chroma buffer2 addr to 0x%X\r\n",
                                  index, MemAddr);
                  }
              }
              MemAddr += XVMIX_LAYER_ADDR_OFFSET;
          }
      }
  }

  if(XVMix_IsLogoEnabled(MixerPtr)) {
    XVidC_VideoWindow Win;

    Win.StartX = 64;
    Win.StartY = 64;
    Win.Width  = 64;
    Win.Height = 64;

    Status = XVMix_LoadLogo(MixerPtr,
                            &Win,
                            Logo_R,
                            Logo_G,
                            Logo_B);
    if(Status != XST_SUCCESS) {
      xil_printf("MIXER ERROR:: Unable to load Logo \r\n");
    }

    if(XVMix_IsLogoPixAlphaEnabled(MixerPtr)) {
      Status = XVMix_LoadLogoPixelAlpha(MixerPtr, &Win, Logo_A);
      if(Status != XST_SUCCESS) {
        xil_printf("MIXER ERROR:: Unable to load Logo pixel alpha \r\n");
      }
    }
  } else {
      xil_printf("INFO: Logo Layer Disabled in HW \r\n");
  }
  XVMix_SetBackgndColor(MixerPtr, XVMIX_BKGND_BLUE, StreamPtr->ColorDepth);

  /*
   * Program CSC coefficients when ENABLE_CSC_COEFFICIENT_REGISTERS
   * is enabled
   */
#if XPAR_V_MIX_0_ENABLE_CSC_COEFFICIENT_REGISTERS
  colorStandard = XVIDC_BT_709;
  colorRange = XVIDC_CR_16_240;
  colorDepth = MixerPtr->Stream.ColorDepth;

  XVMix_SetCscCoeffs(MixerPtr, colorStandard, colorRange, colorDepth);
#endif

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
  int Lock = FALSE;
  u32 Timeout;

  Timeout = VIDEO_MONITOR_LOCK_TIMEOUT;

  usleep(2000000);         //wait

  while(!Lock && Timeout) {
    if(XVMonitor_IsVideoLocked(&vmon)) {
        xil_printf("Locked\r\n");
        Lock = TRUE;
        Status = TRUE;
    }
    --Timeout;
  }

  if(!Timeout) {
      xil_printf("<ERROR:: Not Locked>\r\n\r\n");
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
      xil_printf("<ERROR:: Command Failed>\r\n");
      ++ErrorCount;
  }

  xil_printf("Enable  Master Layer: ");
  Status = XVMix_LayerEnable(MixerPtr, XVMIX_LAYER_MASTER);
  if(Status == XST_SUCCESS) {
      ErrorCount += (!CheckVidoutLock() ? 1 : 0);
  } else {
      xil_printf("<ERROR:: Command Failed>\r\n");
      ++ErrorCount;
  }

   /* Test 2: Layer Enable
      - Set layer window
      - Set layer Alpha, if available
      - Set layer scaling, if available
      - Enable layer
      - Check video lock
      - Move layer window
      - Check video lock
      - Disable layer
      - Check video lock
  */
  for(layerIndex=XVMIX_LAYER_1; layerIndex<XVMix_GetNumLayers(MixerPtr); ++layerIndex) {

    xil_printf("\r\n--> Test Mixer Layer %d <--\r\n", layerIndex);
    if (!(XVMix_IsLayerInterfaceStream(MixerPtr, layerIndex))) {
      baseaddr = XVMix_GetLayerBufferAddr(MixerPtr, layerIndex);
      xil_printf("   Layer Buffer Addr: 0x%X\r\n", baseaddr);
    }
    Win = MixLayerConfig[layerIndex-1];

    XVMix_GetLayerColorFormat(MixerPtr, layerIndex, &Cfmt);

    xil_printf("   Layer Color Format: %s\r\n", XVidC_GetColorFormatStr(Cfmt));
    Stride = ((Cfmt == XVIDC_CSF_YCRCB_422) ? 2: 4); //BytesPerPixel
    Stride *= Win.Width;

    xil_printf("   Set Layer Window (%3d, %3d, %3d, %3d): ",
            Win.StartX, Win.StartY, Win.Width, Win.Height);
    Status = XVMix_SetLayerWindow(MixerPtr, layerIndex, &Win, Stride);
    if(Status != XST_SUCCESS) {
        xil_printf("<ERROR:: Command Failed>\r\n");
        ++ErrorCount;
    } else {
        xil_printf("Done\r\n");
    }

    xil_printf("   Set Layer Alpha to %d: ", XVMIX_ALPHA_MAX);
    if(XVMix_IsAlphaEnabled(MixerPtr, layerIndex)) {
      Status = XVMix_SetLayerAlpha(MixerPtr, layerIndex, XVMIX_ALPHA_MAX);
      if(Status != XST_SUCCESS) {
        xil_printf("<ERROR:: Command Failed>\r\n");
        ++ErrorCount;
      } else {
        xil_printf("Done\r\n");
      }
    } else {
        xil_printf("(Disabled in HW)\r\n");
    }

    xil_printf("   Set Layer Scale Factor to 2x: ");
    if(XVMix_IsScalingEnabled(MixerPtr, layerIndex)) {
      Status = XVMix_SetLayerScaleFactor(MixerPtr,
                                         layerIndex,
                                         XVMIX_SCALE_FACTOR_2X);
      if(Status != XST_SUCCESS) {
        xil_printf("<ERROR:: Command Failed>\r\n");
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
        xil_printf("<ERROR:: Command Failed>\r\n");
        ++ErrorCount;
    } else {
        xil_printf("Done\r\n");
    }

    //Check for vidout lock
    xil_printf("   Check Vidout State: ");
    ErrorCount += (!CheckVidoutLock() ? 1 : 0);

    xil_printf("   Move window (x+12), (y+12): ");
    Status = XVMix_MoveLayerWindow(MixerPtr,
                                   layerIndex,
                                   (Win.StartX+12),
                                   (Win.StartY+12));
    if(Status != XST_SUCCESS) {
      xil_printf("<ERROR:: Command Failed>\r\n");
      ++ErrorCount;
    } else {
      xil_printf("Done\r\n");
    }

    //Check for vidout lock
    xil_printf("   Check Vidout State: ");
    ErrorCount += (!CheckVidoutLock() ? 1 : 0);
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
      xil_printf("<ERROR:: Command Failed>\r\n");
      ++ErrorCount;
    }
  } else {
     xil_printf("  (Disabled in HW)\r\n");
     return(ErrorCount);
  }

  xil_printf("   Logo Pixel Alpha: ");
  if(XVMix_IsLogoPixAlphaEnabled(MixerPtr)) {
      xil_printf("Enabled\r\n");
  } else {
      xil_printf("(Disabled in HW)\r\n");
  }

  {
      XVMix_LogoColorKey Data ={{10,10,10},{40,40,40}};

      xil_printf("   Set Logo Layer Color Key \r\n  "
                 "   Min(10,10,10)  Max(40,40,40): ");
      if(XVMix_IsLogoColorKeyEnabled(MixerPtr)) {

        Status = XVMix_SetLogoColorKey(MixerPtr, Data);
        if(Status != XST_SUCCESS) {
            xil_printf("<ERROR:: Command Failed>\r\n");
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
          xil_printf("ERROR:: Command Failed \r\n");
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
     xil_printf("ERROR:: Command Failed\r\n");
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
#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
  /* Stop Frame Buffer and wait for IDLE */
  for(int i=0; i<XPAR_XV_FRMBUFRD_NUM_INSTANCES; ++i) {
    XVFrmbufRd_Stop(&FBLayer[i].Inst);
  }
#endif

  xil_printf("\r\nReset HLS IP \r\n");
  *gpio_hlsIpReset = 0; //reset IPs
  usleep(1000);         //hold reset line
  *gpio_hlsIpReset = 1; //release reset
  usleep(1000);         //wait
}

/***************************************************************************
*  This is the main loop of the application
***************************************************************************/
int main(void)
{
  int Status, index;
  int FailCount = 0;
  int Lock = FALSE;
#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
  XV_Mix_l2 *MixerPtr = &mix;
  XVidC_VideoStream LayerStream;
  XVidC_ColorFormat StreamFmt, MemFmt;
#endif
  XVidC_ColorFormat Cfmt;
  XVidC_VideoTiming const *TimingPtr;
  XVidC_VideoMode TestModes[NUM_TEST_MODES] =
  { XVIDC_VM_1080_60_P,
    XVIDC_VM_UHD_30_P
  };

  init_platform();

  xil_printf("Start Mixer Example Design Test\r\n");

  /* Setup Reset line and video lock monitor */
  gpio_hlsIpReset = (u32*)XPAR_HLS_IP_RESET_BASEADDR;

  //Release reset line
  *gpio_hlsIpReset = 1;

  /* Initialize IRQ */
  Status = SetupInterrupts();
  if (Status == XST_FAILURE) {
    xil_printf("ERROR:: Interrupt Setup Failed\r\n");
    xil_printf("ERROR:: Test could not be completed\r\n");
    while(1);
  }

  Status = DriverInit();
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Driver Init. Failed\r\n");
    xil_printf("ERROR:: Test could not be completed\r\n");
    while(1);
  }

  /* Enable exceptions. */
  Xil_ExceptionEnable();

  /* Setup a default stream */
  /* Color Format for the default stream is set to the mixer master stream
   * color format
   */
  XVMix_GetLayerColorFormat(&mix, XVMIX_LAYER_MASTER, &Cfmt);

  VidStream.PixPerClk     = tpg.Config.PixPerClk;
  VidStream.ColorFormatId = Cfmt;
  VidStream.ColorDepth    = tpg.Config.MaxDataWidth;

  resetIp();

  /* sanity check */
  if(XVMonitor_IsVideoLocked(&vmon)) {
    xil_printf("ERROR:: Video should not be locked\r\n");
    xil_printf("ERROR:: Test could not be completed\r\n");
    while(1);
  }

  for(index=0; index<NUM_TEST_MODES; ++index)
  {
#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
	int numFB = 0;
	int layerIndex = 0;

	for (layerIndex = XVMIX_LAYER_1; layerIndex <
			MixerPtr->Mix.Config.NumLayers; ++layerIndex) {
		if ((XVMix_IsLayerInterfaceStream(MixerPtr, layerIndex))) {
			/* Stop Frame Buffers */
			XVFrmbufRd_Stop(&FBLayer[numFB].Inst);
			resetIp();
			numFB++;
		}
	}
	numFB = 0;
#endif
    // Get mode to test
    VidStream.VmId = TestModes[index];

    // Get mode timing parameters
    TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
    VidStream.Timing = *TimingPtr;
    VidStream.FrameRate = XVidC_GetFrameRate(VidStream.VmId);

    xil_printf("\r\n********************************************\r\n");
    xil_printf("Test Input Stream: %s (%s)\r\n",
            XVidC_GetVideoModeStr(VidStream.VmId),
            XVidC_GetColorFormatStr(VidStream.ColorFormatId));
    xil_printf("********************************************\r\n");
    ConfigVtc(&VidStream);

#ifdef XPAR_XV_FRMBUFRD_NUM_INSTANCES
    /* Configure Frame Buffers */
    for(int layerIndex=XVMIX_LAYER_1; layerIndex<MixerPtr->Mix.Config.NumLayers; ++layerIndex) {
      if ((XVMix_IsLayerInterfaceStream(MixerPtr, layerIndex))) {
          XVMix_GetLayerColorFormat(MixerPtr, layerIndex, &StreamFmt);
          LayerStream.ColorFormatId = StreamFmt;
          LayerStream.Timing.HActive = MixLayerConfig[layerIndex-1].Width;
          LayerStream.Timing.VActive = MixLayerConfig[layerIndex-1].Height;
          MemFmt = FindMemFormat(StreamFmt);
          int stride = CalcStride(MemFmt,
                                  FBLayer[numFB].Inst.FrmbufRd.Config.AXIMMDataWidth,
                                  &LayerStream);
          ConfigFrmbuf(&FBLayer[numFB].Inst, stride, MemFmt, &LayerStream);
          numFB++;
      }
    }
#endif
    ConfigMixer(&VidStream);
    ConfigTpg(&VidStream);
    xil_printf("Wait for vid out lock: ");

    Lock = CheckVidoutLock();
    if(Lock) {
      Status = RunMixerFeatureTests(&VidStream);
      if(Status != 0) { //problems encountered in feature test
        ++FailCount;
      }
    } else {
      ++FailCount;
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
