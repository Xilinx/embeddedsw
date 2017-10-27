/******************************************************************************
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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xv_frmbufwr_l2.c
* @addtogroup v_frmbuf_wr
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
                         Add new memory formats BGRX8 and UYVY8
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "xv_frmbufwr_l2.h"

/************************** Constant Definitions *****************************/
#define XVFRMBUFWR_MIN_STRM_WIDTH            (64u)
#define XVFRMBUFWR_MIN_STRM_HEIGHT           (64u)
#define XVFRMBUFWR_IDLE_TIMEOUT              (1000000)

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
            StrmFmt = XVIDC_CSF_YCRCB_444;
            break;
        case XVIDC_CSF_MEM_Y10 :
            StrmFmt = XVIDC_CSF_YCRCB_444;
            break;
        case XVIDC_CSF_MEM_BGRX8 :
            StrmFmt = XVIDC_CSF_RGB;
            break;
       case XVIDC_CSF_MEM_UYVY8 :
            StrmFmt = XVIDC_CSF_YCRCB_422;
            break;

        default:
            StrmFmt = (XVidC_ColorFormat)~0;
            break;
    }
    return(StrmFmt);
}

/*****************************************************************************/
/**
* This function initializes the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  DeviceId is instance id of the core
*
* @return XST_SUCCESS if device is found and initialized
*         XST_DEVICE_NOT_FOUND if device is not found
*
******************************************************************************/
int XVFrmbufWr_Initialize(XV_FrmbufWr_l2 *InstancePtr, u16 DeviceId)
{
  int Status;
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Setup the instance */
  memset(InstancePtr, 0, sizeof(XV_FrmbufWr_l2));
  Status = XV_frmbufwr_Initialize(&InstancePtr->FrmbufWr, DeviceId);

  if(Status == XST_SUCCESS) {
    SetPowerOnDefaultState(InstancePtr);
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function initializes the frame buffer write core instance to default state
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return None
*
******************************************************************************/
static void SetPowerOnDefaultState(XV_FrmbufWr_l2 *InstancePtr)
{
  XVidC_VideoStream VidStrm;
  XVidC_VideoTiming const *ResTiming;

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
  XVFrmbufWr_InterruptDisable(InstancePtr);
}

/*****************************************************************************/
/**
* This function enables interrupts in the core
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
void XVFrmbufWr_InterruptEnable(XV_FrmbufWr_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Enable Interrupts */
  XV_frmbufwr_InterruptEnable(&InstancePtr->FrmbufWr, XVFRMBUFWR_IRQ_DONE_MASK);
  XV_frmbufwr_InterruptGlobalEnable(&InstancePtr->FrmbufWr);

  /* Clear autostart bit */
  XV_frmbufwr_DisableAutoRestart(&InstancePtr->FrmbufWr);
}

/*****************************************************************************/
/**
* This function disables interrupts in the core
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
void XVFrmbufWr_InterruptDisable(XV_FrmbufWr_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Disable Interrupts */
  XV_frmbufwr_InterruptDisable(&InstancePtr->FrmbufWr, XVFRMBUFWR_IRQ_DONE_MASK);
  XV_frmbufwr_InterruptGlobalDisable(&InstancePtr->FrmbufWr);

  /* Set autostart bit */
  XV_frmbufwr_EnableAutoRestart(&InstancePtr->FrmbufWr);
}

/*****************************************************************************/
/**
* This function starts the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
void XVFrmbufWr_Start(XV_FrmbufWr_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_frmbufwr_Start(&InstancePtr->FrmbufWr);
}

/*****************************************************************************/
/**
* This function stops the core instance
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
int XVFrmbufWr_Stop(XV_FrmbufWr_l2 *InstancePtr)
{
  int Status = XST_FAILURE;
  u32 isIdle = 0;
  u32 cnt = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Clear autostart bit */
  XV_frmbufwr_DisableAutoRestart(&InstancePtr->FrmbufWr);

  /* Wait for idle */
  do {
    isIdle = XV_frmbufwr_IsIdle(&InstancePtr->FrmbufWr);
    cnt++;
  } while((isIdle!=1) && (cnt < XVFRMBUFWR_IDLE_TIMEOUT));

  if (isIdle == 1 ) {
     Status = XST_SUCCESS;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function configures the frame buffer write memory output
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  StrideInBytes is the memory stride in bytes
* @param  MemFormat is the video format written to memory
* @param  StrmIn is the pointer to output stream configuration
*
* @return none
*
******************************************************************************/
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

/*****************************************************************************/
/**
* This function reads the pointer to the output stream configuration
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return  Pointer to output stream configuration
*
******************************************************************************/
XVidC_VideoStream *XVFrmbufWr_GetVideoStream(XV_FrmbufWr_l2 *InstancePtr)
{
  return(&InstancePtr->Stream);
}

/*****************************************************************************/
/**
* This function sets the buffer address
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Addr is the absolute address of buffer in memory
*
* @return XST_SUCCESS or XST_FAILURE
*
******************************************************************************/
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
  if((Addr % Align) != 0) {
     AddrValid = FALSE;
     Status   = XVFRMBUFWR_ERR_MEM_ADDR_MISALIGNED;
  } else {
     AddrValid = TRUE;
  }

  if(AddrValid) {
    XV_frmbufwr_Set_HwReg_frm_buffer_V(&InstancePtr->FrmbufWr, Addr);
    Status = XST_SUCCESS;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function reads the buffer address
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return Address of buffer in memory
*
******************************************************************************/
UINTPTR XVFrmbufWr_GetBufferAddr(XV_FrmbufWr_l2 *InstancePtr)
{
  UINTPTR ReadVal = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);

  ReadVal = XV_frmbufwr_Get_HwReg_frm_buffer_V(&InstancePtr->FrmbufWr);
  return(ReadVal);
}

/*****************************************************************************/
/**
* This function sets the buffer address for the UV plane for semi-planar formats
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Addr is the absolute address of buffer in memory
*
* @return XST_SUCCESS or XST_FAILURE
*
******************************************************************************/
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
  if((Addr % Align) != 0) {
     AddrValid = FALSE;
     Status   = XVFRMBUFWR_ERR_MEM_ADDR_MISALIGNED;
  } else {
     AddrValid = TRUE;
  }

  if(AddrValid) {
    XV_frmbufwr_Set_HwReg_frm_buffer2_V(&InstancePtr->FrmbufWr, Addr);
    Status = XST_SUCCESS;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function reads the buffer address for the UV plane for semi-planar formats
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return Address of buffer in memory
*
******************************************************************************/
UINTPTR XVFrmbufWr_GetChromaBufferAddr(XV_FrmbufWr_l2 *InstancePtr)
{
  UINTPTR ReadVal = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);

  ReadVal = XV_frmbufwr_Get_HwReg_frm_buffer2_V(&InstancePtr->FrmbufWr);
  return(ReadVal);
}

/*****************************************************************************/
/**
* This function reports the frame buffer write status
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
* @note   none
*
******************************************************************************/
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
  xil_printf("YUYV8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.YUYV8En);
  xil_printf("UYVY8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.UYVY8En);
  xil_printf("RGBX10 Enabled:             %d\r\n", InstancePtr->FrmbufWr.Config.RGBX10En);
  xil_printf("YUVX10 Enabled:             %d\r\n", InstancePtr->FrmbufWr.Config.YUVX10En);
  xil_printf("Y_UV8 Enabled:              %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV8En);
  xil_printf("Y_UV8_420 Enabled:          %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV8_420En);
  xil_printf("RGB8 Enabled:               %d\r\n", InstancePtr->FrmbufWr.Config.RGB8En);
  xil_printf("YUV8 Enabled:               %d\r\n", InstancePtr->FrmbufWr.Config.YUV8En);
  xil_printf("Y_UV10 Enabled:             %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV10En);
  xil_printf("Y_UV10_420 Enabled:         %d\r\n", InstancePtr->FrmbufWr.Config.Y_UV10_420En);
  xil_printf("Y8 Enabled:                 %d\r\n", InstancePtr->FrmbufWr.Config.Y8En);
  xil_printf("Y10 Enabled:                %d\r\n", InstancePtr->FrmbufWr.Config.Y10En);

  xil_printf("Control Reg:                0x%x\r\n", ctrl);
  xil_printf("Width:                      %d\r\n", XV_frmbufwr_Get_HwReg_width(&InstancePtr->FrmbufWr));
  xil_printf("Height:                     %d\r\n", XV_frmbufwr_Get_HwReg_height(&InstancePtr->FrmbufWr));
  xil_printf("Stride (in bytes):          %d\r\n", XV_frmbufwr_Get_HwReg_stride(&InstancePtr->FrmbufWr));
  xil_printf("Video Format:               %d\r\n", XV_frmbufwr_Get_HwReg_video_format(&InstancePtr->FrmbufWr));
  xil_printf("Buffer Address:             0x%x\r\n", XV_frmbufwr_Get_HwReg_frm_buffer_V(&InstancePtr->FrmbufWr));
  xil_printf("Chroma Buffer Address:      0x%x\r\n", XV_frmbufwr_Get_HwReg_frm_buffer2_V(&InstancePtr->FrmbufWr));
}

/** @} */
