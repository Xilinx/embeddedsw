/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdsitxss.c
* @addtogroup dsitxss_v2_0
* @{
*
* This is main code of Xilinx MIPI DSI Tx Subsystem device driver.
* Please see xdsitxss.h for more details of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- ------- -------------------------------------------------------
* 1.0 ram 11/02/16 Initial Release for MIPI DSI TX subsystem
* 1.1 sss 08/17/16 Added 64 bit support
*     sss 08/26/16 Add "Command Queue Vacancy" API
*                  API for getting pixel format
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xdebug.h"
#include "xdsi.h"
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
#include "xdphy.h"
#endif
#include "xdsitxss.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/**
 * This typedef declares the driver instances of all the cores in the subsystem
 */
typedef struct {
	XDsi DsiInst;
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy DphyInst;
#endif
} XDsiTxSs_SubCores;

/**************************** Variable Definitions ***********************************/

XDsiTxSs_SubCores DsiTxSsSubCores[XPAR_XDSITXSS_NUM_INSTANCES]; /**< Define Driver instance of all sub-core
					included in the design */

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

static void XDsiTxSs_GetIncludedSubCores(XDsiTxSs *DsiTxSsPtr);
static s32 XDsiTxSs_SubCoreInitDsi(XDsiTxSs *DsiTxSsPtr);
static s32 XDsiTxSs_SubCoreInitDphy(XDsiTxSs *DsiTxSsPtr);
static s32 ComputeSubCoreAbsAddr(UINTPTR SsBaseAddr, UINTPTR SsHighAddr,
					u32 Offset, UINTPTR *BaseAddr);

/************************** Function Definitions ******************************/

/*****************************************************************************/
/**
* This function initializes the MIPI DSI TX subsystem and included sub-cores.
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
s32 XDsiTxSs_CfgInitialize(XDsiTxSs *InstancePtr, XDsiTxSs_Config *CfgPtr,
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
	XDsiTxSs_GetIncludedSubCores(InstancePtr);

	if (InstancePtr->DsiPtr) {
		Status = XDsiTxSs_SubCoreInitDsi(InstancePtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->DphyPtr != NULL) {
		Status = XDsiTxSs_SubCoreInitDphy(InstancePtr);
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
* This function is used to configure the DSI default parameters that are to
* be handled by the application. It will configure protocol register with
* video mode, bllp mode,eotp
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
u32 XDsiTxSs_DefaultConfigure(XDsiTxSs *InstancePtr)
{
	u32 Status;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Status = XDsi_DefaultConfigure(InstancePtr->DsiPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function is used to activate the DSI Subsystem. Internally it activates
* the DPHY and DSI. Enable/Disable IP core to start processing
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
int XDsiTxSs_Activate(XDsiTxSs *InstancePtr, XDsiSS_Subcore core, u8 Flag)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Flag <= XDSITXSS_ENABLE);
	Xil_AssertVoid(InstancePtr->DsiPtr != NULL);
	Xil_AssertVoid(InstancePtr->DphyPtr != NULL);

	if (core == XDSITXSS_DSI)
		XDsi_Activate(InstancePtr->DsiPtr, Flag);
	else if (core == XDSITXSS_PHY)
		XDphy_Activate(InstancePtr->DphyPtr, Flag);
	else
		return XST_INVALID_PARAM;

	return XST_SUCCESS;
}
/*****************************************************************************/
/**
* This function is used to reset the DSI Subsystem. Internally it resets
* the DPHY and DSI
*
* @param	InstancePtr is a pointer to the Subsystem instance to be worked on.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void XDsiTxSs_Reset(XDsiTxSs *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->DsiPtr);

	XDsi_Reset(InstancePtr->DsiPtr);
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	if (InstancePtr->Config.IsDphyRegIntfcPresent && InstancePtr->DphyPtr) {
		XDphy_Reset(InstancePtr->DphyPtr);
	}
#endif
}

/*****************************************************************************/
/**
* This function reports list of cores included in DSI TX Subsystem
*
* @param	InstancePtr is a pointer to the DSI TX Subsystem instance.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
void XDsiTxSs_ReportCoreInfo(XDsiTxSs *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->DsiPtr);

	xdbg_printf(XDBG_DEBUG_GENERAL, "\n\r  ->MIPI DSI Subsystem Cores\n\r");

	/* Report all the included cores in the subsystem instance */
	if (InstancePtr->DsiPtr) {
		xdbg_printf(XDBG_DEBUG_GENERAL, "  : DSI Tx Controller \n\r");
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
 * * This function sets the mode to send short packet.
 * *
 * * @param        InstancePtr is the XDsiTxSs instance to operate on
 * * @param        mode is the DSI mode (video or command) to operate on
 * *
 * * @return       None
 * *
 * * @note         None.
 * *
 * ****************************************************************************/
int XDsiTxSs_SetDSIMode(XDsiTxSs *InstancePtr, XDsi_DsiModeType mode)
{
	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);
	return XDsi_SetMode(InstancePtr->DsiPtr, mode);
}

/*****************************************************************************/
/**
 * * This function will send the short packet to controller in command mode
 * * Generic Short Packet Register and fill up the structure passed from caller.
 * *
 * * @param        InstancePtr is the XDsiTxSs instance to operate on
 * *
 * * @return
 *		   - XST_SUCCESS on successful packet transmission
 *		   - XST_FAILURE on failure in packet transmission
 * *
 * * @note         None.
 * *
 * ****************************************************************************/
int XDsiTxSs_SendCmdModePacket(XDsiTxSs *InstancePtr)
{
	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);
	return XDsi_SendCmdModePkt(InstancePtr->DsiPtr, &InstancePtr->CmdPkt);
}

/*****************************************************************************/
/**
* This function will send the short packet to controller
* Generic Short Packet Register and fill up the structure passed from caller.
* like to turn on/off peripheral, change color mode
*
* @param	InstancePtr is the XDsiTxSs instance to operate on
*
* @return	None
*
* @note		None.
*
****************************************************************************/
void XDsiTxSs_SendShortPacket(XDsiTxSs *InstancePtr)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);

	XDsi_SendShortPacket(InstancePtr->DsiPtr, &InstancePtr->SpktData);
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
void XDsiTxSs_GetConfigParams(XDsiTxSs *InstancePtr)
{
	/* Verify argument */
	Xil_AssertVoid(InstancePtr != NULL);

	XDsi_GetConfigParams(InstancePtr->DsiPtr, &InstancePtr->ConfigInfo);
}

/*****************************************************************************/
/**
* This function will get the information from the GUI settings
*
* @param	InstancePtr is the XDsiTxss instance to operate on
*
* @return 	Controller ready status
*
* @note		None.
*
****************************************************************************/
u32 XDsiTxSs_IsControllerReady(XDsiTxSs *InstancePtr)
{
	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDsi_IsControllerReady(InstancePtr->DsiPtr);
}

/****************************************************************************/
/**
*
* This function is used to get pixel format
*
* @param	InstancePtr is a pointer to the DsiTxSs Instance to be
*		worked on.
*
* @return	0x0E – Packed RGB565
*		0x1E- packed RGB666
*		0x2E – Loosely packed RGB666
*		0x3E- Packed RGB888
*		0x0B- Compressed Pixel Stream
*
* @note		None
*
****************************************************************************/
u32 XDsiTxSs_GetPixelFormat(XDsiTxSs *InstancePtr)
{
	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDsi_GetPixelFormat(InstancePtr->DsiPtr);
}

/****************************************************************************/
/**
*
* This function is used to get Command queue Vacancy
*
* @param	InstancePtr is a pointer to the DSITxSs Instance to be
*		worked on.
*
* @return	Number of command queue entries can be safely written
* 		to Command queue FIFO, before it goes full.
*
* @note		None
*
****************************************************************************/
u32 XDsiTxSs_GetCmdQVacancy(XDsiTxSs *InstancePtr)
{
	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	return XDsi_GetCmdQVacancy(InstancePtr->DsiPtr);
}

/*****************************************************************************/
/**
* This function Set Timing mode and Resolution. As per user resolution
* selection it will get populate Periperal Timing Parameters from video common
* Library
*
* @param	InstancePtr is the XDsi instance to operate on
* @param 	VideoMode Specifies mode of Interfacing
* @param	Resolution sets the resolution
* @param	BurstPacketSize sets the packet size
*
* @return
*		- XST_SUCCESS is return Video interfacing set was successful
*		- XST_INVALID_PARAM indicates an invalid parameter was
*		specified.
*
* @note		None.
*
****************************************************************************/
s32 XDsiTxSs_SetVideoInterfaceTiming(XDsiTxSs *InstancePtr,
					XDsi_VideoMode VideoMode,
					XVidC_VideoMode Resolution,
					u16 BurstPacketSize)
{
	u32 Status;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	Status = XDsi_SetVideoInterfaceTiming(InstancePtr->DsiPtr, VideoMode,
						Resolution, BurstPacketSize);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Set VideoInterface failed\r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
* This function Set Timning mode and Resolution as per user inputs
*
* @param	InstancePtr is the XDsi instance to operate on
* @param 	VideoMode Specifies mode of Interfacing
* @param	Timing Video Timing parameters
*
* @return
*		- XST_SUCCESS is return Video interfacing was successfully set
*		- XST_INVALID_PARAM indicates an invalid parameter was
*		specified.
*
* @note		None
*
****************************************************************************/
s32 XDsiTxSs_SetCustomVideoInterfaceTiming(XDsiTxSs *InstancePtr,
		XDsi_VideoMode VideoMode, XDsi_VideoTiming  *Timing)
{
	u32 Status;

	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Timing != NULL);

	Status = XDsi_SetCustomVideoInterfaceTiming(InstancePtr->DsiPtr,
							VideoMode, Timing);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "Set Custom VideoInterface failed\r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
* This function queries the subsystem instance configuration to determine
* the included sub-cores. For each sub-core that is present in the design
* the sub-core driver instance is binded with the subsystem sub-core driver
* handle
*
* @param	DsiTxSsPtr is a pointer to the Subsystem instance
*
* @return	None
*
* @note		None
*
******************************************************************************/
static void XDsiTxSs_GetIncludedSubCores(XDsiTxSs *DsiTxSsPtr)
{
	/* Verify argument */
	Xil_AssertVoid(DsiTxSsPtr != NULL);

	DsiTxSsPtr->DsiPtr = ((DsiTxSsPtr->Config.DsiInfo.IsPresent) ?
				(&DsiTxSsSubCores[DsiTxSsPtr->Config.DeviceId].DsiInst) : NULL);
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	DsiTxSsPtr->DphyPtr = ((DsiTxSsPtr->Config.DphyInfo.IsPresent) ?
				(&DsiTxSsSubCores[DsiTxSsPtr->Config.DeviceId].DphyInst) : NULL);
#endif
}

/*****************************************************************************/
/**
* This function initializes the DSI sub-core initialization
*
* @param	DsiTxSsPtr is a pointer to the Subsystem instance
*
* @return
		- XST_SUCCESS on DSI sub core initialization
		- XST_FAILURE on DSI fail initialization
*
* @note		None
*
******************************************************************************/
static s32 XDsiTxSs_SubCoreInitDsi(XDsiTxSs *DsiTxSsPtr)
{
	s32 Status;
	UINTPTR AbsAddr;
	XDsi_Config *ConfigPtr;

	if (!DsiTxSsPtr->DsiPtr) {
		return XST_FAILURE;
	}

	/* Get core configuration */
	xdbg_printf(XDBG_DEBUG_INFO, ">Initializing DSI Tx Controller...\n\r");
	ConfigPtr = XDsi_LookupConfig(DsiTxSsPtr->Config.DsiInfo.DeviceId);
	if (ConfigPtr == NULL) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSITXSS ERR:: DSI not found\n\r");
		return XST_FAILURE;
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = ComputeSubCoreAbsAddr(DsiTxSsPtr->Config.BaseAddr,
					DsiTxSsPtr->Config.HighAddr,
					DsiTxSsPtr->Config.DsiInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSITXSS ERR:: DSI core base"
			"address (0x%x) invalid \n\r", AbsAddr);
		return XST_FAILURE;
	}

	/* Initialize core */
	Status = XDsi_CfgInitialize(DsiTxSsPtr->DsiPtr, ConfigPtr, AbsAddr);

	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSITXSS ERR:: DSI core"
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
* @param	DsiTxSsPtr is a pointer to the Subsystem instance
*
* @return
		- XST_SUCCESS on successful initialization of Dphy
		- XST_FAILURE on Dphy initialization failure
*
* @note		None
*
******************************************************************************/
static s32 XDsiTxSs_SubCoreInitDphy(XDsiTxSs *DsiTxSsPtr)
{
	s32 Status;
	UINTPTR AbsAddr;
	XDphy_Config *ConfigPtr;

	if (!DsiTxSsPtr->DphyPtr) {
		return XST_FAILURE;
	}

	/* Get core configuration */
	xdbg_printf(XDBG_DEBUG_INFO, "->Initializing DPHY ...\n\r");
	ConfigPtr = XDphy_LookupConfig(DsiTxSsPtr->Config.DphyInfo.DeviceId);
	if (!ConfigPtr) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSITXSS ERR:: DPHY not found \n\r");
		return (XST_FAILURE);
	}

	/* Compute absolute base address */
	AbsAddr = 0;
	Status = ComputeSubCoreAbsAddr(DsiTxSsPtr->Config.BaseAddr,
					DsiTxSsPtr->Config.HighAddr,
					DsiTxSsPtr->Config.DphyInfo.AddrOffset,
					&AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSITXSS ERR:: DPHY core base address "
				"(0x%x) invalid \n\r", AbsAddr);
		return XST_FAILURE;
	}

	/* Initialize core */
	Status = XDphy_CfgInitialize(DsiTxSsPtr->DphyPtr, ConfigPtr, AbsAddr);
	if (Status != XST_SUCCESS) {
		xdbg_printf(XDBG_DEBUG_ERROR, "DSITXSS ERR:: Dphy core Initialization "
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
