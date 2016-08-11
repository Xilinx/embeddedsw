/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xv_hdmitxss_coreinit.c
* @addtogroup v_hdmitxss
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
* 1.00         10/07/15 Initial release.
* 1.1   yh     20/01/16 Added remapper support
* 1.2   MG     03/02/16 Added HDCP support
* 1.3   MH     23/04/16 VTC driver has been updated to avoid processor
*                       exceptions. Workarounds have been removed.
* 1.4   MH     23/06/16 Added HDCP repeater support.
* 1.5   YH     18/07/16 Replace xil_printf with xdbg_printf
* 1.6   YH     25/07/16 Used UINTPTR instead of u32 for BaseAddr,HighAddr,Offset
*                       AbsAddr
* 1.7   MH     08/08/16 Updates to optimize out HDCP when excluded.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_hdmitxss_coreinit.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
static int XV_HdmiTxSs_ComputeSubcoreAbsAddr(UINTPTR SubSys_BaseAddr,
    UINTPTR SubSys_HighAddr,
    UINTPTR SubCore_Offset,
    UINTPTR *SubCore_BaseAddr);
static int XV_HdmiTxSs_DdcReadHandler(u8 DeviceAddress,
    u16 ByteCount, u8* BufferPtr, u8 Stop, void *RefPtr);
static int XV_HdmiTxSs_DdcWriteHandler(u8 DeviceAddress,
    u16 ByteCount, u8* BufferPtr, u8 Stop, void *RefPtr);

/*****************************************************************************/
/**
* This function computes the subcore absolute address on axi-lite interface
* Subsystem is mapped at an absolute address and all included sub-cores are
* at pre-defined offset from the subsystem base address. To access the subcore
* register map from host CPU an absolute address is required.
* Subsystem has address range of 1MB (0x00000-0xFFFFF)
*
* @param  SubSys_BaseAddr is the base address of the the Subsystem instance
* @param  SubSys_HighAddr is the max address of the Subsystem instance
* @param  SubCore_Offset is the offset of the specified core
* @param  SubCore_BaseAddr is the computed absolute base address of the subcore
*
* @return XST_SUCCESS if base address computation is successful and within
*         subsystem address range else XST_FAILURE
*
******************************************************************************/
static int XV_HdmiTxSs_ComputeSubcoreAbsAddr(UINTPTR SubSys_BaseAddr,
    UINTPTR SubSys_HighAddr,
    UINTPTR SubCore_Offset,
    UINTPTR *SubCore_BaseAddr)
{
  int Status;
  UINTPTR AbsAddr;

  AbsAddr = SubSys_BaseAddr | SubCore_Offset;
  if ((AbsAddr>=SubSys_BaseAddr) && (AbsAddr<SubSys_HighAddr)) {
    *SubCore_BaseAddr = AbsAddr;
    Status = XST_SUCCESS;
  }
  else {
    *SubCore_BaseAddr = 0;
    Status = XST_FAILURE;
  }

  return(Status);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs_SubcoreInitHdmiTx(XV_HdmiTxSs *HdmiTxSsPtr)
{
  int Status;
  UINTPTR AbsAddr;
  XV_HdmiTx_Config *ConfigPtr;

  if (HdmiTxSsPtr->HdmiTxPtr) {
    /* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing HDMI TX core.... \r\n");
    ConfigPtr  = XV_HdmiTx_LookupConfig(HdmiTxSsPtr->Config.HdmiTx.DeviceId);
    if (ConfigPtr == NULL) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: HDMI TX device not found\r\n");
      return(XST_FAILURE);
    }

    /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiTxSs_ComputeSubcoreAbsAddr(HdmiTxSsPtr->Config.BaseAddress,
                               HdmiTxSsPtr->Config.HighAddress,
                               HdmiTxSsPtr->Config.HdmiTx.AddrOffset,
                               &AbsAddr);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: HDMI TX core base address (0x%x) \
        invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

    /* Initialize core */
    Status = XV_HdmiTx_CfgInitialize(HdmiTxSsPtr->HdmiTxPtr,
                                    ConfigPtr,
                                    AbsAddr);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: HDMI TX Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs_SubcoreInitVtc(XV_HdmiTxSs *HdmiTxSsPtr)
{
  int Status;
  UINTPTR AbsAddr;
  XVtc_Config *ConfigPtr;

  if (HdmiTxSsPtr->VtcPtr) {
    /* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing VTC core.... \r\n");
    ConfigPtr  = XVtc_LookupConfig(HdmiTxSsPtr->Config.Vtc.DeviceId);
    if (ConfigPtr == NULL) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: VTC device not found\r\n");
      return(XST_FAILURE);
    }

    /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiTxSs_ComputeSubcoreAbsAddr(HdmiTxSsPtr->Config.BaseAddress,
                               HdmiTxSsPtr->Config.HighAddress,
                               HdmiTxSsPtr->Config.Vtc.AddrOffset,
                               &AbsAddr);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: CSC core base address (0x%x) \
                            invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

    /* Initialize core */
    Status = XVtc_CfgInitialize(HdmiTxSsPtr->VtcPtr,
                                ConfigPtr,
                                AbsAddr);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: VTC Initialization failed\r\n");
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
* @param  HdmiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs_SubcoreInitHdcpTimer(XV_HdmiTxSs *HdmiTxSsPtr)
{
  int Status;
  UINTPTR AbsAddr;
  XTmrCtr_Config *ConfigPtr;

  if (HdmiTxSsPtr->HdcpTimerPtr) {
    /* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"   ->Initializing AXI Timer core.... \r\n");
    ConfigPtr  = XTmrCtr_LookupConfig(HdmiTxSsPtr->Config.HdcpTimer.DeviceId);
    if (ConfigPtr == NULL) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: AXIS Timer device not found\r\n");
      return(XST_FAILURE);
    }

    /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiTxSs_ComputeSubcoreAbsAddr(HdmiTxSsPtr->Config.BaseAddress,
                               HdmiTxSsPtr->Config.HighAddress,
                               HdmiTxSsPtr->Config.HdcpTimer.AddrOffset,
                               &AbsAddr);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: AXI Timer core base address (0x%x) \
                    invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

    /* Setup the instance */
    memset(HdmiTxSsPtr->HdcpTimerPtr, 0, sizeof(XTmrCtr));

    /* Initialize core */
    XTmrCtr_CfgInitialize(HdmiTxSsPtr->HdcpTimerPtr, ConfigPtr, AbsAddr);
    Status = XTmrCtr_InitHw(HdmiTxSsPtr->HdcpTimerPtr);

    /* Set Timer Counter instance in HDCP to the generic Hdcp1xRef
     * that will be used in callbacks */
    HdmiTxSsPtr->Hdcp14Ptr->Hdcp1xRef = (void *)HdmiTxSsPtr->HdcpTimerPtr;

    /* Initialize the hdcp timer functions */
    XHdcp1x_SetTimerStart(HdmiTxSsPtr->Hdcp14Ptr,
      &XV_HdmiTxSs_HdcpTimerStart);
    XHdcp1x_SetTimerStop(HdmiTxSsPtr->Hdcp14Ptr,
      &XV_HdmiTxSs_HdcpTimerStop);
    XHdcp1x_SetTimerDelay(HdmiTxSsPtr->Hdcp14Ptr,
      &XV_HdmiTxSs_HdcpTimerBusyDelay);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: AXI Timer Initialization failed\r\n");
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
* @param  HdmiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs_SubcoreInitHdcp14(XV_HdmiTxSs *HdmiTxSsPtr)
{
  int Status;
  UINTPTR AbsAddr;
  XHdcp1x_Config *ConfigPtr;

  /* Is the HDCP 1.4 TX present? */
  if (HdmiTxSsPtr->Hdcp14Ptr) {

    /* Is the key loaded? */
    if (HdmiTxSsPtr->Hdcp14KeyPtr) {

      /* Get core configuration */
      xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing HDCP 1.4 core.... \r\n");
      ConfigPtr  = XHdcp1x_LookupConfig(HdmiTxSsPtr->Config.Hdcp14.DeviceId);
      if (ConfigPtr == NULL){
        xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: HDCP 1.4 device not found\r\n");
        return(XST_FAILURE);
      }

      /* Compute absolute base address */
      AbsAddr = 0;
      Status = XV_HdmiTxSs_ComputeSubcoreAbsAddr(HdmiTxSsPtr->Config.BaseAddress,
                                 HdmiTxSsPtr->Config.HighAddress,
                                 HdmiTxSsPtr->Config.Hdcp14.AddrOffset,
                                 &AbsAddr);

      if (Status != XST_SUCCESS){
        xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: HDCP 1.4 core base address (0x%x) invalid %d\r\n",
          AbsAddr);
        return(XST_FAILURE);
      }

      /* Initialize core */
      void *PhyIfPtr = HdmiTxSsPtr->HdmiTxPtr;

      Status = XHdcp1x_CfgInitialize(HdmiTxSsPtr->Hdcp14Ptr,
                                        ConfigPtr,
                                        PhyIfPtr,
                                        AbsAddr);

      /* Self-test the hdcp interface */
      if (XHdcp1x_SelfTest(HdmiTxSsPtr->Hdcp14Ptr) != XST_SUCCESS) {
          Status = XST_FAILURE;
      }

      /* Set-up the DDC Handlers */
      XHdcp1x_SetCallback(HdmiTxSsPtr->Hdcp14Ptr, XHDCP1X_HANDLER_DDC_WRITE,
        XV_HdmiTxSs_DdcWriteHandler, HdmiTxSsPtr->HdmiTxPtr);
      XHdcp1x_SetCallback(HdmiTxSsPtr->Hdcp14Ptr, XHDCP1X_HANDLER_DDC_READ,
        XV_HdmiTxSs_DdcReadHandler, HdmiTxSsPtr->HdmiTxPtr);

      if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: HDCP 1.4 Initialization failed\r\n");
        return(XST_FAILURE);
      }

      /* Key select */
      XHdcp1x_SetKeySelect(HdmiTxSsPtr->Hdcp14Ptr, XV_HDMITXSS_HDCP_KEYSEL);

      /* Load SRM */
      // TODO: Load HDCP 1.4 SRM

      /* Disable HDCP 1.4 repeater */
      HdmiTxSsPtr->Hdcp14Ptr->IsRepeater = 0;
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
* @param  HdmiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs_SubcoreInitHdcp22(XV_HdmiTxSs *HdmiTxSsPtr)
{
  int Status;
  UINTPTR AbsAddr;
  XHdcp22_Tx_Config *Hdcp22TxConfig;

  /* Is the HDCP 2.2 RX present? */
  if (HdmiTxSsPtr->Hdcp22Ptr) {

    /* Is the key loaded? */
    if (HdmiTxSsPtr->Hdcp22Lc128Ptr && HdmiTxSsPtr->Hdcp22SrmPtr) {

      /* Get core configuration */
      xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing HDCP 2.2 core.... \r\n");

      /* Initialize HDCP 2.2 TX */
      Hdcp22TxConfig = XHdcp22Tx_LookupConfig(HdmiTxSsPtr->Config.Hdcp22.DeviceId);
      if (Hdcp22TxConfig == NULL) {
        xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: HDCP 2.2 device not found\r\n");
        return XST_FAILURE;
      }

      /* Compute absolute base address */
      AbsAddr = 0;
      Status = XV_HdmiTxSs_ComputeSubcoreAbsAddr(HdmiTxSsPtr->Config.BaseAddress,
                                 HdmiTxSsPtr->Config.HighAddress,
                                 HdmiTxSsPtr->Config.Hdcp22.AddrOffset,
                                 &AbsAddr);

      if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: HDCP 2.2 core base address (0x%x) invalid %d\r\n",
          AbsAddr);
        return(XST_FAILURE);
      }

      Status = XHdcp22Tx_CfgInitialize(HdmiTxSsPtr->Hdcp22Ptr, Hdcp22TxConfig, AbsAddr);
      if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: HDCP 2.2 Initialization failed\r\n");
        return Status;
      }

      /* Set-up the DDC Handlers */
      XHdcp22Tx_SetCallback(HdmiTxSsPtr->Hdcp22Ptr, XHDCP22_TX_HANDLER_DDC_WRITE,
        XV_HdmiTxSs_DdcWriteHandler, HdmiTxSsPtr->HdmiTxPtr);
      XHdcp22Tx_SetCallback(HdmiTxSsPtr->Hdcp22Ptr, XHDCP22_TX_HANDLER_DDC_READ,
        XV_HdmiTxSs_DdcReadHandler, HdmiTxSsPtr->HdmiTxPtr);

      /* Set polling value */
      XHdcp22Tx_SetMessagePollingValue(HdmiTxSsPtr->Hdcp22Ptr, 2);

      XHdcp22Tx_LogReset(HdmiTxSsPtr->Hdcp22Ptr, FALSE);

      /* Load key */
      XHdcp22Tx_LoadLc128(HdmiTxSsPtr->Hdcp22Ptr, HdmiTxSsPtr->Hdcp22Lc128Ptr);

      /* Load SRM */
      Status = XHdcp22Tx_LoadRevocationTable(HdmiTxSsPtr->Hdcp22Ptr, HdmiTxSsPtr->Hdcp22SrmPtr);
      if (Status != XST_SUCCESS) {
        xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: HDCP 2.2 failed to load SRM\r\n");
        return Status;
      }

      /* Clear the event queue */
      XV_HdmiTxSs_HdcpClearEvents(HdmiTxSsPtr);
    }
  }

  return (XST_SUCCESS);
}
#endif

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs_SubcoreInitRemapperReset(XV_HdmiTxSs *HdmiTxSsPtr)
{
  int Status;
  UINTPTR AbsAddr;
  XGpio_Config *ConfigPtr;

  if (HdmiTxSsPtr->RemapperResetPtr) {
    /* Get core configuration */
    ConfigPtr  = XGpio_LookupConfig(HdmiTxSsPtr->Config.RemapperReset.DeviceId);
    if (ConfigPtr == NULL) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: Reset module for Remapper not found\r\n");
      return(XST_FAILURE);
    }

    /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiTxSs_ComputeSubcoreAbsAddr(HdmiTxSsPtr->Config.BaseAddress,
                               HdmiTxSsPtr->Config.HighAddress,
                               HdmiTxSsPtr->Config.RemapperReset.AddrOffset,
                               &AbsAddr);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: Remapper Reset GPIO core base address (0x%x) \
                            invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

    /* Initialize core */
    Status = XGpio_CfgInitialize(HdmiTxSsPtr->RemapperResetPtr,
                                 ConfigPtr,
                                 AbsAddr);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: Remapper Reset Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiTxSs_SubcoreInitRemapper(XV_HdmiTxSs *HdmiTxSsPtr)
{
  int Status;
  UINTPTR AbsAddr;
  XV_axi4s_remap_Config *ConfigPtr;

  if (HdmiTxSsPtr->RemapperPtr) {
    /* Get core configuration */
    ConfigPtr  = XV_axi4s_remap_LookupConfig(HdmiTxSsPtr->Config.Remapper.DeviceId);
    if (ConfigPtr == NULL) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: Remapper not found\r\n");
      return(XST_FAILURE);
    }

    /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiTxSs_ComputeSubcoreAbsAddr(HdmiTxSsPtr->Config.BaseAddress,
                               HdmiTxSsPtr->Config.HighAddress,
                               HdmiTxSsPtr->Config.Remapper.AddrOffset,
                               &AbsAddr);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: Remapper core base address (0x%x) \
                            invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

    /* Initialize core */
    Status = XV_axi4s_remap_CfgInitialize(HdmiTxSsPtr->RemapperPtr,
                                 ConfigPtr,
                                 AbsAddr);

    if (Status != XST_SUCCESS) {
      xdbg_printf(XDBG_DEBUG_GENERAL,"HDMITXSS ERR:: Remapper Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);

}

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
static int XV_HdmiTxSs_DdcReadHandler(u8 DeviceAddress,
    u16 ByteCount, u8* BufferPtr, u8 Stop, void *RefPtr)
{
  XV_HdmiTx *InstancePtr = (XV_HdmiTx *)RefPtr;
  return XV_HdmiTx_DdcRead(InstancePtr, DeviceAddress, ByteCount, BufferPtr, Stop);
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
static int XV_HdmiTxSs_DdcWriteHandler(u8 DeviceAddress,
    u16 ByteCount, u8* BufferPtr, u8 Stop, void *RefPtr)
{
  XV_HdmiTx *InstancePtr = (XV_HdmiTx *)RefPtr;
  return XV_HdmiTx_DdcWrite(InstancePtr, DeviceAddress, ByteCount, BufferPtr, Stop);
}

/** @} */
