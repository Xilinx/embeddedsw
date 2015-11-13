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

    // Load EDID
    XV_HdmiRx_DdcLoadEdid(HdmiRxSsPtr->HdmiRxPtr, HdmiRxSsPtr->EdidPtr,
		HdmiRxSsPtr->EdidLength);

    /* Reset the hardware and set the flag to indicate the
       subsystem is ready
     */
    XV_HdmiRxSs_Reset(HdmiRxSsPtr);
    HdmiRxSsPtr->IsReady = XIL_COMPONENT_IS_READY;


    if(Status != XST_SUCCESS)
    {
      xil_printf("HDMIRXSS ERR:: HDMI RX Initialization failed\r\n");
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
int XV_HdmiRxSs_SubcoreInitHdcp(XV_HdmiRxSs *HdmiRxSsPtr)
{
  int Status;
  u32 AbsAddr;
  XHdcp1x_Config *ConfigPtr;

  if(HdmiRxSsPtr->HdcpPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing HDCP core.... \r\n");
    ConfigPtr  = XHdcp1x_LookupConfig(HdmiRxSsPtr->Config.Hdcp.DeviceId);
    if(ConfigPtr == NULL)
    {
      xil_printf("HDMIRXSS ERR:: HDCP device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    Status = XV_HdmiRxSs_ComputeSubcoreAbsAddr(HdmiRxSsPtr->Config.BaseAddress,
		                       HdmiRxSsPtr->Config.HighAddress,
		                       HdmiRxSsPtr->Config.Hdcp.AddrOffset,
		                       &AbsAddr);

    if(Status != XST_SUCCESS)
    {
      xil_printf("HDMIRXSS ERR:: HDCP core base address (0x%x) invalid %d\r\n",
		AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    void *PhyIfPtr = HdmiRxSsPtr->HdmiRxPtr;
    Status = XHdcp1x_CfgInitialize(HdmiRxSsPtr->HdcpPtr,
                                      ConfigPtr,
                                      PhyIfPtr,
									  AbsAddr);

	/* Self-test the hdcp interface */
	if (XHdcp1x_SelfTest(HdmiRxSsPtr->HdcpPtr) != XST_SUCCESS) {
		Status = XST_FAILURE;
	}

	if(Status != XST_SUCCESS)
    {
      xil_printf("HDMIRXSS ERR:: HDCP Initialization failed\r\n");
      return(XST_FAILURE);
    }

	XHdcp1x_SetKeySelect(HdmiRxSsPtr->HdcpPtr, XV_HDMIRXSS_HDCP_KEYSEL);
  }
  return(XST_SUCCESS);
}


/** @} */
