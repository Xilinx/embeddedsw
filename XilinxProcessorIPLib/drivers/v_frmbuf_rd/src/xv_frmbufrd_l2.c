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
* @file xv_frmbufrd_l2.c
* @addtogroup v_frmbuf_rd_v2_0
* @{
*
* Frame Buffer Read Layer-2 Driver. The functions in this file provides an
* abstraction from the register peek/poke methodology by implementing most
* common use-case provided by the sub-core. See xv_frmbufrd_l2.h for a detailed
* description of the layer-2 driver
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vyc   04/05/17   Initial Release
* 2.00  vyc   10/04/17   Add second buffer pointer for semi-planar formats
*                        Add memory formats RGBA8, YUVA8, BGRA8, BGRX8, UYVY8
* 3.00  vyc   04/04/18   Add interlaced support
                         Add new memory format BGR8
                         Add interrupt handler for ap_ready
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "xv_frmbufrd_l2.h"

/************************** Constant Definitions *****************************/
#define XVFRMBUFRD_MIN_STRM_WIDTH            (64u)
#define XVFRMBUFRD_MIN_STRM_HEIGHT           (64u)
#define XVFRMBUFRD_IDLE_TIMEOUT              (1000000)

/************************** Function Prototypes ******************************/
static void SetPowerOnDefaultState(XV_FrmbufRd_l2 *InstancePtr);
XVidC_ColorFormat RdMemory2Live(XVidC_ColorFormat MemFmt);

/*****************************************************************************/
/**
* This function maps the memory video formats to the live/stream video formats
*
* @param  MemFmt is the video format read from memory
*
* @return Live/Stream Video Format associated with that memory format
*
******************************************************************************/
XVidC_ColorFormat RdMemory2Live(XVidC_ColorFormat MemFmt)
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
        case XVIDC_CSF_MEM_BGRA8 :
            StrmFmt = XVIDC_CSF_RGBA;
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
int XVFrmbufRd_Initialize(XV_FrmbufRd_l2 *InstancePtr, u16 DeviceId)
{
  int Status;
  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Setup the instance */
  memset(InstancePtr, 0, sizeof(XV_FrmbufRd_l2));
  Status = XV_frmbufrd_Initialize(&InstancePtr->FrmbufRd, DeviceId);

  if(Status == XST_SUCCESS) {
    SetPowerOnDefaultState(InstancePtr);
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function initializes the frame buffer read core instance to default state
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return None
*
******************************************************************************/
static void SetPowerOnDefaultState(XV_FrmbufRd_l2 *InstancePtr)
{
  XVidC_VideoStream VidStrm;
  XVidC_VideoTiming const *ResTiming;

  u32 IrqMask = 0;

  memset(&VidStrm, 0, sizeof(XVidC_VideoStream));

  /* Set Default Stream Out */
  VidStrm.VmId          = XVIDC_VM_1920x1080_60_P;
  VidStrm.ColorFormatId = XVIDC_CSF_RGB;
  VidStrm.FrameRate     = XVIDC_FR_60HZ;
  VidStrm.IsInterlaced  = FALSE;
  VidStrm.ColorDepth    = (XVidC_ColorDepth)InstancePtr->FrmbufRd.Config.MaxDataWidth;
  VidStrm.PixPerClk     = (XVidC_PixelsPerClock)InstancePtr->FrmbufRd.Config.PixPerClk;

  ResTiming = XVidC_GetTimingInfo(VidStrm.VmId);

  VidStrm.Timing = *ResTiming;

  /* Set frame width, height, stride and memory video format */
  XV_frmbufrd_Set_HwReg_width(&InstancePtr->FrmbufRd,  VidStrm.Timing.HActive);
  XV_frmbufrd_Set_HwReg_height(&InstancePtr->FrmbufRd, VidStrm.Timing.VActive);
  XV_frmbufrd_Set_HwReg_stride(&InstancePtr->FrmbufRd, 7680);
  XV_frmbufrd_Set_HwReg_video_format(&InstancePtr->FrmbufRd, XVIDC_CSF_MEM_RGBX8);
  if (XVFrmbufRd_InterlacedEnabled(InstancePtr)) {
    XV_frmbufrd_Set_HwReg_field_id(&InstancePtr->FrmbufRd, 0);
  }
  InstancePtr->Stream = VidStrm;

  /* Setup polling mode */
  IrqMask = XVFRMBUFRD_IRQ_DONE_MASK | XVFRMBUFRD_IRQ_READY_MASK;
  XVFrmbufRd_InterruptDisable(InstancePtr, IrqMask);
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
void XVFrmbufRd_InterruptEnable(XV_FrmbufRd_l2 *InstancePtr, u32 IrqMask)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Enable Interrupts */
  XV_frmbufrd_InterruptEnable(&InstancePtr->FrmbufRd, IrqMask);
  XV_frmbufrd_InterruptGlobalEnable(&InstancePtr->FrmbufRd);

  /* Clear autostart bit */
  XV_frmbufrd_DisableAutoRestart(&InstancePtr->FrmbufRd);
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
void XVFrmbufRd_InterruptDisable(XV_FrmbufRd_l2 *InstancePtr, u32 IrqMask)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Disable Interrupts */
  XV_frmbufrd_InterruptDisable(&InstancePtr->FrmbufRd, IrqMask);
  XV_frmbufrd_InterruptGlobalDisable(&InstancePtr->FrmbufRd);

  /* Set autostart bit */
  XV_frmbufrd_EnableAutoRestart(&InstancePtr->FrmbufRd);
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
void XVFrmbufRd_Start(XV_FrmbufRd_l2 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_frmbufrd_Start(&InstancePtr->FrmbufRd);
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
int XVFrmbufRd_Stop(XV_FrmbufRd_l2 *InstancePtr)
{
  int Status = XST_FAILURE;
  u32 isIdle = 0;
  u32 cnt = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Clear autostart bit */
  XV_frmbufrd_DisableAutoRestart(&InstancePtr->FrmbufRd);

  /* Wait for idle */
  do {
    isIdle = XV_frmbufrd_IsIdle(&InstancePtr->FrmbufRd);
    cnt++;
  } while((isIdle!=1) && (cnt < XVFRMBUFRD_IDLE_TIMEOUT));

  if (isIdle == 1 ) {
     Status = XST_SUCCESS;
  }
  return(Status);
}

/*****************************************************************************/
/**
* This function configures the frame buffer read memory input
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  StrideInBytes is the memory stride in bytes
* @param  MemFormat is the video format to be read from memory
* @param  StrmOut is the pointer to output stream configuration
*
* @return none
*
******************************************************************************/
int XVFrmbufRd_SetMemFormat(XV_FrmbufRd_l2 *InstancePtr,
                            u32 StrideInBytes,
                            XVidC_ColorFormat MemFmt,
                            const XVidC_VideoStream *StrmOut)
{
  int Status = XST_FAILURE;
  u32 FmtValid = FALSE;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(StrmOut != NULL);

  /* copy stream data */
  InstancePtr->Stream = *StrmOut;
  /* Width must be multiple of Samples per Clock */
  Xil_AssertNonvoid((StrmOut->Timing.HActive  % \
                     InstancePtr->FrmbufRd.Config.PixPerClk) == 0);

  /* For 4:2:2 and 4:2:0, columns must be in pairs */
  if (((RdMemory2Live(MemFmt) == XVIDC_CSF_YCRCB_422) || \
       (RdMemory2Live(MemFmt) == XVIDC_CSF_YCRCB_420)) && \
       ((StrmOut->Timing.HActive  % 2) != 0)) {
    Status = XVFRMBUFRD_ERR_FRAME_SIZE_INVALID;

  /* For 4:2:0, rows must be in pairs */
  } else if ((RdMemory2Live(MemFmt) == XVIDC_CSF_YCRCB_420) && \
             ((StrmOut->Timing.VActive  % 2) != 0)) {
    Status = XVFRMBUFRD_ERR_FRAME_SIZE_INVALID;

  /* Check if stride is aligned to aximm width (2*PPC*32-bits) */
  } else if ((StrideInBytes % (2 * InstancePtr->FrmbufRd.Config.PixPerClk * 4)) != 0) {
    Status   = XVFRMBUFRD_ERR_STRIDE_MISALIGNED;

  /* Streaming Video Format must match Memory Video Format */
  } else if (StrmOut->ColorFormatId != RdMemory2Live(MemFmt)) {
    Status = XVFRMBUFRD_ERR_VIDEO_FORMAT_MISMATCH;

  /* Memory Video Format must be supported in hardware */
  } else {
    Status = XVFRMBUFRD_ERR_DISABLED_IN_HW;
    FmtValid = FALSE;
    switch(MemFmt) {
      case XVIDC_CSF_MEM_RGBX8 :
         if (XVFrmbufRd_IsRGBX8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUVX8 :
         if (XVFrmbufRd_IsYUVX8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUYV8 :
         if (XVFrmbufRd_IsYUYV8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_RGBA8 :
         if (XVFrmbufRd_IsRGBA8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUVA8 :
         if (XVFrmbufRd_IsYUVA8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_RGBX10 :
         if (XVFrmbufRd_IsRGBX10Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUVX10 :
         if (XVFrmbufRd_IsYUVX10Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV8 :
         if (XVFrmbufRd_IsY_UV8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV8_420 :
         if (XVFrmbufRd_IsY_UV8_420Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_RGB8 :
         if (XVFrmbufRd_IsRGB8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_YUV8 :
         if (XVFrmbufRd_IsYUV8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV10 :
         if (XVFrmbufRd_IsY_UV10Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y_UV10_420 :
         if (XVFrmbufRd_IsY_UV10_420Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y8 :
         if (XVFrmbufRd_IsY8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_Y10 :
         if (XVFrmbufRd_IsY10Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_BGRA8 :
         if (XVFrmbufRd_IsBGRA8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_BGRX8 :
         if (XVFrmbufRd_IsBGRX8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_UYVY8 :
         if (XVFrmbufRd_IsUYVY8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;
      case XVIDC_CSF_MEM_BGR8 :
         if (XVFrmbufRd_IsBGR8Enabled(InstancePtr)) {
           FmtValid = TRUE;
         }
         break;

      default :
         FmtValid = FALSE;
         break;
    }

    if (FmtValid == TRUE) {
      /* configure frame buffer read */
      XV_frmbufrd_Set_HwReg_width(&InstancePtr->FrmbufRd,
                                  StrmOut->Timing.HActive);
      XV_frmbufrd_Set_HwReg_height(&InstancePtr->FrmbufRd,
                                   StrmOut->Timing.VActive);
      XV_frmbufrd_Set_HwReg_stride(&InstancePtr->FrmbufRd, StrideInBytes);
      XV_frmbufrd_Set_HwReg_video_format(&InstancePtr->FrmbufRd, MemFmt);
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
XVidC_VideoStream *XVFrmbufRd_GetVideoStream(XV_FrmbufRd_l2 *InstancePtr)
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
int XVFrmbufRd_SetBufferAddr(XV_FrmbufRd_l2 *InstancePtr,
                             UINTPTR Addr)
{
  UINTPTR Align;
  u32 AddrValid = FALSE;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(Addr != 0);

  /* Check if addr is aligned to aximm width (2*PPC*32-bits (4Bytes)) */
  Align = 2 * InstancePtr->FrmbufRd.Config.PixPerClk * 4;
  if((Addr % Align) != 0) {
     AddrValid = FALSE;
     Status   = XVFRMBUFRD_ERR_MEM_ADDR_MISALIGNED;
  } else {
     AddrValid = TRUE;
  }

  if(AddrValid) {
    XV_frmbufrd_Set_HwReg_frm_buffer_V(&InstancePtr->FrmbufRd, Addr);
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
UINTPTR XVFrmbufRd_GetBufferAddr(XV_FrmbufRd_l2 *InstancePtr)
{
  UINTPTR ReadVal = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);

  ReadVal = XV_frmbufrd_Get_HwReg_frm_buffer_V(&InstancePtr->FrmbufRd);
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
int XVFrmbufRd_SetChromaBufferAddr(XV_FrmbufRd_l2 *InstancePtr,
                                   UINTPTR Addr)
{
  UINTPTR Align;
  u32 AddrValid = FALSE;
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(Addr != 0);

  /* Check if addr is aligned to aximm width (2*PPC*32-bits (4Bytes)) */
  Align = 2 * InstancePtr->FrmbufRd.Config.PixPerClk * 4;
  if((Addr % Align) != 0) {
     AddrValid = FALSE;
     Status   = XVFRMBUFRD_ERR_MEM_ADDR_MISALIGNED;
  } else {
     AddrValid = TRUE;
  }

  if(AddrValid) {
    XV_frmbufrd_Set_HwReg_frm_buffer2_V(&InstancePtr->FrmbufRd, Addr);
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
UINTPTR XVFrmbufRd_GetChromaBufferAddr(XV_FrmbufRd_l2 *InstancePtr)
{
  UINTPTR ReadVal = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);

  ReadVal = XV_frmbufrd_Get_HwReg_frm_buffer2_V(&InstancePtr->FrmbufRd);
  return(ReadVal);
}

/*****************************************************************************/
/**
* This function sets the field ID
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Field ID
*
* @return XST_SUCCESS or XST_FAILURE
*
******************************************************************************/
int XVFrmbufRd_SetFieldID(XV_FrmbufRd_l2 *InstancePtr,
                          u32 FieldID)
{
  int Status = XST_FAILURE;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->FrmbufRd.Config.Interlaced);

  XV_frmbufrd_Set_HwReg_field_id(&InstancePtr->FrmbufRd, FieldID);
  Status = XST_SUCCESS;

  return(Status);
}

/*****************************************************************************/
/**
* This function reads the field ID
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return Field ID
*
******************************************************************************/
u32 XVFrmbufRd_GetFieldID(XV_FrmbufRd_l2 *InstancePtr)
{
  u32 ReadVal;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->FrmbufRd.Config.Interlaced);

  ReadVal = XV_frmbufrd_Get_HwReg_field_id(&InstancePtr->FrmbufRd);
  return(ReadVal);
}

/*****************************************************************************/
/**
* This function reports the frame buffer read status
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
* @note   none
*
******************************************************************************/
void XVFrmbufRd_DbgReportStatus(XV_FrmbufRd_l2 *InstancePtr)
{
  u32 ctrl;

  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n\r\n----->FRAME BUFFER READ STATUS<----\r\n");

  ctrl  = XV_frmbufrd_ReadReg(InstancePtr->FrmbufRd.Config.BaseAddress,
                              XV_FRMBUFRD_CTRL_ADDR_AP_CTRL);

  xil_printf("Pixels Per Clock:           %d\r\n", InstancePtr->FrmbufRd.Config.PixPerClk);
  xil_printf("Color Depth:                %d\r\n", InstancePtr->FrmbufRd.Config.MaxDataWidth);
  xil_printf("AXI-MM Data Width:          %d\r\n", InstancePtr->FrmbufRd.Config.AXIMMDataWidth);
  xil_printf("RGBX8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.RGBX8En);
  xil_printf("YUVX8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.YUVX8En);
  xil_printf("YUYV8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.YUYV8En);
  xil_printf("RGBA8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.RGBA8En);
  xil_printf("YUVA8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.YUVA8En);
  xil_printf("RGBX10 Enabled:             %d\r\n", InstancePtr->FrmbufRd.Config.RGBX10En);
  xil_printf("YUVX10 Enabled:             %d\r\n", InstancePtr->FrmbufRd.Config.YUVX10En);
  xil_printf("Y_UV8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV8En);
  xil_printf("Y_UV8_420 Enabled:          %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV8_420En);
  xil_printf("RGB8 Enabled:               %d\r\n", InstancePtr->FrmbufRd.Config.RGB8En);
  xil_printf("YUV8 Enabled:               %d\r\n", InstancePtr->FrmbufRd.Config.YUV8En);
  xil_printf("Y_UV10 Enabled:             %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV10En);
  xil_printf("Y_UV10_420 Enabled:         %d\r\n", InstancePtr->FrmbufRd.Config.Y_UV10_420En);
  xil_printf("Y8 Enabled:                 %d\r\n", InstancePtr->FrmbufRd.Config.Y8En);
  xil_printf("Y10 Enabled:                %d\r\n", InstancePtr->FrmbufRd.Config.Y10En);
  xil_printf("BGRA8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.BGRA8En);
  xil_printf("BGRX8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.BGRX8En);
  xil_printf("BGR8 Enabled:               %d\r\n", InstancePtr->FrmbufRd.Config.BGR8En);
  xil_printf("UYVY8 Enabled:              %d\r\n", InstancePtr->FrmbufRd.Config.UYVY8En);
  xil_printf("Interlaced Enabled:         %d\r\n", InstancePtr->FrmbufRd.Config.Interlaced);

  xil_printf("Control Reg:                0x%x\r\n", ctrl);
  xil_printf("Width:                      %d\r\n",   XV_frmbufrd_Get_HwReg_width(&InstancePtr->FrmbufRd));
  xil_printf("Height:                     %d\r\n",   XV_frmbufrd_Get_HwReg_height(&InstancePtr->FrmbufRd));
  xil_printf("Stride (in bytes):          %d\r\n",   XV_frmbufrd_Get_HwReg_stride(&InstancePtr->FrmbufRd));
  xil_printf("Video Format:               %d\r\n",   XV_frmbufrd_Get_HwReg_video_format(&InstancePtr->FrmbufRd));
  xil_printf("Buffer Address:             0x%x\r\n", XV_frmbufrd_Get_HwReg_frm_buffer_V(&InstancePtr->FrmbufRd));
  xil_printf("Chroma Buffer Address:      0x%x\r\n", XV_frmbufrd_Get_HwReg_frm_buffer2_V(&InstancePtr->FrmbufRd));
}

/** @} */
