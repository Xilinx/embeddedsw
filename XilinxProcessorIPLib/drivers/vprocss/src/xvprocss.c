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
* 1.00  rc   05/01/15   Initial Release

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "microblaze_sleep.h"
#include "xvprocss.h"
#include "xenv.h"
#include "xvprocss_dma.h"
#include "xvprocss_router.h"

/************************** Constant Definitions *****************************/

/* HW Reset Network GPIO Channel */
#define GPIO_CH_RESET_SEL                 (1u)

/* VDMA transaction size in Bytes/Pixel */
#define XVPROC_VDMA_TRANS_SIZE_BYTES      (4)

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


/************************** Function Prototypes ******************************/
static void SetPODConfiguration(XVprocss *pVprocss);
static void GetIncludedSubcores(XVprocss *pVprocss);
static int validateVidStreamConfig(XVidC_VideoStream *pStrmIn,
                                   XVidC_VideoStream *pStrmOut);
static int SetupStreamMode(XVprocss *pVprocss);
static int SetupMaxMode(XVprocss *pVprocss);

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
* This function diverts the delay routine used in the subsystem. Preference is
* given to the user registered timer based routine. If no delay handler is
* registered then it uses the platform specific delay handler
*
* @param  pVprocss is a pointer to the subsystem instance
* @param  msec is delay required
*
* @return None
*
******************************************************************************/
static __inline void Waitms(XVprocss *pVprocss, u32 msec)
{
  if(pVprocss->UsrDelaymsec)
  {
	 pVprocss->UsrDelaymsec(pVprocss->pUsrTmr, msec);
  }
  else
  {
	MB_Sleep(msec);
  }
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
* @param  waitmsec is the function pointer to the user defined delay function
* @param  pTimer is the pointer to timer instance used by the delay function
*
* @return None
*
******************************************************************************/
void XVprocss_RegisterDelayHandler(XVprocss *InstancePtr,
                                   XVidC_DelayHandler waitmsec,
                                   void *pTimer)
{
  Xil_AssertVoid(InstancePtr != NULL);

  InstancePtr->UsrDelaymsec = waitmsec;
  InstancePtr->pUsrTmr      = pTimer;
}

/*****************************************************************************/
/**
* This function registers the System Interrupt controller with subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance
* @param  sysIntc is a pointer to the interrupt controller instance
*
* @return None
*
* @note Currently the subsystem and any of its sub-cores do not generate any
*       interrupts. Use of interrupt controller is reserved for future version
******************************************************************************/
void XVprocss_RegisterSysIntc(XVprocss *InstancePtr, XIntc *sysIntc)
{
  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(sysIntc != NULL);

  InstancePtr->pXintc = sysIntc;
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
  XAxis_Switch_Config *RouterCfgPtr;
  int status;

  /* Verify arguments */
  Xil_AssertNonvoid(pVprocss != NULL);
  Xil_AssertNonvoid(CfgPtr != NULL);
  Xil_AssertNonvoid(EffectiveAddr != (u32)NULL);
  Xil_AssertNonvoid(pVprocss->pXintc != NULL);

  /* Setup the instance */
  memcpy((void *)&(pVprocss->Config), (const void *)CfgPtr, sizeof(XVprocss_Config));
  pVprocss->Config.BaseAddress = EffectiveAddr;

  /* Print the configuration selected */
  switch(pVprocss->Config.Mode)
  {
    case XVPROCSS_MODE_MAX:
        xil_printf("    [Subsystem Configuration Mode - Full]\r\n\r\n");
        break;

    case XVPROCSS_MODE_STREAM:
        xil_printf("    [Subsystem Configuration Mode - Scaler-Only]\r\n\r\n");
        break;

    default:
        xil_printf("ERROR: Subsystem Configuration Mode Not Supported. \r\n");
        return(XST_FAILURE);
  }

  /* Determine sub-cores included in the provided instance of subsystem */
  GetIncludedSubcores(pVprocss);

  /* Initialize all included sub_cores */
  if(pVprocss->router)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing AXIS Switch core.... \r\n");
    RouterCfgPtr  = XAxisScr_LookupConfig(pVprocss->Config.Router.DeviceId);
    if(RouterCfgPtr == NULL)
    {
      xil_printf("VPROCSS ERR:: AXIS Switch device not found\r\n");
      return(XST_DEVICE_NOT_FOUND);
    }
    status = XAxisScr_CfgInitialize(pVprocss->router,
                                    RouterCfgPtr,
                                    RouterCfgPtr->BaseAddress);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: AXIS Switch Initialization failed %d\r\n", status);
      return(XST_FAILURE);
   }
  }

  if(pVprocss->rstAxis)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing AXIS Reset core.... \r\n");
    status  = XGpio_Initialize(pVprocss->rstAxis, pVprocss->Config.RstAxis.DeviceId);
    if(status == XST_DEVICE_NOT_FOUND)
    {
      xil_printf("VPROCSS ERR:: AXIS Clk Reset device not found\r\n");
      return(status);
    }
  }

  if(pVprocss->rstAximm)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing AXI-MM Reset core.... \r\n");
    status  = XGpio_Initialize(pVprocss->rstAximm, pVprocss->Config.RstAximm.DeviceId);
    if(status == XST_DEVICE_NOT_FOUND)
    {
      xil_printf("VPROCSS ERR:: AXIS Div2 Clk Reset device not found\r\n");
      return(status);
    }
    /*
     * Make sure AXI-MM interface is not in reset. If in reset it will prevent
     * Deinterlacer from being initialized
     */
    XVprocss_EnableBlock(InstancePtr->rstAximm, GPIO_CH_RESET_SEL, RESET_MASK_IP_AXIMM);
  }

  if(pVprocss->hcrsmplr)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing H Chroma Resampler core.... \r\n");
    status = XV_hcresampler_Initialize(pVprocss->hcrsmplr, pVprocss->Config.HCrsmplr.DeviceId);
    if(status == XST_DEVICE_NOT_FOUND)
    {
      xil_printf("VPROCSS ERR:: H Chroma Resampler device not found\r\n");
      return(status);
    }
  }

  if(pVprocss->vcrsmplrIn)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Input V Chroma Resampler core.... \r\n");
    status = XV_vcresampler_Initialize(pVprocss->vcrsmplrIn, pVprocss->Config.VCrsmplrIn.DeviceId);
    if(status == XST_DEVICE_NOT_FOUND)
    {
      xil_printf("VPROCSS ERR:: Input V Chroma Resampler device not found\r\n");
      return(status);
    }
  }

  if(pVprocss->vcrsmplrOut)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Output V Chroma Resampler core.... \r\n");
    status = XV_vcresampler_Initialize(pVprocss->vcrsmplrOut, pVprocss->Config.VCrsmplrOut.DeviceId);
    if(status == XST_DEVICE_NOT_FOUND)
    {
      xil_printf("VPROCSS ERR:: Output V Chroma Resampler device not found\r\n");
      return(status);
    }
  }

  if(pVprocss->vscaler)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing V Scaler core.... \r\n");
    status = XV_vscaler_Initialize(pVprocss->vscaler, pVprocss->Config.Vscale.DeviceId);
    if(status == XST_DEVICE_NOT_FOUND)
    {
      xil_printf("VPROCSS ERR:: V Scaler device not found\r\n");
      return(status);
    }
  }

  if(pVprocss->hscaler)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing H Scaler core.... \r\n");
    status = XV_hscaler_Initialize(pVprocss->hscaler, pVprocss->Config.Hscale.DeviceId);
    if(status == XST_DEVICE_NOT_FOUND)
    {
      xil_printf("VPROCSS ERR:: H Scaler device not found\r\n");
      return(status);
    }
  }

  if(pVprocss->vdma)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing VDMA core.... \r\n");
    VdmaCfgPtr = XAxiVdma_LookupConfig(pVprocss->Config.Vdma.DeviceId);
    if(VdmaCfgPtr == NULL)
    {
      xil_printf("VPROCSS ERR:: VDMA device not found\r\n");
      return(XST_DEVICE_NOT_FOUND);
    }
    status = XAxiVdma_CfgInitialize(pVprocss->vdma,
                                    VdmaCfgPtr,
                                    VdmaCfgPtr->BaseAddress);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: VDMA Configuration Initialization failed %d\r\n", status);
      return(XST_FAILURE);
    }
  }

  if(pVprocss->lbox)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Letterbox core.... \r\n");
    status = XV_letterbox_Initialize(pVprocss->lbox, pVprocss->Config.Lbox.DeviceId);
    if(status == XST_DEVICE_NOT_FOUND)
    {
      xil_printf("VPROCSS ERR:: LetterBox device not found\r\n");
      return(status);
    }
  }

  if(pVprocss->csc)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing CSC core.... \r\n");
    status = XV_csc_Initialize(pVprocss->csc, pVprocss->Config.Csc.DeviceId);
    if(status == XST_DEVICE_NOT_FOUND)
    {
      xil_printf("VPROCSS ERR:: CSC device not found\r\n");
      return(status);
    }
  }

  if(pVprocss->deint)
  {
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Deinterlacer core.... \r\n");
    status = XV_deinterlacer_Initialize(pVprocss->deint, pVprocss->Config.Deint.DeviceId);
    if(status == XST_DEVICE_NOT_FOUND)
    {
      xil_printf("VPROCSS ERR:: Deinterlacer device not found\r\n");
      return(status);
    }
  }

  /* Validate DDR Frame Buffer Allocation */
  if(pVprocss->vdma)
  {
    u32 vdmaBufReq, deintBufReq, padBuf;
    u32 totBufReq;
    u32 inRange, numVdmaBuf;

    /* Validate External Memory Range available */
    numVdmaBuf = pVprocss->vdma->MaxNumFrames;

    //4 4K2K (YUV444 16-bit) buffers (~190MB) needed for frame buffers
    vdmaBufReq   = numVdmaBuf * (u32)3840 * 2160 * 3 * 2;

    //3 1080i (YUV444 16-bit) field buffers (~18MB) needed for de-interlacer
    deintBufReq  = 3*(u32)1920*540*3*2;

    //1 4K2K buffer (~48MB) reserved as pad between vdma and deint
    padBuf       = (u32)3840*2160*3*2;

    totBufReq = vdmaBufReq + deintBufReq + padBuf; //~256MB (0x1000 0000)
    inRange = (totBufReq < pVprocss->Config.UsrExtMemAddr_Range);
    if(!inRange)
    {
      xil_printf("VPROCSS ERR:: EXT Memory Region Allocated is small.");
      xil_printf("Try increasing memory range\r\n");
      return(XST_FAILURE);
    }

    /*
     * DDR @0x8000 0000
     *  - 512MB space reserved for elf file
     * VDMA Buffer offset = 0x8000 0000 + 0x2000 0000
     *                    = 0xA000 0000
     * vdmaBufReq = 0x0BDD 80000
     * padBuf     = 0x02F7 6000
     *             -------------
     * DeInt Offst= 0x0ED4 E000  --->0xAED4 E0000
     *             -------------
     */
    /* Set Deint Buffer Address Offset */
    if(pVprocss->deint)
    {
      pVprocss->idata.deintBufAddr = pVprocss->Config.UsrExtMemBaseAddr + vdmaBufReq + padBuf;
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

  /* Setup Default Output Stream configuration */
  vidStrmOut.VmId          = XVIDC_VM_1920x1080_60_P;
  vidStrmOut.ColorFormatId = XVIDC_CSF_RGB;
  vidStrmOut.FrameRate     = XVIDC_FR_60HZ;
  vidStrmOut.IsInterlaced  = FALSE;
  vidStrmOut.ColorDepth    = XVIDC_BPC_10;
  vidStrmOut.PixPerClk     = pVprocss->Config.PixPerClock;

  /* Setup Default Input Stream configuration */
  vidStrmIn.VmId          = XVIDC_VM_1920x1080_60_P;
  vidStrmIn.ColorFormatId = XVIDC_CSF_RGB;
  vidStrmIn.FrameRate     = XVIDC_FR_60HZ;
  vidStrmIn.IsInterlaced  = FALSE;
  vidStrmIn.ColorDepth    = XVIDC_BPC_10;
  vidStrmIn.PixPerClk     = pVprocss->Config.PixPerClock;

  /* Setup Video Processing subsystem input/output  configuration */
  XVprocss_SetVidStreamIn(pVprocss,  &vidStrmIn);
  XVprocss_SetVidStreamOut(pVprocss, &vidStrmOut);

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

  pVprocss->idata.vdmaBytesPerPixel = XVPROC_VDMA_TRANS_SIZE_BYTES;

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

  Waitms(InstancePtr, 10); /* hold reset line for 10ms */
  /*
   * Make sure the video IP's are out of reset - IP's cannot be programmed when held
   * in reset. Will cause Axi-Lite bus to lock.
   * Release IP reset - but hold vid_in in reset
   */
//  XVprocss_EnableBlock(InstancePtr->rstAximm, GPIO_CH_RESET_SEL, RESET_MASK_IP_AXIMM);
//  Waitms(InstancePtr, 10); /* wait for AXI-MM IP's to stabilize */
  XVprocss_EnableBlock(InstancePtr->rstAxis,  GPIO_CH_RESET_SEL, RESET_MASK_IP_AXIS);
  Waitms(InstancePtr, 10); /* wait for AXIS to stabilize */

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
  Xil_AssertVoid(InstancePtr != NULL);

  if(XVprocss_IsConfigModeMax(InstancePtr))
  {
    /* send vdma update window to IP */
    if(XVprocss_IsPipModeOn(InstancePtr))
    {
      XVprocss_SetVdmaWinToDnScaleMode(InstancePtr, XVPROCSS_VDMA_UPDATE_WR_CH);
    }
    else
    {
      XVprocss_SetVdmaWinToUpScaleMode(InstancePtr, XVPROCSS_VDMA_UPDATE_RD_CH);
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
    xil_printf("\r\nVPROCSS ERR:: Feature not supported in Streaming Mode\r\n");
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

  Xil_AssertVoid(InstancePtr != NULL);
  Xil_AssertVoid(win != NULL);

  if(XVprocss_IsConfigModeMax(InstancePtr))
  {
    if(mode == XVPROCSS_ZOOM_WIN)
    {
     /* check if DMA includes DRE. If not then auto-align to selected bus width */
     if(!InstancePtr->vdma->ReadChannel.HasDRE)
     {
       wordlen = InstancePtr->vdma->ReadChannel.WordLength-1;

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
       wordlen = InstancePtr->vdma->WriteChannel.WordLength-1;

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
    xil_printf("\r\nVPROCSS ERR:: Feature not supported in Streaming Mode\r\n");
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
    xil_printf("\r\nVPROCSS ERR:: Feature not supported in Streaming Mode\r\n");
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

  Xil_AssertVoid(InstancePtr != NULL);

  if(XVprocss_IsConfigModeMax(InstancePtr))
  {
    InstancePtr->idata.ZoomEn = OnOff;
    InstancePtr->idata.PipEn  = FALSE;

    xil_printf("\r\n  :ZOOM Mode %s \r\n", status[InstancePtr->idata.ZoomEn]);
  }
  else //Scaler Only Config
  {
    xil_printf("\r\nVPROCSS ERR:: Feature not supported in Streaming Mode\r\n");
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

  Xil_AssertVoid(InstancePtr != NULL);

  if(XVprocss_IsConfigModeMax(InstancePtr))
  {
    InstancePtr->idata.PipEn  = OnOff;
    InstancePtr->idata.ZoomEn = FALSE;

    xil_printf("\r\n  :PIP Mode %s \r\n", status[InstancePtr->idata.PipEn]);
  }
  else //Scaler Only Config
  {
    xil_printf("\r\nVPROCSS ERR:: Feature not supported in Streaming Mode\r\n");
  }
}

/*****************************************************************************/
/**
* This function validates the input and output stream configuration
*
* @param  pStrmIn is a pointer to the input stream
* @param  pStrmOut is a pointer to the output stream
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
* @note This function is applicable only for Stream mode configuration of the
*       subsystem. In this mode very limited functionality is possible
******************************************************************************/
static int validateVidStreamConfig(XVidC_VideoStream *pStrmIn,
                                   XVidC_VideoStream *pStrmOut)
{
  if(pStrmIn->IsInterlaced)
  {
    xil_printf("VPROCSS ERR: Interlaced Input not supported\r\n");
    return(XST_FAILURE);
  }

  if(pStrmIn->ColorFormatId == XVIDC_CSF_YCRCB_420)
  {
    xil_printf("VPROCSS ERR: YUV420 Input not supported\r\n");
    return(XST_FAILURE);
  }

  if(pStrmIn->ColorFormatId != pStrmOut->ColorFormatId)
  {
    xil_printf("VPROCSS ERR: Input & Output Stream Color Format different\r\n");
    return(XST_FAILURE);
  }

  if(pStrmIn->ColorDepth != pStrmOut->ColorDepth)
  {
    xil_printf("VPROCSS ERR: Input & Output Color Depth different\r\n");
    return(XST_FAILURE);
  }

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function configures the video subsystem pipeline for Stream Mode
* configuration of the subsystem
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
* @note If use case is possible the subsystem will configure the sub-cores
*       accordingly else will ignore the request
*
******************************************************************************/
static int SetupStreamMode(XVprocss *pVprocss)
{
  u32 vsc_WidthIn, vsc_HeightIn, vsc_HeightOut;
  u32 hsc_HeightIn, hsc_WidthIn, hsc_WidthOut;
  int status = XST_SUCCESS;

  vsc_WidthIn = vsc_HeightIn = vsc_HeightOut = 0;
  hsc_HeightIn = hsc_WidthIn = hsc_WidthOut = 0;

  if((!pVprocss->vscaler) || (!pVprocss->hscaler))
  {
      xil_printf("VPROCSS ERR:: Scaler IP not found\r\n");
      return(XST_FAILURE);
  }

  /* check if input/output stream configuration is supported */
  status = validateVidStreamConfig(&pVprocss->VidIn,
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
    xil_printf("\r\n-->Command Ignored<--\r\n");
  }
  return(status);
}

/*****************************************************************************/
/**
* This function configures the video subsystem pipeline for Maximum (Full)
* mode configuration
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS if successful else XST_FAILURE
*
******************************************************************************/
static int SetupMaxMode(XVprocss *pVprocss)
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
    xil_printf("VPROCSS ERR: Subsystem Routing Table Invalid");
    xil_printf("- Ignoring Configuration Request\r\n");
  }
  return(status);
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
int XVprocss_ConfigureSubsystem(XVprocss *InstancePtr)
{
  int status = XST_SUCCESS;

  Xil_AssertNonvoid(InstancePtr != NULL);

  /* Video Processing Subsystem overrides In/Out Pixel Precision & Pixel/Clk */
  InstancePtr->VidIn.ColorDepth  = (XVidC_ColorDepth)InstancePtr->Config.PixPrecision;
  InstancePtr->VidOut.ColorDepth = (XVidC_ColorDepth)InstancePtr->Config.PixPrecision;

  InstancePtr->VidIn.PixPerClk  = (XVidC_PixelsPerClock)InstancePtr->Config.PixPerClock;

  xil_printf("\r\n****** VPROC SUBSYSTEM INPUT/OUTPUT CONFIG ******\r\n");
  xil_printf("->INPUT\r\n");
  XVidC_ReportStreamInfo(&InstancePtr->VidIn);
  xil_printf("\r\n->OUTPUT\r\n");
  XVidC_ReportStreamInfo(&InstancePtr->VidOut);
  xil_printf("**************************************************\r\n\r\n");

  switch(InstancePtr->Config.Mode)
  {
    case XVPROCSS_MODE_MAX:
        status = SetupMaxMode(InstancePtr);
        break;

    case XVPROCSS_MODE_STREAM:
        //Only configuration supported is V->H
        status = SetupStreamMode(InstancePtr);
        break;

    default:
        xil_printf("VPROCSS ERR: Subsystem Configuration Mode Not Supported. \r\n");
        status = XST_FAILURE;
        break;
  }
  return(status);
}
/** @} */
