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
* @file xvprocss_coreinit.c
* @addtogroup vprocss
* @{
* @details

* Video Processing Subsystem Sub-Cores initialization
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
* 1.00  rco  07/21/15   Initial Release

* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xvprocss_coreinit.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
static int ComputeSubcoreAbsAddr(u32 subsys_baseaddr,
		                         u32 subsys_highaddr,
		                         u32 subcore_offset,
		                         u32 *subcore_baseaddr);


/*****************************************************************************/
/**
* This function computes the subcore absolute address on axi-lite interface
* Subsystem is mapped at an absolute address and all included sub-cores are
* at pre-defined offset from the subsystem base address. To access the subcore
* register map from host CPU an absolute address is required.
* Subsystem has address range of 1MB (0x00000-0xFFFFF)
*
* @param  subsys_baseaddr is the base address of the the Subsystem instance
* @param  subsys_highaddr is the max address of the Subsystem instance
* @param  subcore_offset is the offset of the specified core
* @param  subcore_baseaddr is the computed absolute base address of the subcore
*
* @return XST_SUCCESS if base address computation is successful and within
*         subsystem address range else XST_FAILURE
*
******************************************************************************/
static int ComputeSubcoreAbsAddr(u32 subsys_baseaddr,
		                         u32 subsys_highaddr,
		                         u32 subcore_offset,
		                         u32 *subcore_baseaddr)
{
  int status;
  u32 absAddr;

  absAddr = subsys_baseaddr | subcore_offset;
  if((absAddr>=subsys_baseaddr) && (absAddr<subsys_highaddr))
  {
    *subcore_baseaddr = absAddr;
    status = XST_SUCCESS;
  }
  else
  {
	*subcore_baseaddr = 0;
	status = XST_FAILURE;
  }

  return(status);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitResetAxis(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XGpio_Config *pConfig;

  if(XVprocSsPtr->RstAxisPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing AXIS Reset core.... \r\n");
    pConfig  = XGpio_LookupConfig(XVprocSsPtr->Config.RstAxis.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Reset AXIS device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.RstAxis.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Reset AXIS core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XGpio_CfgInitialize(XVprocSsPtr->RstAxisPtr,
                                 pConfig,
                                 AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Reset AXIS core Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitResetAximm(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XGpio_Config *pConfig;

  if(XVprocSsPtr->RstAximmPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing AXI-MM Reset core.... \r\n");
    pConfig  = XGpio_LookupConfig(XVprocSsPtr->Config.RstAximm .DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Reset AXI-MM device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.RstAximm.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Reset AXI-MM core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XGpio_CfgInitialize(XVprocSsPtr->RstAximmPtr,
                                 pConfig,
                                 AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Reset AXI-MM core Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitRouter(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XAxis_Switch_Config *pConfig;

  if(XVprocSsPtr->RouterPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing AXIS Switch core.... \r\n");
    pConfig  = XAxisScr_LookupConfig(XVprocSsPtr->Config.Router.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: AXIS Switch device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.Router.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: AXIS Switch core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XAxisScr_CfgInitialize(XVprocSsPtr->RouterPtr,
                                    pConfig,
                                    AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: AXIS Switch Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitCsc(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XV_csc_Config *pConfig;

  if(XVprocSsPtr->CscPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing CSC core.... \r\n");
    pConfig  = XV_csc_LookupConfig(XVprocSsPtr->Config.Csc.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: CSC device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.Csc.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: CSC core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_csc_CfgInitialize(XVprocSsPtr->CscPtr,
                                  pConfig,
                                  AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: CSC Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}


/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitHScaler(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XV_hscaler_Config *pConfig;

  if(XVprocSsPtr->HscalerPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Horiz. Scaler core.... \r\n");
    pConfig  = XV_hscaler_LookupConfig(XVprocSsPtr->Config.Hscale.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Horiz. Scaler device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.Hscale.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Horiz. Scaler core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_hscaler_CfgInitialize(XVprocSsPtr->HscalerPtr,
                                      pConfig,
                                      AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Horiz. Scaler Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitVScaler(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XV_vscaler_Config *pConfig;

  if(XVprocSsPtr->VscalerPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Vert. Scaler core.... \r\n");
    pConfig  = XV_vscaler_LookupConfig(XVprocSsPtr->Config.Vscale.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Vert. Scaler device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.Vscale.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Vert. Scaler core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_vscaler_CfgInitialize(XVprocSsPtr->VscalerPtr,
                                      pConfig,
                                      AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Vert. Scaler Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitHCrsmplr(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XV_hcresampler_Config *pConfig;

  if(XVprocSsPtr->HcrsmplrPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing H Chroma Resampler core.... \r\n");
    pConfig  = XV_hcresampler_LookupConfig(XVprocSsPtr->Config.HCrsmplr.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: H Chroma Resampler device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.HCrsmplr.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: H Chroma Resampler core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_hcresampler_CfgInitialize(XVprocSsPtr->HcrsmplrPtr,
                                          pConfig,
                                          AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: H Chroma Resampler Initialization failed\r\n");
      return(XST_FAILURE);
    }

    if(XVprocSsPtr->HcrsmplrPtr->Config.ResamplingType == XV_HCRSMPLR_TYPE_FIR)
    {
      /* Load default filter coefficients */
      XV_HCrsmplLoadDefaultCoeff(XVprocSsPtr->HcrsmplrPtr, &XVprocSsPtr->HcrL2Reg);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitVCrsmpleIn(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XV_vcresampler_Config *pConfig;

  if(XVprocSsPtr->VcrsmplrInPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Input V Chroma Resampler core.... \r\n");
    pConfig  = XV_vcresampler_LookupConfig(XVprocSsPtr->Config.VCrsmplrIn.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Input V Chroma Resampler device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.VCrsmplrIn.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Input V Chroma Resampler core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_vcresampler_CfgInitialize(XVprocSsPtr->VcrsmplrInPtr,
                                          pConfig,
                                          AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Input V Chroma Resampler Initialization failed\r\n");
      return(XST_FAILURE);
    }

    if(XVprocSsPtr->VcrsmplrInPtr->Config.ResamplingType == XV_VCRSMPLR_TYPE_FIR)
    {
      /* Load default filter coefficients */
      XV_VCrsmplLoadDefaultCoeff(XVprocSsPtr->VcrsmplrInPtr, &XVprocSsPtr->VcrInL2Reg);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitVCrsmpleOut(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XV_vcresampler_Config *pConfig;

  if(XVprocSsPtr->VcrsmplrOutPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Output V Chroma Resampler core.... \r\n");
    pConfig  = XV_vcresampler_LookupConfig(XVprocSsPtr->Config.VCrsmplrOut.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Output V Chroma Resampler device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.VCrsmplrOut.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Output V Chroma Resampler core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_vcresampler_CfgInitialize(XVprocSsPtr->VcrsmplrOutPtr,
                                          pConfig,
                                          AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Output V Chroma Resampler Initialization failed\r\n");
      return(XST_FAILURE);
    }

    if(XVprocSsPtr->VcrsmplrOutPtr->Config.ResamplingType == XV_VCRSMPLR_TYPE_FIR)
    {
      /* Load default filter coefficients */
      XV_VCrsmplLoadDefaultCoeff(XVprocSsPtr->VcrsmplrOutPtr, &XVprocSsPtr->VcrOutL2Reg);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitLetterbox(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XV_letterbox_Config *pConfig;

  if(XVprocSsPtr->LboxPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Letterbox core.... \r\n");
    pConfig  = XV_letterbox_LookupConfig(XVprocSsPtr->Config.Lbox.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Letterbox device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.Lbox.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Letterbox core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_letterbox_CfgInitialize(XVprocSsPtr->LboxPtr,
                                        pConfig,
                                        AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Letterbox Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitVdma(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XAxiVdma_Config *pConfig;

  if(XVprocSsPtr->VdmaPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing VDMA core.... \r\n");
    pConfig  = XAxiVdma_LookupConfig(XVprocSsPtr->Config.Vdma.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: VDMA device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.Vdma.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: VDMA core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XAxiVdma_CfgInitialize(XVprocSsPtr->VdmaPtr,
                                    pConfig,
                                    AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: VDMA Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  XVprocSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocSs_SubcoreInitDeinterlacer(XVprocSs *XVprocSsPtr)
{
  int status;
  u32 AbsAddr;
  XV_deinterlacer_Config *pConfig;

  if(XVprocSsPtr->DeintPtr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Deinterlacer core.... \r\n");
    pConfig  = XV_deinterlacer_LookupConfig(XVprocSsPtr->Config.Deint.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Deinterlacer device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(XVprocSsPtr->Config.BaseAddress,
		                           XVprocSsPtr->Config.HighAddress,
		                           XVprocSsPtr->Config.Deint.AddrOffset,
		                           &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Deinterlacer core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_deinterlacer_CfgInitialize(XVprocSsPtr->DeintPtr,
                                           pConfig,
                                           AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Deinterlacer Initialization failed\r\n");
      return(XST_FAILURE);
    }
  }
  return(XST_SUCCESS);
}
/** @} */
