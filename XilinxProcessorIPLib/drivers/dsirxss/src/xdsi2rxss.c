/******************************************************************************
* Copyright 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsi2rxss.c
* @addtogroup dsirxss Overview
* @{
*
* This is main code of AMD MIPI DSI Rx Subsystem device driver.
* Please see xdsirxss.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- ------- -------------------------------------------------------
* 1.0 Kunal 12/02/24 Initial Release for MIPI DSI RX subsystem
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xdebug.h"
#include "xdsi2rx.h"
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
#include "xdphy.h"
#endif
#include "xdsi2rxss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/**
 * This typedef declares the driver instances of all the cores in the subsystem
 */
typedef struct {
	XDsi2Rx Dsi2RxInst;
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy DphyInst;
#endif
} XDsi2Ss_SubCores;

/**************************** Variable Definitions ***********************************/
XDsi2Ss_SubCores Dsi2SsSubCores[];
/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static void XDsi2RxSs_GetIncludedSubCores(XDsi2RxSs *Dsi2RxSsPtr);
static s32 XDsi2RxSs_SubCoreInitDsi(XDsi2RxSs *Dsi2RxSsPtr);
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
static s32 XDsi2RxSs_SubCoreInitDphy(XDsi2RxSs *Dsi2RxSsPtr);
#endif
static s32 ComputeSubCoreAbsAddr(UINTPTR SsBaseAddr, UINTPTR SsHighAddr,
					u32 Offset, UINTPTR *BaseAddr);

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
* This function initializes the MIPI DSI RX subsystem and included sub-cores.
* This function must be called prior to using the subsystem. Initialization
* includes setting up the instance data for top level as well as all included
* sub-core therein, and ensuring the hardware is in a known stable state
*
* @param	InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param	CfgPtr points to the configuration structure associated with the
*		subsystem instance.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
		- XST_SUCCESS if initialization is successful
		- XST_FAILURE if initialization is failure
*
* @note		None.
*
******************************************************************************/
s32 XDsi2RxSs_CfgInitialize(XDsi2RxSs *InstancePtr, XDsi2RxSs_Config *CfgPtr,
							UINTPTR EffectiveAddr)
{
	s32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != 0);

	/* Setup the instance */
	InstancePtr->Config = *CfgPtr;
	InstancePtr->Config.BaseAddr = EffectiveAddr;

	/* Determine sub-cores included in the
	 * provided instance of subsystem
	 */
	XDsi2RxSs_GetIncludedSubCores(InstancePtr);

	if (InstancePtr->Dsi2RxPtr) {
		Status = XDsi2RxSs_SubCoreInitDsi(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->DphyPtr != NULL) {
		Status = XDsi2RxSs_SubCoreInitDphy(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}
#endif
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is used to configure the DSI2RX default parameters that are to
* be handled by the application. It will configure protocol register with Pixel
* mode.
*
* @param	InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return
*		- XST_SUCCESS on successful configuration of parameters
*		- XST_FAILURE on failure scenraio
*
* @note		None.
*
******************************************************************************/
u32 XDsi2RxSs_DefaultConfigure(XDsi2RxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Status = XDsi2Rx_DefaultConfigure(InstancePtr->Dsi2RxPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is used to activate the DSI2RX Subsystem. Internally it activates
* the DPHY and DSI2RX. Enable/Disable IP core to start processing
*
* @param	InstancePtr is a pointer to the Subsystem instance to be worked on.
* @param	core is used to denote the subcore of subsystem
* @param        Flag is used to denote whether to enable or disable the subsystem
*
* @return	XST_SUCCESS is returned if subcore(DSI/DPHY) was
*			successfully enabled or disabled
*		XST_INVALID_PARAM is returned if subsystem core is not found
*
* @note		None.
*
******************************************************************************/
int XDsi2RxSs_Activate(XDsi2RxSs *InstancePtr, XDsi2RxSs_SubCore core, u8 Flag)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Flag <= XDSI2RXSS_ENABLE);
	Xil_AssertNonvoid(InstancePtr->Dsi2RxPtr != NULL);
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	Xil_AssertNonvoid(InstancePtr->DphyPtr != NULL);
#endif
	if (core == XDSI2RXSS_DSI)
		XDsi2Rx_Activate(InstancePtr->Dsi2RxPtr, Flag);
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	else if (core == XDSI2RXSS_PHY)
		XDphy_Activate(InstancePtr->DphyPtr, Flag);
#endif
	else
		return XST_INVALID_PARAM;

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
* This function is used to reset the DSI2RX Subsystem. Internally it resets
* the DPHY and DSI
*
* @param	InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void XDsi2RxSs_Reset(XDsi2RxSs *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Dsi2RxPtr);

	XDsi2Rx_Reset(InstancePtr->Dsi2RxPtr);
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->Config.IsDphyRegIntfcPresent && InstancePtr->DphyPtr) {
		XDphy_Reset(InstancePtr->DphyPtr);
	}
#endif
}

/*****************************************************************************/
/**
* This function reports list of cores included in DSI RX Subsystem
*
* @param	InstancePtr is a pointer to the DSI RX Subsystem instance.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void XDsi2RxSs_ReportCoreInfo(XDsi2RxSs *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Dsi2RxPtr);

	xdbg_printf(XDBG_DEBUG_GENERAL, "\n\r  ->MIPI DSI2RX Subsystem Cores\n\r");

	/* Report all the included cores in the subsystem instance */
	if (InstancePtr->Dsi2RxPtr) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "  : DSI2 Rx Controller \n\r");
	}

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->Config.DphyInfo.IsPresent && InstancePtr->DphyPtr) {
		xdbg_printf(XDBG_DEBUG_GENERAL,"  : DPhy ");
		if (InstancePtr->DphyPtr->Config.IsRegisterPresent) {
			xdbg_printf(XDBG_DEBUG_GENERAL,"with ");
		}
		else {
			xdbg_printf(XDBG_DEBUG_GENERAL,"without ");
		}
		xdbg_printf(XDBG_DEBUG_GENERAL,"register interface \n\r");
	}
#endif
}

/*****************************************************************************/
/**
* This function will get the information from the GUI settings
*
* @param	InstancePtr is the XDsi instance to operate on
*
* @return 	None
*
* @note		None.
*
****************************************************************************/
void XDsi2RxSs_GetConfigParams(XDsi2RxSs *InstancePtr)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);

	XDsi2Rx_GetConfigParams(InstancePtr->Dsi2RxPtr, &InstancePtr->ConfigInfo);
}

/*****************************************************************************/
/**
* This function will get the information from the GUI settings
*
* @param	InstancePtr is the XDsi2RxSs instance to operate on
*
* @return 	Controller ready status
*
* @note		None.
*
****************************************************************************/
u32 XDsi2RxSs_IsControllerReady(XDsi2RxSs *InstancePtr)
{
	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDsi2Rx_IsControllerReady(InstancePtr->Dsi2RxPtr);
}

/****************************************************************************/
/**
*
* This function is used to get pixel format
*
* @param	InstancePtr is a pointer to the DsiRxSs Instance to be
*		worked on.
*
* @return	0x0E â€“ Packed RGB565
*		0x1E- packed RGB666
*		0x2E â€“ Loosely packed RGB666
*		0x3E- Packed RGB888
*		0x0B- Compressed Pixel Stream
*
* @note		None
*
****************************************************************************/
u32 XDsi2RxSs_GetPixelFormat(XDsi2RxSs *InstancePtr)
{
	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDsi2Rx_GetPixelFormat(InstancePtr->Dsi2RxPtr);
}

/*****************************************************************************/
/**
* This function queries the subsystem instance configuration to determine
* the included sub-cores. For each sub-core that is present in the design
* the sub-core driver instance is binded with the subsystem sub-core driver
* handle
*
* @param	Dsi2RxSsPtr is a pointer to the Subsystem instance
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void XDsi2RxSs_GetIncludedSubCores(XDsi2RxSs *Dsi2RxSsPtr)
{
	/* Verify argument */
	Xil_AssertVoid(Dsi2RxSsPtr != NULL);
	u32 Index = 0;
	Index = XDsi2RxSs_GetDrvIndex(Dsi2RxSsPtr, Dsi2RxSsPtr->Config.BaseAddr);

	Dsi2RxSsPtr->Dsi2RxPtr = ((Dsi2RxSsPtr->Config.Dsi2RxInfo.IsPresent) ?
				(&Dsi2SsSubCores[Index].Dsi2RxInst) : NULL);
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	Dsi2RxSsPtr->DphyPtr = ((Dsi2RxSsPtr->Config.DphyInfo.IsPresent) ?
				(&Dsi2SsSubCores[Index].DphyInst) : NULL);
#endif
}

/*****************************************************************************/
/**
* This function initializes the DSI2 RX sub-core initialization
*
* @param	Dsi2RxSsPtr is a pointer to the Subsystem instance
*
* @return
		- XST_SUCCESS on DSI2 RX sub core initialization
		- XST_FAILURE on DSI2 RX fail initialization
*
* @note		None
*
******************************************************************************/
static s32 XDsi2RxSs_SubCoreInitDsi(XDsi2RxSs *Dsi2RxSsPtr)
{
	s32 Status;
	UINTPTR AbsAddr;
	XDsi2Rx_Config *ConfigPtr;

	if (!Dsi2RxSsPtr->Dsi2RxPtr) {
		return XST_FAILURE;
	}

	/* Get core configuration */
	xdbg_printf(XDBG_DEBUG_GENERAL, ">Initializing DSI2 Rx Controller...\n\r");
	ConfigPtr = XDsi2Rx_LookupConfig(Dsi2RxSsPtr->Config.Dsi2RxInfo.AddrOffset);
	if (ConfigPtr == NULL) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSI2RXSS ERR:: DSI2 RX not found\n\r");
		return XST_FAILURE;
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = ComputeSubCoreAbsAddr(Dsi2RxSsPtr->Config.BaseAddr,
					Dsi2RxSsPtr->Config.HighAddr,
					Dsi2RxSsPtr->Config.Dsi2RxInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSI2RXSS ERR:: DSI2 RX core base"
			"address (0x%x) invalid \n\r", AbsAddr);
		return XST_FAILURE;
	}

	/* Initialize core */
	Status = XDsi2Rx_CfgInitialize(Dsi2RxSsPtr->Dsi2RxPtr, ConfigPtr, AbsAddr);

	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSI2RXSS ERR:: DSI core"
			"Initialization failed\n\r");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
* This function initializes the Dphy sub core
*
* @param	Dsi2RxSsPtr is a pointer to the Subsystem instance
*
* @return
		- XST_SUCCESS on successful initialization of Dphy
		- XST_FAILURE on Dphy initialization failure
*
* @note		None
*
******************************************************************************/
static s32 XDsi2RxSs_SubCoreInitDphy(XDsi2RxSs *Dsi2RxSsPtr)
{
	s32 Status;
	UINTPTR AbsAddr;
	XDphy_Config *ConfigPtr;

	if (!Dsi2RxSsPtr->DphyPtr) {
		return XST_FAILURE;
	}

	/* Get core configuration */
	xdbg_printf(XDBG_DEBUG_GENERAL, "->Initializing DPHY ...\n\r");

	ConfigPtr = XDphy_LookupConfig(Dsi2RxSsPtr->Config.DphyInfo.AddrOffset);

	if (!ConfigPtr) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSIRXSS ERR:: DPHY not found \n\r");
		return (XST_FAILURE);
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = ComputeSubCoreAbsAddr(Dsi2RxSsPtr->Config.BaseAddr,
					Dsi2RxSsPtr->Config.HighAddr,
					Dsi2RxSsPtr->Config.DphyInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSIRXSS ERR:: DPHY core base address "
				"(0x%x) invalid \n\r", AbsAddr);
		return XST_FAILURE;
	}

	/* Initialize core */
	Status = XDphy_CfgInitialize(Dsi2RxSsPtr->DphyPtr, ConfigPtr, AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSIRXSS ERR:: Dphy core Initialization "
				"failed \n\r");
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
* The subsystem is aligned to 128K address and has address range of max 192K
* By default, DSI is at offset 0x0_0000 and DPHY is at offset 0x1_0000.
* In case DPHY register interface is also absent then the address
* range shrinks to 64K with only the DSI subcore at offset 0x0_0000.
*
* @param	SsBaseAddr is the base address of the the Subsystem instance
* @param	SsHighAddr is the max address of the Subsystem instance
* @param	Offset is the offset of the specified core
* @param	BaseAddr is the computed absolute base address of the subcore
*
* @return
*		- XST_SUCCESS if base address computation is successful
*		  and within subsystem address range
*		- XST_FAILURE on address out of range
*
* @note		None
*
******************************************************************************/
static s32 ComputeSubCoreAbsAddr(UINTPTR SsBaseAddr,
				UINTPTR SsHighAddr,
				u32 Offset,
				UINTPTR *BaseAddr)
{
	s32 Status;
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
