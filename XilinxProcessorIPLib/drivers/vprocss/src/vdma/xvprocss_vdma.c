/******************************************************************************
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xvprocss_vdma.c
* @addtogroup vprocss
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
* 1.00  rco   07/21/15   Initial Release

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xvidc.h"
#include "xvprocss_vdma.h"
#include "microblaze_sleep.h"

#define XVDMA_RESET_TIMEOUT   (1000000) //10ms at 10ns time period (100MHz clock))
/*****************************************************************************/
/**
* This function starts Read and Write channels
*
* @param  pVdma is the pointer to core instance to be worked on
*
* @return None
*
*******************************************************************************/
void XVprocss_VdmaStart(XAxiVdma *pVdma)
{
  if(pVdma)
  {
    XAxiVdma_DmaStart(pVdma, XAXIVDMA_WRITE);
    XAxiVdma_DmaStart(pVdma, XAXIVDMA_READ);
  }
}

/*****************************************************************************/
/**
* This function stops Read and Write channels
*
* @param  pVdma is the pointer to core instance to be worked on
*
* @return None
*
*******************************************************************************/
void XVprocss_VdmaStop(XAxiVdma *pVdma)
{
  if(pVdma)
  {
    XAxiVdma_DmaStop(pVdma, XAXIVDMA_WRITE);
    XAxiVdma_DmaStop(pVdma, XAXIVDMA_READ);
  }
}

/*****************************************************************************/
/**
* This function resets Read and Write channels.
*
* @param  pVdma is the pointer to core instance to be worked on
*
* @return None
*
*******************************************************************************/
void XVprocss_VdmaReset(XAxiVdma *pVdma)
{
  u32 timeout;

  if(pVdma)
  {
    XAxiVdma_Reset(pVdma, XAXIVDMA_WRITE);
    timeout = XVDMA_RESET_TIMEOUT;
    while(timeout && XAxiVdma_ResetNotDone(pVdma, XAXIVDMA_WRITE))
    {
      timeout -= 1;
    }
    if(!timeout)
    {
      xil_printf("\r\n****CRITICAL HW ERR:: VDMA WR CH RESET STUCK HIGH****\r\n");
    }

    timeout = XVDMA_RESET_TIMEOUT;
    XAxiVdma_Reset(pVdma, XAXIVDMA_READ);
    while(timeout && XAxiVdma_ResetNotDone(pVdma, XAXIVDMA_READ))
    {
      timeout -= 1;
    }
    if(!timeout)
    {
      xil_printf("\r\n****CRITICAL HW ERR:: VDMA RD CH RESET STUCK HIGH****\r\n");
    }
  }
}


/*****************************************************************************/
/**
*
* This function sets up the write channel
*
* @param	pVdma is the instance pointer to the DMA engine.
* @param    WrBaseAddress is the address in DDR where frame buffers are located
* @param	window  is pointer to sub-frame window
* @param	FrameWidth is the active width of the stream to be written
* @param	FrameHeight is the active height of the stream to be written
* @param	Bpp is Bytes Per Pixel required for the stream to be written
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
******************************************************************************/
int XVprocss_VdmaWriteSetup(XAxiVdma *pVdma,
                            u32 WrBaseAddress,
                            XVidC_VideoWindow *window,
                            u32 FrameWidth,
                            u32 FrameHeight,
                            u32 Bpp)
{
  XAxiVdma_DmaSetup WriteCfg;
  int Index;
  u32 Addr;
  int Status;
  u32 HSizeInBytes;
  u32 StrideInBytes;;
  u32 BlockOffset;
  u32 alignBytes = 0;
  int MAX_WR_FRAMES;

  if(pVdma)
  {
    MAX_WR_FRAMES  = pVdma->MaxNumFrames;
    HSizeInBytes   = window->Width*Bpp;
    StrideInBytes  = FrameWidth*Bpp;

    /* Compute start location for sub-frame */
    BlockOffset = (window->StartY * StrideInBytes) + window->StartX*Bpp;

    /* If DMA engine does not support unaligned transfers then align block
     * offset to next data width boundary (DMA WR Data-width = 128)
     */
    if(!pVdma->WriteChannel.HasDRE)
    {
      alignBytes = pVdma->WriteChannel.WordLength-1;
      BlockOffset = (BlockOffset+alignBytes) & ~(alignBytes);
    }

    WriteCfg.VertSizeInput = window->Height;
    WriteCfg.HoriSizeInput = HSizeInBytes;

    WriteCfg.Stride = StrideInBytes;
    WriteCfg.FrameDelay = 0;  /* This example does not test frame delay */

    WriteCfg.EnableCircularBuf = 1;
    WriteCfg.EnableSync = 0;  /* No Gen-Lock */

    WriteCfg.PointNum = 0;    /* Master we synchronize with -> vdma instance being worked with */
    WriteCfg.EnableFrameCounter = 0; /* Endless transfers */

    WriteCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */

    WriteCfg.GenLockRepeat = 1; /* Repeat previous frame on frame errors */
    Status = XAxiVdma_DmaConfig(pVdma, XAXIVDMA_WRITE, &WriteCfg);
    if (Status != XST_SUCCESS)
    {
      xil_printf("Write channel config failed %d\r\n", Status);
      return XST_FAILURE;
    }

    /* Initialize buffer addresses
     *
     * Use physical addresses
     */
    Addr = WrBaseAddress + BlockOffset;
    for(Index = 0; Index < MAX_WR_FRAMES; Index++)
    {
        WriteCfg.FrameStoreStartAddr[Index] = Addr;
        Addr += StrideInBytes * FrameHeight;
    }

    /* Set the buffer addresses for transfer in the DMA engine */
    Status = XAxiVdma_DmaSetBufferAddr(pVdma, XAXIVDMA_WRITE,
                                       WriteCfg.FrameStoreStartAddr);
    if (Status != XST_SUCCESS)
    {
      xil_printf("Write channel set buffer address failed %d\r\n", Status);
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
* @param	pVdma is the instance pointer to the DMA engine.
* @param    RdBaseAddress is the address in DDR where frame buffers are located
* @param	window  is pointer to sub-frame window
* @param	FrameWidth is the active width of the stream to be read
* @param	FrameHeight is the active height of the stream to be read
* @param	Bpp is Bytes Per Pixel required for the stream to be read
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
int XVprocss_VdmaReadSetup(XAxiVdma *pVdma,
                           u32 RdBaseAddress,
                           XVidC_VideoWindow *window,
                           u32 FrameWidth,
                           u32 FrameHeight,
                           u32 Bpp)
{
  XAxiVdma_DmaSetup ReadCfg;
  int Index;
  u32 Addr;
  int Status;
  u32 HSizeInBytes;
  u32 StrideInBytes;
  u32 BlockOffset;
  u32 alignBytes = 0;
  int MAX_RD_FRAMES = pVdma->MaxNumFrames;

  if(pVdma)
  {
    MAX_RD_FRAMES = pVdma->MaxNumFrames;
    HSizeInBytes  = window->Width*Bpp;
    StrideInBytes = FrameWidth*Bpp;

    //Compute start location for sub-frame
    BlockOffset = (window->StartY * StrideInBytes) + window->StartX*Bpp;

    /* If DMA engine does not support unaligned transfers then align block
     * offset to next data width boundary (DMA WR Data-width = 128)
     */
    if(!pVdma->ReadChannel.HasDRE)
    {
      alignBytes = pVdma->ReadChannel.WordLength-1;
      BlockOffset = (BlockOffset+alignBytes) & ~(alignBytes);
    }

    ReadCfg.VertSizeInput = window->Height;
    ReadCfg.HoriSizeInput = HSizeInBytes;

    ReadCfg.Stride = StrideInBytes;
    ReadCfg.FrameDelay = 1;  /* Read is 1 Frame behind write */

    ReadCfg.EnableCircularBuf = 1;
    ReadCfg.EnableSync = 1;  /* Gen-Lock Slave*/

    ReadCfg.PointNum = 0;    /* Master to synchronize with -> vdma instance being worked with */
    ReadCfg.EnableFrameCounter = 0; /* Endless transfers */

    ReadCfg.FixedFrameStoreAddr = 0; /* We are not doing parking */

    Status = XAxiVdma_DmaConfig(pVdma, XAXIVDMA_READ, &ReadCfg);
    if (Status != XST_SUCCESS)
    {
      xil_printf("Read channel config failed %d\r\n", Status);
      return XST_FAILURE;
    }

    /* Initialize buffer addresses
     *
     * These addresses are physical addresses
     */
    Addr = RdBaseAddress + BlockOffset;
    for(Index = 0; Index < MAX_RD_FRAMES; Index++)
    {
      ReadCfg.FrameStoreStartAddr[Index] = Addr;
      Addr += StrideInBytes * FrameHeight;
    }

    /* Set the buffer addresses for transfer in the DMA engine
     * The buffer addresses are physical addresses
     */
    Status = XAxiVdma_DmaSetBufferAddr(pVdma, XAXIVDMA_READ,
                                       ReadCfg.FrameStoreStartAddr);
    if (Status != XST_SUCCESS)
    {
      xil_printf("Read channel set buffer address failed %d\r\n", Status);
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
* @param	pVdma is the instance pointer to the DMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		Currently the return value is ignored at subsystem level.
*
******************************************************************************/
int XVprocss_VdmaStartTransfer(XAxiVdma *pVdma)
{
  int Status;

  if(pVdma)
  {
    Status = XAxiVdma_DmaStart(pVdma, XAXIVDMA_WRITE);
    if (Status != XST_SUCCESS)
    {
      xil_printf("VDMA ERR:: Start Write transfer failed %d\r\n", Status);
      return XST_FAILURE;
    }

    Status = XAxiVdma_DmaStart(pVdma, XAXIVDMA_READ);
    if (Status != XST_SUCCESS)
    {
      xil_printf("VDMA ERR:: Start read transfer failed %d\r\n", Status);
      return XST_FAILURE;
    }
  }
  return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function prints VDMA status on the console
*
* @param  pVdma is the instance pointer to the DMA engine.
* @param  Bpp is Bytes per pixel to be used for data transfer
*
* @return	None
*
******************************************************************************/
void XVprocss_VdmaDbgReportStatus(XAxiVdma *pVdma, u32 Bpp)
{
  u32 height,width,stride;
  u32 regOffset;

  if(pVdma)
  {
    xil_printf("\r\n\r\n----->VDMA IP STATUS<----\r\n");
    xil_printf("INFO: VDMA Rd/Wr Client Width/Stride defined in Bytes Per Pixel\r\n");
    xil_printf("Bytes Per Pixel = %d\r\n\r\n", Bpp);
    xil_printf("Read Channel Setting \r\n" );
    //clear status register before reading
    XAxiVdma_ClearDmaChannelErrors(pVdma, XAXIVDMA_READ, 0xFFFFFFFF);
    MB_Sleep(100);

    //Read Registers
    XAxiVdma_DmaRegisterDump(pVdma, XAXIVDMA_READ);
    regOffset = XAXIVDMA_MM2S_ADDR_OFFSET;
    height =  XAxiVdma_ReadReg(pVdma->BaseAddr, regOffset+0);
    width  =  XAxiVdma_ReadReg(pVdma->BaseAddr, regOffset+4);
    stride =  XAxiVdma_ReadReg(pVdma->BaseAddr, regOffset+8);
    stride &= XAXIVDMA_STRIDE_MASK;

    xil_printf("Height: %d \r\n", height);
    xil_printf("Width : %d (%d)\r\n", width, (width/Bpp));
    xil_printf("Stride: %d (%d)\r\n", stride, (stride/Bpp));

    xil_printf("\r\nWrite Channel Setting \r\n" );
    //clear status register before reading
    XAxiVdma_ClearDmaChannelErrors(pVdma, XAXIVDMA_WRITE, 0xFFFFFFFF);
    MB_Sleep(100);

    //Read Registers
    XAxiVdma_DmaRegisterDump(pVdma, XAXIVDMA_WRITE);
    regOffset = XAXIVDMA_S2MM_ADDR_OFFSET;
    height =  XAxiVdma_ReadReg(pVdma->BaseAddr, regOffset+0);
    width  =  XAxiVdma_ReadReg(pVdma->BaseAddr, regOffset+4);
    stride =  XAxiVdma_ReadReg(pVdma->BaseAddr, regOffset+8);
    stride &= XAXIVDMA_STRIDE_MASK;

    xil_printf("Height: %d \r\n", height);
    xil_printf("Width : %d (%d)\r\n", width, (width/Bpp));
    xil_printf("Stride: %d (%d)\r\n", stride, (stride/Bpp));
  }
}

/*****************************************************************************/
/**
* This function configures the VDMA RD/WR channel for down scale mode
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
* @param  updateCh defines VDMA channel (RD/WR or RD+WR) to update
*
* @return None
*
******************************************************************************/
void XVprocss_VdmaSetWinToDnScaleMode(XVprocss *pVprocss, u32 updateCh)
{
  XVidC_VideoWindow wrWin, rdWin;
  u32 OutputWidth, OutputHeight;
  int status;

  OutputWidth  = pVprocss->VidOut.Timing.HActive;
  OutputHeight = pVprocss->VidOut.Timing.VActive;

  /*VDMA is After Scaler
    WR client will receive PIP (or down scaled) stream from Scaler
    RD client will read at Output resolution
  */

  if((updateCh == XVPROCSS_VDMA_UPDATE_WR_CH) ||
     (updateCh == XVPROCSS_VDMA_UPDATE_ALL_CH)
    )
  {
    /* Setup WR CLIENT window size */
    if(XVprocss_IsPipModeOn(pVprocss))
    {
      /* Read PIP window setting - set elsewhere */
      XVprocss_GetZoomPipWindow(pVprocss, XVPROCSS_PIP_WIN, &wrWin);
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
    status = XVprocss_VdmaWriteSetup(pVprocss->vdma,
                                pVprocss->FrameBufBaseaddr,
                                &wrWin,
                                OutputWidth,
                                OutputHeight,
                                pVprocss->idata.vdmaBytesPerPixel);
    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Unable to configure VDMA Write Channel \r\n");
    }
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

    status = XVprocss_VdmaReadSetup(pVprocss->vdma,
                               pVprocss->FrameBufBaseaddr,
                               &rdWin,
                               OutputWidth,
                               OutputHeight,
                               pVprocss->idata.vdmaBytesPerPixel);
    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Unable to configure VDMA Read Channel \r\n");
    }
  }
}


/*****************************************************************************/
/**
* This function configures the VDMA RD/WR channel for UP scale (or 1:1) mode
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
* @param  updateCh defines VDMA channel (RD/WR or RD+WR) to update
*
* @return None
*
******************************************************************************/
void XVprocss_VdmaSetWinToUpScaleMode(XVprocss *pVprocss, u32 updateCh)
{
  XVidC_VideoWindow wrWin, rdWin;
  u32 InputWidth, InputHeight;
  int status;

  InputWidth  = pVprocss->idata.vidInWidth;
  InputHeight = pVprocss->idata.vidInHeight;

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
    status = XVprocss_VdmaWriteSetup(pVprocss->vdma,
                                pVprocss->FrameBufBaseaddr,
                                &wrWin,
                                InputWidth,
                                InputHeight,
                                pVprocss->idata.vdmaBytesPerPixel);
    if(status != XST_SUCCESS)
    {
      xil_printf("ERR:: Unable to configure VDMA Write Channel \r\n");
    }
  }

  if((updateCh == XVPROCSS_VDMA_UPDATE_RD_CH) ||
     (updateCh == XVPROCSS_VDMA_UPDATE_ALL_CH)
     )
  {
    /* Setup RD CLIENT window size to either crop window or input resolution */
    if(XVprocss_IsZoomModeOn(pVprocss))
    {
      /* Read user defined ZOOM window */
      XVprocss_GetZoomPipWindow(pVprocss, XVPROCSS_ZOOM_WIN, &rdWin);
    }
    else //set RD window to input resolution
    {
      rdWin.StartX = 0;
      rdWin.StartY = 0;
      rdWin.Width  = InputWidth;
      rdWin.Height = InputHeight;
    }

    status = XVprocss_VdmaReadSetup(pVprocss->vdma,
                               pVprocss->FrameBufBaseaddr,
                               &rdWin,
                               InputWidth,
                               InputHeight,
                               pVprocss->idata.vdmaBytesPerPixel);
    if(status != XST_SUCCESS)
    {
      xil_printf("ERR:: Unable to configure VDMA Read Channel \r\n");
    }
  }
}
/** @} */
