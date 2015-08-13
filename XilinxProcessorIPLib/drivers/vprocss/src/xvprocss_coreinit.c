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
* @addtogroup vprocss_v1_0
* @{

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
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitResetAxis(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XGpio_Config *pConfig;

  if(pVprocss->rstAxis)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing AXIS Reset core.... \r\n");
    pConfig  = XGpio_LookupConfig(pVprocss->Config.RstAxis.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Reset AXIS device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.RstAxis.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Reset AXIS core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XGpio_CfgInitialize(pVprocss->rstAxis,
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
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitResetAximm(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XGpio_Config *pConfig;

  if(pVprocss->rstAximm)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing AXI-MM Reset core.... \r\n");
    pConfig  = XGpio_LookupConfig(pVprocss->Config.RstAximm .DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Reset AXI-MM device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.RstAximm.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Reset AXI-MM core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XGpio_CfgInitialize(pVprocss->rstAximm,
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
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitRouter(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XAxis_Switch_Config *pConfig;

  if(pVprocss->router)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing AXIS Switch core.... \r\n");
    pConfig  = XAxisScr_LookupConfig(pVprocss->Config.Router.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: AXIS Switch device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.Router.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: AXIS Switch core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XAxisScr_CfgInitialize(pVprocss->router,
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
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitCsc(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XV_csc_Config *pConfig;

  if(pVprocss->csc)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing CSC core.... \r\n");
    pConfig  = XV_csc_LookupConfig(pVprocss->Config.Csc.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: CSC device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.Csc.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: CSC core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_csc_CfgInitialize(pVprocss->csc,
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
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitHScaler(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XV_hscaler_Config *pConfig;

  if(pVprocss->hscaler)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Horiz. Scaler core.... \r\n");
    pConfig  = XV_hscaler_LookupConfig(pVprocss->Config.Hscale.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Horiz. Scaler device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.Hscale.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Horiz. Scaler core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_hscaler_CfgInitialize(pVprocss->hscaler,
                                      pConfig,
                                      AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Horiz. Scaler Initialization failed\r\n");
      return(XST_FAILURE);
    }

    /* Load Default filter coefficients */
    XV_HScalerLoadDefaultCoeff(pVprocss->hscaler, &pVprocss->hscL2Reg);
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitVScaler(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XV_vscaler_Config *pConfig;

  if(pVprocss->vscaler)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Vert. Scaler core.... \r\n");
    pConfig  = XV_vscaler_LookupConfig(pVprocss->Config.Vscale.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Vert. Scaler device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.Vscale.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Vert. Scaler core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_vscaler_CfgInitialize(pVprocss->vscaler,
                                      pConfig,
                                      AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Vert. Scaler Initialization failed\r\n");
      return(XST_FAILURE);
    }

    /* Load Default filter coefficients */
    XV_VScalerLoadDefaultCoeff(pVprocss->vscaler, &pVprocss->vscL2Reg);

  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitHCrsmplr(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XV_hcresampler_Config *pConfig;

  if(pVprocss->hcrsmplr)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing H Chroma Resampler core.... \r\n");
    pConfig  = XV_hcresampler_LookupConfig(pVprocss->Config.HCrsmplr.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: H Chroma Resampler device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.HCrsmplr.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: H Chroma Resampler core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_hcresampler_CfgInitialize(pVprocss->hcrsmplr,
                                          pConfig,
                                          AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: H Chroma Resampler Initialization failed\r\n");
      return(XST_FAILURE);
    }

    /* Load default filter coefficients */
    XV_HCrsmplLoadDefaultCoeff(pVprocss->hcrsmplr, &pVprocss->hcrL2Reg);
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitVCrsmpleIn(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XV_vcresampler_Config *pConfig;

  if(pVprocss->vcrsmplrIn)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Input V Chroma Resampler core.... \r\n");
    pConfig  = XV_vcresampler_LookupConfig(pVprocss->Config.VCrsmplrIn.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Input V Chroma Resampler device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.VCrsmplrIn.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Input V Chroma Resampler core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_vcresampler_CfgInitialize(pVprocss->vcrsmplrIn,
                                          pConfig,
                                          AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Input V Chroma Resampler Initialization failed\r\n");
      return(XST_FAILURE);
    }

    /* Load default filter coefficients */
    XV_VCrsmplLoadDefaultCoeff(pVprocss->vcrsmplrIn, &pVprocss->vcrInL2Reg);
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitVCrsmpleOut(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XV_vcresampler_Config *pConfig;

  if(pVprocss->vcrsmplrOut)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Output V Chroma Resampler core.... \r\n");
    pConfig  = XV_vcresampler_LookupConfig(pVprocss->Config.VCrsmplrOut.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Output V Chroma Resampler device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.VCrsmplrOut.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Output V Chroma Resampler core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_vcresampler_CfgInitialize(pVprocss->vcrsmplrOut,
                                          pConfig,
                                          AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Output V Chroma Resampler Initialization failed\r\n");
      return(XST_FAILURE);
    }

    /* Load default filter coefficients */
    XV_VCrsmplLoadDefaultCoeff(pVprocss->vcrsmplrOut, &pVprocss->vcrOutL2Reg);
  }
  return(XST_SUCCESS);
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitLetterbox(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XV_letterbox_Config *pConfig;

  if(pVprocss->lbox)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Letterbox core.... \r\n");
    pConfig  = XV_letterbox_LookupConfig(pVprocss->Config.Lbox.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Letterbox device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.Lbox.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Letterbox core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_letterbox_CfgInitialize(pVprocss->lbox,
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
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitVdma(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XAxiVdma_Config *pConfig;

  if(pVprocss->vdma)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing VDMA core.... \r\n");
    pConfig  = XAxiVdma_LookupConfig(pVprocss->Config.Vdma.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: VDMA device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.Vdma.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: VDMA core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XAxiVdma_CfgInitialize(pVprocss->vdma,
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
* @param  pVprocss is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XVprocss_SubcoreInitDeinterlacer(XVprocss *pVprocss)
{
  int status;
  u32 AbsAddr;
  XV_deinterlacer_Config *pConfig;

  if(pVprocss->deint)
  {
	/* Get core configuration */
    xdbg_printf(XDBG_DEBUG_GENERAL,"    ->Initializing Deinterlacer core.... \r\n");
    pConfig  = XV_deinterlacer_LookupConfig(pVprocss->Config.Deint.DeviceId);
    if(pConfig == NULL)
    {
      xil_printf("VPROCSS ERR:: Deinterlacer device not found\r\n");
      return(XST_FAILURE);
    }

	/* Compute absolute base address */
    AbsAddr = 0;
    status = ComputeSubcoreAbsAddr(pVprocss->Config.BaseAddress,
		                       pVprocss->Config.HighAddress,
		                       pVprocss->Config.Deint.AddrOffset,
		                       &AbsAddr);

    if(status != XST_SUCCESS)
    {
      xil_printf("VPROCSS ERR:: Deinterlacer core base address (0x%x) invalid %d\r\n", AbsAddr);
      return(XST_FAILURE);
    }

	/* Initialize core */
    status = XV_deinterlacer_CfgInitialize(pVprocss->deint,
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
