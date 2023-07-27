/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xvprocss_coreinit.c
* @addtogroup vprocss Overview
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
* 1.10  rco  11/25/15   Replace bitwise OR with ADD operation when computing
*                       subcore absolute address
* 2.00  dmc  01/11/16   Write to new Event Log: log sub-core init errors
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xvprocss_coreinit.h"

/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
static int ComputeSubcoreAbsAddr(UINTPTR subsys_baseaddr,
		                         UINTPTR subsys_highaddr,
		                         u32 subcore_offset,
								 UINTPTR *subcore_baseaddr);


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
static int ComputeSubcoreAbsAddr(UINTPTR subsys_baseaddr,
		                         UINTPTR subsys_highaddr,
		                         u32 subcore_offset,
								 UINTPTR *subcore_baseaddr)
{
  int status;
  UINTPTR absAddr;

  absAddr = subsys_baseaddr + subcore_offset;
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
  UINTPTR AbsAddr;
  XGpio_Config *pConfig;

  if(XVprocSsPtr->RstAxisPtr)
  {
	/* Get core configuration */
    pConfig  = XGpio_LookupConfig(XVprocSsPtr->Config.RstAxis.DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_RESAXIS, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_RESAXIS, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XGpio_CfgInitialize(XVprocSsPtr->RstAxisPtr,
                                 pConfig,
                                 AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_RESAXIS, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_RESAXIS, XVPROCSS_EDAT_INITOK);
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
  UINTPTR AbsAddr;
  XGpio_Config *pConfig;

  if(XVprocSsPtr->RstAximmPtr)
  {
	/* Get core configuration */
    pConfig  = XGpio_LookupConfig(XVprocSsPtr->Config.RstAximm .DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_RESAXIM, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_RESAXIM, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XGpio_CfgInitialize(XVprocSsPtr->RstAximmPtr,
                                 pConfig,
                                 AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_RESAXIM, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_RESAXIM, XVPROCSS_EDAT_INITOK);
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
  UINTPTR AbsAddr;
  XAxis_Switch_Config *pConfig;

  if(XVprocSsPtr->RouterPtr)
  {
	/* Get core configuration */
    pConfig  = XAxisScr_LookupConfig(XVprocSsPtr->Config.Router.DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_ROUTER, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_ROUTER, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XAxisScr_CfgInitialize(XVprocSsPtr->RouterPtr,
                                    pConfig,
                                    AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_ROUTER, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_ROUTER, XVPROCSS_EDAT_INITOK);
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
  UINTPTR AbsAddr;
  XV_csc_Config *pConfig;

  if(XVprocSsPtr->CscPtr)
  {
	/* Get core configuration */
    pConfig  = XV_csc_LookupConfig(XVprocSsPtr->Config.Csc.DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_csc_CfgInitialize(&XVprocSsPtr->CscPtr->Csc,
                                  pConfig,
                                  AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_CSC, XVPROCSS_EDAT_INITOK);
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
  UINTPTR AbsAddr;
  XV_hscaler_Config *pConfig;

  if(XVprocSsPtr->HscalerPtr)
  {
	/* Get core configuration */
    pConfig  = XV_hscaler_LookupConfig(XVprocSsPtr->Config.Hscale.DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_hscaler_CfgInitialize(&XVprocSsPtr->HscalerPtr->Hsc,
                                      pConfig,
                                      AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HSCALER, XVPROCSS_EDAT_INITOK);
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
  UINTPTR AbsAddr;
  XV_vscaler_Config *pConfig;

  if(XVprocSsPtr->VscalerPtr)
  {
	/* Get core configuration */
    pConfig  = XV_vscaler_LookupConfig(XVprocSsPtr->Config.Vscale.DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VSCALER, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VSCALER, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_vscaler_CfgInitialize(&XVprocSsPtr->VscalerPtr->Vsc,
                                      pConfig,
                                      AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VSCALER, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VSCALER, XVPROCSS_EDAT_INITOK);
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
  UINTPTR AbsAddr;
  XV_hcresampler_Config *pConfig;

  if(XVprocSsPtr->HcrsmplrPtr)
  {
	/* Get core configuration */
    pConfig  = XV_hcresampler_LookupConfig(XVprocSsPtr->Config.HCrsmplr.DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_hcresampler_CfgInitialize(&XVprocSsPtr->HcrsmplrPtr->Hcr,
                                          pConfig,
                                          AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }

    if(XVprocSsPtr->HcrsmplrPtr->Hcr.Config.ResamplingType == XV_HCRSMPLR_TYPE_FIR)
    {
      /* Load default filter coefficients */
      XV_HCrsmplLoadDefaultCoeff(XVprocSsPtr->HcrsmplrPtr);
	  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_LDCOEF);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_HCR, XVPROCSS_EDAT_INITOK);
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
  UINTPTR AbsAddr;
  XV_vcresampler_Config *pConfig;

  if(XVprocSsPtr->VcrsmplrInPtr)
  {
	/* Get core configuration */
    pConfig  = XV_vcresampler_LookupConfig(XVprocSsPtr->Config.VCrsmplrIn.DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_vcresampler_CfgInitialize(&XVprocSsPtr->VcrsmplrInPtr->Vcr,
                                          pConfig,
                                          AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }

    if(XVprocSsPtr->VcrsmplrInPtr->Vcr.Config.ResamplingType == XV_VCRSMPLR_TYPE_FIR)
    {
      /* Load default filter coefficients */
      XV_VCrsmplLoadDefaultCoeff(XVprocSsPtr->VcrsmplrInPtr);
	  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_LDCOEF);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRI, XVPROCSS_EDAT_INITOK);
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
  UINTPTR AbsAddr;
  XV_vcresampler_Config *pConfig;

  if(XVprocSsPtr->VcrsmplrOutPtr)
  {
	/* Get core configuration */
    pConfig  = XV_vcresampler_LookupConfig(XVprocSsPtr->Config.VCrsmplrOut.DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRO, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRO, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_vcresampler_CfgInitialize(&XVprocSsPtr->VcrsmplrOutPtr->Vcr,
                                          pConfig,
                                          AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRO, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }

    if(XVprocSsPtr->VcrsmplrOutPtr->Vcr.Config.ResamplingType == XV_VCRSMPLR_TYPE_FIR)
    {
      /* Load default filter coefficients */
      XV_VCrsmplLoadDefaultCoeff(XVprocSsPtr->VcrsmplrOutPtr);
	  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRO, XVPROCSS_EDAT_LDCOEF);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_VCRO, XVPROCSS_EDAT_INITOK);
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
  UINTPTR AbsAddr;
  XV_letterbox_Config *pConfig;

  if(XVprocSsPtr->LboxPtr)
  {
	/* Get core configuration */
    pConfig  = XV_letterbox_LookupConfig(XVprocSsPtr->Config.Lbox.DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_LBOX, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_LBOX, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_letterbox_CfgInitialize(&XVprocSsPtr->LboxPtr->Lbox,
                                        pConfig,
                                        AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_LBOX, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_LBOX, XVPROCSS_EDAT_INITOK);
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
  UINTPTR AbsAddr;
  XAxiVdma_Config *pConfig;

  if(XVprocSsPtr->VdmaPtr)
  {
	/* Get core configuration */
    pConfig  = XAxiVdma_LookupConfig(XVprocSsPtr->Config.Vdma.DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_VDMA, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_VDMA, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XAxiVdma_CfgInitialize(XVprocSsPtr->VdmaPtr,
                                    pConfig,
                                    AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_VDMA, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_INIT_VDMA, XVPROCSS_EDAT_INITOK);
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
  UINTPTR AbsAddr;
  XV_deinterlacer_Config *pConfig;

  if(XVprocSsPtr->DeintPtr)
  {
	/* Get core configuration */
    pConfig  = XV_deinterlacer_LookupConfig(XVprocSsPtr->Config.Deint.DeviceId);
    if(pConfig == NULL)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_DEINT, XVPROCSS_EDAT_CFABSENT);
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
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_DEINT, XVPROCSS_EDAT_BADADDR);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_deinterlacer_CfgInitialize(&XVprocSsPtr->DeintPtr->Deint,
                                           pConfig,
                                           AbsAddr);

    if(status != XST_SUCCESS)
    {
      XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_DEINT, XVPROCSS_EDAT_INITFAIL);
      return(XST_FAILURE);
    }
  }

  XVprocSs_LogWrite(XVprocSsPtr, XVPROCSS_EVT_CFG_DEINT, XVPROCSS_EDAT_INITOK);
  return(XST_SUCCESS);
}
/** @} */
