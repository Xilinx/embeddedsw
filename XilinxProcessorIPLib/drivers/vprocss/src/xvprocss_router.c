/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvprocss_router.c
* @addtogroup vprocss_v2_8
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
* 1.00  rco  07/21/15   Initial Release
* 2.00  dmc  12/17/15   Accommodate Full topology with no VDMA
*                       Rename and modify H,VCresample constants and routines
*                       Mods to conform to coding sytle
*            01/11/16   Write to new Event Log: log status and error events
*                       The Event Logging replaces all the printf statements
* 2.1   mpe  04/28/16   Added optional color format conversion handling in
*                       scaler only topology
* 2.2   rco  11/01/16   Add log events to capture failure during router data
*                       flow setup
* 2.4   vyc  10/04/17   Write to Result for XV_CscSetColorSpace
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xvidc.h"
#include "xvprocss_router.h"
#include "xvprocss_vdma.h"

/************************** Constant Definitions *****************************/
/* AXIS Switch Port# connected to input stream */
#define XVPROCSS_AXIS_SWITCH_VIDIN_S0     (0)
#define XVPROCSS_AXIS_SWITCH_VIDOUT_M0    (0)

/************************** Function Prototypes ******************************/
static int validateWindowSize(const XVidC_VideoWindow *win,
		                      const u32 HActive,
		                      const u32 VActive);

static XVprocSs_ScaleMode GetScalingMode(XVprocSs *XVprocSsPtr);


/*****************************************************************************/
/**
* This function checks to make sure sub-frame is inside full frame
*
* @param  win is a pointer to the sub-frame window
* @param  HActive is frame width
* @param  VActive is frame height
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
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return Scaling mode Up/Dpwn or 1:1
*
******************************************************************************/
static XVprocSs_ScaleMode GetScalingMode(XVprocSs *XVprocSsPtr)
{
  int status;
  XVprocSs_ScaleMode mode;
  XVidC_VideoWindow win;
  XVidC_VideoStream *StrmInPtr  = &XVprocSsPtr->VidIn;
  XVidC_VideoStream *StrmOutPtr = &XVprocSsPtr->VidOut;

  if(XVprocSs_IsPipModeOn(XVprocSsPtr))
  {
    /* Read PIP window setting - set elsewhere */
    XVprocSs_GetZoomPipWindow(XVprocSsPtr, XVPROCSS_PIP_WIN, &win);
    /* validate window */
    status = validateWindowSize(&win,
                                XVprocSsPtr->VidOut.Timing.HActive,
                                XVprocSsPtr->VidOut.Timing.VActive);
    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_WRWINBAD);
	  return(XVPROCSS_SCALE_NOT_SUPPORTED);
    }
    else
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_SCALEDN);
      return(XVPROCSS_SCALE_DN);
    }
  }

  if(XVprocSs_IsZoomModeOn(XVprocSsPtr))
  {
    /* Read PIP window setting - set elsewhere */
    XVprocSs_GetZoomPipWindow(XVprocSsPtr, XVPROCSS_ZOOM_WIN, &win);
    /* validate window */
    status = validateWindowSize(&win,
	                            StrmInPtr->Timing.HActive,
	                            StrmInPtr->Timing.VActive);
    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_RDWINBAD);
	  return(XVPROCSS_SCALE_NOT_SUPPORTED);
    }
    else
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_SCALEUP);
      return (XVPROCSS_SCALE_UP);
    }
  }

  /* Pip & Zoom mode are off. Check input/output resolution */
  if((StrmInPtr->Timing.HActive > StrmOutPtr->Timing.HActive) ||
     (StrmInPtr->Timing.VActive > StrmOutPtr->Timing.VActive))
  {
    mode = XVPROCSS_SCALE_DN;
    XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_SCALEDN);
  }
  else if((StrmInPtr->Timing.HActive < StrmOutPtr->Timing.HActive) ||
          (StrmInPtr->Timing.VActive < StrmOutPtr->Timing.VActive))
  {
    mode = XVPROCSS_SCALE_UP;
    XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_SCALEUP);
  }
  else
  {
    mode = XVPROCSS_SCALE_1_1;
    XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_SCALE11);
  }
  return(mode);
}

/*****************************************************************************/
/**
* This function examines the subsystem Input/Output Stream configuration and
* builds a routing table for the supported use-case. The computed routing
* table is stored in the scratch pad memory
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if routing table can be created else XST_FAILURE
*
******************************************************************************/
int XVprocSs_BuildRoutingTable(XVprocSs *XVprocSsPtr)
{
  u32 index = 0;
  XVidC_VideoStream *StrmInPtr  = &XVprocSsPtr->VidIn;
  XVidC_VideoStream *StrmOutPtr = &XVprocSsPtr->VidOut;
  XVprocSs_ContextData *CtxtPtr = &XVprocSsPtr->CtxtData;
  u8 *pTable              = &XVprocSsPtr->CtxtData.RtngTable[0];
  int status = XST_SUCCESS;

  /* Save input resolution */
  CtxtPtr->VidInWidth  = StrmInPtr->Timing.HActive;
  CtxtPtr->VidInHeight = StrmInPtr->Timing.VActive;
  CtxtPtr->StrmCformat = StrmInPtr->ColorFormatId;

  /* Determine Scaling Mode */
  CtxtPtr->ScaleMode = GetScalingMode(XVprocSsPtr);
  if(CtxtPtr->ScaleMode == XVPROCSS_SCALE_NOT_SUPPORTED) {
    XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_SCALEBAD);
    return(XST_FAILURE);
  }

  /* Reset Routing Table */
  memset(pTable, 0, sizeof(CtxtPtr->RtngTable));

  /* Check if input is I/P */
  if(StrmInPtr->IsInterlaced) {
    pTable[index++] = XVPROCSS_SUBCORE_DEINT;
  }

  /* Check if input is 420 */
  if(StrmInPtr->ColorFormatId == XVIDC_CSF_YCRCB_420) {
    //up-sample vertically to 422 as none of the IP supports 420
    pTable[index++] = XVPROCSS_SUBCORE_CR_V_IN;
    CtxtPtr->StrmCformat = XVIDC_CSF_YCRCB_422;
  }

  switch(CtxtPtr->ScaleMode)
  {
    case XVPROCSS_SCALE_1_1:
        if(XVprocSsPtr->VdmaPtr) {
          pTable[index++] = XVPROCSS_SUBCORE_VDMA;
	    }
        break;

    case XVPROCSS_SCALE_UP:
	    if(XVprocSsPtr->VdmaPtr) {
          pTable[index++] = XVPROCSS_SUBCORE_VDMA;     /* VDMA is before Scaler */
	    }
        pTable[index++] = XVPROCSS_SUBCORE_SCALER_V;
        pTable[index++] = XVPROCSS_SUBCORE_SCALER_H;
        break;

    case XVPROCSS_SCALE_DN:
        pTable[index++] = XVPROCSS_SUBCORE_SCALER_H;
        pTable[index++] = XVPROCSS_SUBCORE_SCALER_V;
	    if(XVprocSsPtr->VdmaPtr) {
          pTable[index++] = XVPROCSS_SUBCORE_VDMA;     /* VDMA is after Scaler */
	    }
        break;

    default:
        XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_SCALEBAD);
        return(XST_FAILURE);
        break;
  }

  /* Send stream to LBox to add H/V bars, if needed */
  pTable[index++] = XVPROCSS_SUBCORE_LBOX;

  /* Check for input and output color format to derive required conversions */
  switch(StrmOutPtr->ColorFormatId)
  {
      case XVIDC_CSF_YCRCB_420:
         switch(StrmInPtr->ColorFormatId)
         {
           case XVIDC_CSF_RGB:
              pTable[index++] = XVPROCSS_SUBCORE_CSC;      //convert RGB->444
              pTable[index++] = XVPROCSS_SUBCORE_CR_H;     //convert 444->422
              pTable[index++] = XVPROCSS_SUBCORE_CR_V_OUT; //convert 422->420
              CtxtPtr->CscIn  = XVIDC_CSF_RGB;
              CtxtPtr->CscOut = XVIDC_CSF_YCRCB_444;
              CtxtPtr->HcrIn  = XVIDC_CSF_YCRCB_444;
              CtxtPtr->HcrOut = XVIDC_CSF_YCRCB_422;
              break;

           case XVIDC_CSF_YCRCB_444:
              pTable[index++] = XVPROCSS_SUBCORE_CSC;      //picture control in 444
              pTable[index++] = XVPROCSS_SUBCORE_CR_H;     //convert 444->422
              pTable[index++] = XVPROCSS_SUBCORE_CR_V_OUT; //convert 422->420
              CtxtPtr->CscIn  = XVIDC_CSF_YCRCB_444;
              CtxtPtr->CscOut = XVIDC_CSF_YCRCB_444;
              CtxtPtr->HcrIn  = XVIDC_CSF_YCRCB_444;
              CtxtPtr->HcrOut = XVIDC_CSF_YCRCB_422;
              break;

           case XVIDC_CSF_YCRCB_422:
           case XVIDC_CSF_YCRCB_420: //Input was up converted to 422
              pTable[index++] = XVPROCSS_SUBCORE_CSC;      //picture control in 422
              pTable[index++] = XVPROCSS_SUBCORE_CR_V_OUT; //convert 422->420
              CtxtPtr->CscIn  = XVIDC_CSF_YCRCB_422;
              CtxtPtr->CscOut = XVIDC_CSF_YCRCB_422;
              break;

           default: //Unsupported color format
              XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_CFIN);
              status = XST_FAILURE;
              break;
         }
         break;

      case XVIDC_CSF_RGB:
         switch(StrmInPtr->ColorFormatId)
         {
           case XVIDC_CSF_RGB:
           case XVIDC_CSF_YCRCB_444:  //convert 444->RGB
              pTable[index++] = XVPROCSS_SUBCORE_CSC;
              CtxtPtr->CscIn  = StrmInPtr->ColorFormatId;
              CtxtPtr->CscOut = XVIDC_CSF_RGB;
              break;

           case XVIDC_CSF_YCRCB_422:
           case XVIDC_CSF_YCRCB_420: //Input was up converted to 422
              pTable[index++] = XVPROCSS_SUBCORE_CR_H;     //convert 422->444
              pTable[index++] = XVPROCSS_SUBCORE_CSC;      //convert 444->RGB
              CtxtPtr->HcrIn  = XVIDC_CSF_YCRCB_422;
              CtxtPtr->HcrOut = XVIDC_CSF_YCRCB_444;
              CtxtPtr->CscIn  = XVIDC_CSF_YCRCB_444;
              CtxtPtr->CscOut = XVIDC_CSF_RGB;
              break;

           default: //Unsupported color format
              XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_CFIN);
              status = XST_FAILURE;
              break;
         }
         break;

      case XVIDC_CSF_YCRCB_422:
         switch(StrmInPtr->ColorFormatId)
         {
           case XVIDC_CSF_RGB:
              pTable[index++] = XVPROCSS_SUBCORE_CSC;      //convert RGB->444
              pTable[index++] = XVPROCSS_SUBCORE_CR_H;     //convert 444->422
              CtxtPtr->CscIn  = XVIDC_CSF_RGB;
              CtxtPtr->CscOut = XVIDC_CSF_YCRCB_444;
              CtxtPtr->HcrIn  = XVIDC_CSF_YCRCB_444;
              CtxtPtr->HcrOut = XVIDC_CSF_YCRCB_422;
              break;

           case XVIDC_CSF_YCRCB_444:
              pTable[index++] = XVPROCSS_SUBCORE_CSC;      //picture control in 444
              pTable[index++] = XVPROCSS_SUBCORE_CR_H;     //convert 444->422
              CtxtPtr->CscIn  = XVIDC_CSF_YCRCB_444;
              CtxtPtr->CscOut = XVIDC_CSF_YCRCB_444;
              CtxtPtr->HcrIn  = XVIDC_CSF_YCRCB_444;
              CtxtPtr->HcrOut = XVIDC_CSF_YCRCB_422;
              break;

           case XVIDC_CSF_YCRCB_422:
           case XVIDC_CSF_YCRCB_420: //Input was up converted to 422
              pTable[index++] = XVPROCSS_SUBCORE_CSC;      //picture control in 422
              CtxtPtr->CscIn  = XVIDC_CSF_YCRCB_422;
              CtxtPtr->CscOut = XVIDC_CSF_YCRCB_422;
              break;

           default: //Unsupported color format
              XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_CFIN);
              status = XST_FAILURE;
              break;
         }
         break;

      case XVIDC_CSF_YCRCB_444:
         switch(StrmInPtr->ColorFormatId)
         {
           case XVIDC_CSF_RGB:        //convert 444->RGB
           case XVIDC_CSF_YCRCB_444:
              pTable[index++] = XVPROCSS_SUBCORE_CSC;
              CtxtPtr->CscIn  = StrmInPtr->ColorFormatId;
              CtxtPtr->CscOut = XVIDC_CSF_YCRCB_444;
              break;

           case XVIDC_CSF_YCRCB_422:
           case XVIDC_CSF_YCRCB_420: //Input was up converted to 422
              pTable[index++] = XVPROCSS_SUBCORE_CR_H;     //convert 422->444
              pTable[index++] = XVPROCSS_SUBCORE_CSC;      //picture control
              CtxtPtr->HcrIn  = XVIDC_CSF_YCRCB_422;
              CtxtPtr->HcrOut = XVIDC_CSF_YCRCB_444;
              CtxtPtr->CscIn  = XVIDC_CSF_YCRCB_444;
              CtxtPtr->CscOut = XVIDC_CSF_YCRCB_444;
              break;

           default: //Unsupported color format
              XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_CFIN);
              status = XST_FAILURE;
              break;
         }
         break;

      default:
        XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_CFOUT);
        status = XST_FAILURE;
        break;
  }

  /* Connect Last IP in chain to switch output */
  pTable[index++] = XVPROCSS_AXIS_SWITCH_VIDOUT_M0;

  /* save number of cores in processing path */
  CtxtPtr->RtrNumCores = index;

  if(status == XST_SUCCESS)
    XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_TABLEOK);

  return(status);
}

/*****************************************************************************/
/**
* This function traverses the computed routing table and sets up the AXIS
* switch registers, to route the stream through processing cores, in the order
* defined in the routing map
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XVprocSs_ProgRouterMux(XVprocSs *XVprocSsPtr)
{
  u32 count, nextMi, prevSi;
  u8 *pTable = &XVprocSsPtr->CtxtData.RtngTable[0];
  u32 numProcElem = XVprocSsPtr->CtxtData.RtrNumCores;

  XAxisScr_RegUpdateDisable(XVprocSsPtr->RouterPtr);

  /* Disable all ports */
  XAxisScr_MiPortDisableAll(XVprocSsPtr->RouterPtr);

  /* Connect Input Stream to the 1st core in path */
  nextMi = prevSi = pTable[0];
  XAxisScr_MiPortEnable(XVprocSsPtr->RouterPtr, nextMi, XVPROCSS_AXIS_SWITCH_VIDIN_S0);

  /* Traverse routing map and connect cores in the chain */
  for(count=1; count<numProcElem; ++count)
  {
    nextMi  = pTable[count];
    XAxisScr_MiPortEnable(XVprocSsPtr->RouterPtr, nextMi, prevSi);
    prevSi = nextMi;
  }

  //Enable Router register update
  XAxisScr_RegUpdateEnable(XVprocSsPtr->RouterPtr);

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_ROUTEOK);
}

/*****************************************************************************/
/**
* This function traverses the routing map built earlier and configures each
* sub-core in the processing path per its location in the chain.
* Each core in the processing path is marked and only marked cores are started
* All remaining cores stay disabled
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XVprocSs_SetupRouterDataFlow(XVprocSs *XVprocSsPtr)
{
  XVidC_VideoWindow lboxWin;
  u32 vsc_WidthIn, vsc_HeightIn, vsc_HeightOut;
  u32 hsc_HeightIn, hsc_WidthIn, hsc_WidthOut;
  XVidC_ColorDepth ColorDepth;
  u32 count;
  int Result = XST_SUCCESS;
  int SetupFlag = XST_SUCCESS;
  XVprocSs_ContextData *CtxtPtr = &XVprocSsPtr->CtxtData;
  u8 *pTable = &XVprocSsPtr->CtxtData.RtngTable[0];
  u8 *StartCorePtr = &XVprocSsPtr->CtxtData.StartCore[0];

  vsc_WidthIn = vsc_HeightIn = vsc_HeightOut = 0;
  hsc_HeightIn = hsc_WidthIn = hsc_WidthOut = 0;

  /* Program Video Pipe Sub-Cores */
  if (XVprocSsPtr->VidIn.IsInterlaced) {
    /* Input will de-interlaced first. All downstream IP's work
     * with progressive frame. Adjust active height to reflect the
     * progressive frame to downstream cores
     */
	CtxtPtr->VidInHeight *= 2;
  }

  /* If Vdma is enabled, RD/WR Client needs to be programmed before Scaler */
  if (XVprocSsPtr->VdmaPtr) {
    switch (CtxtPtr->ScaleMode) {
      case XVPROCSS_SCALE_1_1:
      case XVPROCSS_SCALE_UP:
          XVprocSs_VdmaSetWinToUpScaleMode(XVprocSsPtr, XVPROCSS_VDMA_UPDATE_ALL_CH);
          break;

      case XVPROCSS_SCALE_DN:
	      XVprocSs_VdmaSetWinToDnScaleMode(XVprocSsPtr, XVPROCSS_VDMA_UPDATE_ALL_CH);
          break;

      default:
          break;
    }
    StartCorePtr[XVPROCSS_SUBCORE_VDMA] = TRUE;
  }

  for (count=0; count<CtxtPtr->RtrNumCores; ++count) {
    switch (pTable[count]) {
      case XVPROCSS_SUBCORE_SCALER_V:
          if(XVprocSsPtr->VscalerPtr) {
            if(CtxtPtr->ScaleMode == XVPROCSS_SCALE_DN) {
              /* Downscale mode H Scaler is before V Scaler */
              vsc_WidthIn   = hsc_WidthOut;
              vsc_HeightIn  = hsc_HeightIn;
              vsc_HeightOut = ((XVprocSs_IsPipModeOn(XVprocSsPtr)) ? CtxtPtr->WrWindow.Height
                                                                   : XVprocSsPtr->VidOut.Timing.VActive);
            } else {
              /* UpScale mode V Scaler is before H Scaler */
              vsc_WidthIn  = ((XVprocSs_IsZoomModeOn(XVprocSsPtr)) ? CtxtPtr->RdWindow.Width
                                                                   : CtxtPtr->VidInWidth);
              vsc_HeightIn = ((XVprocSs_IsZoomModeOn(XVprocSsPtr)) ? CtxtPtr->RdWindow.Height
                                                                   : CtxtPtr->VidInHeight);
              vsc_HeightOut = XVprocSsPtr->VidOut.Timing.VActive;
            }

            Result = XV_VScalerSetup(XVprocSsPtr->VscalerPtr,
                                     vsc_WidthIn,
                                     vsc_HeightIn,
                                     vsc_HeightOut,
							         XVprocSsPtr->CtxtData.StrmCformat);
            if(Result != XST_SUCCESS) {
              XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VSCALER, XVPROCSS_EDAT_IGNORE);
              SetupFlag = Result;
            } else {
              StartCorePtr[XVPROCSS_SUBCORE_SCALER_V] = TRUE;
            }
          }
          break;

      case XVPROCSS_SUBCORE_SCALER_H:
          if(XVprocSsPtr->HscalerPtr) {
            if(CtxtPtr->ScaleMode == XVPROCSS_SCALE_DN) {
              /* Downscale mode H Scaler is before V Scaler */
              hsc_WidthIn  = CtxtPtr->VidInWidth;
              hsc_HeightIn = CtxtPtr->VidInHeight;
              hsc_WidthOut = ((XVprocSs_IsPipModeOn(XVprocSsPtr)) ?
			  CtxtPtr->WrWindow.Width :
					  XVprocSsPtr->VidOut.Timing.HActive);
            } else {
              /* Upscale mode V Scaler is before H Scaler */
              hsc_WidthIn  = vsc_WidthIn;
              hsc_HeightIn = vsc_HeightOut;
              hsc_WidthOut = XVprocSsPtr->VidOut.Timing.HActive;
            }

            Result = XV_HScalerSetup(XVprocSsPtr->HscalerPtr,
                                     hsc_HeightIn,
                                     hsc_WidthIn,
                                     hsc_WidthOut,
                                     XVprocSsPtr->CtxtData.StrmCformat,
                                     XVprocSsPtr->CtxtData.StrmCformat);
            if(Result != XST_SUCCESS) {
              XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_IGNORE);
              SetupFlag = Result;
            } else {
              StartCorePtr[XVPROCSS_SUBCORE_SCALER_H] = TRUE;
            }
          }
          break;

      case XVPROCSS_SUBCORE_VDMA:
          /* NOP - Programmed before the switch statement */
          break;

      case XVPROCSS_SUBCORE_LBOX:
          if(XVprocSsPtr->LboxPtr) {
            if(XVprocSs_IsPipModeOn(XVprocSsPtr)) {
              /* get the active window for Lbox */
              lboxWin = CtxtPtr->WrWindow;
            } else {
			  //Downscale - Read full image from VDMA
              /* window is same as output resolution */
              lboxWin.StartX = 0;
              lboxWin.StartY = 0;
              lboxWin.Width  = XVprocSsPtr->VidOut.Timing.HActive;
              lboxWin.Height = XVprocSsPtr->VidOut.Timing.VActive;
            }
            XV_LBoxSetActiveWin(XVprocSsPtr->LboxPtr,
                                &lboxWin,
                                XVprocSsPtr->VidOut.Timing.HActive,
                                XVprocSsPtr->VidOut.Timing.VActive);

            /* set background to default color on pipe reset */
            XV_LboxSetBackgroundColor(XVprocSsPtr->LboxPtr,
			                  XVprocSsPtr->CtxtData.LboxBkgndColor,
                                      XVprocSsPtr->CtxtData.StrmCformat,
                                      XVprocSsPtr->VidOut.ColorDepth);

            StartCorePtr[XVPROCSS_SUBCORE_LBOX] = TRUE;
          }
          break;

      case XVPROCSS_SUBCORE_CR_H:
          if(XVprocSsPtr->HcrsmplrPtr) {
            XV_HCrsmplSetActiveSize(XVprocSsPtr->HcrsmplrPtr,
                                    XVprocSsPtr->VidOut.Timing.HActive,
                                    XVprocSsPtr->VidOut.Timing.VActive);

            XV_HCrsmplSetFormat(XVprocSsPtr->HcrsmplrPtr,
                                CtxtPtr->HcrIn,
                                CtxtPtr->HcrOut);
            StartCorePtr[XVPROCSS_SUBCORE_CR_H] = TRUE;
          }
          break;

      case XVPROCSS_SUBCORE_CR_V_IN:
          if(XVprocSsPtr->VcrsmplrInPtr) {
            XV_VCrsmplSetActiveSize(XVprocSsPtr->VcrsmplrInPtr,
			                        CtxtPtr->VidInWidth,
			                        CtxtPtr->VidInHeight);

            XV_VCrsmplSetFormat(XVprocSsPtr->VcrsmplrInPtr,
                                XVIDC_CSF_YCRCB_420,
                                XVIDC_CSF_YCRCB_422);
            StartCorePtr[XVPROCSS_SUBCORE_CR_V_IN] = TRUE;
          }
          break;

      case XVPROCSS_SUBCORE_CR_V_OUT:
          if(XVprocSsPtr->VcrsmplrOutPtr) {
            XV_VCrsmplSetActiveSize(XVprocSsPtr->VcrsmplrOutPtr,
                                    XVprocSsPtr->VidOut.Timing.HActive,
                                    XVprocSsPtr->VidOut.Timing.VActive);

            XV_VCrsmplSetFormat(XVprocSsPtr->VcrsmplrOutPtr,
                                XVIDC_CSF_YCRCB_422,
                                XVIDC_CSF_YCRCB_420);
            StartCorePtr[XVPROCSS_SUBCORE_CR_V_OUT] = TRUE;
          }
          break;

      case XVPROCSS_SUBCORE_CSC:
        if(XVprocSsPtr->CscPtr) {

          // to set up a new resolution, start with default picture settings
          XV_CscSetPowerOnDefaultState(XVprocSsPtr->CscPtr);

	      // set the proper color depth: get it from the vprocss config
          ColorDepth = (XVidC_ColorDepth)XVprocSs_GetColorDepth(XVprocSsPtr);
          XV_CscSetColorDepth(XVprocSsPtr->CscPtr, ColorDepth);

	      // all other picture settings are filled in by XV_CscSetColorspace
          Result = XV_CscSetColorspace(XVprocSsPtr->CscPtr,
                                       CtxtPtr->CscIn,
                                       CtxtPtr->CscOut,
                                       XVprocSsPtr->CscPtr->StandardIn,
                                       XVprocSsPtr->CscPtr->StandardOut,
                                       XVprocSsPtr->CscPtr->OutputRange);

          if(Result != XST_SUCCESS) {
		 XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_IGNORE);
             SetupFlag = Result;
          }

	      // set the Global Window size
          XV_CscSetActiveSize(XVprocSsPtr->CscPtr,
                              XVprocSsPtr->VidOut.Timing.HActive,
                              XVprocSsPtr->VidOut.Timing.VActive);

          StartCorePtr[XVPROCSS_SUBCORE_CSC] = TRUE;
        }
        break;

      case XVPROCSS_SUBCORE_DEINT:
          if(XVprocSsPtr->DeintPtr)
          {
            XV_DeintSetFieldBuffers(XVprocSsPtr->DeintPtr,
			                        CtxtPtr->DeintBufAddr,
			                        XVprocSsPtr->VidIn.ColorFormatId);

            XV_deinterlacer_Set_width(&XVprocSsPtr->DeintPtr->Deint,
			                          CtxtPtr->VidInWidth);

            // VidIn.Timing.VActive is the field height
            XV_deinterlacer_Set_height(&XVprocSsPtr->DeintPtr->Deint,
			                           XVprocSsPtr->VidIn.Timing.VActive);

            // TBD
            XV_deinterlacer_Set_invert_field_id(&XVprocSsPtr->DeintPtr->Deint,
				                                0);
            StartCorePtr[XVPROCSS_SUBCORE_DEINT] = TRUE;
          }
          break;
    }
  }

  if(SetupFlag == XST_SUCCESS) {
    XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_MAX, XVPROCSS_EDAT_MAX_DFLOWOK);
  }

  /* Start all IP Blocks in the processing chain */
  XVprocSs_Start(XVprocSsPtr);

}
/** @} */
