/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvprocss_vdma.c
* @addtogroup vprocss Overview
* @{
* @details

* Video buffer management routine.
* The functions in this file provides an abstraction from the register peek/poke
* methodology by implementing most common use-case provided by the VDMA sub-core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   07/21/15  Initial Release
* 2.00  dmc   03/03/16  Remove xil_print's and report errors via event log
* 2.10  rco   07/21/16  Used UINTPTR instead of u32 for Address
* 2.3   rco   02/09/17  Fix c++ warnings
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xvidc.h"
#include "xvprocss_vdma.h"

#define XVDMA_RESET_TIMEOUT   (1000000) //10ms at 10ns time period (100MHz clock))

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
* This function starts Read and Write channels
*
* @param  XVdmaPtr is the pointer to core instance to be worked on
*
* @return None
*
*******************************************************************************/
void XVprocSs_VdmaStart(XVprocSs *XVprocSsPtr)
{
  XAxiVdma *XVdmaPtr = XVprocSsPtr->VdmaPtr;
  if(XVdmaPtr) {
    XAxiVdma_DmaStart(XVdmaPtr, XAXIVDMA_WRITE);
    XAxiVdma_DmaStart(XVdmaPtr, XAXIVDMA_READ);
  }
}

/*****************************************************************************/
/**
* This function stops Read and Write channels
*
* @param  XVdmaPtr is the pointer to core instance to be worked on
*
* @return None
*
*******************************************************************************/
void XVprocSs_VdmaStop(XVprocSs *XVprocSsPtr)
{
  XAxiVdma *XVdmaPtr = XVprocSsPtr->VdmaPtr;
  if(XVdmaPtr) {
    XAxiVdma_DmaStop(XVdmaPtr, XAXIVDMA_WRITE);
    XAxiVdma_DmaStop(XVdmaPtr, XAXIVDMA_READ);
  }
}

/*****************************************************************************/
/**
* This function resets Read and Write channels.
*
* @param  XVprocSsPtr is the pointer to the VPSS instance
*
* @return None
*
*******************************************************************************/
void XVprocSs_VdmaReset(XVprocSs *XVprocSsPtr)
{
  XAxiVdma *XVdmaPtr = XVprocSsPtr->VdmaPtr;
  u32 timeout;

  if(XVdmaPtr) {
    XAxiVdma_Reset(XVdmaPtr, XAXIVDMA_WRITE);
    timeout = XVDMA_RESET_TIMEOUT;
    while(timeout && XAxiVdma_ResetNotDone(XVdmaPtr, XAXIVDMA_WRITE)) {
      timeout -= 1;
    }
    if(!timeout) {
      // ****CRITICAL HW ERR:: VDMA WR CH RESET STUCK HIGH****
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_OPERR_VDMA, XVPROCSS_EDAT_VDMA_WRRES);
    }

    timeout = XVDMA_RESET_TIMEOUT;
    XAxiVdma_Reset(XVdmaPtr, XAXIVDMA_READ);
    while(timeout && XAxiVdma_ResetNotDone(XVdmaPtr, XAXIVDMA_READ)) {
      timeout -= 1;
    }
    if(!timeout) {
      // ****CRITICAL HW ERR:: VDMA RD CH RESET STUCK HIGH****
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_OPERR_VDMA, XVPROCSS_EDAT_VDMA_RDRES);
    }
  }
}


/*****************************************************************************/
/**
*
* This function sets up the write channel
*
* @param	XVprocSsPtr is the pointer to the VPSS instance.
* @param    WrBaseAddress is the address in DDR where frame buffers are located
* @param	window  is pointer to sub-frame window
* @param	FrameWidth is the active width of the stream to be written
* @param	FrameHeight is the active height of the stream to be written
* @param	PixelWidthInBits is Bits Per Pixel required for the stream
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
******************************************************************************/
int XVprocSs_VdmaWriteSetup(XVprocSs *XVprocSsPtr,
		                    UINTPTR WrBaseAddress,
                            XVidC_VideoWindow *window,
                            u32 FrameWidth,
                            u32 FrameHeight,
                            u32 PixelWidthInBits)
{
  XAxiVdma *XVdmaPtr = XVprocSsPtr->VdmaPtr;
  XAxiVdma_DmaSetup WriteCfg;
  int Index;
  UINTPTR Addr;
  int Status;
  u32 HSizeInBytes;
  u32 StrideInBytes;
  u32 BlockOffset;
  u32 StartHPosBytes;
  u32 alignBytes;

  if(XVdmaPtr)
  {
	memset(&WriteCfg, 0, sizeof(XAxiVdma_DmaSetup));
    HSizeInBytes   = (window->Width  * PixelWidthInBits)/8;
    StrideInBytes  = (FrameWidth     * PixelWidthInBits)/8;
    StartHPosBytes = (window->StartX * PixelWidthInBits)/8;

    /* If DMA engine does not support unaligned transfers then align block
     * offset, hsize and stride to next data width boundary (aximm)
     */
    if(!XVdmaPtr->WriteChannel.HasDRE)
    {
      alignBytes = XVdmaPtr->WriteChannel.WordLength-1;

      HSizeInBytes   = (HSizeInBytes   + alignBytes) & ~(alignBytes);
      StrideInBytes  = (StrideInBytes  + alignBytes) & ~(alignBytes);
      StartHPosBytes = (StartHPosBytes + alignBytes) & ~(alignBytes);
    }

    /* Compute start location for sub-frame */
    BlockOffset = ((window->StartY * StrideInBytes) + StartHPosBytes);

    WriteCfg.VertSizeInput = window->Height;
    WriteCfg.HoriSizeInput = HSizeInBytes;

    WriteCfg.Stride = StrideInBytes;
    WriteCfg.FrameDelay = 0;  /* This example does not test frame delay */

    WriteCfg.EnableCircularBuf = 1;
    WriteCfg.EnableSync = 0;  /* No Gen-Lock */

    WriteCfg.PointNum = 0;    /* Master we synchronize with -> vdma instance being worked with */
    WriteCfg.EnableFrameCounter = 0; /* Endless transfers */

    WriteCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */

    WriteCfg.GenLockRepeat = 0; /* Do not repeat previous frame on frame errors */
    Status = XAxiVdma_DmaConfig(XVdmaPtr, XAXIVDMA_WRITE, &WriteCfg);
    if (Status != XST_SUCCESS)
    {
      // Write channel config failed
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFGERR_VDMA, XVPROCSS_EDAT_VDMA_WRCFG);
      return XST_FAILURE;
    }

    /* Initialize buffer addresses
     *
     * Use physical addresses
     */
    Addr = WrBaseAddress + BlockOffset;
    for(Index = 0; Index < XVdmaPtr->MaxNumFrames; Index++)
    {
        WriteCfg.FrameStoreStartAddr[Index] = Addr;
        Addr += StrideInBytes * FrameHeight;
    }

    /* Set the buffer addresses for transfer in the DMA engine */
    Status = XAxiVdma_DmaSetBufferAddr(XVdmaPtr, XAXIVDMA_WRITE,
                                       WriteCfg.FrameStoreStartAddr);
    if (Status != XST_SUCCESS)
    {
      // Write channel set buffer address failed
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFGERR_VDMA, XVPROCSS_EDAT_VDMA_WRADR);
      return XST_FAILURE;
    }
  }
  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function sets up the read channel
*
* @param	XVprocSsPtr is the pointer to the VPSS instance.
* @param    RdBaseAddress is the address in DDR where frame buffers are located
* @param	window  is pointer to sub-frame window
* @param	FrameWidth is the active width of the stream to be read
* @param	FrameHeight is the active height of the stream to be read
* @param	PixelWidthInBits is Bits Per Pixel required for the stream
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
int XVprocSs_VdmaReadSetup(XVprocSs *XVprocSsPtr,
		                   UINTPTR RdBaseAddress,
                           XVidC_VideoWindow *window,
                           u32 FrameWidth,
                           u32 FrameHeight,
                           u32 PixelWidthInBits)
{
  XAxiVdma *XVdmaPtr = XVprocSsPtr->VdmaPtr;
  XAxiVdma_DmaSetup ReadCfg;
  int Index;
  UINTPTR Addr;
  int Status;
  u32 HSizeInBytes;
  u32 StrideInBytes;
  u32 BlockOffset;
  u32 alignBytes;
  u32 StartHPosBytes;

  if(XVdmaPtr)
  {
    HSizeInBytes   = (window->Width  * PixelWidthInBits)/8;
    StrideInBytes  = (FrameWidth     * PixelWidthInBits)/8;
    StartHPosBytes = (window->StartX * PixelWidthInBits)/8;

    /* If DMA engine does not support unaligned transfers then align block
     * offset, hsize and stride
     * Block offset and Stride are aligned to aximm width
     * hsize is aligned to axis width
     */
    if(!XVdmaPtr->ReadChannel.HasDRE)
    {
      alignBytes = XVdmaPtr->ReadChannel.WordLength-1;

      StrideInBytes  = (StrideInBytes  + alignBytes) & ~(alignBytes);
      StartHPosBytes = (StartHPosBytes + alignBytes) & ~(alignBytes);

      /* align hsize to stream width (axis) */
      alignBytes = XVdmaPtr->ReadChannel.StreamWidth;
      HSizeInBytes  = ((HSizeInBytes+alignBytes-1)/alignBytes)*alignBytes;
    }

    //Compute start location for sub-frame
    BlockOffset = ((window->StartY * StrideInBytes) + StartHPosBytes);

    ReadCfg.VertSizeInput = window->Height;
    ReadCfg.HoriSizeInput = HSizeInBytes;

    ReadCfg.Stride = StrideInBytes;
    ReadCfg.FrameDelay = 1;  /* Read is 1 Frame behind write */

    ReadCfg.EnableCircularBuf = 1;
    ReadCfg.EnableSync = 1;  /* Gen-Lock Slave*/

    ReadCfg.PointNum = 0;    /* Master to synchronize with -> vdma instance being worked with */
    ReadCfg.EnableFrameCounter = 0; /* Endless transfers */

    ReadCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */

    Status = XAxiVdma_DmaConfig(XVdmaPtr, XAXIVDMA_READ, &ReadCfg);
    if (Status != XST_SUCCESS)
    {
      // Read channel config failed
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFGERR_VDMA, XVPROCSS_EDAT_VDMA_RDCFG);
      return XST_FAILURE;
    }

    /* Initialize buffer addresses
     *
     * These addresses are physical addresses
     */
    Addr = RdBaseAddress + BlockOffset;
    for(Index = 0; Index < XVdmaPtr->MaxNumFrames; Index++)
    {
      ReadCfg.FrameStoreStartAddr[Index] = Addr;
      Addr += StrideInBytes * FrameHeight;
    }

    /* Set the buffer addresses for transfer in the DMA engine
     * The buffer addresses are physical addresses
     */
    Status = XAxiVdma_DmaSetBufferAddr(XVdmaPtr, XAXIVDMA_READ,
                                       ReadCfg.FrameStoreStartAddr);
    if (Status != XST_SUCCESS)
    {
      // Read channel set buffer address failed
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFGERR_VDMA, XVPROCSS_EDAT_VDMA_RDADR);
      return XST_FAILURE;
    }
  }
  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function starts the transfer on Read/Write channels. Both channels must
* have been configured before this call is made
*
* @param	XVprocSsPtr is the pointer to the VPSS instance.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		Currently the return value is ignored at subsystem level.
*
******************************************************************************/
int XVprocSs_VdmaStartTransfer(XVprocSs *XVprocSsPtr)
{
  XAxiVdma *XVdmaPtr = XVprocSsPtr->VdmaPtr;
  int Status;

  if(XVdmaPtr)
  {
    Status = XAxiVdma_DmaStart(XVdmaPtr, XAXIVDMA_WRITE);
    if (Status != XST_SUCCESS)
    {
      // VDMA ERR:: Start write transfer failed
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_OPERR_VDMA, XVPROCSS_EDAT_VDMA_WRXFR);
      return XST_FAILURE;
    }

    Status = XAxiVdma_DmaStart(XVdmaPtr, XAXIVDMA_READ);
    if (Status != XST_SUCCESS)
    {
      // VDMA ERR:: Start read transfer failed
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_OPERR_VDMA, XVPROCSS_EDAT_VDMA_RDXFR);
      return XST_FAILURE;
    }
  }
  return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function configures the VDMA RD/WR channel for down scale mode
*
* @param  XVprocSsPtr is the pointer to the VPSS instance.
* @param  updateCh defines VDMA channel (RD/WR or RD+WR) to update
*
* @return None
*
******************************************************************************/
void XVprocSs_VdmaSetWinToDnScaleMode(XVprocSs *XVprocSsPtr, u32 updateCh)
{
  XVidC_VideoWindow wrWin, rdWin;
  u32 OutputWidth, OutputHeight;

  OutputWidth  = XVprocSsPtr->VidOut.Timing.HActive;
  OutputHeight = XVprocSsPtr->VidOut.Timing.VActive;

  /*VDMA is After Scaler
    WR client will receive PIP (or down scaled) stream from Scaler
    RD client will read at Output resolution
  */

  if((updateCh == XVPROCSS_VDMA_UPDATE_WR_CH) ||
     (updateCh == XVPROCSS_VDMA_UPDATE_ALL_CH)
    )
  {
    /* Setup WR CLIENT window size */
    if(XVprocSs_IsPipModeOn(XVprocSsPtr))
    {
      /* Read PIP window setting - set elsewhere */
      XVprocSs_GetZoomPipWindow(XVprocSsPtr, XVPROCSS_PIP_WIN, &wrWin);
    }
    else //Normal Downscale mode
    {
      /* set WR client window to output resolution */
      wrWin.StartX = 0;
      wrWin.StartY = 0;
      wrWin.Width  = OutputWidth;
      wrWin.Height = OutputHeight;
    }

    /* write PIP window stream to DDR */
    XVprocSs_VdmaWriteSetup(XVprocSsPtr,
                            XVprocSsPtr->FrameBufBaseaddr,
                            &wrWin,
                            OutputWidth,
                            OutputHeight,
                            XVprocSsPtr->CtxtData.PixelWidthInBits);
  }

  if((updateCh == XVPROCSS_VDMA_UPDATE_RD_CH) ||
     (updateCh == XVPROCSS_VDMA_UPDATE_ALL_CH)
     )
  {
    /* Setup RD CLIENT window size to output resolution */
    rdWin.StartX = 0;
    rdWin.StartY = 0;
    rdWin.Width  = OutputWidth;
    rdWin.Height = OutputHeight;

    XVprocSs_VdmaReadSetup(XVprocSsPtr,
                           XVprocSsPtr->FrameBufBaseaddr,
                           &rdWin,
                           OutputWidth,
                           OutputHeight,
                           XVprocSsPtr->CtxtData.PixelWidthInBits);
  }
}


/*****************************************************************************/
/**
* This function configures the VDMA RD/WR channel for UP scale (or 1:1) mode
*
* @param  XVprocSsPtr is the pointer to the VPSS instance.
* @param  updateCh defines VDMA channel (RD/WR or RD+WR) to update
*
* @return None
*
******************************************************************************/
void XVprocSs_VdmaSetWinToUpScaleMode(XVprocSs *XVprocSsPtr, u32 updateCh)
{
  XVidC_VideoWindow wrWin, rdWin;
  u32 InputWidth, InputHeight;

  InputWidth  = XVprocSsPtr->CtxtData.VidInWidth;
  InputHeight = XVprocSsPtr->CtxtData.VidInHeight;

  /*VDMA is before Scaler
    WR client will receive streaming input
    RD client will read Window from specified coordinates
  */
  if((updateCh == XVPROCSS_VDMA_UPDATE_WR_CH) ||
     (updateCh == XVPROCSS_VDMA_UPDATE_ALL_CH))
  {
    /* Setup WR Client window size to Input Resolution */
    wrWin.StartX = 0;
    wrWin.StartY = 0;
    wrWin.Width  = InputWidth;
    wrWin.Height = InputHeight;

    /* write input stream to DDR */
    XVprocSs_VdmaWriteSetup(XVprocSsPtr,
                            XVprocSsPtr->FrameBufBaseaddr,
                            &wrWin,
                            InputWidth,
                            InputHeight,
                            XVprocSsPtr->CtxtData.PixelWidthInBits);
  }

  if((updateCh == XVPROCSS_VDMA_UPDATE_RD_CH) ||
     (updateCh == XVPROCSS_VDMA_UPDATE_ALL_CH)
     )
  {
    /* Setup RD CLIENT window size to either crop window or input resolution */
    if(XVprocSs_IsZoomModeOn(XVprocSsPtr))
    {
      /* Read user defined ZOOM window */
      XVprocSs_GetZoomPipWindow(XVprocSsPtr, XVPROCSS_ZOOM_WIN, &rdWin);
    }
    else //set RD window to input resolution
    {
      rdWin.StartX = 0;
      rdWin.StartY = 0;
      rdWin.Width  = InputWidth;
      rdWin.Height = InputHeight;
    }

    XVprocSs_VdmaReadSetup(XVprocSsPtr,
                           XVprocSsPtr->FrameBufBaseaddr,
                           &rdWin,
                           InputWidth,
                           InputHeight,
                           XVprocSsPtr->CtxtData.PixelWidthInBits);
  }
}

/*****************************************************************************/
/**
*
* This function prints VDMA status on the console
*
* @param  XVdmaPtr is the instance pointer to the DMA engine.
* @param  Bpp is Bytes per pixel to be used for data transfer
*
* @return	None
*
******************************************************************************/
void XVprocSs_VdmaDbgReportStatus(XVprocSs *XVprocSsPtr, u32 PixelWidthInBits)
{
  u32 height,width,stride;
  u32 regOffset;
  XAxiVdma *XVdmaPtr = XVprocSsPtr->VdmaPtr;

  if(XVdmaPtr)
  {
    xil_printf("\r\n\r\n----->VDMA IP STATUS<----\r\n");
    xil_printf("INFO: VDMA Rd/Wr Client Width/Stride defined in Bytes Per Pixel\r\n");
    xil_printf("Bytes Per Pixel = %d.%d\r\n\r\n", (PixelWidthInBits/8), (PixelWidthInBits%8));
    xil_printf("Read Channel Setting \r\n" );
    //clear status register before reading
    XAxiVdma_ClearDmaChannelErrors(XVdmaPtr, XAXIVDMA_READ, 0xFFFFFFFF);

    //Read Registers
    XAxiVdma_DmaRegisterDump(XVdmaPtr, XAXIVDMA_READ);
    regOffset = XAXIVDMA_MM2S_ADDR_OFFSET;
    height =  XAxiVdma_ReadReg(XVdmaPtr->BaseAddr, regOffset+0);
    width  =  XAxiVdma_ReadReg(XVdmaPtr->BaseAddr, regOffset+4);
    stride =  XAxiVdma_ReadReg(XVdmaPtr->BaseAddr, regOffset+8);
    stride &= XAXIVDMA_STRIDE_MASK;

    xil_printf("Height: %d \r\n", height);
    xil_printf("Width : %d (%d)\r\n", width, (width*8/PixelWidthInBits));
    xil_printf("Stride: %d (%d)\r\n", stride, (stride*8/PixelWidthInBits));

    xil_printf("\r\nWrite Channel Setting \r\n" );
    //clear status register before reading
    XAxiVdma_ClearDmaChannelErrors(XVdmaPtr, XAXIVDMA_WRITE, 0xFFFFFFFF);

    //Read Registers
    XAxiVdma_DmaRegisterDump(XVdmaPtr, XAXIVDMA_WRITE);
    regOffset = XAXIVDMA_S2MM_ADDR_OFFSET;
    height =  XAxiVdma_ReadReg(XVdmaPtr->BaseAddr, regOffset+0);
    width  =  XAxiVdma_ReadReg(XVdmaPtr->BaseAddr, regOffset+4);
    stride =  XAxiVdma_ReadReg(XVdmaPtr->BaseAddr, regOffset+8);
    stride &= XAXIVDMA_STRIDE_MASK;

    xil_printf("Height: %d \r\n", height);
    xil_printf("Width : %d (%d)\r\n", width, (width*8/PixelWidthInBits));
    xil_printf("Stride: %d (%d)\r\n", stride, (stride*8/PixelWidthInBits));
  }
}

/** @} */
