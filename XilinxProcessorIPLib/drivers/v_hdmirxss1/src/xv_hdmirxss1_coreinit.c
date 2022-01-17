/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmirxss1_coreinit.c
* @addtogroup v_hdmirxss1_v3_1
* @{
* @details

* HDMI RX Subsystem Sub-Cores initialization
* The functions in this file provides an abstraction from the initialization
* sequence for included sub-cores. Subsystem is assigned an address and range
* on the axi-lite interface. This address space is condensed where-in each
* sub-core is at a fixed offset from the subsystem base address. For processor
* to be able to access the sub-core this offset needs to be transalted into a
* absolute address within the subsystems addressable range
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
#include "xv_hdmirxss1_coreinit.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
#ifdef USE_HDCP_RX
static void XV_HdmiRxSs1_DdcSetRegAddrHandler(void *RefPtr, u32 Data);
static void XV_HdmiRxSs1_DdcSetRegDataHandler(void *RefPtr, u32 Data);
static u32 XV_HdmiRxSs1_DdcGetRegDataHandler(void *RefPtr);
static void XV_HdmiRxSs1_DdcHdcpCallback(void *RefPtr, int Type);
static void XV_HdmiRxSs1_DdcHdcp14ProtocolEvtCallback(void *RefPtr);
static void XV_HdmiRxSs1_DdcHdcp22ProtocolEvtCallback(void *RefPtr);
#endif
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
static u32 XV_HdmiRxSs1_DdcGetWriteMessageBufferWordsHandler(void *RefPtr);
static u32 XV_HdmiRxSs1_DdcGetReadMessageBufferWordsHandler(void *RefPtr);
static u32 XV_HdmiRxSs1_DdcIsReadMessageBufferEmptyHandler(void *RefPtr);
static u32 XV_HdmiRxSs1_DdcIsWriteMessageBufferEmptyHandler(void *RefPtr);
static void XV_HdmiRxSs1_DdcClearReadMessageBufferHandler(void *RefPtr);
static void XV_HdmiRxSs1_DdcClearWriteMessageBufferHandler(void *RefPtr);
static void XV_HdmiRxSs1_LinkErrorCallback(void *RefPtr);
#endif

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiRxSs1Ptr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiRxSs1_SubcoreInitHdmiRx1(XV_HdmiRxSs1 *HdmiRxSs1Ptr)
{
  int Status;
  XV_HdmiRx1_Config *ConfigPtr;

  if (HdmiRxSs1Ptr->HdmiRx1Ptr)
  {
    /* Get core configuration */
#ifdef XV_HDMIRXSS1_LOG_ENABLE
    XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_HDMIRX1_INIT, 0);
#endif
    ConfigPtr  = XV_HdmiRx1_LookupConfig(HdmiRxSs1Ptr->Config.HdmiRx1.DeviceId);
    if (ConfigPtr == NULL)
    {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMIRXSS1 ERR:: HDMI RX device not found\r\n");
      return(XST_FAILURE);
    }

    ConfigPtr->AxiLiteClkFreq = HdmiRxSs1Ptr->Config.AxiLiteClkFreq;

    /* Initialize core */
    Status = XV_HdmiRx1_CfgInitialize(HdmiRxSs1Ptr->HdmiRx1Ptr,
                                    ConfigPtr,
                                    HdmiRxSs1Ptr->Config.HdmiRx1.AbsAddr);

    if (Status != XST_SUCCESS)
    {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMIRXSS1 ERR:: HDMI RX Initialization failed\r\n");
      return(XST_FAILURE);
    }

    /* Load EDID*/
    XV_HdmiRx1_DdcLoadEdid(HdmiRxSs1Ptr->HdmiRx1Ptr, HdmiRxSs1Ptr->EdidPtr,
        HdmiRxSs1Ptr->EdidLength);

    /* set the video interface and Ppc */
    HdmiRxSs1Ptr->HdmiRx1Ptr->SubsysVidIntfc = HdmiRxSs1Ptr->Config.VideoInterface;
    HdmiRxSs1Ptr->HdmiRx1Ptr->SubsysPpc = HdmiRxSs1Ptr->Config.Ppc;
  }
  return(XST_SUCCESS);
}

#ifdef XPAR_XHDCP_NUM_INSTANCES
/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiRxSs1Ptr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiRxSs1_SubcoreInitHdcpTimer(XV_HdmiRxSs1 *HdmiRxSs1Ptr)
{
  int Status;
  XTmrCtr_Config *ConfigPtr;

  if (HdmiRxSs1Ptr->HdcpTimerPtr)
  {
    /* Get core configuration */
#ifdef XV_HDMIRXSS1_LOG_ENABLE
    XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_HDCPTIMER_INIT, 0);
#endif
    ConfigPtr  = XTmrCtr_LookupConfig(HdmiRxSs1Ptr->Config.HdcpTimer.DeviceId);
    if (ConfigPtr == NULL)
    {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMIRXSS1 ERR:: AXIS Timer device not found\r\n");
      return(XST_FAILURE);
    }

    /* Setup the instance */
    memset(HdmiRxSs1Ptr->HdcpTimerPtr, 0, sizeof(XTmrCtr));

    /* Initialize core */
    XTmrCtr_CfgInitialize(HdmiRxSs1Ptr->HdcpTimerPtr,
                          ConfigPtr,
                          HdmiRxSs1Ptr->Config.HdcpTimer.AbsAddr);


    Status = XTmrCtr_InitHw(HdmiRxSs1Ptr->HdcpTimerPtr);

    /* Set Timer Counter instance in HDCP to the generic Hdcp1xRef
       that will be used in callbacks */
    HdmiRxSs1Ptr->Hdcp14Ptr->Hdcp1xRef = (void *)HdmiRxSs1Ptr->HdcpTimerPtr;

    /* Initialize the hdcp timer functions */
    XHdcp1x_SetTimerStart(HdmiRxSs1Ptr->Hdcp14Ptr,
      &XV_HdmiRxSs1_HdcpTimerStart);
    XHdcp1x_SetTimerStop(HdmiRxSs1Ptr->Hdcp14Ptr,
      &XV_HdmiRxSs1_HdcpTimerStop);
    XHdcp1x_SetTimerDelay(HdmiRxSs1Ptr->Hdcp14Ptr,
      &XV_HdmiRxSs1_HdcpTimerBusyDelay);

    if (Status != XST_SUCCESS)
    {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMIRXSS1 ERR:: AXI Timer Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}
#endif

#ifdef XPAR_XHDCP_NUM_INSTANCES
/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiRxSs1Ptr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiRxSs1_SubcoreInitHdcp14(XV_HdmiRxSs1 *HdmiRxSs1Ptr)
{
  int Status;
  XHdcp1x_Config *ConfigPtr;

  /* Is the HDCP 1.4 RX present? */
  if (HdmiRxSs1Ptr->Hdcp14Ptr) {

    /* Is the key loaded? */
    if (HdmiRxSs1Ptr->Hdcp14KeyPtr) {

      /* Get core configuration */
#ifdef XV_HDMIRXSS1_LOG_ENABLE
      XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_HDCP14_INIT, 0);
#endif
      ConfigPtr  = XHdcp1x_LookupConfig(HdmiRxSs1Ptr->Config.Hdcp14.DeviceId);
      if (ConfigPtr == NULL)
      {
        xdbg_printf(XDBG_DEBUG_GENERAL,"HDMIRXSS1 ERR:: HDCP 1.4 device not found\r\n");
        return(XST_FAILURE);
      }

      /* Initialize core */
      void *PhyIfPtr = HdmiRxSs1Ptr->HdmiRx1Ptr;
      Status = XHdcp1x_CfgInitialize(HdmiRxSs1Ptr->Hdcp14Ptr,
                                        ConfigPtr,
                                        PhyIfPtr,
                                        HdmiRxSs1Ptr->Config.Hdcp14.AbsAddr);

      /* Self-test the hdcp interface */
      if (XHdcp1x_SelfTest(HdmiRxSs1Ptr->Hdcp14Ptr) != XST_SUCCESS) {
          Status = XST_FAILURE;
      }

      if (Status != XST_SUCCESS)
      {
        xdbg_printf(XDBG_DEBUG_GENERAL,"HDMIRXSS1 ERR:: HDCP 1.4 Initialization failed\r\n");
        return(XST_FAILURE);
      }

      /* Set-up the DDC Handlers */
      XHdcp1x_SetCallback(HdmiRxSs1Ptr->Hdcp14Ptr,
                          XHDCP1X_HANDLER_DDC_SETREGADDR,
						  (void *)(XHdcp1x_SetDdcHandler)XV_HdmiRxSs1_DdcSetRegAddrHandler,
						  (void *)HdmiRxSs1Ptr->HdmiRx1Ptr);

      XHdcp1x_SetCallback(HdmiRxSs1Ptr->Hdcp14Ptr,
                          XHDCP1X_HANDLER_DDC_SETREGDATA,
						  (void *)(XHdcp1x_SetDdcHandler)XV_HdmiRxSs1_DdcSetRegDataHandler,
						  (void *)HdmiRxSs1Ptr->HdmiRx1Ptr);

      XHdcp1x_SetCallback(HdmiRxSs1Ptr->Hdcp14Ptr,
                          XHDCP1X_HANDLER_DDC_GETREGDATA,
						  (void *)(XHdcp1x_GetDdcHandler)XV_HdmiRxSs1_DdcGetRegDataHandler,
						  (void *)HdmiRxSs1Ptr->HdmiRx1Ptr);

      /* Select key */
      XHdcp1x_SetKeySelect(HdmiRxSs1Ptr->Hdcp14Ptr, XV_HDMIRXSS1_HDCP_KEYSEL);

      /* Disable HDCP 1.4 repeater */
      HdmiRxSs1Ptr->Hdcp14Ptr->IsRepeater = 0;

      /* Set-up the HDMI RX HDCP Callback Handler */
      XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                XV_HDMIRX1_HANDLER_HDCP,
				(void *)XV_HdmiRxSs1_DdcHdcpCallback,
                (void *)HdmiRxSs1Ptr);

      /* Set-up the HDMI RX HDCP 1.4 Protocol Event Callback Handler */
	  XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
			    XV_HDMIRX1_HANDLER_DDC_HDCP_14_PROT,
				(void *)XV_HdmiRxSs1_DdcHdcp14ProtocolEvtCallback,
				(void *)HdmiRxSs1Ptr);

      /* Enable HDMI-RX DDC interrupts */
      XV_HdmiRx1_DdcIntrEnable(HdmiRxSs1Ptr->HdmiRx1Ptr);

      /* Enable HDMI-RX HDCP */
      XV_HdmiRx1_DdcHdcpEnable(HdmiRxSs1Ptr->HdmiRx1Ptr);

      /* Clear the HDCP KSV Fifo */
      XV_HdmiRx1_DdcHdcpClearReadMessageBuffer(HdmiRxSs1Ptr->HdmiRx1Ptr);

      /* Clear the event queue */
      XV_HdmiRxSs1_HdcpClearEvents(HdmiRxSs1Ptr);

      XHdcp1x_LateInit(HdmiRxSs1Ptr->Hdcp14Ptr);
    }
  }
  return(XST_SUCCESS);
}
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiRxSs1Ptr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiRxSs1_SubcoreInitHdcp22(XV_HdmiRxSs1 *HdmiRxSs1Ptr)
{
  int Status;
  XHdcp22_Rx_Config *ConfigPtr;

  /* Is the HDCP 2.2 RX present? */
  if (HdmiRxSs1Ptr->Hdcp22Ptr) {

    /* Are the keys loaded? */
    if (HdmiRxSs1Ptr->Hdcp22Lc128Ptr && HdmiRxSs1Ptr->Hdcp22PrivateKeyPtr) {

      /* Get core configuration */
#ifdef XV_HDMIRXSS1_LOG_ENABLE
      XV_HdmiRxSs1_LogWrite(HdmiRxSs1Ptr, XV_HDMIRXSS1_LOG_EVT_HDCP22_INIT, 0);
#endif
      ConfigPtr  = XHdcp22Rx_LookupConfig(HdmiRxSs1Ptr->Config.Hdcp22.DeviceId);
      if (ConfigPtr == NULL)
      {
        xdbg_printf(XDBG_DEBUG_GENERAL,"HDMIRXSS1 ERR:: HDCP 2.2 device not found\r\n");
        return (XST_FAILURE);
      }

      /* Initialize core */
      Status = XHdcp22Rx_CfgInitialize(HdmiRxSs1Ptr->Hdcp22Ptr,
                                       ConfigPtr,
                                       HdmiRxSs1Ptr->Config.Hdcp22.AbsAddr);
      if (Status != XST_SUCCESS)
      {
        xdbg_printf(XDBG_DEBUG_GENERAL,"HDMIRXSS1 ERR:: HDCP 2.2 Initialization failed\r\n");
        return(XST_FAILURE);
      }

      /* Set-up the DDC Handlers */
      XHdcp22Rx_SetCallback(HdmiRxSs1Ptr->Hdcp22Ptr,
                            XHDCP22_RX_HANDLER_DDC_SETREGADDR,
							(void *)(XHdcp22_Rx_SetHandler)XV_HdmiRxSs1_DdcSetRegAddrHandler,
							(void *)HdmiRxSs1Ptr->HdmiRx1Ptr);

      XHdcp22Rx_SetCallback(HdmiRxSs1Ptr->Hdcp22Ptr,
                            XHDCP22_RX_HANDLER_DDC_SETREGDATA,
							(void *)(XHdcp22_Rx_SetHandler)XV_HdmiRxSs1_DdcSetRegDataHandler,
							(void *)HdmiRxSs1Ptr->HdmiRx1Ptr);

      XHdcp22Rx_SetCallback(HdmiRxSs1Ptr->Hdcp22Ptr,
                            XHDCP22_RX_HANDLER_DDC_GETREGDATA,
							(void *)(XHdcp22_Rx_GetHandler)XV_HdmiRxSs1_DdcGetRegDataHandler,
							(void *)HdmiRxSs1Ptr->HdmiRx1Ptr);

      XHdcp22Rx_SetCallback(HdmiRxSs1Ptr->Hdcp22Ptr,
                            XHDCP22_RX_HANDLER_DDC_GETWBUFSIZE,
							(void *)(XHdcp22_Rx_GetHandler)XV_HdmiRxSs1_DdcGetWriteMessageBufferWordsHandler,
							(void *)HdmiRxSs1Ptr->HdmiRx1Ptr);

      XHdcp22Rx_SetCallback(HdmiRxSs1Ptr->Hdcp22Ptr,
                            XHDCP22_RX_HANDLER_DDC_GETRBUFSIZE,
							(void *)(XHdcp22_Rx_GetHandler)XV_HdmiRxSs1_DdcGetReadMessageBufferWordsHandler,
							(void *)HdmiRxSs1Ptr->HdmiRx1Ptr);

      XHdcp22Rx_SetCallback(HdmiRxSs1Ptr->Hdcp22Ptr,
                            XHDCP22_RX_HANDLER_DDC_ISWBUFEMPTY,
							(void *)(XHdcp22_Rx_GetHandler)XV_HdmiRxSs1_DdcIsWriteMessageBufferEmptyHandler,
							(void *)HdmiRxSs1Ptr->HdmiRx1Ptr);

      XHdcp22Rx_SetCallback(HdmiRxSs1Ptr->Hdcp22Ptr,
                            XHDCP22_RX_HANDLER_DDC_ISRBUFEMPTY,
							(void *)(XHdcp22_Rx_GetHandler)XV_HdmiRxSs1_DdcIsReadMessageBufferEmptyHandler,
							(void *)HdmiRxSs1Ptr->HdmiRx1Ptr);

      XHdcp22Rx_SetCallback(HdmiRxSs1Ptr->Hdcp22Ptr,
                            XHDCP22_RX_HANDLER_DDC_CLEARRBUF,
							(void *)(XHdcp22_Rx_RunHandler)XV_HdmiRxSs1_DdcClearReadMessageBufferHandler,
							(void *)HdmiRxSs1Ptr->HdmiRx1Ptr);

      XHdcp22Rx_SetCallback(HdmiRxSs1Ptr->Hdcp22Ptr,
                            XHDCP22_RX_HANDLER_DDC_CLEARWBUF,
							(void *)(XHdcp22_Rx_RunHandler)XV_HdmiRxSs1_DdcClearWriteMessageBufferHandler,
							(void *)HdmiRxSs1Ptr->HdmiRx1Ptr);


      /* Set-up the HDMI RX HDCP Callback Handler */
      XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                XV_HDMIRX1_HANDLER_HDCP,
				(void *)XV_HdmiRxSs1_DdcHdcpCallback,
                (void *)HdmiRxSs1Ptr);

      /* Set-up the HDMI RX HDCP 2.2 Protocol Event Callback Handler */
	  XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
				XV_HDMIRX1_HANDLER_DDC_HDCP_22_PROT,
				(void *)XV_HdmiRxSs1_DdcHdcp22ProtocolEvtCallback,
				(void *)HdmiRxSs1Ptr);

      /* Set-up the HDMI RX Link error Callback Handler */
      XV_HdmiRx1_SetCallback(HdmiRxSs1Ptr->HdmiRx1Ptr,
                XV_HDMIRX1_HANDLER_LINK_ERROR,
				(void *)XV_HdmiRxSs1_LinkErrorCallback,
                (void *)HdmiRxSs1Ptr);

      /* Load Production Keys */
      XHdcp22Rx_LoadLc128(HdmiRxSs1Ptr->Hdcp22Ptr, HdmiRxSs1Ptr->Hdcp22Lc128Ptr);
      XHdcp22Rx_LoadPublicCert(HdmiRxSs1Ptr->Hdcp22Ptr, HdmiRxSs1Ptr->Hdcp22PrivateKeyPtr+40);
      XHdcp22Rx_LoadPrivateKey(HdmiRxSs1Ptr->Hdcp22Ptr, HdmiRxSs1Ptr->Hdcp22PrivateKeyPtr+562);
#ifdef XV_HDMIRXSS1_LOG_ENABLE
      XHdcp22Rx_LogReset(HdmiRxSs1Ptr->Hdcp22Ptr, FALSE);
#endif
      /* Enable HDMI-RX DDC interrupts */
      XV_HdmiRx1_DdcIntrEnable(HdmiRxSs1Ptr->HdmiRx1Ptr);

      /* Enable HDMI-RX HDCP */
      XV_HdmiRx1_DdcHdcpEnable(HdmiRxSs1Ptr->HdmiRx1Ptr);

      /* Clear the event queue */
      XV_HdmiRxSs1_HdcpClearEvents(HdmiRxSs1Ptr);

      /* Default enable broadcasting */
      XHdcp22Rx_SetBroadcast(HdmiRxSs1Ptr->Hdcp22Ptr, TRUE);
    }
  }

  return (XST_SUCCESS);
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
 *
 * This is the DDC set register address handler for the RX.
 *
 * @param RefPtr is a callback reference to the HDMI RX instance.
 *
 * @param Data is the address to be written.
 *
 * @return None.
 *
 ******************************************************************************/
static void XV_HdmiRxSs1_DdcSetRegAddrHandler(void *RefPtr, u32 Data)
{
  XV_HdmiRx1 *InstancePtr = (XV_HdmiRx1 *)RefPtr;
  XV_HdmiRx1_DdcHdcpSetAddress(InstancePtr, Data);
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
 *
 * This is the DDC set register data handler for the RX.
 *
 * @param RefPtr is a callback reference to the HDMI RX instance.
 *
 * @param Data is the data to be written.
 *
 * @return None.
 *
 ******************************************************************************/
static void XV_HdmiRxSs1_DdcSetRegDataHandler(void *RefPtr, u32 Data)
{
  XV_HdmiRx1 *InstancePtr = (XV_HdmiRx1 *)RefPtr;
  XV_HdmiRx1_DdcHdcpWriteData(InstancePtr, Data);
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
 *
 * This is the DDC get register data handler for the RX.
 *
 * @param RefPtr is a callback reference to the HDMI RX instance.
 *
 * @return The read data.
 *
 ******************************************************************************/
static u32 XV_HdmiRxSs1_DdcGetRegDataHandler(void *RefPtr)
{
  XV_HdmiRx1 *InstancePtr = (XV_HdmiRx1 *)RefPtr;
  return XV_HdmiRx1_DdcHdcpReadData(InstancePtr);
}
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
/*****************************************************************************/
/**
 *
 * This is the DDC get write message buffer words handler for the RX.
 *
 * @param RefPtr is a callback reference to the HDMI RX instance.
 *
 * @return The number of words in the Write Message Buffer.
 *
 ******************************************************************************/
static u32 XV_HdmiRxSs1_DdcGetWriteMessageBufferWordsHandler(void *RefPtr)
{
  XV_HdmiRx1 *InstancePtr = (XV_HdmiRx1 *)RefPtr;
  return XV_HdmiRx1_DdcGetHdcpWriteMessageBufferWords(InstancePtr);
}
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
/*****************************************************************************/
/**
 *
 * This is the DDC get read message buffer words handler for the RX.
 *
 * @param RefPtr is a callback reference to the HDMI RX instance.
 *
 * @return The number of words in the Read Message Buffer.
 *
 ******************************************************************************/
static u32 XV_HdmiRxSs1_DdcGetReadMessageBufferWordsHandler(void *RefPtr)
{
  XV_HdmiRx1 *InstancePtr = (XV_HdmiRx1 *)RefPtr;
  return XV_HdmiRx1_DdcGetHdcpReadMessageBufferWords(InstancePtr);
}
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
/*****************************************************************************/
/**
 *
 * This is the DDC get read message buffer is empty handler for the RX.
 *
 * @param RefPtr is a callback reference to the HDMI RX instance.
 *
 * @return
 *  - TRUE if read message buffer is empty.
 *  - FALSE if read message buffer is not empty.
 *
 ******************************************************************************/
static u32 XV_HdmiRxSs1_DdcIsReadMessageBufferEmptyHandler(void *RefPtr)
{
  XV_HdmiRx1 *InstancePtr = (XV_HdmiRx1 *)RefPtr;
  return XV_HdmiRx1_DdcIsHdcpReadMessageBufferEmpty(InstancePtr);
}
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
/*****************************************************************************/
/**
 *
 * This is the DDC get write message buffer is empty handler for the RX.
 *
 * @param RefPtr is a callback reference to the HDMI RX instance.
 *
 * @return
 *  - TRUE if write message buffer is empty.
 *  - FALSE if write message buffer is not empty.
 *
 ******************************************************************************/
static u32 XV_HdmiRxSs1_DdcIsWriteMessageBufferEmptyHandler(void *RefPtr)
{
  XV_HdmiRx1 *InstancePtr = (XV_HdmiRx1 *)RefPtr;
  return XV_HdmiRx1_DdcIsHdcpWriteMessageBufferEmpty(InstancePtr);
}
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
/*****************************************************************************/
/**
 *
 * This is the DDC clear read message buffer handler for the RX.
 *
 * @param RefPtr is a callback reference to the HDMI RX instance.
 *
 * @return None
 *
 ******************************************************************************/
static void XV_HdmiRxSs1_DdcClearReadMessageBufferHandler(void *RefPtr)
{
  XV_HdmiRx1 *InstancePtr = (XV_HdmiRx1 *)RefPtr;
  XV_HdmiRx1_DdcHdcpClearReadMessageBuffer(InstancePtr);
}
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
/*****************************************************************************/
/**
 *
 * This is the DDC clear write message buffer for the RX.
 *
 * @param RefPtr is a callback reference to the HDMI RX instance.
 *
 * @return None
 *
 ******************************************************************************/
static void XV_HdmiRxSs1_DdcClearWriteMessageBufferHandler(void *RefPtr)
{
  XV_HdmiRx1 *InstancePtr = (XV_HdmiRx1 *)RefPtr;
  XV_HdmiRx1_DdcHdcpClearWriteMessageBuffer(InstancePtr);
}
#endif

#ifdef USE_HDCP_RX
/*****************************************************************************/
/**
* This function is called when the HDMI-RX DDC HDCP interrupt has occurred.
*
* @param RefPtr is a callback reference to the HDCP22 RX instance.
* @param Type indicates the cause of the interrupt.
*
* @return None.
*
* @note   None.
******************************************************************************/
static void XV_HdmiRxSs1_DdcHdcpCallback(void *RefPtr, int Type)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr;
  HdmiRxSs1Ptr = (XV_HdmiRxSs1*) RefPtr;

  switch (Type)
  {
    /* HDCP 2.2. write message event*/
    case XV_HDMIRX1_DDC_STA_HDCP_WMSG_NEW_EVT_MASK:
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
      if (HdmiRxSs1Ptr->Hdcp22Ptr) {
        XHdcp22Rx_SetWriteMessageAvailable(HdmiRxSs1Ptr->Hdcp22Ptr);
      }
#endif
      break;

    /* HDCP 2.2 read message event*/
    case XV_HDMIRX1_DDC_STA_HDCP_RMSG_END_EVT_MASK:
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
      if (HdmiRxSs1Ptr->Hdcp22Ptr) {
        XHdcp22Rx_SetReadMessageComplete(HdmiRxSs1Ptr->Hdcp22Ptr);
      }
#endif
      break;

    /* HDCP 2.2 read not complete event*/
    case XV_HDMIRX1_DDC_STA_HDCP_RMSG_NC_EVT_MASK:
#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
      if (HdmiRxSs1Ptr->Hdcp22Ptr) {
        XHdcp22Rx_SetDdcError(HdmiRxSs1Ptr->Hdcp22Ptr);
      }
#endif
      break;

    /* HDCP 1.4 Aksv event*/
    case XV_HDMIRX1_DDC_STA_HDCP_AKSV_EVT_MASK:
#ifdef XPAR_XHDCP_NUM_INSTANCES
      if (HdmiRxSs1Ptr->Hdcp14Ptr) {
        XHdcp1x_ProcessAKsv(HdmiRxSs1Ptr->Hdcp14Ptr);
      }
#endif
      break;

    default:
      break;
  }
}

/*****************************************************************************/
/**
* This function is called when the HDMI-RX DDC HDCP 1.4 Protocol Event
* interrupt has occurred.
*
* @param RefPtr is a callback reference to the HDCP22 RX instance.
*
* @return None.
*
* @note   None.
******************************************************************************/
static void XV_HdmiRxSs1_DdcHdcp14ProtocolEvtCallback(void *RefPtr)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr;
  HdmiRxSs1Ptr = (XV_HdmiRxSs1*) RefPtr;

  /* Enable HDCP 1.4 */
#if defined(XPAR_XHDCP_NUM_INSTANCES) && defined(XPAR_XHDCP22_RX_NUM_INSTANCES)
  if (HdmiRxSs1Ptr->Hdcp14Ptr && HdmiRxSs1Ptr->Hdcp22Ptr) {
	if (XV_HdmiRxSs1_HdcpSetProtocol(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_14) !=
	    XST_SUCCESS) {
	  XV_HdmiRxSs1_HdcpSetProtocol(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_22);
	}
  }
#endif
}
#endif

#ifdef XPAR_XHDCP22_RX_NUM_INSTANCES
/*****************************************************************************/
/**
* This function is called when the HDMI-RX link error has occurred.
*
* @param RefPtr is a callback reference to the HDCP22 RX instance.
*
* @return None.
*
* @note   None.
******************************************************************************/
static void XV_HdmiRxSs1_LinkErrorCallback(void *RefPtr)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr;
  HdmiRxSs1Ptr = (XV_HdmiRxSs1*) RefPtr;

  /* HDCP 2.2*/
  if (HdmiRxSs1Ptr->Hdcp22Ptr) {
    if (HdmiRxSs1Ptr->HdcpProtocol == XV_HDMIRXSS1_HDCP_22) {
      XHdcp22Rx_SetLinkError(HdmiRxSs1Ptr->Hdcp22Ptr);
    }
  }
}

/*****************************************************************************/
/**
* This function is called when the HDMI-RX DDC HDCP 2.2 Protocol Event
* interrupt has occurred.
*
* @param RefPtr is a callback reference to the HDCP22 RX instance.
*
* @return None.
*
* @note   None.
******************************************************************************/
static void XV_HdmiRxSs1_DdcHdcp22ProtocolEvtCallback(void *RefPtr)
{
  XV_HdmiRxSs1 *HdmiRxSs1Ptr;
  HdmiRxSs1Ptr = (XV_HdmiRxSs1*) RefPtr;

  /* Enable HDCP 2.2 */
#if defined(XPAR_XHDCP_NUM_INSTANCES) && defined(XPAR_XHDCP22_RX_NUM_INSTANCES)
  if (HdmiRxSs1Ptr->Hdcp14Ptr && HdmiRxSs1Ptr->Hdcp22Ptr) {
	if (XV_HdmiRxSs1_HdcpSetProtocol(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_22) !=
		XST_SUCCESS) {
	  XV_HdmiRxSs1_HdcpSetProtocol(HdmiRxSs1Ptr, XV_HDMIRXSS1_HDCP_14);
	}
  }
#endif
}
#endif

/** @} */
