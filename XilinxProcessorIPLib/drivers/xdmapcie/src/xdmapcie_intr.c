/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xdmapcie_intr.c
* @addtogroup xdmapcie Overview
* @{
*
* @brief Implements the interrupt handling functions for the XDmaPcie IP.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	01/30/2019	First release
* </pre>
*
******************************************************************************/

/****************************** Include Files ********************************/
#include "xdmapcie.h"
#if defined(versal_2vp)
#include "xdmapcie_common.h"
#endif

/*************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

/*************************** Variable Definitions ****************************/

/*************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
* @brief Enables the global interrupt.
*
* @param[in] InstancePtr Pointer to the XDmaPcie instance to operate on.
*
* @note This bit resides in the Bridge Status and Control Register.
*
******************************************************************************/
void XDmaPcie_EnableGlobalInterrupt(XDmaPcie *InstancePtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XDMAPCIE_BSC_OFFSET);

	Data |= (XDMAPCIE_BSC_GI_MASK >> XDMAPCIE_BSC_GI_SHIFT);
	XDmaPcie_WriteReg(InstancePtr->Config.BaseAddress,
						XDMAPCIE_BSC_OFFSET, Data);

}

/*****************************************************************************/
/**
* @brief Disables the global interrupt.
*
* @param[in] InstancePtr Pointer to the XDmaPcie instance to operate on.
*
* @note This bit resides in the Bridge Status and Control Register.
*
******************************************************************************/
void XDmaPcie_DisableGlobalInterrupt(XDmaPcie *InstancePtr)
{
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XDMAPCIE_BSC_OFFSET);

	Data &= ~(XDMAPCIE_BSC_GI_MASK >> XDMAPCIE_BSC_GI_SHIFT);

	XDmaPcie_WriteReg(InstancePtr->Config.BaseAddress,
						XDMAPCIE_BSC_OFFSET, Data);

}

/*****************************************************************************/
/**
* @brief Enables the IP interrupt bits passed in EnableMask.
*
* @param[in] InstancePtr Pointer to the XDmaPcie instance to operate on.
* @param[in] EnableMask Bit pattern of the interrupts to be enabled.
*
* @note An interrupt that is already enabled before this function is called
*       stays enabled regardless of the value of EnableMask.
*
******************************************************************************/
void XDmaPcie_EnableInterrupts(XDmaPcie *InstancePtr, u32 EnableMask)
{
#if defined(versal_2vp)
	UINTPTR CoreBase;
#else
	u32 Data = 0;
#endif

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

#if defined(versal_2vp)
	(void)EnableMask;
	CoreBase = XDmaPcie_Cpm6CoreBase(InstancePtr->Config.Name,
					 InstancePtr->Config.BaseAddress);

	/* Clear the legacy INTx and misc event masks to enable them. */
	XDmaPcie_WriteReg(CoreBase, XDMAPCIE_CPM6_LEGACY_INTX_OFFSET,
			  XDMAPCIE_CPM6_INTR_ENABLE_ALL_MASK);
	XDmaPcie_WriteReg(CoreBase, XDMAPCIE_CPM6_MISC_EVENT_OFFSET,
			  XDMAPCIE_CPM6_INTR_ENABLE_ALL_MASK);
#else
	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
						XDMAPCIE_IM_OFFSET);

	XDmaPcie_WriteReg(InstancePtr->Config.BaseAddress,
				XDMAPCIE_IM_OFFSET, (Data | EnableMask));
#endif

}

/*****************************************************************************/
/**
* @brief Disables the IP interrupt bits passed in DisableMask.
*
* @param[in] InstancePtr Pointer to the XDmaPcie instance to operate on.
* @param[in] DisableMask Bit pattern of the interrupts to be disabled.
*
* @note An interrupt that is already disabled before this function is called
*       stays disabled regardless of the value of DisableMask.
*
******************************************************************************/
void XDmaPcie_DisableInterrupts(XDmaPcie *InstancePtr, u32 DisableMask)
{
#if defined(versal_2vp)
	UINTPTR CoreBase;
#else
	u32 Data = 0;
#endif

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

#if defined(versal_2vp)
	(void)DisableMask;
	CoreBase = XDmaPcie_Cpm6CoreBase(InstancePtr->Config.Name,
					 InstancePtr->Config.BaseAddress);

	/* Mask the legacy INTA/B/C/D interrupts. */
	XDmaPcie_WriteReg(CoreBase, XDMAPCIE_CPM6_LEGACY_INTX_OFFSET,
			  XDMAPCIE_CPM6_LEGACY_INTX_DISABLE_MASK);

	/* Mask the misc / INTx aggregate event. */
	XDmaPcie_WriteReg(CoreBase, XDMAPCIE_CPM6_MISC_EVENT_OFFSET,
			  XDMAPCIE_CPM6_MISC_EVENT_DISABLE_MASK);
#else
	Data = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XDMAPCIE_IM_OFFSET);

	XDmaPcie_WriteReg(InstancePtr->Config.BaseAddress,
				XDMAPCIE_IM_OFFSET, (Data & (~DisableMask)));
#endif

}

/*****************************************************************************/
/**
* @brief Gets the currently enabled interrupt bits of the IP.
*
* @param[in]  InstancePtr Pointer to the XDmaPcie instance to operate on.
* @param[out] EnabledMaskPtr Pointer to a variable that receives the enabled
*             interrupt bits read from the IP.
*
******************************************************************************/
void XDmaPcie_GetEnabledInterrupts(XDmaPcie *InstancePtr, u32 *EnabledMaskPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(EnabledMaskPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

#if defined(versal_2vp)
	*EnabledMaskPtr = XDmaPcie_ReadReg(
		XDmaPcie_Cpm6CoreBase(InstancePtr->Config.Name,
				      InstancePtr->Config.BaseAddress),
		XDMAPCIE_CPM6_MISC_EVENT_OFFSET);
#else
	*EnabledMaskPtr = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XDMAPCIE_IM_OFFSET);
#endif

}

/*****************************************************************************/
/**
* @brief Gets the currently pending interrupt bits of the IP.
*
* @param[in]  InstancePtr Pointer to the XDmaPcie instance to operate on.
* @param[out] PendingMaskPtr Pointer to a variable that receives the pending
*             interrupt bits read from the IP.
*
******************************************************************************/
void XDmaPcie_GetPendingInterrupts(XDmaPcie *InstancePtr, u32 *PendingMaskPtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(PendingMaskPtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

#if defined(versal_2vp)
	*PendingMaskPtr = XDmaPcie_ReadReg(
		XDmaPcie_Cpm6CoreBase(InstancePtr->Config.Name,
				      InstancePtr->Config.BaseAddress),
		XDMAPCIE_CPM6_LEGACY_INTX_OFFSET);
#else
	*PendingMaskPtr = XDmaPcie_ReadReg(InstancePtr->Config.BaseAddress,
							XDMAPCIE_ID_OFFSET);
#endif

}

/*****************************************************************************/
/**
* @brief Clears the pending interrupt bits of the IP passed in ClearMask.
*
* @param[in] InstancePtr Pointer to the XDmaPcie instance to operate on.
* @param[in] ClearMask Bit pattern of the pending interrupts to be cleared.
*
******************************************************************************/
void XDmaPcie_ClearPendingInterrupts(XDmaPcie *InstancePtr, u32 ClearMask)
{
#if defined(versal_2vp)
	UINTPTR CoreBase;
#endif

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

#if defined(versal_2vp)
	CoreBase = XDmaPcie_Cpm6CoreBase(InstancePtr->Config.Name,
					 InstancePtr->Config.BaseAddress);

	XDmaPcie_WriteReg(CoreBase, XDMAPCIE_CPM6_LEGACY_INTX_OFFSET,
			XDmaPcie_ReadReg(CoreBase,
				XDMAPCIE_CPM6_LEGACY_INTX_OFFSET) & (ClearMask));
#else

	XDmaPcie_WriteReg((InstancePtr->Config.BaseAddress), XDMAPCIE_ID_OFFSET,
			XDmaPcie_ReadReg((InstancePtr->Config.BaseAddress),
				XDMAPCIE_ID_OFFSET) & (ClearMask));
#endif
}
/** @} */
