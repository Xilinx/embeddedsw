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
* @file xv_hdmirxss_coreinit.c
* @addtogroup v_hdmirxss
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
* 1.00         10/07/15 Initial release.
* 1.1   yh     20/01/16 Added remapper support
* 1.2   MG     20/01/16 Added HDCP support
* 1.3   MH     08/03/16 Added DDC read message not complete event to
*                       the function XV_HdmiRxSs_DdcHdcpCallback.
*                       Updated XV_HdmiRxSs_LinkErrorCallback
*                       function to set link error flag.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xv_hdmirxss_coreinit.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
static int XV_HdmiRxSs_ComputeSubcoreAbsAddr(u32 SubSys_BaseAddr,
                                 u32 SubSys_HighAddr,
                                 u32 subcore_offset,
                                 u32 *SubCore_BaseAddr);
static void XV_HdmiRxSs_DdcSetRegAddrHandler(void *RefPtr, u32 Data);
static void XV_HdmiRxSs_DdcSetRegDataHandler(void *RefPtr, u32 Data);
static u32 XV_HdmiRxSs_DdcGetRegDataHandler(void *RefPtr);
static u32 XV_HdmiRxSs_DdcGetWriteMessageBufferWordsHandler(void *RefPtr);
static u32 XV_HdmiRxSs_DdcGetReadMessageBufferWordsHandler(void *RefPtr);
static u32 XV_HdmiRxSs_DdcIsReadMessageBufferEmptyHandler(void *RefPtr);
static u32 XV_HdmiRxSs_DdcIsWriteMessageBufferEmptyHandler(void *RefPtr);
static void XV_HdmiRxSs_DdcClearReadMessageBufferHandler(void *RefPtr);
static void XV_HdmiRxSs_DdcClearWriteMessageBufferHandler(void *RefPtr);
static void XV_HdmiRxSs_DdcHdcpCallback(void *RefPtr, int Type);
static void XV_HdmiRxSs_LinkErrorCallback(void *RefPtr);

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
static int XV_HdmiRxSs_ComputeSubcoreAbsAddr(u32 SubSys_BaseAddr,
                                 u32 SubSys_HighAddr,
                                 u32 SubCore_Offset,
                                 u32 *SubCore_BaseAddr)
{
  int Status;
  u32 absAddr;

  absAddr = SubSys_BaseAddr | SubCore_Offset;
  if((absAddr>=SubSys_BaseAddr) && (absAddr<SubSys_HighAddr))
  {
    *SubCore_BaseAddr = absAddr;
    Status = XST_SUCCESS;
  }
  else
  {
    *SubCore_BaseAddr = 0;
    Status = XST_FAILURE;
  }

  return(Status);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiRxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiRxSs_SubcoreInitHdmiRx(XV_HdmiRxSs *HdmiRxSsPtr)
{
  int Status;
  u32 AbsAddr;
  XV_HdmiRx_Config *ConfigPtr;

  if(HdmiRxSsPtr->HdmiRxPtr)
  {
    /* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing HDMI RX core.... \r\n");
    ConfigPtr  = XV_HdmiRx_LookupConfig(HdmiRxSsPtr->Config.HdmiRx.DeviceId);
    if(ConfigPtr == NULL)
    {
      xil_printf("HDMIRXSS ERR:: HDMI RX device not found\r\n");
      return(XST_FAILURE);
    }

    /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiRxSs_ComputeSubcoreAbsAddr(HdmiRxSsPtr->Config.BaseAddress,
                               HdmiRxSsPtr->Config.HighAddress,
                               HdmiRxSsPtr->Config.HdmiRx.AddrOffset,
                               &AbsAddr);

    if(Status != XST_SUCCESS)
    {
      xil_printf("HDMIRXSS ERR:: HDMI RX core base address (0x%x) \
        invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

    /* Initialize core */
    Status = XV_HdmiRx_CfgInitialize(HdmiRxSsPtr->HdmiRxPtr,
                                    ConfigPtr,
                                    AbsAddr);

    if (Status != XST_SUCCESS)
    {
      xil_printf("HDMIRXSS ERR:: HDMI RX Initialization failed\r\n");
      return(XST_FAILURE);
    }

    // Load EDID
    XV_HdmiRx_DdcLoadEdid(HdmiRxSsPtr->HdmiRxPtr, HdmiRxSsPtr->EdidPtr,
        HdmiRxSsPtr->EdidLength);

    /* Reset the hardware and set the flag to indicate the
       subsystem is ready
     */
    XV_HdmiRxSs_Reset(HdmiRxSsPtr);
    HdmiRxSsPtr->IsReady = XIL_COMPONENT_IS_READY;


  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiRxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiRxSs_SubcoreInitHdcpTimer(XV_HdmiRxSs *HdmiRxSsPtr)
{
  int Status;
  u32 AbsAddr;
  XTmrCtr_Config *ConfigPtr;

  if(HdmiRxSsPtr->HdcpTimerPtr)
  {
    /* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    \
        ->Initializing AXI Timer core.... \r\n");
    ConfigPtr  = XTmrCtr_LookupConfig(HdmiRxSsPtr->Config.HdcpTimer.DeviceId);
    if(ConfigPtr == NULL)
    {
      xil_printf("HDMIRXSS ERR:: AXIS Timer device not found\r\n");
      return(XST_FAILURE);
    }

    /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiRxSs_ComputeSubcoreAbsAddr(HdmiRxSsPtr->Config.BaseAddress,
                               HdmiRxSsPtr->Config.HighAddress,
                               HdmiRxSsPtr->Config.HdcpTimer.AddrOffset,
                               &AbsAddr);

    if(Status != XST_SUCCESS)
    {
      xil_printf("HDMIRXSS ERR:: AXI Timer core base address (0x%x) \
        invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

    /* Setup the instance */
    memset(HdmiRxSsPtr->HdcpTimerPtr, 0, sizeof(XTmrCtr));

    /* Initialize core */
    XTmrCtr_CfgInitialize(HdmiRxSsPtr->HdcpTimerPtr, ConfigPtr, AbsAddr);
    Status = XTmrCtr_InitHw(HdmiRxSsPtr->HdcpTimerPtr);

    if(Status != XST_SUCCESS)
    {
      xil_printf("HDMIRXSS ERR:: AXI Timer Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiRxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiRxSs_SubcoreInitHdcp14(XV_HdmiRxSs *HdmiRxSsPtr)
{
  int Status;
  u32 AbsAddr;
  XHdcp1x_Config *ConfigPtr;

  /* Is the HDCP 1.4 RX present? */
  if (HdmiRxSsPtr->Hdcp14Ptr) {

    /* Is the key loaded? */
    if (HdmiRxSsPtr->Hdcp14KeyPtr) {

      /* Get core configuration */
      xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing HDCP 1.4 core.... \r\n");
      ConfigPtr  = XHdcp1x_LookupConfig(HdmiRxSsPtr->Config.Hdcp14.DeviceId);
      if(ConfigPtr == NULL)
      {
        xil_printf("HDMIRXSS ERR:: HDCP 1.4 device not found\r\n");
        return(XST_FAILURE);
      }

      /* Compute absolute base address */
      AbsAddr = 0;
      Status = XV_HdmiRxSs_ComputeSubcoreAbsAddr(HdmiRxSsPtr->Config.BaseAddress,
                                 HdmiRxSsPtr->Config.HighAddress,
                                 HdmiRxSsPtr->Config.Hdcp14.AddrOffset,
                                 &AbsAddr);

      if(Status != XST_SUCCESS)
      {
        xil_printf("HDMIRXSS ERR:: HDCP 1.4 core base address (0x%x) invalid %d\r\n",
          AbsAddr);
        return(XST_FAILURE);
      }

      /* Initialize core */
      void *PhyIfPtr = HdmiRxSsPtr->HdmiRxPtr;
      Status = XHdcp1x_CfgInitialize(HdmiRxSsPtr->Hdcp14Ptr,
                                        ConfigPtr,
                                        PhyIfPtr,
                                        AbsAddr);

      /* Self-test the hdcp interface */
      if (XHdcp1x_SelfTest(HdmiRxSsPtr->Hdcp14Ptr) != XST_SUCCESS) {
          Status = XST_FAILURE;
      }

      if(Status != XST_SUCCESS)
      {
        xil_printf("HDMIRXSS ERR:: HDCP 1.4 Initialization failed\r\n");
        return(XST_FAILURE);
      }

      /* Set-up the DDC Handlers */
      XHdcp1x_SetCallback(HdmiRxSsPtr->Hdcp14Ptr,  XHDCP1X_HANDLER_DDC_SETREGADDR,  (XHdcp1x_SetDdcHandler)XV_HdmiRxSs_DdcSetRegAddrHandler, HdmiRxSsPtr->HdmiRxPtr);
      XHdcp1x_SetCallback(HdmiRxSsPtr->Hdcp14Ptr,  XHDCP1X_HANDLER_DDC_SETREGDATA,  (XHdcp1x_SetDdcHandler)XV_HdmiRxSs_DdcSetRegDataHandler, HdmiRxSsPtr->HdmiRxPtr);
      XHdcp1x_SetCallback(HdmiRxSsPtr->Hdcp14Ptr,  XHDCP1X_HANDLER_DDC_GETREGDATA,  (XHdcp1x_GetDdcHandler)XV_HdmiRxSs_DdcGetRegDataHandler, HdmiRxSsPtr->HdmiRxPtr);

      /* Select key */
      XHdcp1x_SetKeySelect(HdmiRxSsPtr->Hdcp14Ptr, XV_HDMIRXSS_HDCP_KEYSEL);

      /* Disable HDCP 1.4 repeater */
      HdmiRxSsPtr->Hdcp14Ptr->IsRepeater = 0;

      /* Set-up the HDMI RX HDCP Callback Handler */
      XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                XV_HDMIRX_HANDLER_HDCP,
                XV_HdmiRxSs_DdcHdcpCallback,
                (void *)HdmiRxSsPtr);

      /* Enable HDMI-RX DDC interrupts */
      XV_HdmiRx_DdcIntrEnable(HdmiRxSsPtr->HdmiRxPtr);

      /* Enable HDMI-RX HDCP */
      XV_HdmiRx_DdcHdcpEnable(HdmiRxSsPtr->HdmiRxPtr);

      /* Clear the event queue */
      XV_HdmiRxSs_HdcpClearEvents(HdmiRxSsPtr);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiRxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiRxSs_SubcoreInitHdcp22(XV_HdmiRxSs *HdmiRxSsPtr)
{
  int Status;
  u32 AbsAddr;
  XHdcp22_Rx_Config *ConfigPtr;

  /* Is the HDCP 2.2 RX present? */
  if (HdmiRxSsPtr->Hdcp22Ptr) {

    /* Are the keys loaded? */
    if (HdmiRxSsPtr->Hdcp22Lc128Ptr && HdmiRxSsPtr->Hdcp22PrivateKeyPtr) {

      /* Get core configuration */
      xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing HDCP 2.2 core.... \r\n");

      ConfigPtr  = XHdcp22Rx_LookupConfig(HdmiRxSsPtr->Config.Hdcp22.DeviceId);
      if(ConfigPtr == NULL)
      {
        xil_printf("HDMIRXSS ERR:: HDCP 2.2 device not found\r\n");
        return (XST_FAILURE);
      }

      /* Compute absolute base address */
      AbsAddr = 0;
      Status = XV_HdmiRxSs_ComputeSubcoreAbsAddr(HdmiRxSsPtr->Config.BaseAddress,
                                 HdmiRxSsPtr->Config.HighAddress,
                                 HdmiRxSsPtr->Config.Hdcp22.AddrOffset,
                                 &AbsAddr);

      if(Status != XST_SUCCESS)
      {
        xil_printf("HDMIRXSS ERR:: HDCP 2.2 core base address (0x%x) invalid %d\r\n",
          AbsAddr);
        return(XST_FAILURE);
      }

      /* Initialize core */
      Status = XHdcp22Rx_CfgInitialize(HdmiRxSsPtr->Hdcp22Ptr, ConfigPtr, AbsAddr);
      if (Status != XST_SUCCESS)
      {
        xil_printf("HDMIRXSS ERR:: HDCP 2.2 Initialization failed\r\n");
        return(XST_FAILURE);
      }

      /* Set-up the DDC Handlers */
      XHdcp22Rx_SetCallback(HdmiRxSsPtr->Hdcp22Ptr,  XHDCP22_RX_HANDLER_DDC_SETREGADDR,  (XHdcp22_Rx_SetHandler)XV_HdmiRxSs_DdcSetRegAddrHandler,                  HdmiRxSsPtr->HdmiRxPtr);
      XHdcp22Rx_SetCallback(HdmiRxSsPtr->Hdcp22Ptr,  XHDCP22_RX_HANDLER_DDC_SETREGDATA,  (XHdcp22_Rx_SetHandler)XV_HdmiRxSs_DdcSetRegDataHandler,                    HdmiRxSsPtr->HdmiRxPtr);
      XHdcp22Rx_SetCallback(HdmiRxSsPtr->Hdcp22Ptr,  XHDCP22_RX_HANDLER_DDC_GETREGDATA,  (XHdcp22_Rx_GetHandler)XV_HdmiRxSs_DdcGetRegDataHandler,                    HdmiRxSsPtr->HdmiRxPtr);
      XHdcp22Rx_SetCallback(HdmiRxSsPtr->Hdcp22Ptr,  XHDCP22_RX_HANDLER_DDC_GETWBUFSIZE, (XHdcp22_Rx_GetHandler)XV_HdmiRxSs_DdcGetWriteMessageBufferWordsHandler,    HdmiRxSsPtr->HdmiRxPtr);
      XHdcp22Rx_SetCallback(HdmiRxSsPtr->Hdcp22Ptr,  XHDCP22_RX_HANDLER_DDC_GETRBUFSIZE, (XHdcp22_Rx_GetHandler)XV_HdmiRxSs_DdcGetReadMessageBufferWordsHandler,     HdmiRxSsPtr->HdmiRxPtr);
      XHdcp22Rx_SetCallback(HdmiRxSsPtr->Hdcp22Ptr,  XHDCP22_RX_HANDLER_DDC_ISWBUFEMPTY, (XHdcp22_Rx_GetHandler)XV_HdmiRxSs_DdcIsWriteMessageBufferEmptyHandler,     HdmiRxSsPtr->HdmiRxPtr);
      XHdcp22Rx_SetCallback(HdmiRxSsPtr->Hdcp22Ptr,  XHDCP22_RX_HANDLER_DDC_ISRBUFEMPTY, (XHdcp22_Rx_GetHandler)XV_HdmiRxSs_DdcIsReadMessageBufferEmptyHandler,      HdmiRxSsPtr->HdmiRxPtr);
      XHdcp22Rx_SetCallback(HdmiRxSsPtr->Hdcp22Ptr,  XHDCP22_RX_HANDLER_DDC_CLEARRBUF,   (XHdcp22_Rx_RunHandler)XV_HdmiRxSs_DdcClearReadMessageBufferHandler,        HdmiRxSsPtr->HdmiRxPtr);
      XHdcp22Rx_SetCallback(HdmiRxSsPtr->Hdcp22Ptr,  XHDCP22_RX_HANDLER_DDC_CLEARWBUF,   (XHdcp22_Rx_RunHandler)XV_HdmiRxSs_DdcClearWriteMessageBufferHandler,       HdmiRxSsPtr->HdmiRxPtr);

      /* Set-up the HDMI RX HDCP Callback Handler */
      XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                XV_HDMIRX_HANDLER_HDCP,
                XV_HdmiRxSs_DdcHdcpCallback,
                (void *)HdmiRxSsPtr);

      /* Set-up the HDMI RX Link error Callback Handler */
      XV_HdmiRx_SetCallback(HdmiRxSsPtr->HdmiRxPtr,
                XV_HDMIRX_HANDLER_LINK_ERROR,
                XV_HdmiRxSs_LinkErrorCallback,
                (void *)HdmiRxSsPtr);

      /* Load Production Keys */
      XHdcp22Rx_LoadLc128(HdmiRxSsPtr->Hdcp22Ptr, HdmiRxSsPtr->Hdcp22Lc128Ptr);
      XHdcp22Rx_LoadPublicCert(HdmiRxSsPtr->Hdcp22Ptr, HdmiRxSsPtr->Hdcp22PrivateKeyPtr+40);
      XHdcp22Rx_LoadPrivateKey(HdmiRxSsPtr->Hdcp22Ptr, HdmiRxSsPtr->Hdcp22PrivateKeyPtr+562);

      XHdcp22Rx_LogReset(HdmiRxSsPtr->Hdcp22Ptr, FALSE);

      /* Enable HDMI-RX DDC interrupts */
      XV_HdmiRx_DdcIntrEnable(HdmiRxSsPtr->HdmiRxPtr);

      /* Enable HDMI-RX HDCP */
      XV_HdmiRx_DdcHdcpEnable(HdmiRxSsPtr->HdmiRxPtr);

      /* Clear the event queue */
      XV_HdmiRxSs_HdcpClearEvents(HdmiRxSsPtr);
    }
  }

  return (XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiRxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiRxSs_SubcoreInitRemapperReset(XV_HdmiRxSs *HdmiRxSsPtr)
{
  int Status;
  u32 AbsAddr;
  XGpio_Config *ConfigPtr;

  if (HdmiRxSsPtr->RemapperResetPtr) {
    /* Get core configuration */
    ConfigPtr  = XGpio_LookupConfig(HdmiRxSsPtr->Config.RemapperReset.DeviceId);
    if (ConfigPtr == NULL) {
      xil_printf("HDMIRXSS ERR:: Reset module for Remapper not found\r\n");
      return(XST_FAILURE);
    }

    /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiRxSs_ComputeSubcoreAbsAddr(HdmiRxSsPtr->Config.BaseAddress,
                               HdmiRxSsPtr->Config.HighAddress,
                               HdmiRxSsPtr->Config.RemapperReset.AddrOffset,
                               &AbsAddr);

    if (Status != XST_SUCCESS) {
      xil_printf("HDMIRXSS ERR:: Remapper Reset GPIO core base address (0x%x) \
                            invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

    //AbsAddr = HdmiRxSsPtr->Config.RemapperReset.AddrOffset;

    /* Initialize core */
    Status = XGpio_CfgInitialize(HdmiRxSsPtr->RemapperResetPtr,
                                 ConfigPtr,
                                 AbsAddr);

    if (Status != XST_SUCCESS) {
      xil_printf("HDMIRXSS ERR:: Remapper Reset Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  HdmiRxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XV_HdmiRxSs_SubcoreInitRemapper(XV_HdmiRxSs *HdmiRxSsPtr)
{
  int Status;
  u32 AbsAddr;
  XV_axi4s_remap_Config *ConfigPtr;

  if (HdmiRxSsPtr->RemapperPtr) {
    /* Get core configuration */
    ConfigPtr  = XV_axi4s_remap_LookupConfig(HdmiRxSsPtr->Config.Remapper.DeviceId);
    if (ConfigPtr == NULL) {
      xil_printf("HDMIRXSS ERR:: Remapper not found\r\n");
      return(XST_FAILURE);
    }

    /* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiRxSs_ComputeSubcoreAbsAddr(HdmiRxSsPtr->Config.BaseAddress,
                               HdmiRxSsPtr->Config.HighAddress,
                               HdmiRxSsPtr->Config.Remapper.AddrOffset,
                               &AbsAddr);

    if (Status != XST_SUCCESS) {
      xil_printf("HDMIRXSS ERR:: Remapper core base address (0x%x) \
                            invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

    //AbsAddr = HdmiRxSsPtr->Config.Remapper.AddrOffset;

    /* Initialize core */
    Status = XV_axi4s_remap_CfgInitialize(HdmiRxSsPtr->RemapperPtr,
                                 ConfigPtr,
                                 AbsAddr);

    if (Status != XST_SUCCESS) {
      xil_printf("HDMIRXSS ERR:: Remapper Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);

}

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
static void XV_HdmiRxSs_DdcSetRegAddrHandler(void *RefPtr, u32 Data)
{
  XV_HdmiRx *InstancePtr = (XV_HdmiRx *)RefPtr;
  XV_HdmiRx_DdcHdcpSetAddress(InstancePtr, Data);
}

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
static void XV_HdmiRxSs_DdcSetRegDataHandler(void *RefPtr, u32 Data)
{
  XV_HdmiRx *InstancePtr = (XV_HdmiRx *)RefPtr;
  XV_HdmiRx_DdcHdcpWriteData(InstancePtr, Data);
}

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
static u32 XV_HdmiRxSs_DdcGetRegDataHandler(void *RefPtr)
{
  XV_HdmiRx *InstancePtr = (XV_HdmiRx *)RefPtr;
  return XV_HdmiRx_DdcHdcpReadData(InstancePtr);
}

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
static u32 XV_HdmiRxSs_DdcGetWriteMessageBufferWordsHandler(void *RefPtr)
{
  XV_HdmiRx *InstancePtr = (XV_HdmiRx *)RefPtr;
  return XV_HdmiRx_DdcGetHdcpWriteMessageBufferWords(InstancePtr);
}

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
static u32 XV_HdmiRxSs_DdcGetReadMessageBufferWordsHandler(void *RefPtr)
{
  XV_HdmiRx *InstancePtr = (XV_HdmiRx *)RefPtr;
  return XV_HdmiRx_DdcGetHdcpReadMessageBufferWords(InstancePtr);
}

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
static u32 XV_HdmiRxSs_DdcIsReadMessageBufferEmptyHandler(void *RefPtr)
{
  XV_HdmiRx *InstancePtr = (XV_HdmiRx *)RefPtr;
  return XV_HdmiRx_DdcIsHdcpReadMessageBufferEmpty(InstancePtr);
}

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
static u32 XV_HdmiRxSs_DdcIsWriteMessageBufferEmptyHandler(void *RefPtr)
{
  XV_HdmiRx *InstancePtr = (XV_HdmiRx *)RefPtr;
  return XV_HdmiRx_DdcIsHdcpWriteMessageBufferEmpty(InstancePtr);
}

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
static void XV_HdmiRxSs_DdcClearReadMessageBufferHandler(void *RefPtr)
{
  XV_HdmiRx *InstancePtr = (XV_HdmiRx *)RefPtr;
  XV_HdmiRx_DdcHdcpClearReadMessageBuffer(InstancePtr);
}

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
static void XV_HdmiRxSs_DdcClearWriteMessageBufferHandler(void *RefPtr)
{
  XV_HdmiRx *InstancePtr = (XV_HdmiRx *)RefPtr;
  XV_HdmiRx_DdcHdcpClearWriteMessageBuffer(InstancePtr);
}

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
static void XV_HdmiRxSs_DdcHdcpCallback(void *RefPtr, int Type)
{
  XV_HdmiRxSs *HdmiRxSsPtr;
  HdmiRxSsPtr = RefPtr;

  switch (Type)
  {
    // HDCP 2.2. write message
    case XV_HDMIRX_DDC_STA_HDCP_WMSG_NEW_EVT_MASK:
      XHdcp22Rx_SetWriteMessageAvailable(HdmiRxSsPtr->Hdcp22Ptr);
      break;

    // HDCP 2.2 read message
    case XV_HDMIRX_DDC_STA_HDCP_RMSG_END_EVT_MASK:
      XHdcp22Rx_SetReadMessageComplete(HdmiRxSsPtr->Hdcp22Ptr);
      break;

    // HDCP 2.2 read not complete
    case XV_HDMIRX_DDC_STA_HDCP_RMSG_NC_EVT_MASK:
      XHdcp22Rx_SetDdcError(HdmiRxSsPtr->Hdcp22Ptr);
      break;

    // HDCP 1.4 Aksv
    case XV_HDMIRX_DDC_STA_HDCP_AKSV_EVT_MASK:
      XHdcp1x_ProcessAKsv(HdmiRxSsPtr->Hdcp14Ptr);
      break;

    default:
      break;
  }
}

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
static void XV_HdmiRxSs_LinkErrorCallback(void *RefPtr)
{
  XV_HdmiRxSs *HdmiRxSsPtr;
  HdmiRxSsPtr = RefPtr;

  // HDCP 2.2
  if (HdmiRxSsPtr->HdcpProtocol == XV_HDMIRXSS_HDCP_22) {
    XHdcp22Rx_SetLinkError(HdmiRxSsPtr->Hdcp22Ptr);
  }
}

/** @} */
