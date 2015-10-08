/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc. All rights reserved.
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

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_hdmitxss_coreinit.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
static int XV_HdmiTxSs_ComputeSubcoreAbsAddr(u32 SubSys_BaseAddr,
       u32 SubSys_HighAddr,
       u32 SubCore_Offset,
       u32 *SubCore_BaseAddr);

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
static int XV_HdmiTxSs_ComputeSubcoreAbsAddr(u32 SubSys_BaseAddr,
       u32 SubSys_HighAddr,
       u32 SubCore_Offset,
       u32 *SubCore_BaseAddr)
{
  int Status;
  u32 AbsAddr;

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
  u32 AbsAddr;
  XV_HdmiTx_Config *ConfigPtr;

  if (HdmiTxSsPtr->HdmiTxPtr) {
       /* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing HDMI TX core.... \r\n");
    ConfigPtr  = XV_HdmiTx_LookupConfig(HdmiTxSsPtr->Config.HdmiTx.DeviceId);
    if (ConfigPtr == NULL) {
      xil_printf("HDMITXSS ERR:: HDMI TX device not found\r\n");
      return(XST_FAILURE);
    }

       /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiTxSs_ComputeSubcoreAbsAddr(HdmiTxSsPtr->Config.BaseAddress,
                                      HdmiTxSsPtr->Config.HighAddress,
                                      HdmiTxSsPtr->Config.HdmiTx.AddrOffset,
                                      &AbsAddr);

    if (Status != XST_SUCCESS) {
      xil_printf("HDMITXSS ERR:: HDMI TX core base address (0x%x) \
               invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

       /* Initialize core */
    Status = XV_HdmiTx_CfgInitialize(HdmiTxSsPtr->HdmiTxPtr,
                                    ConfigPtr,
                                    AbsAddr);

    if (Status != XST_SUCCESS) {
      xil_printf("HDMITXSS ERR:: HDMI TX Initialization failed\r\n");
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
  u32 AbsAddr;
  XVtc_Config *ConfigPtr;

  if (HdmiTxSsPtr->VtcPtr) {
       /* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing VTC core.... \r\n");
    ConfigPtr  = XVtc_LookupConfig(HdmiTxSsPtr->Config.Vtc.DeviceId);
    if (ConfigPtr == NULL) {
      xil_printf("HDMITXSS ERR:: VTC device not found\r\n");
      return(XST_FAILURE);
    }

       /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiTxSs_ComputeSubcoreAbsAddr(HdmiTxSsPtr->Config.BaseAddress,
                                      HdmiTxSsPtr->Config.HighAddress,
                                      HdmiTxSsPtr->Config.Vtc.AddrOffset,
                                      &AbsAddr);

    if (Status != XST_SUCCESS) {
      xil_printf("HDMITXSS ERR:: CSC core base address (0x%x) \
                                                       invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

       /* Initialize core */
    Status = XVtc_CfgInitialize(HdmiTxSsPtr->VtcPtr,
                                ConfigPtr,
                                AbsAddr);

    if (Status != XST_SUCCESS) {
      xil_printf("HDMITXSS ERR:: VTC Initialization failed\r\n");
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
int XV_HdmiTxSs_SubcoreInitHdcpTimer(XV_HdmiTxSs *HdmiTxSsPtr)
{
  int Status;
  u32 AbsAddr;
  XTmrCtr_Config *ConfigPtr;

  if (HdmiTxSsPtr->HdcpTimerPtr) {
       /* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"   ->Initializing AXI Timer core.... \r\n");
    ConfigPtr  = XTmrCtr_LookupConfig(HdmiTxSsPtr->Config.HdcpTimer.DeviceId);
    if (ConfigPtr == NULL) {
      xil_printf("HDMITXSS ERR:: AXIS Timer device not found\r\n");
      return(XST_FAILURE);
    }

       /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiTxSs_ComputeSubcoreAbsAddr(HdmiTxSsPtr->Config.BaseAddress,
                                      HdmiTxSsPtr->Config.HighAddress,
                                      HdmiTxSsPtr->Config.HdcpTimer.AddrOffset,
                                      &AbsAddr);

    if (Status != XST_SUCCESS) {
      xil_printf("HDMITXSS ERR:: AXI Timer core base address (0x%x) \
                                       invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

       /* Setup the instance */
       memset(HdmiTxSsPtr->HdcpTimerPtr, 0, sizeof(XTmrCtr));

       /* Initialize core */
       XTmrCtr_CfgInitialize(HdmiTxSsPtr->HdcpTimerPtr, ConfigPtr, AbsAddr);
       Status = XTmrCtr_InitHw(HdmiTxSsPtr->HdcpTimerPtr);

       /* Initialize the hdcp timer functions */
       XHdcp1x_SetTimerStart(&XV_HdmiTxSs_HdcpTimerStart);
       XHdcp1x_SetTimerStop(&XV_HdmiTxSs_HdcpTimerStop);
       XHdcp1x_SetTimerDelay(&XV_HdmiTxSs_HdcpTimerBusyDelay);

    if (Status != XST_SUCCESS) {
      xil_printf("HDMITXSS ERR:: AXI Timer Initialization failed\r\n");
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
int XV_HdmiTxSs_SubcoreInitHdcp(XV_HdmiTxSs *HdmiTxSsPtr)
{
  int Status;
  u32 AbsAddr;
  XHdcp1x_Config *ConfigPtr;

  if (HdmiTxSsPtr->HdcpPtr){
       /* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing HDCP core.... \r\n");
    ConfigPtr  = XHdcp1x_LookupConfig(HdmiTxSsPtr->Config.Hdcp.DeviceId);
    if (ConfigPtr == NULL){
      xil_printf("HDMITXSS ERR:: HDCP device not found\r\n");
      return(XST_FAILURE);
    }

       /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiTxSs_ComputeSubcoreAbsAddr(HdmiTxSsPtr->Config.BaseAddress,
                                      HdmiTxSsPtr->Config.HighAddress,
                                      HdmiTxSsPtr->Config.Hdcp.AddrOffset,
                                      &AbsAddr);

    if (Status != XST_SUCCESS){
      xil_printf("HDMITXSS ERR:: HDCP core base address (0x%x) invalid %d\r\n",
               AbsAddr);
      return(XST_FAILURE);
    }

       /* Initialize core */
    void *PhyIfPtr = HdmiTxSsPtr->HdmiTxPtr;

    Status = XHdcp1x_CfgInitialize(HdmiTxSsPtr->HdcpPtr,
                                      ConfigPtr,
                                      PhyIfPtr,
                                                                         AbsAddr);

       /* Self-test the hdcp interface */
       if (XHdcp1x_SelfTest(HdmiTxSsPtr->HdcpPtr) != XST_SUCCESS) {
               Status = XST_FAILURE;
       }

       if (Status != XST_SUCCESS) {
      xil_printf("HDMITXSS ERR:: HDCP Initialization failed\r\n");
      return(XST_FAILURE);
    }

       XHdcp1x_SetKeySelect(HdmiTxSsPtr->HdcpPtr, XV_HDMITXSS_HDCP_KEYSEL);
  }
  return(XST_SUCCESS);
}

/** @} */
