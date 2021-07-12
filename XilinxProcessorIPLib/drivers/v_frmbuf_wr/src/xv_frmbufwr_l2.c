/******************************************************************************
* Copyright (C) 2017-2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_frmbufwr_l2.c
* @addtogroup v_frmbuf_wr_v4_5
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

  if (Status == XST_SUCCESS) {
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

/*****************************************************************************/
/**
* This function enables interrupts in the core
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return none
*
******************************************************************************/
void XVFrmbufWr_InterruptEnable(XV_FrmbufWr_l2 *InstancePtr, u32 IrqMask)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Enable Interrupts */
  XV_frmbufwr_InterruptEnable(&InstancePtr->FrmbufWr, IrqMask);
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
void XVFrmbufWr_InterruptDisable(XV_FrmbufWr_l2 *InstancePtr, u32 IrqMask)
{
  Xil_AssertVoid(InstancePtr != NULL);

  /* Disable Interrupts */
  XV_frmbufwr_InterruptDisable(&InstancePtr->FrmbufWr, IrqMask);
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
* @return XST_SUCCESS if the core is stop state
*         XST_FAILURE if the core is not in stop state
*
******************************************************************************/
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
/*****************************************************************************/
/**
* This function Waits for the core to reach idle state
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return XST_SUCCESS if the core is in idle state
*         XST_FAILURE if the core is not in idle state
*
******************************************************************************/
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
* or Only U Plane for 3 planar formats
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

/*****************************************************************************/
/**
* This function reads the buffer address for the UV plane for semi-planar formats
* or Only U plane for 3 planar formats
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
* This function sets the buffer address for the V plane for 3 planar formats
*
* @param  InstancePtr is a pointer to core instance to be worked upon
* @param  Addr is the absolute address of buffer in memory
*
* @return XST_SUCCESS or XST_FAILURE
*
******************************************************************************/
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

/*****************************************************************************/
/**
* This function reads the buffer address for the V plane for 3 planar formats
*
* @param  InstancePtr is a pointer to core instance to be worked upon
*
* @return Address of buffer in memory
*
******************************************************************************/
UINTPTR XVFrmbufWr_GetVChromaBufferAddr(XV_FrmbufWr_l2 *InstancePtr)
{
  UINTPTR ReadVal = 0;
  Xil_AssertNonvoid(InstancePtr != NULL);
  ReadVal = XV_frmbufwr_Get_HwReg_frm_buffer3_V(&InstancePtr->FrmbufWr);
  return(ReadVal);
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
u32 XVFrmbufWr_GetFieldID(XV_FrmbufWr_l2 *InstancePtr)
{
  u32 ReadVal = 0;

  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->FrmbufWr.Config.Interlaced);

  ReadVal = XV_frmbufwr_Get_HwReg_field_id(&InstancePtr->FrmbufWr);
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
  xil_printf("Interlaced Enabled:         %d\r\n", InstancePtr->FrmbufWr.Config.Interlaced);

  xil_printf("Control Reg:                0x%x\r\n", ctrl);
  xil_printf("Width:                      %d\r\n", XV_frmbufwr_Get_HwReg_width(&InstancePtr->FrmbufWr));
  xil_printf("Height:                     %d\r\n", XV_frmbufwr_Get_HwReg_height(&InstancePtr->FrmbufWr));
  xil_printf("Stride (in bytes):          %d\r\n", XV_frmbufwr_Get_HwReg_stride(&InstancePtr->FrmbufWr));
  xil_printf("Video Format:               %d\r\n", XV_frmbufwr_Get_HwReg_video_format(&InstancePtr->FrmbufWr));
  xil_printf("Buffer Address:             0x%x\r\n", XV_frmbufwr_Get_HwReg_frm_buffer_V(&InstancePtr->FrmbufWr));
  xil_printf("Chroma Buffer Address:      0x%x\r\n", XV_frmbufwr_Get_HwReg_frm_buffer2_V(&InstancePtr->FrmbufWr));
}

/** @} */
