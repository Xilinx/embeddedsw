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
* @addtogroup vprocss_v1_0
* @{
*
* This is main code of Xilinx Video Processing Subsystem device driver.
* Please see xvprocss.h for more details of the driver.
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
  XAxis_Switch router;
  XGpio rstAxis;      //Reset for IP's running at AXIS Clk
  XGpio rstAximm;     //Reset for IP's with AXI MM interface

  XV_hcresampler hcrsmplr;
  XV_vcresampler vcrsmplrIn;
  XV_vcresampler vcrsmplrOut;
  XV_vscaler vscaler;
  XV_hscaler hscaler;
  XAxiVdma vdma;
  XV_letterbox lbox;
  XV_csc csc;
  XV_deinterlacer deint;
}XVprocss_SubCores;

/**************************** Local Global ***********************************/
XVprocss_SubCores subcoreRepo; /**< Define Driver instance of all sub-core
                                    included in the design */

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
const u16 XVprocss_PixelHStep[XVIDC_PPC_NUM_SUPPORTED][4] =
{
  {16,   4,  32,   8}, //XVIDC_PPC_1
  {16,   4,  64,  16}, //XVIDC_PPC_2
  {32, 128, 128,  32}, //XVIDC_PPC_4
};

/************************** Function Prototypes ******************************/
static void SetPODConfiguration(XVprocss *pVprocss);
static void GetIncludedSubcores(XVprocss *pVprocss);
static int ValidateSubsystemConfig(XVprocss *InstancePtr);
static int ValidateScalerOnlyConfig(XVidC_VideoStream *pStrmIn,
                                    XVidC_VideoStream *pStrmOut);
static int SetupModeScalerOnly(XVprocss *pVprocss);
static int SetupModeMax(XVprocss *pVprocss);

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
static __inline u32 XVprocss_GetResetState(XGpio *pReset, u32 channel)
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
static __inline void XVprocss_EnableBlock(XGpio *pReset, u32 channel, u32 ipBlock)
{
  u32 val;

  if(pReset)
  {
    val = XVprocss_GetResetState(pReset, channel);
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
static __inline void XVprocss_ResetBlock(XGpio *pReset, u32 channel, u32 ipBlock)
{
  u32 val;

  if(pReset)
  {
    val = XVprocss_GetResetState(pReset, channel);
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
* @param  pVprocss is a pointer to the subsystem instance
* @param  msec is delay required
*
* @return None
*
******************************************************************************/
static __inline void WaitUs(XVprocss *pVprocss, u32 MicroSeconds)
{
  if(MicroSeconds == 0)
	  return;

#if defined(__arm__)
  /* Wait the requested amount of time. */
  usleep(MicroSeconds);
#elif defined(__MICROBLAZE__)
  if(pVprocss->UsrDelayUs)
  {
    /* Use the time handler specified by the user for
     * better accuracy
     */
	 pVprocss->UsrDelayUs(pVprocss->pUsrTmr, MicroSeconds);
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
* This function reports list of cores included in Video Processing Subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance.
*
* @return None
*
******************************************************************************/
void XVprocss_ReportCoreInfo(XVprocss *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n  ->Video Processing Subsystem Cores\r\n");

  /* Report all the included cores in the subsystem instance */
  if(InstancePtr->hcrsmplr)
  {
    xil_printf("    : Horiz. Chroma Resampler \r\n");
  }

  if(InstancePtr->vcrsmplrIn)
  {
    xil_printf("    : Vert Chroma Resampler - Input\r\n");
  }

  if(InstancePtr->vcrsmplrOut)
  {
    xil_printf("    : Vert Chroma Resampler - Output\r\n");
  }

  if(InstancePtr->hscaler)
  {
    xil_printf("    : H Scaler\r\n");
  }

  if(InstancePtr->vscaler)
  {
    xil_printf("    : V Scaler\r\n");
  }

  if(InstancePtr->vdma)
  {
    xil_printf("    : VDMA\r\n");
  }

  if(InstancePtr->lbox)
  {
    xil_printf("    : LetterBox\r\n");
  }

  if(InstancePtr->csc)
  {
    xil_printf("    : Color Space Converter\r\n");
  }

  if(InstancePtr->deint)
  {
    xil_printf("    : Deinterlacer\r\n");
  }

  if(InstancePtr->rstAxis)
  {
    xil_printf("    : Reset (AXIS)\r\n");
  }

  if(InstancePtr->rstAximm)
  {
    xil_printf("    : Reset (AXI-MM) \r\n");
  }

  if(InstancePtr->router)
  {
    xil_printf("    : AXIS Router\r\n");
  }
}

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
void XVprocss_RegisterDelayHandler(XVprocss *InstancePtr,
                                   XVidC_DelayHandler CallbackFunc,
                                   void *CallbackRef)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(CallbackFunc != NULL);
  Xil_AssertVoid(CallbackRef != NULL);

  InstancePtr->UsrDelayUs = CallbackFunc;
  InstancePtr->pUsrTmr    = CallbackRef;
}

/*****************************************************************************/
/**
* This function queries the subsystem instance configuration to determine
* the included sub-cores. For each sub-core that is present in the design
* the sub-core driver instance is binded with the subsystem sub-core driver
* handle
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
static void GetIncludedSubcores(XVprocss *pVprocss)
{
  pVprocss->hcrsmplr    = ((pVprocss->Config.HCrsmplr.isPresent)    \
                              ? (&subcoreRepo.hcrsmplr)    : NULL);
  pVprocss->vcrsmplrIn  = ((pVprocss->Config.VCrsmplrIn.isPresent)  \
                              ? (&subcoreRepo.vcrsmplrIn)  : NULL);
  pVprocss->vcrsmplrOut = ((pVprocss->Config.VCrsmplrOut.isPresent) \
                              ? (&subcoreRepo.vcrsmplrOut) : NULL);
  pVprocss->vscaler     = ((pVprocss->Config.Vscale.isPresent)      \
                              ? (&subcoreRepo.vscaler)     : NULL);
  pVprocss->hscaler     = ((pVprocss->Config.Hscale.isPresent)      \
                              ? (&subcoreRepo.hscaler)     : NULL);
  pVprocss->vdma        = ((pVprocss->Config.Vdma.isPresent)        \
                              ? (&subcoreRepo.vdma)        : NULL);
  pVprocss->lbox        = ((pVprocss->Config.Lbox.isPresent)        \
                              ? (&subcoreRepo.lbox)        : NULL);
  pVprocss->csc         = ((pVprocss->Config.Csc.isPresent)         \
                              ? (&subcoreRepo.csc)         : NULL);
  pVprocss->deint       = ((pVprocss->Config.Deint.isPresent)       \
                              ? (&subcoreRepo.deint)       : NULL);
  pVprocss->router      = ((pVprocss->Config.Router.isPresent)      \
                              ? (&subcoreRepo.router)      : NULL);
  pVprocss->rstAxis     = ((pVprocss->Config.RstAxis.isPresent)     \
                              ? (&subcoreRepo.rstAxis)     : NULL);
  pVprocss->rstAximm    = ((pVprocss->Config.RstAximm.isPresent)    \
                              ? (&subcoreRepo.rstAximm)    : NULL);
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
void XVprocss_SetFrameBufBaseaddr(XVprocss *InstancePtr, u32 addr)
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
int XVprocss_CfgInitialize(XVprocss *InstancePtr, XVprocss_Config *CfgPtr,
                          u32 EffectiveAddr)
{
  XVprocss *pVprocss = InstancePtr;
  XAxiVdma_Config *VdmaCfgPtr;
  int status;
  u32 AbsAddr;

  /* Verify arguments */
  Xil_AssertNonvoid(pVprocss != NULL);
  Xil_AssertNonvoid(CfgPtr != NULL);
  Xil_AssertNonvoid(EffectiveAddr != (u32)NULL);

  /* Setup the instance */
  memcpy((void *)&(pVprocss->Config), (const void *)CfgPtr, sizeof(XVprocss_Config));
  pVprocss->Config.BaseAddress = EffectiveAddr;

  switch(pVprocss->Config.Topology)
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
  GetIncludedSubcores(pVprocss);

  /* Initialize all included sub_cores */
  if(pVprocss->rstAxis)
  {
	if(XVprocss_SubcoreInitResetAxis(pVprocss) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }

  if(pVprocss->rstAximm)
  {
	if(XVprocss_SubcoreInitResetAximm(pVprocss) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
    /*
     * Make sure AXI-MM interface is not in reset. If in reset it will prevent
     * Deinterlacer from being initialized
     */
    XVprocss_EnableBlock(InstancePtr->rstAximm, GPIO_CH_RESET_SEL, RESET_MASK_IP_AXIMM);
  }

  if(pVprocss->router)
  {
	if(XVprocss_SubcoreInitRouter(pVprocss) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }

  if(pVprocss->csc)
  {
	if(XVprocss_SubcoreInitCsc(pVprocss) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }

  if(pVprocss->hscaler)
  {
	if(XVprocss_SubcoreInitHScaler(pVprocss) != XST_SUCCESS)
    {
      return(XST_FAILURE);
    }
  }

  if(pVprocss->vscaler)
  {
	if(XVprocss_SubcoreInitVScaler(pVprocss) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}
  }

  if(pVprocss->hcrsmplr)
  {
	if(XVprocss_SubcoreInitHCrsmplr(pVprocss) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}
  }

  if(pVprocss->vcrsmplrIn)
  {
	if(XVprocss_SubcoreInitVCrsmpleIn(pVprocss) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}
  }

  if(pVprocss->vcrsmplrOut)
  {
	if(XVprocss_SubcoreInitVCrsmpleOut(pVprocss) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}
  }

  if(pVprocss->lbox)
  {
	if(XVprocss_SubcoreInitLetterbox(pVprocss) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}
  }

  if(pVprocss->vdma)
  {
	if(XVprocss_SubcoreInitVdma(pVprocss) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}

	if(pVprocss->FrameBufBaseaddr == 0)
	{
	  xil_printf("\r\nVPROCSS ERR:: Video Frame Buffer base address not set\r\n");
	  xil_printf("              Use XVprocss_SetFrameBufBaseaddr() API before subsystem init\r\n\r\n");
	  return(XST_FAILURE);
	}
  }


  if(pVprocss->deint)
  {
	if(XVprocss_SubcoreInitDeinterlacer(pVprocss) != XST_SUCCESS)
	{
	  return(XST_FAILURE);
	}

    /* Set Deinterlacer buffer offset in allocated DDR Frame Buffer */
    if(pVprocss->vdma) //vdma must be present for this to work
    {
      u32 vdmaBufReq, bufsize;
      u32 Bpp; //bytes per pixel

      Bpp = (pVprocss->Config.ColorDepth + 7)/8;

      //compute buffer size based on subsystem configuration
      //For 1 4K2K buffer (YUV444 16-bit) size is ~48MB
      bufsize = pVprocss->Config.MaxWidth *
		        pVprocss->Config.MaxHeight *
		        pVprocss->Config.NumVidComponents *
		        Bpp;

      //VDMA requires 4 buffers for total size of ~190MB
      vdmaBufReq = pVprocss->vdma->MaxNumFrames * bufsize;

      /*
       * vdmaBufReq = 0x0BDD 80000
       * padBuf     = 0x02F7 6000 (1 buffer is added as pad between vdma and deint)
       *             -------------
       * DeInt Offst= 0x0ED4 E000
       *             -------------
       */
      /* Set Deint Buffer Address Offset */
      pVprocss->idata.deintBufAddr = pVprocss->FrameBufBaseaddr + vdmaBufReq + bufsize;
    }
    else
    {
      xil_printf("\r\nVPROCSS ERR:: VDMA IP not found. Unable to assign De-interlacer buffer offsets\r\n");
      return(XST_FAILURE);
    }
  }

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the subsystem to power on default state. Subsystem
* device id is used to access the HW configuration for the instance.
* This configuration is then initialized along with any sub-cores included.
* As the last step the internal state of the subsystem is set to pre-defined
* power up default and Subsystem Ready state is set
*
* @param  InstancePtr is the pointer to subsystem instance
* @param  DeviceId is the unique device ID of the device for the lookup operation.
*
* @return XST_SUCCESS if successful
*         XST_DEVICE_NOT_FOUND if no device match is found.
*         XST_FAILURE otherwise
*
*******************************************************************************/
int XVprocss_PowerOnInit(XVprocss *InstancePtr, u32 DeviceId)
{
  XVprocss_Config *ConfigPtr;
  int status;

  Xil_AssertNonvoid(InstancePtr != NULL);

  ConfigPtr = XVprocss_LookupConfig(DeviceId);
  if(ConfigPtr == NULL)
  {
      InstancePtr->IsReady = 0;
      return (XST_DEVICE_NOT_FOUND);
  }

  //Initialize top level and all included sub-cores
  status = XVprocss_CfgInitialize(InstancePtr, ConfigPtr, ConfigPtr->BaseAddress);
  if(status != XST_SUCCESS)
  {
    xil_printf("ERR:: Video Processing Subsystem Initialization failed %d\r\n", status);
    return(XST_FAILURE);
  }

  /* All sub-cores static configuration extracted. Next configure subsystem to
   * power on default state
   */
  SetPODConfiguration(InstancePtr);

  /* Reset the hardware and set the flag to indicate the
     subsystem is ready
   */
  XVprocss_Reset(InstancePtr);
  InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function configures the Video Processing subsystem internal blocks
* to power on default configuration
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
static void SetPODConfiguration(XVprocss *pVprocss)
{
  XVidC_VideoStream vidStrmIn;
  XVidC_VideoStream vidStrmOut;
  XVidC_VideoWindow win;
  u16 PixPrecisionIndex;

  /* Setup Default Output Stream configuration */
  vidStrmOut.VmId          = XVIDC_VM_1920x1080_60_P;
  vidStrmOut.ColorFormatId = XVIDC_CSF_RGB;
  vidStrmOut.FrameRate     = XVIDC_FR_60HZ;
  vidStrmOut.IsInterlaced  = FALSE;
  vidStrmOut.ColorDepth    = pVprocss->Config.ColorDepth;
  vidStrmOut.PixPerClk     = pVprocss->Config.PixPerClock;

  /* Setup Default Input Stream configuration */
  vidStrmIn.VmId          = XVIDC_VM_1920x1080_60_P;
  vidStrmIn.ColorFormatId = XVIDC_CSF_RGB;
  vidStrmIn.FrameRate     = XVIDC_FR_60HZ;
  vidStrmIn.IsInterlaced  = FALSE;
  vidStrmIn.ColorDepth    = pVprocss->Config.ColorDepth;
  vidStrmIn.PixPerClk     = pVprocss->Config.PixPerClock;

  /* Setup Video Processing subsystem input/output  configuration */
  XVprocss_SetVidStreamIn(pVprocss,  &vidStrmIn);
  XVprocss_SetVidStreamOut(pVprocss, &vidStrmOut);

  /* compute data width supported by vdma */
  pVprocss->idata.PixelWidthInBits = pVprocss->Config.NumVidComponents *
			                         pVprocss->VidIn.ColorDepth;
  switch(pVprocss->Config.PixPerClock)
  {
    case XVIDC_PPC_1:
    case XVIDC_PPC_2:
	 if(pVprocss->Config.ColorDepth == XVIDC_BPC_10)
	 {
	   /* Align the bit width to next byte boundary for this particular case
	    * Num_Channel	Color Depth		PixelWidth		Align
	    * ----------------------------------------------------
	    *    2				10				20			 24
	    *    3				10				30			 32
	    *
	    *    HW will do the bit padding for 20->24 and 30->32
	    */
	   pVprocss->idata.PixelWidthInBits = ((pVprocss->idata.PixelWidthInBits + 7)/8)*8;
	 }
	 break;

    default:
	 break;
  }

  /* Setup Pixel Step size for define HW configuration */
  switch(pVprocss->Config.ColorDepth)
  {
    case XVIDC_BPC_8:  PixPrecisionIndex = 0; break;
    case XVIDC_BPC_10: PixPrecisionIndex = 1; break;
    case XVIDC_BPC_12: PixPrecisionIndex = 2; break;
    case XVIDC_BPC_16: PixPrecisionIndex = 3; break;
    default: break;
  }
  pVprocss->idata.PixelHStepSize = XVprocss_PixelHStep[pVprocss->Config.PixPerClock>>1][PixPrecisionIndex];

  if(XVprocss_IsConfigModeMax(pVprocss))
  {
    /* Set default Zoom Window */
    win.Width  = 400;
    win.Height = 400;
    win.StartX = win.StartY = 0;

    XVprocss_SetZoomPipWindow(pVprocss,
                              XVPROCSS_ZOOM_WIN,
                              &win);

    /* Set default PIP Window */
    XVprocss_SetZoomPipWindow(pVprocss,
                              XVPROCSS_PIP_WIN,
                              &win);
  }

  /* Release reset before programming any IP Block */
  XVprocss_EnableBlock(pVprocss->rstAxis,  GPIO_CH_RESET_SEL, RESET_MASK_ALL_BLOCKS);

  /* User parameter configuration */
  /* Set default background color for Letter Box */
  if(pVprocss->lbox)
  {
    XV_LboxSetBackgroundColor(pVprocss->lbox,
                              XLBOX_BKGND_BLACK,
                              pVprocss->VidOut.ColorFormatId,
                              pVprocss->VidOut.ColorDepth);
  }
  /* Initialize CSC sub-core layer 2 driver. This block has FW register map */
  if(pVprocss->csc)
  {
    XV_CscInitPowerOnDefault(&pVprocss->cscL2Reg);
    XV_CscSetColorDepth((&pVprocss->cscL2Reg), vidStrmIn.ColorDepth);
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
void XVprocss_Start(XVprocss *InstancePtr)
{
  u8 *pStartCore;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  pStartCore = &InstancePtr->idata.startCore[0];
  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Start Video Processing Subsystem.... \r\n");

  if(pStartCore[XVPROCSS_RTR_CR_V_OUT])
    XV_VCrsmplStart(InstancePtr->vcrsmplrOut);

  if(pStartCore[XVPROCSS_RTR_CR_H])
    XV_HCrsmplStart(InstancePtr->hcrsmplr);

  if(pStartCore[XVPROCSS_RTR_CSC])
    XV_CscStart(InstancePtr->csc);

  if(pStartCore[XVPROCSS_RTR_LBOX])
    XV_LBoxStart(InstancePtr->lbox);

  if(pStartCore[XVPROCSS_RTR_SCALER_H])
    XV_HScalerStart(InstancePtr->hscaler);

  if(pStartCore[XVPROCSS_RTR_SCALER_V])
    XV_VScalerStart(InstancePtr->vscaler);

  if(pStartCore[XVPROCSS_RTR_VDMA])
    XVprocss_VdmaStartTransfer(InstancePtr->vdma);

  if(pStartCore[XVPROCSS_RTR_DEINT])
    XV_DeintStart(InstancePtr->deint);

  if(pStartCore[XVPROCSS_RTR_CR_V_IN])
    XV_VCrsmplStart(InstancePtr->vcrsmplrIn);

  /* Subsystem ready to accept axis - Enable Video Input */
  XVprocss_EnableBlock(InstancePtr->rstAxis,  GPIO_CH_RESET_SEL, RESET_MASK_VIDEO_IN);
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
void XVprocss_Stop(XVprocss *InstancePtr)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Stop Video Processing Subsystem.... \r\n");

  if(InstancePtr->vcrsmplrIn)
    XV_VCrsmplStop(InstancePtr->vcrsmplrIn);

  if(InstancePtr->deint)
    XV_DeintStop(InstancePtr->deint);

  if(InstancePtr->vdma)
    XVprocss_VdmaStop(InstancePtr->vdma);

  if(InstancePtr->vscaler)
    XV_VScalerStop(InstancePtr->vscaler);

  if(InstancePtr->hscaler)
    XV_HScalerStop(InstancePtr->hscaler);

  if(InstancePtr->lbox)
    XV_LBoxStop(InstancePtr->lbox);

  if(InstancePtr->csc)
    XV_CscStop(InstancePtr->csc);

  if(InstancePtr->hcrsmplr)
    XV_HCrsmplStop(InstancePtr->hcrsmplr);

  if(InstancePtr->vcrsmplrOut)
    XV_VCrsmplStop(InstancePtr->vcrsmplrOut);
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
void XVprocss_Reset(XVprocss *InstancePtr)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  xdbg_printf(XDBG_DEBUG_GENERAL,"  ->Reset Video Processing Subsystem.... \r\n");

  /* Soft Reset */
  XVprocss_VdmaReset(InstancePtr->vdma);

  /* Reset All IP Blocks on AXIS interface*/
  XVprocss_ResetBlock(InstancePtr->rstAxis,  GPIO_CH_RESET_SEL, RESET_MASK_ALL_BLOCKS);
  /* Reset All IP Blocks on AXI-MM interface*/
//  XVprocss_ResetBlock(InstancePtr->rstAximm, GPIO_CH_RESET_SEL, RESET_MASK_IP_AXIMM);
  /* If enabled, Stop AXI-MM traffic */
   if((InstancePtr->deint) && (InstancePtr->idata.startCore[XVPROCSS_RTR_DEINT]))
   {
     XV_DeintStop(InstancePtr->deint);
   }

   WaitUs(InstancePtr, 100); /* hold reset line for 100us */
  /*
   * Make sure the video IP's are out of reset - IP's cannot be programmed when held
   * in reset. Will cause Axi-Lite bus to lock.
   * Release IP reset - but hold vid_in in reset
   */
//  XVprocss_EnableBlock(InstancePtr->rstAximm, GPIO_CH_RESET_SEL, RESET_MASK_IP_AXIMM);
//  Waitms(InstancePtr, 10); /* wait for AXI-MM IP's to stabilize */
  XVprocss_EnableBlock(InstancePtr->rstAxis,  GPIO_CH_RESET_SEL, RESET_MASK_IP_AXIS);
  WaitUs(InstancePtr, 1000); /* wait 1ms for AXIS to stabilize */

  /* Reset start core flags */
  memset(InstancePtr->idata.startCore, 0, sizeof(InstancePtr->idata.startCore));
}

/*****************************************************************************/
/**
* This function configures the video subsystem input interface
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  StrmIn is the pointer to input stream configuration
*
* @return XST_SUCESS if successful else XST_FAILURE
*
******************************************************************************/
int XVprocss_SetVidStreamIn(XVprocss *InstancePtr,
                            const XVidC_VideoStream *StrmIn)
{
  int status;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(StrmIn != NULL);

  /* Set Timing information based on resolution ID */
  status = XVprocss_SetStreamResolution(&InstancePtr->VidIn, StrmIn->VmId);

  if(!status)
  {
    InstancePtr->VidIn.ColorFormatId = StrmIn->ColorFormatId;
    InstancePtr->VidIn.ColorDepth    = StrmIn->ColorDepth;
    InstancePtr->VidIn.PixPerClk     = StrmIn->PixPerClk;
    InstancePtr->VidIn.FrameRate     = StrmIn->FrameRate;
    InstancePtr->VidIn.IsInterlaced  = StrmIn->IsInterlaced;
  }
  return(status);
}

/*****************************************************************************/
/**
* This function configures the video subsystem output interface
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  StrmOut is the pointer to input stream configuration
*
* @return XST_SUCESS if successful else XST_FAILURE
*
******************************************************************************/
int XVprocss_SetVidStreamOut(XVprocss *InstancePtr,
                             const XVidC_VideoStream *StrmOut)
{
  int status;

  /* Verify arguments */
  Xil_AssertNonvoid(InstancePtr != NULL);
  Xil_AssertNonvoid(StrmOut != NULL);

  /* Set Timing information based on resolution ID */
  status = XVprocss_SetStreamResolution(&InstancePtr->VidOut, StrmOut->VmId);

  if(!status)
  {
    InstancePtr->VidOut.ColorFormatId = StrmOut->ColorFormatId;
    InstancePtr->VidOut.ColorDepth    = StrmOut->ColorDepth;
    InstancePtr->VidOut.PixPerClk     = StrmOut->PixPerClk;
    InstancePtr->VidOut.FrameRate     = StrmOut->FrameRate;
    InstancePtr->VidOut.IsInterlaced  = StrmOut->IsInterlaced;
  }
  return(status);
}

/*****************************************************************************/
/**
* This function validates the video mode id against the supported resolutions
* and if successful extracts the timing information for the mode
*
* @param  StreamPtr is a pointer to the video stream to be configured
* @param  VmId is the Video Mode ID of the new resolution to be set

* @return XST_SUCESS if successful else XST_FAILURE
*
******************************************************************************/
int XVprocss_SetStreamResolution(XVidC_VideoStream *StreamPtr,
                                 const XVidC_VideoMode VmId)
{
  int status;

  /* Verify arguments */
  Xil_AssertNonvoid(StreamPtr != NULL);

  if(VmId < XVIDC_VM_NUM_SUPPORTED)
  {
    XVidC_VideoTiming const *pResTiming = XVidC_GetTimingInfo(VmId);

    StreamPtr->VmId = VmId;
    memcpy(&StreamPtr->Timing, pResTiming, sizeof(XVidC_VideoTiming));
    status = XST_SUCCESS;
  }
  else
  {
    xil_printf("\r\nVPROCSS ERR:: Stream Resolution Not Supported. Command Ignored\r\n");
    status = XST_FAILURE;
  }
  return(status);
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
void XVprocss_UpdateZoomPipWindow(XVprocss *InstancePtr)
{
  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  if(XVprocss_IsConfigModeMax(InstancePtr))
  {
    /* send vdma update window to IP */
    if(XVprocss_IsPipModeOn(InstancePtr))
    {
      XVprocss_VdmaSetWinToDnScaleMode(InstancePtr, XVPROCSS_VDMA_UPDATE_WR_CH);
    }
    else
    {
      XVprocss_VdmaSetWinToUpScaleMode(InstancePtr, XVPROCSS_VDMA_UPDATE_RD_CH);
    }

    XVprocss_VdmaStartTransfer(InstancePtr->vdma);

    /*
     * Final output of Video Processing subsystem goes via LBox IP
     * Program the output resolution window
     */
    if(XVprocss_IsPipModeOn(InstancePtr))
    {
      XV_LBoxSetActiveWin(InstancePtr->lbox,
                          &InstancePtr->idata.wrWindow,
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
void XVprocss_SetZoomPipWindow(XVprocss *InstancePtr,
                               XVprocss_Win   mode,
                               XVidC_VideoWindow *win)
{
  u16 wordlen;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(win != NULL);

  if(XVprocss_IsConfigModeMax(InstancePtr))
  {
    if(mode == XVPROCSS_ZOOM_WIN)
    {
     /* check if DMA includes DRE. If not then auto-align to selected bus width */
     if(!InstancePtr->vdma->ReadChannel.HasDRE)
     {
       wordlen = InstancePtr->idata.PixelHStepSize-1;

       win->StartX = ((win->StartX + wordlen) & ~(wordlen));
       win->Width  = ((win->Width  + wordlen) & ~(wordlen));
     }
     //VDMA RD Client
     InstancePtr->idata.rdWindow.StartX = win->StartX;
     InstancePtr->idata.rdWindow.StartY = win->StartY;
     InstancePtr->idata.rdWindow.Width  = win->Width;
     InstancePtr->idata.rdWindow.Height = win->Height;
    }
    else //PIP
    {
     /* check if DMA does not have DRE then auto-align */
     if(!InstancePtr->vdma->WriteChannel.HasDRE)
     {
       wordlen = InstancePtr->idata.PixelHStepSize-1;

       win->StartX = ((win->StartX + wordlen) & ~(wordlen));
       win->Width  = ((win->Width  + wordlen) & ~(wordlen));
     }
     //VDMA WR Client
     InstancePtr->idata.wrWindow.StartX = win->StartX;
     InstancePtr->idata.wrWindow.StartY = win->StartY;
     InstancePtr->idata.wrWindow.Width  = win->Width;
     InstancePtr->idata.wrWindow.Height = win->Height;
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
void XVprocss_GetZoomPipWindow(XVprocss *InstancePtr,
                               XVprocss_Win   mode,
                               XVidC_VideoWindow *win)
{
  /* Verify arguments */
   Xil_AssertVoid(InstancePtr != NULL);
   Xil_AssertVoid(win != NULL);

   if(XVprocss_IsConfigModeMax(InstancePtr))
   {
     if(mode == XVPROCSS_ZOOM_WIN)
     {
       win->StartX = InstancePtr->idata.rdWindow.StartX;
       win->StartY = InstancePtr->idata.rdWindow.StartY;
       win->Width  = InstancePtr->idata.rdWindow.Width;
       win->Height = InstancePtr->idata.rdWindow.Height;
     }
     else //PIP
     {
       win->StartX = InstancePtr->idata.wrWindow.StartX;
       win->StartY = InstancePtr->idata.wrWindow.StartY;
       win->Width  = InstancePtr->idata.wrWindow.Width;
       win->Height = InstancePtr->idata.wrWindow.Height;
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
* @note User must call XVprocss_ConfigureSubsystem() for change to take effect
*       This call has not been added here such that it provides an opportunity
*       to make the change during vertical blanking at system level. This
*       behavior will change once shadow register support is available in
*       sub-core IP's
*       This function is not applicable in Subsystem Stream Mode Configuration
*
******************************************************************************/
void XVprocss_SetZoomMode(XVprocss *InstancePtr, u8 OnOff)
{
  char *status[] = {"OFF","ON"};

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  if(XVprocss_IsConfigModeMax(InstancePtr))
  {
    InstancePtr->idata.ZoomEn = OnOff;
    InstancePtr->idata.PipEn  = FALSE;

    xdbg_printf(XDBG_DEBUG_GENERAL,"\r\n  :ZOOM Mode %s \r\n", status[InstancePtr->idata.ZoomEn]);
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
* @note User must call XVprocss_ConfigureSubsystem() for change to take effect
*       This call has not been added here such that it provides an opportunity
*       to make the change during vertical blanking at system level. This
*       behavior will change once shadow register support is available in
*       sub-core IP's
*       This function is not applicable in Subsystem Stream Mode Configuration
*
******************************************************************************/
void XVprocss_SetPipMode(XVprocss *InstancePtr, u8 OnOff)
{
  char *status[] = {"OFF","ON"};

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  if(XVprocss_IsConfigModeMax(InstancePtr))
  {
    InstancePtr->idata.PipEn  = OnOff;
    InstancePtr->idata.ZoomEn = FALSE;

    xdbg_printf(XDBG_DEBUG_GENERAL,"\r\n  :PIP Mode %s \r\n", status[InstancePtr->idata.PipEn]);
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
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
* @note If use case is possible the subsystem will configure the sub-cores
*       accordingly else will ignore the request
*
******************************************************************************/
static int SetupModeScalerOnly(XVprocss *pVprocss)
{
  u32 vsc_WidthIn, vsc_HeightIn, vsc_HeightOut;
  u32 hsc_HeightIn, hsc_WidthIn, hsc_WidthOut;
  int status = XST_SUCCESS;

  vsc_WidthIn = vsc_HeightIn = vsc_HeightOut = 0;
  hsc_HeightIn = hsc_WidthIn = hsc_WidthOut = 0;

  if((!pVprocss->vscaler) || (!pVprocss->hscaler))
  {
	xdbg_printf(XDBG_DEBUG_GENERAL,"VPROCSS ERR:: Scaler IP not found\r\n");
    return(XST_FAILURE);
  }

  /* check if input/output stream configuration is supported */
  status = ValidateScalerOnlyConfig(&pVprocss->VidIn,
                                    &pVprocss->VidOut);

  if(status ==  XST_SUCCESS)
  {
    /* Reset All IP Blocks */
    XVprocss_Reset(pVprocss);

    /* UpScale mode V Scaler is before H Scaler */
    vsc_WidthIn   = pVprocss->VidIn.Timing.HActive;
    vsc_HeightIn  = pVprocss->VidIn.Timing.VActive;
    vsc_HeightOut = pVprocss->VidOut.Timing.VActive;

    hsc_WidthIn  = vsc_WidthIn;
    hsc_HeightIn = vsc_HeightOut;
    hsc_WidthOut = pVprocss->VidOut.Timing.HActive;

    /* Configure scaler to scale input to output resolution */
    xdbg_printf(XDBG_DEBUG_GENERAL,"  -> Configure VScaler for %dx%d to %dx%d\r\n", \
            (int)vsc_WidthIn, (int)vsc_HeightIn, (int)vsc_WidthIn, (int)vsc_HeightOut);

    XV_VScalerSetup(pVprocss->vscaler,
		        &pVprocss->vscL2Reg,
                    vsc_WidthIn,
                    vsc_HeightIn,
                    vsc_HeightOut);

    xdbg_printf(XDBG_DEBUG_GENERAL,"  -> Configure HScaler for %dx%d to %dx%d\r\n", \
                       (int)hsc_WidthIn, (int)hsc_HeightIn, (int)hsc_WidthOut, (int)hsc_HeightIn);

    XV_HScalerSetup(pVprocss->hscaler,
		        &pVprocss->hscL2Reg,
                    hsc_HeightIn,
                    hsc_WidthIn,
                    hsc_WidthOut,
                    pVprocss->VidIn.ColorFormatId);

    /* Start Scaler sub-cores */
    XV_HScalerStart(pVprocss->hscaler);
    XV_VScalerStart(pVprocss->vscaler);

    /* Subsystem Ready to accept input stream - Enable Video Input */
    XVprocss_EnableBlock(pVprocss->rstAxis,  GPIO_CH_RESET_SEL, RESET_MASK_VIDEO_IN);
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
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
******************************************************************************/
static int SetupModeMax(XVprocss *pVprocss)
{
  int status;

  /* Build Routing table */
  status = XVprocss_BuildRoutingTable(pVprocss);

  if(status == XST_SUCCESS)
  {
    /* Reset All IP Blocks */
    XVprocss_Reset(pVprocss);

    /* Set AXI Switch registers */
    XVprocss_ProgRouterMux(pVprocss);

    /* program use case */
    XVprocss_SetupRouterDataFlow(pVprocss);
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
static int ValidateSubsystemConfig(XVprocss *InstancePtr)
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
	if(InstancePtr->vcrsmplrIn)
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
	if(InstancePtr->vcrsmplrOut)
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
	if(!InstancePtr->deint)
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
int XVprocss_SetSubsystemConfig(XVprocss *InstancePtr)
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

  switch(InstancePtr->Config.Topology)
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
* This function reports the subsystem HW and input/output stream configuration
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XVprocss_ReportSubsystemConfig(XVprocss *InstancePtr)
{
  char *topology[2] = {"Scaler-Only", "Full-Fledged"};
  XVidC_VideoWindow win;

  /* Verify arguments */
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n****** Video Processing Subsystem Configuration ******\r\n");
  /* Print the configuration selected */
  if(InstancePtr->Config.Topology <= XVPROCSS_TOPOLOGY_FULL_FLEDGED)
  {
    xil_printf("\r\nTopology: %s\r\n", topology[InstancePtr->Config.Topology]);
    XVprocss_ReportCoreInfo(InstancePtr);
  }
  else
  {
	xil_printf("VPROCSS ERR:: Unknown Topology\r\n");
	return;
  }
  xil_printf("\r\nPixels/Clk  = %d\r\n", InstancePtr->Config.PixPerClock);
  xil_printf("Color Depth = %d\r\n", InstancePtr->Config.ColorDepth);
  xil_printf("Num Video Components = %d\r\n", InstancePtr->Config.NumVidComponents);
  xil_printf("Max Width Supported  = %d\r\n", InstancePtr->Config.MaxWidth);
  xil_printf("Max Height Supported = %d\r\n", InstancePtr->Config.MaxHeight);

  xil_printf("\r\n------ SUBSYSTEM INPUT/OUTPUT CONFIG ------\r\n");
  xil_printf("->INPUT\r\n");
  XVidC_ReportStreamInfo(&InstancePtr->VidIn);
  xil_printf("\r\n->OUTPUT\r\n");
  XVidC_ReportStreamInfo(&InstancePtr->VidOut);

  if(XVprocss_IsConfigModeMax(InstancePtr))
  {
    if(XVprocss_IsZoomModeOn(InstancePtr))
    {
	  xil_printf("\r\nZoom Mode: ON\r\n");
	  XVprocss_GetZoomPipWindow(InstancePtr, XVPROCSS_ZOOM_WIN, &win);
	  xil_printf("   Start X    = %d\r\n", win.StartX);
	  xil_printf("   Start Y    = %d\r\n", win.StartY);
	  xil_printf("   Win Width  = %d\r\n", win.Width);
	  xil_printf("   Win Height = %d\r\n", win.Height);
    }
    else
    {
      xil_printf("\r\nZoom Mode: OFF\r\n");
    }

    if(XVprocss_IsPipModeOn(InstancePtr))
    {
	  xil_printf("\r\nPip Mode: ON\r\n");
	  XVprocss_GetZoomPipWindow(InstancePtr, XVPROCSS_PIP_WIN, &win);
	  xil_printf("   Start X    = %d\r\n", win.StartX);
	  xil_printf("   Start Y    = %d\r\n", win.StartY);
	  xil_printf("   Win Width  = %d\r\n", win.Width);
	  xil_printf("   Win Height = %d\r\n", win.Height);
    }
    else
    {
      xil_printf("\r\nPip  Mode: OFF\r\n");
    }
  }
  xil_printf("**************************************************\r\n\r\n");
}
/** @} */
