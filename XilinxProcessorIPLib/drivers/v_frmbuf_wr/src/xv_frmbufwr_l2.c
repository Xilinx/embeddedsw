/******************************************************************************
* Copyright (C) 2017-2022 Xilinx Inc. All rights reserved.
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_frmbufwr_l2.c
* @addtogroup v_frmbuf_wr Overview
* @{
*
* Frame Buffer Write Layer-2 Driver. The functions in this file provides an
* abstraction from the register peek/poke methodology by implementing most
* common use-case provided by the sub-core. See xv_frmbufwr_l2.h for a detailed
* description of the layer-2 driver
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vyc   04/05/17   Initial Release
* 2.00  vyc   10/04/17   Add second buffer pointer for semi-planar formats
*                        Add new memory formats BGRX8 and UYVY8
* 3.00  vyc   04/04/18   Add interlaced support
*                        Add new memory format BGR8
*                        Add interrupt handler for ap_ready
* 4.00  pv    11/10/18   Added flushing feature support in driver.
*			 flush bit should be set and held (until reset) by
*			 software to flush pending transactions.IP is expecting
*			 a hard reset, when flushing is done.(There is a flush
*			 status bit and is asserted when the flush is done).
* 4.10  vv    02/05/19   Added new pixel formats with 12 and 16 bpc.
* 4.50  kp    12/07/21   Added new 3 planar video format Y_U_V8.
* 4.60  kp    10/27/21   Added new 3 planar video format Y_U_V10.
* 4.70  pg    05/23/23   Added new 3 planar video format Y_U_V8_420.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "sleep.h"
#include "xv_frmbufwr_l2.h"

/************************** Constant Definitions *****************************/
#define XVFRMBUFWR_MIN_STRM_WIDTH            (64u)
#define XVFRMBUFWR_MIN_STRM_HEIGHT           (64u)
#define XVFRMBUFWR_IDLE_TIMEOUT              (1000000)
#define XV_WAIT_FOR_FLUSH_DONE		         (25)
#define XV_WAIT_FOR_FLUSH_DONE_TIMEOUT		 (2000)
/************************** Function Prototypes ******************************/
static void SetPowerOnDefaultState(XV_FrmbufWr_l2 *InstancePtr);
XVidC_ColorFormat WrMemory2Live(XVidC_ColorFormat MemFmt);

/*****************************************************************************/
/**
* This function maps the memory video formats to the live/stream video formats
*
* @param  MemFmt is the video format written to memory
*
* @return Live/Stream Video Format associated with that memory format
*
******************************************************************************/
XVidC_ColorFormat WrMemory2Live(XVidC_ColorFormat MemFmt)
{
	XVidC_ColorFormat StrmFmt;

	switch(MemFmt) {
		case XVIDC_CSF_MEM_RGBX8 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_YUVX8 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_YUYV8 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_RGBA8 :
			StrmFmt = XVIDC_CSF_RGBA;
			break;
		case XVIDC_CSF_MEM_YUVA8 :
			StrmFmt = XVIDC_CSF_YCRCBA_444;
			break;
		case XVIDC_CSF_MEM_BGRA8 :
			StrmFmt = XVIDC_CSF_RGBA;
			break;
		case XVIDC_CSF_MEM_RGBX10 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_YUVX10 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV8 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV8_420 :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_RGB8 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_YUV8 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV10 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV10_420 :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_Y8 :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
		case XVIDC_CSF_MEM_Y10 :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
		case XVIDC_CSF_MEM_BGRX8 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_UYVY8 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_BGR8 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_RGBX12 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_YUVX12 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV12 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV12_420 :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_Y12 :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
		case XVIDC_CSF_MEM_RGB16 :
			StrmFmt = XVIDC_CSF_RGB;
			break;
		case XVIDC_CSF_MEM_YUV16 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV16 :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV16_420 :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_Y16 :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
		case XVIDC_CSF_MEM_Y_U_V8 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_U_V10 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_U_V8_420 :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_Y_U_V12 :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_U_V10_L16LE :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV10_L16LE :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV10_420_L16LE :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_Y10_L16LE :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
		case XVIDC_CSF_MEM_Y_U_V12_L16LE :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV12_L16LE :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV12_420_L16LE :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_Y12_L16LE :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
		case XVIDC_CSF_MEM_Y_U_V10_M16LE :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV10_M16LE :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV10_420_M16LE :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_Y10_M16LE :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
		case XVIDC_CSF_MEM_Y_U_V12_M16LE :
			StrmFmt = XVIDC_CSF_YCRCB_444;
			break;
		case XVIDC_CSF_MEM_Y_UV12_M16LE :
			StrmFmt = XVIDC_CSF_YCRCB_422;
			break;
		case XVIDC_CSF_MEM_Y_UV12_420_M16LE :
			StrmFmt = XVIDC_CSF_YCRCB_420;
			break;
		case XVIDC_CSF_MEM_Y12_M16LE :
			StrmFmt = XVIDC_CSF_YONLY;
			break;
		default:
			StrmFmt = (XVidC_ColorFormat)~0;
			break;
	}
	return(StrmFmt);
}

/**
 * XVFrmbufWr_Initialize - Initializes the frame buffer write instance.
 *
 * This function initializes an XV_FrmbufWr_l2 instance, setting up its internal
 * state and configuring the underlying hardware or driver instance. It clears
 * the instance memory, initializes the lower-level frame buffer write driver,
 * and sets the power-on default state if initialization is successful.
 *
 * @param InstancePtr Pointer to the XV_FrmbufWr_l2 instance to be initialized.
 * @param DeviceId    Device ID of the frame buffer write core (used in non-SDT builds).
 *
 * @return
 *   - XST_SUCCESS if initialization was successful.
 *   - Error code otherwise.
 */
#ifndef SDT
int XVFrmbufWr_Initialize(XV_FrmbufWr_l2 *InstancePtr, u16 DeviceId)
#else
int XVFrmbufWr_Initialize(XV_FrmbufWr_l2 *InstancePtr, UINTPTR BaseAddress)
#endif
{
  int Status;
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Setup the instance */
  memset(InstancePtr, 0, sizeof(XV_FrmbufWr_l2));
#ifndef SDT
  Status = XV_frmbufwr_Initialize(&InstancePtr->FrmbufWr, DeviceId);
#else
  Status = XV_frmbufwr_Initialize(&InstancePtr->FrmbufWr, BaseAddress);
#endif

  if (Status == XST_SUCCESS) {
    SetPowerOnDefaultState(InstancePtr);
  }
  return(Status);
}

/**
 * @brief Sets the XV_FrmbufWr_l2 instance to its power-on default state.
 *
 * This function initializes the video stream parameters to default values,
 * including resolution (1920x1080 @ 60Hz, progressive), color format (RGB),
 * frame rate (60Hz), color depth, and pixels per clock based on the instance
 * configuration. It also sets the hardware registers for frame width, height,
 * stride, and memory video format. The function disables DONE and READY
 * interrupts to configure the instance in polling mode.
 *
 * @param InstancePtr Pointer to the XV_FrmbufWr_l2 instance to initialize.
 *
 * @return None
 */
static void SetPowerOnDefaultState(XV_FrmbufWr_l2 *InstancePtr)
{
  XVidC_VideoStream VidStrm;
  XVidC_VideoTiming const *ResTiming;

  u32 IrqMask = 0;

  memset(&VidStrm, 0, sizeof(XVidC_VideoStream));

  /* Set Default Stream In */
  VidStrm.VmId          = XVIDC_VM_1920x1080_60_P;
  VidStrm.ColorFormatId = XVIDC_CSF_RGB;
  VidStrm.FrameRate     = XVIDC_FR_60HZ;
  VidStrm.IsInterlaced  = FALSE;
  VidStrm.ColorDepth    = (XVidC_ColorDepth)InstancePtr->FrmbufWr.Config.MaxDataWidth;
  VidStrm.PixPerClk     = (XVidC_PixelsPerClock)InstancePtr->FrmbufWr.Config.PixPerClk;

  ResTiming = XVidC_GetTimingInfo(VidStrm.VmId);

  VidStrm.Timing = *ResTiming;

  /* Set frame width, height, stride and memory video format */
  XV_frmbufwr_Set_HwReg_width(&InstancePtr->FrmbufWr,  VidStrm.Timing.HActive);
  XV_frmbufwr_Set_HwReg_height(&InstancePtr->FrmbufWr, VidStrm.Timing.VActive);
  XV_frmbufwr_Set_HwReg_stride(&InstancePtr->FrmbufWr, 7680);
  XV_frmbufwr_Set_HwReg_video_format(&InstancePtr->FrmbufWr, XVIDC_CSF_MEM_RGBX8);
  InstancePtr->Stream = VidStrm;

  /* Setup polling mode */
  IrqMask = XVFRMBUFWR_IRQ_DONE_MASK | XVFRMBUFWR_IRQ_READY_MASK;
  XVFrmbufWr_InterruptDisable(InstancePtr, IrqMask);
}

/**
 * Enables specific interrupts for the Frame Buffer Write core and globally enables interrupts.
 * Also disables the auto-restart feature of the Frame Buffer Write core.
 *
 * @param InstancePtr Pointer to the XV_FrmbufWr_l2 instance.
 * @param IrqMask     Bitmask specifying which interrupts to enable.
 *
 * This function asserts that the provided instance pointer is not NULL,
 * enables the specified interrupts, globally enables interrupts for the core,
 * and clears the auto-restart bit to prevent automatic restarts.
 */
void XVFrmbufWr_InterruptEnable(XV_FrmbufWr_l2 *InstancePtr, u32 IrqMask)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Enable Interrupts */
  XV_frmbufwr_InterruptEnable(&InstancePtr->FrmbufWr, IrqMask);
  XV_frmbufwr_InterruptGlobalEnable(&InstancePtr->FrmbufWr);

  /* Clear autostart bit */
  XV_frmbufwr_DisableAutoRestart(&InstancePtr->FrmbufWr);
}

/**
 * Disable specific interrupts for the Frame Buffer Write core and enable auto-restart.
 *
 * This function disables the interrupts specified by the IrqMask for the Frame Buffer Write
 * instance pointed to by InstancePtr. It also globally disables all interrupts for the core,
 * and sets the auto-restart bit to enable automatic restarting of the core after completion.
 *
 * @param InstancePtr Pointer to the XV_FrmbufWr_l2 instance.
 * @param IrqMask is the interrupt mask the driver interrupt handler
 * 	    	passes to the callback function.
 *
 * @note The function asserts that InstancePtr is not NULL.
 */
void XVFrmbufWr_InterruptDisable(XV_FrmbufWr_l2 *InstancePtr, u32 IrqMask)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Disable Interrupts */
  XV_frmbufwr_InterruptDisable(&InstancePtr->FrmbufWr, IrqMask);
  XV_frmbufwr_InterruptGlobalDisable(&InstancePtr->FrmbufWr);

  /* Set autostart bit */
  XV_frmbufwr_EnableAutoRestart(&InstancePtr->FrmbufWr);
}

/**
 * @brief Starts the Frame Buffer Write core.
 *
 * This function initiates the operation of the Frame Buffer Write (FrmbufWr) core
 * for the given instance. It asserts that the provided instance pointer is not NULL,
 * and then calls the lower-level start function for the hardware core.
 *
 * @param InstancePtr Pointer to the XV_FrmbufWr_l2 instance to be started.
 */
void XVFrmbufWr_Start(XV_FrmbufWr_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_frmbufwr_Start(&InstancePtr->FrmbufWr);
}

/**
 * Stops the Frame Buffer Write core.
 *
 * This function disables the autostart feature and initiates a flush operation
 * on the Frame Buffer Write core. It waits for the flush operation to complete
 * or until a timeout occurs. If the flush does not complete within the allowed
 * time, the function returns a failure status.
 *
 * @param  InstancePtr  Pointer to the XV_FrmbufWr_l2 instance.
 *
 * @return
 *     - XST_SUCCESS if the flush operation completes successfully.
 *     - XST_FAILURE if the flush operation does not complete within the timeout.
 */
int XVFrmbufWr_Stop(XV_FrmbufWr_l2 *InstancePtr)
{
  int Status = XST_SUCCESS;
  u32 cnt = 0;
  u32 Data = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Clear autostart bit */
  XV_frmbufwr_DisableAutoRestart(&InstancePtr->FrmbufWr);

  /* Flush the core bit */
  XV_frmbufwr_SetFlushbit(&InstancePtr->FrmbufWr);

  do {
    Data = XV_frmbufwr_Get_FlushDone(&InstancePtr->FrmbufWr);
    usleep(XV_WAIT_FOR_FLUSH_DONE_TIMEOUT);
    cnt++;
  } while((Data == 0) && (cnt < XV_WAIT_FOR_FLUSH_DONE));

  if (Data == 0)
        Status = XST_FAILURE;

  return(Status);
}

/**
 * Waits for the Frame Buffer Write core to become idle.
 *
 * This function polls the status of the Frame Buffer Write hardware core
 * until it becomes idle or a timeout occurs. It checks the idle status
 * by calling XV_frmbufwr_IsIdle() in a loop, up to a maximum number of
 * iterations defined by XVFRMBUFWR_IDLE_TIMEOUT.
 *
 * @param  InstancePtr Pointer to the XV_FrmbufWr_l2 instance.
 *
 * @return
 *     - XST_SUCCESS if the core becomes idle within the timeout period.
 *     - XST_FAILURE if the core does not become idle before the timeout.
 */
int XVFrmbufWr_WaitForIdle(XV_FrmbufWr_l2 *InstancePtr)
{
  int Status = XST_FAILURE;
  u32 isIdle = 0;
  u32 cnt = 0;
  /* Wait for idle */
  do {
    isIdle = XV_frmbufwr_IsIdle(&InstancePtr->FrmbufWr);
    cnt++;
  } while((isIdle!=1) && (cnt < XVFRMBUFWR_IDLE_TIMEOUT));

  if (isIdle == 1 ) {
     Status = XST_SUCCESS;
  }

  return Status;
}

/**
 * Configure the memory format and validate parameters for the frame buffer write instance.
 *
 * This function sets up the memory format for the frame buffer writer, validates the input parameters,
 * and configures the hardware registers if all checks pass. It ensures that the video stream and memory
 * format are compatible, the stride is properly aligned, and the selected memory format is supported
 * by the hardware instance.
 *
 * @param InstancePtr    Pointer to the XV_FrmbufWr_l2 instance.
 * @param StrideInBytes  Stride (in bytes) for each line in memory.
 * @param MemFmt         Desired memory color format (XVidC_ColorFormat).
 * @param StrmIn         Pointer to the input video stream structure (XVidC_VideoStream).
 *
 * @return
 *   - XST_SUCCESS if the configuration is successful.
 *   - XVFRMBUFWR_ERR_FRAME_SIZE_INVALID if the frame size is invalid for the selected format.
 *   - XVFRMBUFWR_ERR_STRIDE_MISALIGNED if the stride is not properly aligned.
 *   - XVFRMBUFWR_ERR_VIDEO_FORMAT_MISMATCH if the stream and memory formats do not match.
 *   - XVFRMBUFWR_ERR_DISABLED_IN_HW if the selected memory format is not supported in hardware.
 *
 * @note
 *   - The function asserts that InstancePtr and StrmIn are not NULL.
 *   - The function copies the input stream structure to the instance.
 *   - For 4:2:2 and 4:2:0 formats, width and/or height must be even.
 *   - The stride must be aligned to 2 * PixPerClk * 4 bytes.
 *   - The memory format must be enabled in the hardware configuration.
 */
int XVFrmbufWr_SetMemFormat(XV_FrmbufWr_l2 *InstancePtr,
                            u32 StrideInBytes,
                            XVidC_ColorFormat MemFmt,
                            const XVidC_VideoStream *StrmIn)
{
  int Status = XST_FAILURE;
  u32 FmtValid = FALSE;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(StrmIn != NULL);

  /* copy stream data */
  InstancePtr->Stream = *StrmIn;

  /* Width must be multiple of Samples per Clock */
  Xil_AssertNonvoid((StrmIn->Timing.HActive  % \
                    InstancePtr->FrmbufWr.Config.PixPerClk) == 0);

  /* For 4:2:2 and 4:2:0, columns must be in pairs */
  if (((WrMemory2Live(MemFmt) == XVIDC_CSF_YCRCB_422) || \
       (WrMemory2Live(MemFmt) == XVIDC_CSF_YCRCB_420)) && \
       ((StrmIn->Timing.HActive  % 2) != 0)) {
        Status = XVFRMBUFWR_ERR_FRAME_SIZE_INVALID;

  /* For 4:2:0, rows must be in pairs */
  } else if ((WrMemory2Live(MemFmt) == XVIDC_CSF_YCRCB_420) && \
             ((StrmIn->Timing.VActive  % 2) != 0)) {
        Status = XVFRMBUFWR_ERR_FRAME_SIZE_INVALID;

  /* Check if stride is aligned to aximm width (2*PPC*32-bits) */
  } else if ((StrideInBytes % (2 * InstancePtr->FrmbufWr.Config.PixPerClk * 4)) != 0) {
        Status   = XVFRMBUFWR_ERR_STRIDE_MISALIGNED;

  /* Streaming Video Format must match Memory Video Format */
  } else if (StrmIn->ColorFormatId != WrMemory2Live(MemFmt)) {
        Status = XVFRMBUFWR_ERR_VIDEO_FORMAT_MISMATCH;

  /* Memory Video Format must be supported in hardware */
  } else {
    Status = XVFRMBUFWR_ERR_DISABLED_IN_HW;
    FmtValid = FALSE;
    switch(MemFmt) {
      case XVIDC_CSF_MEM_RGBX8 :
         if (XVFrmbufWr_IsRGBX8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUVX8 :
         if (XVFrmbufWr_IsYUVX8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUYV8 :
         if (XVFrmbufWr_IsYUYV8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_RGBA8 :
         if (XVFrmbufWr_IsRGBA8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUVA8 :
         if (XVFrmbufWr_IsYUVA8Enabled(InstancePtr)) {
		   FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_BGRA8 :
         if (XVFrmbufWr_IsBGRA8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_RGBX10 :
         if (XVFrmbufWr_IsRGBX10Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUVX10 :
         if (XVFrmbufWr_IsYUVX10Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV8 :
         if (XVFrmbufWr_IsY_UV8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV8_420 :
         if (XVFrmbufWr_IsY_UV8_420Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_RGB8 :
         if (XVFrmbufWr_IsRGB8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUV8 :
         if (XVFrmbufWr_IsYUV8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV10 :
         if (XVFrmbufWr_IsY_UV10Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV10_420 :
         if (XVFrmbufWr_IsY_UV10_420Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y8 :
         if (XVFrmbufWr_IsY8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y10 :
         if (XVFrmbufWr_IsY10Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_BGRX8 :
         if (XVFrmbufWr_IsBGRX8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_UYVY8 :
         if (XVFrmbufWr_IsUYVY8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_BGR8 :
         if (XVFrmbufWr_IsBGR8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_RGBX12 :
         if (XVFrmbufWr_IsRGBX12Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_RGB16 :
         if (XVFrmbufWr_IsRGB16Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUVX12 :
         if (XVFrmbufWr_IsYUVX12Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUV16 :
         if (XVFrmbufWr_IsYUV16Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV12 :
         if (XVFrmbufWr_IsY_UV12Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV16 :
         if (XVFrmbufWr_IsY_UV16Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV12_420 :
         if (XVFrmbufWr_IsY_UV12_420Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV16_420 :
         if (XVFrmbufWr_IsY_UV16_420Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y12 :
         if (XVFrmbufWr_IsY12Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y16 :
         if (XVFrmbufWr_IsY16Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_U_V8 :
         if (XVFrmbufWr_IsY_U_V8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_U_V10 :
         if (XVFrmbufWr_IsY_U_V10Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_U_V8_420 :
         if (XVFrmbufWr_IsY_U_V8_420Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_U_V12 :
         if (XVFrmbufWr_IsY_U_V12Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
			case XVIDC_CSF_MEM_Y_U_V10_L16LE :
                if (XVFrmbufWr_IsY_U_V10_L16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV10_L16LE :
                if (XVFrmbufWr_IsY_UV10_L16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV10_420_L16LE :
                if (XVFrmbufWr_IsY_UV10_420_L16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y10_L16LE :
                if (XVFrmbufWr_IsY10_L16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_U_V12_L16LE :
                if (XVFrmbufWr_IsY_U_V12_L16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV12_L16LE :
                if (XVFrmbufWr_IsY_UV12_L16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV12_420_L16LE :
                if (XVFrmbufWr_IsY_UV12_420_L16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y12_L16LE :
                if (XVFrmbufWr_IsY12_L16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_U_V10_M16LE :
                if (XVFrmbufWr_IsY_U_V10_M16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV10_M16LE :
                if (XVFrmbufWr_IsY_UV10_M16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV10_420_M16LE :
                if (XVFrmbufWr_IsY_UV10_420_M16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y10_M16LE :
                if (XVFrmbufWr_IsY10_M16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_U_V12_M16LE :
                if (XVFrmbufWr_IsY_U_V12_M16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV12_M16LE :
                if (XVFrmbufWr_IsY_UV12_M16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y_UV12_420_M16LE :
                if (XVFrmbufWr_IsY_UV12_420_M16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
			case XVIDC_CSF_MEM_Y12_M16LE :
                if (XVFrmbufWr_IsY12_M16LEEnabled(InstancePtr)) {
				   FmtValid = TRUE;
				}
				break;
      default :
         FmtValid = FALSE;
         break;
    }

      if (FmtValid == TRUE) {
        /* configure frame buffer write */
        XV_frmbufwr_Set_HwReg_width(&InstancePtr->FrmbufWr,
                                    StrmIn->Timing.HActive);
        XV_frmbufwr_Set_HwReg_height(&InstancePtr->FrmbufWr,
                                     StrmIn->Timing.VActive);
        XV_frmbufwr_Set_HwReg_stride(&InstancePtr->FrmbufWr, StrideInBytes);
        XV_frmbufwr_Set_HwReg_video_format(&InstancePtr->FrmbufWr, MemFmt);
        Status = XST_SUCCESS;
      }
  }

  return(Status);
}

/**
 * Retrieves a pointer to the video stream associated with the given Frame Buffer Write instance.
 *
 * @param  InstancePtr  Pointer to the XV_FrmbufWr_l2 instance.
 *
 * @return Pointer to the XVidC_VideoStream structure associated with the instance.
 */
XVidC_VideoStream *XVFrmbufWr_GetVideoStream(XV_FrmbufWr_l2 *InstancePtr)
{
  return(&InstancePtr->Stream);
}

/**
 * Sets the frame buffer address for the Frame Buffer Write core.
 *
 * This function validates and sets the memory address for the frame buffer.
 * The address must be aligned to the AXI-MM width, which is calculated as
 * 2 * PixelsPerClock * 4 bytes. If the address is not properly aligned,
 * the function returns an error code indicating misalignment.
 *
 * @param InstancePtr Pointer to the XV_FrmbufWr_l2 instance.
 * @param Addr        Physical address to be set as the frame buffer base address.
 *
 * @return
 *   - XST_SUCCESS if the address is valid and set successfully.
 *   - XVFRMBUFWR_ERR_MEM_ADDR_MISALIGNED if the address is not properly aligned.
 *   - XST_FAILURE for other failures.
 *
 * @note
 *   - The function asserts that InstancePtr is not NULL and Addr is not zero.
 *   - The alignment requirement is based on the AXI-MM interface and the
 *     core's PixelsPerClock configuration.
 */
int XVFrmbufWr_SetBufferAddr(XV_FrmbufWr_l2 *InstancePtr,
                             UINTPTR Addr)
{
  UINTPTR Align;
  u32 AddrValid = FALSE;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(Addr != 0);

  /* Check if addr is aligned to aximm width (2*PPC*32-bits (4Bytes)) */
  Align = 2 * InstancePtr->FrmbufWr.Config.PixPerClk * 4;
  if ((Addr % Align) != 0) {
     AddrValid = FALSE;
     Status   = XVFRMBUFWR_ERR_MEM_ADDR_MISALIGNED;
  } else {
     AddrValid = TRUE;
  }

  if (AddrValid) {
    XV_frmbufwr_Set_HwReg_frm_buffer_V(&InstancePtr->FrmbufWr, Addr);
    Status = XST_SUCCESS;
  }
  return(Status);
}

/**
 * Retrieves the current buffer address used by the frame buffer write instance.
 *
 * @param InstancePtr: Pointer to the XV_FrmbufWr_l2 instance.
 *
 * @return
 *   The address (UINTPTR) of the current frame buffer.
 *
 * This function asserts that the provided instance pointer is not NULL,
 * then reads and returns the hardware register value corresponding to the
 * frame buffer address.
 */
UINTPTR XVFrmbufWr_GetBufferAddr(XV_FrmbufWr_l2 *InstancePtr)
{
  UINTPTR ReadVal = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);

  ReadVal = XV_frmbufwr_Get_HwReg_frm_buffer_V(&InstancePtr->FrmbufWr);
  return(ReadVal);
}

/**
 * Sets the chroma buffer address for the frame buffer write instance.
 *
 * This function sets the address of the chroma buffer for the given frame buffer write instance.
 * It checks if the provided address is aligned to the required AXI memory width (2 * PixelsPerClock * 4 bytes).
 * If the address is not properly aligned, the function returns an error code indicating misalignment.
 * Otherwise, it updates the hardware register with the new chroma buffer address.
 *
 * @param InstancePtr Pointer to the XV_FrmbufWr_l2 instance.
 * @param Addr        Address to be set for the chroma buffer.
 *
 * @return
 *   - XST_SUCCESS if the address is valid and set successfully.
 *   - XVFRMBUFWR_ERR_MEM_ADDR_MISALIGNED if the address is not properly aligned.
 *   - XST_FAILURE for other failures.
 *
 * @note
 *   The address must be aligned to (2 * PixelsPerClock * 4) bytes.
 *   The function asserts that InstancePtr is not NULL and Addr is not zero.
 */
int XVFrmbufWr_SetChromaBufferAddr(XV_FrmbufWr_l2 *InstancePtr,
                             UINTPTR Addr)
{
  UINTPTR Align;
  u32 AddrValid = FALSE;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(Addr != 0);

  /* Check if addr is aligned to aximm width (2*PPC*32-bits (4Bytes)) */
  Align = 2 * InstancePtr->FrmbufWr.Config.PixPerClk * 4;
  if ((Addr % Align) != 0) {
     AddrValid = FALSE;
     Status   = XVFRMBUFWR_ERR_MEM_ADDR_MISALIGNED;
  } else {
     AddrValid = TRUE;
  }

  if (AddrValid) {
    XV_frmbufwr_Set_HwReg_frm_buffer2_V(&InstancePtr->FrmbufWr, Addr);
    Status = XST_SUCCESS;
  }
  return(Status);
}

/**
 * Retrieves the address of the chroma buffer for the given Frame Buffer Write instance.
 *
 * @param InstancePtr Pointer to the XV_FrmbufWr_l2 instance.
 * @return The address (UINTPTR) of the chroma buffer.
 *
 * This function asserts that the provided instance pointer is not NULL,
 * then reads and returns the chroma buffer address from the hardware register.
 */
UINTPTR XVFrmbufWr_GetChromaBufferAddr(XV_FrmbufWr_l2 *InstancePtr)
{
  UINTPTR ReadVal = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);

  ReadVal = XV_frmbufwr_Get_HwReg_frm_buffer2_V(&InstancePtr->FrmbufWr);
  return(ReadVal);
}

/**
 * Sets the vertical chroma buffer address for the frame buffer write instance.
 *
 * This function sets the address for the vertical chroma buffer in the frame buffer write hardware.
 * The address must be aligned to a specific boundary determined by the number of pixels per clock
 * and the data width. If the address is not properly aligned, the function returns an error code.
 *
 * @param InstancePtr Pointer to the XV_FrmbufWr_l2 instance.
 * @param Addr        Address to be set for the vertical chroma buffer.
 *
 * @return
 *   - XST_SUCCESS if the address is valid and set successfully.
 *   - XVFRMBUFWR_ERR_MEM_ADDR_MISALIGNED if the address is not properly aligned.
 *   - XST_FAILURE for other failures.
 *
 * @note
 *   The address alignment is calculated as 2 * PixPerClk * 4 bytes.
 *   The function asserts that InstancePtr is not NULL and Addr is not zero.
 */
int XVFrmbufWr_SetVChromaBufferAddr(XV_FrmbufWr_l2 *InstancePtr,
                             UINTPTR Addr)
{
  UINTPTR Align;
  u32 AddrValid = FALSE;
  int Status = XST_FAILURE;
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(Addr != 0);
  Align = 2 * InstancePtr->FrmbufWr.Config.PixPerClk * 4;
  if ((Addr % Align) != 0) {
     AddrValid = FALSE;
     Status   = XVFRMBUFWR_ERR_MEM_ADDR_MISALIGNED;
  } else {
     AddrValid = TRUE;
  }
  if (AddrValid) {
    XV_frmbufwr_Set_HwReg_frm_buffer3_V(&InstancePtr->FrmbufWr, Addr);
    Status = XST_SUCCESS;
  }
  return(Status);
}

/**
 * Retrieves the address of the chroma buffer (V component) for the given frame buffer write instance.
 *
 * @param InstancePtr Pointer to the XV_FrmbufWr_l2 instance.
 * @return The address (UINTPTR) of the V chroma buffer.
 *
 * @note The function asserts that InstancePtr is not NULL.
 */
UINTPTR XVFrmbufWr_GetVChromaBufferAddr(XV_FrmbufWr_l2 *InstancePtr)
{
  UINTPTR ReadVal = 0;
  Xil_AssertNonvoid(InstancePtr != NULL);
  ReadVal = XV_frmbufwr_Get_HwReg_frm_buffer3_V(&InstancePtr->FrmbufWr);
  return(ReadVal);
}

/**
 * Retrieves the current Field ID from the Frame Buffer Write hardware.
 *
 * @param InstancePtr: Pointer to the XV_FrmbufWr_l2 instance.
 *
 * This function asserts that the provided instance pointer is not NULL and that the
 * Frame Buffer Write configuration is set to interlaced mode. It then reads and returns
 * the current Field ID from the hardware register.
 *
 * @return
 *   The current Field ID value read from the hardware.
 */
u32 XVFrmbufWr_GetFieldID(XV_FrmbufWr_l2 *InstancePtr)
{
  u32 ReadVal = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->FrmbufWr.Config.Interlaced);

  ReadVal = XV_frmbufwr_Get_HwReg_field_id(&InstancePtr->FrmbufWr);
  return(ReadVal);
}

/**
 * XVFrmbufWr_DbgReportStatus - Prints the current status and configuration of the Frame Buffer Write core.
 *
 * @param InstancePtr Pointer to the XV_FrmbufWr_l2 instance.
 *
 * This function prints a detailed debug report of the Frame Buffer Write (FrmbufWr) core's
 * configuration and status registers to the standard output using xil_printf. The report includes:
 *   - Pixels per clock
 *   - Color depth and AXI-MM data width
 *   - Enabled color formats (RGBX8, BGRX8, YUVX8, etc.)
 *   - Interlaced and tile format enable status
 *   - Control register value
 *   - Current width, height, stride, video format
 *   - Buffer addresses (luma, chroma, vchroma)
 *
 * The function asserts that the provided instance pointer is not NULL.
 *
 * This function is intended for debugging and diagnostic purposes.
 *
 * @note   none
 */
void XVFrmbufWr_DbgReportStatus(XV_FrmbufWr_l2 *InstancePtr)
{
  u32 ctrl;

  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->FRAME BUFFER WRITE STATUS<----\r\n");

  ctrl  = XV_frmbufwr_ReadReg(InstancePtr->FrmbufWr.Config.BaseAddress,
                              XV_FRMBUFWR_CTRL_ADDR_AP_CTRL);

  xil_printf("Pixels Per Clock:           %d\r\n", InstancePtr->FrmbufWr.Config.PixPerClk);
  xil_printf("Color Depth:                %d\r\n", InstancePtr->FrmbufWr.Config.MaxDataWidth);
  xil_printf("AXI-MM Data Width:          %d\r\n", InstancePtr->FrmbufWr.Config.AXIMMDataWidth);
  xil_printf("RGBX8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.RGBX8En);
  xil_printf("BGRX8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.BGRX8En);
  xil_printf("YUVX8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.YUVX8En);
  xil_printf("RGBA8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.RGBA8En);
  xil_printf("BGRA8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.BGRA8En);
  xil_printf("YUVA8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.YUVA8En);
  xil_printf("YUYV8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.YUYV8En);
  xil_printf("UYVY8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.UYVY8En);
  xil_printf("RGBX10 Enabled:             %d\r\n", InstancePtr->FrmbufWr.Config.RGBX10En);
  xil_printf("YUVX10 Enabled:             %d\r\n", InstancePtr->FrmbufWr.Config.YUVX10En);
  xil_printf("Y_UV8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV8En);
  xil_printf("Y_UV8_420 Enabled:          %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV8_420En);
  xil_printf("RGB8 Enabled:               %d\r\n", InstancePtr->FrmbufWr.Config.RGB8En);
  xil_printf("BGR8 Enabled:               %d\r\n", InstancePtr->FrmbufWr.Config.BGR8En);
  xil_printf("YUV8 Enabled:               %d\r\n", InstancePtr->FrmbufWr.Config.YUV8En);
  xil_printf("Y_UV10 Enabled:             %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV10En);
  xil_printf("Y_UV10_420 Enabled:         %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV10_420En);
  xil_printf("Y8 Enabled:                 %d\r\n", InstancePtr->FrmbufWr.Config.Y8En);
  xil_printf("Y10 Enabled:                %d\r\n", InstancePtr->FrmbufWr.Config.Y10En);
  xil_printf("RGBX12 Enabled:             %d\r\n", InstancePtr->FrmbufWr.Config.RGBX12En);
  xil_printf("RGB16 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.RGB16En);
  xil_printf("YUVX12 Enabled:             %d\r\n", InstancePtr->FrmbufWr.Config.YUVX12En);
  xil_printf("YUV16 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.YUV16En);
  xil_printf("Y_UV12 Enabled:             %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV12En);
  xil_printf("Y_UV16 Enabled:             %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV16En);
  xil_printf("Y_UV12_420 Enabled:         %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV12_420En);
  xil_printf("Y_UV16_420 Enabled:         %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV16_420En);
  xil_printf("Y12 Enabled:                %d\r\n", InstancePtr->FrmbufWr.Config.Y12En);
  xil_printf("Y16 Enabled:                %d\r\n", InstancePtr->FrmbufWr.Config.Y16En);
  xil_printf("Y_U_V8 Enabled:             %d\r\n", InstancePtr->FrmbufWr.Config.Y_U_V8En);
  xil_printf("Y_U_V10 Enabled:            %d\r\n", InstancePtr->FrmbufWr.Config.Y_U_V10En);
  xil_printf("Y_U_V8_420 Enabled:         %d\r\n", InstancePtr->FrmbufWr.Config.Y_U_V8_420En);
  xil_printf("Y_U_V12 Enabled:            %d\r\n", InstancePtr->FrmbufWr.Config.Y_U_V12En);
  xil_printf("Y_U_V10_L16LE Enabled:      %d\r\n", InstancePtr->FrmbufWr.Config.Y_U_V10_L16LEEn);
  xil_printf("Y_UV10_L16LE Enabled:       %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV10_L16LEEn);
  xil_printf("Y_UV10_420_L16LE Enabled:   %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV10_420_L16LEEn);
  xil_printf("YY10_L16LE Enabled:         %d\r\n", InstancePtr->FrmbufWr.Config.Y10_L16LEEn);
  xil_printf("Y_U_V12_L16LE Enabled:      %d\r\n", InstancePtr->FrmbufWr.Config.Y_U_V12_L16LEEn);
  xil_printf("Y_UV12_L16LE Enabled:       %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV12_L16LEEn);
  xil_printf("Y_UV12_420_L16LE Enabled:   %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV12_420_L16LEEn);
  xil_printf("YY12_L16LE Enabled:         %d\r\n", InstancePtr->FrmbufWr.Config.Y12_L16LEEn);
  xil_printf("Y_U_V10_M16LE Enabled:      %d\r\n", InstancePtr->FrmbufWr.Config.Y_U_V10_M16LEEn);
  xil_printf("Y_UV10_M16LE Enabled:       %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV10_M16LEEn);
  xil_printf("Y_UV10_420_M16LE Enabled:   %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV10_420_M16LEEn);
  xil_printf("YY10_M16LE Enabled:         %d\r\n", InstancePtr->FrmbufWr.Config.Y10_M16LEEn);
  xil_printf("Y_U_V12_M16LE Enabled:      %d\r\n", InstancePtr->FrmbufWr.Config.Y_U_V12_M16LEEn);
  xil_printf("Y_UV12_M16LE Enabled:       %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV12_M16LEEn);
  xil_printf("Y_UV12_420_M16LE Enabled:   %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV12_420_M16LEEn);
  xil_printf("YY12_M16LE Enabled:         %d\r\n", InstancePtr->FrmbufWr.Config.Y12_M16LEEn);
  xil_printf("Interlaced Enabled:         %d\r\n", InstancePtr->FrmbufWr.Config.Interlaced);
  xil_printf("Tile format Enabled:        %d\r\n", InstancePtr->FrmbufWr.Config.IsTileFormat);
  xil_printf("Low Latency Support:        %d\r\n", InstancePtr->FrmbufWr.Config.EnSyncSignals);
  xil_printf("Partial Frame Support:      %d\r\n", InstancePtr->FrmbufWr.Config.EnPartialFrm);

  xil_printf("Control Reg:                0x%x\r\n", ctrl);
  xil_printf("Width:                      %d\r\n", XV_frmbufwr_Get_HwReg_width(&InstancePtr->FrmbufWr));
  xil_printf("Height:                     %d\r\n", XV_frmbufwr_Get_HwReg_height(&InstancePtr->FrmbufWr));
  xil_printf("Stride (in bytes):          %d\r\n", XV_frmbufwr_Get_HwReg_stride(&InstancePtr->FrmbufWr));
  xil_printf("Video Format:               %d\r\n", XV_frmbufwr_Get_HwReg_video_format(&InstancePtr->FrmbufWr));
  xil_printf("Buffer Address:             0x%x\r\n", XV_frmbufwr_Get_HwReg_frm_buffer_V(&InstancePtr->FrmbufWr));
  xil_printf("Chroma Buffer Address:      0x%x\r\n", XV_frmbufwr_Get_HwReg_frm_buffer2_V(&InstancePtr->FrmbufWr));
  xil_printf("VChroma Buffer Address:     0x%x\r\n", XV_frmbufwr_Get_HwReg_frm_buffer3_V(&InstancePtr->FrmbufWr));
}

/** @} */
