/******************************************************************************
* Copyright (C) 2016 - 2020  Xilinx, Inc. All rights reserved.
* Copyright 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitxss.c
*
* This is main code of Xilinx HDMI Transmitter Subsystem device driver.
* Please see xv_hdmitxss.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00         10/07/15 Initial release.
* 1.01  MG     17/12/16 Fixed issue in function SetAudioChannels
*                       Updated function XV_HdmiTxSs_SendAuxInfoframe
* 1.02  yh     12/01/16 Check vtc existance before configuring it
* 1.03  yh     15/01/16 Add 3D Support
* 1.04  yh     20/01/16 Added remapper support
* 1.05  yh     01/02/16 Added set_ppc api
* 1.06  yh     01/02/16 Removed xil_print "Cable (dis)connected"
* 1.07  yh     15/02/16 Added default value to XV_HdmiTxSs_ConfigRemapper
* 1.08  MG     03/02/16 Added HDCP support
* 1.09  MG     09/03/16 Added XV_HdmiTxSs_SetHdmiMode and XV_HdmiTxSs_SetDviMode
*                       Removed reduced blanking support
* 1.10  MH     03/15/16 Moved HDCP 2.2 reset from stream up/down callback to
*                       connect callback
* 1.11  YH     18/03/16 Add XV_HdmiTxSs_SendGenericAuxInfoframe function
* 1.12  MH     23/04/16 1. HDCP 1.x driver now uses AXI timer 4.1, so updated
*                       to use AXI Timer config structure to determine timer
*                       clock frequency
*                       2. HDCP 1.x driver has fixed the problem where the
*                       reset for the receiver causes the entire DDC peripheral
*                       to get reset. Based on this change the driver has been
*                       updated to use XV_HdmiTxSs_HdcpReset and
*                       XV_HdmiTxSs_HdcpReset functions directly.
*                       3. Updated XV_HdmiTxSs_HdcpEnable and
*                       XV_HdmiTxSs_HdcpEnable functions to ensure that
*                       HDCP 1.4 and 2.2 are mutually exclusive.
*                       This fixes the problem where HDCP 1.4 and 2.2
*                       state machines are running simultaneously.
* 1.13   MH    23/06/16 Added HDCP repeater support.
* 1.14   YH    18/07/16 1. Replace xil_print with xdbg_printf.
*                       2. XV_HdmiTx_VSIF VSIF global variable local to
*                        XV_HdmiTxSs_SendVSInfoframe
*                       3. Replace MB_Sleep() with usleep
*                       4. Remove checking VideoMode < XVIDC_VM_NUM_SUPPORTED in
*                       XV_HdmiTxSs_SetStream to support customized video format
* 1.15   YH    25/07/16 Used UINTPTR instead of u32 for BaseAddress
*                       XV_HdmiTxSs_CfgInitialize
* 1.16   YH    04/08/16 Remove unused functions
*                       XV_HdmiTxSs_GetSubSysStruct
* 1.17   MH    08/08/16 Updates to optimize out HDCP when excluded.
* 1.18   YH    17/08/16 Remove sleep in XV_HdmiTxSs_ResetRemapper
*                       Added Event Log
*                       Combine Report function into one ReportInfo
* 1.19   YH    27/08/16 Remove unused functions XV_HdmiTxSs_SetUserTimerHandler
*                       XV_HdmiTxSs_WaitUs
* 1.20   MH    08/10/16 Update function call sequence in
*                       XV_HdmiTxSs_StreamUpCallback
*
* 1.1x   mmo   04/11/16 Updated the XV_HdmiTxSs_SetAudioChannels API which
*                       currently calls XV_HdmiTx_SetAudioChannels driver,
*                       which sets the Audio Channels
*                       accordingly. This fixed is made during v1.2 (2016.1)
* 1.21  YH     14/11/16 Added API to enable/disable YUV420/Pixel Repeat Mode
*                       for video bridge
* 1.22  YH     14/11/16 Remove Remapper APIs as remapper feature is moved to
*                       video bridge and controlled by HDMI core
* 1.23  mmo    03/01/17 Move HDCP Related API to xv_hdmitxss_hdcp.c
*                       Remove inclusion of the xenv.h and sleep.h as it not
*                           used
*                       Replaced "print" with "xil_printf"
*                       Replace Carriage Return (\r) and Line Feed (\n) order,\
*                           where the Carriage Return + Line Feed order is used.
* 1.24  mmo    02/03/17 Added XV_HdmiTxSs_ReadEdidSegment API for Multiple
*                             Segment Support and HDMI Compliance Test
*                       Updated the XV_HdmiTxSs_ShowEdid API to have support
*                             multiple EDID.
* 1.25  MH     21/04/17 Updated to set HDMI mode in functions
*                             XV_HdmiTxSS_SetHdmiMode and XV_HdmiTxSS_SetDviMode.
* 1.40  YH     07/07/17 Fixed issue with VTC register read when video clock is
*                             not present
*                       Report HDMI/DVI mode in HDMI example design info log
*                       Added Video Masking APIs
* 1.41  mmo    02/08/17 Initialize the hdcp1.4 first before the hdcp1.4 timer
*                              as the hdcp1.4 timer requires hdcp1.4 to be
*                              initialize
*       MH     09/08/17 Added function XV_HdmiTxSs_HdcpSetCapability
*              22/08/17 Added function XV_HdmiTxSs_SetAudioFormat
* 1.42  YH     06/10/17 Updated function XV_HdmiTxSs_SetAudioFormat
*                       Added function XV_HdmiTxSs_GetAudioFormat
*       EB     17/10/17 Added function XV_HdmiTxSs_ReportAudio
*                       Updated function XV_HdmiTxSs_ReportInfo
* 1.43  MMO    19/12/17 Added XV_HdmiTxSS_SetTMDS API
* 5.00  EB     16/01/18 Updated XV_HdmiTxSS_SetTMDS API
*                       Updated XV_HdmiTxSs_SetVideoStream API
*                       Moved XV_HdmiTxSs_SendVSInfoframe function to HDMI
*							    Common Library
*                       Updated function XV_HdmiTxSs_StreamStart,
*                               XV_HdmiTxSs_SendGenericAuxInfoframe
*                       Added function XV_HdmiTxSs_GetAviInfoframe,
*                           XV_HdmiTxSs_GetAudioInfoframe, XV_HdmiTxSs_GetVSIF
*                           XV_HdmiTxSs_GetAuxiliary
*                       Updated XV_HdmiTxSs_ConfigBridgeMode so Pixel
*                           Pepetition AVI InfoFrame is sent out
*                       Deprecating XV_HdmiTxSs_SendAviInfoframe and
*                           XV_HdmiTxSs_SendGeneralControlPacket APIs
*       YH     16/01/18 Added dedicated reset for each clock domain
*                       Added bridge unlock interrupt
*                       Added PIO_OUT to set GCP_AVMUTE
*       EB     23/01/18 Added function
*                           XV_HdmiTxSs_SetVideoStreamHdmi14ScramblingOverrideFlag
*              25/01/18 Added function XV_HdmiTxSs_SetScrambler
*              01/02/18	Updated function XV_HdmiTxSs_VtcSetup and changed the
*                           input parameters to it to enable logging of
*                           unsupported video timing by VTC
*       SM     28/02/18 Added definition of XV_HdmiTxSS_SetAppVersion() API
* 5.20  EB     03/08/18 Updated XV_HdmiTxSS_MaskSetRed, XV_HdmiTxSS_MaskSetGreen,
*                           XV_HdmiTxSS_MaskSetBlue API
*                       Replaced XV_HdmiTx_AudioMute API call with
*                           XV_HdmiTx_AudioDisable
*                       Replaced XV_HdmiTx_AudioUnmute API call with
*                           XV_HdmiTx_AudioEnable
*                       Replaced XV_HdmiTx_AudioUnmute API call with
* 		MMO    11/08/18 Added Bridge Overflow and Bridge Underflow Interrupt
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_hdmitxss.h"
#include "xv_hdmitxss_coreinit.h"

/************************** Constant Definitions *****************************/
/* Pixel definition in 8 bit resolution in YUV color space*/
static const u8 bkgndColorYUV[XV_BKGND_LAST][3] =
{
  {  0, 128, 128}, //Black
  {255, 128, 128}, //White
  { 76,  85, 255}, //Red
  {149,  43,  21}, //Green
  { 29, 255, 107}, //Blue
  {  0,   0,   0}  //Place holder for Noise Video Mask
};

/* Pixel map in RGB color space*/
/* {Green, Blue, Red} */
static const u8 bkgndColorRGB[XV_BKGND_LAST][3] =
{
  {0, 0, 0}, //Black
  {1, 1, 1}, //White
  {0, 0, 1}, //Red
  {1, 0, 0}, //Green
  {0, 1, 0}, //Blue
  {0, 0, 0}  //Place holder for Noise Video Mask
};

/**************************** Type Definitions *******************************/
/**
 * This typedef declares the driver instances of all the cores in the subsystem
 */
typedef struct
{
#ifdef XPAR_XHDCP_NUM_INSTANCES
  XTmrCtr HdcpTimer;
  XHdcp1x Hdcp14;
#endif
#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
  XHdcp22_Tx  Hdcp22;
#endif
  XV_HdmiTx HdmiTx;
  XVtc Vtc;
}XV_HdmiTxSs_SubCores;

/**************************** Local Global ***********************************/
#ifndef SDT
XV_HdmiTxSs_SubCores XV_HdmiTxSs_SubCoreRepo[XPAR_XV_HDMITXSS_NUM_INSTANCES];
                /**< Define Driver instance of all sub-core
                                    included in the design */

#else
XV_HdmiTxSs_SubCores XV_HdmiTxSs_SubCoreRepo[];
#endif
/************************** Function Prototypes ******************************/
#ifndef SDT
static void XV_HdmiTxSs_GetIncludedSubcores(XV_HdmiTxSs *HdmiTxSsPtr,
                                            u16 DevId);
#else
static void XV_HdmiTxSs_GetIncludedSubcores(XV_HdmiTxSs *HdmiTxSsPtr,
					    UINTPTR BaseAddress);
#endif
static int XV_HdmiTxSs_RegisterSubsysCallbacks(XV_HdmiTxSs *InstancePtr);
static int XV_HdmiTxSs_VtcSetup(XV_HdmiTxSs *HdmiTxSsPtr);
static u32 XV_HdmiTxSS_SetTMDS(XV_HdmiTxSs *InstancePtr,
                        XVidC_VideoMode VideoMode,
                        XVidC_ColorFormat ColorFormat,
                        XVidC_ColorDepth Bpc);
static void XV_HdmiTxSs_SendAviInfoframe(XV_HdmiTx *HdmiTxPtr);
static void XV_HdmiTxSs_SendVSInfoframe(XV_HdmiTx *HdmiTxPtr);
static void XV_HdmiTxSs_ConnectCallback(void *CallbackRef);
static void XV_HdmiTxSs_ToggleCallback(void *CallbackRef);
static void XV_HdmiTxSs_BrdgUnlockedCallback(void *CallbackRef);
static void XV_HdmiTxSs_BrdgOverflowCallback(void *CallbackRef);
static void XV_HdmiTxSs_BrdgUnderflowCallback(void *CallbackRef);
static void XV_HdmiTxSs_VsCallback(void *CallbackRef);
static void XV_HdmiTxSs_StreamUpCallback(void *CallbackRef);
static void XV_HdmiTxSs_StreamDownCallback(void *CallbackRef);
static u32 XV_HdmiTxSS_GetVidMaskColorValue(XV_HdmiTxSs *InstancePtr,
											u16 Value);

static void XV_HdmiTxSs_ReportCoreInfo(XV_HdmiTxSs *InstancePtr);
static void XV_HdmiTxSs_ReportTiming(XV_HdmiTxSs *InstancePtr);
static void XV_HdmiTxSs_ReportAudio(XV_HdmiTxSs *InstancePtr);
static void XV_HdmiTxSs_ReportSubcoreVersion(XV_HdmiTxSs *InstancePtr);

static void XV_HdmiTxSs_ConfigBridgeMode(XV_HdmiTxSs *InstancePtr);

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
* This macro selects the bridge YUV420 mode
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem
*
*****************************************************************************/
#define XV_HdmiTxSs_BridgeYuv420(InstancePtr, Enable) \
{ \
    XV_HdmiTx_Bridge_yuv420(InstancePtr->HdmiTxPtr, Enable); \
}

/*****************************************************************************/
/**
* This macro selects the bridge pixel repeat mode
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem
*
*****************************************************************************/
#define XV_HdmiTxSs_BridgePixelRepeat(InstancePtr, Enable) \
{ \
    XV_HdmiTx_Bridge_pixel(InstancePtr->HdmiTxPtr, Enable); \
}
/************************** Function Definition ******************************/

/*****************************************************************************/
/**
 * This function sets the core into HDMI mode
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem
 *
 *****************************************************************************/
void XV_HdmiTxSS_SetHdmiMode(XV_HdmiTxSs *InstancePtr)
{
    XV_HdmiTx_SetHdmiMode(InstancePtr->HdmiTxPtr);

#ifdef XPAR_XHDCP_NUM_INSTANCES
    if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetHdmiMode(InstancePtr->Hdcp14Ptr, TRUE);
    }
#endif
}

/*****************************************************************************/
/**
 * This function sets the core into DVI mode
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem
 *
 *****************************************************************************/
void XV_HdmiTxSS_SetDviMode(XV_HdmiTxSs *InstancePtr)
{
    XV_HdmiTx_SetDviMode(InstancePtr->HdmiTxPtr);

#ifdef XPAR_XHDCP_NUM_INSTANCES
    if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetHdmiMode(InstancePtr->Hdcp14Ptr, FALSE);
    }
#endif
}

/*****************************************************************************/
/**
* This function reports list of cores included in Video Processing Subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance.
*
* @return None
*
******************************************************************************/
static void XV_HdmiTxSs_ReportCoreInfo(XV_HdmiTxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  xil_printf("\r\n  ->HDMI TX Subsystem Cores\r\n");

  /* Report all the included cores in the subsystem instance */
  if (InstancePtr->HdmiTxPtr) {
    xil_printf("    : HDMI TX \r\n");
  }

  if (InstancePtr->VtcPtr) {
    xil_printf("    : VTC Core \r\n");
  }

#ifdef XPAR_XHDCP_NUM_INSTANCES
  if (InstancePtr->Hdcp14Ptr) {
    xil_printf("    : HDCP 1.4 TX \r\n");
  }

  if (InstancePtr->HdcpTimerPtr) {
    xil_printf("    : HDCP: AXIS Timer\r\n");
  }
#endif

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
  if (InstancePtr->Hdcp22Ptr) {
    xil_printf("    : HDCP 2.2 TX \r\n");
  }
#endif
}

/*****************************************************************************/
/**
 * This function calls the interrupt handler for HDMI TX
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem
 *
 *****************************************************************************/
void XV_HdmiTxSS_HdmiTxIntrHandler(XV_HdmiTxSs *InstancePtr)
{
    XV_HdmiTx_IntrHandler(InstancePtr->HdmiTxPtr);
}
/*****************************************************************************/
/**
 * This function register's all sub-core ISR's with interrupt controller and
 * any subsystem level call back function with requisite sub-core
 *
 * @param  InstancePtr is a pointer to the Subsystem instance to be
 *       worked on.
 *
 *****************************************************************************/
static int XV_HdmiTxSs_RegisterSubsysCallbacks(XV_HdmiTxSs *InstancePtr)
{
  XV_HdmiTxSs *HdmiTxSsPtr = InstancePtr;

  /** Register HDMI callbacks */
  if (HdmiTxSsPtr->HdmiTxPtr) {
    /*
     * Register call back for Tx Core Interrupts.
     */
    XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
                          XV_HDMITX_HANDLER_CONNECT,
						  (void *)XV_HdmiTxSs_ConnectCallback,
						  (void *)InstancePtr);

    XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
                          XV_HDMITX_HANDLER_TOGGLE,
						  (void *)XV_HdmiTxSs_ToggleCallback,
						  (void *)InstancePtr);

    XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
                          XV_HDMITX_HANDLER_BRDGUNLOCK,
						  (void *)XV_HdmiTxSs_BrdgUnlockedCallback,
						  (void *)InstancePtr);

    XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
                          XV_HDMITX_HANDLER_BRDGOVERFLOW,
						  (void *)XV_HdmiTxSs_BrdgOverflowCallback,
						  (void *)InstancePtr);

    XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
                          XV_HDMITX_HANDLER_BRDGUNDERFLOW,
						  (void *)XV_HdmiTxSs_BrdgUnderflowCallback,
						  (void *)InstancePtr);

    XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
                          XV_HDMITX_HANDLER_VS,
						  (void *)XV_HdmiTxSs_VsCallback,
						  (void *)InstancePtr);

    XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
                          XV_HDMITX_HANDLER_STREAM_UP,
						  (void *)XV_HdmiTxSs_StreamUpCallback,
						  (void *)InstancePtr);

    XV_HdmiTx_SetCallback(HdmiTxSsPtr->HdmiTxPtr,
                          XV_HDMITX_HANDLER_STREAM_DOWN,
						  (void *)XV_HdmiTxSs_StreamDownCallback,
						  (void *)InstancePtr);
  }

  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function queries the subsystem instance configuration to determine
* the included sub-cores. For each sub-core that is present in the design
* the sub-core driver instance is binded with the subsystem sub-core driver
* handle
*
* @param  HdmiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
#ifndef SDT
static void XV_HdmiTxSs_GetIncludedSubcores(XV_HdmiTxSs *HdmiTxSsPtr, u16 DevId)
{
  HdmiTxSsPtr->HdmiTxPtr     = ((HdmiTxSsPtr->Config.HdmiTx.IsPresent)    \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].HdmiTx) : NULL);
  HdmiTxSsPtr->VtcPtr        = ((HdmiTxSsPtr->Config.Vtc.IsPresent)  \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].Vtc) : NULL);
#ifdef XPAR_XHDCP_NUM_INSTANCES
  // HDCP 1.4
  HdmiTxSsPtr->Hdcp14Ptr       = ((HdmiTxSsPtr->Config.Hdcp14.IsPresent) \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].Hdcp14) : NULL);
  HdmiTxSsPtr->HdcpTimerPtr  = ((HdmiTxSsPtr->Config.HdcpTimer.IsPresent) \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].HdcpTimer) : NULL);
#endif
#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
  // HDCP 2.2
  HdmiTxSsPtr->Hdcp22Ptr       = ((HdmiTxSsPtr->Config.Hdcp22.IsPresent) \
                        ? (&XV_HdmiTxSs_SubCoreRepo[DevId].Hdcp22) : NULL);
#endif
}
#else
static void XV_HdmiTxSs_GetIncludedSubcores(XV_HdmiTxSs *HdmiTxSsPtr,
					     UINTPTR BaseAddress)
{
	u32 Index = 0;

	Index = XV_HdmiTxSs_GetDrvIndex(HdmiTxSsPtr, BaseAddress);
	HdmiTxSsPtr->HdmiTxPtr = ((HdmiTxSsPtr->Config.HdmiTx.IsPresent) \
		? (&XV_HdmiTxSs_SubCoreRepo[Index].HdmiTx) : NULL);
	HdmiTxSsPtr->VtcPtr = ((HdmiTxSsPtr->Config.Vtc.IsPresent)  \
		? (&XV_HdmiTxSs_SubCoreRepo[Index].Vtc) : NULL);
#ifdef XPAR_XHDCP_NUM_INSTANCES
	/* HDCP 1.4*/
	HdmiTxSsPtr->Hdcp14Ptr = ((HdmiTxSsPtr->Config.Hdcp14.IsPresent) \
		? (&XV_HdmiTxSs_SubCoreRepo[Index].Hdcp14) : NULL);
	HdmiTxSsPtr->HdcpTimerPtr = ((HdmiTxSsPtr->Config.HdcpTimer.IsPresent) \
		? (&XV_HdmiTxSs_SubCoreRepo[Index].HdcpTimer) : NULL);
#endif
#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
	/* HDCP 2.2*/
	HdmiTxSsPtr->Hdcp22Ptr = ((HdmiTxSsPtr->Config.Hdcp22.IsPresent) \
		? (&XV_HdmiTxSs_SubCoreRepo[Index].Hdcp22) : NULL);
#endif
}
#endif
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
int XV_HdmiTxSs_CfgInitialize(XV_HdmiTxSs *InstancePtr,
                              XV_HdmiTxSs_Config *CfgPtr,
                              UINTPTR EffectiveAddr)
{
  XV_HdmiTxSs *HdmiTxSsPtr = InstancePtr;

  /* Verify arguments */
  Xil_AssertNonvoid(HdmiTxSsPtr != NULL);
  Xil_AssertNonvoid(CfgPtr != NULL);
  Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)NULL);

  /* Setup the instance */
  memcpy((void *)&(HdmiTxSsPtr->Config), (const void *)CfgPtr,
    sizeof(XV_HdmiTxSs_Config));
  HdmiTxSsPtr->Config.BaseAddress = EffectiveAddr;

  /* Initialize InfoFrame */
  (void)memset((void *)&(HdmiTxSsPtr->AVIInfoframe), 0, sizeof(XHdmiC_AVI_InfoFrame));
  (void)memset((void *)&(HdmiTxSsPtr->VSIF), 0, sizeof(XHdmiC_VSIF));
  (void)memset((void *)&(HdmiTxSsPtr->AudioInfoframe), 0, sizeof(XHdmiC_AudioInfoFrame));
  memset((void *)&(HdmiTxSsPtr->DrmInfoframe), 0, sizeof(XHdmiC_DRMInfoFrame));
  HdmiTxSsPtr->DrmInfoframe.Static_Metadata_Descriptor_ID = 0xff;
  HdmiTxSsPtr->DrmInfoframe.EOTF = 0xff;

  /* Determine sub-cores included in the provided instance of subsystem */
#ifndef SDT
  XV_HdmiTxSs_GetIncludedSubcores(HdmiTxSsPtr, CfgPtr->DeviceId);
#else
  XV_HdmiTxSs_GetIncludedSubcores(HdmiTxSsPtr, CfgPtr->BaseAddress);
#endif

  /* Initialize all included sub_cores */

  // HDCP 1.4
#ifdef XPAR_XHDCP_NUM_INSTANCES
  if (HdmiTxSsPtr->Hdcp14Ptr) {
    if (XV_HdmiTxSs_SubcoreInitHdcp14(HdmiTxSsPtr) != XST_SUCCESS){
      return(XST_FAILURE);
    }
  }

  if (HdmiTxSsPtr->HdcpTimerPtr) {
    if (XV_HdmiTxSs_SubcoreInitHdcpTimer(HdmiTxSsPtr) != XST_SUCCESS){
      return(XST_FAILURE);
    }
  }
#endif

  if (HdmiTxSsPtr->HdmiTxPtr) {
    if (XV_HdmiTxSs_SubcoreInitHdmiTx(HdmiTxSsPtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
    XV_HdmiTx_SetAxiClkFreq(HdmiTxSsPtr->HdmiTxPtr,
                            HdmiTxSsPtr->Config.AxiLiteClkFreq);
  }

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
  // HDCP 2.2
  if (HdmiTxSsPtr->Hdcp22Ptr) {
    if (XV_HdmiTxSs_SubcoreInitHdcp22(HdmiTxSsPtr) != XST_SUCCESS){
      return(XST_FAILURE);
    }
  }
#endif

  if (HdmiTxSsPtr->VtcPtr) {
    if (XV_HdmiTxSs_SubcoreInitVtc(HdmiTxSsPtr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  /* Register Callbacks */
  XV_HdmiTxSs_RegisterSubsysCallbacks(HdmiTxSsPtr);

  /* Set default HDCP protocol */
  HdmiTxSsPtr->HdcpProtocol = XV_HDMITXSS_HDCP_NONE;

  /* HDCP ready flag */

#ifdef USE_HDCP_TX
  /* Default value */
  HdmiTxSsPtr->HdcpIsReady = (FALSE);
  XV_HdmiTxSs_HdcpSetCapability(HdmiTxSsPtr, XV_HDMITXSS_HDCP_BOTH);
#endif

#if defined(XPAR_XHDCP_NUM_INSTANCES) && defined(XPAR_XHDCP22_TX_NUM_INSTANCES)
  /* HDCP is ready when both HDCP cores are instantiated and both keys
     are loaded */
  if (HdmiTxSsPtr->Hdcp14Ptr && HdmiTxSsPtr->Hdcp22Ptr &&
      HdmiTxSsPtr->Hdcp22Lc128Ptr && HdmiTxSsPtr->Hdcp22SrmPtr &&
      HdmiTxSsPtr->Hdcp14KeyPtr) {
    HdmiTxSsPtr->HdcpIsReady = (TRUE);
  }
#endif

#if defined(XPAR_XHDCP_NUM_INSTANCES)
  /* HDCP is ready when only the HDCP 1.4 core is instantiated and the key
     is loaded */
  if (!HdmiTxSsPtr->HdcpIsReady && HdmiTxSsPtr->Hdcp14Ptr &&
       HdmiTxSsPtr->Hdcp14KeyPtr) {
    HdmiTxSsPtr->HdcpIsReady = (TRUE);
  }
#endif

#if defined(XPAR_XHDCP22_TX_NUM_INSTANCES)
  /* HDCP is ready when only the HDCP 2.2 core is instantiated and the key
     is loaded */
  if (!HdmiTxSsPtr->HdcpIsReady && HdmiTxSsPtr->Hdcp22Ptr &&
       HdmiTxSsPtr->Hdcp22Lc128Ptr &&
      HdmiTxSsPtr->Hdcp22SrmPtr) {
    HdmiTxSsPtr->HdcpIsReady = (TRUE);
  }
#endif

  /* Set the flag to indicate the subsystem is ready */
  XV_HdmiTxSs_Reset(HdmiTxSsPtr);
  HdmiTxSsPtr->IsReady = XIL_COMPONENT_IS_READY;

  /* Initialize the application version with 0 <default value>.
   * Application need to set the this variable properly to let driver know
   * what version of application is being used.
   */
  HdmiTxSsPtr->AppMajVer = 0;
  HdmiTxSsPtr->AppMinVer = 0;

  return(XST_SUCCESS);
}

/****************************************************************************/
/**
* This function starts the HDMI TX subsystem including all sub-cores that are
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
void XV_HdmiTxSs_Start(XV_HdmiTxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);
#ifdef XV_HDMITXSS_LOG_ENABLE
  XV_HdmiTxSs_LogWrite(InstancePtr, XV_HDMITXSS_LOG_EVT_START, 0);
#endif
}

/*****************************************************************************/
/**
* This function stops the HDMI TX subsystem including all sub-cores
* Stop the video pipe starting from front to back
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_Stop(XV_HdmiTxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);
#ifdef XV_HDMITXSS_LOG_ENABLE
  XV_HdmiTxSs_LogWrite(InstancePtr, XV_HDMITXSS_LOG_EVT_STOP, 0);
#endif
  if (InstancePtr->VtcPtr) {
    /* Disable VTC */
    XVtc_DisableGenerator(InstancePtr->VtcPtr);
  }
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
void XV_HdmiTxSs_Reset(XV_HdmiTxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);
#ifdef XV_HDMITXSS_LOG_ENABLE
  XV_HdmiTxSs_LogWrite(InstancePtr, XV_HDMITXSS_LOG_EVT_RESET, 0);
#endif
  /* Assert HDMI TXCore resets */
  XV_HdmiTxSs_TXCore_LRST(InstancePtr, TRUE);
  XV_HdmiTxSs_TXCore_VRST(InstancePtr, TRUE);

  /* Assert VID_OUT bridge resets */
  XV_HdmiTxSs_SYSRST(InstancePtr, TRUE);
  XV_HdmiTxSs_VRST(InstancePtr, TRUE);

  /* Release VID_IN bridge resets */
  XV_HdmiTxSs_SYSRST(InstancePtr, FALSE);
  XV_HdmiTxSs_VRST(InstancePtr, FALSE);

  /* Release HDMI TXCore resets */
  XV_HdmiTxSs_TXCore_LRST(InstancePtr, FALSE);
  XV_HdmiTxSs_TXCore_VRST(InstancePtr, FALSE);
}

/*****************************************************************************/
/**
* This function asserts or releases the Internal Video reset
* of the HDMI subcore within the subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_TXCore_VRST(XV_HdmiTxSs *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx_INT_VRST(InstancePtr->HdmiTxPtr, Reset);
}

/*****************************************************************************/
/**
* This function asserts or releases the Internal Link reset
* of the HDMI subcore within the subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_TXCore_LRST(XV_HdmiTxSs *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx_INT_LRST(InstancePtr->HdmiTxPtr, Reset);
}

/*****************************************************************************/
/**
* This function asserts or releases the video reset of other
* blocks within the subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_VRST(XV_HdmiTxSs *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx_EXT_VRST(InstancePtr->HdmiTxPtr, Reset);
}

/*****************************************************************************/
/**
* This function asserts or releases the system reset of other
* blocks within the subsystem
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_SYSRST(XV_HdmiTxSs *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx_EXT_SYSRST(InstancePtr->HdmiTxPtr, Reset);
}

/*****************************************************************************/
/**
* This function sets the HDMI TX AUX GCP register AVMUTE bit.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_SetGcpAvmuteBit(XV_HdmiTxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx_SetGcpAvmuteBit(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
* This function clears the HDMI TX AUX GCP register AVMUTE bit.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_ClearGcpAvmuteBit(XV_HdmiTxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx_ClearGcpAvmuteBit(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
* This function sets the HDMI TX AUX GCP register CLEAR_AVMUTE bit.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_SetGcpClearAvmuteBit(XV_HdmiTxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx_SetGcpClearAvmuteBit(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
* This function clears the HDMI TX AUX GCP register CLEAR_AVMUTE bit.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSs_ClearGcpClearAvmuteBit(XV_HdmiTxSs *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx_ClearGcpClearAvmuteBit(InstancePtr->HdmiTxPtr);
}
/*****************************************************************************/
/**
*
* This function configures Video Timing Controller (VTC).
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static int XV_HdmiTxSs_VtcSetup(XV_HdmiTxSs *HdmiTxSsPtr)
{
  /* Polarity configuration */
  XVtc_Polarity Polarity;
  XVtc_SourceSelect SourceSelect;
  XVtc_Timing VideoTiming;
  u32 HdmiTx_Hblank;
  u32 Vtc_Hblank;

  /* Disable Generator */
  XVtc_Reset(HdmiTxSsPtr->VtcPtr);
  XVtc_DisableGenerator(HdmiTxSsPtr->VtcPtr);
  XVtc_Disable(HdmiTxSsPtr->VtcPtr);

  /* Set up source select */
  memset((void *)&SourceSelect, 0, sizeof(SourceSelect));

  /* 1 = Generator registers, 0 = Detector registers */
  SourceSelect.VChromaSrc = 1;
  SourceSelect.VActiveSrc = 1;
  SourceSelect.VBackPorchSrc = 1;
  SourceSelect.VSyncSrc = 1;
  SourceSelect.VFrontPorchSrc = 1;
  SourceSelect.VTotalSrc = 1;
  SourceSelect.HActiveSrc = 1;
  SourceSelect.HBackPorchSrc = 1;
  SourceSelect.HSyncSrc = 1;
  SourceSelect.HFrontPorchSrc = 1;
  SourceSelect.HTotalSrc = 1;

  XVtc_SetSource(HdmiTxSsPtr->VtcPtr, &SourceSelect);

  VideoTiming.HActiveVideo = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.HActive;
  VideoTiming.HFrontPorch = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.HFrontPorch;
  VideoTiming.HSyncWidth = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.HSyncWidth;
  VideoTiming.HBackPorch = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.HBackPorch;
  VideoTiming.HSyncPolarity = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.HSyncPolarity;

  /* Vertical Timing */
  VideoTiming.VActiveVideo = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.VActive;

  VideoTiming.V0FrontPorch = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.F0PVFrontPorch;
  VideoTiming.V0BackPorch = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.F0PVBackPorch;
  VideoTiming.V0SyncWidth = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.F0PVSyncWidth;

  VideoTiming.V1FrontPorch = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.F1VFrontPorch;
  VideoTiming.V1SyncWidth = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.F1VSyncWidth;
  VideoTiming.V1BackPorch = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.F1VBackPorch;

  VideoTiming.VSyncPolarity = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.VSyncPolarity;

  VideoTiming.Interlaced = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.IsInterlaced;

    /* 4 pixels per clock */
    if (HdmiTxSsPtr->HdmiTxPtr->Stream.Video.PixPerClk == XVIDC_PPC_4) {
    	/* If the parameters below are not divisible by the current PPC setting,
    	 * log an error as VTC does not support such video timing
    	 */
		if (VideoTiming.HActiveVideo & 0x3 || VideoTiming.HFrontPorch & 0x3 ||
				VideoTiming.HBackPorch & 0x3 || VideoTiming.HSyncWidth & 0x3) {
#ifdef XV_HDMITXSS_LOG_ENABLE
				XV_HdmiTxSs_LogWrite(HdmiTxSsPtr,
						XV_HDMITXSS_LOG_EVT_VTC_RES_ERR, 0);
#endif
		}
		VideoTiming.HActiveVideo = VideoTiming.HActiveVideo/4;
		VideoTiming.HFrontPorch = VideoTiming.HFrontPorch/4;
		VideoTiming.HBackPorch = VideoTiming.HBackPorch/4;
		VideoTiming.HSyncWidth = VideoTiming.HSyncWidth/4;
    }

    /* 2 pixels per clock */
    else if (HdmiTxSsPtr->HdmiTxPtr->Stream.Video.PixPerClk == XVIDC_PPC_2) {
    	/* If the parameters below are not divisible by the current PPC setting,
    	 * log an error as VTC does not support such video timing
    	 */
		if (VideoTiming.HActiveVideo & 0x1 || VideoTiming.HFrontPorch & 0x1 ||
				VideoTiming.HBackPorch & 0x1 || VideoTiming.HSyncWidth & 0x1) {
#ifdef XV_HDMITXSS_LOG_ENABLE
			XV_HdmiTxSs_LogWrite(HdmiTxSsPtr,
					XV_HDMITXSS_LOG_EVT_VTC_RES_ERR, 0);
#endif
		}
		VideoTiming.HActiveVideo = VideoTiming.HActiveVideo/2;
		VideoTiming.HFrontPorch = VideoTiming.HFrontPorch/2;
		VideoTiming.HBackPorch = VideoTiming.HBackPorch/2;
		VideoTiming.HSyncWidth = VideoTiming.HSyncWidth/2;
    }

    /* 1 pixels per clock */
    else {
		VideoTiming.HActiveVideo = VideoTiming.HActiveVideo;
		VideoTiming.HFrontPorch = VideoTiming.HFrontPorch;
		VideoTiming.HBackPorch = VideoTiming.HBackPorch;
		VideoTiming.HSyncWidth = VideoTiming.HSyncWidth;
    }

    /* For YUV420 the line width is double there for double the blanking */
    if (HdmiTxSsPtr->HdmiTxPtr->Stream.Video.ColorFormatId == XVIDC_CSF_YCRCB_420) {
    	/* If the parameters below are not divisible by the current PPC setting,
    	 * log an error as VTC does not support such video timing
    	 */
		if (VideoTiming.HActiveVideo & 0x1 || VideoTiming.HFrontPorch & 0x1 ||
				VideoTiming.HBackPorch & 0x1 || VideoTiming.HSyncWidth & 0x1) {
#ifdef XV_HDMITXSS_LOG_ENABLE
			XV_HdmiTxSs_LogWrite(HdmiTxSsPtr,
					XV_HDMITXSS_LOG_EVT_VTC_RES_ERR, 0);
#endif
		}
		VideoTiming.HActiveVideo = VideoTiming.HActiveVideo/2;
		VideoTiming.HFrontPorch = VideoTiming.HFrontPorch/2;
		VideoTiming.HBackPorch = VideoTiming.HBackPorch/2;
		VideoTiming.HSyncWidth = VideoTiming.HSyncWidth/2;
    }

/** When compensating the vtc horizontal timing parameters for the pixel mode
* (quad or dual) rounding errors might be introduced (due to the divide)
* If this happens, the vtc total horizontal blanking is less than the hdmi tx
* horizontal blanking.
* As a result the hdmi tx vid out bridge is not able to lock to
* the incoming video stream.
* This process will check the horizontal blank timing and compensate
* for this condition.
* Calculate hdmi tx horizontal blanking */

  HdmiTx_Hblank = HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.HFrontPorch +
    HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.HSyncWidth +
    HdmiTxSsPtr->HdmiTxPtr->Stream.Video.Timing.HBackPorch;

  do {
    // Calculate vtc horizontal blanking
    Vtc_Hblank = VideoTiming.HFrontPorch +
        VideoTiming.HBackPorch +
        VideoTiming.HSyncWidth;

    // Quad pixel mode
    if (HdmiTxSsPtr->HdmiTxPtr->Stream.Video.PixPerClk == XVIDC_PPC_4) {
      Vtc_Hblank *= 4;
    }

    // Dual pixel mode
    else if (HdmiTxSsPtr->HdmiTxPtr->Stream.Video.PixPerClk == XVIDC_PPC_2) {
      Vtc_Hblank *= 2;
    }

    // Single pixel mode
    else {
      //Vtc_Hblank *= 1;
    }

    /* For YUV420 the line width is double there for double the blanking */
    if (HdmiTxSsPtr->HdmiTxPtr->Stream.Video.ColorFormatId == XVIDC_CSF_YCRCB_420) {
        Vtc_Hblank *= 2;
    }

    // If the horizontal total blanking differs,
    // then increment the Vtc horizontal front porch.
    if (Vtc_Hblank != HdmiTx_Hblank) {
      VideoTiming.HFrontPorch++;
    }

  } while (Vtc_Hblank < HdmiTx_Hblank);

  if (Vtc_Hblank != HdmiTx_Hblank) {
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "Error! Current format with total Hblank (%d) cannot \r\n",
                  HdmiTx_Hblank);
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "       be transmitted with pixels per clock = %d\r\n",
                  HdmiTxSsPtr->HdmiTxPtr->Stream.Video.PixPerClk);
      return (XST_FAILURE);
  }

  XVtc_SetGeneratorTiming(HdmiTxSsPtr->VtcPtr, &VideoTiming);

  /* Set up Polarity of all outputs */
  memset((void *)&Polarity, 0, sizeof(XVtc_Polarity));
  Polarity.ActiveChromaPol = 1;
  Polarity.ActiveVideoPol = 1;

  //Polarity.FieldIdPol = 0;
  if (VideoTiming.Interlaced) {
    Polarity.FieldIdPol = 1;
  }
  else {
    Polarity.FieldIdPol = 0;
  }

  Polarity.VBlankPol = VideoTiming.VSyncPolarity;
  Polarity.VSyncPol = VideoTiming.VSyncPolarity;
  Polarity.HBlankPol = VideoTiming.HSyncPolarity;
  Polarity.HSyncPol = VideoTiming.HSyncPolarity;

  XVtc_SetPolarity(HdmiTxSsPtr->VtcPtr, &Polarity);

  /* VTC driver does not take care of the setting of the VTC in
   * interlaced operation. As a work around the register
   * is set manually */
  if (VideoTiming.Interlaced) {
    /* Interlaced mode */
    XVtc_WriteReg(HdmiTxSsPtr->VtcPtr->Config.BaseAddress, 0x68, 0x42);
  }
  else {
    /* Progressive mode */
    XVtc_WriteReg(HdmiTxSsPtr->VtcPtr->Config.BaseAddress, 0x68, 0x2);
  }

  /* Enable generator module */
  XVtc_Enable(HdmiTxSsPtr->VtcPtr);
  XVtc_EnableGenerator(HdmiTxSsPtr->VtcPtr);
  XVtc_RegUpdateEnable(HdmiTxSsPtr->VtcPtr);

  return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function set the TMDSClock and return it.
*
* @param  None.
*
* @return Calculated TMDS Clock
*
* @note   None.
*
*****************************************************************************/
static u32 XV_HdmiTxSS_SetTMDS(XV_HdmiTxSs *InstancePtr,
                        XVidC_VideoMode VideoMode,
                        XVidC_ColorFormat ColorFormat,
                        XVidC_ColorDepth Bpc) {

    u32 TmdsClk;

    TmdsClk = XV_HdmiTx_GetTmdsClk(InstancePtr->HdmiTxPtr,
                                     VideoMode,
                                     ColorFormat,
                                     Bpc);

    /* Store TMDS clock for future reference */
	InstancePtr->HdmiTxPtr->Stream.TMDSClock = TmdsClk;

    return TmdsClk;
}

/*****************************************************************************/
/**
*
* This function is called when a TX connect event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_ConnectCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;

  /* Is the cable connected */
  if (XV_HdmiTx_IsStreamConnected(HdmiTxSsPtr->HdmiTxPtr)) {
#ifdef XV_HDMITXSS_LOG_ENABLE
    XV_HdmiTxSs_LogWrite(HdmiTxSsPtr, XV_HDMITXSS_LOG_EVT_CONNECT, 0);
#endif

    /* Reset DDC */
    XV_HdmiTx_DdcDisable(HdmiTxSsPtr->HdmiTxPtr);

    /* Set stream connected flag */
    HdmiTxSsPtr->IsStreamConnected = (TRUE);

#ifdef USE_HDCP_TX
    /* Push connect event to the HDCP event queue */
    XV_HdmiTxSs_HdcpPushEvent(HdmiTxSsPtr, XV_HDMITXSS_HDCP_CONNECT_EVT);
#endif
  }

  /* TX cable is disconnected */
  else {
#ifdef XV_HDMITXSS_LOG_ENABLE
    XV_HdmiTxSs_LogWrite(HdmiTxSsPtr, XV_HDMITXSS_LOG_EVT_DISCONNECT, 0);
#endif
    /* Assert HDMI TXCore link reset */
    XV_HdmiTxSs_TXCore_LRST(HdmiTxSsPtr, TRUE);
    XV_HdmiTxSs_TXCore_VRST(HdmiTxSsPtr, TRUE);

    /* Assert SYSCLK VID_OUT bridge reset */
    XV_HdmiTxSs_SYSRST(HdmiTxSsPtr, TRUE);
    XV_HdmiTxSs_VRST(HdmiTxSsPtr, TRUE);

    /* Reset DDC */
    XV_HdmiTx_DdcDisable(HdmiTxSsPtr->HdmiTxPtr);

    /* Set stream connected flag */
    HdmiTxSsPtr->IsStreamConnected = (FALSE);

#ifdef USE_HDCP_TX
    /* Push disconnect event to the HDCP event queue */
    XV_HdmiTxSs_HdcpPushEvent(HdmiTxSsPtr, XV_HDMITXSS_HDCP_DISCONNECT_EVT);
#endif
  }

  /* Check if user callback has been registered */
  if (HdmiTxSsPtr->ConnectCallback) {
    HdmiTxSsPtr->ConnectCallback(HdmiTxSsPtr->ConnectRef);
  }
}

/*****************************************************************************/
/**
*
* This function is called when a TX toggle event has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_ToggleCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;

  /* Reset DDC */
  XV_HdmiTx_DdcDisable(HdmiTxSsPtr->HdmiTxPtr);

  /* Set toggle flag */
  HdmiTxSsPtr->IsStreamToggled = TRUE;
#ifdef XV_HDMITXSS_LOG_ENABLE
  XV_HdmiTxSs_LogWrite(HdmiTxSsPtr, XV_HDMITXSS_LOG_EVT_TOGGLE, 0);
#endif
  /* Check if user callback has been registered */
  if (HdmiTxSsPtr->ToggleCallback) {
    HdmiTxSsPtr->ToggleCallback(HdmiTxSsPtr->ToggleRef);
  }

  /* Clear toggle flag */
  HdmiTxSsPtr->IsStreamToggled = FALSE;
}

/*****************************************************************************/
/**
*
* This function is called when a bridge unlocked has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_BrdgUnlockedCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;

#ifdef XV_HDMITXSS_LOG_ENABLE
  XV_HdmiTxSs_LogWrite(HdmiTxSsPtr, XV_HDMITXSS_LOG_EVT_BRDG_UNLOCKED, 0);
#endif

  /* Check if user callback has been registered */
  if (HdmiTxSsPtr->BrdgUnlockedCallback) {
      HdmiTxSsPtr->BrdgUnlockedCallback(HdmiTxSsPtr->BrdgUnlockedRef);
  }
}

/*****************************************************************************/
/**
*
* This function is called when a bridge Overflow has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_BrdgOverflowCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;

  xdbg_printf(XDBG_DEBUG_GENERAL,
              "\r\nWarning: TX Bridge Overflow\r\n");

  /* Check if user callback has been registered */
  if (HdmiTxSsPtr->BrdgOverflowCallback) {
      HdmiTxSsPtr->BrdgOverflowCallback(HdmiTxSsPtr->BrdgOverflowRef);
  }
}


/*****************************************************************************/
/**
*
* This function is called when a bridge Underflow has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_BrdgUnderflowCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;

  xdbg_printf(XDBG_DEBUG_GENERAL,
              "\r\nWarning: TX Bridge Underflow\r\n");

  /* Check if user callback has been registered */
  if (HdmiTxSsPtr->BrdgUnderflowCallback) {
      HdmiTxSsPtr->BrdgUnderflowCallback(HdmiTxSsPtr->BrdgUnderflowRef);
  }
}

/*****************************************************************************/
/**
*
* This function is called when a TX vsync has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_VsCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;

  /* Support of backward compatibility by checking HDMI-TXSs Major AppVersion
   * parameter. If value is 0, then TX SS driver will send
   * the InfoFrame. Note: The APIs used here are deprecated.
   */
  if (HdmiTxSsPtr->AppMajVer == 0) {
  	// AVI infoframe
  	XV_HdmiTxSs_SendAviInfoframe(HdmiTxSsPtr->HdmiTxPtr);

  	// Vendor-Specific InfoFrame
	XV_HdmiTxSs_SendVSInfoframe(HdmiTxSsPtr->HdmiTxPtr);
  }

  // Check if user callback has been registered
  if (HdmiTxSsPtr->VsCallback) {
      HdmiTxSsPtr->VsCallback(HdmiTxSsPtr->VsRef);
  }
}

/*****************************************************************************/
/**
*
* This function sends AVI info frames.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_SendAviInfoframe(XV_HdmiTx *HdmiTx)
{
  u8 Index;
  u8 Data;
  u8 Crc;

  /* Header, Packet type*/
  HdmiTx->Aux.Header.Byte[0] = 0x82;

  /* Version */
  HdmiTx->Aux.Header.Byte[1] = 0x02;

  /* Length */
  HdmiTx->Aux.Header.Byte[2] = 13;

  /* Checksum (this will be calculated by the HDMI TX IP) */
  HdmiTx->Aux.Header.Byte[3] = 0;

  /* Data */
  switch (HdmiTx->Stream.Video.ColorFormatId) {
    case XVIDC_CSF_YCRCB_422:
      Data = 1 << 5;
      break;

    case XVIDC_CSF_YCRCB_444:
      Data = 2 << 5;
      break;

    case XVIDC_CSF_YCRCB_420:
      Data = 3 << 5;
      break;

    default:
      Data = 0;
      break;
  }

  HdmiTx->Aux.Data.Byte[1] = Data;

  HdmiTx->Aux.Data.Byte[2] = 0;
  HdmiTx->Aux.Data.Byte[3] = 0;

  if (!XVidC_IsStream3D(&HdmiTx->Stream.Video) &&
      (HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_24_P ||
       HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_25_P ||
       HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_30_P ||
       HdmiTx->Stream.Video.VmId == XVIDC_VM_4096x2160_24_P)) {
    HdmiTx->Aux.Data.Byte[4] = 0;
  }
  else {
      HdmiTx->Aux.Data.Byte[4] = HdmiTx->Stream.Vic;
  }

  for (Index = 5; Index < 32; Index++) {
    HdmiTx->Aux.Data.Byte[Index] = 0;
  }

  /* Calculate AVI infoframe checksum */
  Crc = 0;

  /* Header */
  for (Index = 0; Index < 3; Index++) {
    Crc += HdmiTx->Aux.Header.Byte[Index];
  }

  /* Data */
  for (Index = 1; Index < 5; Index++) {
    Crc += HdmiTx->Aux.Data.Byte[Index];
  }

  Crc = 256 - Crc;

  HdmiTx->Aux.Data.Byte[0] = Crc;

  XV_HdmiTx_AuxSend(HdmiTx);
}

/*****************************************************************************/
/**
*
* This function sends the Vendor Specific Info Frame.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_SendVSInfoframe(XV_HdmiTx *HdmiTx)
{
    XV_HdmiTx_VSIF VSIF;

    VSIF.Version = 0x1;
    VSIF.IEEE_ID = 0xC03;

    if (XVidC_IsStream3D(&HdmiTx->Stream.Video)) {
        VSIF.Format = XV_HDMITX_VSIF_VF_3D;
        VSIF.Info_3D.Stream = HdmiTx->Stream.Video.Info_3D;
        VSIF.Info_3D.MetaData.IsPresent = FALSE;
    }
    else if (HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_24_P ||
             HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_25_P ||
             HdmiTx->Stream.Video.VmId == XVIDC_VM_3840x2160_30_P ||
             HdmiTx->Stream.Video.VmId == XVIDC_VM_4096x2160_24_P) {
        VSIF.Format = XV_HDMITX_VSIF_VF_EXTRES;

        /* Set HDMI VIC */
        switch(HdmiTx->Stream.Video.VmId) {
            case XVIDC_VM_4096x2160_24_P :
                VSIF.HDMI_VIC = 4;
                break;
            case XVIDC_VM_3840x2160_24_P :
                VSIF.HDMI_VIC = 3;
                break;
            case XVIDC_VM_3840x2160_25_P :
                VSIF.HDMI_VIC = 2;
                break;
            case XVIDC_VM_3840x2160_30_P :
                VSIF.HDMI_VIC = 1;
                break;
            default :
                break;
        }
    }
    else {
        VSIF.Format = XV_HDMITX_VSIF_VF_NOINFO;
    }

    XV_HdmiTx_VSIF_GeneratePacket(&VSIF,(XHdmiC_Aux *)&HdmiTx->Aux);

    XV_HdmiTx_AuxSend(HdmiTx);
}

/*****************************************************************************/
/**
*
* This function is called when the TX stream is up.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_StreamUpCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;

  /* Set stream up flag */
  HdmiTxSsPtr->IsStreamUp = (TRUE);

#ifdef USE_HDCP_TX
  /* Push the stream-up event to the HDCP event queue */
  XV_HdmiTxSs_HdcpPushEvent(HdmiTxSsPtr, XV_HDMITXSS_HDCP_STREAMUP_EVT);
#endif

  /* Check if user callback has been registered.
     User may change the video stream properties in the callback;
     therefore, execute the callback before changing stream settings. */
  if (HdmiTxSsPtr->StreamUpCallback) {
      HdmiTxSsPtr->StreamUpCallback(HdmiTxSsPtr->StreamUpRef);
  }

  /* Set TX sample rate */
  XV_HdmiTx_SetSampleRate(HdmiTxSsPtr->HdmiTxPtr, HdmiTxSsPtr->SamplingRate);

  /* Release VID_IN bridge resets */
  XV_HdmiTxSs_SYSRST(HdmiTxSsPtr, FALSE);
  XV_HdmiTxSs_VRST(HdmiTxSsPtr, FALSE);

  /* Release HDMI TXCore resets */
  XV_HdmiTxSs_TXCore_LRST(HdmiTxSsPtr, FALSE);
  XV_HdmiTxSs_TXCore_VRST(HdmiTxSsPtr, FALSE);

  if (HdmiTxSsPtr->VtcPtr) {
    /* Setup VTC */
    XV_HdmiTxSs_VtcSetup(HdmiTxSsPtr);
  }

  if (HdmiTxSsPtr->AudioEnabled) {
      /* HDMI TX unmute audio */
      HdmiTxSsPtr->AudioMute = (FALSE);
      XV_HdmiTx_AudioEnable(HdmiTxSsPtr->HdmiTxPtr);
  }

  /* Configure video bridge mode according to HW setting and video format */
  XV_HdmiTxSs_ConfigBridgeMode(HdmiTxSsPtr);
#ifdef XV_HDMITXSS_LOG_ENABLE
  XV_HdmiTxSs_LogWrite(HdmiTxSsPtr, XV_HDMITXSS_LOG_EVT_STREAMUP, 0);
#endif
}

/*****************************************************************************/
/**
*
* This function is called when the TX stream is down.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_StreamDownCallback(void *CallbackRef)
{
  XV_HdmiTxSs *HdmiTxSsPtr = (XV_HdmiTxSs *)CallbackRef;
  /* Assert HDMI TXCore link reset */
  XV_HdmiTxSs_TXCore_LRST(HdmiTxSsPtr, TRUE);
  XV_HdmiTxSs_TXCore_VRST(HdmiTxSsPtr, TRUE);

  /* Assert SYSCLK VID_OUT bridge reset */
  XV_HdmiTxSs_SYSRST(HdmiTxSsPtr, TRUE);
  XV_HdmiTxSs_VRST(HdmiTxSsPtr, TRUE);

  /* Reset DDC */
  XV_HdmiTx_DdcDisable(HdmiTxSsPtr->HdmiTxPtr);

  /* Set stream up flag */
  HdmiTxSsPtr->IsStreamUp = (FALSE);
  HdmiTxSsPtr->DrmInfoframe.Static_Metadata_Descriptor_ID = 0xff;
  HdmiTxSsPtr->DrmInfoframe.EOTF = 0xff;
#ifdef XV_HDMITXSS_LOG_ENABLE
  XV_HdmiTxSs_LogWrite(HdmiTxSsPtr, XV_HDMITXSS_LOG_EVT_STREAMDOWN, 0);
#endif
#ifdef USE_HDCP_TX
  /* Push the stream-down event to the HDCP event queue */
  XV_HdmiTxSs_HdcpPushEvent(HdmiTxSsPtr, XV_HDMITXSS_HDCP_STREAMDOWN_EVT);
#endif

  /* Check if user callback has been registered */
  if (HdmiTxSsPtr->StreamDownCallback) {
      HdmiTxSsPtr->StreamDownCallback(HdmiTxSsPtr->StreamDownRef);
  }
}

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                     Callback Function Type
* -----------------------         ---------------------------------------------
* (XV_HDMITXSS_HANDLER_CONNECT)       HpdCallback
* (XV_HDMITXSS_HANDLER_VS)            VsCallback
* (XV_HDMITXSS_HANDLER_STREAM_DOWN)   StreamDownCallback
* (XV_HDMITXSS_HANDLER_STREAM_UP)     StreamUpCallback
* (XV_HDMITXSS_HANDLER_BRDGOVERFLOW)  BrdgOverflowCallback
* (XV_HDMITXSS_HANDLER_BRDGUNDERFLOW) BrdgUnderflowCallback
* (XV_HDMITXSS_HANDLER_HDCP_AUTHENTICATED)
* (XV_HDMITXSS_HANDLER_HDCP_DOWNSTREAM_TOPOLOGY_AVAILABLE)
* (XV_HDMITXSS_HANDLER_HDCP_UNAUTHENTICATED)
* </pre>
*
* @param    InstancePtr is a pointer to the HDMI TX Subsystem instance.
* @param    HandlerType specifies the type of handler.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*       callback function when it is invoked.
*
* @return
*       - XST_SUCCESS if callback function installed successfully.
*       - XST_INVALID_PARAM when HandlerType is invalid.
*
* @note     Invoking this function for a handler that already has been
*       installed replaces it with the new handler.
*
******************************************************************************/
int XV_HdmiTxSs_SetCallback(XV_HdmiTxSs *InstancePtr,
		XV_HdmiTxSs_HandlerType HandlerType,
		void *CallbackFunc,
		void *CallbackRef)
{
    u32 Status;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(HandlerType >= (XV_HDMITXSS_HANDLER_CONNECT));
    Xil_AssertNonvoid(CallbackFunc != NULL);
    Xil_AssertNonvoid(CallbackRef != NULL);

    /* Check for handler type */
    switch (HandlerType) {
        // Connect
        case (XV_HDMITXSS_HANDLER_CONNECT):
            InstancePtr->ConnectCallback = (XV_HdmiTxSs_Callback)CallbackFunc;
            InstancePtr->ConnectRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Toggle
        case (XV_HDMITXSS_HANDLER_TOGGLE):
            InstancePtr->ToggleCallback = (XV_HdmiTxSs_Callback)CallbackFunc;
            InstancePtr->ToggleRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Bridge Unlocked
        case (XV_HDMITXSS_HANDLER_BRDGUNLOCK):
            InstancePtr->BrdgUnlockedCallback =
			    (XV_HdmiTxSs_Callback)CallbackFunc;
            InstancePtr->BrdgUnlockedRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Bridge Overflow
        case (XV_HDMITXSS_HANDLER_BRDGOVERFLOW):
            InstancePtr->BrdgOverflowCallback =
			    (XV_HdmiTxSs_Callback)CallbackFunc;
            InstancePtr->BrdgOverflowRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Bridge Underflow
        case (XV_HDMITXSS_HANDLER_BRDGUNDERFLOW):
            InstancePtr->BrdgUnderflowCallback =
			    (XV_HdmiTxSs_Callback)CallbackFunc;
            InstancePtr->BrdgUnderflowRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Vsync
        case (XV_HDMITXSS_HANDLER_VS):
            InstancePtr->VsCallback = (XV_HdmiTxSs_Callback)CallbackFunc;
            InstancePtr->VsRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Stream down
        case (XV_HDMITXSS_HANDLER_STREAM_DOWN):
            InstancePtr->StreamDownCallback =
                (XV_HdmiTxSs_Callback)CallbackFunc;
            InstancePtr->StreamDownRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // Stream up
        case (XV_HDMITXSS_HANDLER_STREAM_UP):
            InstancePtr->StreamUpCallback = (XV_HdmiTxSs_Callback)CallbackFunc;
            InstancePtr->StreamUpRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        // HDCP authenticated
        case (XV_HDMITXSS_HANDLER_HDCP_AUTHENTICATED):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            /* Register HDCP 1.4 callbacks */
            if (InstancePtr->Hdcp14Ptr) {
              XHdcp1x_SetCallback(InstancePtr->Hdcp14Ptr,
                                  XHDCP1X_HANDLER_AUTHENTICATED,
								  (void *)(XHdcp1x_Callback) CallbackFunc,
								  (void *)CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
            /* Register HDCP 2.2 callbacks */
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Tx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_TX_HANDLER_AUTHENTICATED,
									(void *)(XHdcp22_Tx_Callback)CallbackFunc,
									(void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        // HDCP downstream topology available
        case (XV_HDMITXSS_HANDLER_HDCP_DOWNSTREAM_TOPOLOGY_AVAILABLE):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            /** Register HDCP 1.4 callbacks */
            if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetCallBack(InstancePtr->Hdcp14Ptr,
                            (XHdcp1x_HandlerType) XHDCP1X_RPTR_HDLR_REPEATER_EXCHANGE,
							(void *) (XHdcp1x_Callback)CallbackFunc,
							(void *) CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
            /** Register HDCP 2.2 callbacks */
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Tx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_TX_HANDLER_DOWNSTREAM_TOPOLOGY_AVAILABLE,
									(void *)(XHdcp22_Tx_Callback)CallbackFunc,
									(void *)CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
            break;

        // HDCP unauthenticated
        case (XV_HDMITXSS_HANDLER_HDCP_UNAUTHENTICATED):
#ifdef XPAR_XHDCP_NUM_INSTANCES
            /** Register HDCP 1.4 callbacks */
            if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetCallBack(InstancePtr->Hdcp14Ptr,
                            XHDCP1X_HANDLER_UNAUTHENTICATED,
							(void *) (XHdcp1x_Callback)CallbackFunc,
							(void *) CallbackRef);
            }
#endif

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
            /** Register HDCP 2.2 callbacks */
            if (InstancePtr->Hdcp22Ptr) {
              XHdcp22Tx_SetCallback(InstancePtr->Hdcp22Ptr,
                                    XHDCP22_TX_HANDLER_UNAUTHENTICATED,
                                    (void *) (XHdcp22_Tx_Callback)CallbackFunc,
									(void *) CallbackRef);
            }
#endif
            Status = (XST_SUCCESS);
          break;

        default:
            Status = (XST_INVALID_PARAM);
            break;
    }

    return Status;
}

/*****************************************************************************/
/**
*
* This function reads the HDMI Sink EDID.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_ReadEdid(XV_HdmiTxSs *InstancePtr, u8 *Buffer)
{
    u32 Status;

    // Default
    Status = (XST_FAILURE);

    // Check if a sink is connected
    if (InstancePtr->IsStreamConnected == (TRUE)) {

      *Buffer = 0x00;   // Offset zero
      Status = XV_HdmiTx_DdcWrite(InstancePtr->HdmiTxPtr, 0x50, 1, Buffer,
        (FALSE));

      // Check if write was successful
      if (Status == (XST_SUCCESS)) {
        // Read edid
        Status = XV_HdmiTx_DdcRead(InstancePtr->HdmiTxPtr, 0x50, 256, Buffer,
            (TRUE));
      }
    }
  return Status;
}

/*****************************************************************************/
/**
*
* This function reads one block from the HDMI Sink EDID.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_ReadEdidSegment(XV_HdmiTxSs *InstancePtr, u8 *Buffer, u8 segment)
{
    u32 Status;

    u8 dummy = 0;

    // Default
    Status = (XST_FAILURE);

    // Check if a sink is connected
    if (InstancePtr->IsStreamConnected == (TRUE)) {

	  // For multiple segment EDID read
	  // First, read the first block, then read address 0x7e to know how many
	  // blocks, if more than 2 blocks, then select segment after first 2 blocks
	  // Use the following code to select segment

	  if(segment != 0) {
        // Segment Pointer
        Status = XV_HdmiTx_DdcWrite(InstancePtr->HdmiTxPtr, 0x30, 1, &segment,
        (FALSE));
	  }

      // Read blocks
      dummy = 0x00;   // Offset zero
      Status = XV_HdmiTx_DdcWrite(InstancePtr->HdmiTxPtr, 0x50, 1, &dummy,
        (FALSE));

      // Check if write was successful
      if (Status == (XST_SUCCESS)) {
        // Read edid
        Status = XV_HdmiTx_DdcRead(InstancePtr->HdmiTxPtr, 0x50, 128, Buffer,
            (TRUE));
      }

	  if(segment != 0) {
        // Segment Pointer
        Status = XV_HdmiTx_DdcWrite(InstancePtr->HdmiTxPtr, 0x30, 1, &segment,
        (FALSE));
	  }

      // Read blocks
      dummy = 0x80;   // Offset 128
      Status = XV_HdmiTx_DdcWrite(InstancePtr->HdmiTxPtr, 0x50, 1, &dummy,
        (FALSE));

      // Check if write was successful
      if (Status == (XST_SUCCESS)) {
        // Read edid
        Status = XV_HdmiTx_DdcRead(InstancePtr->HdmiTxPtr, 0x50, 128, &Buffer[128],
            (TRUE));
      }
    }
    else {
      xil_printf("No sink is connected.\r\n");
      xil_printf("Please connect a HDMI sink.\r\n");
    }
  return Status;
}

/*****************************************************************************/
/**
*
* This function shows the HDMI source edid.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_ShowEdid(XV_HdmiTxSs *InstancePtr)
{
    u8 Buffer[256];
    u8 Row;
    u8 Column;
    u8 Valid;
    u32 Status;
    u8 EdidManName[4];
	u8 segment = 0;
    u8 ExtensionFlag = 0;

    // Check if a sink is connected
    if (InstancePtr->IsStreamConnected == (TRUE)) {

      // Default
      Valid = (FALSE);

      // Read Sink Edid Segment 0
      Status = XV_HdmiTxSs_ReadEdidSegment(InstancePtr, (u8*)&Buffer, segment);

      // Check if read was successful
      if (Status == (XST_SUCCESS)) {
        XVidC_EdidGetManName(&Buffer[0], (char *) EdidManName);
        xil_printf("\r\nMFG name : %S\r\n", EdidManName);

		ExtensionFlag = Buffer[126];
		ExtensionFlag = ExtensionFlag >> 1;
        xil_printf("Number of Segment : %d\r\n", ExtensionFlag+1);
        xil_printf("\r\nRaw data\r\n");
        xil_printf("----------------------------------------------------\r\n");
	  }

      segment = 0;
	  while (segment <= ExtensionFlag)
	  {
        // Check if read was successful
        if (Status == (XST_SUCCESS)) {
          xil_printf("\r\n---- Segment %d ----\r\n", segment);
          xil_printf("----------------------------------------------------\r\n");
          for (Row = 0; Row < 16; Row++) {
            xil_printf("%02X : ", (Row*16));
            for (Column = 0; Column < 16; Column++) {
              xil_printf("%02X ", Buffer[(Row*16)+Column]);
            }
        xil_printf("\r\n");
          }
          Valid = (TRUE);

          segment++;
		  if(segment <= ExtensionFlag) {
            Status = XV_HdmiTxSs_ReadEdidSegment(InstancePtr, (u8*)&Buffer, segment);
		  }
        }
	  }

      if (!Valid) {
        xil_printf("Error reading EDID\r\n");
      }
    }

    else {
      xil_printf("No sink is connected.\r\n");
    }
}

/*****************************************************************************/
/**
*
* This function starts the HDMI TX stream
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_StreamStart(XV_HdmiTxSs *InstancePtr)
{
  u32 TmdsClk;

  // Set TX pixel rate
  XV_HdmiTx_SetPixelRate(InstancePtr->HdmiTxPtr);

  // Set TX color depth
  XV_HdmiTx_SetColorDepth(InstancePtr->HdmiTxPtr);

  // Set TX color format
  XV_HdmiTx_SetColorFormat(InstancePtr->HdmiTxPtr);

  // Set the TMDS Clock
  TmdsClk = XV_HdmiTxSS_SetTMDS(InstancePtr,
				 InstancePtr->HdmiTxPtr->Stream.Video.VmId,
				 InstancePtr->HdmiTxPtr->Stream.Video.ColorFormatId,
				 InstancePtr->HdmiTxPtr->Stream.Video.ColorDepth);

  // Set TX scrambler
  XV_HdmiTx_Scrambler(InstancePtr->HdmiTxPtr);

  // Set TX clock ratio
  XV_HdmiTx_ClockRatio(InstancePtr->HdmiTxPtr);
#ifdef XV_HDMITXSS_LOG_ENABLE
  XV_HdmiTxSs_LogWrite(InstancePtr, XV_HDMITXSS_LOG_EVT_STREAMSTART, 0);
#endif
  if ((InstancePtr->HdmiTxPtr->Stream.IsHdmi20 == (FALSE))
  		&& (TmdsClk > 340000000)) {
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "\r\nWarning: Sink does not support HDMI 2.0\r\n");
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "         Connect to HDMI 2.0 Sink or \r\n");
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "         Change to HDMI 1.4 video format\r\n\r\n");
  }
}

/*****************************************************************************/
/**
*
* This function enables / disables the TX scrambler
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  Enable TRUE:Enable scrambler FALSE:Disable scrambler
*
* @return None.
*
* @note   The scrambler setting will revert to the default behavior during the
*         next stream configuration (scrambler is enabled for HDMI 2.0 video
*         and disabled when the video is of HDMI 1.4 or lower)
*
******************************************************************************/
void XV_HdmiTxSs_SetScrambler(XV_HdmiTxSs *InstancePtr, u8 Enable)
{
	if (InstancePtr->HdmiTxPtr->Stream.IsHdmi20 == (TRUE)) {
		/* Override the default behavior of the scrambler so that scrambling
		 * can be disabled even when TMDS Clock > 340 MHz
		 */
		InstancePtr->HdmiTxPtr->Stream.OverrideScrambler = (TRUE);
		/* Set the IsScrambled flag in order to enable or disable scrambler
		 * when XV_HdmiTx_Scrambler is called
		 */
		XV_HdmiTxSs_SetVideoStreamScramblingFlag(InstancePtr, Enable);
		/* Enable / disable scrambler depending on the value set to
		 * IsScrambled
		 */
		XV_HdmiTx_Scrambler(InstancePtr->HdmiTxPtr);
		// Disable the override of the scrambler
		InstancePtr->HdmiTxPtr->Stream.OverrideScrambler = (FALSE);
	} else {
		xdbg_printf(XDBG_DEBUG_GENERAL,
			  "\r\nWarning: Scrambler is always disabled when sink does"
			  "not support SCDC control interface\r\n");
	}
}

/*****************************************************************************/
/**
*
* This function sends audio info frames.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SendAuxInfoframe(XV_HdmiTxSs *InstancePtr, void *Aux)
{
  u8 Index;
  u8 Crc;
  XHdmiC_Aux *tx_aux = (XHdmiC_Aux *)Aux;

  if (Aux == (NULL)) {
      /* Header, Packet type */
      InstancePtr->HdmiTxPtr->Aux.Header.Byte[0] = 0x84;

      /* Version */
      InstancePtr->HdmiTxPtr->Aux.Header.Byte[1] = 0x01;

      /* Length */
      InstancePtr->HdmiTxPtr->Aux.Header.Byte[2] = 10;

      /* Checksum (this will be calculated by the HDMI TX IP) */
      InstancePtr->HdmiTxPtr->Aux.Header.Byte[3] = 0;

      /* 2 Channel count. Audio coding type refer to stream */
      InstancePtr->HdmiTxPtr->Aux.Data.Byte[1] = 0x1;

      for (Index = 2; Index < 32; Index++) {
        InstancePtr->HdmiTxPtr->Aux.Data.Byte[Index] = 0;
      }

      /* Calculate AVI infoframe checksum */
      Crc = 0;

      /* Header */
      for (Index = 0; Index < 3; Index++) {
        Crc += InstancePtr->HdmiTxPtr->Aux.Header.Byte[Index];
      }

      /* Data */
      for (Index = 1; Index < 5; Index++) {
        Crc += InstancePtr->HdmiTxPtr->Aux.Data.Byte[Index];
      }

      Crc = 256 - Crc;
      InstancePtr->HdmiTxPtr->Aux.Data.Byte[0] = Crc;

      XV_HdmiTx_AuxSend(InstancePtr->HdmiTxPtr);

  }

  else {
      // Copy Audio Infoframe
      if (tx_aux->Header.Byte[0] == 0x84) {
        // Header
        InstancePtr->HdmiTxPtr->Aux.Header.Data = tx_aux->Header.Data;

        // Data
        for (Index = 0; Index < 8; Index++) {
          InstancePtr->HdmiTxPtr->Aux.Data.Data[Index] =
            tx_aux->Data.Data[Index];
        }
      }
  }

  /* Send packet */
  XV_HdmiTx_AuxSend(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
*
* This function sends generic info frames.
*
* @param  None.
*
* @return
*       - XST_SUCCESS if infoframes transmitted successfully.
*       - XST_FAILURE if AUX FIFO is full.
*
* @note   None.
*
******************************************************************************/
u32 XV_HdmiTxSs_SendGenericAuxInfoframe(XV_HdmiTxSs *InstancePtr, void *Aux)
{
  u8 Index;
  XHdmiC_Aux *tx_aux = (XHdmiC_Aux *)Aux;

  // Header
  InstancePtr->HdmiTxPtr->Aux.Header.Data = tx_aux->Header.Data;

  // Data
  for (Index = 0; Index < 8; Index++) {
    InstancePtr->HdmiTxPtr->Aux.Data.Data[Index] =
    tx_aux->Data.Data[Index];
  }

  /* Send packet */
  return XV_HdmiTx_AuxSend(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS number of active audio channels
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  AudioChannels
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetAudioChannels(XV_HdmiTxSs *InstancePtr, u8 AudioChannels)
{
	// Audio InfoFrame's channel count starts from 2ch = 1
	InstancePtr->AudioInfoframe.ChannelCount = AudioChannels - 1;
    InstancePtr->AudioChannels = AudioChannels;
    XV_HdmiTx_SetAudioChannels(InstancePtr->HdmiTxPtr, AudioChannels);
#ifdef XV_HDMITXSS_LOG_ENABLE
    XV_HdmiTxSs_LogWrite(InstancePtr,
                         XV_HDMITXSS_LOG_EVT_SETAUDIOCHANNELS,
                         AudioChannels);
#endif
}

/*****************************************************************************/
/**
*
* This function set HDMI TX audio parameters
*
* @param  Enable 0: Unmute the audio 1: Mute the audio.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_AudioMute(XV_HdmiTxSs *InstancePtr, u8 Enable)
{
  //Audio Mute Mode
  if (Enable){
	XV_HdmiTx_AudioDisable(InstancePtr->HdmiTxPtr);
#ifdef XV_HDMITXSS_LOG_ENABLE
    XV_HdmiTxSs_LogWrite(InstancePtr, XV_HDMITXSS_LOG_EVT_AUDIOMUTE, 0);
#endif
  }
  else{
	XV_HdmiTx_AudioEnable(InstancePtr->HdmiTxPtr);
#ifdef XV_HDMITXSS_LOG_ENABLE
    XV_HdmiTxSs_LogWrite(InstancePtr, XV_HDMITXSS_LOG_EVT_AUDIOUNMUTE, 0);
#endif
  }
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS Audio Format
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  format 1:HBR 0:L-PCM
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetAudioFormat(XV_HdmiTxSs *InstancePtr,
		 XV_HdmiTx_AudioFormatType format)
{
    XV_HdmiTx_SetAudioFormat(InstancePtr->HdmiTxPtr, format);
}

/*****************************************************************************/
/**
*
* This function gets the active audio format
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
*
* @return   Active audio format of HDMI Tx Subsystem
*
* @note     None.
*
******************************************************************************/
XV_HdmiTx_AudioFormatType XV_HdmiTxSs_GetAudioFormat(XV_HdmiTxSs *InstancePtr)
{
	XV_HdmiTx_AudioFormatType Format;

	Format = XV_HdmiTx_GetAudioFormat(InstancePtr->HdmiTxPtr);

    return Format;
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS Aux structure
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_Aux *XV_HdmiTxSs_GetAuxiliary(XV_HdmiTxSs *InstancePtr)
{
    return (&(InstancePtr->HdmiTxPtr->Aux));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS AVI InfoFrame structure
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
*
* @return XHdmiC_AVI_InfoFrame pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_AVI_InfoFrame *XV_HdmiTxSs_GetAviInfoframe(XV_HdmiTxSs *InstancePtr)
{
    return (&(InstancePtr->AVIInfoframe));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS Audio InfoFrame structure
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
*
* @return XHdmiC_AudioInfoFrame pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_AudioInfoFrame *XV_HdmiTxSs_GetAudioInfoframe(XV_HdmiTxSs *InstancePtr)
{
    return (&(InstancePtr->AudioInfoframe));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS Vendor Specific InfoFrame
* structure
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
*
* @return XHdmiC_VSIF pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_VSIF *XV_HdmiTxSs_GetVSIF(XV_HdmiTxSs *InstancePtr)
{
    return (&(InstancePtr->VSIF));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS DRM InfoFrame
* structure
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
*
* @return XHdmiC_DRMInfoFrame pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_DRMInfoFrame *XV_HdmiTxSs_GetDrmInfoframe(XV_HdmiTxSs *InstancePtr)
{
    return (&(InstancePtr->DrmInfoframe));
}


/*****************************************************************************/
/**
*
* This function set HDMI TX susbsystem stream parameters
*
* @param  None.
*
* @return Calculated TMDS Clock
*
* @note   None.
*
******************************************************************************/
u32 XV_HdmiTxSs_SetStream(XV_HdmiTxSs *InstancePtr,
        XVidC_VideoMode VideoMode, XVidC_ColorFormat ColorFormat,
        XVidC_ColorDepth Bpc, XVidC_3DInfo *Info3D)
{
  u32 TmdsClock = 0;

  TmdsClock = XV_HdmiTx_SetStream(InstancePtr->HdmiTxPtr, VideoMode,
    ColorFormat, Bpc, InstancePtr->Config.Ppc, Info3D);

#ifdef XV_HDMITXSS_LOG_ENABLE
  XV_HdmiTxSs_LogWrite(InstancePtr, XV_HDMITXSS_LOG_EVT_SETSTREAM, 0);
#endif
  if(TmdsClock == 0) {
    xdbg_printf(XDBG_DEBUG_GENERAL,
                "\r\nWarning: Sink does not support HDMI 2.0\r\n");
    xdbg_printf(XDBG_DEBUG_GENERAL,
                "         Connect to HDMI 2.0 Sink or \r\n");
    xdbg_printf(XDBG_DEBUG_GENERAL,
                "         Change to HDMI 1.4 video format\r\n\r\n");
}

  return TmdsClock;
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS video stream
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
XVidC_VideoStream *XV_HdmiTxSs_GetVideoStream(XV_HdmiTxSs *InstancePtr)
{
    return (&InstancePtr->HdmiTxPtr->Stream.Video);
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video stream
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetVideoStream(XV_HdmiTxSs *InstancePtr,
                                    XVidC_VideoStream VidStream)
{
    // Set Stream Properties
    InstancePtr->HdmiTxPtr->Stream.Video = VidStream;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video Identification code
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  SamplingRate Value
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetSamplingRate(XV_HdmiTxSs *InstancePtr, u8 SamplingRate)
{
    InstancePtr->SamplingRate = SamplingRate;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video Identification code
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  InstancePtr VIC Flag Value
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetVideoIDCode(XV_HdmiTxSs *InstancePtr, u8 Vic)
{
    InstancePtr->HdmiTxPtr->Stream.Vic = Vic;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  InstancePtr VIC Value 1:HDMI 0:DVI
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetVideoStreamType(XV_HdmiTxSs *InstancePtr, u8 StreamType)
{
    //InstancePtr->HdmiTxPtr->Stream.IsHdmi = StreamType;
    if (StreamType) {
    	XV_HdmiTxSS_SetHdmiMode(InstancePtr);
    	XV_HdmiTxSs_AudioMute(InstancePtr, FALSE);
    } else {
    	XV_HdmiTxSs_AudioMute(InstancePtr, TRUE);
    	XV_HdmiTxSS_SetDviMode(InstancePtr);
    }
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  IsScrambled 1:IsScrambled 0: not Scrambled
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetVideoStreamScramblingFlag(XV_HdmiTxSs *InstancePtr,
                                                            u8 IsScrambled)
{
    InstancePtr->HdmiTxPtr->Stream.IsScrambled = IsScrambled;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video stream scrambling behaviour. Setting
* OverrideScramble to true will force enabling/disabling scrambling function
* based on the value of IsScrambled flag.
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  OverrideScramble
*         0: Scrambling is always enabled for HDMI 2.0 resolutions and is
*            enabled /or disabled for HDMI 1.4 resolutions based on IsScrambled
*            value
*         1: Enable scrambling if IsScrambled is TRUE and disable scrambling
*            if IsScrambled is FALSE
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetVideoStreamScramblingOverrideFlag(XV_HdmiTxSs *InstancePtr,
                                                           u8 OverrideScramble)
{
    InstancePtr->HdmiTxPtr->Stream.OverrideScrambler = OverrideScramble;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS TMDS Cock Ratio
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param  Ratio 0 - 1/10, 1 - 1/40
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_SetTmdsClockRatio(XV_HdmiTxSs *InstancePtr, u8 Ratio)
{
    InstancePtr->HdmiTxPtr->Stream.TMDSClockRatio = Ratio;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video Identification code
*
* @param  InstancePtr pointer to XV_HdmiTxSs instance
* @param
*
* @return Stream Data Structure (TMDS Clock)
*
* @note   None.
*
******************************************************************************/
u32 XV_HdmiTxSs_GetTmdsClockFreqHz(XV_HdmiTxSs *InstancePtr)
{
    return (InstancePtr->HdmiTxPtr->Stream.TMDSClock);
}

/*****************************************************************************/
/**
*
* This function detects connected sink is a HDMI 2.0/HDMI 1.4 sink device
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
*
* @return
*       - XST_SUCCESS if HDMI 2.0
*       - XST_FAILURE if HDMI 1.4
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_DetectHdmi20(XV_HdmiTxSs *InstancePtr)
{
      return (XV_HdmiTx_DetectHdmi20(InstancePtr->HdmiTxPtr));
}

/*****************************************************************************/
/**
*
* This function is called when HDMI TX SS TMDS clock changes
*
* @param  None.
*
* @return None
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_RefClockChangeInit(XV_HdmiTxSs *InstancePtr)
{

      /* Assert VID_IN bridge resets */
      XV_HdmiTxSs_SYSRST(InstancePtr, TRUE);
      XV_HdmiTxSs_VRST(InstancePtr, TRUE);

      /* Assert HDMI TXCore resets */
      XV_HdmiTxSs_TXCore_LRST(InstancePtr, TRUE);
      XV_HdmiTxSs_TXCore_VRST(InstancePtr, TRUE);

      /* Clear variables */
      XV_HdmiTx_Clear(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
*
* This function prints the HDMI TX SS timing information
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_ReportTiming(XV_HdmiTxSs *InstancePtr)
{
      if (((u32) XV_HdmiTx_GetMode(InstancePtr->HdmiTxPtr)) == 0) {
        xil_printf("HDMI TX Mode - DVI \r\n");
      }
      else {
        xil_printf("HDMI TX Mode - HDMI \r\n");
      }

      if (XV_HdmiTxSS_IsMasked(InstancePtr) == 0) {
	  xil_printf("HDMI Video Mask is Disabled\r\n\r\n");
      }
      else {
	  xil_printf("HDMI Video Mask is Enabled\r\n\r\n");
      }

      XV_HdmiTx_DebugInfo(InstancePtr->HdmiTxPtr);
      xil_printf("Scrambled: %0d\r\n",
        (XV_HdmiTx_IsStreamScrambled(InstancePtr->HdmiTxPtr)));
      xil_printf("Sample rate: %0d\r\n",
        (XV_HdmiTx_GetSampleRate(InstancePtr->HdmiTxPtr)));
      xil_printf("\r\n");

}

/*****************************************************************************/
/**
*
* This function prints the HDMI TX SS DRM If information
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_ReportDRMInfo(XV_HdmiTxSs *InstancePtr)
{
	XHdmiC_DRMInfoFrame *DrmInfoFramePtr;
	DrmInfoFramePtr = XV_HdmiTxSs_GetDrmInfoframe(InstancePtr);

	if (DrmInfoFramePtr->EOTF != 0xff)
		xil_printf("eotf: %d\r\n", DrmInfoFramePtr->EOTF);

	if (DrmInfoFramePtr->Static_Metadata_Descriptor_ID == 0xFF) {
		xil_printf("No DRM info\r\n");
		return;
	}

	xil_printf("DRM IF info:\r\n");
	xil_printf("desc id: %d\r\n", DrmInfoFramePtr->Static_Metadata_Descriptor_ID);
	xil_printf("display primaries x0, y0, x1, y1, x2, y2: %d %d %d %d %d %d\r\n",
			DrmInfoFramePtr->disp_primaries[0].x, DrmInfoFramePtr->disp_primaries[0].y,
			DrmInfoFramePtr->disp_primaries[1].x, DrmInfoFramePtr->disp_primaries[1].y,
			DrmInfoFramePtr->disp_primaries[2].x, DrmInfoFramePtr->disp_primaries[2].y
		  );
	xil_printf("white point x, y: %d %d\r\n",
			DrmInfoFramePtr->white_point.x, DrmInfoFramePtr->white_point.y);
	xil_printf("min/max display mastering luminance: %d %d\r\n",
			DrmInfoFramePtr->Min_Disp_Mastering_Luminance,
			DrmInfoFramePtr->Max_Disp_Mastering_Luminance);
	xil_printf("Max_CLL: %d\r\n", DrmInfoFramePtr->Max_Content_Light_Level);
	xil_printf("max_fall: %d\r\n", DrmInfoFramePtr->Max_Frame_Average_Light_Level);
}
/*****************************************************************************/
/**
*
* This function prints the HDMI TX SS audio information
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_ReportAudio(XV_HdmiTxSs *InstancePtr)
{
	xil_printf("Format   : ");
	switch (XV_HdmiTxSs_GetAudioFormat(InstancePtr)) {
	case 0:
		xil_printf("L-PCM\r\n");
		break;
	case 1:
		xil_printf("HBR\r\n");
		break;
	case 2:
		xil_printf("3D\r\n");
		break;
	default:
		xil_printf("Invalid\r\n");
		break;
  }
  xil_printf("Channels : %d\r\n",
  XV_HdmiTx_GetAudioChannels(InstancePtr->HdmiTxPtr));
}

/*****************************************************************************/
/**
*
* This function prints the HDMI TX SS subcore versions
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_ReportSubcoreVersion(XV_HdmiTxSs *InstancePtr)
{
  u32 Data;

  if (InstancePtr->HdmiTxPtr) {
     Data = XV_HdmiTx_GetVersion(InstancePtr->HdmiTxPtr);
     xil_printf("  HDMI TX version : %02d.%02d (%04x)\r\n",
        ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }

  if (InstancePtr->VtcPtr){
	 if (InstancePtr->IsStreamUp == TRUE){
       Data = XVtc_GetVersion(InstancePtr->VtcPtr);
       xil_printf("  VTC version     : %02d.%02d (%04x)\r\n",
          ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
	 }
	 else {
       xil_printf("  VTC version is not available for reading as ");
	   xil_printf("HDMI TX Video Clock is not ready.\r\n");
	 }
  }

#ifdef XPAR_XHDCP_NUM_INSTANCES
  // HDCP 1.4
  if (InstancePtr->Hdcp14Ptr){
     Data = XHdcp1x_GetVersion(InstancePtr->Hdcp14Ptr);
     xil_printf("  HDCP 1.4 TX version : %02d.%02d (%04x)\r\n",
        ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }
#endif

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
  // HDCP 2.2
  if (InstancePtr->Hdcp22Ptr) {
   Data = XHdcp22Tx_GetVersion(InstancePtr->Hdcp22Ptr);
   xil_printf("  HDCP 2.2 TX version : %02d.%02d (%04x)\r\n",
    ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }
#endif

}


/*****************************************************************************/
/**
*
* This function prints the HDMI TX SS subcore versions
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs_ReportInfo(XV_HdmiTxSs *InstancePtr)
{
    xil_printf("------------\r\n");
    xil_printf("HDMI TX SubSystem\r\n");
    xil_printf("------------\r\n");
    XV_HdmiTxSs_ReportCoreInfo(InstancePtr);
    XV_HdmiTxSs_ReportSubcoreVersion(InstancePtr);
    xil_printf("\r\n");
    xil_printf("HDMI TX timing\r\n");
    xil_printf("------------\r\n");
    XV_HdmiTxSs_ReportTiming(InstancePtr);
    xil_printf("Audio\r\n");
    xil_printf("---------\r\n");
    XV_HdmiTxSs_ReportAudio(InstancePtr);
    xil_printf("DRM info frame\r\n");
    xil_printf("--------------\r\n");
    XV_HdmiTxSs_ReportDRMInfo(InstancePtr);
}
/*****************************************************************************/
/**
*
* This function checks if the video stream is up.
*
* @param  None.
*
* @return
*   - TRUE if stream is up.
*   - FALSE if stream is down.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_IsStreamUp(XV_HdmiTxSs *InstancePtr)
{
  /* Verify arguments. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  return (InstancePtr->IsStreamUp);
}

/*****************************************************************************/
/**
*
* This function checks if the interface is connected.
*
* @param  None.
*
* @return
*   - TRUE if the interface is connected.
*   - FALSE if the interface is not connected.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_IsStreamConnected(XV_HdmiTxSs *InstancePtr)
{
  /* Verify arguments. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  return (InstancePtr->IsStreamConnected);
}

/*****************************************************************************/
/**
*
* This function checks if the interface has toggled.
*
* @param  None.
*
* @return
*   - TRUE if the interface HPD has toggled.
*   - FALSE if the interface HPD has not toggled.
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs_IsStreamToggled(XV_HdmiTxSs *InstancePtr)
{
  /* Verify arguments. */
  Xil_AssertNonvoid(InstancePtr != NULL);

  return (InstancePtr->IsStreamToggled);
}

/*****************************************************************************/
/**
*
* This function configures the Bridge for YUV420 and repeater functionality
*
* @param InstancePtr  Instance Pointer to the main data structure
* @param None
*
* @return
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs_ConfigBridgeMode(XV_HdmiTxSs *InstancePtr) {
	XVidC_VideoStream *HdmiTxSsVidStreamPtr;
    HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(InstancePtr);

    XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
    AviInfoFramePtr = XV_HdmiTxSs_GetAviInfoframe(InstancePtr);

    // Pixel Repetition factor of 3 and above are not supported by the bridge
    if (AviInfoFramePtr->PixelRepetition > XHDMIC_PIXEL_REPETITION_FACTOR_2) {
#ifdef XV_HDMITXSS_LOG_ENABLE
    	XV_HdmiTxSs_LogWrite(InstancePtr, XV_HDMITXSS_LOG_EVT_PIX_REPEAT_ERR,
    			AviInfoFramePtr->PixelRepetition);
#endif
    	return;
    }

    if (HdmiTxSsVidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_420) {
        /*********************************************************
         * 420 Support
         *********************************************************/
         XV_HdmiTxSs_BridgePixelRepeat(InstancePtr, FALSE);
         AviInfoFramePtr->PixelRepetition = XHDMIC_PIXEL_REPETITION_FACTOR_1;
         XV_HdmiTxSs_BridgeYuv420(InstancePtr, TRUE);
    }
    else {
        if ((AviInfoFramePtr->PixelRepetition ==
					XHDMIC_PIXEL_REPETITION_FACTOR_2))
        {
            /*********************************************************
             * NTSC/PAL Support
             *********************************************************/
             XV_HdmiTxSs_BridgeYuv420(InstancePtr, FALSE);
             XV_HdmiTxSs_BridgePixelRepeat(InstancePtr, TRUE);
        }
        else {
            XV_HdmiTxSs_BridgeYuv420(InstancePtr, FALSE);
            XV_HdmiTxSs_BridgePixelRepeat(InstancePtr, FALSE);
            AviInfoFramePtr->PixelRepetition =
					XHDMIC_PIXEL_REPETITION_FACTOR_1;
        }
    }
}

/*****************************************************************************/
/**
* This function will set the default in HDF.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
* @param    Id is the XV_HdmiTxSs ID to operate on.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs_SetDefaultPpc(XV_HdmiTxSs *InstancePtr, u8 Id) {
    extern XV_HdmiTxSs_Config
#ifndef SDT
	XV_HdmiTxSs_ConfigTable[XPAR_XV_HDMITXSS_NUM_INSTANCES];
#else
	XV_HdmiTxSs_ConfigTable[];
#endif
    InstancePtr->Config.Ppc = XV_HdmiTxSs_ConfigTable[Id].Ppc;
}

/*****************************************************************************/
/**
* This function will set PPC specified by user.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
* @param    Id is the XV_HdmiTxSs ID to operate on.
* @param    Ppc is the PPC to be set.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs_SetPpc(XV_HdmiTxSs *InstancePtr, u8 Id, u8 Ppc) {
    InstancePtr->Config.Ppc = (XVidC_PixelsPerClock) Ppc;
    Id = Id; //squash unused variable compiler warning
}

/*****************************************************************************/
/**
* This function will enable the video masking.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS_MaskEnable(XV_HdmiTxSs *InstancePtr)
{
    XV_HdmiTx_MaskEnable(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
* This function will disable the video masking.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS_MaskDisable(XV_HdmiTxSs *InstancePtr)
{
    XV_HdmiTx_MaskDisable(InstancePtr->HdmiTxPtr);
}

/*****************************************************************************/
/**
* This function will enable or disable the noise in the video mask.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
* @param	Enable specifies TRUE/FALSE value to either enable or disable the
*		Noise.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS_MaskNoise(XV_HdmiTxSs *InstancePtr, u8 Enable)
{
    XV_HdmiTx_MaskNoise(InstancePtr->HdmiTxPtr, Enable);
}

static u32 XV_HdmiTxSS_GetVidMaskColorValue(XV_HdmiTxSs *InstancePtr,
											u16 Value)
{
	u32 Data;
	s8 Temp;

	Temp = InstancePtr->Config.MaxBitsPerPixel -
				InstancePtr->HdmiTxPtr->Stream.Video.ColorDepth;
	Data = Value << ((Temp > 0) ? Temp : 0);

	return Data;
}

/*****************************************************************************/
/**
* This function will set the red component in the video mask.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
* @param	Value specifies the video mask value set to red component
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS_MaskSetRed(XV_HdmiTxSs *InstancePtr, u16 Value)
{
    u32 Data;

    Data = XV_HdmiTxSS_GetVidMaskColorValue(InstancePtr, Value);

	XV_HdmiTx_MaskSetRed(InstancePtr->HdmiTxPtr, (Data));
}


/*****************************************************************************/
/**
* This function will set the green component in the video mask.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
* @param	Value specifies the video mask value set to green component
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS_MaskSetGreen(XV_HdmiTxSs *InstancePtr, u16 Value)
{
    u32 Data;

    Data = XV_HdmiTxSS_GetVidMaskColorValue(InstancePtr, Value);

	XV_HdmiTx_MaskSetGreen(InstancePtr->HdmiTxPtr, (Data));
}

/*****************************************************************************/
/**
* This function will set the blue component in the video mask.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
* @param	Value specifies the video mask value set to blue component
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS_MaskSetBlue(XV_HdmiTxSs *InstancePtr, u16 Value)
{
    u32 Data;

    Data = XV_HdmiTxSS_GetVidMaskColorValue(InstancePtr, Value);

	XV_HdmiTx_MaskSetBlue(InstancePtr->HdmiTxPtr, (Data));
}

/*****************************************************************************/
/**
* This function configures the background color for Video Masking Feature
*
* @param  InstancePtr is a pointer to the core instance to be worked on.
* @param  ColorId is the background color requested
*
* @return None
*
******************************************************************************/
void XV_HdmiTxSS_SetBackgroundColor(XV_HdmiTxSs *InstancePtr,
                                    XVMaskColorId  ColorId)
{
    u16 Cr_R_val,y_G_val,Cb_B_val;
    u16 scale;

    XVidC_ColorFormat cfmt;
	XVidC_ColorDepth bpc;

    XVidC_VideoStream *HdmiTxSsVidStreamPtr;
    HdmiTxSsVidStreamPtr = XV_HdmiTxSs_GetVideoStream(InstancePtr);

    cfmt = HdmiTxSsVidStreamPtr->ColorFormatId;
	bpc = InstancePtr->HdmiTxPtr->Stream.Video.ColorDepth;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  if(cfmt == XVIDC_CSF_RGB)
  {
    scale = ((1<<bpc)-1);
    y_G_val  = bkgndColorRGB[ColorId][0] * scale;
    Cb_B_val = bkgndColorRGB[ColorId][1] * scale;
    Cr_R_val = bkgndColorRGB[ColorId][2] * scale;
  }
  else //YUV
  {
    scale =  (1<<(bpc-XVIDC_BPC_8));
    y_G_val  = bkgndColorYUV[ColorId][0] * scale;
    Cb_B_val = bkgndColorYUV[ColorId][1] * scale;
    Cr_R_val = bkgndColorYUV[ColorId][2] * scale;
  }

  if(ColorId == XV_BKGND_NOISE) {
	XV_HdmiTxSS_MaskNoise(InstancePtr, (TRUE));
  }
  else {
	XV_HdmiTxSS_MaskNoise(InstancePtr, (FALSE));
    /* Set Background color (outside window) */
	XV_HdmiTxSS_MaskSetRed(InstancePtr,  Cr_R_val);
	XV_HdmiTxSS_MaskSetGreen(InstancePtr, y_G_val);
	XV_HdmiTxSS_MaskSetBlue(InstancePtr, Cb_B_val);
  }
  XV_HdmiTxSS_MaskEnable(InstancePtr);
}


/*****************************************************************************/
/**
* This function will get the current video mask mode
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
*
* @return   Current mode.
*       0 = Video masking is disabled
*       1 = Video masking is enabled
*
* @note     None.
*
*
******************************************************************************/
u8 XV_HdmiTxSS_IsMasked(XV_HdmiTxSs *InstancePtr)
{
    return ((u8)XV_HdmiTx_IsMasked(InstancePtr->HdmiTxPtr));
}

/*****************************************************************************/
/**
* This function will set the major and minor application version in TXSs struct
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
* @param    maj is the major version of the application.
* @param    min is the minor version of the application.
* @return   void.
*
* @note     None.
*
*
******************************************************************************/
void XV_HdmiTxSS_SetAppVersion(XV_HdmiTxSs *InstancePtr, u8 maj, u8 min)
{
	InstancePtr->AppMajVer = maj;
	InstancePtr->AppMinVer = min;
}

/*****************************************************************************/
/**
* This function will Configure Timegrid based on the tolerance value for HPD and
* Toggle event
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs core instance.
* @param    Type is the tolerance type.
* @param    ToleranceVal is the tolerance value to be applied.
* @return   void.
*
* @note     None.
*
*
******************************************************************************/
void XV_HdmiTxSS_SetHpdTolerance(XV_HdmiTxSs *InstancePtr,
				 XV_HdmiTxSs_HpdToleranceType Type,
				 u16 ToleranceVal)
{
	u32 Val;

	Val = XV_HdmiTx_GetTime1Ms(InstancePtr);

	switch (Type) {
	case XV_HDMITXSS_LEADING_TOLERANCE:
		Val += ToleranceVal;
		break;
	case XV_HDMITXSS_LAGGING_TOLERANCE:
		Val -= ToleranceVal;
		break;
	default:
		xil_printf("Unknown Tolerance type\r\n");
		return;
	}

	/* Configure Timegrid based on the tolerance value for HPD and Toggle event */
	XV_HdmiTx_WriteReg(InstancePtr->Config.BaseAddress,
			   XV_HDMITX_HPD_TIMEGRID_OFFSET, Val);
}
