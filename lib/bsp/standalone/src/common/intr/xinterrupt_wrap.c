/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xinterrupt_wrap.c
*
* The xinterrupt_wrap.c file contains interrupt related functions and macros.
* Contains wrapper functions for the scugic/axi intc Interrupt controller
* drivers.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date   Changes
* ----- ---- -------- -------------------------------------------------------
* 7.2   mus  22/11/21 First release of xil interrupt support
* 9.0   adk  17/04/23 Added support for system device-tree flow
* 9.0   adk  27/04/23 Use XScuGic_LookupConfigBaseAddr() API for xsct flow
* 9.1   mus  16/04/24 Add support for software generated interrupts.
* 9.2   ml   05/08/24 Add Support for connecting fast interrupt for intc.
* 9.2   adk  11/09/24 Update XGetPriorityTriggerType() with IntrId to IntrNum
* 		      transformation.
* 9.2   ml   19/09/24 Fix compilation warnings by typecasting and adding
*                     conditional compilation checks.
* 9.4   ml   24/07/24 Fixed GCC warnings
* 9.5   vmt  12/12/25 Fixed XConfigInterruptCntrl() to check IsReady, preventing
* 		      re-initialization that clears interrupt enables.
* 9.5   ml   26/02/26 Fixed PPI interrupt ID encoding.
* 9.5   ml   13/03/26 Fixed XSetPriorityTriggerType trigger mapping.
* 9.5   ml   31/03/26 Bridge INTC through GIC in XSetupInterruptSystem for
*                     cascaded INTC-to-GIC topologies. Fixed XDisableIntrId
*                     to decode IntrId before passing to XIntc_Disable.
* 9.5   ml   20/08/26 Add per-root XIntcInitRoots[] and XIntcGicConnectedRoots[]
*                     for SDT multi-root INTC cascade. XIntcGetInstance() walks
*                     IntrParent chain to locate the correct root instance.
* </pre>
*
******************************************************************************/

#include "xinterrupt_wrap.h"

#ifdef XIL_INTERRUPT

#if defined (XPAR_SCUGIC) /* available in xscugic.h */
static XScuGic XScuGicInstance;
static int ScuGicInitialized;
#endif

#if defined (XPAR_AXI_INTC) /* available in xintc.h */
/* One XIntc instance per chain root; multi-root designs need separate instances. */
#ifdef SDT
static XIntc XIntcInstances[XPAR_INTC_NUM_DRV_INSTANCES];
#else
static XIntc XIntcInstances[XPAR_XINTC_NUM_INSTANCES];
#endif
#if defined (SDT)
/* Base address of each initialized chain root; index matches XIntcInstances[]. */
static UINTPTR XIntcInitRoots[XPAR_INTC_NUM_DRV_INSTANCES];
static u32     XIntcInitCount;
#if defined (XPAR_SCUGIC)
/* Base addresses of roots whose GIC SPI has been connected, in connection order. */
static UINTPTR XIntcGicConnectedRoots[XPAR_INTC_NUM_DRV_INSTANCES];
static u32     XIntcGicConnectedCount;
#endif
#endif

/*
 * Returns the XIntc instance for the chain root of IntcParent.
 * In SDT mode walks the IntrParent chain and returns NULL if not yet initialized.
 * In non-SDT mode always returns the single global instance (slot 0).
 */
static XIntc *XIntcGetInstance(UINTPTR IntcParent)
{
#ifdef SDT
	XIntc_Config *RootCfg = XIntc_GetChainRoot(IntcParent);
	UINTPTR RootBase = (RootCfg != NULL) ? RootCfg->BaseAddress
					     : XGet_BaseAddr(IntcParent);
	u32 i;
	for (i = 0; i < XIntcInitCount; i++) {
		if (XIntcInitRoots[i] == RootBase)
			return &XIntcInstances[i];
	}
	return NULL;
#else
	/* Non-SDT: single root, always slot 0 */
	(void)IntcParent;
	return &XIntcInstances[0];
#endif
}
#endif

/*****************************************************************************/
/**
*
* @brief    Initializes the interrupt controller.
*
* @param    IntcParent: Interrupt controller baseaddress and type.
*
* @return   XST_SUCCESS if initialization was successful
* 	    XST_FAILURE in case of failure
*
* @note     None.
*
******************************************************************************/
int XConfigInterruptCntrl(UINTPTR IntcParent)
{
#if defined (XPAR_AXI_INTC) || defined (XPAR_SCUGIC)
	int Status = XST_FAILURE;
	UINTPTR BaseAddr = XGet_BaseAddr(IntcParent);
#endif
#if defined (XPAR_AXI_INTC)
#ifdef SDT
	XIntc_Config *RootCfg;
	UINTPTR InitBaseAddr;
#endif
#endif

	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		XScuGic_Config *CfgPtr = NULL;
		if (XScuGicInstance.IsReady != XIL_COMPONENT_IS_READY) {
#if defined(SDT)
			CfgPtr = XScuGic_LookupConfig(BaseAddr);
#else
			CfgPtr = XScuGic_LookupConfigBaseAddr(BaseAddr);
#endif
			if (CfgPtr == NULL) {
				return XST_FAILURE;
			}
			if (!ScuGicInitialized) {
				Status = XScuGic_CfgInitialize(&XScuGicInstance, CfgPtr, CfgPtr->DistBaseAddress);
			} else {
				Status = XST_SUCCESS;
			}
		} else {
			Status = XST_SUCCESS;
		}
		return Status;
#else
		return XST_FAILURE;
#endif
	} else {
#if defined (XPAR_AXI_INTC)
#ifdef SDT
		/* Walk IntrParent chain to find chain root; each root initializes once. */
		RootCfg = XIntc_GetChainRoot(IntcParent);
		InitBaseAddr = (RootCfg != NULL) ? RootCfg->BaseAddress : BaseAddr;
		{
			u32 i, found = 0;

			for (i = 0; i < XIntcInitCount; i++) {
				if (XIntcInitRoots[i] == InitBaseAddr) {
					found = 1;
					break;
				}
			}
			if (!found) {
				XIntc *InstPtr;

				if (XIntcInitCount >= XPAR_INTC_NUM_DRV_INSTANCES)
					return XST_FAILURE;
				InstPtr = &XIntcInstances[XIntcInitCount];
				InstPtr->IsStarted = 0;
				Status = XIntc_Initialize(InstPtr, InitBaseAddr);
				if (Status == XST_SUCCESS)
					XIntcInitRoots[XIntcInitCount++] = InitBaseAddr;
			} else {
				Status = XST_SUCCESS;
			}
		}
#else
		if (XIntcInstances[0].IsReady != XIL_COMPONENT_IS_READY) {
			XIntc_Config *PrimaryCfg = XIntc_GetPrimaryController();
			UINTPTR InitBaseAddr;

			if (PrimaryCfg != NULL &&
			    PrimaryCfg->IntcType != XIN_INTC_NOCASCADE)
				InitBaseAddr = PrimaryCfg->BaseAddress;
			else
				InitBaseAddr = BaseAddr;
			Status = XIntc_Initialize(&XIntcInstances[0], InitBaseAddr);
		} else {
			Status = XST_SUCCESS;
		}
#endif
		return Status;
#else
		return XST_FAILURE;
#endif
	}
}

/*****************************************************************************/
/**
*
* @brief    connects to the interrupt controller.
*
* @param    IntrId: Interrupt Id.
* @param    IntrHandler: Interrupt handler.
* @param    CallBackRef: Callback reference for handler.
* @param    IntcParent: Interrupt controller baseaddress and type.
*
* @return   XST_SUCCESS if initialization was successful
* 	    XST_FAILURE in case of failure
*
* @note     None.
*
******************************************************************************/
int XConnectToInterruptCntrl(u32 IntrId, void *IntrHandler, void *CallBackRef, UINTPTR IntcParent)
{
	int Status = XST_FAILURE;
#if !defined (XPAR_AXI_INTC) && !defined (XPAR_SCUGIC)
	(void)IntrId;
	(void)IntrHandler;
	(void)CallBackRef;
#endif
#if defined (XPAR_SCUGIC)
	int Doconnect = FALSE;
#endif

	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		if (ScuGicInitialized) {
			Doconnect = 1;
		}
		if (Doconnect) {
			u16 IntrNum = XGet_IntrId(IntrId);
			u16 Offset = XGet_IntrOffset(IntrId);

			IntrNum += Offset;
			Status = XScuGic_Connect(&XScuGicInstance, IntrNum,  \
						 (Xil_ExceptionHandler) IntrHandler, CallBackRef);
			return Status;
		} else {
			return XST_SUCCESS;
		}
#else
		return Status;
#endif
	} else {
#if defined (XPAR_AXI_INTC)
		u16 IntrNum = XGet_IntrId(IntrId);
#ifdef SDT
		/* SDT: register into the target controller's HandlerTable by base address */
		{
			UINTPTR LeafBase = XGet_BaseAddr(IntcParent);

			if (XIntc_LookupConfig(LeafBase) == NULL)
				return XST_FAILURE;
			XIntc_RegisterHandler(LeafBase, IntrNum,
					      (XInterruptHandler)IntrHandler, CallBackRef);
			Status = XST_SUCCESS;
		}
#else
		XIntc *IntcInstPtr = XIntcGetInstance(IntcParent);
		if (IntcInstPtr == NULL)
			return XST_FAILURE;
		Status = XIntc_Connect(IntcInstPtr, IntrNum, \
				       (XInterruptHandler)IntrHandler, CallBackRef);
#endif
		return Status;
#else
		return Status;
#endif

	}
}

/*****************************************************************************/
/**
*
* @brief    connects to the Fast interrupt controller.
*
* @param    IntrId: Interrupt Id.
* @param    IntrHandler: Interrupt handler.
* @param    IntcParent: Interrupt controller baseaddress and type.
*
* @return   XST_SUCCESS if initialization was successful
* 	    XST_FAILURE in case of failure
*
* @note     None.
*
******************************************************************************/
#if defined (__MICROBLAZE__) || defined(__riscv)
int XConnectToFastInterruptCntrl(u32 IntrId, void *IntrHandler, UINTPTR IntcParent)
{
	int Status = XST_FAILURE;
#if !defined (XPAR_AXI_INTC)
	(void)IntrId;
	(void)IntrHandler;
#endif

	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_INTC) {
#if defined (XPAR_AXI_INTC)
		u16 IntrNum = XGet_IntrId(IntrId);
#ifdef SDT
		/* SDT: construct temporary leaf instance to target its IVAR directly. */
		{
			UINTPTR LeafBase = XGet_BaseAddr(IntcParent);
			XIntc_Config *LeafCfg;
			XIntc LeafInst;

			LeafCfg = XIntc_LookupConfig(LeafBase);
			if (LeafCfg == NULL)
				return XST_FAILURE;
			LeafInst.BaseAddress = LeafBase;
			LeafInst.CfgPtr = LeafCfg;
			LeafInst.IsReady = XIL_COMPONENT_IS_READY;
			Status = XIntc_ConnectFastHandler(&LeafInst, IntrNum, \
							  (XFastInterruptHandler)IntrHandler);
		}
#else
		/* Non-SDT: XIntc_LookupConfig takes DeviceId, not base address. */
		{
			XIntc *InstPtr = XIntcGetInstance(IntcParent);

			if (InstPtr == NULL)
				return XST_FAILURE;
			Status = XIntc_ConnectFastHandler(InstPtr, IntrNum, \
							  (XFastInterruptHandler)IntrHandler);
		}
#endif
		return Status;
#else
		return XST_FAILURE;
#endif
	}
	return Status;
}
#endif
/*****************************************************************************/
/**
*
* @brief    disconnects the interrupt controller.
*
* @param    IntrId: Interrupt Id.
* @param    IntcParent: Interrupt controller baseaddress and type.
*
* @return   XST_SUCCESS if initialization was successful
* 	    XST_FAILURE in case of failure
*
* @note     None.
*
******************************************************************************/
int XDisconnectInterruptCntrl(u32 IntrId, UINTPTR IntcParent)
{
#if !defined (XPAR_SCUGIC) && !defined (XPAR_AXI_INTC)
	(void)IntrId;
#endif
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		u16 IntrNum = XGet_IntrId(IntrId);
		u16 Offset = XGet_IntrOffset(IntrId);

		IntrNum += Offset;
		XScuGic_Disconnect(&XScuGicInstance, IntrNum);
#else
		return XST_FAILURE;
#endif
	} else {
#if defined (XPAR_AXI_INTC)
		u16 IntrNum = XGet_IntrId(IntrId);
#if defined (SDT)
		/* SDT: target leaf controller — handlers were registered by leaf base. */
		UINTPTR LeafBase = XGet_BaseAddr(IntcParent);
		u32 Ier = XIntc_In32(LeafBase + XIN_IER_OFFSET);

		XIntc_Out32(LeafBase + XIN_IER_OFFSET, Ier & ~(1U << IntrNum));
		XIntc_RegisterHandler(LeafBase, IntrNum, XNullHandler, NULL);
#else
		XIntc *DisInstPtr = XIntcGetInstance(IntcParent);
		if (DisInstPtr == NULL)
			return XST_FAILURE;
		XIntc_Disconnect(DisInstPtr, IntrNum);
#endif
#else
		return XST_FAILURE;
#endif
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief    Starts the interrupt controller.
*
* @param    Mode: Interrupt controller mode type.
* @param    IntcParent: Interrupt controller baseaddress and type.
*
* @return   XST_SUCCESS if initialization was successful
* 	    XST_FAILURE in case of failure
*
* @note     None.
*
******************************************************************************/
int XStartInterruptCntrl(u32 Mode, UINTPTR IntcParent)
{
#if defined (XPAR_AXI_INTC)
	int Status = XST_FAILURE;
#else
	(void) Mode;
#endif
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
		/*
		 * For XPAR_SCUGIC, XConfigInterruptCntrl starts controller
		 * hence returning without doing anything
		 */
		return 0;
	} else  {
#if defined (XPAR_AXI_INTC)
		XIntc *InstPtr = XIntcGetInstance(IntcParent);

		if (InstPtr == NULL)
			return XST_FAILURE;
		if (InstPtr->IsStarted != XIL_COMPONENT_IS_STARTED)
			Status = XIntc_Start(InstPtr, Mode);
		else
			Status = XST_SUCCESS;
		return Status;
#else
		return XST_FAILURE;
#endif

	}

}

/*****************************************************************************/
/**
*
* @brief    Enable the interrupt id.
*
* @param    IntrId: Interrupt Id.
* @param    IntcParent: Interrupt controller baseaddress and type.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XEnableIntrId( u32 IntrId, UINTPTR IntcParent)
{
#if !defined (XPAR_SCUGIC) && !defined (XPAR_AXI_INTC)
        (void)IntrId;
#endif

	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		u16 IntrNum = XGet_IntrId(IntrId);
		u16 Offset = XGet_IntrOffset(IntrId);
		IntrNum += Offset;
		XScuGic_Enable(&XScuGicInstance, IntrNum);
#endif

	} else {
#if defined (XPAR_AXI_INTC)
		u16 IntrNum = XGet_IntrId(IntrId);
#ifdef SDT
		UINTPTR BaseAddr = XGet_BaseAddr(IntcParent);
		u32 Ier = XIntc_In32(BaseAddr + XIN_IER_OFFSET);
		XIntc_Out32(BaseAddr + XIN_IER_OFFSET, Ier | (1U << IntrNum));
#else
		XIntc *InstPtr = XIntcGetInstance(IntcParent);
		if (InstPtr != NULL)
			XIntc_Enable(InstPtr, IntrNum);
#endif
#endif
	}

}

/*****************************************************************************/
/**
*
* @brief    disable the interrupt id.
*
* @param    IntrId: Interrupt Id.
* @param    IntcParent: Interrupt controller baseaddress and type.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XDisableIntrId( u32 IntrId, UINTPTR IntcParent)
{
#if !defined (XPAR_SCUGIC) && !defined (XPAR_AXI_INTC)
        (void)IntrId;
#endif

	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		u16 IntrNum = XGet_IntrId(IntrId);
		u16 Offset = XGet_IntrOffset(IntrId);

		IntrNum += Offset;
		XScuGic_Disable(&XScuGicInstance, IntrNum);
#endif
	} else {
#if defined (XPAR_AXI_INTC)
		u16 IntrNum = XGet_IntrId(IntrId);
#ifdef SDT
		UINTPTR BaseAddr = XGet_BaseAddr(IntcParent);
		u32 Ier = XIntc_In32(BaseAddr + XIN_IER_OFFSET);
		XIntc_Out32(BaseAddr + XIN_IER_OFFSET, Ier & ~(1U << IntrNum));
#else
		XIntc *InstPtr = XIntcGetInstance(IntcParent);
		if (InstPtr != NULL)
			XIntc_Disable(InstPtr, IntrNum);
#endif
#endif
	}
}

#if defined (XPAR_SCUGIC)
/*****************************************************************************/
/**
*
* @brief    Configures the priority and trigger type.
*
* @param    IntrId: Interrupt Id.
* @param    Priority: Priority of the interrupt
* @param    IntcParent: Interrupt controller baseaddress and type.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XSetPriorityTriggerType( u32 IntrId, u8 Priority, UINTPTR IntcParent)
{
	u32 TriggerType = XGet_TriggerType(IntrId);

	/* Edge trigger types (1,2) map to edge-triggered hardware config */
	u8 Trigger = ((TriggerType == XIL_TRIG_LOW_TO_HIGH_EDGE) ||
	              (TriggerType == XIL_TRIG_HIGH_TO_LOW_EDGE)) ?
	              XINTR_IS_EDGE_TRIGGERED : XINTR_IS_LEVEL_TRIGGERED;

	u16 IntrNum = XGet_IntrId(IntrId);
	u16 Offset = XGet_IntrOffset(IntrId);

	IntrNum += Offset;
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
		XScuGic_SetPriorityTriggerType(&XScuGicInstance, IntrNum, Priority, Trigger);
	}
}

/*****************************************************************************/
/**
*
* @brief    Gets the priority of the interrupt controller.
*
* @param    IntrId: Interrupt Id.
* @param    Priority: Priority of the interrupt
* @param    Trigger: Trigger type of the interrupt
* @param    IntcParent: Interrupt controller baseaddress and type.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XGetPriorityTriggerType( u32 IntrId, u8 *Priority, u8 *Trigger,  UINTPTR IntcParent)
{
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
		u16 IntrNum = XGet_IntrId(IntrId);
		u16 Offset = XGet_IntrOffset(IntrId);

		IntrNum += Offset;
		XScuGic_GetPriorityTriggerType(&XScuGicInstance, IntrNum, Priority, Trigger);
	}
}
#endif

/*****************************************************************************/
/**
*
* @brief    stops the interrupt controller.
*
* @param    IntcParent: Interrupt controller baseaddress and type.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XStopInterruptCntrl( UINTPTR IntcParent)
{
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		XScuGic_Stop(&XScuGicInstance);
#endif
	} else {
#if defined (XPAR_AXI_INTC)
		XIntc *StopInstPtr = XIntcGetInstance(IntcParent);
		if (StopInstPtr != NULL)
			XIntc_Stop(StopInstPtr);
#endif
	}

}

/*****************************************************************************/
/**
*
* @brief    Registers the interrupt handler.
*
* @param    IntrHandler: Interrupt handler.
* @param    IntcParent: Interrupt controller baseaddress and type.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void XRegisterInterruptHandler(void *IntrHandler,  UINTPTR IntcParent)
{
#if !defined (XPAR_SCUGIC) && !defined (XPAR_AXI_INTC)
	(void)IntrHandler;
#endif
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		if (IntrHandler == NULL) {
			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, \
						     (Xil_ExceptionHandler) XScuGic_InterruptHandler,
						     &XScuGicInstance);
		} else {
			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, \
						     (Xil_ExceptionHandler) IntrHandler,
						     &XScuGicInstance);

		}
#endif
	} else {
#if defined (XPAR_AXI_INTC)
		/* Register the correct chain root's instance as the exception handler. */
		XIntc *RegInstPtr = XIntcGetInstance(IntcParent);
		if (RegInstPtr == NULL)
			return;
		if (IntrHandler == NULL) {
			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, \
						     (Xil_ExceptionHandler) XIntc_InterruptHandler,
						     RegInstPtr);
		} else {
			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, \
						     (Xil_ExceptionHandler) IntrHandler,
						     RegInstPtr);
		}
#endif
	}
}

/*****************************************************************************/
/**
*
* @brief    Setup the interrupt system.
*
* @param    DriverInstance: Driver instance pointer.
* @param    IntrHandler: Interrupt handler function pointer.
* @param    IntrId: Interrupt Id.
* @param    IntcParent: Interrupt controller baseaddress and type.
* @param    Priority: Interrupt priority.
*
* @return   XST_SUCCESS if initialization was successful
* 	    XST_FAILURE in case of failure
*
* @note     None.
*
******************************************************************************/
int XSetupInterruptSystem(void *DriverInstance, void *IntrHandler, u32 IntrId,  UINTPTR IntcParent,
			  u16 Priority)
{
	int Status;

	Status = XConfigInterruptCntrl(IntcParent);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#if defined (XPAR_SCUGIC)
	/* Only set ScuGicInitialized if we actually initialized a GIC */
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
		ScuGicInitialized = TRUE;
	}
	XSetPriorityTriggerType( IntrId, Priority, IntcParent);
#else
	(void)Priority;
#endif
	Status = XConnectToInterruptCntrl( IntrId, IntrHandler, \
					   DriverInstance, IntcParent);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
#if defined (XPAR_AXI_INTC)
	XStartInterruptCntrl(XIN_REAL_MODE, IntcParent);
#endif
	Xil_ExceptionInit();

#if defined (XPAR_AXI_INTC) && defined (XPAR_SCUGIC) && defined (SDT)
	/* Bridge INTC->GIC only for INTC-type IntcParent; skip for direct GIC interrupts. */
	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_INTC) {
		u16 GicIntrNum;
		u8 cpu_mask;
		u32 j, gicConnected = 0;
		XIntc *InstPtr = XIntcGetInstance(IntcParent);

		if (InstPtr == NULL)
			return XST_FAILURE;

		GicIntrNum = XGet_IntrId(InstPtr->CfgPtr->IntrId) +
			     XGet_IntrOffset(InstPtr->CfgPtr->IntrId);

		/* Connect each root's GIC SPI once; track via array (NULL check unreliable). */
		for (j = 0; j < XIntcGicConnectedCount; j++) {
			if (XIntcGicConnectedRoots[j] == InstPtr->BaseAddress) {
				gicConnected = 1;
				break;
			}
		}

		if (!gicConnected) {
			if (!ScuGicInitialized) {
				Status = XConfigInterruptCntrl(InstPtr->CfgPtr->IntrParent);
				if (Status != XST_SUCCESS)
					return XST_FAILURE;
				ScuGicInitialized = TRUE;
			}

			cpu_mask = (u8)XScuGic_GetCpuID();

			XScuGic_SetPriorityTriggerType(&XScuGicInstance,
						       GicIntrNum,
						       XINTERRUPT_DEFAULT_PRIORITY,
						       XINTR_IS_LEVEL_TRIGGERED);

			Status = XScuGic_Connect(&XScuGicInstance, GicIntrNum,
						 (Xil_ExceptionHandler)XIntc_InterruptHandler,
						 (void *)InstPtr);
			if (Status != XST_SUCCESS)
				return XST_FAILURE;

			XScuGic_InterruptMaptoCpu(&XScuGicInstance, cpu_mask, GicIntrNum);
			XScuGic_Enable(&XScuGicInstance, GicIntrNum);

			if (XIntcGicConnectedCount < XPAR_INTC_NUM_DRV_INSTANCES)
				XIntcGicConnectedRoots[XIntcGicConnectedCount++] = InstPtr->BaseAddress;
			else
				return XST_FAILURE;

			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
						     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
						     &XScuGicInstance);
		}
	} else {
		XRegisterInterruptHandler(NULL, IntcParent);
	}
#else
	XRegisterInterruptHandler(NULL, IntcParent);
#endif

	XEnableIntrId(IntrId, IntcParent);
	Xil_ExceptionEnable();
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* @brief    Convert legacy interrupt id to encoded interrupt ID needed by
*           interrupt wrapper layer.
*
* @param    LegacyIntrId Interrupt ID of specific peripheral/PL-PS interrupt
*           port documented in TRM of targeted SoC.
* @param  : TriggerType Trigger type for targeted interrupt ID (documented in
*           TRM of targeted SoC)
* 		1 = low-to-high edge triggered
*	        2 = high-to-low edge triggered
*	        4 = active high level-sensitive
*	        8 = active low level-sensitive
*
* @param    IntrType Interrupt type of targeted interrupt ID.
* 		0 = SPI
* 		1 = PPI (not applicable for AXI INTC)
* 		2 = SGI
* @param    IntcType Parent interrupt controller of targeted interrupt ID
* 		1 = AXI INTC
* 		0 = GIC
*
* @param    IntrId Interrupt ID in encoded format compliant with interrupt
*           wrapper layer. It is output parameter.
* @return   XST_SUCCESS if LegacyIntrId is encoded successfully and copied to
*                       IntrId
*           XST_FAILURE in case of incorrect input parameter
*
* @note     None.
*
******************************************************************************/
s32 XGetEncodedIntrId(u32 LegacyIntrId, u32 TriggerType, u8 IntrType, u8 IntcType, u32 *IntrId )
{
	s32 Status = XST_FAILURE;

	if (IntcType != XINTC_TYPE_IS_SCUGIC && IntcType != XINTC_TYPE_IS_INTC) {
		return Status;
	}
	*IntrId = LegacyIntrId;

	if (IntcType == XINTC_TYPE_IS_SCUGIC) {
		if (IntrType == XSPI) {
			*IntrId -= 32;
		} else if (IntrType == XPPI) {
			*IntrId -= 16;
			*IntrId |= (1 << XINTC_INTR_TYPE_SHIFT);
		} else if (IntrType == XSGI) {
			*IntrId |= (1 << XINTC_IS_SGI_INTR_SHIFT);
		} else {
			return Status;
		}
	}

	Status = XST_SUCCESS;

	*IntrId |= ((TriggerType << XINTC_TRIGGER_SHIFT) & XINTC_TRIGGER_MASK);

	return Status;

}
/*****************************************************************************/
/**
*
* @brief    Trigger software interrupt
*
* @param    IntrId Targeted interrupt ID
* @param    IntcType Parent interrupt controller base address in encoded format
* @param    Cpu_Id List of CPUs to send the interrupt. NA for AXI INTC.
*           For VERSAL_NET bits 0-7 specifies core ID to send the interrupt.
*           bits 8-15 specifies the cluster id.

* @return   XST_SUCCESS - Successful generation of software interrupt
*           XST_FAILURE - in case of failure
*
* @note     None.
*
******************************************************************************/
s32 XTriggerSoftwareIntr(u32 IntrId, UINTPTR IntcParent, u32 Cpu_Id)
{
	s32 Status = XST_SUCCESS;
	u16 IntrNum = XGet_IntrId(IntrId);
#if !defined (XPAR_SCUGIC) && !defined (XPAR_AXI_INTC)
	(void) IntrNum;
	(void) Cpu_Id;
#endif

	if (XGet_IntcType(IntcParent) == XINTC_TYPE_IS_SCUGIC) {
#if defined (XPAR_SCUGIC)
		Status = XScuGic_SoftwareIntr(&XScuGicInstance, IntrNum, Cpu_Id);
#endif
	} else {
#if defined (XPAR_AXI_INTC)
		(void) Cpu_Id;
#ifdef SDT
		/* SDT: IntcParent encodes the target controller; write directly to its ISR */
		UINTPTR TgtBase = XGet_BaseAddr(IntcParent);
		XIntc_Config *TgtCfg;
		XIntc TempSwInst;

		TgtCfg = XIntc_LookupConfig(TgtBase);
		if (TgtCfg == NULL)
			return XST_FAILURE;
		TempSwInst.BaseAddress = TgtBase;
		TempSwInst.CfgPtr = TgtCfg;
		TempSwInst.IsReady = XIL_COMPONENT_IS_READY;
		Status = XIntc_TriggerSwIntr(&TempSwInst, IntrNum);
#else
		XIntc *SwInstPtr = XIntcGetInstance(IntcParent);
		if (SwInstPtr == NULL)
			return XST_FAILURE;
		Status = XIntc_TriggerSwIntr(SwInstPtr, IntrNum);
#endif
#endif
	}

	return Status;

}

#endif
