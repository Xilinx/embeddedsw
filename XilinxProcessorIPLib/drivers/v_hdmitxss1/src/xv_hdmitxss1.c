/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitxss1.c
*
* This is main code of Xilinx HDMI Transmitter Subsystem device driver.
* Please see xv_hdmitxss1.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  EB   22/05/18 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_hdmitxss1.h"
#include "xv_hdmitxss1_coreinit.h"

/************************** Constant Definitions *****************************/
/* Pixel definition in 8 bit resolution in YUV color space*/
static const u8 bkgndColorYUV[XV_BKGND_LAST][3] =
{
  {  0, 128, 128}, /*Black*/
  {255, 128, 128}, /*White*/
  { 76,  85, 255}, /*Red*/
  {149,  43,  21}, /*Green*/
  { 29, 255, 107}, /*Blue*/
  {  0,   0,   0}  /*Place holder for Noise Video Mask*/
};

/* Pixel map in RGB color space*/
/* {Green, Blue, Red} */
static const u8 bkgndColorRGB[XV_BKGND_LAST][3] =
{
  {0, 0, 0}, /*Black*/
  {1, 1, 1}, /*White*/
  {0, 0, 1}, /*Red*/
  {1, 0, 0}, /*Green*/
  {0, 1, 0}, /*Blue*/
  {0, 0, 0}  /*Place holder for Noise Video Mask*/
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
  XV_HdmiTx1 HdmiTx1;
  XVtc Vtc;
}XV_HdmiTxSs1_SubCores;

/**************************** Local Global ***********************************/
XV_HdmiTxSs1_SubCores XV_HdmiTxSs1_SubCoreRepo[XPAR_XV_HDMITXSS1_NUM_INSTANCES];
                /**< Define Driver instance of all sub-core
                                    included in the design */

/************************** Function Prototypes ******************************/
static void XV_HdmiTxSs1_GetIncludedSubcores(XV_HdmiTxSs1 *HdmiTxSs1Ptr,
                                            u16 DevId);
static int XV_HdmiTxSs1_RegisterSubsysCallbacks(XV_HdmiTxSs1 *InstancePtr);
static int XV_HdmiTxSs1_VtcSetup(XV_HdmiTxSs1 *HdmiTxSs1Ptr);
static u64 XV_HdmiTxSS1_SetTMDS(XV_HdmiTxSs1 *InstancePtr);
static void XV_HdmiTxSs1_ConnectCallback(void *CallbackRef);
static void XV_HdmiTxSs1_ToggleCallback(void *CallbackRef);
static void XV_HdmiTxSs1_BrdgLockedCallback(void *CallbackRef);
static void XV_HdmiTxSs1_BrdgUnlockedCallback(void *CallbackRef);
static void XV_HdmiTxSs1_BrdgOverflowCallback(void *CallbackRef);
static void XV_HdmiTxSs1_BrdgUnderflowCallback(void *CallbackRef);
static void XV_HdmiTxSs1_VsCallback(void *CallbackRef);
static void XV_HdmiTxSs1_StreamUpCallback(void *CallbackRef);
static void XV_HdmiTxSs1_StreamDownCallback(void *CallbackRef);
static void XV_HdmiTxSs1_ErrorCallback(void *CallbackRef);
static void XV_HdmiTxSs1_FrlLtsLCallback(void *CallbackRef);
static void XV_HdmiTxSs1_FrlLts1Callback(void *CallbackRef);
static void XV_HdmiTxSs1_FrlLts2Callback(void *CallbackRef);
static void XV_HdmiTxSs1_FrlLts3Callback(void *CallbackRef);
static void XV_HdmiTxSs1_FrlLts4Callback(void *CallbackRef);
static void XV_HdmiTxSs1_FrlLtsPCallback(void *CallbackRef);
static void XV_HdmiTxSs1_CedUpdateCallback(void *CallbackRef);

static u32 XV_HdmiTxSS1_GetVidMaskColorValue(XV_HdmiTxSs1 *InstancePtr,
											u16 Value);

static void XV_HdmiTxSs1_ConfigBridgeMode(XV_HdmiTxSs1 *InstancePtr);

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
* This macro selects the bridge YUV420 mode
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem
*
*****************************************************************************/
#define XV_HdmiTxSs1_BridgeYuv420(InstancePtr, Enable) \
{ \
    XV_HdmiTx1_Bridge_yuv420(InstancePtr->HdmiTx1Ptr, Enable); \
}

/*****************************************************************************/
/**
* This macro selects the bridge pixel repeat mode
*
* @param  InstancePtr is a pointer to the HDMI TX Subsystem
*
*****************************************************************************/
#define XV_HdmiTxSs1_BridgePixelRepeat(InstancePtr, Enable) \
{ \
    XV_HdmiTx1_Bridge_pixel(InstancePtr->HdmiTx1Ptr, Enable); \
}
/************************** Function Definition ******************************/

/*****************************************************************************/
/**
 * This function sets the core into HDMI mode
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem
 *
 *****************************************************************************/
void XV_HdmiTxSS1_SetHdmiFrlMode(XV_HdmiTxSs1 *InstancePtr)
{
    XV_HdmiTx1_SetHdmiFrlMode(InstancePtr->HdmiTx1Ptr);

#ifdef XPAR_XHDCP_NUM_INSTANCES
    if (InstancePtr->Hdcp14Ptr) {
        XHdcp1x_SetHdmiMode(InstancePtr->Hdcp14Ptr, TRUE);
    }
#endif
}

/*****************************************************************************/
/**
 * This function sets the core into HDMI mode
 *
 * @param  InstancePtr is a pointer to the HDMI TX Subsystem
 *
 *****************************************************************************/
void XV_HdmiTxSS1_SetHdmiTmdsMode(XV_HdmiTxSs1 *InstancePtr)
{
    XV_HdmiTx1_SetHdmiTmdsMode(InstancePtr->HdmiTx1Ptr);

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
void XV_HdmiTxSS1_SetDviMode(XV_HdmiTxSs1 *InstancePtr)
{
    XV_HdmiTx1_SetDviMode(InstancePtr->HdmiTx1Ptr);

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
void XV_HdmiTxSs1_ReportCoreInfo(XV_HdmiTxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\r\n  ->HDMI TX Subsystem Cores\r\n");

	/* Report all the included cores in the subsystem instance */
	if (InstancePtr->HdmiTx1Ptr) {
		xil_printf("    : HDMI TX \r\n");
	}

	if (XV_HdmiTx1_ReadReg(InstancePtr->HdmiTx1Ptr->Config.BaseAddress,
							(XV_HDMITX1_PIO_IN_OFFSET)) &
								XV_HDMITX1_PIO_IN_VID_RDY_MASK) {
		if (InstancePtr->VtcPtr) {
			xil_printf("    : VTC Core \r\n");
		}
	} else {
		xil_printf("No VTC Configuration, No Video Clock!\r\n");
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
void XV_HdmiTxSS1_HdmiTx1IntrHandler(XV_HdmiTxSs1 *InstancePtr)
{
    XV_HdmiTx1_IntrHandler(InstancePtr->HdmiTx1Ptr);
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
static int XV_HdmiTxSs1_RegisterSubsysCallbacks(XV_HdmiTxSs1 *InstancePtr)
{
  XV_HdmiTxSs1 *HdmiTxSs1Ptr = InstancePtr;

  /** Register HDMI callbacks */
  if (HdmiTxSs1Ptr->HdmiTx1Ptr) {
    /*
     * Register call back for Tx Core Interrupts.
     */
    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
                          XV_HDMITX1_HANDLER_CONNECT,
						  (void *)XV_HdmiTxSs1_ConnectCallback,
						  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
                          XV_HDMITX1_HANDLER_TOGGLE,
						  (void *)XV_HdmiTxSs1_ToggleCallback,
						  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
                          XV_HDMITX1_HANDLER_BRDGLOCK,
						  (void *)XV_HdmiTxSs1_BrdgLockedCallback,
						  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
                          XV_HDMITX1_HANDLER_BRDGUNLOCK,
						  (void *)XV_HdmiTxSs1_BrdgUnlockedCallback,
						  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
                          XV_HDMITX1_HANDLER_BRDGOVERFLOW,
						  (void *)XV_HdmiTxSs1_BrdgOverflowCallback,
						  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
                          XV_HDMITX1_HANDLER_BRDGUNDERFLOW,
						  (void *)XV_HdmiTxSs1_BrdgUnderflowCallback,
						  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
                          XV_HDMITX1_HANDLER_VS,
						  (void *)XV_HdmiTxSs1_VsCallback,
						  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
                          XV_HDMITX1_HANDLER_STREAM_UP,
						  (void *)XV_HdmiTxSs1_StreamUpCallback,
						  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
                          XV_HDMITX1_HANDLER_STREAM_DOWN,
						  (void *)XV_HdmiTxSs1_StreamDownCallback,
						  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_FRL_CONFIG,
    					  (void *)XV_HdmiTxSs1_FrlConfigCallback,
    					  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_FRL_FFE,
    					  (void *)XV_HdmiTxSs1_FrlFfeCallback,
    					  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_FRL_START,
    					  (void *)XV_HdmiTxSs1_FrlStartCallback,
    					  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_FRL_STOP,
    					  (void *)XV_HdmiTxSs1_FrlStopCallback,
    					  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_TMDS_CONFIG,
    					  (void *)XV_HdmiTxSs1_TmdsConfigCallback,
    					  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_FRL_LTSL,
    					  (void *)XV_HdmiTxSs1_FrlLtsLCallback,
    					  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_FRL_LTS1,
    					  (void *)XV_HdmiTxSs1_FrlLts1Callback,
    					  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_FRL_LTS2,
    					  (void *)XV_HdmiTxSs1_FrlLts2Callback,
    					  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_FRL_LTS3,
    					  (void *)XV_HdmiTxSs1_FrlLts3Callback,
    					  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_FRL_LTS4,
    					  (void *)XV_HdmiTxSs1_FrlLts4Callback,
    					  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_FRL_LTSP,
    					  (void *)XV_HdmiTxSs1_FrlLtsPCallback,
    					  (void *)InstancePtr);

    XV_HdmiTx1_SetCallback(HdmiTxSs1Ptr->HdmiTx1Ptr,
    					  XV_HDMITX1_HANDLER_CED_UPDATE,
    					  (void *)XV_HdmiTxSs1_CedUpdateCallback,
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
* @param  HdmiTxSs1Ptr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
static void XV_HdmiTxSs1_GetIncludedSubcores(XV_HdmiTxSs1 *HdmiTxSs1Ptr, u16 DevId)
{
  HdmiTxSs1Ptr->HdmiTx1Ptr     = ((HdmiTxSs1Ptr->Config.HdmiTx1.IsPresent)    \
                        ? (&XV_HdmiTxSs1_SubCoreRepo[DevId].HdmiTx1) : NULL);
  HdmiTxSs1Ptr->VtcPtr        = ((HdmiTxSs1Ptr->Config.Vtc.IsPresent)  \
                        ? (&XV_HdmiTxSs1_SubCoreRepo[DevId].Vtc) : NULL);
#ifdef XPAR_XHDCP_NUM_INSTANCES
  /* HDCP 1.4*/
  HdmiTxSs1Ptr->Hdcp14Ptr       = ((HdmiTxSs1Ptr->Config.Hdcp14.IsPresent) \
                        ? (&XV_HdmiTxSs1_SubCoreRepo[DevId].Hdcp14) : NULL);
  HdmiTxSs1Ptr->HdcpTimerPtr  = ((HdmiTxSs1Ptr->Config.HdcpTimer.IsPresent) \
                        ? (&XV_HdmiTxSs1_SubCoreRepo[DevId].HdcpTimer) : NULL);
#endif
#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
  /* HDCP 2.2*/
  HdmiTxSs1Ptr->Hdcp22Ptr       = ((HdmiTxSs1Ptr->Config.Hdcp22.IsPresent) \
                        ? (&XV_HdmiTxSs1_SubCoreRepo[DevId].Hdcp22) : NULL);
#endif
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
int XV_HdmiTxSs1_CfgInitialize(XV_HdmiTxSs1 *InstancePtr,
                              XV_HdmiTxSs1_Config *CfgPtr,
                              UINTPTR EffectiveAddr)
{
  XV_HdmiTxSs1 *HdmiTxSs1Ptr = InstancePtr;

  /* Verify arguments */
  Xil_AssertNonvoid(HdmiTxSs1Ptr != NULL);
  Xil_AssertNonvoid(CfgPtr != NULL);
  Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)NULL);

  /* Setup the instance */
  memcpy((void *)&(HdmiTxSs1Ptr->Config), (const void *)CfgPtr,
    sizeof(XV_HdmiTxSs1_Config));
  HdmiTxSs1Ptr->Config.BaseAddress = EffectiveAddr;

  /* Initialize InfoFrame */
  (void)memset((void *)&(HdmiTxSs1Ptr->AVIInfoframe), 0, sizeof(XHdmiC_AVI_InfoFrame));
  (void)memset((void *)&(HdmiTxSs1Ptr->VSIF), 0, sizeof(XHdmiC_VSIF));
  (void)memset((void *)&(HdmiTxSs1Ptr->AudioInfoframe), 0, sizeof(XHdmiC_AudioInfoFrame));

  /* Determine sub-cores included in the provided instance of subsystem */
  XV_HdmiTxSs1_GetIncludedSubcores(HdmiTxSs1Ptr, CfgPtr->DeviceId);

  /* Initialize all included sub_cores */

  /* HDCP 1.4*/
#ifdef XPAR_XHDCP_NUM_INSTANCES
  if (HdmiTxSs1Ptr->Hdcp14Ptr) {
    if (XV_HdmiTxSs1_SubcoreInitHdcp14(HdmiTxSs1Ptr) != XST_SUCCESS){
      return(XST_FAILURE);
    }
  }

  if (HdmiTxSs1Ptr->HdcpTimerPtr) {
    if (XV_HdmiTxSs1_SubcoreInitHdcpTimer(HdmiTxSs1Ptr) != XST_SUCCESS){
      return(XST_FAILURE);
    }
  }
#endif

  if (HdmiTxSs1Ptr->HdmiTx1Ptr) {
    if (XV_HdmiTxSs1_SubcoreInitHdmiTx1(HdmiTxSs1Ptr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
    XV_HdmiTx1_SetAxiClkFreq(HdmiTxSs1Ptr->HdmiTx1Ptr,
                            HdmiTxSs1Ptr->Config.AxiLiteClkFreq);
  }

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
  /* HDCP 2.2*/
  if (HdmiTxSs1Ptr->Hdcp22Ptr) {
    if (XV_HdmiTxSs1_SubcoreInitHdcp22(HdmiTxSs1Ptr) != XST_SUCCESS){
      return(XST_FAILURE);
    }
  }
#endif

  if (HdmiTxSs1Ptr->VtcPtr) {
    if (XV_HdmiTxSs1_SubcoreInitVtc(HdmiTxSs1Ptr) != XST_SUCCESS) {
      return(XST_FAILURE);
    }
  }

  /* Register Callbacks */
  XV_HdmiTxSs1_RegisterSubsysCallbacks(HdmiTxSs1Ptr);

  /* Set default HDCP protocol */
  HdmiTxSs1Ptr->HdcpProtocol = XV_HDMITXSS1_HDCP_NONE;

  /* HDCP ready flag */

#ifdef USE_HDCP_TX
  /* Default value */
  HdmiTxSs1Ptr->HdcpIsReady = (FALSE);
  XV_HdmiTxSs1_HdcpSetCapability(HdmiTxSs1Ptr, XV_HDMITXSS1_HDCP_BOTH);
#endif

#if defined(XPAR_XHDCP_NUM_INSTANCES) && defined(XPAR_XHDCP22_TX_NUM_INSTANCES)
  /* HDCP is ready when both HDCP cores are instantiated and both keys
     are loaded */
  if (HdmiTxSs1Ptr->Hdcp14Ptr && HdmiTxSs1Ptr->Hdcp22Ptr &&
      HdmiTxSs1Ptr->Hdcp22Lc128Ptr && HdmiTxSs1Ptr->Hdcp22SrmPtr &&
      HdmiTxSs1Ptr->Hdcp14KeyPtr) {
    HdmiTxSs1Ptr->HdcpIsReady = (TRUE);
  }
#endif

#if defined(XPAR_XHDCP_NUM_INSTANCES)
  /* HDCP is ready when only the HDCP 1.4 core is instantiated and the key
     is loaded */
  if (!HdmiTxSs1Ptr->HdcpIsReady && HdmiTxSs1Ptr->Hdcp14Ptr &&
       HdmiTxSs1Ptr->Hdcp14KeyPtr) {
    HdmiTxSs1Ptr->HdcpIsReady = (TRUE);
  }
#endif

#if defined(XPAR_XHDCP22_TX_NUM_INSTANCES)
  /* HDCP is ready when only the HDCP 2.2 core is instantiated and the key
     is loaded */
  if (!HdmiTxSs1Ptr->HdcpIsReady && HdmiTxSs1Ptr->Hdcp22Ptr &&
       HdmiTxSs1Ptr->Hdcp22Lc128Ptr &&
      HdmiTxSs1Ptr->Hdcp22SrmPtr) {
    HdmiTxSs1Ptr->HdcpIsReady = (TRUE);
  }
#endif

  /* Set the flag to indicate the subsystem is ready */
  XV_HdmiTxSs1_Reset(HdmiTxSs1Ptr);
  HdmiTxSs1Ptr->IsReady = XIL_COMPONENT_IS_READY;

  /* Initialize the application version with 0 <default value>.
   * Application need to set the this variable properly to let driver know
   * what version of application is being used.
   */
  HdmiTxSs1Ptr->AppMajVer = 0;
  HdmiTxSs1Ptr->AppMinVer = 0;

  /* Disable the Logging */
  HdmiTxSs1Ptr->EnableHDCPLogging = (FALSE);
  HdmiTxSs1Ptr->EnableHDMILogging = (FALSE);

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
void XV_HdmiTxSs1_Start(XV_HdmiTxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

#ifdef XV_HDMITXSS1_LOG_ENABLE
  XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_START, 0);
#endif

  XV_HdmiTx1_Start(InstancePtr->HdmiTx1Ptr);
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
void XV_HdmiTxSs1_Stop(XV_HdmiTxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  if (InstancePtr->VtcPtr) {
    /* Disable VTC */
    XVtc_DisableGenerator(InstancePtr->VtcPtr);
  }

  XV_HdmiTx1_Stop(InstancePtr->HdmiTx1Ptr);

#ifdef XV_HDMITXSS1_LOG_ENABLE
  XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_STOP, 0);
#endif
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
void XV_HdmiTxSs1_Reset(XV_HdmiTxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);
#ifdef XV_HDMITXSS1_LOG_ENABLE
  XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_RESET, 0);
#endif
  /* Assert HDMI TXCore resets */
  XV_HdmiTxSs1_TXCore_LRST(InstancePtr, TRUE);
  XV_HdmiTxSs1_TXCore_VRST(InstancePtr, TRUE);

  /* Assert VID_OUT bridge resets */
  XV_HdmiTxSs1_SYSRST(InstancePtr, TRUE);
  XV_HdmiTxSs1_VRST(InstancePtr, TRUE);

  /* Release VID_IN bridge resets */
  XV_HdmiTxSs1_SYSRST(InstancePtr, FALSE);
  XV_HdmiTxSs1_VRST(InstancePtr, FALSE);

  /* Release HDMI TXCore resets */
  XV_HdmiTxSs1_TXCore_LRST(InstancePtr, FALSE);
  XV_HdmiTxSs1_TXCore_VRST(InstancePtr, FALSE);
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
void XV_HdmiTxSs1_TXCore_VRST(XV_HdmiTxSs1 *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx1_INT_VRST(InstancePtr->HdmiTx1Ptr, Reset);
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
void XV_HdmiTxSs1_TXCore_LRST(XV_HdmiTxSs1 *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx1_INT_LRST(InstancePtr->HdmiTx1Ptr, Reset);
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
void XV_HdmiTxSs1_VRST(XV_HdmiTxSs1 *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx1_EXT_VRST(InstancePtr->HdmiTx1Ptr, Reset);
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
void XV_HdmiTxSs1_SYSRST(XV_HdmiTxSs1 *InstancePtr, u8 Reset)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx1_EXT_SYSRST(InstancePtr->HdmiTx1Ptr, Reset);
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
void XV_HdmiTxSs1_SetGcpAvmuteBit(XV_HdmiTxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx1_SetGcpAvmuteBit(InstancePtr->HdmiTx1Ptr);
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
void XV_HdmiTxSs1_ClearGcpAvmuteBit(XV_HdmiTxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx1_ClearGcpAvmuteBit(InstancePtr->HdmiTx1Ptr);
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
void XV_HdmiTxSs1_SetGcpClearAvmuteBit(XV_HdmiTxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx1_SetGcpClearAvmuteBit(InstancePtr->HdmiTx1Ptr);
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
void XV_HdmiTxSs1_ClearGcpClearAvmuteBit(XV_HdmiTxSs1 *InstancePtr)
{
  Xil_AssertVoid(InstancePtr != NULL);

  XV_HdmiTx1_ClearGcpClearAvmuteBit(InstancePtr->HdmiTx1Ptr);
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
static int XV_HdmiTxSs1_VtcSetup(XV_HdmiTxSs1 *HdmiTxSs1Ptr)
{
  /* Polarity configuration */
  XVtc_Polarity Polarity;
  XVtc_SourceSelect SourceSelect;
  XVtc_Timing VideoTiming;
  u32 HdmiTx1_Hblank;
  u32 Vtc_Hblank;

  /* Disable Generator */
  XVtc_Reset(HdmiTxSs1Ptr->VtcPtr);
  XVtc_DisableGenerator(HdmiTxSs1Ptr->VtcPtr);
  XVtc_Disable(HdmiTxSs1Ptr->VtcPtr);

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

  XVtc_SetSource(HdmiTxSs1Ptr->VtcPtr, &SourceSelect);

  VideoTiming.HActiveVideo = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.HActive;
  VideoTiming.HFrontPorch = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.HFrontPorch;
  VideoTiming.HSyncWidth = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.HSyncWidth;
  VideoTiming.HBackPorch = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.HBackPorch;
  VideoTiming.HSyncPolarity = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.HSyncPolarity;

  /* Vertical Timing */
  VideoTiming.VActiveVideo = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.VActive;

  VideoTiming.V0FrontPorch = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.F0PVFrontPorch;
  VideoTiming.V0BackPorch = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.F0PVBackPorch;
  VideoTiming.V0SyncWidth = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.F0PVSyncWidth;

  VideoTiming.V1FrontPorch = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.F1VFrontPorch;
  VideoTiming.V1SyncWidth = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.F1VSyncWidth;
  VideoTiming.V1BackPorch = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.F1VBackPorch;

  VideoTiming.VSyncPolarity = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.VSyncPolarity;

  VideoTiming.Interlaced = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.IsInterlaced;

    /* 4 pixels per clock */
    if (HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.PixPerClk == XVIDC_PPC_4) {
    	/* If the parameters below are not divisible by the current PPC setting,
    	 * log an error as VTC does not support such video timing
    	 */
		if (VideoTiming.HActiveVideo & 0x3 || VideoTiming.HFrontPorch & 0x3 ||
				VideoTiming.HBackPorch & 0x3 || VideoTiming.HSyncWidth & 0x3) {
#ifdef XV_HDMITXSS1_LOG_ENABLE
				XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr,
						XV_HDMITXSS1_LOG_EVT_VTC_RES_ERR, 0);
#endif
				XV_HdmiTxSs1_ErrorCallback(HdmiTxSs1Ptr);
		}
		VideoTiming.HActiveVideo = VideoTiming.HActiveVideo/4;
		VideoTiming.HFrontPorch = VideoTiming.HFrontPorch/4;
		VideoTiming.HBackPorch = VideoTiming.HBackPorch/4;
		VideoTiming.HSyncWidth = VideoTiming.HSyncWidth/4;
    }

    /* 2 pixels per clock */
    else if (HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.PixPerClk == XVIDC_PPC_2) {
    	/* If the parameters below are not divisible by the current PPC setting,
    	 * log an error as VTC does not support such video timing
    	 */
		if (VideoTiming.HActiveVideo & 0x1 || VideoTiming.HFrontPorch & 0x1 ||
				VideoTiming.HBackPorch & 0x1 || VideoTiming.HSyncWidth & 0x1) {
#ifdef XV_HDMITXSS1_LOG_ENABLE
			XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr,
					XV_HDMITXSS1_LOG_EVT_VTC_RES_ERR, 0);
#endif
			XV_HdmiTxSs1_ErrorCallback(HdmiTxSs1Ptr);
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
    if (HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.ColorFormatId == XVIDC_CSF_YCRCB_420) {
    	/* If the parameters below are not divisible by the current PPC setting,
    	 * log an error as VTC does not support such video timing
    	 */
		if (VideoTiming.HActiveVideo & 0x1 || VideoTiming.HFrontPorch & 0x1 ||
				VideoTiming.HBackPorch & 0x1 || VideoTiming.HSyncWidth & 0x1) {
#ifdef XV_HDMITXSS1_LOG_ENABLE
			XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr,
					XV_HDMITXSS1_LOG_EVT_VTC_RES_ERR, 0);
#endif
			XV_HdmiTxSs1_ErrorCallback(HdmiTxSs1Ptr);
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

  HdmiTx1_Hblank = HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.HFrontPorch +
    HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.HSyncWidth +
    HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.Timing.HBackPorch;

  do {
    /* Calculate vtc horizontal blanking*/
    Vtc_Hblank = VideoTiming.HFrontPorch +
        VideoTiming.HBackPorch +
        VideoTiming.HSyncWidth;

    /* Quad pixel mode*/
    if (HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.PixPerClk == XVIDC_PPC_4) {
      Vtc_Hblank *= 4;
    }

    /* Dual pixel mode*/
    else if (HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.PixPerClk == XVIDC_PPC_2) {
      Vtc_Hblank *= 2;
    }

    /* Single pixel mode*/
    else {
      /*Vtc_Hblank *= 1;*/
    }

    /* For YUV420 the line width is double there for double the blanking */
    if (HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.ColorFormatId == XVIDC_CSF_YCRCB_420) {
        Vtc_Hblank *= 2;
    }

    /* If the horizontal total blanking differs,*/
    /* then increment the Vtc horizontal front porch.*/
    if (Vtc_Hblank != HdmiTx1_Hblank) {
      VideoTiming.HFrontPorch++;
    }

  } while (Vtc_Hblank < HdmiTx1_Hblank);

  if (Vtc_Hblank != HdmiTx1_Hblank) {
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "Error! Current format with total Hblank (%d) cannot \r\n",
                  HdmiTx1_Hblank);
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "       be transmitted with pixels per clock = %d\r\n",
                  HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Video.PixPerClk);
      return (XST_FAILURE);
  }

  XVtc_SetGeneratorTiming(HdmiTxSs1Ptr->VtcPtr, &VideoTiming);

  /* Set up Polarity of all outputs */
  memset((void *)&Polarity, 0, sizeof(XVtc_Polarity));
  Polarity.ActiveChromaPol = 1;
  Polarity.ActiveVideoPol = 1;

  /*Polarity.FieldIdPol = 0;*/
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

  XVtc_SetPolarity(HdmiTxSs1Ptr->VtcPtr, &Polarity);

  /* VTC driver does not take care of the setting of the VTC in
   * interlaced operation. As a work around the register
   * is set manually */
  if (VideoTiming.Interlaced) {
    /* Interlaced mode */
    XVtc_WriteReg(HdmiTxSs1Ptr->VtcPtr->Config.BaseAddress, 0x68, 0x42);
  }
  else {
    /* Progressive mode */
    XVtc_WriteReg(HdmiTxSs1Ptr->VtcPtr->Config.BaseAddress, 0x68, 0x2);
  }

  /* Enable generator module */
  XVtc_Enable(HdmiTxSs1Ptr->VtcPtr);
  XVtc_EnableGenerator(HdmiTxSs1Ptr->VtcPtr);
  XVtc_RegUpdateEnable(HdmiTxSs1Ptr->VtcPtr);

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
static u64 XV_HdmiTxSS1_SetTMDS(XV_HdmiTxSs1 *InstancePtr)
{
    u64 TmdsClk;

    TmdsClk = XV_HdmiTx1_GetTmdsClk(InstancePtr->HdmiTx1Ptr);

    /* Store TMDS clock for future reference */
	InstancePtr->HdmiTx1Ptr->Stream.TMDSClock = TmdsClk;

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
static void XV_HdmiTxSs1_ConnectCallback(void *CallbackRef)
{
  XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

  /* Is the cable connected */
  if (XV_HdmiTx1_IsStreamConnected(HdmiTxSs1Ptr->HdmiTx1Ptr)) {
#ifdef XV_HDMITXSS1_LOG_ENABLE
    XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_CONNECT, 0);
#endif

    /* Reset DDC */
    XV_HdmiTx1_DdcDisable(HdmiTxSs1Ptr->HdmiTx1Ptr);

    /* Set stream connected flag */
    HdmiTxSs1Ptr->IsStreamConnected = (TRUE);

#ifdef USE_HDCP_TX
    /* Push connect event to the HDCP event queue */
    XV_HdmiTxSs1_HdcpPushEvent(HdmiTxSs1Ptr, XV_HDMITXSS1_HDCP_CONNECT_EVT);
#endif
  }

  /* TX cable is disconnected */
  else {
#ifdef XV_HDMITXSS1_LOG_ENABLE
    XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_DISCONNECT, 0);
#endif
    /* Assert HDMI TXCore link reset */
    XV_HdmiTxSs1_TXCore_LRST(HdmiTxSs1Ptr, TRUE);
    XV_HdmiTxSs1_TXCore_VRST(HdmiTxSs1Ptr, TRUE);

    /* Assert SYSCLK VID_OUT bridge reset */
    XV_HdmiTxSs1_SYSRST(HdmiTxSs1Ptr, TRUE);
    XV_HdmiTxSs1_VRST(HdmiTxSs1Ptr, TRUE);

    /* Reset DDC */
    XV_HdmiTx1_DdcDisable(HdmiTxSs1Ptr->HdmiTx1Ptr);

    /* Set stream connected flag */
    HdmiTxSs1Ptr->IsStreamConnected = (FALSE);

#ifdef USE_HDCP_TX
    /* Push disconnect event to the HDCP event queue */
    XV_HdmiTxSs1_HdcpPushEvent(HdmiTxSs1Ptr, XV_HDMITXSS1_HDCP_DISCONNECT_EVT);
#endif
  }

  /* Check if user callback has been registered */
  if (HdmiTxSs1Ptr->ConnectCallback) {
    HdmiTxSs1Ptr->ConnectCallback(HdmiTxSs1Ptr->ConnectRef);
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
static void XV_HdmiTxSs1_ToggleCallback(void *CallbackRef)
{
  XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

  /* Reset DDC */
  XV_HdmiTx1_DdcDisable(HdmiTxSs1Ptr->HdmiTx1Ptr);

  /* Set toggle flag */
  HdmiTxSs1Ptr->IsStreamToggled = TRUE;
#ifdef XV_HDMITXSS1_LOG_ENABLE
  XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_TOGGLE, 0);
#endif
  /* Check if user callback has been registered */
  if (HdmiTxSs1Ptr->ToggleCallback) {
    HdmiTxSs1Ptr->ToggleCallback(HdmiTxSs1Ptr->ToggleRef);
  }

  /* Clear toggle flag */
  HdmiTxSs1Ptr->IsStreamToggled = FALSE;
}

/*****************************************************************************/
/**
*
* This function is called when a bridge locked has occurred.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs1_BrdgLockedCallback(void *CallbackRef)
{
  XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
  XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_BRDG_LOCKED, 0);
#endif

  /* Check if user callback has been registered */
  if (HdmiTxSs1Ptr->BrdgLockedCallback) {
      HdmiTxSs1Ptr->BrdgLockedCallback(HdmiTxSs1Ptr->BrdgLockedRef);
  }
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
static void XV_HdmiTxSs1_BrdgUnlockedCallback(void *CallbackRef)
{
  XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
  XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_BRDG_UNLOCKED, 0);
#endif

  /* Check if user callback has been registered */
  if (HdmiTxSs1Ptr->BrdgUnlockedCallback) {
      HdmiTxSs1Ptr->BrdgUnlockedCallback(HdmiTxSs1Ptr->BrdgUnlockedRef);
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
static void XV_HdmiTxSs1_BrdgOverflowCallback(void *CallbackRef)
{
  XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

  xdbg_printf(XDBG_DEBUG_GENERAL,
              "\r\nWarning: TX Bridge Overflow\r\n");

  /* Check if user callback has been registered */
  if (HdmiTxSs1Ptr->BrdgOverflowCallback) {
      HdmiTxSs1Ptr->BrdgOverflowCallback(HdmiTxSs1Ptr->BrdgOverflowRef);
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
static void XV_HdmiTxSs1_BrdgUnderflowCallback(void *CallbackRef)
{
  XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

  xdbg_printf(XDBG_DEBUG_GENERAL,
              "\r\nWarning: TX Bridge Underflow\r\n");

  /* Check if user callback has been registered */
  if (HdmiTxSs1Ptr->BrdgUnderflowCallback) {
      HdmiTxSs1Ptr->BrdgUnderflowCallback(HdmiTxSs1Ptr->BrdgUnderflowRef);
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
static void XV_HdmiTxSs1_VsCallback(void *CallbackRef)
{
  XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

  /* Check if user callback has been registered*/
  if (HdmiTxSs1Ptr->VsCallback) {
      HdmiTxSs1Ptr->VsCallback(HdmiTxSs1Ptr->VsRef);
  }
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
static void XV_HdmiTxSs1_StreamUpCallback(void *CallbackRef)
{
  XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

  /* Set stream up flag */
  HdmiTxSs1Ptr->IsStreamUp = (TRUE);

#ifdef USE_HDCP_TX
  /* Push the stream-up event to the HDCP event queue */
  XV_HdmiTxSs1_HdcpPushEvent(HdmiTxSs1Ptr, XV_HDMITXSS1_HDCP_STREAMUP_EVT);
#endif

  /* Check if user callback has been registered.
     User may change the video stream properties in the callback;
     therefore, execute the callback before changing stream settings. */
  if (HdmiTxSs1Ptr->StreamUpCallback) {
      HdmiTxSs1Ptr->StreamUpCallback(HdmiTxSs1Ptr->StreamUpRef);
  }

  /* Set TX sample rate */
  XV_HdmiTx1_SetSampleRate(HdmiTxSs1Ptr->HdmiTx1Ptr, HdmiTxSs1Ptr->SamplingRate);

  /* Release VID_IN bridge resets */
  XV_HdmiTxSs1_SYSRST(HdmiTxSs1Ptr, FALSE);
  XV_HdmiTxSs1_VRST(HdmiTxSs1Ptr, FALSE);

  /* Release HDMI TXCore resets */
  XV_HdmiTxSs1_TXCore_LRST(HdmiTxSs1Ptr, FALSE);
  XV_HdmiTxSs1_TXCore_VRST(HdmiTxSs1Ptr, FALSE);

  if (HdmiTxSs1Ptr->VtcPtr) {
	 if (XV_HdmiTx1_ReadReg(HdmiTxSs1Ptr->HdmiTx1Ptr->Config.BaseAddress,
						  (XV_HDMITX1_PIO_IN_OFFSET)) &
						 XV_HDMITX1_PIO_IN_VID_RDY_MASK) {
		/* Setup VTC */
		XV_HdmiTxSs1_VtcSetup(HdmiTxSs1Ptr);
	 } else {
		 xil_printf("No Vid Clk!\r\n");
	 }
  }

  if (HdmiTxSs1Ptr->AudioEnabled) {
      /* HDMI TX unmute audio */
      HdmiTxSs1Ptr->AudioMute = (FALSE);
      XV_HdmiTx1_AudioEnable(HdmiTxSs1Ptr->HdmiTx1Ptr);
  }

  /* Configure video bridge mode according to HW setting and video format */
  XV_HdmiTxSs1_ConfigBridgeMode(HdmiTxSs1Ptr);
#ifdef XV_HDMITXSS1_LOG_ENABLE
  XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_STREAMUP, 0);
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
static void XV_HdmiTxSs1_StreamDownCallback(void *CallbackRef)
{
  XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;
  /* Assert HDMI TXCore link reset */
  XV_HdmiTxSs1_TXCore_LRST(HdmiTxSs1Ptr, TRUE);
  XV_HdmiTxSs1_TXCore_VRST(HdmiTxSs1Ptr, TRUE);

  /* Assert SYSCLK VID_OUT bridge reset */
  XV_HdmiTxSs1_SYSRST(HdmiTxSs1Ptr, TRUE);
  XV_HdmiTxSs1_VRST(HdmiTxSs1Ptr, TRUE);

  /* Reset DDC */
  XV_HdmiTx1_DdcDisable(HdmiTxSs1Ptr->HdmiTx1Ptr);

  /* Set stream up flag */
  HdmiTxSs1Ptr->IsStreamUp = (FALSE);
#ifdef XV_HDMITXSS1_LOG_ENABLE
  XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_STREAMDOWN, 0);
#endif
#ifdef USE_HDCP_TX
  /* Push the stream-down event to the HDCP event queue */
  XV_HdmiTxSs1_HdcpPushEvent(HdmiTxSs1Ptr, XV_HDMITXSS1_HDCP_STREAMDOWN_EVT);
#endif

  /* Check if user callback has been registered */
  if (HdmiTxSs1Ptr->StreamDownCallback) {
      HdmiTxSs1Ptr->StreamDownCallback(HdmiTxSs1Ptr->StreamDownRef);
  }
}

/*****************************************************************************/
/**
* This function is called whenever user needs to be informed of an error
* condition.
*
* @param  None.
*
* @return None.
*
* @note   None.
*
******************************************************************************/
static void XV_HdmiTxSs1_ErrorCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

	if (HdmiTxSs1Ptr->ErrorCallback != NULL) {
		HdmiTxSs1Ptr->ErrorCallback(HdmiTxSs1Ptr->ErrorRef);
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
* (XV_HDMITXSS1_HANDLER_CONNECT)       HpdCallback
* (XV_HDMITXSS1_HANDLER_VS)            VsCallback
* (XV_HDMITXSS1_HANDLER_STREAM_DOWN)   StreamDownCallback
* (XV_HDMITXSS1_HANDLER_STREAM_UP)     StreamUpCallback
* (XV_HDMITXSS1_HANDLER_BRDGOVERFLOW)  BrdgOverflowCallback
* (XV_HDMITXSS1_HANDLER_BRDGUNDERFLOW) BrdgUnderflowCallback
* (XV_HDMITXSS1_HANDLER_HDCP_AUTHENTICATED)
* (XV_HDMITXSS1_HANDLER_HDCP_DOWNSTREAM_TOPOLOGY_AVAILABLE)
* (XV_HDMITXSS1_HANDLER_HDCP_UNAUTHENTICATED)
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
int XV_HdmiTxSs1_SetCallback(XV_HdmiTxSs1 *InstancePtr,
		XV_HdmiTxSs1_HandlerType HandlerType,
		void *CallbackFunc,
		void *CallbackRef)
{
    u32 Status;

    /* Verify arguments. */
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(HandlerType >= (XV_HDMITXSS1_HANDLER_CONNECT));
    Xil_AssertNonvoid(CallbackFunc != NULL);
    Xil_AssertNonvoid(CallbackRef != NULL);

    /* Check for handler type */
    switch (HandlerType) {
        /* Connect*/
        case (XV_HDMITXSS1_HANDLER_CONNECT):
            InstancePtr->ConnectCallback = (XV_HdmiTxSs1_Callback)CallbackFunc;
            InstancePtr->ConnectRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Toggle*/
        case (XV_HDMITXSS1_HANDLER_TOGGLE):
            InstancePtr->ToggleCallback = (XV_HdmiTxSs1_Callback)CallbackFunc;
            InstancePtr->ToggleRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Bridge Locked*/
        case (XV_HDMITXSS1_HANDLER_BRDGLOCK):
            InstancePtr->BrdgLockedCallback =
			    (XV_HdmiTxSs1_Callback)CallbackFunc;
            InstancePtr->BrdgLockedRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Bridge Unlocked*/
        case (XV_HDMITXSS1_HANDLER_BRDGUNLOCK):
            InstancePtr->BrdgUnlockedCallback =
			    (XV_HdmiTxSs1_Callback)CallbackFunc;
            InstancePtr->BrdgUnlockedRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Bridge Overflow*/
        case (XV_HDMITXSS1_HANDLER_BRDGOVERFLOW):
            InstancePtr->BrdgOverflowCallback =
			    (XV_HdmiTxSs1_Callback)CallbackFunc;
            InstancePtr->BrdgOverflowRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Bridge Underflow*/
        case (XV_HDMITXSS1_HANDLER_BRDGUNDERFLOW):
            InstancePtr->BrdgUnderflowCallback =
			    (XV_HdmiTxSs1_Callback)CallbackFunc;
            InstancePtr->BrdgUnderflowRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Vsync*/
        case (XV_HDMITXSS1_HANDLER_VS):
            InstancePtr->VsCallback = (XV_HdmiTxSs1_Callback)CallbackFunc;
            InstancePtr->VsRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Stream down*/
        case (XV_HDMITXSS1_HANDLER_STREAM_DOWN):
            InstancePtr->StreamDownCallback =
                (XV_HdmiTxSs1_Callback)CallbackFunc;
            InstancePtr->StreamDownRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

        /* Stream up*/
        case (XV_HDMITXSS1_HANDLER_STREAM_UP):
            InstancePtr->StreamUpCallback = (XV_HdmiTxSs1_Callback)CallbackFunc;
            InstancePtr->StreamUpRef = CallbackRef;
            Status = (XST_SUCCESS);
            break;

		/* FRL Config*/
		case (XV_HDMITXSS1_HANDLER_FRL_CONFIG):
			InstancePtr->FrlConfigCallback = (XV_HdmiTxSs1_Callback)CallbackFunc;
			InstancePtr->FrlConfigRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;

		/* FRL FFE*/
		case (XV_HDMITXSS1_HANDLER_FRL_FFE):
			InstancePtr->FrlFfeCallback = (XV_HdmiTxSs1_Callback)CallbackFunc;
			InstancePtr->FrlFfeRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;

		/* FRL Start*/
		case (XV_HDMITXSS1_HANDLER_FRL_START):
			InstancePtr->FrlStartCallback = (XV_HdmiTxSs1_Callback)CallbackFunc;
			InstancePtr->FrlStartRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;

		/* FRL Stop*/
		case (XV_HDMITXSS1_HANDLER_FRL_STOP):
			InstancePtr->FrlStopCallback = (XV_HdmiTxSs1_Callback)CallbackFunc;
			InstancePtr->FrlStopRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;

		/* TMDS Config*/
		case (XV_HDMITXSS1_HANDLER_TMDS_CONFIG):
			InstancePtr->TmdsConfigCallback = (XV_HdmiTxSs1_Callback)CallbackFunc;
			InstancePtr->TmdsConfigRef = CallbackRef;
			Status = (XST_SUCCESS);
			break;

        /* HDCP authenticated*/
        case (XV_HDMITXSS1_HANDLER_HDCP_AUTHENTICATED):
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

        /* HDCP downstream topology available*/
        case (XV_HDMITXSS1_HANDLER_HDCP_DOWNSTREAM_TOPOLOGY_AVAILABLE):
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

        /* HDCP unauthenticated*/
        case (XV_HDMITXSS1_HANDLER_HDCP_UNAUTHENTICATED):
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
* This function installs an asynchronous callback function for the LogWrite
* API:
*
* @param    InstancePtr is a pointer to the HDMI TX Subsystem instance.
* @param    CallbackFunc is the address of the callback function.
* @param    CallbackRef is a user data item that will be passed to the
*       callback function when it is invoked.
*
* @return
*       - XST_SUCCESS if callback function installed successfully.
*       - XST_INVALID_PARAM when callback function is not installed
*                successfully.
*
******************************************************************************/
int XV_HdmiTxSs1_SetLogCallback(XV_HdmiTxSs1 *InstancePtr,
    u64 *CallbackFunc,
    void *CallbackRef)
{
	u32 Status = (XST_FAILURE);

    InstancePtr->LogWriteCallback = (XV_HdmiTxSs1_LogCallback)CallbackFunc;
    InstancePtr->LogWriteRef = CallbackRef;
    Status = (XST_SUCCESS);

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
int XV_HdmiTxSs1_ReadEdid(XV_HdmiTxSs1 *InstancePtr, u8 *Buffer)
{
    u32 Status;

    /* Default*/
    Status = (XST_FAILURE);

    /* Check if a sink is connected*/
    if (InstancePtr->IsStreamConnected == (TRUE)) {

      *Buffer = 0x00;   /* Offset zero*/
      Status = XV_HdmiTx1_DdcWrite(InstancePtr->HdmiTx1Ptr, 0x50, 1, Buffer,
        (FALSE));

      /* Check if write was successful*/
      if (Status == (XST_SUCCESS)) {
        /* Read edid*/
        Status = XV_HdmiTx1_DdcRead(InstancePtr->HdmiTx1Ptr, 0x50, 256, Buffer,
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
int XV_HdmiTxSs1_ReadEdidSegment(XV_HdmiTxSs1 *InstancePtr, u8 *Buffer, u8 segment)
{
    u32 Status;

    u8 dummy = 0;

    /* Default*/
    Status = (XST_FAILURE);

    /* Check if a sink is connected*/
    if (InstancePtr->IsStreamConnected == (TRUE)) {

	  /* For multiple segment EDID read*/
	  /* First, read the first block, then read address 0x7e to know how many*/
	  /* blocks, if more than 2 blocks, then select segment after first 2 blocks*/
	  /* Use the following code to select segment*/

	  if (segment != 0) {
        /* Segment Pointer*/
        Status = XV_HdmiTx1_DdcWrite(InstancePtr->HdmiTx1Ptr, 0x30, 1, &segment,
        (FALSE));
	  }

      /* Read blocks*/
      dummy = 0x00;   /* Offset zero*/
      Status = XV_HdmiTx1_DdcWrite(InstancePtr->HdmiTx1Ptr, 0x50, 1, &dummy,
        (FALSE));

      /* Check if write was successful*/
      if (Status == (XST_SUCCESS)) {
        /* Read edid*/
        Status = XV_HdmiTx1_DdcRead(InstancePtr->HdmiTx1Ptr, 0x50, 128, Buffer,
            (TRUE));
      }

	  if (segment != 0) {
        /* Segment Pointer*/
        Status = XV_HdmiTx1_DdcWrite(InstancePtr->HdmiTx1Ptr, 0x30, 1, &segment,
        (FALSE));
	  }

      /* Read blocks*/
      dummy = 0x80;   /* Offset 128*/
      Status = XV_HdmiTx1_DdcWrite(InstancePtr->HdmiTx1Ptr, 0x50, 1, &dummy,
        (FALSE));

      /* Check if write was successful*/
      if (Status == (XST_SUCCESS)) {
        /* Read edid*/
        Status = XV_HdmiTx1_DdcRead(InstancePtr->HdmiTx1Ptr, 0x50, 128, &Buffer[128],
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
void XV_HdmiTxSs1_ShowEdid(XV_HdmiTxSs1 *InstancePtr)
{
    u8 Buffer[256];
    u8 Row;
    u8 Column;
    u8 Valid;
    u32 Status;
    u8 EdidManName[4];
	u8 segment = 0;
    u8 ExtensionFlag = 0;

    /* Check if a sink is connected*/
    if (InstancePtr->IsStreamConnected == (TRUE)) {

      /* Default*/
      Valid = (FALSE);

      /* Read Sink Edid Segment 0*/
      Status = XV_HdmiTxSs1_ReadEdidSegment(InstancePtr, (u8*)&Buffer, segment);

      /* Check if read was successful*/
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
        /* Check if read was successful*/
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
		  if (segment <= ExtensionFlag) {
            Status = XV_HdmiTxSs1_ReadEdidSegment(InstancePtr, (u8*)&Buffer, segment);
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
void XV_HdmiTxSs1_StreamStart(XV_HdmiTxSs1 *InstancePtr)
{
  u64 TmdsClk;

  /* Set TX pixel rate*/
  XV_HdmiTx1_SetPixelRate(InstancePtr->HdmiTx1Ptr);

  /* Set TX color depth*/
  XV_HdmiTx1_SetColorDepth(InstancePtr->HdmiTx1Ptr);

  /* Set TX color format*/
  XV_HdmiTx1_SetColorFormat(InstancePtr->HdmiTx1Ptr);

  /* Set the TMDS Clock*/
  TmdsClk = XV_HdmiTxSS1_SetTMDS(InstancePtr);

  /* Set TX scrambler*/
  XV_HdmiTx1_Scrambler(InstancePtr->HdmiTx1Ptr);

  /* Set TX clock ratio*/
  XV_HdmiTx1_ClockRatio(InstancePtr->HdmiTx1Ptr);
#ifdef XV_HDMITXSS1_LOG_ENABLE
  XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_STREAMSTART, 0);
#endif
  if ((InstancePtr->HdmiTx1Ptr->Stream.ScdcSupport == (FALSE))
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
void XV_HdmiTxSs1_SetScrambler(XV_HdmiTxSs1 *InstancePtr, u8 Enable)
{
	if (InstancePtr->HdmiTx1Ptr->Stream.ScdcSupport == (TRUE)) {
		/* Override the default behavior of the scrambler so that scrambling
		 * can be disabled even when TMDS Clock > 340 MHz
		 */
		InstancePtr->HdmiTx1Ptr->Stream.OverrideScrambler = (TRUE);
		/* Set the IsScrambled flag in order to enable or disable scrambler
		 * when XV_HdmiTx1_Scrambler is called
		 */
		XV_HdmiTxSs1_SetVideoStreamScramblingFlag(InstancePtr, Enable);
		/* Enable / disable scrambler depending on the value set to
		 * IsScrambled
		 */
		XV_HdmiTx1_Scrambler(InstancePtr->HdmiTx1Ptr);
		/* Disable the override of the scrambler*/
		InstancePtr->HdmiTx1Ptr->Stream.OverrideScrambler = (FALSE);
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
void XV_HdmiTxSs1_SendAuxInfoframe(XV_HdmiTxSs1 *InstancePtr, void *Aux)
{
  u8 Index;
  u8 Crc;
  XHdmiC_Aux *tx_aux = (XHdmiC_Aux *)Aux;

  if (Aux == (NULL)) {
      /* Header, Packet type */
      InstancePtr->HdmiTx1Ptr->Aux.Header.Byte[0] = 0x84;

      /* Version */
      InstancePtr->HdmiTx1Ptr->Aux.Header.Byte[1] = 0x01;

      /* Length */
      InstancePtr->HdmiTx1Ptr->Aux.Header.Byte[2] = 10;

      /* Checksum (this will be calculated by the HDMI TX IP) */
      InstancePtr->HdmiTx1Ptr->Aux.Header.Byte[3] = 0;

      /* 2 Channel count. Audio coding type refer to stream */
      InstancePtr->HdmiTx1Ptr->Aux.Data.Byte[1] = 0x1;

      for (Index = 2; Index < 32; Index++) {
        InstancePtr->HdmiTx1Ptr->Aux.Data.Byte[Index] = 0;
      }

      /* Calculate AVI infoframe checksum */
      Crc = 0;

      /* Header */
      for (Index = 0; Index < 3; Index++) {
        Crc += InstancePtr->HdmiTx1Ptr->Aux.Header.Byte[Index];
      }

      /* Data */
      for (Index = 1; Index < 5; Index++) {
        Crc += InstancePtr->HdmiTx1Ptr->Aux.Data.Byte[Index];
      }

      Crc = 256 - Crc;
      InstancePtr->HdmiTx1Ptr->Aux.Data.Byte[0] = Crc;

      XV_HdmiTx1_AuxSend(InstancePtr->HdmiTx1Ptr);

  }

  else {
      /* Copy Audio Infoframe*/
      if (tx_aux->Header.Byte[0] == 0x84) {
        /* Header*/
        InstancePtr->HdmiTx1Ptr->Aux.Header.Data = tx_aux->Header.Data;

        /* Data*/
        for (Index = 0; Index < 8; Index++) {
          InstancePtr->HdmiTx1Ptr->Aux.Data.Data[Index] =
            tx_aux->Data.Data[Index];
        }
      }
  }

  /* Send packet */
  XV_HdmiTx1_AuxSend(InstancePtr->HdmiTx1Ptr);
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
u32 XV_HdmiTxSs1_SendGenericAuxInfoframe(XV_HdmiTxSs1 *InstancePtr, void *Aux)
{
  u8 Index;
  XHdmiC_Aux *tx_aux = (XHdmiC_Aux *)Aux;

  /* Header*/
  InstancePtr->HdmiTx1Ptr->Aux.Header.Data = tx_aux->Header.Data;

  /* Data*/
  for (Index = 0; Index < 8; Index++) {
    InstancePtr->HdmiTx1Ptr->Aux.Data.Data[Index] =
    tx_aux->Data.Data[Index];
  }

  /* Send packet */
  return XV_HdmiTx1_AuxSend(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS number of active audio channels
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
* @param  AudioChannels
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetAudioChannels(XV_HdmiTxSs1 *InstancePtr, u8 AudioChannels)
{
	/* Audio InfoFrame's channel count starts from 2ch = 1*/
	InstancePtr->AudioInfoframe.ChannelCount = AudioChannels - 1;
    InstancePtr->AudioChannels = AudioChannels;
    XV_HdmiTx1_SetAudioChannels(InstancePtr->HdmiTx1Ptr, AudioChannels);
#ifdef XV_HDMITXSS1_LOG_ENABLE
    XV_HdmiTxSs1_LogWrite(InstancePtr,
                         XV_HDMITXSS1_LOG_EVT_SETAUDIOCHANNELS,
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
void XV_HdmiTxSs1_AudioMute(XV_HdmiTxSs1 *InstancePtr, u8 Enable)
{
  /*Audio Mute Mode*/
  if (Enable){
	XV_HdmiTx1_AudioDisable(InstancePtr->HdmiTx1Ptr);
#ifdef XV_HDMITXSS1_LOG_ENABLE
    XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_AUDIOMUTE, 0);
#endif
  }
  else{
	XV_HdmiTx1_AudioEnable(InstancePtr->HdmiTx1Ptr);
#ifdef XV_HDMITXSS1_LOG_ENABLE
    XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_AUDIOUNMUTE, 0);
#endif
  }
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS Audio Format
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
* @param  format 1:HBR 0:L-PCM
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetAudioFormat(XV_HdmiTxSs1 *InstancePtr,
		 XV_HdmiTx1_AudioFormatType format)
{
    XV_HdmiTx1_SetAudioFormat(InstancePtr->HdmiTx1Ptr, format);
}

/*****************************************************************************/
/**
*
* This function gets the active audio format
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return   Active audio format of HDMI Tx Subsystem
*
* @note     None.
*
******************************************************************************/
XV_HdmiTx1_AudioFormatType XV_HdmiTxSs1_GetAudioFormat(XV_HdmiTxSs1 *InstancePtr)
{
	XV_HdmiTx1_AudioFormatType Format;

	Format = XV_HdmiTx1_GetAudioFormat(InstancePtr->HdmiTx1Ptr);

    return Format;
}

/*****************************************************************************/
/**
*
* This function Sets use the internal ACR Module
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
* @param  Audio Sample Frequency in Hz
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetIntACR(XV_HdmiTxSs1 *InstancePtr)
{
	InstancePtr->HdmiTx1Ptr->CTS_N_Source =
			XV_HDMITX1_INTERNAL_CTS_N;
}

/*****************************************************************************/
/**
*
* This function Sets use the external ACR Module
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
* @param  Audio Sample Frequency in Hz
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetExtACR(XV_HdmiTxSs1 *InstancePtr)
{
	InstancePtr->HdmiTx1Ptr->CTS_N_Source =
			XV_HDMITX1_EXTERNAL_CTS_N;
}

/*****************************************************************************/
/**
*
* This function Sets the audio sampling frequency
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
* @param  Audio Sample Frequency in Hz
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetSampleFrequency(XV_HdmiTxSs1 *InstancePtr,
		XHdmiC_SamplingFrequencyVal AudSampleFreqVal)
{
	InstancePtr->HdmiTx1Ptr->Stream.Audio.SampleFrequency =
			AudSampleFreqVal;

	switch (AudSampleFreqVal) {
		case XHDMIC_SAMPLING_FREQ_32K:
			InstancePtr->HdmiTx1Ptr->Stream.Audio.SFreq =
				XHDMIC_SAMPLING_FREQUENCY_32K;
		break;
		case XHDMIC_SAMPLING_FREQ_44_1K:
			InstancePtr->HdmiTx1Ptr->Stream.Audio.SFreq =
				XHDMIC_SAMPLING_FREQUENCY_44_1K;
		break;
		case XHDMIC_SAMPLING_FREQ_48K:
			InstancePtr->HdmiTx1Ptr->Stream.Audio.SFreq =
				XHDMIC_SAMPLING_FREQUENCY_48K;
		break;
		case XHDMIC_SAMPLING_FREQ_88_2K:
			InstancePtr->HdmiTx1Ptr->Stream.Audio.SFreq =
				XHDMIC_SAMPLING_FREQUENCY_88_2K;
		break;
		case XHDMIC_SAMPLING_FREQ_96K:
			InstancePtr->HdmiTx1Ptr->Stream.Audio.SFreq =
				XHDMIC_SAMPLING_FREQUENCY_96K;
		break;
		case XHDMIC_SAMPLING_FREQ_176_4K:
			InstancePtr->HdmiTx1Ptr->Stream.Audio.SFreq =
				XHDMIC_SAMPLING_FREQUENCY_176_4K;
		break;
		case XHDMIC_SAMPLING_FREQ_192K:
			InstancePtr->HdmiTx1Ptr->Stream.Audio.SFreq =
				XHDMIC_SAMPLING_FREQUENCY_192K;
		break;
		default:
			InstancePtr->HdmiTx1Ptr->Stream.Audio.SFreq =
				XHDMIC_SAMPLING_FREQUENCY;
#ifdef XV_HDMITXSS1_LOG_ENABLE
			XV_HdmiTxSs1_LogWrite(InstancePtr,
				XV_HDMITXSS1_LOG_EVT_AUDIOINVALIDSAMPRATE, 0);
#endif
			XV_HdmiTxSs1_ErrorCallback(InstancePtr);
		break;
	}
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS Aux structure
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_Aux *XV_HdmiTxSs1_GetAuxiliary(XV_HdmiTxSs1 *InstancePtr)
{
    return (&(InstancePtr->HdmiTx1Ptr->Aux));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS AVI InfoFrame structure
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
*
* @return XHdmiC_AVI_InfoFrame pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_AVI_InfoFrame *XV_HdmiTxSs1_GetAviInfoframe(XV_HdmiTxSs1 *InstancePtr)
{
    return (&(InstancePtr->AVIInfoframe));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS Audio InfoFrame structure
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
*
* @return XHdmiC_AudioInfoFrame pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_AudioInfoFrame *XV_HdmiTxSs1_GetAudioInfoframe(XV_HdmiTxSs1 *InstancePtr)
{
    return (&(InstancePtr->AudioInfoframe));
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS Vendor Specific InfoFrame
* structure
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
*
* @return XHdmiC_VSIF pointer
*
* @note   None.
*
******************************************************************************/
XHdmiC_VSIF *XV_HdmiTxSs1_GetVSIF(XV_HdmiTxSs1 *InstancePtr)
{
    return (&(InstancePtr->VSIF));
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
u64 XV_HdmiTxSs1_SetStream(XV_HdmiTxSs1 *InstancePtr,
		XVidC_VideoTiming VideoTiming,
		XVidC_FrameRate FrameRate,
		XVidC_ColorFormat ColorFormat,
		XVidC_ColorDepth Bpc,
		XVidC_3DInfo *Info3D)
{
	u64 TmdsClock = 0;
	TmdsClock = XV_HdmiTx1_SetStream(InstancePtr->HdmiTx1Ptr,
			VideoTiming, FrameRate, ColorFormat, Bpc,
			InstancePtr->Config.Ppc, Info3D);

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_SETSTREAM, 0);
#endif
	if (TmdsClock == 0) {
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"\r\nWarning: Sink does not support HDMI 2.0"
				"\r\n");
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"         Connect to HDMI 2.0 Sink or \r\n");
		xdbg_printf(XDBG_DEBUG_GENERAL,
				"         Change to HDMI 1.4 video format"
				"\r\n\r\n");
	}

	return TmdsClock;
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS video stream
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
XVidC_VideoStream *XV_HdmiTxSs1_GetVideoStream(XV_HdmiTxSs1 *InstancePtr)
{
    return (&InstancePtr->HdmiTx1Ptr->Stream.Video);
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video stream
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
* @param
*
* @return XVidC_VideoStream pointer
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetVideoStream(XV_HdmiTxSs1 *InstancePtr,
                                    XVidC_VideoStream VidStream)
{
    /* Set Stream Properties*/
    InstancePtr->HdmiTx1Ptr->Stream.Video = VidStream;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video Identification code
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
* @param  SamplingRate Value
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetSamplingRate(XV_HdmiTxSs1 *InstancePtr, u8 SamplingRate)
{
    InstancePtr->SamplingRate = SamplingRate;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video Identification code
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
* @param  InstancePtr VIC Flag Value
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetVideoIDCode(XV_HdmiTxSs1 *InstancePtr, u8 Vic)
{
    InstancePtr->HdmiTx1Ptr->Stream.Vic = Vic;
}

/*****************************************************************************/
/**
*
* This function returns the HDMI TX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
*
* @return Value 1:HDMI TMDS 0:DVI.
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiTxSs1_GetVideoStreamType(XV_HdmiTxSs1 *InstancePtr)
{
	return InstancePtr->HdmiTx1Ptr->Stream.IsHdmi;
}

/*****************************************************************************/
/**
*
* This function returns the pointer to HDMI TX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
*
* @return Transport Mode  1:FRL 0:TMDS
*
* @note   None.
*
******************************************************************************/
u8 XV_HdmiTxSs1_GetTransportMode(XV_HdmiTxSs1 *InstancePtr)
{
    return (InstancePtr->HdmiTx1Ptr->Stream.IsFrl);
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video stream type
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
* @param  IsScrambled 1:IsScrambled 0: not Scrambled
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetVideoStreamScramblingFlag(XV_HdmiTxSs1 *InstancePtr,
                                                            u8 IsScrambled)
{
    InstancePtr->HdmiTx1Ptr->Stream.IsScrambled = IsScrambled;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video stream scrambling behaviour. Setting
* OverrideScramble to true will force enabling/disabling scrambling function
* based on the value of IsScrambled flag.
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
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
void XV_HdmiTxSs1_SetVideoStreamScramblingOverrideFlag(XV_HdmiTxSs1 *InstancePtr,
                                                           u8 OverrideScramble)
{
    InstancePtr->HdmiTx1Ptr->Stream.OverrideScrambler = OverrideScramble;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS TMDS Cock Ratio
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
* @param  Ratio 0 - 1/10, 1 - 1/40
*
* @return None.
*
* @note   None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetTmdsClockRatio(XV_HdmiTxSs1 *InstancePtr, u8 Ratio)
{
    InstancePtr->HdmiTx1Ptr->Stream.TMDSClockRatio = Ratio;
}

/*****************************************************************************/
/**
*
* This function Sets the HDMI TX SS video Identification code
*
* @param  InstancePtr pointer to XV_HdmiTxSs1 instance
* @param
*
* @return Stream Data Structure (TMDS Clock)
*
* @note   None.
*
******************************************************************************/
u32 XV_HdmiTxSs1_GetTmdsClockFreqHz(XV_HdmiTxSs1 *InstancePtr)
{
    return (InstancePtr->HdmiTx1Ptr->Stream.TMDSClock);
}

/*****************************************************************************/
/**
*
* This function detects connected sink is a HDMI 2.0/HDMI 1.4 sink device
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return
*       - XST_SUCCESS if HDMI 2.0
*       - XST_FAILURE if HDMI 1.4
*
* @note   None.
*
******************************************************************************/
int XV_HdmiTxSs1_DetectHdmi20(XV_HdmiTxSs1 *InstancePtr)
{
      return (XV_HdmiTx1_DetectHdmi20(InstancePtr->HdmiTx1Ptr));
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
void XV_HdmiTxSs1_RefClockChangeInit(XV_HdmiTxSs1 *InstancePtr)
{

      /* Assert VID_IN bridge resets */
      XV_HdmiTxSs1_SYSRST(InstancePtr, TRUE);
      XV_HdmiTxSs1_VRST(InstancePtr, TRUE);

      /* Assert HDMI TXCore resets */
      XV_HdmiTxSs1_TXCore_LRST(InstancePtr, TRUE);
      XV_HdmiTxSs1_TXCore_VRST(InstancePtr, TRUE);

      /* Clear variables */
      XV_HdmiTx1_Clear(InstancePtr->HdmiTx1Ptr);
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
void XV_HdmiTxSs1_ReportTiming(XV_HdmiTxSs1 *InstancePtr)
{
      if (((u32) XV_HdmiTx1_GetMode(InstancePtr->HdmiTx1Ptr)) == 0) {
        xil_printf("HDMI TX Mode - DVI");
      } else {
        xil_printf("HDMI TX Mode - HDMI ");
        if (InstancePtr->HdmiTx1Ptr->Stream.IsFrl == FALSE) {
        	xil_printf("TMDS");
        } else {
        	xil_printf("FRL (%d lanes @ %d Gbps)",
        			InstancePtr->HdmiTx1Ptr->Stream.Frl.Lanes,
				InstancePtr->HdmiTx1Ptr->Stream.Frl.LineRate);
        }
      }

      xil_printf("\r\n");

      if (XV_HdmiTxSS1_IsMasked(InstancePtr) == 0) {
	  xil_printf("HDMI Video Mask is Disabled\r\n\r\n");
      }
      else {
	  xil_printf("HDMI Video Mask is Enabled\r\n\r\n");
      }

      XV_HdmiTx1_DebugInfo(InstancePtr->HdmiTx1Ptr);
      xil_printf("VIC: %0d\r\n", InstancePtr->HdmiTx1Ptr->Stream.Vic);
      xil_printf("Scrambled: %0d\r\n",
        (XV_HdmiTx1_IsStreamScrambled(InstancePtr->HdmiTx1Ptr)));
      xil_printf("Sample rate: %0d\r\n",
        (XV_HdmiTx1_GetSampleRate(InstancePtr->HdmiTx1Ptr)));
      xil_printf("\r\n");

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
void XV_HdmiTxSs1_ReportAudio(XV_HdmiTxSs1 *InstancePtr)
{
  xil_printf("Format   : ");
  if (XV_HdmiTxSs1_GetAudioFormat(InstancePtr) == 1) {
	  xil_printf("HBR\r\n");
  }
  else {
	  xil_printf("L-PCM\r\n");
  }
  xil_printf("Channels : %d\r\n",
  XV_HdmiTx1_GetAudioChannels(InstancePtr->HdmiTx1Ptr));
  xil_printf("ACR CTS  : %d\r\n",
		  XV_HdmiTxSs1_GetAudioCtsVal(InstancePtr->HdmiTx1Ptr));
  xil_printf("ACR N    : %d\r\n",
		  XV_HdmiTxSs1_GetAudioNVal(InstancePtr->HdmiTx1Ptr));
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
void XV_HdmiTxSs1_ReportSubcoreVersion(XV_HdmiTxSs1 *InstancePtr)
{
  u32 Data;

  if (InstancePtr->HdmiTx1Ptr) {
     Data = XV_HdmiTx1_GetVersion(InstancePtr->HdmiTx1Ptr);
     xil_printf("  HDMI TX version : %02d.%02d (%04x)\r\n",
        ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }

  if (InstancePtr->VtcPtr){
      if (InstancePtr->IsStreamUp == TRUE){
          if (XV_HdmiTx1_ReadReg(InstancePtr->HdmiTx1Ptr->Config.BaseAddress,
                                  (XV_HDMITX1_PIO_IN_OFFSET)) &
                                      XV_HDMITX1_PIO_IN_VID_RDY_MASK) {
              Data = XVtc_GetVersion(InstancePtr->VtcPtr);
              xil_printf("  VTC version     : %02d.%02d (%04x)\r\n",
                 ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
          } else {
              xil_printf("  VTC version is not available for reading as ");
              xil_printf("HDMI TX Video Clock is not ready.\r\n");
          }
      } else {
          xil_printf("  VTC version is not available for reading as ");
          xil_printf("HDMI TX Video Clock is not ready.\r\n");
      }
  }

#ifdef XPAR_XHDCP_NUM_INSTANCES
  /* HDCP 1.4*/
  if (InstancePtr->Hdcp14Ptr){
     Data = XHdcp1x_GetVersion(InstancePtr->Hdcp14Ptr);
     xil_printf("  HDCP 1.4 TX version : %02d.%02d (%04x)\r\n",
        ((Data >> 24) & 0xFF), ((Data >> 16) & 0xFF), (Data & 0xFFFF));
  }
#endif

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
  /* HDCP 2.2*/
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
void XV_HdmiTxSs1_ReportInfo(XV_HdmiTxSs1 *InstancePtr)
{
    xil_printf("------------\r\n");
    xil_printf("HDMI TX SubSystem\r\n");
    xil_printf("------------\r\n");
    XV_HdmiTxSs1_ReportCoreInfo(InstancePtr);
    XV_HdmiTxSs1_ReportSubcoreVersion(InstancePtr);
    xil_printf("\r\n");
    xil_printf("HDMI TX timing\r\n");
    xil_printf("------------\r\n");
    XV_HdmiTxSs1_ReportTiming(InstancePtr);
    xil_printf("Audio\r\n");
    xil_printf("---------\r\n");
    XV_HdmiTxSs1_ReportAudio(InstancePtr);
}

/******************************************************************************/
/**
*
* This function prints debug information on STDIO/UART console.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_DebugInfo(XV_HdmiTxSs1 *InstancePtr)
{
	XV_HdmiTx1_DebugInfo(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
* This function prints out the sub-core register dump
*
* @param	InstancePtr  Instance Pointer to the main data structure
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XV_HdmiTxSs1_RegisterDebug(XV_HdmiTxSs1 *InstancePtr)
{
	u32 RegOffset;

	/* HDMI TX Core */
	XV_HdmiTx1_RegisterDebug(InstancePtr->HdmiTx1Ptr);

	/* VTC Core */
	if (InstancePtr->VtcPtr){
		xil_printf("-------------------------------------\r\n");
		xil_printf("        VTC Register Dump \r\n");
		xil_printf("-------------------------------------\r\n");
		if (InstancePtr->IsStreamUp == TRUE){
			if (XV_HdmiTx1_ReadReg(
				InstancePtr->HdmiTx1Ptr->Config.BaseAddress,
					(XV_HDMITX1_PIO_IN_OFFSET)) &
					XV_HDMITX1_PIO_IN_VID_RDY_MASK) {
				for (RegOffset = 0;
					RegOffset <= XVTC_GGD_OFFSET;) {
					xil_printf("0x%04x      0x%08x\r\n",
							RegOffset,
					XV_HdmiTx1_ReadReg(
						InstancePtr->Config.Vtc.AbsAddr,
						RegOffset));
					RegOffset += 4;
				}
			}
		}
	}
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
int XV_HdmiTxSs1_IsStreamUp(XV_HdmiTxSs1 *InstancePtr)
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
int XV_HdmiTxSs1_IsStreamConnected(XV_HdmiTxSs1 *InstancePtr)
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
int XV_HdmiTxSs1_IsStreamToggled(XV_HdmiTxSs1 *InstancePtr)
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
static void XV_HdmiTxSs1_ConfigBridgeMode(XV_HdmiTxSs1 *InstancePtr) {
	XVidC_VideoStream *HdmiTxSs1VidStreamPtr;
    HdmiTxSs1VidStreamPtr = XV_HdmiTxSs1_GetVideoStream(InstancePtr);

    XHdmiC_AVI_InfoFrame *AviInfoFramePtr;
    AviInfoFramePtr = XV_HdmiTxSs1_GetAviInfoframe(InstancePtr);

    /* Pixel Repetition factor of 3 and above are not supported by the bridge*/
    if (AviInfoFramePtr->PixelRepetition > XHDMIC_PIXEL_REPETITION_FACTOR_2) {
#ifdef XV_HDMITXSS1_LOG_ENABLE
    	XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_PIX_REPEAT_ERR,
    			AviInfoFramePtr->PixelRepetition);
#endif
    	return;
    }

    if (HdmiTxSs1VidStreamPtr->ColorFormatId == XVIDC_CSF_YCRCB_420) {
        /*********************************************************
         * 420 Support
         *********************************************************/
         XV_HdmiTxSs1_BridgePixelRepeat(InstancePtr, FALSE);
         AviInfoFramePtr->PixelRepetition = XHDMIC_PIXEL_REPETITION_FACTOR_1;
         XV_HdmiTxSs1_BridgeYuv420(InstancePtr, TRUE);
    }
    else {
        if ((AviInfoFramePtr->PixelRepetition ==
					XHDMIC_PIXEL_REPETITION_FACTOR_2))
        {
            /*********************************************************
             * NTSC/PAL Support
             *********************************************************/
             XV_HdmiTxSs1_BridgeYuv420(InstancePtr, FALSE);
             XV_HdmiTxSs1_BridgePixelRepeat(InstancePtr, TRUE);
        }
        else {
            XV_HdmiTxSs1_BridgeYuv420(InstancePtr, FALSE);
            XV_HdmiTxSs1_BridgePixelRepeat(InstancePtr, FALSE);
            AviInfoFramePtr->PixelRepetition =
					XHDMIC_PIXEL_REPETITION_FACTOR_1;
        }
    }
}

/*****************************************************************************/
/**
* This function will set the default in HDF.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
* @param    Id is the XV_HdmiTxSs1 ID to operate on.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetDefaultPpc(XV_HdmiTxSs1 *InstancePtr, u8 Id) {
    extern XV_HdmiTxSs1_Config
                        XV_HdmiTxSs1_ConfigTable[XPAR_XV_HDMITXSS1_NUM_INSTANCES];
    InstancePtr->Config.Ppc = XV_HdmiTxSs1_ConfigTable[Id].Ppc;
}

/*****************************************************************************/
/**
* This function will set PPC specified by user.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
* @param    Id is the XV_HdmiTxSs1 ID to operate on.
* @param    Ppc is the PPC to be set.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSs1_SetPpc(XV_HdmiTxSs1 *InstancePtr, u8 Id, u8 Ppc) {
    InstancePtr->Config.Ppc = (XVidC_PixelsPerClock) Ppc;
    Id = Id; /*squash unused variable compiler warning*/
}

/*****************************************************************************/
/**
* This function will enable the video masking.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS1_MaskEnable(XV_HdmiTxSs1 *InstancePtr)
{
    XV_HdmiTx1_MaskEnable(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
* This function will disable the video masking.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS1_MaskDisable(XV_HdmiTxSs1 *InstancePtr)
{
    XV_HdmiTx1_MaskDisable(InstancePtr->HdmiTx1Ptr);
}

/*****************************************************************************/
/**
* This function will enable or disable the noise in the video mask.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
* @param	Enable specifies TRUE/FALSE value to either enable or disable the
*		Noise.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS1_MaskNoise(XV_HdmiTxSs1 *InstancePtr, u8 Enable)
{
    XV_HdmiTx1_MaskNoise(InstancePtr->HdmiTx1Ptr, Enable);
}

static u32 XV_HdmiTxSS1_GetVidMaskColorValue(XV_HdmiTxSs1 *InstancePtr,
											u16 Value)
{
	u32 Data;
	s8 Temp;

	Temp = InstancePtr->Config.MaxBitsPerPixel -
				InstancePtr->HdmiTx1Ptr->Stream.Video.ColorDepth;
	Data = Value << ((Temp > 0) ? Temp : 0);

	return Data;
}

/*****************************************************************************/
/**
* This function will set the red component in the video mask.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
* @param	Value specifies the video mask value set to red component
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS1_MaskSetRed(XV_HdmiTxSs1 *InstancePtr, u16 Value)
{
    u32 Data;

    Data = XV_HdmiTxSS1_GetVidMaskColorValue(InstancePtr, Value);

	XV_HdmiTx1_MaskSetRed(InstancePtr->HdmiTx1Ptr, (Data));
}


/*****************************************************************************/
/**
* This function will set the green component in the video mask.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
* @param	Value specifies the video mask value set to green component
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS1_MaskSetGreen(XV_HdmiTxSs1 *InstancePtr, u16 Value)
{
    u32 Data;

    Data = XV_HdmiTxSS1_GetVidMaskColorValue(InstancePtr, Value);

	XV_HdmiTx1_MaskSetGreen(InstancePtr->HdmiTx1Ptr, (Data));
}

/*****************************************************************************/
/**
* This function will set the blue component in the video mask.
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
* @param	Value specifies the video mask value set to blue component
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XV_HdmiTxSS1_MaskSetBlue(XV_HdmiTxSs1 *InstancePtr, u16 Value)
{
    u32 Data;

    Data = XV_HdmiTxSS1_GetVidMaskColorValue(InstancePtr, Value);

	XV_HdmiTx1_MaskSetBlue(InstancePtr->HdmiTx1Ptr, (Data));
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
void XV_HdmiTxSS1_SetBackgroundColor(XV_HdmiTxSs1 *InstancePtr,
                                    XVMaskColorId  ColorId)
{
    u16 Cr_R_val,y_G_val,Cb_B_val;
    u16 scale;

    XVidC_ColorFormat cfmt;
	XVidC_ColorDepth bpc;

    XVidC_VideoStream *HdmiTxSs1VidStreamPtr;
    HdmiTxSs1VidStreamPtr = XV_HdmiTxSs1_GetVideoStream(InstancePtr);

    cfmt = HdmiTxSs1VidStreamPtr->ColorFormatId;
	bpc = InstancePtr->HdmiTx1Ptr->Stream.Video.ColorDepth;

  /*
   * Assert validates the input arguments
   */
  Xil_AssertVoid(InstancePtr != NULL);

  if (cfmt == XVIDC_CSF_RGB)
  {
    scale = ((1<<bpc)-1);
    y_G_val  = bkgndColorRGB[ColorId][0] * scale;
    Cb_B_val = bkgndColorRGB[ColorId][1] * scale;
    Cr_R_val = bkgndColorRGB[ColorId][2] * scale;
  }
  else /*YUV*/
  {
    scale =  (1<<(bpc-XVIDC_BPC_8));
    y_G_val  = bkgndColorYUV[ColorId][0] * scale;
    Cb_B_val = bkgndColorYUV[ColorId][1] * scale;
    Cr_R_val = bkgndColorYUV[ColorId][2] * scale;
  }

  if (ColorId == XV_BKGND_NOISE) {
	XV_HdmiTxSS1_MaskNoise(InstancePtr, (TRUE));
  }
  else {
	XV_HdmiTxSS1_MaskNoise(InstancePtr, (FALSE));
    /* Set Background color (outside window) */
	XV_HdmiTxSS1_MaskSetRed(InstancePtr,  Cr_R_val);
	XV_HdmiTxSS1_MaskSetGreen(InstancePtr, y_G_val);
	XV_HdmiTxSS1_MaskSetBlue(InstancePtr, Cb_B_val);
  }
  XV_HdmiTxSS1_MaskEnable(InstancePtr);
}


/*****************************************************************************/
/**
* This function will get the current video mask mode
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
*
* @return   Current mode.
*       0 = Video masking is disabled
*       1 = Video masking is enabled
*
* @note     None.
*
*
******************************************************************************/
u8 XV_HdmiTxSS1_IsMasked(XV_HdmiTxSs1 *InstancePtr)
{
    return ((u8)XV_HdmiTx1_IsMasked(InstancePtr->HdmiTx1Ptr));
}

/*****************************************************************************/
/**
* This function will set the major and minor application version in TXSs struct
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
* @param    maj is the major version of the application.
* @param    min is the minor version of the application.
* @return   void.
*
* @note     None.
*
*
******************************************************************************/
void XV_HdmiTxSS1_SetAppVersion(XV_HdmiTxSs1 *InstancePtr, u8 maj, u8 min)
{
	InstancePtr->AppMajVer = maj;
	InstancePtr->AppMinVer = min;
}


/*****************************************************************************/
/**
* This function will Stop the FRL stream
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
* @return   void.
*
* @note     None.
*
*
******************************************************************************/
void XV_HdmiTxSS1_StopFRLStream(XV_HdmiTxSs1 *InstancePtr)
{
	if (InstancePtr->HdmiTx1Ptr->Stream.IsFrl == TRUE) {

        /* Disable Audio */
        XV_HdmiTx1_AudioDisable(InstancePtr->HdmiTx1Ptr);

        /* Disable AUX */
        XV_HdmiTx1_AuxDisable(InstancePtr->HdmiTx1Ptr);

		/* Assert HDMI TXCore link reset */
		XV_HdmiTxSs1_TXCore_LRST(InstancePtr, TRUE);
		XV_HdmiTxSs1_TXCore_VRST(InstancePtr, TRUE);

		/* Assert SYSCLK VID_OUT bridge reset */
		XV_HdmiTxSs1_SYSRST(InstancePtr, TRUE);
		XV_HdmiTxSs1_VRST(InstancePtr, TRUE);

		/* Reset DDC */
		XV_HdmiTx1_DdcDisable(InstancePtr->HdmiTx1Ptr);

		/* Set stream up flag */
		InstancePtr->IsStreamUp = (FALSE);
#ifdef XV_HDMITXSS1_LOG_ENABLE
		XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_STREAMDOWN, 0);
#endif
#ifdef USE_HDCP_TX
/* Push the stream-down event to the HDCP event queue */
		XV_HdmiTxSs1_HdcpPushEvent(InstancePtr, XV_HDMITXSS1_HDCP_STREAMDOWN_EVT);
#endif

		/* Check if user callback has been registered */
		if (InstancePtr->StreamDownCallback) {
		InstancePtr->StreamDownCallback(InstancePtr->StreamDownRef);
		}
	}
}

/*****************************************************************************/
/**
* This function will Start the FRL stream
*
* @param    InstancePtr is a pointer to the XV_HdmiTxSs1 core instance.
* @return   void.
*
* @note     None.
*
*
******************************************************************************/

void XV_HdmiTxSS1_StartFRLStream(XV_HdmiTxSs1 *InstancePtr)
{
	if (InstancePtr->HdmiTx1Ptr->Stream.IsFrl == TRUE) {

		/* Enable the AUX peripheral */
		XV_HdmiTx1_AuxEnable(InstancePtr->HdmiTx1Ptr);

		/* Enable the AUX peripheral interrupt */
		XV_HdmiTx1_AuxIntrEnable(InstancePtr->HdmiTx1Ptr);

		/* Set stream up flag */
		InstancePtr->IsStreamUp = (TRUE);

#ifdef USE_HDCP_TX
		/* Push the stream-up event to the HDCP event queue */
		XV_HdmiTxSs1_HdcpPushEvent(InstancePtr, XV_HDMITXSS1_HDCP_STREAMUP_EVT);
#endif

		/* Check if user callback has been registered.
		User may change the video stream properties in the callback;
		therefore, execute the callback before changing stream settings. */
		if (InstancePtr->StreamUpCallback) {
			InstancePtr->StreamUpCallback(InstancePtr->StreamUpRef);
		}

		/* Set TX sample rate */
		XV_HdmiTx1_SetSampleRate(InstancePtr->HdmiTx1Ptr, InstancePtr->SamplingRate);

		/* Release VID_IN bridge resets */
		XV_HdmiTxSs1_SYSRST(InstancePtr, FALSE);
		XV_HdmiTxSs1_VRST(InstancePtr, FALSE);

		/* Release HDMI TXCore resets */
		XV_HdmiTxSs1_TXCore_LRST(InstancePtr, FALSE);
		XV_HdmiTxSs1_TXCore_VRST(InstancePtr, FALSE);

		if (InstancePtr->VtcPtr) {
			if (XV_HdmiTx1_ReadReg(InstancePtr->HdmiTx1Ptr->Config.BaseAddress,
									(XV_HDMITX1_PIO_IN_OFFSET)) &
									XV_HDMITX1_PIO_IN_VID_RDY_MASK) {
				/* Setup VTC */
				XV_HdmiTxSs1_VtcSetup(InstancePtr);
			} else {
				xil_printf("VTC Configuration Unsuccessful, No Video Clock!\r\n");
			}
		}

		if (InstancePtr->AudioEnabled) {
			/* HDMI TX unmute audio */
			InstancePtr->AudioMute = (FALSE);
			XV_HdmiTx1_AudioEnable(InstancePtr->HdmiTx1Ptr);
		}

		/* Configure video bridge mode according to HW setting and video format */
		XV_HdmiTxSs1_ConfigBridgeMode(InstancePtr);
#ifdef XV_HDMITXSS1_LOG_ENABLE
		XV_HdmiTxSs1_LogWrite(InstancePtr, XV_HDMITXSS1_LOG_EVT_STREAMUP, 0);
#endif

	}
}

static void XV_HdmiTxSs1_FrlLtsLCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_FRL_LTSL, 0);
#endif
}

static void XV_HdmiTxSs1_FrlLts1Callback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_FRL_LTS1, 0);
#endif
}

static void XV_HdmiTxSs1_FrlLts2Callback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_FRL_LTS2,
			(HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Frl.FfeLevels << 4) |
			HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Frl.LineRate);
#endif
}

static void XV_HdmiTxSs1_FrlLts3Callback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_FRL_LTS3,
			HdmiTxSs1Ptr->HdmiTx1Ptr->DBMessage);
#endif
}

static void XV_HdmiTxSs1_FrlLts4Callback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_FRL_LTS4,
			(HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Frl.FfeLevels << 4) |
			HdmiTxSs1Ptr->HdmiTx1Ptr->Stream.Frl.LineRate);
#endif
}

static void XV_HdmiTxSs1_FrlLtsPCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

#ifdef XV_HDMITXSS1_LOG_ENABLE
	XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_FRL_LTSP,
			HdmiTxSs1Ptr->HdmiTx1Ptr->DBMessage);
#endif
}

static void XV_HdmiTxSs1_CedUpdateCallback(void *CallbackRef)
{
	XV_HdmiTxSs1 *HdmiTxSs1Ptr = (XV_HdmiTxSs1 *)CallbackRef;

  /* Check if user callback has been registered */
  if (HdmiTxSs1Ptr->CedUpdateCallback) {
    HdmiTxSs1Ptr->CedUpdateCallback(HdmiTxSs1Ptr->ConnectRef);
  }
}

u8 *XV_HdmiTxSs1_GetScdcEdRegisters(XV_HdmiTxSs1 *InstancePtr)
{
	return XV_HdmiTx1_GetScdcEdRegisters(InstancePtr->HdmiTx1Ptr);
}
