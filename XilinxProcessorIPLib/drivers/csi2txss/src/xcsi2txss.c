/******************************************************************************
 *
 * Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* @file xcsi2txss.c
* @addtogroup csi2txss_v1_1
* @{
*
* This is main code of Xilinx MIPI CSI Tx Subsystem device driver.
* Please see xcsi2txss.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who   Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 sss 07/14/16 Initial release
*     vsa 15/12/17 Add support for Clock Mode
* 1.2 vsa 02/28/18 Add Frame End Generation feature
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdebug.h"
#include "xcsi2tx.h"
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
#include "xdphy.h"
#endif
#include "xcsi2txss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/**
 * This typedef declares the driver instances of all the cores in the subsystem
 */
typedef struct {
	XCsi2Tx CsiInst;
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy DphyInst;
#endif
} XCsi2TxSs_SubCores;

/**************************** Local Global ***********************************/

/* Define Driver instance of all sub-core included in the design */
XCsi2TxSs_SubCores Csi2TxSsSubCores[XPAR_XCSI2TXSS_NUM_INSTANCES];

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static void Csi2TxSs_GetIncludedSubCores(XCsi2TxSs *Csi2TxSsPtr);
static u32 Csi2TxSs_SubCoreInitCsi(XCsi2TxSs *Csi2TxSsPtr);
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
static u32 Csi2TxSs_SubCoreInitDphy(XCsi2TxSs *Csi2TxSsPtr);
#endif
static u32 Csi2TxSs_ComputeSubCoreAbsAddr(UINTPTR SsBaseAddr,
					UINTPTR SsHighAddr,
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
u32 XCsi2TxSs_CfgInitialize(XCsi2TxSs *InstancePtr, XCsi2TxSs_Config *CfgPtr,
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
	Csi2TxSs_GetIncludedSubCores(InstancePtr);

	/* Initialize all included sub_cores */
	if (InstancePtr->CsiPtr) {
		Status = Csi2TxSs_SubCoreInitCsi(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->Config.IsDphyRegIntfcPresent && InstancePtr->DphyPtr) {
		Status = Csi2TxSs_SubCoreInitDphy(InstancePtr);
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
static void Csi2TxSs_GetIncludedSubCores(XCsi2TxSs *Csi2TxSsPtr)
{
	Csi2TxSsPtr->CsiPtr = ((Csi2TxSsPtr->Config.CsiInfo.IsPresent) ?
	(&Csi2TxSsSubCores[Csi2TxSsPtr->Config.DeviceId].CsiInst) : NULL);

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	Csi2TxSsPtr->DphyPtr = ((Csi2TxSsPtr->Config.DphyInfo.IsPresent) ?
		(&Csi2TxSsSubCores[Csi2TxSsPtr->Config.DeviceId].DphyInst) : NULL);
#endif

}

/*****************************************************************************/
/**
* This function is used to configure the CSI lanes and interrupts that are to
* be handled by the application. Refer to XCsi2Tx.h for interrupt masks.
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
*		worked on.
* @param	ActiveLanes is no of active lanes to be configure. This value
*		ranges between 1 and 4.
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
u32 XCsi2TxSs_Configure(XCsi2TxSs *InstancePtr, u8 ActiveLanes, u32 IntrMask)
{
	XCsi2Tx *CsiPtr;
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->CsiPtr != NULL);
	Xil_AssertNonvoid(XCsi2Tx_IsActiveLaneCountValid(InstancePtr->CsiPtr,
						ActiveLanes));

	CsiPtr = InstancePtr->CsiPtr;

	if (InstancePtr->Config.FEGenEnabled) {
		IntrMask &= (XCSI2TX_ISR_ALLINTR_MASK |
			     XCSITX_LCSTAT_VC0_IER_MASK |
			     XCSITX_LCSTAT_VC1_IER_MASK |
			     XCSITX_LCSTAT_VC2_IER_MASK |
			     XCSITX_LCSTAT_VC3_IER_MASK);
	} else
		IntrMask &= XCSI2TX_ISR_ALLINTR_MASK;

	XCsi2Tx_IntrEnable(CsiPtr, IntrMask);

	CsiPtr->ActiveLanes = ActiveLanes;
	Status = XCsi2Tx_Configure(CsiPtr);
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
u32 XCsi2TxSs_Activate(XCsi2TxSs *InstancePtr, u8 Flag)
{
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Flag <= XCSI2TX_ENABLE);

	Status = XCsi2Tx_Activate(InstancePtr->CsiPtr, Flag);
	if (Status != XST_SUCCESS)
		return Status;

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
* the DPHY and CSI
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
u32 XCsi2TxSs_Reset(XCsi2TxSs *InstancePtr)
{
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr);
	Xil_AssertNonvoid(InstancePtr->CsiPtr);

	Status = XCsi2Tx_Reset(InstancePtr->CsiPtr);
	if (Status == XST_FAILURE) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSI2TX SubSys Reset faild\n\r");
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
void XCsi2TxSs_ReportCoreInfo(XCsi2TxSs *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	xdbg_printf(XDBG_DEBUG_INFO,"\n\r ->MIPI CSI2 TX Subsystem Cores\n\r");

	/* Report all the included cores in the subsystem instance */
	if (InstancePtr->CsiPtr) {
		xdbg_printf(XDBG_DEBUG_INFO,"    : CSI2 Tx Controller \n\r");
	}

#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->DphyPtr) {
		xdbg_printf(XDBG_DEBUG_INFO,"    : DPhy ");
		if (InstancePtr->Config.IsDphyRegIntfcPresent) {
			xdbg_printf(XDBG_DEBUG_INFO,"with ");
		}
		else {
			xdbg_printf(XDBG_DEBUG_INFO,"without ");
		}

		xdbg_printf(XDBG_DEBUG_INFO,"register interface \n\r");
	}
#endif

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
void XCsi2TxSs_GetShortPacket(XCsi2TxSs *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	XCsi2Tx_GetShortPacket(InstancePtr->CsiPtr, &InstancePtr->SpktData);
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
static u32 Csi2TxSs_SubCoreInitCsi(XCsi2TxSs *CsiSsPtr)
{
	u32 Status;
	UINTPTR AbsAddr;
	XCsi2Tx_Config *ConfigPtr;

	/* Get core configuration */
	xdbg_printf(XDBG_DEBUG_INFO, "->Initializing CSI2 Tx Controller.\n\r");
	ConfigPtr = XCsi2Tx_LookupConfig(CsiSsPtr->Config.CsiInfo.DeviceId);
	if (ConfigPtr == NULL) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			"CSISS2TX ERR:: CSI not found\n\r");
		return XST_FAILURE;
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = Csi2TxSs_ComputeSubCoreAbsAddr(CsiSsPtr->Config.BaseAddr,
					CsiSsPtr->Config.HighAddr,
					CsiSsPtr->Config.CsiInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSISS2TX ERR:: CSI core"
			"base address (0x%x) invalid %d\n\r", AbsAddr);
		return XST_FAILURE;
	}

	/* Initialize core */
	Status = XCsi2Tx_CfgInitialize(CsiSsPtr->CsiPtr, ConfigPtr, AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,
			"CSISS2TX ERR:: CSI core Initialization failed\n\r");
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}



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
static u32 Csi2TxSs_SubCoreInitDphy(XCsi2TxSs *CsiSsPtr)
{
	u32 Status;
	UINTPTR AbsAddr;
	XDphy_Config *ConfigPtr;

	/* Get core configuration */
	xdbg_printf(XDBG_DEBUG_INFO, "->Initializing DPHY ...\n\r");
	ConfigPtr = XDphy_LookupConfig(CsiSsPtr->Config.DphyInfo.DeviceId);
	if (!ConfigPtr) {
		xdbg_printf(XDBG_DEBUG_ERROR,
				"CSISS2TX ERR:: DPHY not found\n\r");
		return XST_FAILURE;
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = Csi2TxSs_ComputeSubCoreAbsAddr(CsiSsPtr->Config.BaseAddr,
					CsiSsPtr->Config.HighAddr,
					CsiSsPtr->Config.DphyInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR,"CSISS2TX ERR:: DPHY core base"
			"address (0x%x) invalid %d\n\r", AbsAddr);
		return XST_FAILURE;
	}

	/* Initialize core */
	Status = XDphy_CfgInitialize(CsiSsPtr->DphyPtr, ConfigPtr, AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "CSISS2TX ERR:: Dphy core"
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
* The subsystem is aligned to 4K address and has address range of max 8K
* (0x00000-0x2000). By default, CSI is at offset 0x0_0000, and DPHY is at
* offset 0x0_1000.
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
static u32 Csi2TxSs_ComputeSubCoreAbsAddr(UINTPTR SsBaseAddr,
					UINTPTR SsHighAddr,
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

/****************************************************************************/
/**
*
* This function is to set the Line Synchronization packet Generation status
*
* @param	InstancePtr is a pointer to the CSI2 TX SS Instance to be
*		worked on.
*
* @param	Value
* 		0 : DISABLE
* 		1 : ENABLE
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XCsi2TxSs_LineGen(XCsi2TxSs *InstancePtr, u32 Value) {
	XCsi2Tx_SetLineGen(InstancePtr->CsiPtr, Value);
}

/****************************************************************************/
/**
*
* This function is to set Generic Short Packet Entries
*
* @param	InstancePtr is a pointer to the CSI2 TX SS Instance to be
*		worked on.
*
* @param	Value
* 		GSP 32 bit Entry
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XCsi2TxSs_SetGSPEntry(XCsi2TxSs *InstancePtr, u32 Value) {
	XCsi2Tx_SetGSPEntry(InstancePtr->CsiPtr, Value);
}

/****************************************************************************/
/**
*
* This function is used to get the Pixel Mode
*
* @param	InstancePtr is a pointer to the CSI2TX SS Instance to be
*		worked on.
*
* @return	0x0  - Single Pixel Mode
* 		0x1  - Dual Pixel Mode
* 		0x3  - Quad Pixel Mode
*
* @note		None
*
****************************************************************************/
u32 XCsi2TxSs_GetPixelMode(XCsi2TxSs *InstancePtr) {
	return XCsi2Tx_GetPixelMode(InstancePtr->CsiPtr);
}

/****************************************************************************/
/**
*
* This function is used to get the number of lanes configured in
* the IP.
*
* @param	InstancePtr is a pointer to the CSI2TX SS Instance to be
*		worked on.
*
* @return	Max number of lanes available in u32 format
*
* @note		None
*
****************************************************************************/
u32 XCsi2TxSs_GetMaxLaneCount(XCsi2TxSs *InstancePtr) {
	return XCsi2Tx_GetMaxLaneCount(InstancePtr->CsiPtr);
}

/****************************************************************************/
/**
*
* This function is used to check if lanes are in ulps mode
*
* @param	InstancePtr is a pointer to the CSI2TX SS Instance to be
*		worked on.
*
* @return
*		- 1 - ulps mode enter
*		- 0 - ulps mode exit
*
* @note		None
*
****************************************************************************/
u32 XCsi2TxSs_IsUlps(XCsi2TxSs *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr);
	Xil_AssertNonvoid(InstancePtr->CsiPtr);

	return XCsi2Tx_IsUlps(InstancePtr->CsiPtr);
}

/****************************************************************************/
/**
*
* This function is used to set lanes in ulps mode
*
* @param	InstancePtr is a pointer to the CSI2TX SS Instance to be
*		worked on.
*
* @param	Value
*		- 1 - ulps mode enter
*		- 0 - ulps mode exit
*
* @return	None
*
* @note		None
*
****************************************************************************/
void XCsi2TxSs_SetUlps(XCsi2TxSs *InstancePtr, u32 Value)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(InstancePtr->CsiPtr);

	XCsi2Tx_SetUlps(InstancePtr->CsiPtr, Value);
}

/*****************************************************************************/
/**
* This function is used to set the CSI2 Tx Subsystem Clock Mode as either
* Continuous (0) or Non-Continuous (1) mode
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
*		worked on.
*
* @param	Mode for Continuous Mode (0) or Non-continuous Mode (1)
*
* @return	None
*
* @note		None
*
******************************************************************************/
void XCsi2TxSs_SetClkMode(XCsi2TxSs *InstancePtr, u8 Mode)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr);
	Xil_AssertVoid(InstancePtr->CsiPtr);

	XCsi2Tx_SetClkMode(InstancePtr->CsiPtr, Mode);
}

/*****************************************************************************/
/**
* This function is used to get the CSI2 Tx Subsystem Clock Mode as either
* Continuous (0) or Non-Continuous (1) mode
*
* @param	InstancePtr is a pointer to the Subsystem instance to be
*		worked on.
*
* @return	0 - Continuous Clock Mode
* 		1 - Non-continuous Clock Mode
*
* @note		None
*
******************************************************************************/
u32 XCsi2TxSs_GetClkMode(XCsi2TxSs *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr);
	Xil_AssertNonvoid(InstancePtr->CsiPtr);

	return (XCsi2Tx_GetClkMode(InstancePtr->CsiPtr));
}

/*****************************************************************************/
/**
 * This function sets the Line Count for virtual Channel if Frame End
 * Generation feature is enabled. This is to be called before starting
 * the core.
 *
 * @param	InstancePtr is a pointer to the Subsystem instance to be
 *		worked on.
 * @param	VC is which Virtual channel to be configured for (0-3).
 * @param	LineCount is valid line count for the Virtual channel.
 *
 * @return
 *		- XST_NO_FEATURE if Frame End generation is not enabled
 *		- XST_INVALID_PARAM if any param is invalid e.g.
 *			VC is always 0 to 3 and Line Count is 0 to 0xFFFF.
 *		- XST_FAILURE in case the core is already running.
 *		- XST_SUCCESS otherwise
 *
 * @note	None.
 *
******************************************************************************/
u32 XCsi2TxSs_SetLineCountForVC(XCsi2TxSs *InstancePtr, u8 VC, u16 LineCount)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Is Frame End generation feature enabled? */
	if (!InstancePtr->Config.FEGenEnabled)
		return XST_NO_FEATURE;

	return XCsi2Tx_SetLineCountForVC(InstancePtr->CsiPtr, VC, LineCount);
}

/*****************************************************************************/
/**
 * This function gets the Line Count for virtual Channel if Frame End
 * Generation feature is enabled.
 *
 * @param	InstancePtr is a pointer to the Subsystem instance to be
 *		worked on.
 * @param	VC is which Virtual channel to be configured for (0-3).
 * @param	LineCount is pointer to variable to be filled with line count
 *		for the Virtual channel
 *
 * @return
 *		- XST_NO_FEATURE if Frame End generation is not enabled
 *		- XST_INVALID_PARAM if any param is invalid e.g.
 *			VC is always 0 to 3 and Line Count is 0 to 0xFFFF.
 *		- XST_SUCCESS otherwise
 *
 * @note	None.
 *
******************************************************************************/
u32 XCsi2TxSs_GetLineCountForVC(XCsi2TxSs *InstancePtr, u8 VC, u16 *LineCount)
{
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Is Frame End generation feature enabled? */
	if (!InstancePtr->Config.FEGenEnabled)
		return XST_NO_FEATURE;

	return XCsi2Tx_GetLineCountForVC(InstancePtr->CsiPtr, VC, LineCount);
}
/** @} */
