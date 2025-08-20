/*******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_frmbufwr_example.c
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
* 4.50  kp    12/07/21   Added new 3 planar video format Y_U_V8
* 4.60  kp    12/03/21   Added new 3 planar video format Y_U_V10
* </pre>
*
******************************************************************************/

#include "xparameters.h"
#include "platform.h"
#include "sleep.h"
#ifndef SDT
#if defined(__MICROBLAZE__) || defined(__riscv)
#include "xintc.h"
#else
#include "xscugic.h"
#endif
#else
#include "xinterrupt_wrap.h"
#endif
#include "xv_frmbufrd_l2.h"
#include "xv_frmbufwr_l2.h"
#include "xvidc.h"
#include "xvtc.h"
#include "xgpio.h"

#if defined(__MICROBLAZE__) || defined(__riscv)
#ifndef  SDT
#define DDR_BASEADDR XPAR_MIG7SERIES_0_BASEADDR
#else
#define DDR_BASEADDR XPAR_MIG_0_BASEADDRESS
#endif
#else
#define DDR_BASEADDR XPAR_DDR_MEM_BASEADDR
#endif

#define XVFRMBUFRD_BUFFER_BASEADDR (DDR_BASEADDR + (0x20000000))
#define XVFRMBUFWR_BUFFER_BASEADDR (DDR_BASEADDR + (0x21000000))

#define VIDEO_MONITOR_LOCK_TIMEOUT (1000000)
#define FRMBUF_IDLE_TIMEOUT (1000000)

#define NUM_TEST_MODES 4
#define NUM_TEST_FORMATS 29

#define CHROMA_ADDR_OFFSET   (0x01000000U)
#define V_CHROMA_ADDR_OFFSET (0x03000000U)

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
  {XVIDC_CSF_MEM_Y16,        XVIDC_CSF_YONLY, 16},
  {XVIDC_CSF_MEM_Y_U_V8,     XVIDC_CSF_YCRCB_444, 8},
  {XVIDC_CSF_MEM_Y_U_V10,    XVIDC_CSF_YCRCB_444, 10},
  {XVIDC_CSF_MEM_Y_U_V8_420, XVIDC_CSF_YCRCB_420, 8}
};

XV_FrmbufRd_l2     frmbufrd;
XV_frmbufrd_Config frmbufrd_cfg;
XV_FrmbufWr_l2     frmbufwr;
XV_frmbufwr_Config frmbufwr_cfg;
XVtc       vtc;
#if defined(__MICROBLAZE__) || defined(__riscv)
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

#ifndef SDT
/**
 * @brief Sets up the interrupt controller and connects interrupt handlers for frame buffer read and write modules.
 *
 * This function initializes the interrupt controller and connects the interrupt service routines (ISRs)
 * for the frame buffer read (FRMBUF RD) and frame buffer write (FRMBUF WR) hardware modules. It supports
 * both MicroBlaze and ARM Cortex-A9/A53 (Zynq/ZynqMP) architectures by using the appropriate interrupt
 * controller APIs (XIntc for MicroBlaze, XScuGic for ARM).
 *
 * For MicroBlaze:
 *   - Initializes the XIntc interrupt controller.
 *   - Connects the FRMBUF RD and FRMBUF WR ISRs.
 *   - Enables the corresponding interrupt vectors.
 *   - Starts the interrupt controller in real mode.
 *
 * For ARM (Zynq/ZynqMP):
 *   - Looks up and initializes the XScuGic interrupt controller.
 *   - Connects the FRMBUF RD and FRMBUF WR ISRs.
 *   - Registers the interrupt handler with the exception table.
 *   - Enables the corresponding interrupt vectors.
 *
 * @return
 *   - XST_SUCCESS if all steps complete successfully.
 *   - XST_FAILURE or XST_DEVICE_NOT_FOUND if any step fails.
 */
static int SetupInterrupts(void)
{
#if defined(__MICROBLAZE__) || defined(__riscv)
  int Status;
  XIntc *IntcPtr = &intc;

  /* Initialize the Interrupt controller */
  Status = XIntc_Initialize(IntcPtr,
                            XPAR_PROCESSOR_SS_PROCESSOR_AXI_INTC_DEVICE_ID);
  if (Status != XST_SUCCESS) {
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
  IntcCfgPtr = XScuGic_LookupConfig(XPAR_SCUGIC_0_DEVICE_ID);
  if (IntcCfgPtr == NULL)
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
#endif

/**
 * @brief Initializes the video processing hardware drivers.
 *
 * This function performs the initialization of the following hardware components:
 *   - Video Timing Controller (VTC)
 *   - Frame Buffer Read (frmbufrd)
 *   - Frame Buffer Write (frmbufwr)
 *   - Video Lock Monitor GPIO (vmon)
 *
 * The function uses conditional compilation to select between device ID and base address
 * depending on whether SDT is defined. It checks for configuration lookup failures and
 * initialization errors for each component, printing error messages via xil_printf and
 * returning XST_FAILURE on failure.
 *
 * @return
 *   - XST_SUCCESS if all components are initialized successfully.
 *   - XST_FAILURE if any component fails to initialize.
 */
static int DriverInit(void)
{
  int Status;
  XVtc_Config *vtc_Config;
  XGpio_Config *GpioCfgPtr;

#ifndef SDT
  vtc_Config = XVtc_LookupConfig(XPAR_V_TC_0_DEVICE_ID);
#else
  vtc_Config = XVtc_LookupConfig(XPAR_V_TC_0_BASEADDR);
#endif
  if (vtc_Config == NULL) {
    xil_printf("ERROR:: VTC device not found\r\n");
    return(XST_FAILURE);
  }

  Status = XVtc_CfgInitialize(&vtc, vtc_Config, vtc_Config->BaseAddress);
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: VTC Initialization failed %d\r\n", Status);
    return(XST_FAILURE);
  }

#ifndef SDT
  Status = XVFrmbufRd_Initialize(&frmbufrd, XPAR_V_FRMBUF_RD_0_DEVICE_ID);
#else
  Status = XVFrmbufRd_Initialize(&frmbufrd, XPAR_V_FRMBUF_RD_0_BASEADDR);
#endif
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: Frame Buffer Read initialization failed\r\n");
    return(XST_FAILURE);
  }

#ifndef SDT
  Status = XVFrmbufWr_Initialize(&frmbufwr, XPAR_V_FRMBUF_WR_0_DEVICE_ID);
#else
  Status = XVFrmbufWr_Initialize(&frmbufwr, XPAR_V_FRMBUF_WR_0_BASEADDR);
#endif
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: Frame Buffer Write initialization failed\r\n");
    return(XST_FAILURE);
  }

  //Video Lock Monitor
#ifndef SDT
  GpioCfgPtr = XGpio_LookupConfig(XPAR_VIDEO_LOCK_MONITOR_DEVICE_ID);
#else
  GpioCfgPtr = XGpio_LookupConfig(XPAR_VIDEO_LOCK_MONITOR_BASEADDR);
#endif
  if (GpioCfgPtr == NULL) {
    xil_printf("ERROR:: Video Lock Monitor GPIO device not found\r\n");
    return(XST_FAILURE);
  }

  Status = XGpio_CfgInitialize(&vmon,
                               GpioCfgPtr,
                               GpioCfgPtr->BaseAddress);
  if (Status != XST_SUCCESS)  {
    xil_printf("ERROR:: Video Lock Monitor GPIO Initialization failed %d\r\n",
               Status);
    return(XST_FAILURE);
  }

  return(XST_SUCCESS);
}

/**
 * @brief Configures the Video Timing Controller (VTC) based on the provided video stream parameters.
 *
 * This function initializes and sets up the VTC timing structure using the timing
 * information from the given XVidC_VideoStream pointer. It divides the horizontal
 * timing parameters by the number of pixels per clock to match the VTC's requirements,
 * copies the vertical timing parameters, and applies the configuration to the VTC hardware.
 * After configuration, it enables the VTC, its generator, and register updates.
 *
 * @param StreamPtr Pointer to an XVidC_VideoStream structure containing the video timing parameters.
 *
 * @return None.
 */
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

/**
 * CalcStride - Calculate the stride (number of bytes per video line) for a given color format.
 *
 * This function computes the stride required for a video frame buffer line based on the color format,
 * AXI memory-mapped data width, and the video stream's active width. The stride is aligned to the
 * AXI memory-mapped data width in bytes.
 *
 * @param  Cfmt           The color format of the video stream (XVidC_ColorFormat).
 * @param  AXIMMDataWidth The AXI memory-mapped data width in bits.
 * @param  StreamPtr      Pointer to the XVidC_VideoStream structure containing video timing information.
 *
 * @return The stride in bytes, aligned to the AXI memory-mapped data width.
 *
 * Color format to bytes-per-pixel mapping:
 *   - Y_UV10, Y_UV10_420, Y10:         4 bytes per 3 pixels
 *   - Y_UV8, Y_UV8_420, Y8, Y_U_V8, Y_U_V10: 1 byte per pixel
 *   - RGB8, YUV8, BGR8:                3 bytes per pixel
 *   - RGBX12, YUVX12:                  5 bytes per pixel
 *   - Y_UV12, Y_UV12_420, Y12:         3 bytes per 2 pixels
 *   - RGB16, YUV16:                    6 bytes per pixel
 *   - YUYV8, UYVY8, Y_UV16, Y_UV16_420, Y16: 2 bytes per pixel
 *   - Default:                         4 bytes per pixel
 *
 * The stride is calculated as:
 *   stride = ceil((width * bytes_per_pixel) / MMWidthBytes) * MMWidthBytes
 * where MMWidthBytes = AXIMMDataWidth / 8.
 */
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
    case  XVIDC_CSF_MEM_Y_U_V8:
	case  XVIDC_CSF_MEM_Y_U_V10:
      /* 1 byte per pixel (Y_UV8, Y_UV8_420, Y8, Y_U_V10) */
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

/**
 * @brief Configures the Frame Buffer Read and Write IP cores with the specified parameters.
 *
 * This function stops the frame buffer IPs, resets them, waits for them to become idle,
 * and then configures their memory formats and buffer addresses according to the provided
 * stride, color format, and video stream. It also handles chroma and V buffer addresses
 * for semi-planar and 3-planar color formats, respectively. Interrupts are enabled and
 * the frame buffers are started after configuration.
 *
 * @param StrideInBytes   The stride (in bytes) for the frame buffer memory.
 * @param Cfmt            The color format (XVidC_ColorFormat) to be used for the frame buffer.
 * @param StreamPtr       Pointer to the video stream configuration (XVidC_VideoStream).
 *
 * @return
 *   - XST_SUCCESS if configuration is successful.
 *   - XST_FAILURE if any configuration step fails.
 *
 * @note
 *   - This function assumes that the global variables 'frmbufrd' and 'frmbufwr' are
 *     properly initialized.
 *   - The function also assumes that the macros XVFRMBUFRD_BUFFER_BASEADDR,
 *     XVFRMBUFWR_BUFFER_BASEADDR, CHROMA_ADDR_OFFSET, and V_CHROMA_ADDR_OFFSET are defined.
 *   - The function prints error messages via xil_printf if any step fails.
 */
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
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: Unable to configure Frame Buffer Read\r\n");
    return(XST_FAILURE);
  }

  Status = XVFrmbufWr_SetMemFormat(&frmbufwr, StrideInBytes, Cfmt, StreamPtr);
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: Unable to configure Frame Buffer Write\r\n");
    return(XST_FAILURE);
  }

  Status = XVFrmbufRd_SetBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR);
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: Unable to configure Frame Buffer Read buffer address\r\n");
    return(XST_FAILURE);
  }

  Status = XVFrmbufWr_SetBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR);
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: Unable to configure Frame Buffer Write buffer address\r\n");
    return(XST_FAILURE);
  }

  /* Set Chroma Buffer Address for semi-planar color formats */
  if ((Cfmt == XVIDC_CSF_MEM_Y_UV8) || (Cfmt == XVIDC_CSF_MEM_Y_UV8_420) ||
      (Cfmt == XVIDC_CSF_MEM_Y_UV10) || (Cfmt == XVIDC_CSF_MEM_Y_UV10_420) ||
      (Cfmt == XVIDC_CSF_MEM_Y_UV12) || (Cfmt == XVIDC_CSF_MEM_Y_UV12_420) ||
      (Cfmt == XVIDC_CSF_MEM_Y_UV16) || (Cfmt == XVIDC_CSF_MEM_Y_UV16_420) ||
      (Cfmt == XVIDC_CSF_MEM_Y_U_V8) || (Cfmt == XVIDC_CSF_MEM_Y_U_V10) ||
      (Cfmt == XVIDC_CSF_MEM_Y_U_V8_420)) {
	  Status = XVFrmbufRd_SetChromaBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR+CHROMA_ADDR_OFFSET);
	  if (Status != XST_SUCCESS) {
		  xil_printf("ERROR:: Unable to configure Frame Buffer Read chroma buffer address\r\n");
		  return(XST_FAILURE);
	  }
	  Status = XVFrmbufWr_SetChromaBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR+CHROMA_ADDR_OFFSET);
	  if (Status != XST_SUCCESS) {
		  xil_printf("ERROR:: Unable to configure Frame Buffer Write chroma buffer address\r\n");
		  return(XST_FAILURE);
	  }
  }

  /* Set V Buffer Address for 3 planar color formats */
  if ((Cfmt == XVIDC_CSF_MEM_Y_U_V8) || (Cfmt == XVIDC_CSF_MEM_Y_U_V10) || (Cfmt == XVIDC_CSF_MEM_Y_U_V8_420)) {
	  Status = XVFrmbufRd_SetVChromaBufferAddr(&frmbufrd, XVFRMBUFRD_BUFFER_BASEADDR+V_CHROMA_ADDR_OFFSET);
	  if (Status != XST_SUCCESS) {
		  xil_printf("ERROR:: Unable to configure Frame Buffer Read V buffer address\r\n");
		  return(XST_FAILURE);
	  }
	  Status = XVFrmbufWr_SetVChromaBufferAddr(&frmbufwr, XVFRMBUFWR_BUFFER_BASEADDR+V_CHROMA_ADDR_OFFSET);
	  if (Status != XST_SUCCESS) {
		  xil_printf("ERROR:: Unable to configure Frame Buffer Write V buffer address\r\n");
		  return(XST_FAILURE);
	  }
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

/**
 * ValidateTestCase - Validates the combination of pixel-per-clock, video mode,
 *                    data width, and video format for hardware support.
 *
 * @param PixPerClk:   Number of pixels processed per clock cycle.
 * @param Mode:        Video mode (resolution, refresh rate, etc.) as defined by XVidC_VideoMode.
 * @param DataWidth:   Data width in bits (e.g., 8, 10, 12, 16).
 * @param Format:      Video format structure containing format information, including bit depth.
 *
 * @return
 *   TRUE  - If the specified combination of parameters is supported by the hardware.
 *   FALSE - If the combination is not supported (with diagnostic messages printed).
 *
 * The function checks:
 *   - If the selected video mode is supported for the given pixel-per-clock value.
 *   - If the data width is compatible with the memory video format's bit depth.
 *   - Prints diagnostic messages if an unsupported combination is detected.
 */
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

/**
 * @brief Checks if the video output is locked within a specified timeout period.
 *
 * This function waits for a short period, then repeatedly checks if the video output
 * is locked by calling XVMonitor_IsVideoLocked(). If the lock is detected within the
 * timeout period, it prints "Locked" and returns TRUE. If the timeout expires without
 * detecting a lock, it prints an error message and returns FALSE.
 *
 * @return
 *   TRUE if the video output is locked within the timeout period, FALSE otherwise.
 */
static int CheckVidoutLock(void)
{
  int Status = FALSE;
  int Lock = FALSE;
  u32 Timeout;

  Timeout = VIDEO_MONITOR_LOCK_TIMEOUT;

  usleep(1000000); //wait

  while(!Lock && Timeout) {
    if (XVMonitor_IsVideoLocked(&vmon)) {
      xil_printf("Locked\r\n");
      Lock = TRUE;
      Status = TRUE;
    }
    --Timeout;
  }

  if (!Timeout) {
      xil_printf("<ERROR:: Not Locked>\r\n\r\n");
  }
  return(Status);
}

/**
 * @brief Callback function for Frame Buffer Read interrupt.
 *
 * This function is called when a Frame Buffer Read interrupt is received.
 * It starts the frame buffer read operation by calling XVFrmbufRd_Start().
 *
 * @param data Pointer to user data (unused).
 * @return Always returns NULL.
 */
void *XVFrameBufferRdCallback(void *data)
{
	//xil_printf("\nFrame Buffer Read interrupt received.\r\n");
	  XVFrmbufRd_Start(&frmbufrd);
}

/**
 * @brief Callback function for Frame Buffer Write interrupt.
 *
 * This function is called when a Frame Buffer Write interrupt is received.
 * It starts the frame buffer write operation by invoking XVFrmbufWr_Start().
 *
 * @param data Pointer to user data (unused in this implementation).
 * @return Always returns NULL.
 */
void *XVFrameBufferWrCallback(void *data)
{
	//xil_printf("\nFrame Buffer Read interrupt received.\r\n");
	  XVFrmbufWr_Start(&frmbufwr);
}

/**
 * @brief Checks for video input overflow condition.
 *
 * This function monitors the video input for an overflow condition by polling
 * the video monitor for a specified timeout period. If an overflow is detected
 * within the timeout, an error message is printed; otherwise, a message
 * indicating no overflow is printed.
 *
 * @return
 *   - TRUE if a video overflow was detected.
 *   - FALSE if no overflow was detected within the timeout period.
 */
static int CheckVidinOverflow(void)
{
  int Overflow = FALSE;
  u32 Timeout;

  Timeout = VIDEO_MONITOR_LOCK_TIMEOUT;

  while(!Overflow && Timeout) {
    Overflow |= XVMonitor_DidVideoOverflow(&vmon);
    --Timeout;
  }

  if (Overflow) {
    xil_printf("<ERROR:: Video overflow>\r\n\r\n");
  } else {
    xil_printf("No overflow\r\n");
  }

  return(Overflow);
}

/**
 * @brief Resets the HLS (High-Level Synthesis) IP core and stops the frame buffer write operation.
 *
 * This function performs the following steps:
 * 1. Stops the frame buffer write core and waits for it to become idle.
 * 2. Prints a message indicating that the HLS IP is being reset.
 * 3. Asserts the reset line for the HLS IP by writing 0 to the reset GPIO.
 * 4. Waits for 1 millisecond to ensure the reset is asserted.
 * 5. Deasserts the reset line by writing 1 to the reset GPIO.
 * 6. Waits for another millisecond to allow the IP to come out of reset.
 *
 * This ensures a proper reset sequence for the HLS IP and the frame buffer write core.
 */
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

/**
 * @brief Main application entry point for the Frame Buffer Example Design Test.
 *
 * This function initializes the platform, configures hardware drivers (VTC, frame buffers, GPIO),
 * sets up interrupts, and iterates through all supported video formats and test video modes.
 * For each valid combination, it configures the VTC and frame buffer, checks for video lock and overflow,
 * and tracks pass/fail results. At the end, it prints a summary of the test results.
 *
 * @return 0 on success, 1 on failure.
 */
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

  /* Initialize VTC, Frame Buffers, GPIO */
  Status = DriverInit();
  if (Status != XST_SUCCESS) {
    xil_printf("ERROR:: Driver Initialization Failed\r\n");
    xil_printf("ERROR:: Test could not be completed\r\n");
    return(1);
  }

  /* Initialize IRQ */
#ifndef SDT
  Status = SetupInterrupts();
  if (Status == XST_FAILURE) {
    xil_printf("ERROR:: Interrupt Setup Failed\r\n");
    xil_printf("ERROR:: Test could not be completed\r\n");
    return(1);
  }
#else
  Status = XSetupInterruptSystem(&frmbufwr,&XVFrmbufWr_InterruptHandler,
				       frmbufwr.FrmbufWr.Config.IntrId,
				       frmbufwr.FrmbufWr.Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
  if (Status == XST_FAILURE) {
    xil_printf("ERROR:: frmbufwr Interrupt Setup Failed\r\n");
    xil_printf("ERROR:: Test could not be completed\r\n");
    return(1);
  }
Status = XSetupInterruptSystem(&frmbufrd,&XVFrmbufRd_InterruptHandler,
				       frmbufrd.FrmbufRd.Config.IntrId,
				       frmbufrd.FrmbufRd.Config.IntrParent,
				       XINTERRUPT_DEFAULT_PRIORITY);
  if (Status == XST_FAILURE) {
    xil_printf("ERROR:: frmbufrd Interrupt Setup Failed\r\n");
    xil_printf("ERROR:: Test could not be completed\r\n");
    return(1);
  }
#endif

  /* Enable exceptions. */
  Xil_ExceptionEnable();

  XVFrmbufRd_SetCallback(&frmbufrd, XVFRMBUFRD_HANDLER_DONE, XVFrameBufferRdCallback,
		(void *)&frmbufrd);

  XVFrmbufWr_SetCallback(&frmbufwr, XVFRMBUFWR_HANDLER_DONE, XVFrameBufferWrCallback,
		(void *)&frmbufwr);

  /* Setup a default stream */
  VidStream.PixPerClk  = frmbufwr.FrmbufWr.Config.PixPerClk;
  VidStream.ColorDepth = frmbufwr.FrmbufWr.Config.MaxDataWidth;

  resetIp();

  /* Sanity check */
  if (XVMonitor_IsVideoLocked(&vmon)) {
    xil_printf("ERROR:: Video should not be locked\r\n");
    xil_printf("ERROR:: Test could not be completed\r\n");
    return(1);
  }

  for (format=0; format<NUM_TEST_FORMATS; format++)
  {
    /* Get video format to test */
    Cfmt = ColorFormats[format].MemFormat;
    VidStream.ColorFormatId = ColorFormats[format].StreamFormat;

    for (index=0; index<NUM_TEST_MODES; ++index)
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
        if (Lock && !Overflow) {
          ++PassCount;
        } else {
          ++FailCount;
        }

        resetIp();
        if (XVMonitor_IsVideoLocked(&vmon)) {
          xil_printf("ERROR:: Video should not be locked\r\n");
          xil_printf("ERROR:: Test could not be completed\r\n");
          return(1);
        } else {
          xil_printf("INFO:: Video unlocked\r\n");
        }
      }
    }
  }

  if (FailCount) {
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
