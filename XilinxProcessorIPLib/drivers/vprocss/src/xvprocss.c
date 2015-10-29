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
* @file xvprocss.c
*
* This is main code of Xilinx Video Processing Subsystem device driver.
* Please see xvprocss.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  rco   08/28/15   Initial Release
* 2.00  rco   11/05/15  Update to adapt to sub-core layer 2 changes
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xvprocss.h"
#include "xenv.h"
#include "xvprocss_vdma.h"
#include "xvprocss_router.h"
#include "xvprocss_coreinit.h"

#if defined(__arm__)
#include "sleep.h"
#elif defined(__MICROBLAZE__)
#include "microblaze_sleep.h"
#endif

/************************** Constant Definitions *****************************/

/* HW Reset Network GPIO Channel */
#define GPIO_CH_RESET_SEL                 (1u)

/** @name Reset Network
 *
 * @{
 * The following constants define various reset lines in the subsystem
 */
#define RESET_MASK_VIDEO_IN               (0x01) /**< Reset line going out of subsystem */
#define RESET_MASK_IP_AXIS                (0x02) /**< Reset line for all video IP blocks */
#define RESET_MASK_IP_AXIMM               (0x01) /**< Reset line for AXI-MM blocks */
/*@}*/

#define RESET_MASK_ALL_BLOCKS             (RESET_MASK_VIDEO_IN  | \
                                           RESET_MASK_IP_AXIS)

/**************************** Type Definitions *******************************/
/**
 * This typedef declares the driver instances of all the cores in the subsystem
 */
typedef struct
{
  XAxis_Switch Router;
  XGpio RstAxis;      //Reset for IP's running at AXIS Clk
  XGpio RstAximm;     //Reset for IP's with AXI MM interface

  XV_Hcresampler_l2 Hcrsmplr;
  XV_Vcresampler_l2 VcrsmplrIn;
  XV_Vcresampler_l2 VcrsmplrOut;
  XV_Vscaler_l2 Vscaler;
  XV_Hscaler_l2 Hscaler;
  XAxiVdma Vdma;
  XV_Lbox_l2 Lbox;
  XV_Csc_l2 Csc;
  XV_Deint_l2 Deint;
}XVprocSs_SubCores;

/**************************** Local Global ***********************************/
XVprocSs_SubCores subcoreRepo; /**< Define Driver instance of all sub-core
                                    included in the design */

static const char *XVprocSsIpStr[XVPROCSS_SUBCORE_MAX] =  {
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

/** @name VDMA Alignment required step size
 *
 * @{
 * The following constants define various Zoom/Pip window horizontal step sizes
 * that keeps the VDMA access aligned to aximm interface width, based on
 * Pixels/Clock and Color Depth of the configured subsystem. This is required
 * as current version of VDMA does not support DRE if interface width is
 * >64bits. Software has to align the hsize and stride for all possible
 * configurations supported by the subsystem.
 * Current subsystem version supports the following
 *  - Number of components = 3 (Fixed)
 *  - Pixels/Clock         = 1, 2, 4
 *  - Color Depth          = 8, 10, 12, 16   (4 variations)
 */
const u16 XVprocSs_PixelHStep[XVIDC_PPC_NUM_SUPPORTED][4] =
{
  {16,   4,  32,   8}, //XVIDC_PPC_1
  {16,   4,  64,  16}, //XVIDC_PPC_2
  {32, 128, 128,  32}, //XVIDC_PPC_4
};

/************************** Function Prototypes ******************************/
static void SetPowerOnDefaultState(XVprocSs *XVprocSsPtr);
static void GetIncludedSubcores(XVprocSs *XVprocSsPtr);
static int ValidateSubsystemConfig(XVprocSs *InstancePtr);
static int ValidateScalerOnlyConfig(XVidC_VideoStream *pStrmIn,
                                    XVidC_VideoStream *pStrmOut);
static int SetupModeScalerOnly(XVprocSs *XVprocSsPtr);
static int SetupModeMax(XVprocSs *XVprocSsPtr);

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
* This macro reads the subsystem reset network state
*
* @param  pReset is a pointer to the Reset IP Block
* @param  channel is number of reset channel to work upon
*
* @return Reset state
*           -1: Normal
*           -0: Reset
*
******************************************************************************/
static __inline u32 XVprocSs_GetResetState(XGpio *pReset, u32 channel)
{
  return(XGpio_DiscreteRead(pReset, channel));
}

/*****************************************************************************/
/**
* This macro enables the IP's connected to subsystem reset network
*
* @param  pReset is a pointer to the Reset IP Block
* @param  channel is number of reset channel to work upon
* @param  ipBlock is the reset line to be activated
*
* @return None
*
* @note If reset block is not included in the subsystem instance function does
*       not do anything
******************************************************************************/
static __inline void XVprocSs_EnableBlock(XGpio *pReset, u32 channel, u32 ipBlock)
{
  u32 val;

  if(pReset)
  {
    val = XVprocSs_GetResetState(pReset, channel);
    val |= ipBlock;
    XGpio_DiscreteWrite(pReset, channel, val);
  }
}

/*****************************************************************************/
/**
* This macro resets the IP connected to subsystem reset network
*
* @param  pReset is a pointer to the Reset IP Block
* @param  channel is number of reset channel to work upon
* @param  ipBlock is the reset line to be asserted
*
* @return None
*
* @note If reset block is not included in the subsystem instance function does
*       not do anything
******************************************************************************/
static __inline void XVprocSs_ResetBlock(XGpio *pReset, u32 channel, u32 ipBlock)
{
  u32 val;

  if(pReset)
  {
    val = XVprocSs_GetResetState(pReset, channel);
    val &= ~ipBlock;
    XGpio_DiscreteWrite(pReset, channel, val);
  }
}

/*****************************************************************************/
/**
* This function routes the delay routine used in the subsystem. Preference is
* given to the user registered timer based routine. If no delay handler is
* registered then it uses the platform specific delay handler
*
* @param  XVprocSsPtr is a pointer to the subsystem instance
* @param  msec is delay required
*
* @return None
*
******************************************************************************/
static __inline void WaitUs(XVprocSs *XVprocSsPtr, u32 MicroSeconds)
{
  if(MicroSeconds == 0)
	  return;

#if defined(__arm__)
  /* Wait the requested amount of time. */
  usleep(MicroSeconds);
#elif defined(__MICROBLAZE__)
  if(XVprocSsPtr->UsrDelayUs)
  {
    /* Use the time handler specified by the user for
     * better accuracy
     */
	 XVprocSsPtr->UsrDelayUs(XVprocSsPtr->UsrTmrPtr, MicroSeconds);
  }
  else
  {
    /* MicroBlaze sleep only has millisecond accuracy. Round up. */
    u32 MilliSeconds = (MicroSeconds + 999) / 1000;

	MB_Sleep(MilliSeconds);
  }
#endif
}

/************************** Function Definition ******************************/

/*****************************************************************************/
/**
* This function registers the user defined delay/sleep function with subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance
* @param  CallbackFunc is the function pointer to the user defined delay
*         function
* @param  CallbackRef is the pointer to timer instance used by the delay
*         function
*
* @return None
*
******************************************************************************/
void XVprocSs_SetUserTimerHandler(XVprocSs *InstancePtr,
                                  XVidC_DelayHandler CallbackFunc,
                                  void *CallbackRef)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(CallbackFunc != NULL);
  Xil_AssertVoid(CallbackRef != NULL);

  InstancePtr->UsrDelayUs = CallbackFunc;
  InstancePtr->UsrTmrPtr  = CallbackRef;
}

/*****************************************************************************/
/**
* This function queries the subsystem instance configuration to determine
* the included sub-cores. For each sub-core that is present in the design
* the sub-core driver instance is binded with the subsystem sub-core driver
* handle
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
static void GetIncludedSubcores(XVprocSs *XVprocSsPtr)
{
  XVprocSsPtr->HcrsmplrPtr    = ((XVprocSsPtr->Config.HCrsmplr.IsPresent)    \
                              ? (&subcoreRepo.Hcrsmplr)    : NULL);
  XVprocSsPtr->VcrsmplrInPtr  = ((XVprocSsPtr->Config.VCrsmplrIn.IsPresent)  \
                              ? (&subcoreRepo.VcrsmplrIn)  : NULL);
  XVprocSsPtr->VcrsmplrOutPtr = ((XVprocSsPtr->Config.VCrsmplrOut.IsPresent) \
                              ? (&subcoreRepo.VcrsmplrOut) : NULL);
  XVprocSsPtr->VscalerPtr     = ((XVprocSsPtr->Config.Vscale.IsPresent)      \
                              ? (&subcoreRepo.Vscaler)     : NULL);
  XVprocSsPtr->HscalerPtr     = ((XVprocSsPtr->Config.Hscale.IsPresent)      \
                              ? (&subcoreRepo.Hscaler)     : NULL);
  XVprocSsPtr->VdmaPtr        = ((XVprocSsPtr->Config.Vdma.IsPresent)        \
                              ? (&subcoreRepo.Vdma)        : NULL);
  XVprocSsPtr->LboxPtr        = ((XVprocSsPtr->Config.Lbox.IsPresent)        \
                              ? (&subcoreRepo.Lbox)        : NULL);
  XVprocSsPtr->CscPtr         = ((XVprocSsPtr->Config.Csc.IsPresent)         \
                              ? (&subcoreRepo.Csc)         : NULL);
  XVprocSsPtr->DeintPtr       = ((XVprocSsPtr->Config.Deint.IsPresent)       \
                              ? (&subcoreRepo.Deint)       : NULL);
  XVprocSsPtr->RouterPtr      = ((XVprocSsPtr->Config.Router.IsPresent)      \
                              ? (&subcoreRepo.Router)      : NULL);
  XVprocSsPtr->RstAxisPtr     = ((XVprocSsPtr->Config.RstAxis.IsPresent)     \
                              ? (&subcoreRepo.RstAxis)     : NULL);
  XVprocSsPtr->RstAximmPtr    = ((XVprocSsPtr->Config.RstAximm.IsPresent)    \
                              ? (&subcoreRepo.RstAximm)    : NULL);
}

/*****************************************************************************/
/**
* This function sets the base address of the video frame buffers used by the
* subsystem instance
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  addr is the base address of the video frame buffers
*
* @return None
*
******************************************************************************/
void XVprocSs_SetFrameBufBaseaddr(XVprocSs *InstancePtr, u32 addr)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(addr != 0);

  InstancePtr->FrameBufBaseaddr = addr;
}


/*****************************************************************************/
/**
* This function initializes the video subsystem and included sub-cores.
* This function must be called prior to using the subsystem. Initialization
* includes setting up the instance data for top level as well as all included
* sub-core therein, and ensuring the hardware is in a known stable state.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  CfgPtr points to the configuration structure associated with the
*         subsystem instance.
* @param  EffectiveAddr is the base address of the device. If address
*         translation is being used, then this parameter must reflect the
*         virtual base address. Otherwise, the physical address should be
*         used.
*
* @return XST_SUCCESS if initialization is successful else XST_FAILURE
*
******************************************************************************/
int XVprocSs_CfgInitialize(XVprocSs *InstancePtr, XVprocSs_Config *CfgPtr,
                          u32 EffectiveAddr)
{
  XVprocSs *XVprocSsPtr = InstancePtr;
  XAxiVdma_Config *VdmaCfgPtr;
  int status;
  u32 AbsAddr;

  /* Verify arguments */
  Xil_AssertNonvoid(XVprocSsPtr != NULL);
  Xil_AssertNonvoid(CfgPtr != NULL);
  Xil_AssertNonvoid(EffectiveAddr != (u32)NULL);

  /* Setup the instance */
  memcpy((void *)&(XVprocSsPtr->Config), (const void *)CfgPtr, sizeof(XVprocSs_Config));
  XVprocSsPtr->Config.BaseAddress = EffectiveAddr;

  switch(XVprocSs_GetSubsystemTopology(InstancePtr))
  {
    case XVPROCSS_TOPOLOGY_FULL_FLEDGED:
	    xdbg_printf(XDBG_DEBUG_GENERAL,"    [Subsystem Configuration Mode - Full]\r\n\r\n");
        break;

    case XVPROCSS_TOPOLOGY_SCALER_ONLY:
	    xdbg_printf(XDBG_DEBUG_GENERAL,"    [Subsystem Configuration Mode - Scaler-Only]\r\n\r\n");
        break;

    default:
        xil_printf("ERROR: Subsystem Configuration Mode Not Supported. \r\n");
        return(XST_FAILURE);
  }

  /* Determine sub-cores included in the provided instance of subsystem */
  GetIncludedSubcores(XVprocSsPtr);

  /* Initialize all included sub_cores */
  if(XVprocSsPtr->RstAxisPtr)
  {
	if(XVprocSs_SubcoreInitResetAxis(XVprocSsPtr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }

  if(XVprocSsPtr->RstAximmPtr)
  {
	if(XVprocSs_SubcoreInitResetAximm(XVprocSsPtr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
    /*
     * Make sure AXI-MM interface is not in reset. If in reset it will prevent
     * Deinterlacer from being initialized
     */
    XVprocSs_EnableBlock(InstancePtr->RstAximmPtr, GPIO_CH_RESET_SEL, RESET_MASK_IP_AXIMM);
  }

  if(XVprocSsPtr->RouterPtr)
  {
	if(XVprocSs_SubcoreInitRouter(XVprocSsPtr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }

  if(XVprocSsPtr->CscPtr)
  {
	if(XVprocSs_SubcoreInitCsc(XVprocSsPtr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }

  if(XVprocSsPtr->HscalerPtr)
  {
	if(XVprocSs_SubcoreInitHScaler(XVprocSsPtr) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }

  if(XVprocSsPtr->VscalerPtr)
  {
	if(XVprocSs_SubcoreInitVScaler(XVprocSsPtr) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}
  }

  if(XVprocSsPtr->HcrsmplrPtr)
  {
	if(XVprocSs_SubcoreInitHCrsmplr(XVprocSsPtr) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}
  }

  if(XVprocSsPtr->VcrsmplrInPtr)
  {
	if(XVprocSs_SubcoreInitVCrsmpleIn(XVprocSsPtr) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}
  }

  if(XVprocSsPtr->VcrsmplrOutPtr)
  {
	if(XVprocSs_SubcoreInitVCrsmpleOut(XVprocSsPtr) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}
  }

  if(XVprocSsPtr->LboxPtr)
  {
	if(XVprocSs_SubcoreInitLetterbox(XVprocSsPtr) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}
  }

  if(XVprocSsPtr->VdmaPtr)
  {
	if(XVprocSs_SubcoreInitVdma(XVprocSsPtr) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}

	if(XVprocSsPtr->FrameBufBaseaddr == 0)
	{
	  xil_printf("\r\nVPROCSS ERR:: Video Frame Buffer base address not set\r\n");
	  xil_printf("              Use XVprocSs_SetFrameBufBaseaddr() API before subsystem init\r\n\r\n");
	  return(XST_FAILURE);
	}
  }


  if(XVprocSsPtr->DeintPtr)
  {
	if(XVprocSs_SubcoreInitDeinterlacer(XVprocSsPtr) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}

    /* Set Deinterlacer buffer offset in allocated DDR Frame Buffer */
    if(XVprocSsPtr->VdmaPtr) //Vdma must be present for this to work
    {
      u32 vdmaBufReq, bufsize;
      u32 Bpp; //bytes per pixel

      Bpp = (XVprocSsPtr->Config.ColorDepth + 7)/8;

      //compute buffer size based on subsystem configuration
      //For 1 4K2K buffer (YUV444 16-bit) size is ~48MB
      bufsize = XVprocSsPtr->Config.MaxWidth *
		        XVprocSsPtr->Config.MaxHeight *
		        XVprocSsPtr->Config.NumVidComponents *
		        Bpp;

      //VDMA requires 4 buffers for total size of ~190MB
      vdmaBufReq = XVprocSsPtr->VdmaPtr->MaxNumFrames * bufsize;

      /*
       * vdmaBufReq = 0x0BDD 80000
       * padBuf     = 0x02F7 6000 (1 buffer is added as pad between Vdma and Deint)
       *             -------------
       * DeInt Offst= 0x0ED4 E000
       *             -------------
       */
      /* Set Deint Buffer Address Offset */
      XVprocSsPtr->CtxtData.DeintBufAddr = XVprocSsPtr->FrameBufBaseaddr + vdmaBufReq + bufsize;
    }
    else
    {
      xil_printf("\r\nVPROCSS ERR:: VDMA IP not found. Unable to assign De-interlacer buffer offsets\r\n");
      return(XST_FAILURE);
    }
  }

  /* Set subsystem to power on default state */
  SetPowerOnDefaultState(InstancePtr);

  /* Reset the hardware and set the flag to indicate the
     subsystem is ready
   */
  XVprocSs_Reset(InstancePtr);

  InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function configures the Video Processing subsystem internal blocks
* to power on default configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
static void SetPowerOnDefaultState(XVprocSs *XVprocSsPtr)
{
  XVidC_VideoStream vidStrmIn;
  XVidC_VideoStream vidStrmOut;
  XVidC_VideoWindow win;
  u16 PixPrecisionIndex;
  XVidC_VideoTiming const *TimingPtr;

  /* Setup Default Output Stream configuration */
  vidStrmOut.VmId          = XVIDC_VM_1920x1080_60_P;
  vidStrmOut.ColorFormatId = XVIDC_CSF_RGB;
  vidStrmOut.FrameRate     = XVIDC_FR_60HZ;
  vidStrmOut.IsInterlaced  = FALSE;
  vidStrmOut.ColorDepth    = XVprocSsPtr->Config.ColorDepth;
  vidStrmOut.PixPerClk     = XVprocSsPtr->Config.PixPerClock;

  TimingPtr = XVidC_GetTimingInfo(vidStrmOut.VmId);
  vidStrmOut.Timing = *TimingPtr;

  /* Setup Default Input Stream configuration */
  vidStrmIn.VmId          = XVIDC_VM_1920x1080_60_P;
  vidStrmIn.ColorFormatId = XVIDC_CSF_RGB;
  vidStrmIn.FrameRate     = XVIDC_FR_60HZ;
  vidStrmIn.IsInterlaced  = FALSE;
  vidStrmIn.ColorDepth    = XVprocSsPtr->Config.ColorDepth;
  vidStrmIn.PixPerClk     = XVprocSsPtr->Config.PixPerClock;

  TimingPtr = XVidC_GetTimingInfo(vidStrmIn.VmId);
  vidStrmIn.Timing = *TimingPtr;

  /* Setup Video Processing subsystem input/output  configuration */
  XVprocSs_SetVidStreamIn(XVprocSsPtr,  &vidStrmIn);
  XVprocSs_SetVidStreamOut(XVprocSsPtr, &vidStrmOut);

  /* compute data width supported by Vdma */
  XVprocSsPtr->CtxtData.PixelWidthInBits = XVprocSsPtr->Config.NumVidComponents *
			                               XVprocSsPtr->VidIn.ColorDepth;
  switch(XVprocSsPtr->Config.PixPerClock)
  {
    case XVIDC_PPC_1:
    case XVIDC_PPC_2:
	 if(XVprocSsPtr->Config.ColorDepth == XVIDC_BPC_10)
	 {
	   /* Align the bit width to next byte boundary for this particular case
	    * Num_Channel	Color Depth		PixelWidth		Align
	    * ----------------------------------------------------
	    *    2				10				20			 24
	    *    3				10				30			 32
	    *
	    *    HW will do the bit padding for 20->24 and 30->32
	    */
	   XVprocSsPtr->CtxtData.PixelWidthInBits = ((XVprocSsPtr->CtxtData.PixelWidthInBits + 7)/8)*8;
	 }
	 break;

    default:
	 break;
  }

  /* Set default Pip/Zoom window increment step size */
  switch(XVprocSsPtr->Config.ColorDepth)
  {
    case XVIDC_BPC_8:  PixPrecisionIndex = 0; break;
    case XVIDC_BPC_10: PixPrecisionIndex = 1; break;
    case XVIDC_BPC_12: PixPrecisionIndex = 2; break;
    case XVIDC_BPC_16: PixPrecisionIndex = 3; break;
    default: break;
  }

  XVprocSsPtr->CtxtData.PixelHStepSize = XVprocSs_PixelHStep[XVprocSsPtr->Config.PixPerClock>>1][PixPrecisionIndex];

  if(XVprocSs_IsConfigModeMax(XVprocSsPtr))
  {
    /* Set default Zoom Window */
    win.Width  = 400;
    win.Height = 400;
    win.StartX = win.StartY = 0;

    XVprocSs_SetZoomPipWindow(XVprocSsPtr,
                              XVPROCSS_ZOOM_WIN,
                              &win);

    /* Set default PIP Window */
    XVprocSs_SetZoomPipWindow(XVprocSsPtr,
                              XVPROCSS_PIP_WIN,
                              &win);
  }

  /* Release reset before programming any IP Block */
  XVprocSs_EnableBlock(XVprocSsPtr->RstAxisPtr,  GPIO_CH_RESET_SEL, RESET_MASK_ALL_BLOCKS);

  /* User parameter configuration */
  /* Set default background color for Letter Box (PIP) */
  if(XVprocSsPtr->LboxPtr)
  {
    XV_LboxSetBackgroundColor(XVprocSsPtr->LboxPtr,
                              XLBOX_BKGND_BLACK,
                              XVprocSsPtr->VidOut.ColorFormatId,
                              XVprocSsPtr->VidOut.ColorDepth);
  }
  /* Initialize CSC sub-core layer 2 driver. This block has FW register map */
  if(XVprocSsPtr->CscPtr)
  {
    XV_CscSetPowerOnDefaultState(XVprocSsPtr->CscPtr);
    XV_CscSetColorDepth(XVprocSsPtr->CscPtr, vidStrmIn.ColorDepth);
  }
}

/****************************************************************************/
/**
* This function starts the video subsystem including all sub-cores that are
* included in the processing pipeline for a given use-case. Video pipe is
* started from back to front
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
* @note Cores are started only if the corresponding start flag in the scratch
*       pad memory is set. This allows to selectively start only those cores
*       included in the processing chain
******************************************************************************/
void XVprocSs_Start(XVprocSs *InstancePtr)
{
  u8 *StartCorePtr;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  StartCorePtr = &InstancePtr->CtxtData.StartCore[0];
  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Start Video Processing Subsystem.... \r\n");

  if(StartCorePtr[XVPROCSS_SUBCORE_CR_V_OUT])
    XV_VCrsmplStart(InstancePtr->VcrsmplrOutPtr);

  if(StartCorePtr[XVPROCSS_SUBCORE_CR_H])
    XV_HCrsmplStart(InstancePtr->HcrsmplrPtr);

  if(StartCorePtr[XVPROCSS_SUBCORE_CSC])
    XV_CscStart(InstancePtr->CscPtr);

  if(StartCorePtr[XVPROCSS_SUBCORE_LBOX])
    XV_LBoxStart(InstancePtr->LboxPtr);

  if(StartCorePtr[XVPROCSS_SUBCORE_SCALER_H])
    XV_HScalerStart(InstancePtr->HscalerPtr);

  if(StartCorePtr[XVPROCSS_SUBCORE_SCALER_V])
    XV_VScalerStart(InstancePtr->VscalerPtr);

  if(StartCorePtr[XVPROCSS_SUBCORE_VDMA])
    XVprocSs_VdmaStartTransfer(InstancePtr->VdmaPtr);

  if(StartCorePtr[XVPROCSS_SUBCORE_DEINT])
    XV_DeintStart(InstancePtr->DeintPtr);

  if(StartCorePtr[XVPROCSS_SUBCORE_CR_V_IN])
    XV_VCrsmplStart(InstancePtr->VcrsmplrInPtr);

  /* Subsystem ready to accept axis - Enable Video Input */
  XVprocSs_EnableBlock(InstancePtr->RstAxisPtr,  GPIO_CH_RESET_SEL, RESET_MASK_VIDEO_IN);
}

/*****************************************************************************/
/**
* This function stops the video subsystem including all sub-cores
* Stop the video pipe starting from front to back
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XVprocSs_Stop(XVprocSs *InstancePtr)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Stop Video Processing Subsystem.... \r\n");

  if(InstancePtr->VcrsmplrInPtr)
    XV_VCrsmplStop(InstancePtr->VcrsmplrInPtr);

  if(InstancePtr->DeintPtr)
    XV_DeintStop(InstancePtr->DeintPtr);

  if(InstancePtr->VdmaPtr)
    XVprocSs_VdmaStop(InstancePtr->VdmaPtr);

  if(InstancePtr->VscalerPtr)
    XV_VScalerStop(InstancePtr->VscalerPtr);

  if(InstancePtr->HscalerPtr)
    XV_HScalerStop(InstancePtr->HscalerPtr);

  if(InstancePtr->LboxPtr)
    XV_LBoxStop(InstancePtr->LboxPtr);

  if(InstancePtr->CscPtr)
    XV_CscStop(InstancePtr->CscPtr);

  if(InstancePtr->HcrsmplrPtr)
    XV_HCrsmplStop(InstancePtr->HcrsmplrPtr);

  if(InstancePtr->VcrsmplrOutPtr)
    XV_VCrsmplStop(InstancePtr->VcrsmplrOutPtr);
}

/*****************************************************************************/
/**
* This function resets the video subsystem sub-cores. There are 2 reset
* networks within the subsystem
*  - For cores that are on AXIS interface
*  - For cores that are on AXI-MM interface
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XVprocSs_Reset(XVprocSs *InstancePtr)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Reset Video Processing Subsystem.... \r\n");

  /* Soft Reset */
  XVprocSs_VdmaReset(InstancePtr->VdmaPtr);

  /* Reset All IP Blocks on AXIS interface*/
  XVprocSs_ResetBlock(InstancePtr->RstAxisPtr,  GPIO_CH_RESET_SEL, RESET_MASK_ALL_BLOCKS);
  /* Reset All IP Blocks on AXI-MM interface*/
//  XVprocSs_ResetBlock(InstancePtr->RstAximmPtr, GPIO_CH_RESET_SEL, RESET_MASK_IP_AXIMM);
  /* If enabled, Stop AXI-MM traffic */
   if((InstancePtr->DeintPtr) && (InstancePtr->CtxtData.StartCore[XVPROCSS_SUBCORE_DEINT]))
   {
     XV_DeintStop(InstancePtr->DeintPtr);
   }

   WaitUs(InstancePtr, 100); /* hold reset line for 100us */
  /*
   * Make sure the video IP's are out of reset - IP's cannot be programmed when held
   * in reset. Will cause Axi-Lite bus to lock.
   * Release IP reset - but hold vid_in in reset
   */
//  XVprocSs_EnableBlock(InstancePtr->RstAximmPtr, GPIO_CH_RESET_SEL, RESET_MASK_IP_AXIMM);
//  Waitms(InstancePtr, 10); /* wait for AXI-MM IP's to stabilize */
  XVprocSs_EnableBlock(InstancePtr->RstAxisPtr,  GPIO_CH_RESET_SEL, RESET_MASK_IP_AXIS);
  WaitUs(InstancePtr, 1000); /* wait 1ms for AXIS to stabilize */

  /* Reset start core flags */
  memset(InstancePtr->CtxtData.StartCore, 0, sizeof(InstancePtr->CtxtData.StartCore));
}

/*****************************************************************************/
/**
* This function configures the video subsystem input interface
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  StrmIn is the pointer to input stream configuration
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
******************************************************************************/
int XVprocSs_SetVidStreamIn(XVprocSs *InstancePtr,
                            const XVidC_VideoStream *StrmIn)
{
  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(StrmIn != NULL);
  Xil_AssertNonvoid((StrmIn->Timing.HActive > 0) &&
                    (StrmIn->Timing.VActive > 0));

  /* set stream properties */
  InstancePtr->VidIn = *StrmIn;

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function configures the video subsystem output interface
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  StrmOut is the pointer to input stream configuration
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
******************************************************************************/
int XVprocSs_SetVidStreamOut(XVprocSs *InstancePtr,
                             const XVidC_VideoStream *StrmOut)
{
  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(StrmOut != NULL);
  Xil_AssertNonvoid((StrmOut->Timing.HActive > 0) &&
                    (StrmOut->Timing.VActive > 0));

  /* set stream properties */
  InstancePtr->VidOut = *StrmOut;

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function validates the video mode id against the supported resolutions
* and if successful extracts the timing information for the mode
*
* @param  StreamPtr is a pointer to the video stream to be configured
* @param  VmId is the Video Mode ID of the new resolution to be set
* @param  Timing is the timing parameters of the new resolution to be set

* @return XST_SUCCESS if successful else XST_FAILURE
*
******************************************************************************/
int XVprocSs_SetStreamResolution(XVidC_VideoStream *StreamPtr,
                                 const XVidC_VideoMode VmId,
                                 XVidC_VideoTiming const *Timing)
{
  int status;

  /* Verify arguments */
  Xil_AssertNonvoid(StreamPtr != NULL);
  Xil_AssertNonvoid(VmId != XVIDC_VM_NOT_SUPPORTED);
  Xil_AssertNonvoid(Timing != NULL);
  Xil_AssertNonvoid((Timing->HActive > 0) &&
                    (Timing->VActive > 0));


  /* update stream timing properties */
  StreamPtr->VmId   = VmId;
  StreamPtr->Timing = *Timing;

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function updates the Pip/Zoom window currently on screen in-place.
* This implies the video is not blanked and the new coordinates will update
* instantly as the function executes
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
* @note This function must be called only after the respective mode (PIP/Zoom)
*       has been enabled and user wants to move window to a new location
*       This function is not applicable in Subsystem Stream Mode Configuration
*
******************************************************************************/
void XVprocSs_UpdateZoomPipWindow(XVprocSs *InstancePtr)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  if(XVprocSs_IsConfigModeMax(InstancePtr))
  {
    /* send Vdma update window to IP */
    if(XVprocSs_IsPipModeOn(InstancePtr))
    {
      XVprocSs_VdmaSetWinToDnScaleMode(InstancePtr, XVPROCSS_VDMA_UPDATE_WR_CH);
    }
    else
    {
      XVprocSs_VdmaSetWinToUpScaleMode(InstancePtr, XVPROCSS_VDMA_UPDATE_RD_CH);
    }

    XVprocSs_VdmaStartTransfer(InstancePtr->VdmaPtr);

    /*
     * Final output of Video Processing subsystem goes via LBox IP
     * Program the output resolution window
     */
    if(XVprocSs_IsPipModeOn(InstancePtr))
    {
      XV_LBoxSetActiveWin(InstancePtr->LboxPtr,
                          &InstancePtr->CtxtData.WrWindow,
                          InstancePtr->VidOut.Timing.HActive,
                          InstancePtr->VidOut.Timing.VActive);
    }
  }
  else //Scaler Only Config
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported in Streaming Mode\r\n");
  }
}

/*****************************************************************************/
/**
* This function allows user to set the Zoom or PIP window. Scratch pad memory
* is updated with the new window information
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  mode is feature to be updated PIP or ZOOM
* @param  win is structure that contains window coordinates and size
*
* @return None
*
* @note For Zoom mode RD client window is written in scratch pad memory
*       For Pip mode WR client window is written in scratch pad memory
*       This function is not applicable in Subsystem Stream Mode Configuration
*
******************************************************************************/
void XVprocSs_SetZoomPipWindow(XVprocSs *InstancePtr,
                               XVprocSs_Win   mode,
                               XVidC_VideoWindow *win)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(win != NULL);

  if(XVprocSs_IsConfigModeMax(InstancePtr))
  {
    if(mode == XVPROCSS_ZOOM_WIN)
    {
      /* If DMA engine does not support unaligned transfers then
       * - align window StartX to next PixelHStepSize boundary
       * - align window size to 2*Pixels/Clock
       */
      if(!InstancePtr->VdmaPtr->ReadChannel.HasDRE)
      {
	u32 AlignStartX, AlignWidth;

	AlignStartX = InstancePtr->CtxtData.PixelHStepSize;
	AlignWidth  = 2*InstancePtr->Config.PixPerClock;

	win->StartX = ((win->StartX + AlignStartX - 1)/AlignStartX)*AlignStartX;
	win->Width  = ((win->Width  + AlignWidth  - 1)/AlignWidth)*AlignWidth;
      }
      //VDMA RD Client
      InstancePtr->CtxtData.RdWindow.StartX = win->StartX;
      InstancePtr->CtxtData.RdWindow.StartY = win->StartY;
      InstancePtr->CtxtData.RdWindow.Width  = win->Width;
      InstancePtr->CtxtData.RdWindow.Height = win->Height;
    }
    else //PIP
    {
      /* If DMA engine does not support unaligned transfers then
       * - align window StartX to next PixelHStepSize boundary
       * - align window size to 2*Pixels/Clock
       */
      if(!InstancePtr->VdmaPtr->WriteChannel.HasDRE)
      {
        u32 AlignStartX, AlignWidth;

	AlignStartX = InstancePtr->CtxtData.PixelHStepSize;
	AlignWidth  = 2*InstancePtr->Config.PixPerClock;

	win->StartX = ((win->StartX + AlignStartX - 1)/AlignStartX)*AlignStartX;
	win->Width  = ((win->Width  + AlignWidth  - 1)/AlignWidth)*AlignWidth;
      }
      //VDMA WR Client
      InstancePtr->CtxtData.WrWindow.StartX = win->StartX;
      InstancePtr->CtxtData.WrWindow.StartY = win->StartY;
      InstancePtr->CtxtData.WrWindow.Width  = win->Width;
      InstancePtr->CtxtData.WrWindow.Height = win->Height;
    }
  }
  else //Scaler Only Config
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported in Streaming Mode\r\n");
  }
}

/*****************************************************************************/
/**
* This function reads the user defined Zoom/Pip window from scratch pad memory
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  mode is feature (PIP or ZOOM) whose window coordinates are to be
*         retrieved
* @param  win is structure that will contain read window coordinates and size
*
** @note For Zoom mode RD client window is read from scratch pad memory
*        For Pip mode WR client window is read from scratch pad memory
*        This function is not applicable in Subsystem Stream Mode Configuration
*
******************************************************************************/
void XVprocSs_GetZoomPipWindow(XVprocSs *InstancePtr,
                               XVprocSs_Win   mode,
                               XVidC_VideoWindow *win)
{
  /* Verify arguments */
   Xil_AssertVoid(InstancePtr != NULL);
   Xil_AssertVoid(win != NULL);

   if(XVprocSs_IsConfigModeMax(InstancePtr))
   {
     if(mode == XVPROCSS_ZOOM_WIN)
     {
       win->StartX = InstancePtr->CtxtData.RdWindow.StartX;
       win->StartY = InstancePtr->CtxtData.RdWindow.StartY;
       win->Width  = InstancePtr->CtxtData.RdWindow.Width;
       win->Height = InstancePtr->CtxtData.RdWindow.Height;
     }
     else //PIP
     {
       win->StartX = InstancePtr->CtxtData.WrWindow.StartX;
       win->StartY = InstancePtr->CtxtData.WrWindow.StartY;
       win->Width  = InstancePtr->CtxtData.WrWindow.Width;
       win->Height = InstancePtr->CtxtData.WrWindow.Height;
     }
   }
   else //Scaler Only Config
   {
	 xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported in Streaming Mode\r\n");
   }
}

/*****************************************************************************/
/**
* This function configures the video subsystem to enable/disable ZOOM feature
* If ZOOM mode is set to ON but user has not set window coordinates then
* quarter of input stream resolution at coordinates 0,0 is set as the default
* zoom window
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  OnOff is the action required
*
* @return None
*
* @note User must call XVprocSs_ConfigureSubsystem() for change to take effect
*       This call has not been added here such that it provides an opportunity
*       to make the change during vertical blanking at system level. This
*       behavior will change once shadow register support is available in
*       sub-core IP's
*       This function is not applicable in Subsystem Stream Mode Configuration
*
******************************************************************************/
void XVprocSs_SetZoomMode(XVprocSs *InstancePtr, u8 OnOff)
{
  char *status[] = {"OFF","ON"};

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  if(XVprocSs_IsConfigModeMax(InstancePtr))
  {
    InstancePtr->CtxtData.ZoomEn = OnOff;
    InstancePtr->CtxtData.PipEn  = FALSE;

    xdbg_printf(XDBG_DEBUG_GENERAL,"\r\n  :ZOOM Mode %s \r\n", status[InstancePtr->CtxtData.ZoomEn]);
  }
  else //Scaler Only Config
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported in Streaming Mode\r\n");
  }
}

/*****************************************************************************/
/**
* This function configures the video subsystem to enable/disable PIP feature
* If PIP mode is set to ON but user has not set window coordinates then
* half of input stream resolution at coordinates 0,0 is set as the default
* zoom window
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  OnOff is the action required
*
* @return None
*
* @note User must call XVprocSs_ConfigureSubsystem() for change to take effect
*       This call has not been added here such that it provides an opportunity
*       to make the change during vertical blanking at system level. This
*       behavior will change once shadow register support is available in
*       sub-core IP's
*       This function is not applicable in Subsystem Stream Mode Configuration
*
******************************************************************************/
void XVprocSs_SetPipMode(XVprocSs *InstancePtr, u8 OnOff)
{
  char *status[] = {"OFF","ON"};

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  if(XVprocSs_IsConfigModeMax(InstancePtr))
  {
    InstancePtr->CtxtData.PipEn  = OnOff;
    InstancePtr->CtxtData.ZoomEn = FALSE;

    xdbg_printf(XDBG_DEBUG_GENERAL,"\r\n  :PIP Mode %s \r\n", status[InstancePtr->CtxtData.PipEn]);
  }
  else //Scaler Only Config
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported in Streaming Mode\r\n");
  }
}

/*****************************************************************************/
/**
* This function validates the input and output stream configuration for scaler
* only configuration
*
* @param  pStrmIn is a pointer to the input stream
* @param  pStrmOut is a pointer to the output stream
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
* @note This function is applicable only for Stream mode configuration of the
*       subsystem. In this mode very limited functionality is available
******************************************************************************/
static int ValidateScalerOnlyConfig(XVidC_VideoStream *pStrmIn,
                                    XVidC_VideoStream *pStrmOut)
{
  if(pStrmIn->ColorFormatId == XVIDC_CSF_YCRCB_420)
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR: YUV420 Input not supported\r\n");
    return(XST_FAILURE);
  }

  if(pStrmIn->ColorFormatId != pStrmOut->ColorFormatId)
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR: Input & Output Stream Color Format different\r\n");
    return(XST_FAILURE);
  }

  if(pStrmIn->ColorDepth != pStrmOut->ColorDepth)
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR: Input & Output Color Depth different\r\n");
    return(XST_FAILURE);
  }

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function configures the video subsystem pipeline for ScalerOnly
* topology of the subsystem
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
* @note If use case is possible the subsystem will configure the sub-cores
*       accordingly else will ignore the request
*
******************************************************************************/
static int SetupModeScalerOnly(XVprocSs *XVprocSsPtr)
{
  u32 vsc_WidthIn, vsc_HeightIn, vsc_HeightOut;
  u32 hsc_HeightIn, hsc_WidthIn, hsc_WidthOut;
  int status = XST_SUCCESS;

  vsc_WidthIn = vsc_HeightIn = vsc_HeightOut = 0;
  hsc_HeightIn = hsc_WidthIn = hsc_WidthOut = 0;

  if((!XVprocSsPtr->VscalerPtr) || (!XVprocSsPtr->HscalerPtr))
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Scaler IP not found\r\n");
    return(XST_FAILURE);
  }

  /* check if input/output stream configuration is supported */
  status = ValidateScalerOnlyConfig(&XVprocSsPtr->VidIn,
                                    &XVprocSsPtr->VidOut);

  if(status ==  XST_SUCCESS)
  {
    /* Reset All IP Blocks */
    XVprocSs_Reset(XVprocSsPtr);

    /* UpScale mode V Scaler is before H Scaler */
    vsc_WidthIn   = XVprocSsPtr->VidIn.Timing.HActive;
    vsc_HeightIn  = XVprocSsPtr->VidIn.Timing.VActive;
    vsc_HeightOut = XVprocSsPtr->VidOut.Timing.VActive;

    hsc_WidthIn  = vsc_WidthIn;
    hsc_HeightIn = vsc_HeightOut;
    hsc_WidthOut = XVprocSsPtr->VidOut.Timing.HActive;

    /* Configure scaler to scale input to output resolution */
    xdbg_printf(XDBG_DEBUG_GENERAL,"  -> Configure VScaler for %dx%d to %dx%d\r\n", \
            (int)vsc_WidthIn, (int)vsc_HeightIn, (int)vsc_WidthIn, (int)vsc_HeightOut);

    XV_VScalerSetup(XVprocSsPtr->VscalerPtr,
                    vsc_WidthIn,
                    vsc_HeightIn,
                    vsc_HeightOut);

    xdbg_printf(XDBG_DEBUG_GENERAL,"  -> Configure HScaler for %dx%d to %dx%d\r\n", \
                       (int)hsc_WidthIn, (int)hsc_HeightIn, (int)hsc_WidthOut, (int)hsc_HeightIn);

    XV_HScalerSetup(XVprocSsPtr->HscalerPtr,
                    hsc_HeightIn,
                    hsc_WidthIn,
                    hsc_WidthOut,
                    XVprocSsPtr->VidIn.ColorFormatId);

    /* Start Scaler sub-cores */
    XV_HScalerStart(XVprocSsPtr->HscalerPtr);
    XV_VScalerStart(XVprocSsPtr->VscalerPtr);

    /* Subsystem Ready to accept input stream - Enable Video Input */
    XVprocSs_EnableBlock(XVprocSsPtr->RstAxisPtr,  GPIO_CH_RESET_SEL, RESET_MASK_VIDEO_IN);
  }
  else
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"\r\n-->Command Ignored<--\r\n");
  }
  return(status);
}

/*****************************************************************************/
/**
* This function configures the video subsystem pipeline for Maximum
* (Full_Fledged) topology
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
******************************************************************************/
static int SetupModeMax(XVprocSs *XVprocSsPtr)
{
  int status;

  /* Build Routing table */
  status = XVprocSs_BuildRoutingTable(XVprocSsPtr);

  if(status == XST_SUCCESS)
  {
    /* Reset All IP Blocks */
    XVprocSs_Reset(XVprocSsPtr);

    /* Set AXI Switch registers */
    XVprocSs_ProgRouterMux(XVprocSsPtr);

    /* program use case */
    XVprocSs_SetupRouterDataFlow(XVprocSsPtr);
  }
  else
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR: Subsystem Routing Table Invalid");
	xdbg_printf(XDBG_DEBUG_GENERAL,"- Ignoring Configuration Request\r\n");
  }
  return(status);
}


/*****************************************************************************/
/**
* This function validates the input and output stream configuration against the
* Subsystem hardware capabilities
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
******************************************************************************/
static int ValidateSubsystemConfig(XVprocSs *InstancePtr)
{
  XVidC_VideoStream *StrmIn  = &InstancePtr->VidIn;
  XVidC_VideoStream *StrmOut = &InstancePtr->VidOut;

  /* Runtime Color Depth conversion not supported
   * Always overwrite input/output stream color depth with subsystem setting
   */
  StrmIn->ColorDepth  = InstancePtr->Config.ColorDepth;
  StrmOut->ColorDepth = InstancePtr->Config.ColorDepth;

  /* Runtime Pixel/Clock conversion not supported
   * Always overwrite input/output stream pixel/clk with subsystem setting
   */
  StrmIn->PixPerClk  = InstancePtr->Config.PixPerClock;
  StrmOut->PixPerClk = InstancePtr->Config.PixPerClock;

  /* Check Stream Width is aligned at Samples/Clock boundary */
  if(((StrmIn->Timing.HActive  % InstancePtr->Config.PixPerClock) != 0) ||
     ((StrmOut->Timing.HActive % InstancePtr->Config.PixPerClock) != 0))
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Input/Output Width not aligned with Samples/Clk\r\n");
	return(XST_FAILURE);
  }

  /* Check if Subsystem HW Configuration can process requested resolution*/
  if(((StrmIn->PixPerClk  == XVIDC_PPC_1) && (StrmIn->VmId > XVIDC_VM_3840x2160_30_P))||
	 ((StrmOut->PixPerClk == XVIDC_PPC_1) && (StrmOut->VmId > XVIDC_VM_3840x2160_30_P)))
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: 1 Sample/Clk can support Max resolution of 4K2K@30Hz \r\n");
	return(XST_FAILURE);
  }

  /* Check for YUV422 In/Out stream width is even */
  if(((StrmIn->ColorFormatId  == XVIDC_CSF_YCRCB_422) && ((StrmIn->Timing.HActive % 2) != 0)) ||
	 ((StrmOut->ColorFormatId == XVIDC_CSF_YCRCB_422) && ((StrmOut->Timing.HActive % 2) != 0)))
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: YUV422 stream width must be even\r\n");
	return(XST_FAILURE);
  }

  /* Check for YUV420 In stream width and height is even */
  if(StrmIn->ColorFormatId == XVIDC_CSF_YCRCB_420)
  {
	if(InstancePtr->VcrsmplrInPtr)
	{
      if(((StrmIn->Timing.HActive % 2) != 0) && ((StrmIn->Timing.VActive % 2) != 0))
	  {
	    xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: YUV420 input stream width and height must be even\r\n");
	    return(XST_FAILURE);
	  }
	}
	else
	{
	  xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Vertical Chroma Resampler IP not found. YUV420 Input not supported\r\n");
	  return(XST_FAILURE);
	}
  }

  /* Check for YUV420 out stream width and height is even */
  if(StrmOut->ColorFormatId == XVIDC_CSF_YCRCB_420)
  {
	if(InstancePtr->VcrsmplrOutPtr)
	{
      if(((StrmOut->Timing.HActive % 2) != 0) && ((StrmOut->Timing.VActive % 2) != 0))
	  {
	    xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: YUV420 output stream width and height must be even\r\n");
	    return(XST_FAILURE);
	  }
	}
	else
	{
	  xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Vertical Chroma Resampler IP not found. YUV420 Output not supported\r\n");
	  return(XST_FAILURE);
	}
  }

  /* Check for Interlaced input limitation */
  if(StrmIn->IsInterlaced)
  {
	if(StrmIn->ColorFormatId == XVIDC_CSF_YCRCB_420)
	{
	  xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Interlaced YUV420 stream not supported\r\n");
	  return(XST_FAILURE);
	}
	if(!InstancePtr->DeintPtr)
	{
	  xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Interlaced input not supported\r\n");
	  return(XST_FAILURE);
	}
  }
  return(XST_SUCCESS);
}


/*****************************************************************************/
/**
* This function is the entry point into the video processing subsystem driver
* processing path. It will examine the instantiated subsystem configuration mode
* and the input and output stream configuration. Based on the available
* information control flow is determined and requisite sub-cores are configured
* to implement the supported use case
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
******************************************************************************/
int XVprocSs_SetSubsystemConfig(XVprocSs *InstancePtr)
{
  int status = XST_SUCCESS;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);

#ifdef DEBUG
  xil_printf("\r\n****** VPROC SUBSYSTEM INPUT/OUTPUT CONFIG ******\r\n");
  xil_printf("->INPUT\r\n");
  XVidC_ReportStreamInfo(&InstancePtr->VidIn);
  xil_printf("\r\n->OUTPUT\r\n");
  XVidC_ReportStreamInfo(&InstancePtr->VidOut);
  xil_printf("**************************************************\r\n\r\n");
#endif

  /* validate subsystem configuration */
  if(ValidateSubsystemConfig(InstancePtr) != XST_SUCCESS)
  {
	 return(XST_FAILURE);
  }

  switch(XVprocSs_GetSubsystemTopology(InstancePtr))
  {
    case XVPROCSS_TOPOLOGY_FULL_FLEDGED:
        status = SetupModeMax(InstancePtr);
        break;

    case XVPROCSS_TOPOLOGY_SCALER_ONLY:
        //Only configuration supported is V->H
        status = SetupModeScalerOnly(InstancePtr);
        break;

    default:
	xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR: Subsystem Configuration Mode Not Supported. \r\n");
        status = XST_FAILURE;
        break;
  }
  return(status);
}

/*****************************************************************************/
/**
* This function returns picture brighntess setting
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return current value (0-100), if csc subcore is included
*
******************************************************************************/
s32 XVprocSs_GetPictureBrightness(XVprocSs *InstancePtr)
{
  s32 Retval;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->CscPtr != NULL);

  if(InstancePtr->CscPtr)
  {
	Retval = XV_CscGetBrightness(InstancePtr->CscPtr);
  }
  return(Retval);
}

/*****************************************************************************/
/**
* This function updates picture brighntess setting with specified value
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  NewValue is the new value to be written
* 			- Range: 0-100
*
* @return None
*
* @note   Applicable only if CSC core is included in the subsystem
*
******************************************************************************/
void XVprocSs_SetPictureBrightness(XVprocSs *InstancePtr, s32 NewValue)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->CscPtr != NULL);
  Xil_AssertVoid((NewValue >= 0) && (NewValue <= 100));

  if(InstancePtr->CscPtr)
  {
	XV_CscSetBrightness(InstancePtr->CscPtr, NewValue);
  }
}

/*****************************************************************************/
/**
* This function returns picture contrast setting
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return current value (0-100), if csc subcore is included
*
******************************************************************************/
s32 XVprocSs_GetPictureContrast(XVprocSs *InstancePtr)
{
  s32 Retval;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->CscPtr != NULL);

  if(InstancePtr->CscPtr)
  {
	Retval = XV_CscGetContrast(InstancePtr->CscPtr);
  }
  return(Retval);
}

/*****************************************************************************/
/**
* This function updates picture contrast setting with specified value
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  NewValue is the new value to be written
* 			- Range: 0-100
*
* @return None
*
* @note   Applicable only if CSC core is included in the subsystem
*
******************************************************************************/
void XVprocSs_SetPictureContrast(XVprocSs *InstancePtr, s32 NewValue)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->CscPtr != NULL);
  Xil_AssertVoid((NewValue >= 0) && (NewValue <= 100));

  if(InstancePtr->CscPtr)
  {
	XV_CscSetContrast(InstancePtr->CscPtr, NewValue);
  }
}

/*****************************************************************************/
/**
* This function returns picture saturation setting
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return current value (0-100), if csc subcore is included
*
******************************************************************************/
s32 XVprocSs_GetPictureSaturation(XVprocSs *InstancePtr)
{
  s32 Retval;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->CscPtr != NULL);

  if(InstancePtr->CscPtr)
  {
	Retval = XV_CscGetSaturation(InstancePtr->CscPtr);
  }
  return(Retval);
}

/*****************************************************************************/
/**
* This function updates picture saturation setting with specified value
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  NewValue is the new value to be written
* 			- Range: 0-100
*
* @return None
*
* @note   Applicable only if CSC core is included in the subsystem
*
******************************************************************************/
void XVprocSs_SetPictureSaturation(XVprocSs *InstancePtr, s32 NewValue)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->CscPtr != NULL);
  Xil_AssertVoid((NewValue >= 0) && (NewValue <= 100));

  if(InstancePtr->CscPtr)
  {
	XV_CscSetSaturation(InstancePtr->CscPtr, NewValue);
  }
}

/*****************************************************************************/
/**
* This function returns picture gain setting for the specified color channel
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  ChId is the color channel id for which gain is requested
*
* @return current value (0-100), if csc subcore is included
*
******************************************************************************/
s32 XVprocSs_GetPictureGain(XVprocSs *InstancePtr, XVprocSs_ColorChannel ChId)
{
  s32 Retval;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->CscPtr != NULL);
  Xil_AssertNonvoid((ChId >= XVPROCSS_COLOR_CH_Y_RED) &&
		            (ChId < XVPROCSS_COLOR_CH_NUM_SUPPORTED));

  if(InstancePtr->CscPtr)
  {
	switch(ChId)
	{
	  case XVPROCSS_COLOR_CH_Y_RED:
			Retval = XV_CscGetRedGain(InstancePtr->CscPtr);
			break;

	  case XVPROCSS_COLOR_CH_CB_GREEN:
			Retval = XV_CscGetGreenGain(InstancePtr->CscPtr);
			break;

	  case XVPROCSS_COLOR_CH_CR_BLUE:
			Retval = XV_CscGetBlueGain(InstancePtr->CscPtr);
			break;

	  default:
		    break;
	}
  }
  else
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported\r\n");
  }
  return(Retval);
}

/*****************************************************************************/
/**
* This function updates picture gain setting with specified value
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  ChId is the color channel id for which gain is to be updated
* @param  NewValue is the new value to be written
* 			- Range: 0-100
*
* @return none
*
* @note   Applicable only if CSC core is included in the subsystem
*
******************************************************************************/
void XVprocSs_SetPictureGain(XVprocSs *InstancePtr,
		                     XVprocSs_ColorChannel ChId,
		                     s32 NewValue)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->CscPtr != NULL);
  Xil_AssertVoid((ChId >= XVPROCSS_COLOR_CH_Y_RED) &&
		         (ChId <= XVPROCSS_COLOR_CH_NUM_SUPPORTED));
  Xil_AssertVoid((NewValue >= 0) && (NewValue <= 100));

  if(InstancePtr->CscPtr)
  {
    switch(ChId)
    {
      case XVPROCSS_COLOR_CH_Y_RED:
            XV_CscSetRedGain(InstancePtr->CscPtr, NewValue);
			break;

	  case XVPROCSS_COLOR_CH_CB_GREEN:
            XV_CscSetGreenGain(InstancePtr->CscPtr, NewValue);
			break;

	  case XVPROCSS_COLOR_CH_CR_BLUE:
            XV_CscSetBlueGain(InstancePtr->CscPtr, NewValue);
			break;

	  default:
			break;
    }
  }
}

/*****************************************************************************/
/**
* This function returns picture color standard setting for input
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return Current set color standard, if csc core is included
*		    - XVIDC_BT_2020
*		    - XVIDC_BT_709
*		    - XVIDC_BT_601
*		   Else
*		     XVIDC_BT_UNKNOWN
*
******************************************************************************/
XVidC_ColorStd XVprocSs_GetPictureColorStdIn(XVprocSs *InstancePtr)
{
  XVidC_ColorStd Retval = XVIDC_BT_UNKNOWN;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->CscPtr != NULL);

  if(InstancePtr->CscPtr)
  {
	Retval = XV_CscGetColorStdIn(InstancePtr->CscPtr);
  }
  return(Retval);
}

/*****************************************************************************/
/**
* This function returns picture color standard setting for output
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return Current set color standard, if csc core is included
*		    - XVIDC_BT_2020
*		    - XVIDC_BT_709
*		    - XVIDC_BT_601
*		   Else
*		     XVIDC_BT_UNKNOWN
*
******************************************************************************/
XVidC_ColorStd XVprocSs_GetPictureColorStdOut(XVprocSs *InstancePtr)
{
  XVidC_ColorStd Retval = XVIDC_BT_UNKNOWN;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->CscPtr != NULL);

  if(InstancePtr->CscPtr)
  {
	Retval = XV_CscGetColorStdOut(InstancePtr->CscPtr);
  }
  return(Retval);
}

/*****************************************************************************/
/**
* This function sets picture color standard setting for input
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  NewVal is the required color standard
*		    - XVIDC_BT_2020
*		    - XVIDC_BT_709
*		    - XVIDC_BT_601
*
* @return None
*
* @note   Applicable only if CSC core is included in the subsystem
*
******************************************************************************/
void XVprocSs_SetPictureColorStdIn(XVprocSs *InstancePtr,
	                               XVidC_ColorStd NewVal)
{
  XV_Csc_l2 *CscPtr = InstancePtr->CscPtr;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->CscPtr != NULL);
  Xil_AssertVoid((NewVal >= XVIDC_BT_2020) &&
		         (NewVal < XVIDC_BT_NUM_SUPPORTED));

  if(InstancePtr->CscPtr)
  {
    XV_CscSetColorspace(CscPtr,
                        CscPtr->ColorFormatIn,
                        CscPtr->ColorFormatOut,
                        NewVal,
                        CscPtr->StandardOut,
                        CscPtr->OutputRange);
  }
}

/*****************************************************************************/
/**
* This function sets picture color standard setting for output
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  NewVal is the required color standard
*		    - XVIDC_BT_2020
*		    - XVIDC_BT_709
*		    - XVIDC_BT_601
*
* @return None
*
* @note   Applicable only if CSC core is included in the subsystem
*
******************************************************************************/
void XVprocSs_SetPictureColorStdOut(XVprocSs *InstancePtr,
	                                XVidC_ColorStd NewVal)
{
  XV_Csc_l2 *CscPtr = InstancePtr->CscPtr;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->CscPtr != NULL);
  Xil_AssertVoid((NewVal >= XVIDC_BT_2020) &&
		         (NewVal < XVIDC_BT_NUM_SUPPORTED));

  if(InstancePtr->CscPtr)
  {
    XV_CscSetColorspace(CscPtr,
                        CscPtr->ColorFormatIn,
                        CscPtr->ColorFormatOut,
                        CscPtr->StandardIn,
                        NewVal,
                        CscPtr->OutputRange);
  }
}

/*****************************************************************************/
/**
* This function returns picture color range for output
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return Current set output range, if csc core is included
*    		- XVIDC_CR_16_235
*	    	- XVIDC_CR_16_240
*		    - XVIDC_CR_0_255
*		  Else
*		     XVIDC_CR_UNKNOWN_RANGE
*
******************************************************************************/
XVidC_ColorRange XVprocSs_GetPictureColorRange(XVprocSs *InstancePtr)
{
  XVidC_ColorRange Retval = XVIDC_CR_UNKNOWN_RANGE;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->CscPtr != NULL);

  if(InstancePtr->CscPtr)
  {
	Retval = XV_CscGetOutputRange(InstancePtr->CscPtr);
  }
  return(Retval);
}

/*****************************************************************************/
/**
* This function sets picture color range for output
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  NewVal is the required color range
*    		- XVIDC_CR_16_235
*	    	- XVIDC_CR_16_240
*		    - XVIDC_CR_0_255
*
* @return None
*
* @note   Applicable only if CSC core is included in the subsystem
*
******************************************************************************/
void XVprocSs_SetPictureColorRange(XVprocSs *InstancePtr,
	                               XVidC_ColorRange NewVal)
{
  XV_Csc_l2 *CscPtr = InstancePtr->CscPtr;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->CscPtr != NULL);
  Xil_AssertVoid((NewVal >= XVIDC_CR_16_235) &&
		         (NewVal < XVIDC_CR_NUM_SUPPORTED));

  if(InstancePtr->CscPtr)
  {
    XV_CscSetColorspace(CscPtr,
                        CscPtr->ColorFormatIn,
                        CscPtr->ColorFormatOut,
                        CscPtr->StandardIn,
                        CscPtr->StandardOut,
                        NewVal);
  }
}

/*****************************************************************************/
/**
* This function sets picture active window.
* Post this function call all further picture settings will apply only within
* the defined window. Active window gets reset everytime subsystem
* configuration changes
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  Win is the pointer to window coordinates within which picture
*         settings should be applied
*
* @return XST_SUCCESS if window is valid else XST_FAILURE
*
* @note   Applicable only if CSC core is included in the subsystem
*
******************************************************************************/
int XVprocSs_SetPictureActiveWindow(XVprocSs *InstancePtr,
	                                XVidC_VideoWindow *Win)
{
  u16 width, height;
  int status = XST_FAILURE;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(InstancePtr->CscPtr != NULL);
  Xil_AssertNonvoid(Win != NULL);

  if(InstancePtr->CscPtr)
  {
    width  = XV_csc_Get_HwReg_width(&InstancePtr->CscPtr->Csc);
	height = XV_csc_Get_HwReg_height(&InstancePtr->CscPtr->Csc);

	//Check if window is within the active frame resolution
	if(((Win->StartX < 0) || (Win->StartX > width))  ||
	   ((Win->StartY < 0) || (Win->StartY > height)) ||
	   ((Win->StartX + Win->Width) > width)         ||
	   ((Win->StartY + Win->Height) > height))
	{
	  status = XST_FAILURE;
	}
	else
	{
	  status = XST_SUCCESS;
	}

	if(status == XST_SUCCESS)
	{
	  XV_CscSetDemoWindow(InstancePtr->CscPtr, Win);
	}
  }
  return(status);
}

/*****************************************************************************/
/**
* This function sets PIP background color
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  ColorIs is the requested background color
*           - XLBOX_BKGND_BLACK
*           - XLBOX_BKGND_WHITE
*           - XLBOX_BKGND_RED
*           - XLBOX_BKGND_GREEN
*           - XLBOX_BKGND_BLUE
*
* @return None
*
* @note   Applicable only if Letterbox core is included in the subsystem
*
******************************************************************************/
void XVprocSs_SetPIPBackgroundColor(XVprocSs *InstancePtr,
		                            XLboxColorId ColorId)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(InstancePtr->LboxPtr != NULL);
  Xil_AssertVoid((ColorId >= XLBOX_BKGND_BLACK) &&
		         (ColorId < XLBOX_BKGND_LAST));

  if(InstancePtr->LboxPtr)
  {
	XV_LboxSetBackgroundColor(InstancePtr->LboxPtr,
			                  ColorId,
			                  InstancePtr->CtxtData.StrmCformat,
			                  InstancePtr->VidIn.ColorDepth);
  }
  else
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported\r\n");
  }
}

/*****************************************************************************/
/**
* This function enables user to load external filter coefficients for
* Scaler cores, independently for H & V
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  CoreId is the Scaler core to be worked on
*           - XVPROCSS_SUBCORE_SCALER_V
*           - XVPROCSS_SUBCORE_SCALER_H
* @param  num_phases is the number of phases supported by Scaler
* @param  num_taps is the effective taps to be used by Scaler
* @param  Coeff is the pointer to the filter table to be loaded
*
* @return None
*
* @note   Applicable only if Scaler cores are included in the subsystem
*
******************************************************************************/
void XVprocSs_LoadScalerCoeff(XVprocSs *InstancePtr,
		                      u32 CoreId,
                              u16 num_phases,
                              u16 num_taps,
                              const short *Coeff)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(Coeff != NULL);
  Xil_AssertVoid((CoreId == XVPROCSS_SUBCORE_SCALER_V) ||
		         (CoreId == XVPROCSS_SUBCORE_SCALER_H));

  switch(CoreId)
  {
    case XVPROCSS_SUBCORE_SCALER_V:
        if(InstancePtr->VscalerPtr)
	{
	  XV_VScalerLoadExtCoeff(InstancePtr->VscalerPtr,
		                     num_phases,
		                     num_taps,
		                     Coeff);
        }
        else
        {
          xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported\r\n");
        }

	break;

    case XVPROCSS_SUBCORE_SCALER_H:
	if(InstancePtr->HscalerPtr)
	{
	  XV_HScalerLoadExtCoeff(InstancePtr->HscalerPtr,
		                     num_phases,
		                     num_taps,
		                     Coeff);
        }
	else
	{
	  xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported\r\n");
	}
	break;

    default:
	break;
  }
}

/*****************************************************************************/
/**
* This function enables user to load external filter coefficients for
* Chroma Resampler cores, independently for H & V
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  CoreId is the resampler core to be worked on
*           - XVPROCSS_SUBCORE_CR_H
*           - XVPROCSS_SUBCORE_CR_V_IN
*           - XVPROCSS_SUBCORE_CR_V_OUT
* @param  num_taps is the taps of the resampler hw instance
* @param  Coeff is the pointer to the filter table to be loaded
*
* @return None
*
* @note   Applicable only if Resampler cores are included in the subsystem
*
******************************************************************************/
void XVprocSs_LoadChromaResamplerCoeff(XVprocSs *InstancePtr,
		                               u32 CoreId,
                                       u16 num_taps,
                                       const short *Coeff)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(Coeff != NULL);
  Xil_AssertVoid((CoreId == XVPROCSS_SUBCORE_CR_H)    ||
			     (CoreId == XVPROCSS_SUBCORE_CR_V_IN) ||
			     (CoreId == XVPROCSS_SUBCORE_CR_V_OUT));

  switch(CoreId)
  {
	case XVPROCSS_SUBCORE_CR_H:
		if(InstancePtr->HcrsmplrPtr)
		{
		  XV_HCrsmplrLoadExtCoeff(InstancePtr->HcrsmplrPtr, num_taps, Coeff);
		}
		else
		{
		  xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported\r\n");
		}
		break;

	case XVPROCSS_SUBCORE_CR_V_IN:
		if(InstancePtr->VcrsmplrInPtr)
		{
		  XV_VCrsmplrLoadExtCoeff(InstancePtr->VcrsmplrInPtr, num_taps, Coeff);
		}
	    else
		{
		  xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported\r\n");
		}
		break;

	case XVPROCSS_SUBCORE_CR_V_OUT:
		if(InstancePtr->VcrsmplrOutPtr)
		{
		  XV_VCrsmplrLoadExtCoeff(InstancePtr->VcrsmplrOutPtr, num_taps, Coeff);
		}
		else
		{
		  xdbg_printf(XDBG_DEBUG_GENERAL,"\r\nVPROCSS ERR:: Feature not supported\r\n");
		}
		break;

	default:
	    break;
  }
}

/*****************************************************************************/
/**
* This function reports the status of specified sub-core
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  SubcoreId is the subcore index from the below list
*           - XVPROCSS_SUBCORE_SCALER_V
*           - XVPROCSS_SUBCORE_SCALER_H
*           - XVPROCSS_SUBCORE_VDMA
*           - XVPROCSS_SUBCORE_LBOX
*           - XVPROCSS_SUBCORE_CR_H
*           - XVPROCSS_SUBCORE_CR_V_IN
*           - XVPROCSS_SUBCORE_CR_V_OUT
*           - XVPROCSS_SUBCORE_CSC
*           - XVPROCSS_SUBCORE_DEINT
*
* @return None
*
******************************************************************************/
void XVprocSs_ReportSubcoreStatus(XVprocSs *InstancePtr,
		                          u32 SubcoreId)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid((SubcoreId >= XVPROCSS_SUBCORE_SCALER_V) &&
		         (SubcoreId < XVPROCSS_SUBCORE_MAX));

  switch(SubcoreId)
  {
    case XVPROCSS_SUBCORE_SCALER_V:
         XV_VScalerDbgReportStatus(InstancePtr->VscalerPtr);
	 break;

    case XVPROCSS_SUBCORE_SCALER_H:
         XV_HScalerDbgReportStatus(InstancePtr->HscalerPtr);
	 break;

    case XVPROCSS_SUBCORE_VDMA:
         XVprocSs_VdmaDbgReportStatus(InstancePtr->VdmaPtr,
			                      InstancePtr->CtxtData.PixelWidthInBits);
	 break;

    case XVPROCSS_SUBCORE_LBOX:
         XV_LBoxDbgReportStatus(InstancePtr->LboxPtr);
	 break;

    case XVPROCSS_SUBCORE_CR_H:
         XV_HCrsmplDbgReportStatus(InstancePtr->HcrsmplrPtr);
	 break;

    case XVPROCSS_SUBCORE_CR_V_IN:
         XV_VCrsmplDbgReportStatus(InstancePtr->VcrsmplrInPtr);
	 break;

    case XVPROCSS_SUBCORE_CR_V_OUT:
         XV_VCrsmplDbgReportStatus(InstancePtr->VcrsmplrOutPtr);
	 break;

    case XVPROCSS_SUBCORE_CSC:
	 XV_CscDbgReportStatus(InstancePtr->CscPtr);
	 break;

    case XVPROCSS_SUBCORE_DEINT:
	 XV_DeintDbgReportStatus(InstancePtr->DeintPtr);
	 break;

    default:
	 break;
  }
}

/*****************************************************************************/
/**
* This function reports Video Processing Subsystem HW configuration
*
* @param  InstancePtr is a pointer to the Subsystem instance.
*
* @return None
*
******************************************************************************/
void XVprocSs_ReportSubsystemCoreInfo(XVprocSs *InstancePtr)
{
  char *topology[2] = {"Scaler-Only", "Full-Fledged"};
  int SubsytemTopology;

  Xil_AssertVoid(InstancePtr != NULL);

  SubsytemTopology = XVprocSs_GetSubsystemTopology(InstancePtr);

  xil_printf("\r\n****** Video Processing Subsystem Configuration ******\r\n");
  /* Print the configuration selected */
  if(SubsytemTopology < XVPROCSS_TOPOLOGY_NUM_SUPPORTED) {
    xil_printf("\r\nTopology: %s\r\n", topology[SubsytemTopology]);
  } else {
	xil_printf("VPROCSS ERR:: Topology: Not Supported\r\n");
	return;
  }

  xil_printf("\r\n  ->Sub-Cores Included\r\n");

  /* Report all the included cores in the subsystem instance */
  if(InstancePtr->HcrsmplrPtr)
  {
    xil_printf("    : Horiz. Chroma Resampler \r\n");
  }

  if(InstancePtr->VcrsmplrInPtr)
  {
    xil_printf("    : Vert Chroma Resampler - Input\r\n");
  }

  if(InstancePtr->VcrsmplrOutPtr)
  {
    xil_printf("    : Vert Chroma Resampler - Output\r\n");
  }

  if(InstancePtr->HscalerPtr)
  {
    xil_printf("    : H Scaler\r\n");
  }

  if(InstancePtr->VscalerPtr)
  {
    xil_printf("    : V Scaler\r\n");
  }

  if(InstancePtr->VdmaPtr)
  {
    xil_printf("    : VDMA\r\n");
  }

  if(InstancePtr->LboxPtr)
  {
    xil_printf("    : LetterBox\r\n");
  }

  if(InstancePtr->CscPtr)
  {
    xil_printf("    : Color Space Converter\r\n");
  }

  if(InstancePtr->DeintPtr)
  {
    xil_printf("    : Deinterlacer\r\n");
  }

  if(InstancePtr->RstAxisPtr)
  {
    xil_printf("    : Reset (AXIS)\r\n");
  }

  if(InstancePtr->RstAximmPtr)
  {
    xil_printf("    : Reset (AXI-MM) \r\n");
  }

  if(InstancePtr->RouterPtr)
  {
    xil_printf("    : AXIS Router\r\n");
  }

  xil_printf("\r\nPixels/Clk  = %d\r\n", InstancePtr->Config.PixPerClock);
  xil_printf("Color Depth = %d\r\n", InstancePtr->Config.ColorDepth);
  xil_printf("Num Video Components = %d\r\n", InstancePtr->Config.NumVidComponents);
  xil_printf("Max Width Supported  = %d\r\n", InstancePtr->Config.MaxWidth);
  xil_printf("Max Height Supported = %d\r\n", InstancePtr->Config.MaxHeight);
}

/*****************************************************************************/
/**
* This function reports the subsystem HW and input/output stream configuration
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XVprocSs_ReportSubsystemConfig(XVprocSs *InstancePtr)
{
  XVidC_VideoWindow win;
  u32 count;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n------ SUBSYSTEM INPUT/OUTPUT CONFIG ------\r\n");
  xil_printf("->INPUT\r\n");
  XVidC_ReportStreamInfo(&InstancePtr->VidIn);
  xil_printf("\r\n->OUTPUT\r\n");
  XVidC_ReportStreamInfo(&InstancePtr->VidOut);

  if(XVprocSs_IsConfigModeMax(InstancePtr))
  {
    if(XVprocSs_IsZoomModeOn(InstancePtr))
    {
	  xil_printf("\r\nZoom Mode: ON\r\n");
	  XVprocSs_GetZoomPipWindow(InstancePtr, XVPROCSS_ZOOM_WIN, &win);
	  xil_printf("   Start X    = %d\r\n", win.StartX);
	  xil_printf("   Start Y    = %d\r\n", win.StartY);
	  xil_printf("   Win Width  = %d\r\n", win.Width);
	  xil_printf("   Win Height = %d\r\n", win.Height);
	  xil_printf("\r\n   HStep Size = %d\r\n", XVprocSs_GetPipZoomWinHStepSize(InstancePtr));
    }
    else
    {
      xil_printf("\r\nZoom Mode: OFF\r\n");
    }

    if(XVprocSs_IsPipModeOn(InstancePtr))
    {
	  xil_printf("\r\nPip Mode: ON\r\n");
	  XVprocSs_GetZoomPipWindow(InstancePtr, XVPROCSS_PIP_WIN, &win);
	  xil_printf("   Start X    = %d\r\n", win.StartX);
	  xil_printf("   Start Y    = %d\r\n", win.StartY);
	  xil_printf("   Win Width  = %d\r\n", win.Width);
	  xil_printf("   Win Height = %d\r\n", win.Height);
	  xil_printf("\r\n   HStep Size = %d\r\n", XVprocSs_GetPipZoomWinHStepSize(InstancePtr));
    }
    else
    {
      xil_printf("\r\nPip  Mode: OFF\r\n");
    }

    count = 0;
    //print IP Data Flow Map
    xil_printf("\r\Data Flow Map: VidIn");
    while(count<InstancePtr->CtxtData.RtrNumCores)
    {
      xil_printf(" -> %s",XVprocSsIpStr[InstancePtr->CtxtData.RtngTable[count++]]);
    }
  }
  xil_printf("\r\n**************************************************\r\n\r\n");
}
