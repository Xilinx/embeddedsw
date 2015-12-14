/******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
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
* @file xcsiss.c
* @addtogroup csiss
* @{
* @details
*
* This is main code of Xilinx MIPI CSI Rx Subsystem device driver.
* Please see xcsiss.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vs    07/21/15   Initial Release

* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xcsi.h"
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
#include "xdphy.h"
#endif
#if (XPAR_XIIC_NUM_INSTANCES > 0)
#include "xiic.h"
#endif
#include "xcsiss.h"
/************************** Constant Definitions *****************************/
/**************************** Type Definitions *******************************/
/**
 * This typedef declares the driver instances of all the cores in the subsystem
 */
typedef struct {
	XCsi  CsiInst;
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy DphyInst;
#endif
#if (XPAR_XIIC_NUM_INSTANCES > 0)
	XIic  IicInst;
#endif
} XCsiSs_SubCores;

/**************************** Local Global ***********************************/
XCsiSs_SubCores CsiSsSubCores; /**< Define Driver instance of all sub-core
                                  included in the design */

/***************** Macros (Inline Functions) Definitions *********************/
/************************** Function Prototypes ******************************/
static void XCsiSs_GetIncludedSubCores(XCsiSs *CsiSsPtr);

/************************** Variable Definitions *****************************/
/*****************************************************************************/
/**
* This function initializes the MIPI CSI subsystem and included sub-cores.
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
int XCsiSs_CfgInitialize(XCsiSs *InstancePtr, XCsiSs_Config *CfgPtr,
				u32 EffectiveAddr)
{
	XCsiSs *CsiSsPtr = InstancePtr;
	int Status;
	u32 AbsAddr;

	/* Verify arguments */
	Xil_AssertNonvoid(CsiSsPtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (u32)NULL);

	/* Setup the instance */
	CsiSsPtr->Config = *CfgPtr;

	CsiSsPtr->Config.BaseAddr = EffectiveAddr;
	/* Determine sub-cores included in the provided instance of subsystem */
	XCsiSs_GetIncludedSubCores(CsiSsPtr);

	/* Initialize all included sub_cores */
#if (XPAR_XIIC_NUM_INSTANCES > 0)
	if (CsiSsPtr->IicPtr) {
		Status = XCsiSs_SubCoreInitIic(CsiSsPtr);
		if (Status != XST_SUCCESS) {
			return (XST_FAILURE);
		}
	}
#endif

	if (CsiSsPtr->CsiPtr) {
		Status = XCsiSs_SubCoreInitCsi(CsiSsPtr);
		if (Status != XST_SUCCESS) {
			return (XST_FAILURE);
		}
	}

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (CsiSsPtr->DphyPtr) {
		Status = XCsiSs_SubCoreInitDphy(CsiSsPtr);
		if (Status != XST_SUCCESS) {
			return (XST_FAILURE);
		}
	}
#endif

	CsiSsPtr->IsReady = XIL_COMPONENT_IS_READY;
	return (XST_SUCCESS);
}

#if (XPAR_XIIC_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
* This function returns the Iic Instance Ptr if Iic is present in the subsystem
* The application now will need to use Iic functions to access and configure
* the CSI Transmitter (Camera) via Camera Control Interface(subset protocol of
* IIC). Please refer to Camera specs for details on how to access and
* configure.
*
* This function is called after XCsiSs_CfgInitialize()
* This function must be called prior to using the IIC functions.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return Pointer to IIC instance or NULL
*
******************************************************************************/
XIic* XCsiSs_GetIicInstance(XCsiSs *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->IicPtr) {
		return InstancePtr->IicPtr;
	}

	return NULL;
}
#endif
/*****************************************************************************/
/**
* This function is used to configure the CSI lanes and interrupts that are to
* be handled by the application. Refer to XCsi.h for interrupt masks.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  UsrOpt is a pointer to the User options struct
*
* @return XST_SUCCESS on successful configuration of parameters
*         XST_FAILURE otherwise
*
******************************************************************************/
u32 XCsiSs_Configure(XCsiSs *InstancePtr, XCsiSs_UsrOpt *UsrOpt)
{
	XCsi *CsiPtr;
	u32 Status;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(UsrOpt != NULL);
	Xil_AssertNonvoid(InstancePtr->CsiPtr != NULL);

	CsiPtr = InstancePtr->CsiPtr;

	if (UsrOpt->Lanes > CsiPtr->Config.MaxLanesPresent) {
		return XST_FAILURE;
	}

	UsrOpt->IntrRequest &= XCSI_ISR_ALLINTR_MASK;

	CsiPtr->ActiveLanes = UsrOpt->Lanes;

	Status = XCsi_Configure(CsiPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XCsi_InterruptEnable(CsiPtr, UsrOpt->IntrRequest);

	InstancePtr->UsrOpt.Lanes = UsrOpt->Lanes;
	InstancePtr->UsrOpt.IntrRequest = UsrOpt->IntrRequest;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is used to get the user configuration of the CSI Subsystem.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return Pointer to XCsiSs_UsrOpt containing the user defined values.
*
******************************************************************************/

XCsiSs_UsrOpt * XCsiSs_GetUsrOpt(XCsiSs *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	return &(InstancePtr->UsrOpt);
}

/*****************************************************************************/
/**
* This function is used to activate the CSI Subsystem. Internally it activates
* the DPHY and CSI.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param  Flag is used to denote whether to enable or disable the subsystem
*
* @return None
*
******************************************************************************/
void XCsiSs_Activate(XCsiSs *InstancePtr, u8 Flag)
{
	u32 Status;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flag <= XCSI_ENABLE);

	XCsi_Activate(InstancePtr->CsiPtr, Flag);

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->DphyPtr->Config.IsRegisterPresent) {
		XDphy_Activate(InstancePtr->DphyPtr, Flag);
	}
#endif

	return;
}

/*****************************************************************************/
/**
* This function is used to reset the CSI Subsystem. Internally it resets
* the DPHY, CSI and the IIC.
*
* @param  InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @param  ResetType tells which or all subcores to be reset.
*
* @return XST_SUCCESS if all the sub core IP resets occur correctly
* 	  XST_FAILURE if reset fails for any sub core IP fails
*
******************************************************************************/
u32 XCsiSs_Reset(XCsiSs *InstancePtr, XCsiSs_ResetType ResetType)
{
	u32 Status = XST_SUCCESS;
	XCsi *CsiPtr;
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy *DphyPtr;
#endif
#if (XPAR_XIIC_NUM_INSTANCES > 0)
	XIic *IicPtr;
#endif
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(InstancePtr->CsiPtr);
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	Xil_AssertVoid(InstancePtr->DphyPtr);
#endif

	CsiPtr = InstancePtr->CsiPtr;
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	DphyPtr = InstancePtr->DphyPtr;
#endif

	switch (ResetType) {
		case XCSISS_RESET_SUBCORE_IIC:
			if (InstancePtr->Config.IsIICPresent) {
#if (XPAR_XIIC_NUM_INSTANCES > 0)
				IicPtr = InstancePtr->IicPtr;
				XIic_Reset(IicPtr);
#endif
			}
			break;
		case XCSISS_RESET_SUBCORE_CSI:
			Status = XCsi_Reset(CsiPtr);
			break;
		case XCSISS_RESET_SUBCORE_DPHY:
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
			if (DphyPtr->Config.IsRegisterPresent) {
				XDphy_Reset(DphyPtr);
			}
#endif
			break;
		case XCSISS_RESET_SUBCORE_ALL:
			if (InstancePtr->Config.IsIICPresent) {
#if (XPAR_XIIC_NUM_INSTANCES > 0)
				IicPtr = InstancePtr->IicPtr;
				XIic_Reset(IicPtr);
#endif
			}

			Status = XCsi_Reset(CsiPtr);

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
			if (DphyPtr->Config.IsRegisterPresent) {
				XDphy_Reset(DphyPtr);
			}
#endif
			break;
		default:
			break;
	}

	if (Status == XST_FAILURE) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSI SubSys Reset failed\r\n");
	}

	return Status;
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
void XCsiSs_ReportCoreInfo(XCsiSs *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	xil_printf("\r\n  ->MIPI CSI Subsystem Cores\r\n");

	/* Report all the included cores in the subsystem instance */
	if (InstancePtr->CsiPtr) {
		xil_printf("    : CSI Rx Controller \r\n");
	}

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->DphyPtr) {
		xil_printf("    : DPhy ");
		if (InstancePtr->DphyPtr->Config.IsRegisterPresent) {
			xil_printf("with ");
		}
		else {
			xil_printf("without ");
		}

		xil_printf("register interface \r\n");
	}
#endif

#if (XPAR_XIIC_NUM_INSTANCES > 0)
	if (InstancePtr->IicPtr) {
		xil_printf("	: IIC \r\n");
	}
#endif
}

/*****************************************************************************/
/**
* This function gets the short packets
*
* @param  InstancePtr is a pointer to the Subsystem instance.
*
* @return None
*
******************************************************************************/
void XCsiSs_GetShortPacket(XCsiSs *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);

	XCsi_GetShortPacket(InstancePtr->CsiPtr, &InstancePtr->SpktData);

}

/*****************************************************************************/
/**
* This function gets the clk and data lane info
*
* @param  InstancePtr is a pointer to the Subsystem instance.
*
* @return None
*
******************************************************************************/
void XCsiSs_GetLaneInfo(XCsiSs *InstancePtr)
{
	u8 Index;

	Xil_AssertVoid(InstancePtr != NULL);

	XCsi_GetClkLaneInfo(InstancePtr->CsiPtr, &(InstancePtr->ClkInfo));

	for (Index = 0; Index < InstancePtr->Config.LanesPresent; Index++) {
		XCsi_GetDataLaneInfo(InstancePtr->CsiPtr, Index,
				&(InstancePtr->DLInfo[Index]));
	}

}

/*****************************************************************************/
/**
* This function queries the subsystem instance configuration to determine
* the included sub-cores. For each sub-core that is present in the design
* the sub-core driver instance is binded with the subsystem sub-core driver
* handle
*
* @param  CsiSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return None
*
******************************************************************************/
static void XCsiSs_GetIncludedSubCores(XCsiSs *CsiSsPtr)
{
	CsiSsPtr->CsiPtr = ((CsiSsPtr->Config.CsiInfo.IsPresent) ?
				(&CsiSsSubCores.CsiInst) : NULL);
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	CsiSsPtr->DphyPtr = ((CsiSsPtr->Config.DphyInfo.IsPresent) ?
				(&CsiSsSubCores.DphyInst) : NULL);
#endif
#if (XPAR_XIIC_NUM_INSTANCES > 0)
	CsiSsPtr->IicPtr = ((CsiSsPtr->Config.IicInfo.IsPresent) ?
				(&CsiSsSubCores.IicInst) : NULL);
#endif
}
/** @} */
