/******************************************************************************
* Copyright (C) 2015 - 2022 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcsiss.c
* @addtogroup csiss Overview
* @{
*
* This is main code of Xilinx MIPI CSI Rx Subsystem device driver.
* Please see xcsiss.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 07/21/15 Initial release
* 1.1 sss 08/17/16 Added 64 bit support
*     sss 08/29/16 Added check for Dphy register interface
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xcsi.h"
#if (XPAR_XMIPI_RX_PHY_NUM_INSTANCES > 0)
#include "xmipi_rx_phy.h"
#endif
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
	XCsi CsiInst;
#if (XPAR_XMIPI_RX_PHY_NUM_INSTANCES > 0)
	XMipi_Rx_Phy MipiRxPhyInst;
#endif
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy DphyInst;
#endif
#if (XPAR_XIIC_NUM_INSTANCES > 0)
	XIic IicInst;
#endif
} XCsiSs_SubCores;

/**************************** Local Global ***********************************/

/* Define Driver instance of all sub-core included in the design */
#ifndef SDT
XCsiSs_SubCores CsiSsSubCores[XPAR_XCSISS_NUM_INSTANCES];
#else
XCsiSs_SubCores CsiSsSubCores[];
#endif
/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static void CsiSs_GetIncludedSubCores(XCsiSs *CsiSsPtr);
static u32 CsiSs_SubCoreInitCsi(XCsiSs *CsiSsPtr);
#if (XPAR_XIIC_NUM_INSTANCES > 0)
static u32 CsiSs_SubCoreInitIic(XCsiSs *CsiSsPtr);
#endif
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
static u32 CsiSs_SubCoreInitDphy(XCsiSs *CsiSsPtr);
#endif
#if (XPAR_XMIPI_RX_PHY_NUM_INSTANCES > 0)
static u32 CsiSs_SubCoreInitMipiRxPhy(XCsiSs *CsiSsPtr);
#endif

static u32 CsiSs_ComputeSubCoreAbsAddr(UINTPTR SsBaseAddr, UINTPTR SsHighAddr,
					u32 Offset, UINTPTR *BaseAddr);

/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function initializes the MIPI CSI subsystem and included sub-cores.
* This function must be called prior to using the subsystem. Initialization
* includes setting up the instance data for top level as well as all included
* sub-core therein, and ensuring the hardware is in a known stable state.
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
		worked on.
* @param	CfgPtr points to the configuration structure associated
*		with the subsystem instance.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*		- XST_SUCCESS if initialization is successful
*		- XST_FAILURE, otherwise
*
* @note		None.
*
******************************************************************************/
u32 XCsiSs_CfgInitialize(XCsiSs *InstancePtr, XCsiSs_Config *CfgPtr,
				UINTPTR EffectiveAddr)
{
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != 0);

	/* Setup the instance */
	InstancePtr->Config = *CfgPtr;
	InstancePtr->Config.BaseAddr = EffectiveAddr;

	/* Determine sub-cores included in provided instance of subsystem */
	CsiSs_GetIncludedSubCores(InstancePtr);

	/* Initialize all included sub_cores */
#if (XPAR_XIIC_NUM_INSTANCES > 0)
	if (InstancePtr->IicPtr) {
		Status = CsiSs_SubCoreInitIic(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}
#endif
	if (InstancePtr->CsiPtr) {
		Status = CsiSs_SubCoreInitCsi(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->Config.IsDphyRegIntfcPresent && InstancePtr->DphyPtr) {
		Status = CsiSs_SubCoreInitDphy(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}
#elif (XPAR_XMIPI_RX_PHY_NUM_INSTANCES > 0)
	if (InstancePtr->Config.IsMipiRxPhyRegIntfcPresent && InstancePtr->MipiRxPhyPtr) {
		Status = CsiSs_SubCoreInitMipiRxPhy(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

#endif
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	return XST_SUCCESS;
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
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
		worked on.
*
* @return	Pointer to IIC instance or NULL
*
* @note		This function is called after XCsiSs_CfgInitialize()
*		This function must be called prior to using the IIC functions.
*
******************************************************************************/
XIic* XCsiSs_GetIicInstance(XCsiSs *InstancePtr)
{
	/* Verify argument. */
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
* @param	InstancePtr is a pointer to the Subsystem instance to be
*		worked on.
* @param	ActiveLanes is no of active lanes to be configure. This value
*		ranges between 1 and 4. In case Dynamic Active Lane config is
*		enabled, this value can't exceed maximum lanes present. When
*		Dynamic Active Lane config is disabled, it should be equal to
*		maximum lanes.
* @param	IntrMask Indicates Mask for enable interrupts.
*
* @return
*		- XST_SUCCESS on successful configuration of parameters
* 		- XST_FAILURE otherwise
*
* @note		When EnableActiveLanes is 0, then the ActiveLanes parameter
*		passed should be equal to Max Lanes of design.
*
******************************************************************************/
u32 XCsiSs_Configure(XCsiSs *InstancePtr, u8 ActiveLanes, u32 IntrMask)
{
	XCsi *CsiPtr;
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->CsiPtr != NULL);
	Xil_AssertNonvoid(XCsi_IsActiveLaneCountValid(InstancePtr->CsiPtr,
						ActiveLanes));

	CsiPtr = InstancePtr->CsiPtr;

	IntrMask &= XCSI_ISR_ALLINTR_MASK;
	XCsi_IntrEnable(CsiPtr, IntrMask);

	CsiPtr->ActiveLanes = ActiveLanes;
	Status = XCsi_Configure(CsiPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is used to activate the CSI Subsystem. Internally it activates
* the DPHY and CSI.
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
		worked on.
* @param	Flag is used to denote whether to enable or disable the
*		subsystem
*
* @return
*		- XST_SUCCESS on successful operation
*		- XST_FAILURE on failed operation
*
* @note		None
*
******************************************************************************/
u32 XCsiSs_Activate(XCsiSs *InstancePtr, u8 Flag)
{
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Flag <= XCSI_ENABLE);

	Status = XCsi_Activate(InstancePtr->CsiPtr, Flag);
	if (Status != XST_SUCCESS)
		return Status;

#if (XPAR_XMIPI_RX_PHY_NUM_INSTANCES > 0)
	if (InstancePtr->Config.IsMipiRxPhyRegIntfcPresent && InstancePtr->MipiRxPhyPtr) {
		XMipi_Rx_Phy_Activate(InstancePtr->MipiRxPhyPtr, Flag);
	}
#endif
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->Config.IsDphyRegIntfcPresent && InstancePtr->DphyPtr) {
		XDphy_Activate(InstancePtr->DphyPtr, Flag);
	}
#endif
	return Status;
}

/*****************************************************************************/
/**
* This function is used to reset the CSI Subsystem. Internally it resets
* the DPHY and CSI only as the IIC instance is separately handled by the
* application.
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
*		worked on.
*
* @return
*		- XST_SUCCESS if all the sub core IP resets occur correctly
*		- XST_FAILURE if reset fails for any sub core IP fails
*
* @note		None
*
******************************************************************************/
u32 XCsiSs_Reset(XCsiSs *InstancePtr)
{
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr);
	Xil_AssertNonvoid(InstancePtr->CsiPtr);

	Status = XCsi_Reset(InstancePtr->CsiPtr);
	if (Status == XST_FAILURE) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSI SubSys Reset failed\n\r");
	}

	return Status;
}

/*****************************************************************************/
/**
* This function reports list of cores included.
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
* 		worked on.
*
* @return	None
*
* @note		None
*
******************************************************************************/
void XCsiSs_ReportCoreInfo(XCsiSs *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xdbg_printf(XDBG_DEBUG_GENERAL,"\n\r  ->MIPI CSI Subsystem Cores\n\r");

	/* Report all the included cores in the subsystem instance */
	if (InstancePtr->CsiPtr) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"    : CSI Rx Controller \n\r");
	}

#if (XPAR_XMIPI_RX_PHY_NUM_INSTANCES > 0)
	if (InstancePtr->MipiRxPhyPtr) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"    : XMipi Rx Phy ");
		if (InstancePtr->Config.IsMipiRxPhyRegIntfcPresent) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"with ");
		}
		else {
			xdbg_printf(XDBG_DEBUG_GENERAL,"without ");
		}

		xdbg_printf(XDBG_DEBUG_GENERAL,"register interface \n\r");
	}
#endif
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->DphyPtr) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"    : DPhy ");
		if (InstancePtr->Config.IsDphyRegIntfcPresent) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"with ");
		}
		else {
			xdbg_printf(XDBG_DEBUG_GENERAL,"without ");
		}

		xdbg_printf(XDBG_DEBUG_GENERAL,"register interface \n\r");
	}
#endif

#if (XPAR_XIIC_NUM_INSTANCES > 0)
	if (InstancePtr->IicPtr) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"	: IIC \n\r");
	}
#endif
}


/*****************************************************************************/
/**
* This function will control the virtual channels selection dynamically.
*
* @param	InstancePtr is the XCsi instance to operate on
* @param	Value will set the virtual channels corresponding to each bit value.
*
* @return 	None
*
****************************************************************************/
void XCsiSs_SetVCSelection(XCsiSs *InstancePtr, u16 Value)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);

	/*CMN_VC=all means VcNo value is 4 if vcx feature support is disabled and
	 * VcNo value is 16 if vcx feature support is enabled*/
	if(((!InstancePtr->Config.EnableVCx) && (InstancePtr->Config.VcNo != XCSI_V10_MAX_VC)) ||
			((InstancePtr->Config.EnableVCx) && (InstancePtr->Config.VcNo != XCSI_V20_MAX_VC))) {
		xdbg_printf(XDBG_DEBUG_ERROR,"CSISS ERR:: To support dynamic VC Selection,"
				" User has to select 'Allowed VC to all' in IP Configuration. \n\r");
		return;
	}

	XCsi_SetVCSelection(InstancePtr->CsiPtr, Value);
}

/*****************************************************************************/
/**
* This function will return the virtual channels selected.
*
* @param	InstancePtr is the XCsi instance to operate on
*
* @return 	Value of selected VCs.
*
****************************************************************************/
u32 XCsiSs_GetVCSelection(XCsiSs *InstancePtr)
{
	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*CMN_VC=all means VcNo value is 4 if vcx feature support is disabled and
	 *  VcNo value is 16 if vcx feature support is enabled*/
	if(((!InstancePtr->Config.EnableVCx) && (InstancePtr->Config.VcNo != XCSI_V10_MAX_VC)) ||
			((InstancePtr->Config.EnableVCx) && (InstancePtr->Config.VcNo != XCSI_V20_MAX_VC))) {
		xdbg_printf(XDBG_DEBUG_ERROR,"CSISS ERR:: To support dynamic VC Selection,"
				" User has to select 'Allowed VC to all' in IP Configuration. \n\r");
		return 0;
	}

	return XCsi_GetVCSelection(InstancePtr->CsiPtr);
}

/*****************************************************************************/
/**
* This function gets the short packets
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
*		worked on.
*
* @return	None
*
* @note		None
*
******************************************************************************/
void XCsiSs_GetShortPacket(XCsiSs *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XCsi_GetShortPacket(InstancePtr->CsiPtr, &InstancePtr->SpktData);
}

/*****************************************************************************/
/**
* This function gets the clk and data lane info
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
		worked on.
*
* @return	None
*
* @note		None
*
******************************************************************************/
void XCsiSs_GetLaneInfo(XCsiSs *InstancePtr)
{
	u8 Index;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XCsi_GetClkLaneInfo(InstancePtr->CsiPtr, &(InstancePtr->ClkInfo));

	for (Index = 0; Index < InstancePtr->Config.LanesPresent; Index++) {
		XCsi_GetDataLaneInfo(InstancePtr->CsiPtr, Index,
				&(InstancePtr->DLInfo[Index]));
	}
}

/*****************************************************************************/
/**
* This function gets the virtual channel information
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
		worked on.
*
* @return	None
*
* @note		None
*
******************************************************************************/
void XCsiSs_GetVCInfo(XCsiSs *InstancePtr)
{
	u8 Index;
	u8 MaxVC;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	if(InstancePtr->Config.EnableVCx)
		MaxVC = XCSI_V20_MAX_VC;
	else
		MaxVC = XCSI_V10_MAX_VC;

	for (Index = 0; Index < MaxVC; Index++) {
		XCsi_GetVCInfo(InstancePtr->CsiPtr, Index,
			&InstancePtr->VCInfo[Index]);
	}
}

/*****************************************************************************/
/**
* This function queries the subsystem instance configuration to determine
* the included sub-cores. For each sub-core that is present in the design
* the sub-core driver instance is binded with the subsystem sub-core driver
* handle
*
* @param	CsiSsPtr is a pointer to the Subsystem instance to be worked.
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void CsiSs_GetIncludedSubCores(XCsiSs *CsiSsPtr)
{
#ifndef SDT
	CsiSsPtr->CsiPtr = ((CsiSsPtr->Config.CsiInfo.IsPresent) ?
		(&CsiSsSubCores[CsiSsPtr->Config.DeviceId].CsiInst) : NULL);

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	CsiSsPtr->DphyPtr = ((CsiSsPtr->Config.DphyInfo.IsPresent) ?
		(&CsiSsSubCores[CsiSsPtr->Config.DeviceId].DphyInst) : NULL);
#endif

#if (XPAR_XIIC_NUM_INSTANCES > 0)
	CsiSsPtr->IicPtr = ((CsiSsPtr->Config.IicInfo.IsPresent) ?
		(&CsiSsSubCores[CsiSsPtr->Config.DeviceId].IicInst) : NULL);
#endif
#else
	u32 Index = 0;
	Index = XCsiSs_GetDrvIndex(CsiSsPtr, CsiSsPtr->Config.BaseAddr);

	CsiSsPtr->CsiPtr = ((CsiSsPtr->Config.CsiInfo.IsPresent) ?
		(&CsiSsSubCores[Index].CsiInst) : NULL);

#if (XPAR_XMIPI_RX_PHY_NUM_INSTANCES > 0)
	CsiSsPtr->MipiRxPhyPtr = ((CsiSsPtr->Config.MipiRxPhyInfo.IsPresent) ?
		(&CsiSsSubCores[Index].MipiRxPhyInst) : NULL);
#endif
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	CsiSsPtr->DphyPtr = ((CsiSsPtr->Config.DphyInfo.IsPresent) ?
		(&CsiSsSubCores[Index].DphyInst) : NULL);
#endif

#if (XPAR_XIIC_NUM_INSTANCES > 0)
	CsiSsPtr->IicPtr = ((CsiSsPtr->Config.IicInfo.IsPresent) ?
		(&CsiSsSubCores[Index].IicInst) : NULL);
#endif
#endif
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param	CsiSsPtr is a pointer to the Subsystem instance to be worked.
*
* @return
*		- XST_SUCCESS If CSI sub core is initialised sucessfully
*		- XST_FAILURE If CSI sub core inititlization fails
*
* @note		None
*
******************************************************************************/
static u32 CsiSs_SubCoreInitCsi(XCsiSs *CsiSsPtr)
{
	u32 Status;
	UINTPTR AbsAddr;
	XCsi_Config *ConfigPtr;

	/* Get core configuration */
	xdbg_printf(XDBG_DEBUG_GENERAL, "->Initializing CSI Rx Controller...\n\r");
#ifndef SDT
	ConfigPtr = XCsi_LookupConfig(CsiSsPtr->Config.CsiInfo.DeviceId);
#else
	ConfigPtr = XCsi_LookupConfig(CsiSsPtr->Config.CsiInfo.AddrOffset);
#endif
	if (ConfigPtr == NULL) {
		xdbg_printf(XDBG_DEBUG_ERROR,"CSISS ERR:: CSI not found\n\r");
		return XST_FAILURE;
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = CsiSs_ComputeSubCoreAbsAddr(CsiSsPtr->Config.BaseAddr,
					CsiSsPtr->Config.HighAddr,
					CsiSsPtr->Config.CsiInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSISS ERR:: CSI core base "
			"address (0x%x) invalid %d\n\r", AbsAddr);
		return XST_FAILURE;
	}

	/* Initialize core */
	Status = XCsi_CfgInitialize(CsiSsPtr->CsiPtr, ConfigPtr, AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSISS ERR:: CSI core "
			"Initialization failed\n\r");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

#if (XPAR_XIIC_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param	CsiSsPtr is a pointer to the Subsystem instance to be worked.
*
* @return
*		- XST_SUCCESS If IIC sub core is initialised sucessfully
*		- XST_FAILURE Otherwise
*
* @note		None
*
******************************************************************************/
static u32 CsiSs_SubCoreInitIic(XCsiSs *CsiSsPtr)
{
	u32  Status;
	UINTPTR AbsAddr;
	XIic_Config *ConfigPtr;

	/* Get core configuration */
	xdbg_printf(XDBG_DEBUG_GENERAL,"->Initializing IIC MIPI CSI "
		"subsystem.\n\r");
#ifndef SDT
	ConfigPtr = XIic_LookupConfig(CsiSsPtr->Config.IicInfo.DeviceId);
#else
	ConfigPtr = XIic_LookupConfig(CsiSsPtr->Config.IicInfo.AddrOffset);
#endif
	if (!ConfigPtr) {
		xdbg_printf(XDBG_DEBUG_ERROR,"CSISS ERR:: IIC not found\n\r");
		return XST_FAILURE;
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = CsiSs_ComputeSubCoreAbsAddr(CsiSsPtr->Config.BaseAddr,
					CsiSsPtr->Config.HighAddr,
					CsiSsPtr->Config.IicInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,"CSISS ERR:: Iic core base "
			"address (0x%x) invalid %d\n\r", AbsAddr);
		return XST_FAILURE;
	}

	/* Initialize core */
	Status = XIic_CfgInitialize(CsiSsPtr->IicPtr, ConfigPtr, AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSISS ERR:: Iic core "
			"Initialization failed\n\r");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
#endif

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param	CsiSsPtr is a pointer to the Subsystem instance to be worked.
*
* @return
*		- XST_SUCCESS If DPHY sub core is initialised sucessfully
*		- XST_FAILURE If DPHY sub core initialization failed
*
* @note		None
*
******************************************************************************/
static u32 CsiSs_SubCoreInitDphy(XCsiSs *CsiSsPtr)
{
	u32 Status;
	UINTPTR AbsAddr;
	XDphy_Config *ConfigPtr;

	/* Get core configuration */
	xdbg_printf(XDBG_DEBUG_GENERAL, "->Initializing DPHY ...\n\r");
#ifndef SDT
	ConfigPtr = XDphy_LookupConfig(CsiSsPtr->Config.DphyInfo.DeviceId);
#else
	ConfigPtr = XDphy_LookupConfig(CsiSsPtr->Config.DphyInfo.AddrOffset);
#endif
	if (!ConfigPtr) {
		xdbg_printf(XDBG_DEBUG_ERROR,"CSISS ERR:: DPHY not found\n\r");
		return XST_FAILURE;
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = CsiSs_ComputeSubCoreAbsAddr(CsiSsPtr->Config.BaseAddr,
					CsiSsPtr->Config.HighAddr,
					CsiSsPtr->Config.DphyInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,"CSISS ERR:: DPHY core base "
			"address (0x%x) invalid %d\n\r", AbsAddr);
		return XST_FAILURE;
	}

	/* Initialize core */
	Status = XDphy_CfgInitialize(CsiSsPtr->DphyPtr, ConfigPtr, AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSISS ERR:: Dphy core "
			"Initialization failed\n\r");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
#endif

#if (XPAR_XMIPI_RX_PHY_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param	CsiSsPtr is a pointer to the Subsystem instance to be worked.
*
* @return
*		- XST_SUCCESS If DPHY sub core is initialised sucessfully
*		- XST_FAILURE If DPHY sub core initialization failed
*
* @note		None
*
******************************************************************************/
static u32 CsiSs_SubCoreInitMipiRxPhy(XCsiSs *CsiSsPtr)
{
	u32 Status;
	UINTPTR AbsAddr;
	XMipi_Rx_Phy_Config *ConfigPtr;

	/* Get core configuration */
	xdbg_printf(XDBG_DEBUG_GENERAL, "->Initializing MIPI RX PHY ...\n\r");

	ConfigPtr = XMipi_Rx_Phy_LookupConfig(CsiSsPtr->Config.MipiRxPhyInfo.AddrOffset);

	if (!ConfigPtr) {
		xdbg_printf(XDBG_DEBUG_ERROR,"CSISS ERR:: MIPI RX PHY not found\n\r");
		return XST_FAILURE;
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = CsiSs_ComputeSubCoreAbsAddr(CsiSsPtr->Config.BaseAddr,
					CsiSsPtr->Config.HighAddr,
					CsiSsPtr->Config.MipiRxPhyInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,"CSISS ERR:: MIPI RX PHY core base "
			"address (0x%x) invalid %d\n\r", AbsAddr);
		return XST_FAILURE;
	}

	/* Initialize core */
	Status = XMipi_Rx_Phy_CfgInitialize(CsiSsPtr->MipiRxPhyPtr, ConfigPtr, AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSISS ERR:: Mipi  Rx Phy core "
			"Initialization failed\n\r");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
* This function computes the subcore absolute address on axi-lite interface
* Subsystem is mapped at an absolute address and all included sub-cores are
* at pre-defined offset from the subsystem base address. To access the subcore
* register map from host CPU an absolute address is required.
* The subsystem is aligned to 64K address and has address range of max 192K
* (0x00000-0x2FFFF) in case IIC is present and DPHY register interface is
* selected. By default, CSI is at offset 0x0_0000, IIC is at offset 0x1_0000
* and DPHY is at offset 0x2_0000.
* In case of IIC being absent and DPHY has register interface, the address
* range shrinks to 128K (0x00000 - 0x1FFFF) with DPHY moving to offset
* 0x1_0000. In case DPHY register interface is also absent then the address
* range shrinks to 64K with only the CSI subcore at offset 0x0_0000.
*
* @param	SsBaseAddr is the base address of the the Subsystem instance
* @param	SsHighAddr is the max address of the Subsystem instance
* @param	Offset is the offset of the specified core
* @param	BaseAddr is the computed absolute base address of the subcore
*
* @return
*		- XST_SUCCESS if base address computation is successful and
*		within subsystem address range
*		- XST_FAILURE Otherwise
*
* @note		None
*
******************************************************************************/
static u32 CsiSs_ComputeSubCoreAbsAddr(UINTPTR SsBaseAddr, UINTPTR SsHighAddr,
					u32 Offset, UINTPTR *BaseAddr)
{
	u32 Status;
	UINTPTR AbsAddr;

	AbsAddr = SsBaseAddr + Offset;

	if ((AbsAddr >= SsBaseAddr) && (AbsAddr < SsHighAddr)) {
		*BaseAddr = AbsAddr;
		Status = XST_SUCCESS;
	}
	else {
		*BaseAddr = 0;
		Status = XST_FAILURE;
	}

	return Status;
}
/** @} */
