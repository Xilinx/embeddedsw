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
* @file xvprocss_router.c
* @addtogroup vprocss_v1_0
* @{

* Video buffer management routine.
* The functions in this file provides an abstraction from the register peek/poke
* methodology by implementing most common use-case provided by the VDMA sub-core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco  07/21/15   Initial Release

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xvidc.h"
#include "xvprocss_router.h"
#include "xvprocss_vdma.h"

/************************** Constant Definitions *****************************/
/* AXIS Switch Port# connected to input stream */
#define AXIS_SWITCH_VIDIN_S0              (0)

/************************** Function Prototypes ******************************/
static int validateWindowSize(const XVidC_VideoWindow *win,
		                      const u32 HActive,
		                      const u32 VActive);

static XVprocss_ScaleMode GetScalingMode(XVprocss *pVprocss);


/*****************************************************************************/
/**
* This function checks to make sure sub-frame is inside full frame
*
* @param  win is a pointer to the sub-frame window
* @param  Resolution is a pointer to the current output resolution
*
* @return XST_SUCCESS if window is valid else XST_FAILURE
*
******************************************************************************/
static int validateWindowSize(const XVidC_VideoWindow *win,
                              const u32 HActive,
                              const u32 VActive)
{
  Xil_AssertNonvoid(win != NULL);

  //Check if window is valid
  if((win->Width == 0) || (win->Height == 0))
  {
	  return(XST_FAILURE);
  }

  //Check if window is within the Resolution being programmed
  if(((win->StartX > HActive)) ||
	 ((win->StartY > VActive)) ||
     ((win->StartX + win->Width) > HActive)   ||
     ((win->StartY + win->Height) > VActive))
  {
    return(XST_FAILURE);
  }
  else
  {
    return(XST_SUCCESS);
  }
}

/*****************************************************************************/
/**
* This function computes the scaling mode based on input/output stream
* resolution. It also accounts for Zoom or PIP mode, if enabled
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return Scaling mode Up/Dpwn or 1:1
*
******************************************************************************/
static XVprocss_ScaleMode GetScalingMode(XVprocss *pVprocss)
{
  int status;
  XVprocss_ScaleMode mode;
  XVidC_VideoWindow win;
  XVidC_VideoStream *pStrIn  = &pVprocss->VidIn;
  XVidC_VideoStream *pStrOut = &pVprocss->VidOut;

  if(XVprocss_IsPipModeOn(pVprocss))
  {
    /* Read PIP window setting - set elsewhere */
    XVprocss_GetZoomPipWindow(pVprocss, XVPROCSS_PIP_WIN, &win);
    /* validate window */
    status = validateWindowSize(&win,
                                pVprocss->VidOut.Timing.HActive,
                                pVprocss->VidOut.Timing.VActive);
    if(status != XST_SUCCESS)
    {
      xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: VDMA Write Channel Window Invalid \r\n");
	  return(XVPROCSS_SCALE_NOT_SUPPORTED);
    }
    else
    {
      xdbg_printf(XDBG_DEBUG_GENERAL,"\r\n PIP Mode ON: Scale %dx%d -> %dx%d window in output stream\r\n",
		            pStrIn->Timing.HActive,
		            pStrIn->Timing.VActive,
		            pVprocss->idata.wrWindow.Width,
		            pVprocss->idata.wrWindow.Height);

      return(XVPROCSS_SCALE_DN);
    }
  }

  if(XVprocss_IsZoomModeOn(pVprocss))
  {
    /* Read PIP window setting - set elsewhere */
    XVprocss_GetZoomPipWindow(pVprocss, XVPROCSS_ZOOM_WIN, &win);
    /* validate window */
    status = validateWindowSize(&win,
	                            pStrIn->Timing.HActive,
	                            pStrIn->Timing.VActive);
    if(status != XST_SUCCESS)
    {
      xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: VDMA Read Channel Window Invalid \r\n");
	  return(XVPROCSS_SCALE_NOT_SUPPORTED);
    }
    else
    {
      xdbg_printf(XDBG_DEBUG_GENERAL,"\r\n Zoom Mode ON: Scale %dx%d window from Input Stream -> %dx%d\r\n",
                    pVprocss->idata.rdWindow.Width,
                    pVprocss->idata.rdWindow.Height,
		            pStrOut->Timing.HActive,
		            pStrOut->Timing.VActive);

      return (XVPROCSS_SCALE_UP);
    }
  }

  /* Pip & Zoom mode are off. Check input/output resolution */
  if((pStrIn->Timing.HActive > pStrOut->Timing.HActive) ||
     (pStrIn->Timing.VActive > pStrOut->Timing.VActive))
  {
    mode = XVPROCSS_SCALE_DN;
  }
  else if((pStrIn->Timing.HActive < pStrOut->Timing.HActive) ||
          (pStrIn->Timing.VActive < pStrOut->Timing.VActive))
  {
    mode = XVPROCSS_SCALE_UP;
  }
  else
  {
    mode = XVPROCSS_SCALE_1_1;
  }
  return(mode);
}

/*****************************************************************************/
/**
* This function examines the subsystem Input/Output Stream configuration and
* builds a routing table for the supported use-case. The computed routing
* table is stored in the scratch pad memory
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if routing table can be created else XST_FAILURE
*
******************************************************************************/
int XVprocss_BuildRoutingTable(XVprocss *pVprocss)
{
#ifdef DEBUG
  const char *ipStr[XVPROCSS_RTR_MAX] =
  {
    "VID_OUT",
    "SCALER-V",
    "SCALER-H",
    "VDMA",
    "LBOX",
    "CR-H",
    "CR-VIn",
    "CR-VOut",
    "CSC",
    "DEINT",
  };
#endif

  u32 index = 0;
  XVidC_VideoStream *pStrIn  = &pVprocss->VidIn;
  XVidC_VideoStream *pStrOut = &pVprocss->VidOut;
  XVprocss_IData *pCfg    = &pVprocss->idata;
  u8 *pTable              = &pVprocss->idata.RtngTable[0];
  int status = XST_SUCCESS;

  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Build AXIS Routing Map for Subsystem Use-Case.... \r\n");

  /* Save input resolution */
  pCfg->vidInWidth  = pStrIn->Timing.HActive;
  pCfg->vidInHeight = pStrIn->Timing.VActive;
  pCfg->strmCformat = pStrIn->ColorFormatId;

  /* Determine Scaling Mode */
  pCfg->memEn = FALSE;
  pCfg->ScaleMode = GetScalingMode(pVprocss);
  if(pCfg->ScaleMode == XVPROCSS_SCALE_NOT_SUPPORTED)
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Scaling Mode not supported\r\n");
    return(XST_FAILURE);
  }

  /* Reset Routing Table */
  memset(pTable, 0, sizeof(pCfg->RtngTable));

  /* Check if input is I/P */
  if(pStrIn->IsInterlaced)
  {
    pTable[index++] = XVPROCSS_RTR_DEINT;
  }

  /* Check if input is 420 */
  if(pStrIn->ColorFormatId == XVIDC_CSF_YCRCB_420)
  {
    //up-sample vertically to 422 as none of the IP supports 420
    pTable[index++] = XVPROCSS_RTR_CR_V_IN;
    pCfg->strmCformat = XVIDC_CSF_YCRCB_422;
  }

  switch(pCfg->ScaleMode)
  {
    case XVPROCSS_SCALE_1_1:
        pTable[index++] = XVPROCSS_RTR_VDMA;
        pCfg->memEn = TRUE;
        break;

    case XVPROCSS_SCALE_UP:
        pTable[index++] = XVPROCSS_RTR_VDMA;     /* VDMA is before Scaler */
        pTable[index++] = XVPROCSS_RTR_SCALER_V;
        pTable[index++] = XVPROCSS_RTR_SCALER_H;
        pCfg->memEn = TRUE;
        break;

    case XVPROCSS_SCALE_DN:
        pTable[index++] = XVPROCSS_RTR_SCALER_H;
        pTable[index++] = XVPROCSS_RTR_SCALER_V;
        pTable[index++] = XVPROCSS_RTR_VDMA;     /* VDMA is after Scaler */
        pCfg->memEn = TRUE;
        break;

    default:
	xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Scaling Mode cannot be determined.\r\n");
        return(XST_FAILURE);
        break;
  }

  /* Send stream to LBox to add H/V bars, if needed */
  pTable[index++] = XVPROCSS_RTR_LBOX;

  /* Check for input and output color format to derive required conversions */
  switch(pStrOut->ColorFormatId)
  {
      case XVIDC_CSF_YCRCB_420:
         switch(pStrIn->ColorFormatId)
         {
           case XVIDC_CSF_RGB:
              pTable[index++] = XVPROCSS_RTR_CSC;      //convert RGB->444
              pTable[index++] = XVPROCSS_RTR_CR_H;     //convert 444->422
              pTable[index++] = XVPROCSS_RTR_CR_V_OUT; //convert 422->420
              pCfg->cscIn  = XVIDC_CSF_RGB;
              pCfg->cscOut = XVIDC_CSF_YCRCB_444;
              pCfg->hcrIn  = XVIDC_CSF_YCRCB_444;
              pCfg->hcrOut = XVIDC_CSF_YCRCB_422;
              break;

           case XVIDC_CSF_YCRCB_444:
              pTable[index++] = XVPROCSS_RTR_CSC;      //picture control in 444
              pTable[index++] = XVPROCSS_RTR_CR_H;     //convert 444->422
              pTable[index++] = XVPROCSS_RTR_CR_V_OUT; //convert 422->420
              pCfg->cscIn  = XVIDC_CSF_YCRCB_444;
              pCfg->cscOut = XVIDC_CSF_YCRCB_444;
              pCfg->hcrIn  = XVIDC_CSF_YCRCB_444;
              pCfg->hcrOut = XVIDC_CSF_YCRCB_422;
              break;

           case XVIDC_CSF_YCRCB_422:
           case XVIDC_CSF_YCRCB_420: //Input was up converted to 422
              pTable[index++] = XVPROCSS_RTR_CSC;      //picture control in 422
              pTable[index++] = XVPROCSS_RTR_CR_V_OUT; //convert 422->420
              pCfg->cscIn  = XVIDC_CSF_YCRCB_422;
              pCfg->cscOut = XVIDC_CSF_YCRCB_422;
              break;

           default: //Unsupported color format
		  xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Input Color Format Not Supported \r\n");
              status = XST_FAILURE;
              break;
         }
         break;

      case XVIDC_CSF_RGB:
         switch(pStrIn->ColorFormatId)
         {
           case XVIDC_CSF_RGB:
           case XVIDC_CSF_YCRCB_444:  //convert 444->RGB
              pTable[index++] = XVPROCSS_RTR_CSC;
              pCfg->cscIn  = pStrIn->ColorFormatId;
              pCfg->cscOut = XVIDC_CSF_RGB;
              break;

           case XVIDC_CSF_YCRCB_422:
           case XVIDC_CSF_YCRCB_420: //Input was up converted to 422
              pTable[index++] = XVPROCSS_RTR_CR_H;     //convert 422->444
              pTable[index++] = XVPROCSS_RTR_CSC;      //convert 444->RGB
              pCfg->hcrIn  = XVIDC_CSF_YCRCB_422;
              pCfg->hcrOut = XVIDC_CSF_YCRCB_444;
              pCfg->cscIn  = XVIDC_CSF_YCRCB_444;
              pCfg->cscOut = XVIDC_CSF_RGB;
              break;

           default: //Unsupported color format
		  xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Input Color Format Not Supported \r\n");
              status = XST_FAILURE;
              break;
         }
         break;

      case XVIDC_CSF_YCRCB_422:
         switch(pStrIn->ColorFormatId)
         {
           case XVIDC_CSF_RGB:
              pTable[index++] = XVPROCSS_RTR_CSC;      //convert RGB->444
              pTable[index++] = XVPROCSS_RTR_CR_H;     //convert 444->422
              pCfg->cscIn  = XVIDC_CSF_RGB;
              pCfg->cscOut = XVIDC_CSF_YCRCB_444;
              pCfg->hcrIn  = XVIDC_CSF_YCRCB_444;
              pCfg->hcrOut = XVIDC_CSF_YCRCB_422;
              break;

           case XVIDC_CSF_YCRCB_444:
              pTable[index++] = XVPROCSS_RTR_CSC;      //picture control in 444
              pTable[index++] = XVPROCSS_RTR_CR_H;     //convert 444->422
              pCfg->cscIn  = XVIDC_CSF_YCRCB_444;
              pCfg->cscOut = XVIDC_CSF_YCRCB_444;
              pCfg->hcrIn  = XVIDC_CSF_YCRCB_444;
              pCfg->hcrOut = XVIDC_CSF_YCRCB_422;
              break;

           case XVIDC_CSF_YCRCB_422:
           case XVIDC_CSF_YCRCB_420: //Input was up converted to 422
              pTable[index++] = XVPROCSS_RTR_CSC;      //picture control in 422
              pCfg->cscIn  = XVIDC_CSF_YCRCB_422;
              pCfg->cscOut = XVIDC_CSF_YCRCB_422;
              break;

           default: //Unsupported color format
		  xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Input Color Format Not Supported \r\n");
              status = XST_FAILURE;
              break;
         }
         break;

      case XVIDC_CSF_YCRCB_444:
         switch(pStrIn->ColorFormatId)
         {
           case XVIDC_CSF_RGB:        //convert 444->RGB
           case XVIDC_CSF_YCRCB_444:
              pTable[index++] = XVPROCSS_RTR_CSC;
              pCfg->cscIn  = pStrIn->ColorFormatId;
              pCfg->cscOut = XVIDC_CSF_YCRCB_444;
              break;

           case XVIDC_CSF_YCRCB_422:
           case XVIDC_CSF_YCRCB_420: //Input was up converted to 422
              pTable[index++] = XVPROCSS_RTR_CR_H;     //convert 422->444
              pTable[index++] = XVPROCSS_RTR_CSC;      //picture control
              pCfg->hcrIn  = XVIDC_CSF_YCRCB_422;
              pCfg->hcrOut = XVIDC_CSF_YCRCB_444;
              pCfg->cscIn  = XVIDC_CSF_YCRCB_444;
              pCfg->cscOut = XVIDC_CSF_YCRCB_444;
              break;

           default: //Unsupported color format
		  xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Input Color Format Not Supported \r\n");
              status = XST_FAILURE;
              break;
         }
         break;

      default:
	  xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Output Color Format Not Supported \r\n");
        status = XST_FAILURE;
        break;
  }

  /* Connect Last IP in chain to switch output */
  pTable[index++] = XVPROCSS_RTR_VIDOUT;

  /* save number of cores in processing path */
  pCfg->RtrNumCores = index;

#ifdef DEBUG
  if(status == XST_SUCCESS)
  {
    u32 count = 0;

    //print IP Data Flow Map
    xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nGenerated Map: VidIn");
    while(count<index)
    {
      xdbg_printf(XDBG_DEBUG_GENERAL," -> %s",ipStr[pTable[count++]]);
    }
    xdbg_printf(XDBG_DEBUG_GENERAL,"\r\n\r\n");
  }
#endif

  return(status);
}

/*****************************************************************************/
/**
* This function traverses the computed routing table and sets up the AXIS
* switch registers, to route the stream through processing cores, in the order
* defined in the routing map
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XVprocss_ProgRouterMux(XVprocss *pVprocss)
{
  u32 count, nextMi, prevSi;
  u8 *pTable = &pVprocss->idata.RtngTable[0];
  u32 numProcElem = pVprocss->idata.RtrNumCores;

  XAxisScr_RegUpdateDisable(pVprocss->router);

  /* Disable all ports */
  XAxisScr_MiPortDisableAll(pVprocss->router);

  /* Connect Input Stream to the 1st core in path */
  nextMi = prevSi = pTable[0];
  XAxisScr_MiPortEnable(pVprocss->router, nextMi, AXIS_SWITCH_VIDIN_S0);

  /* Traverse routing map and connect cores in the chain */
  for(count=1; count<numProcElem; ++count)
  {
    nextMi  = pTable[count];
    XAxisScr_MiPortEnable(pVprocss->router, nextMi, prevSi);
    prevSi = nextMi;
  }

  //Enable Router register update
  XAxisScr_RegUpdateEnable(pVprocss->router);
}

/*****************************************************************************/
/**
* This function traverses the routing map built earlier and configures each
* sub-core in the processing path per its location in the chain.
* Each core in the processing path is marked and only marked cores are started
* All remaining cores stay disabled
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XVprocss_SetupRouterDataFlow(XVprocss *pVprocss)
{
  XVidC_VideoWindow lboxWin;
  u32 vsc_WidthIn, vsc_HeightIn, vsc_HeightOut;
  u32 hsc_HeightIn, hsc_WidthIn, hsc_WidthOut;
  u32 count;
  XVprocss_IData *pCfg = &pVprocss->idata;
  u8 *pTable = &pVprocss->idata.RtngTable[0];
  u8 *pStartCore = &pVprocss->idata.startCore[0];
  vsc_WidthIn = vsc_HeightIn = vsc_HeightOut = 0;
  hsc_HeightIn = hsc_WidthIn = hsc_WidthOut = 0;

  /* Program Video Pipe Sub-Cores */
  if(pVprocss->VidIn.IsInterlaced)
  {
    /* Input will de-interlaced first. All downstream IP's work
     * with progressive frame. Adjust active height to reflect the
     * progressive frame to downstream cores
     */
	pCfg->vidInHeight *= 2;
  }

  /* If Vdma is enabled, RD/WR Client needs to be programmed before Scaler */
  if((pVprocss->vdma) && (pCfg->memEn))
  {
    switch(pCfg->ScaleMode)
    {
      case XVPROCSS_SCALE_1_1:
      case XVPROCSS_SCALE_UP:
          XVprocss_VdmaSetWinToUpScaleMode(pVprocss, XVPROCSS_VDMA_UPDATE_ALL_CH);
          break;

      case XVPROCSS_SCALE_DN:
	      XVprocss_VdmaSetWinToDnScaleMode(pVprocss, XVPROCSS_VDMA_UPDATE_ALL_CH);
          break;

      default:
          break;
    }
    pStartCore[XVPROCSS_RTR_VDMA] = TRUE;
  }

  for(count=0; count<pCfg->RtrNumCores; ++count)
  {
    switch(pTable[count])
    {
      case XVPROCSS_RTR_SCALER_V:
          if(pVprocss->vscaler)
          {
            if(pCfg->ScaleMode == XVPROCSS_SCALE_DN)
            {
              /* Downscale mode H Scaler is before V Scaler */
              vsc_WidthIn   = hsc_WidthOut;
              vsc_HeightIn  = hsc_HeightIn;
              vsc_HeightOut = ((XVprocss_IsPipModeOn(pVprocss)) ? pCfg->wrWindow.Height
                                                                : pVprocss->VidOut.Timing.VActive);
            }
            else
            {
              /* UpScale mode V Scaler is before H Scaler */
              vsc_WidthIn  = ((XVprocss_IsZoomModeOn(pVprocss)) ? pCfg->rdWindow.Width
                                                                : pCfg->vidInWidth);
              vsc_HeightIn = ((XVprocss_IsZoomModeOn(pVprocss)) ? pCfg->rdWindow.Height
                                                                : pCfg->vidInHeight);
              vsc_HeightOut = pVprocss->VidOut.Timing.VActive;
            }

            xdbg_printf(XDBG_DEBUG_GENERAL,"  -> Configure VScaler for %dx%d to %dx%d\r\n", \
                           (int)vsc_WidthIn, (int)vsc_HeightIn, (int)vsc_WidthIn, (int)vsc_HeightOut);

            XV_VScalerSetup(pVprocss->vscaler,
			                &pVprocss->vscL2Reg,
                            vsc_WidthIn,
                            vsc_HeightIn,
                            vsc_HeightOut);
            pStartCore[XVPROCSS_RTR_SCALER_V] = TRUE;
          }
          break;

      case XVPROCSS_RTR_SCALER_H:
          if(pVprocss->hscaler)
          {
            if(pCfg->ScaleMode == XVPROCSS_SCALE_DN)
            {
              /* Downscale mode H Scaler is before V Scaler */
              hsc_WidthIn  = pCfg->vidInWidth;
              hsc_HeightIn = pCfg->vidInHeight;
              hsc_WidthOut = ((XVprocss_IsPipModeOn(pVprocss)) ? pCfg->wrWindow.Width
                                                               : pVprocss->VidOut.Timing.HActive);
            }
            else
            {
              /* Upscale mode V Scaler is before H Scaler */
              hsc_WidthIn  = vsc_WidthIn;
              hsc_HeightIn = vsc_HeightOut;
              hsc_WidthOut = pVprocss->VidOut.Timing.HActive;
            }

            xdbg_printf(XDBG_DEBUG_GENERAL,"  -> Configure HScaler for %dx%d to %dx%d\r\n", \
                               (int)hsc_WidthIn, (int)hsc_HeightIn, (int)hsc_WidthOut, (int)hsc_HeightIn);

            XV_HScalerSetup(pVprocss->hscaler,
			                &pVprocss->hscL2Reg,
                            hsc_HeightIn,
                            hsc_WidthIn,
                            hsc_WidthOut,
                            pVprocss->idata.strmCformat);
            pStartCore[XVPROCSS_RTR_SCALER_H] = TRUE;
          }
          break;

      case XVPROCSS_RTR_VDMA:
          /* NOP - Programmed before the switch statement */
          break;

      case XVPROCSS_RTR_LBOX:
          if(pVprocss->lbox)
          {
            if(XVprocss_IsPipModeOn(pVprocss))
            {
              /* get the active window for lbox */
              lboxWin = pCfg->wrWindow;
            }
            else //Downscale - Read full image from VDMA
            {
              /* window is same as output resolution */
              lboxWin.StartX = 0;
              lboxWin.StartY = 0;
              lboxWin.Width  = pVprocss->VidOut.Timing.HActive;
              lboxWin.Height = pVprocss->VidOut.Timing.VActive;
            }
            XV_LBoxSetActiveWin(pVprocss->lbox,
                                &lboxWin,
                                pVprocss->VidOut.Timing.HActive,
                                pVprocss->VidOut.Timing.VActive);

            /* set background to default color on pipe reset */
            XV_LboxSetBackgroundColor(pVprocss->lbox,
                                      XLBOX_BKGND_BLACK,
                                      pVprocss->idata.strmCformat,
                                      pVprocss->VidOut.ColorDepth);

            pStartCore[XVPROCSS_RTR_LBOX] = TRUE;
          }
          break;

      case XVPROCSS_RTR_CR_H:
          if(pVprocss->hcrsmplr)
          {
            XV_HCrsmplSetActiveSize(pVprocss->hcrsmplr,
                                    pVprocss->VidOut.Timing.HActive,
                                    pVprocss->VidOut.Timing.VActive);

            XV_HCrsmplSetFormat(pVprocss->hcrsmplr,
			            &pVprocss->hcrL2Reg,
                                pCfg->hcrIn,
                                pCfg->hcrOut);
            pStartCore[XVPROCSS_RTR_CR_H] = TRUE;
          }
          break;

      case XVPROCSS_RTR_CR_V_IN:
          if(pVprocss->vcrsmplrIn)
          {
            XV_VCrsmplSetActiveSize(pVprocss->vcrsmplrIn,
			                pCfg->vidInWidth,
			                pCfg->vidInHeight);

            XV_VCrsmplSetFormat(pVprocss->vcrsmplrIn,
			            &pVprocss->vcrInL2Reg,
                                XVIDC_CSF_YCRCB_420,
                                XVIDC_CSF_YCRCB_422);
            pStartCore[XVPROCSS_RTR_CR_V_IN] = TRUE;
          }
          break;

      case XVPROCSS_RTR_CR_V_OUT:
          if(pVprocss->vcrsmplrOut)
          {
            XV_VCrsmplSetActiveSize(pVprocss->vcrsmplrOut,
                                    pVprocss->VidOut.Timing.HActive,
                                    pVprocss->VidOut.Timing.VActive);

            XV_VCrsmplSetFormat(pVprocss->vcrsmplrOut,
			            &pVprocss->vcrOutL2Reg,
                                XVIDC_CSF_YCRCB_422,
                                XVIDC_CSF_YCRCB_420);
            pStartCore[XVPROCSS_RTR_CR_V_OUT] = TRUE;
          }
          break;

      case XVPROCSS_RTR_CSC:
          if(pVprocss->csc)
          {
            XV_CscSetColorspace(pVprocss->csc,
                                &pVprocss->cscL2Reg,
                                pCfg->cscIn,
                                pCfg->cscOut,
                                pVprocss->cscL2Reg.StandardIn,
                                pVprocss->cscL2Reg.StandardOut,
                                pVprocss->cscL2Reg.OutputRange);

            XV_CscSetActiveSize(pVprocss->csc,
                                pVprocss->VidOut.Timing.HActive,
                                pVprocss->VidOut.Timing.VActive);

            pStartCore[XVPROCSS_RTR_CSC] = TRUE;
          }
          break;

      case XVPROCSS_RTR_DEINT:
          if(pVprocss->deint)
          {
	        xdbg_printf(XDBG_DEBUG_GENERAL,"  -> Configure Deinterlacer for %dx%d to %dx%d\r\n", \
		              (int)pVprocss->VidIn.Timing.HActive,
		              (int)pVprocss->VidIn.Timing.VActive,
		              (int)pCfg->vidInWidth,
		              (int)pCfg->vidInHeight);

            XV_DeintSetFieldBuffers(pVprocss->deint,
			                pCfg->deintBufAddr,
			                        pVprocss->VidIn.ColorFormatId);

            XV_deinterlacer_Set_width(pVprocss->deint,
			                  pCfg->vidInWidth);

            XV_deinterlacer_Set_height(pVprocss->deint,
			                   pVprocss->VidIn.Timing.VActive); //field height

            XV_deinterlacer_Set_invert_field_id(pVprocss->deint, 0); //TBD
            pStartCore[XVPROCSS_RTR_DEINT] = TRUE;
          }
          break;
    }
  }

  /* Start all IP Blocks in the processing chain */
  XVprocss_Start(pVprocss);
}
/** @} */
