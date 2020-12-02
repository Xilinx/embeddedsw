/******************************************************************************
* Copyright (C) 2018 – 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xv_hdmitxss1_coreinit.c
* @addtogroup v_hdmitxss1_v2_0
* @{
* @details

* HDMI TX Subsystem Sub-Cores initialization
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
#include "xv_hdmitxss1_coreinit.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
#ifdef USE_HDCP_TX
static int XV_HdmiTxSs1_DdcReadHandler(u8 DeviceAddress,
    u16 ByteCount, u8* BufferPtr, u8 Stop, void *RefPtr);
static int XV_HdmiTxSs1_DdcWriteHandler(u8 DeviceAddress,
    u16 ByteCount, u8* BufferPtr, u8 Stop, void *RefPtr);
#endif

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiTxSs1Ptr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs1_SubcoreInitHdmiTx1(XV_HdmiTxSs1 *HdmiTxSs1Ptr)
{
  int Status;
  XV_HdmiTx1_Config *ConfigPtr;

  if (HdmiTxSs1Ptr->HdmiTx1Ptr) {
    /* Get core configuration */
#ifdef XV_HDMITXSS1_LOG_ENABLE
    XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_HDMITX1_INIT, 0);
#endif
    ConfigPtr  = XV_HdmiTx1_LookupConfig(HdmiTxSs1Ptr->Config.HdmiTx1.DeviceId);
    if (ConfigPtr == NULL) {
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "HDMITXSS1 ERR:: HDMI TX device not found\r\n");
      return(XST_FAILURE);
    }

    ConfigPtr->AxiLiteClkFreq = HdmiTxSs1Ptr->Config.AxiLiteClkFreq;

    /* Initialize core */
    Status = XV_HdmiTx1_CfgInitialize(HdmiTxSs1Ptr->HdmiTx1Ptr,
                                    ConfigPtr,
                                    HdmiTxSs1Ptr->Config.HdmiTx1.AbsAddr);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "HDMITXSS1 ERR:: HDMI TX Initialization failed\r\n");
      return(XST_FAILURE);
    }

    /* Set default transmitter mode to HDMI */
    XV_HdmiTxSS1_SetHdmiTmdsMode(HdmiTxSs1Ptr);
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiTxSs1Ptr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs1_SubcoreInitVtc(XV_HdmiTxSs1 *HdmiTxSs1Ptr)
{
  int Status;
  XVtc_Config *ConfigPtr;

  if (HdmiTxSs1Ptr->VtcPtr) {
    /* Get core configuration */
#ifdef XV_HDMITXSS1_LOG_ENABLE
    XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_VTC_INIT, 0);
#endif
    ConfigPtr  = XVtc_LookupConfig(HdmiTxSs1Ptr->Config.Vtc.DeviceId);
    if (ConfigPtr == NULL) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS1 ERR:: VTC device not found\r\n");
      return(XST_FAILURE);
    }

    /* Initialize core */
    Status = XVtc_CfgInitialize(HdmiTxSs1Ptr->VtcPtr,
                                ConfigPtr,
                                HdmiTxSs1Ptr->Config.Vtc.AbsAddr);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "HDMITXSS1 ERR:: VTC Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

#ifdef XPAR_XHDCP_NUM_INSTANCES
/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiTxSs1Ptr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs1_SubcoreInitHdcpTimer(XV_HdmiTxSs1 *HdmiTxSs1Ptr)
{
  int Status;
  XTmrCtr_Config *ConfigPtr;

  if (HdmiTxSs1Ptr->HdcpTimerPtr) {
    /* Get core configuration */
#ifdef XV_HDMITXSS1_LOG_ENABLE
    XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_HDCPTIMER_INIT, 0);
#endif
    ConfigPtr  = XTmrCtr_LookupConfig(HdmiTxSs1Ptr->Config.HdcpTimer.DeviceId);
    if (ConfigPtr == NULL) {
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "HDMITXSS1 ERR:: AXIS Timer device not found\r\n");
      return(XST_FAILURE);
    }

    /* Setup the instance */
    memset(HdmiTxSs1Ptr->HdcpTimerPtr, 0, sizeof(XTmrCtr));

    /* Initialize core */
    XTmrCtr_CfgInitialize(HdmiTxSs1Ptr->HdcpTimerPtr,
                          ConfigPtr,
                          HdmiTxSs1Ptr->Config.HdcpTimer.AbsAddr);

    Status = XTmrCtr_InitHw(HdmiTxSs1Ptr->HdcpTimerPtr);

    /* Set Timer Counter instance in HDCP to the generic Hdcp1xRef
     * that will be used in callbacks */
    HdmiTxSs1Ptr->Hdcp14Ptr->Hdcp1xRef = (void *)HdmiTxSs1Ptr->HdcpTimerPtr;

    /* Initialize the hdcp timer functions */
    XHdcp1x_SetTimerStart(HdmiTxSs1Ptr->Hdcp14Ptr,
      &XV_HdmiTxSs1_HdcpTimerStart);
    XHdcp1x_SetTimerStop(HdmiTxSs1Ptr->Hdcp14Ptr,
      &XV_HdmiTxSs1_HdcpTimerStop);
    XHdcp1x_SetTimerDelay(HdmiTxSs1Ptr->Hdcp14Ptr,
      &XV_HdmiTxSs1_HdcpTimerBusyDelay);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,
                  "HDMITXSS1 ERR:: AXI Timer Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiTxSs1Ptr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs1_SubcoreInitHdcp14(XV_HdmiTxSs1 *HdmiTxSs1Ptr)
{
  int Status;
  XHdcp1x_Config *ConfigPtr;

  /* Is the HDCP 1.4 TX present? */
  if (HdmiTxSs1Ptr->Hdcp14Ptr) {

    /* Is the key loaded? */
    if (HdmiTxSs1Ptr->Hdcp14KeyPtr) {

      /* Get core configuration */
#ifdef XV_HDMITXSS1_LOG_ENABLE
      XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_HDCP14_INIT, 0);
#endif
      ConfigPtr  = XHdcp1x_LookupConfig(HdmiTxSs1Ptr->Config.Hdcp14.DeviceId);
      if (ConfigPtr == NULL){
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "HDMITXSS1 ERR:: HDCP 1.4 device not found\r\n");
        return(XST_FAILURE);
      }

      /* Initialize core */
      void *PhyIfPtr = HdmiTxSs1Ptr->HdmiTx1Ptr;

      Status = XHdcp1x_CfgInitialize(HdmiTxSs1Ptr->Hdcp14Ptr,
                                        ConfigPtr,
                                        PhyIfPtr,
                                        HdmiTxSs1Ptr->Config.Hdcp14.AbsAddr);

      /* Self-test the hdcp interface */
      if (XHdcp1x_SelfTest(HdmiTxSs1Ptr->Hdcp14Ptr) != XST_SUCCESS) {
          Status = XST_FAILURE;
      }

      /* Set-up the DDC Handlers */
      XHdcp1x_SetCallback(HdmiTxSs1Ptr->Hdcp14Ptr,
                          XHDCP1X_HANDLER_DDC_WRITE,
                          (void *)XV_HdmiTxSs1_DdcWriteHandler,
						  (void *)HdmiTxSs1Ptr->HdmiTx1Ptr);

      XHdcp1x_SetCallback(HdmiTxSs1Ptr->Hdcp14Ptr,
                          XHDCP1X_HANDLER_DDC_READ,
						  (void *)XV_HdmiTxSs1_DdcReadHandler,
						  (void *)HdmiTxSs1Ptr->HdmiTx1Ptr);

      if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "HDMITXSS1 ERR:: HDCP 1.4 Initialization failed\r\n");
        return(XST_FAILURE);
      }

      /* Key select */
      XHdcp1x_SetKeySelect(HdmiTxSs1Ptr->Hdcp14Ptr, XV_HDMITXSS1_HDCP_KEYSEL);

      /* Load SRM */


      /* Disable HDCP 1.4 repeater */
      HdmiTxSs1Ptr->Hdcp14Ptr->IsRepeater = 0;
    }
  }
  return(XST_SUCCESS);
}
#endif

#ifdef XPAR_XHDCP22_TX_NUM_INSTANCES
/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiTxSs1Ptr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs1_SubcoreInitHdcp22(XV_HdmiTxSs1 *HdmiTxSs1Ptr)
{
  int Status;
  XHdcp22_Tx_Config *Hdcp22TxConfig;

  /* Is the HDCP 2.2 TX present? */
  if (HdmiTxSs1Ptr->Hdcp22Ptr) {

    /* Is the key loaded? */
    if (HdmiTxSs1Ptr->Hdcp22Lc128Ptr && HdmiTxSs1Ptr->Hdcp22SrmPtr) {

      /* Get core configuration */
#ifdef XV_HDMITXSS1_LOG_ENABLE
      XV_HdmiTxSs1_LogWrite(HdmiTxSs1Ptr, XV_HDMITXSS1_LOG_EVT_HDCP22_INIT, 0);
#endif
      /* Initialize HDCP 2.2 TX */
      Hdcp22TxConfig =
                    XHdcp22Tx_LookupConfig(HdmiTxSs1Ptr->Config.Hdcp22.DeviceId);

      if (Hdcp22TxConfig == NULL) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "HDMITXSS1 ERR:: HDCP 2.2 device not found\r\n");
        return XST_FAILURE;
      }

      Status = XHdcp22Tx_CfgInitialize(HdmiTxSs1Ptr->Hdcp22Ptr,
                                       Hdcp22TxConfig,
                                       HdmiTxSs1Ptr->Config.Hdcp22.AbsAddr);
      if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "HDMITXSS1 ERR:: HDCP 2.2 Initialization failed\r\n");
        return Status;
      }

      /* Set-up the DDC Handlers */
      XHdcp22Tx_SetCallback(HdmiTxSs1Ptr->Hdcp22Ptr,
                            XHDCP22_TX_HANDLER_DDC_WRITE,
							(void *)XV_HdmiTxSs1_DdcWriteHandler,
							(void *)HdmiTxSs1Ptr->HdmiTx1Ptr);

      XHdcp22Tx_SetCallback(HdmiTxSs1Ptr->Hdcp22Ptr,
                            XHDCP22_TX_HANDLER_DDC_READ,
							(void *)XV_HdmiTxSs1_DdcReadHandler,
							(void *)HdmiTxSs1Ptr->HdmiTx1Ptr);

      /* Set polling value */
      XHdcp22Tx_SetMessagePollingValue(HdmiTxSs1Ptr->Hdcp22Ptr, 10);

      XHdcp22Tx_LogReset(HdmiTxSs1Ptr->Hdcp22Ptr, FALSE);

      /* Load key */
      XHdcp22Tx_LoadLc128(HdmiTxSs1Ptr->Hdcp22Ptr, HdmiTxSs1Ptr->Hdcp22Lc128Ptr);

      /* Load SRM */
      Status = XHdcp22Tx_LoadRevocationTable(HdmiTxSs1Ptr->Hdcp22Ptr,
                                             HdmiTxSs1Ptr->Hdcp22SrmPtr);
      if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL,
                    "HDMITXSS1 ERR:: HDCP 2.2 failed to load SRM\r\n");
        return Status;
      }

      /* Clear the event queue */
      XV_HdmiTxSs1_HdcpClearEvents(HdmiTxSs1Ptr);
    }
  }

  return (XST_SUCCESS);
}
#endif


#ifdef USE_HDCP_TX
/*****************************************************************************/
/**
 *
 * This is the DDC read handler for the TX.
 *
 * @param DeviceAddress is the 7-bit I2C slave address
 *
 * @param ByteCount is the number of bytes to read
 *
 * @param BufferPtr is a pointer to the buffer where
 *        the read data is written to.
 *
 * @param Stop indicates if a I2C stop condition is generated
 *        at the end of the burst.
 *
 * @param RefPtr is a callback reference to the HDMI TX instance
 *
 * @return
 *  - XST_SUCCESS if action was successful
 *  - XST_FAILURE if action was not successful
 *
 ******************************************************************************/
static int XV_HdmiTxSs1_DdcReadHandler(u8 DeviceAddress,
    u16 ByteCount, u8* BufferPtr, u8 Stop, void *RefPtr)
{
  XV_HdmiTx1 *InstancePtr = (XV_HdmiTx1 *)RefPtr;
  return XV_HdmiTx1_DdcRead(InstancePtr,
                           DeviceAddress,
                           ByteCount,
                           BufferPtr,
                           Stop);
}

/*****************************************************************************/
/**
 *
 * This is the DDC write handler for the TX.
 *
 * @param DeviceAddress is the 7-bit I2C slave address
 *
 * @param ByteCount is the number of bytes to write
 *
 * @param BufferPtr is a pointer to the buffer containing
 *        the data to be written.
 *
 * @param Stop indicates if a I2C stop condition is generated
 *        at the end of the burst.
 *
 * @param RefPtr is a callback reference to the HDMI TX instance
 *
 * @return
 *  - XST_SUCCESS if action was successful
 *  - XST_FAILURE if action was not successful
 *
 ******************************************************************************/
static int XV_HdmiTxSs1_DdcWriteHandler(u8 DeviceAddress,
    u16 ByteCount, u8* BufferPtr, u8 Stop, void *RefPtr)
{
  XV_HdmiTx1 *InstancePtr = (XV_HdmiTx1 *)RefPtr;
  return XV_HdmiTx1_DdcWrite(InstancePtr,
                            DeviceAddress,
                            ByteCount,
                            BufferPtr,
                            Stop);
}
#endif

/** @} */
