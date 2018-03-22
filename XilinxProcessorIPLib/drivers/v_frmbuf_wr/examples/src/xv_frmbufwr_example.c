/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 *
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file main.c
*
* This file demonstrates the example usage of Frame Buffer Read/Write IP
* available in catalogue. Please refer v_frmbuf_wr example design guide for
* details on HW setup.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vyc   04/05/17   Initial Release
* 2.00  vyc   10/04/17   Add second buffer pointer for semi-planar formats
*                        Add new memory formats BGRX8 and UYVY8
* 3.00  vyc   04/04/18   Add support for ZCU102, ZCU104, ZCU106
*                        Add new memory format BGR8
* 4.00  pv    11/10/18   Added flushing feature support in driver.
*			 flush bit should be set and held (until reset) by
*			 software to flush pending transactions.IP is expecting
*			 a hard reset, when flushing is done.(There is a flush
*			 status bit and is asserted when the flush is done).
* 4.10  vv    02/05/19   Added new pixel formats with 12 and 16 bpc.
* </pre>
*
******************************************************************************/

#include "xparameters.h"
#include "platform.h"
#include "sleep.h"
#if defined(__MICROBLAZE__)
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#include "xv_frmbufrd_l2.h"
#include "xv_frmbufwr_l2.h"
#include "xvidc.h"
#include "xvtc.h"
#include "xgpio.h"

#if defined(__MICROBLAZE__)
#define DDR_BASEADDR XPAR_MIG7SERIES_0_BASEADDR
#else
#define DDR_BASEADDR XPAR_DDR_MEM_BASEADDR
#endif

#define XVFRMBUFRD_BUFFER_BASEADDR (DDR_BASEADDR + (0x20000000))
#define XVFRMBUFWR_BUFFER_BASEADDR (DDR_BASEADDR + (0x21000000))

#define VIDEO_MONITOR_LOCK_TIMEOUT (1000000)
#define FRMBUF_IDLE_TIMEOUT (1000000)

#define NUM_TEST_MODES 4
#define NUM_TEST_FORMATS 26

#define CHROMA_ADDR_OFFSET   (0x01000000U)

//mapping between memory and streaming video formats
typedef struct {
  XVidC_ColorFormat MemFormat;
  XVidC_ColorFormat StreamFormat;
  u16 FormatBits;
} VideoFormats;

VideoFormats ColorFormats[NUM_TEST_FORMATS] =
{
  //memory format            stream format        bits per component
  {XVIDC_CSF_MEM_RGBX8,      XVIDC_CSF_RGB,       8},
  {XVIDC_CSF_MEM_YUVX8,      XVIDC_CSF_YCRCB_444, 8},
  {XVIDC_CSF_MEM_YUYV8,      XVIDC_CSF_YCRCB_422, 8},
  {XVIDC_CSF_MEM_RGBX10,     XVIDC_CSF_RGB,       10},
  {XVIDC_CSF_MEM_YUVX10,     XVIDC_CSF_YCRCB_444, 10},
  {XVIDC_CSF_MEM_Y_UV8,      XVIDC_CSF_YCRCB_422, 8},
  {XVIDC_CSF_MEM_Y_UV8_420,  XVIDC_CSF_YCRCB_420, 8},
  {XVIDC_CSF_MEM_RGB8,       XVIDC_CSF_RGB,       8},
  {XVIDC_CSF_MEM_YUV8,       XVIDC_CSF_YCRCB_444, 8},
  {XVIDC_CSF_MEM_Y_UV10,     XVIDC_CSF_YCRCB_422, 10},
  {XVIDC_CSF_MEM_Y_UV10_420, XVIDC_CSF_YCRCB_420, 10},
  {XVIDC_CSF_MEM_Y8,         XVIDC_CSF_YONLY, 8},
  {XVIDC_CSF_MEM_Y10,        XVIDC_CSF_YONLY, 10},
  {XVIDC_CSF_MEM_BGRX8,      XVIDC_CSF_RGB,       8},
  {XVIDC_CSF_MEM_UYVY8,      XVIDC_CSF_YCRCB_422, 8},
  {XVIDC_CSF_MEM_BGR8,       XVIDC_CSF_RGB,       8},
  {XVIDC_CSF_MEM_RGBX12,     XVIDC_CSF_RGB,       12},
  {XVIDC_CSF_MEM_RGB16,      XVIDC_CSF_RGB,       16},
  {XVIDC_CSF_MEM_YUVX12,     XVIDC_CSF_YCRCB_444, 12},
  {XVIDC_CSF_MEM_YUV16,      XVIDC_CSF_YCRCB_444, 16},
  {XVIDC_CSF_MEM_Y_UV12,     XVIDC_CSF_YCRCB_422, 12},
  {XVIDC_CSF_MEM_Y_UV16,     XVIDC_CSF_YCRCB_422, 16},
  {XVIDC_CSF_MEM_Y_UV12_420, XVIDC_CSF_YCRCB_420, 12},
  {XVIDC_CSF_MEM_Y_UV16_420, XVIDC_CSF_YCRCB_420, 16},
  {XVIDC_CSF_MEM_Y12,        XVIDC_CSF_YONLY, 12},
  {XVIDC_CSF_MEM_Y16,        XVIDC_CSF_YONLY, 16}
};

XV_FrmbufRd_l2     frmbufrd;
XV_frmbufrd_Config frmbufrd_cfg;
XV_FrmbufWr_l2     frmbufwr;
XV_frmbufwr_Config frmbufwr_cfg;
XVtc       vtc;
#if defined(__MICROBLAZE__)
XIntc      intc;
#else
XScuGic    intc;
#endif
XGpio      vmon;

XVidC_VideoStream VidStream;

u32 volatile *gpio_hlsIpReset;

/*****************************************************************************/
/**
 * These macros read GPIO to check video lock and overflow status
 *
 * @param  GpioPtr is pointer to the gpio Instance
 * @return T/F
 *
 *****************************************************************************/
#define XVMonitor_IsVideoLocked(GpioPtr) (XGpio_DiscreteRead(GpioPtr, 1))
#define XVMonitor_DidVideoOverflow(GpioPtr) (XGpio_DiscreteRead(GpioPtr, 2))


void resetIp(void);
static int DriverInit(void);
static int SetupInterrupts(void);
static u32 CalcStride(XVidC_ColorFormat Cfmt,
                      u16 AXIMMDataWidth,
                      XVidC_VideoStream *StreamPtr);
static int ConfigFrmbuf(u32 StrideInBytes,
                         XVidC_ColorFormat Cfmt,
                         XVidC_VideoStream *StreamPtr);
static void ConfigVtc(XVidC_VideoStream *StreamPtr);
static int ValidateTestCase(u16 PixPerClk,
                            XVidC_VideoMode Mode,
                            u16 DataWidth,
                            VideoFormats Format);
static int CheckVidoutLock(void);
static int CheckVidinOverflow(void);

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
                         XPAR_PROCESSOR_SS_PROCESSOR_AXI_INTC_V_FRMBUF_RD_0_INTERRUPT_INTR,
                         (XInterruptHandler)XVFrmbufRd_InterruptHandler,
                         &frmbufrd);
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: FRMBUF RD interrupt connect failed!\r\n");
    return XST_FAILURE;
  }

  Status = XIntc_Connect(IntcPtr,
                         XPAR_PROCESSOR_SS_PROCESSOR_AXI_INTC_V_FRMBUF_WR_0_INTERRUPT_INTR,
                         (XInterruptHandler)XVFrmbufWr_InterruptHandler,
                         &frmbufwr);
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: FRMBUF WR interrupt connect failed!\r\n");
    return XST_FAILURE;
  }

  /* Enable the interrupt vector at the interrupt controller */
  XIntc_Enable(IntcPtr,
               XPAR_PROCESSOR_SS_PROCESSOR_AXI_INTC_V_FRMBUF_RD_0_INTERRUPT_INTR);
  XIntc_Enable(IntcPtr,
               XPAR_PROCESSOR_SS_PROCESSOR_AXI_INTC_V_FRMBUF_WR_0_INTERRUPT_INTR);

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
  IntcCfgPtr = XScuGic_LookupConfig(XPAR_PSU_ACPU_GIC_DEVICE_ID);
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
                            XPAR_FABRIC_V_FRMBUF_RD_0_INTERRUPT_INTR,
                            (XInterruptHandler)XVFrmbufRd_InterruptHandler,
                            (void *)&frmbufrd);
  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                               (Xil_ExceptionHandler) XScuGic_InterruptHandler,
                               IntcPtr);
  if (Status != XST_SUCCESS) {
    xil_printf("ERR:: Frame Buffer Read interrupt connect failed!\r\n");
    return XST_FAILURE;
  }

  /* Enable the interrupt vector at the interrupt controller */
  XScuGic_Enable(IntcPtr, XPAR_FABRIC_V_FRMBUF_RD_0_INTERRUPT_INTR);

  /* Hook up interrupt service routine */
  Status |= XScuGic_Connect(IntcPtr,
                            XPAR_FABRIC_V_FRMBUF_WR_0_INTERRUPT_INTR,
                            (XInterruptHandler)XVFrmbufWr_InterruptHandler,
                            (void *)&frmbufwr);
  if (Status != XST_SUCCESS) {
    xil_printf("ERR:: Frame Buffer Read interrupt connect failed!\r\n");
    return XST_FAILURE;
  }

  /* Enable the interrupt vector at the interrupt controller */
  XScuGic_Enable(IntcPtr, XPAR_FABRIC_V_FRMBUF_WR_0_INTERRUPT_INTR);

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

  Status = XVFrmbufRd_Initialize(&frmbufrd, XPAR_V_FRMBUF_RD_0_DEVICE_ID);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Frame Buffer Read initialization failed\r\n");
    return(XST_FAILURE);
  }

  Status = XVFrmbufWr_Initialize(&frmbufwr, XPAR_V_FRMBUF_WR_0_DEVICE_ID);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Frame Buffer Write initialization failed\r\n");
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
    xil_printf("ERROR:: Video Lock Monitor GPIO Initialization failed %d\r\n",
               Status);
    return(XST_FAILURE);
  }

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
 * This function configures VTC for defined mode
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
  u8 bpp_numerator;
  u8 bpp_denominator = 1;

  switch (Cfmt) {
    case XVIDC_CSF_MEM_Y_UV10:
    case XVIDC_CSF_MEM_Y_UV10_420:
    case XVIDC_CSF_MEM_Y10:
      /* 4 bytes per 3 pixels (Y_UV10, Y_UV10_420, Y10) */
      bpp_numerator = 4;
      bpp_denominator = 3;
      break;
    case XVIDC_CSF_MEM_Y_UV8:
    case XVIDC_CSF_MEM_Y_UV8_420:
    case  XVIDC_CSF_MEM_Y8:
      /* 1 byte per pixel (Y_UV8, Y_UV8_420, Y8) */
      bpp_numerator = 1;
      break;
    case XVIDC_CSF_MEM_RGB8:
    case  XVIDC_CSF_MEM_YUV8:
    case XVIDC_CSF_MEM_BGR8:
      /* 3 bytes per pixel (RGB8, YUV8, BGR8) */
      bpp_numerator = 3;
      break;
    case XVIDC_CSF_MEM_RGBX12:
    case  XVIDC_CSF_MEM_YUVX12:
      /* 5 bytes per pixel (RGBX12, YUVX12) */
      bpp_numerator = 5;
      break;
    case XVIDC_CSF_MEM_Y_UV12:
    case  XVIDC_CSF_MEM_Y_UV12_420:
    case XVIDC_CSF_MEM_Y12:
      /* 3 bytes per 2 pixels (Y_UV12, Y_UV12_420, Y12) */
      bpp_numerator = 3;
      bpp_denominator = 2;
      break;
    case XVIDC_CSF_MEM_RGB16:
    case XVIDC_CSF_MEM_YUV16:
      /* 6 bytes per pixel (RGB16, YUV16) */
      bpp_numerator = 6;
      break;
    case XVIDC_CSF_MEM_YUYV8:
    case XVIDC_CSF_MEM_UYVY8:
    case XVIDC_CSF_MEM_Y_UV16:
    case XVIDC_CSF_MEM_Y_UV16_420:
    case XVIDC_CSF_MEM_Y16:
      /* 2 bytes per pixel (YUYV8, UYVY8, Y_UV16, Y_UV16_420, Y16) */
      bpp_numerator = 2;
      break;
    default:
      /* 4 bytes per pixel */
      bpp_numerator = 4;
  }
  stride = ((((width * bpp_numerator) / bpp_denominator) +
    MMWidthBytes - 1) / MMWidthBytes) * MMWidthBytes;

  return(stride);
}

/*****************************************************************************/
/**
 * This function configures Frame Buffer for defined mode
 *
 * @return XST_SUCCESS if init is OK else XST_FAILURE
 *
 *****************************************************************************/
static int ConfigFrmbuf(u32 StrideInBytes,
                        XVidC_ColorFormat Cfmt,
                        XVidC_VideoStream *StreamPtr)
{
  int Status;

  /* Stop Frame Buffers */
  XVFrmbufRd_Stop(&frmbufrd);
  XVFrmbufWr_Stop(&frmbufwr);
  resetIp();
  XVFrmbufWr_WaitForIdle(&frmbufwr);
  XVFrmbufRd_WaitForIdle(&frmbufrd);

  /* Configure Frame Buffers */
  Status = XVFrmbufRd_SetMemFormat(&frmbufrd, StrideInBytes, Cfmt, StreamPtr);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Unable to configure Frame Buffer Read\r\n");
    return(XST_FAILURE);
  }

  Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Unable to configure Frame Buffer Read buffer address\r\n");
    return(XST_FAILURE);
  }

  /* Set Chroma Buffer Address for semi-planar color formats */
  if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
      (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420) ||
      (Cfmt == XVIDC_CSF_MEM_Y_UV12) || (Cfmt == XVIDC_CSF_MEM_Y_UV12_420) ||
      (Cfmt == XVIDC_CSF_MEM_Y_UV16) || (Cfmt == XVIDC_CSF_MEM_Y_UV16_420)) {
    Status = XVFrmbufRd_SetChromaBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR+CHROMA_ADDR_OFFSET);
    if(Status != XST_SUCCESS) {
      xil_printf("ERROR:: Unable to configure Frame Buffer Read chroma buffer address\r\n");
      return(XST_FAILURE);
    }
    Status = XVFrmbufWr_SetChromaBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR+CHROMA_ADDR_OFFSET);
    if(Status != XST_SUCCESS) {
      xil_printf("ERROR:: Unable to configure Frame Buffer Write chroma buffer address\r\n");
      return(XST_FAILURE);
    }
  }

  Status = XVFrmbufWr_SetMemFormat(&frmbufwr, StrideInBytes, Cfmt, StreamPtr);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Unable to configure Frame Buffer Write\r\n");
    return(XST_FAILURE);
  }

  Status = XVFrmbufWr_SetBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR);
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Unable to configure Frame Buffer Write buffer address\r\n");
    return(XST_FAILURE);
  }

  /* Enable Interrupt */
  XVFrmbufRd_InterruptEnable(&frmbufrd, XVFRMBUFRD_IRQ_DONE_MASK);
  XVFrmbufWr_InterruptEnable(&frmbufwr, XVFRMBUFRD_IRQ_DONE_MASK);

  /* Start Frame Buffers */
  XVFrmbufWr_Start(&frmbufwr);
  XVFrmbufRd_Start(&frmbufrd);

  xil_printf("INFO: FRMBUF configured\r\n");
  return(Status);
}

/*****************************************************************************/
/**
 * This function checks if video mode and format are supported by HW
 *
 * @return TRUE if testcase is valid else FALSE
 *
 *****************************************************************************/
static int ValidateTestCase(u16 PixPerClk,
                            XVidC_VideoMode Mode,
                            u16 DataWidth,
                            VideoFormats Format)
{
  int Status = TRUE;
  int valid_mode = TRUE;
  int valid_format = TRUE;


  if ((PixPerClk == 1) && (Mode == XVIDC_VM_UHD_60_P)) {
    xil_printf("Video Mode %s not supported for 1 pixel/clock\r\n", XVidC_GetVideoModeStr(Mode));
    valid_mode = 0;
  } else {
    valid_mode = 1;
  }

  if (DataWidth == 16 && Format.FormatBits <= 16) {
      //all Memory Video Formats supported
      valid_format = TRUE;
  } else if (DataWidth == 12 && Format.FormatBits <= 12) {
      //only 12-bit 10-bit and 8-bit Memory Video Formats supported
      valid_format = TRUE;
  } else if (DataWidth == 10 && Format.FormatBits <= 10) {
      //only 10-bit and 8-bit Memory Video Formats supported
      valid_format = TRUE;
  } else if (DataWidth == 8 && Format.FormatBits == 8) {
      //only 8-bit Memory Video Formats supported
      valid_format = TRUE;
  } else {
      valid_format = FALSE;
      xil_printf("Video Format %s is not supported in hardware\r\n",
                 XVidC_GetColorFormatStr(Format.MemFormat));
  }

  Status = (valid_mode && valid_format);
  return(Status);
}

/*****************************************************************************/
/**
 * This function checks Video Out lock status
 *
 * @return T/F
 *
 *****************************************************************************/
static int CheckVidoutLock(void)
{
  int Status = FALSE;
  int Lock = FALSE;
  u32 Timeout;

  Timeout = VIDEO_MONITOR_LOCK_TIMEOUT;

  usleep(1000000); //wait

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
 * This function checks Video In overflow status
 *
 * @return T/F
 *
 *****************************************************************************/
static int CheckVidinOverflow(void)
{
  int Overflow = FALSE;
  u32 Timeout;

  Timeout = VIDEO_MONITOR_LOCK_TIMEOUT;

  while(!Overflow && Timeout) {
    Overflow |= XVMonitor_DidVideoOverflow(&vmon);
    --Timeout;
  }

  if(Overflow) {
    xil_printf("<ERROR:: Video overflow>\r\n\r\n");
  } else {
    xil_printf("No overflow\r\n");
  }

  return(Overflow);
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

  /* Stop Frame Buffer and wait for IDLE */
  XVFrmbufWr_Stop(&frmbufwr);

  xil_printf("\r\nReset HLS IP \r\n");
  *gpio_hlsIpReset = 0;  //reset IPs
  usleep(1000);          //hold reset line
  *gpio_hlsIpReset = 1;  //release reset
  usleep(1000);          //wait
}


/***************************************************************************
*  This is the main loop of the application
***************************************************************************/
int main(void)
{
  int Status, index, format;
  int valid;
  int stride;
  int FailCount = 0;
  int PassCount = 0;
  int TestCount = 0;
  int Lock = FALSE;
  int Overflow = FALSE;
  XVidC_ColorFormat Cfmt;
  XVidC_VideoTiming const *TimingPtr;

  XVidC_VideoMode TestModes[NUM_TEST_MODES] =
  {
    XVIDC_VM_720_60_P,
    XVIDC_VM_1080_60_P,
    XVIDC_VM_UHD_30_P,
    XVIDC_VM_UHD_60_P
  };

  init_platform();

  xil_printf("Start Frame Buffer Example Design Test\r\n");

  /* Setup Reset line and video lock monitor */
  gpio_hlsIpReset = (u32*)XPAR_HLS_IP_RESET_BASEADDR;

  /* Release reset line */
  *gpio_hlsIpReset = 1;

  /* Initialize IRQ */
  Status = SetupInterrupts();
  if (Status == XST_FAILURE) {
    xil_printf("ERROR:: Interrupt Setup Failed\r\n");
    xil_printf("ERROR:: Test could not be completed\r\n");
    return(1);
  }

  /* Initialize VTC, Frame Buffers, GPIO */
  Status = DriverInit();
  if(Status != XST_SUCCESS) {
    xil_printf("ERROR:: Driver Initialization Failed\r\n");
    xil_printf("ERROR:: Test could not be completed\r\n");
    return(1);
  }

  /* Enable exceptions. */
  Xil_ExceptionEnable();

  /* Setup a default stream */
  VidStream.PixPerClk  = frmbufwr.FrmbufWr.Config.PixPerClk;
  VidStream.ColorDepth = frmbufwr.FrmbufWr.Config.MaxDataWidth;

  resetIp();

  /* Sanity check */
  if(XVMonitor_IsVideoLocked(&vmon)) {
    xil_printf("ERROR:: Video should not be locked\r\n");
    xil_printf("ERROR:: Test could not be completed\r\n");
    return(1);
  }

  for (format=0; format<NUM_TEST_FORMATS; format++)
  {
    /* Get video format to test */
    Cfmt = ColorFormats[format].MemFormat;
    VidStream.ColorFormatId = ColorFormats[format].StreamFormat;

    for(index=0; index<NUM_TEST_MODES; ++index)
    {
      /* Get mode to test */
      VidStream.VmId = TestModes[index];

      /* Validate testcase format and mode */
      valid = ValidateTestCase(frmbufwr.FrmbufWr.Config.PixPerClk,
                               TestModes[index],
                               frmbufwr.FrmbufWr.Config.MaxDataWidth,
                               ColorFormats[format]);

      if (valid)
      {
          ++TestCount;

        /* Get mode timing parameters */
        TimingPtr = XVidC_GetTimingInfo(VidStream.VmId);
        VidStream.Timing = *TimingPtr;
        VidStream.FrameRate = XVidC_GetFrameRate(VidStream.VmId);

        xil_printf("\r\n********************************************\r\n");
        xil_printf("Test Input Stream: %s (%s)\r\n",
                   XVidC_GetVideoModeStr(VidStream.VmId),
                   XVidC_GetColorFormatStr(Cfmt));
        xil_printf("********************************************\r\n");

        /* Configure VTC */
        ConfigVtc(&VidStream);

        /* Configure Frame Buffer */
        stride = CalcStride(Cfmt,
                            frmbufwr.FrmbufWr.Config.AXIMMDataWidth,
                            &VidStream);

        ConfigFrmbuf(stride, Cfmt, &VidStream);

        xil_printf("Wait for vid out lock: ");
        Lock = CheckVidoutLock();
        Overflow = CheckVidinOverflow();
        if(Lock && !Overflow) {
          ++PassCount;
        } else {
          ++FailCount;
        }

        resetIp();
        if(XVMonitor_IsVideoLocked(&vmon)) {
          xil_printf("ERROR:: Video should not be locked\r\n");
          xil_printf("ERROR:: Test could not be completed\r\n");
          return(1);
        } else {
          xil_printf("INFO:: Video unlocked\r\n");
        }
      }
    }
  }

  if(FailCount) {
    xil_printf("\r\n\r\nINFO: Test completed. %d/%d tests failed.\r\n",
               FailCount, TestCount);
  } else if (PassCount > 0){
    xil_printf("\r\n\r\nINFO: Test completed successfully. %d/%d tests passed.\r\n",
               PassCount, TestCount);
  } else {
    xil_printf("\r\n\r\nINFO: No tests ran.\r\n");
  }

  return(0);
}
