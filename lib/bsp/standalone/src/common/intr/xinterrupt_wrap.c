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
static XIntc XIntcInstance ;
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
	XIntc_Config *PrimaryCfg;
	UINTPTR InitBaseAddr;
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
		if (XIntcInstance.IsReady != XIL_COMPONENT_IS_READY) {
			/*
			 * In cascade mode, override the caller-supplied BaseAddr
			 * with the PRIMARY controller's address, since
			 * XIntc_Initialize must always target the root controller
			 * that connects to the CPU (or GIC).
			 */
			PrimaryCfg = XIntc_GetPrimaryController();
			if (PrimaryCfg != NULL &&
			    PrimaryCfg->IntcType != XIN_INTC_NOCASCADE) {
				InitBaseAddr = PrimaryCfg->BaseAddress;
			} else {
				InitBaseAddr = BaseAddr;
			}

			Status = XIntc_Initialize(&XIntcInstance, InitBaseAddr);
		} else {
			Status = XST_SUCCESS;
		}
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
		Status = XIntc_Connect(&XIntcInstance, IntrNum, \
				       (XInterruptHandler)IntrHandler, CallBackRef);
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
		Status = XIntc_ConnectFastHandler(&XIntcInstance, IntrNum, \
						  (XFastInterruptHandler)IntrHandler);
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
		XIntc_Disconnect(&XIntcInstance, IntrNum);
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
		if (XIntcInstance.IsStarted != XIL_COMPONENT_IS_STARTED) {
			Status = XIntc_Start(&XIntcInstance, Mode);
		} else {
			Status = XST_SUCCESS;
		}
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
		XIntc_Enable(&XIntcInstance, IntrNum);
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
		XIntc_Disable(&XIntcInstance, IntrNum);
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
		XIntc_Stop(&XIntcInstance);
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
		/*
		 * For cascade mode, the exception handler must always be registered with
		 * the PRIMARY controller (connected to CPU), not secondary controllers.
		 * Use XIntcInstance which is initialized with the PRIMARY controller.
		 */
		if (IntrHandler == NULL) {
			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, \
						     (Xil_ExceptionHandler) XIntc_InterruptHandler,
						     &XIntcInstance);
		} else {
			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, \
						     (Xil_ExceptionHandler) IntrHandler,
						     &XIntcInstance);
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
	{
		static int GicIntcSetup = 0;
		XIntc_Config *PrimaryCfg = XIntc_GetPrimaryController();

		if (!GicIntcSetup && PrimaryCfg != NULL &&
		    (PrimaryCfg->IntrParent != XINTC_NO_PARENT) &&
		    XGet_IntcType(PrimaryCfg->IntrParent) == XINTC_TYPE_IS_SCUGIC) {
			u16 GicIntrNum;

			/* Initialize GIC if needed for INTC->GIC connection */
			if (!ScuGicInitialized) {
				Status = XConfigInterruptCntrl(PrimaryCfg->IntrParent);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				ScuGicInitialized = TRUE;
			}

			GicIntrNum = XGet_IntrId(PrimaryCfg->IntrId) +
				     XGet_IntrOffset(PrimaryCfg->IntrId);

			XScuGic_SetPriorityTriggerType(&XScuGicInstance,
						       GicIntrNum,
						       XINTERRUPT_DEFAULT_PRIORITY,
						       XINTR_IS_LEVEL_TRIGGERED);

			Status = XScuGic_Connect(&XScuGicInstance, GicIntrNum,
						 (Xil_ExceptionHandler)XIntc_InterruptHandler,
						 &XIntcInstance);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			XScuGic_Enable(&XScuGicInstance, GicIntrNum);

			/*
			 * In INTC->GIC mode, register GIC as the CPU exception
			 * handler since it is the top-level controller.
			 */
			Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
						     (Xil_ExceptionHandler)XScuGic_InterruptHandler,
						     &XScuGicInstance);

			GicIntcSetup = 1;
		} else {
			XRegisterInterruptHandler(NULL, IntcParent);
		}
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
		Status = XIntc_TriggerSwIntr(&XIntcInstance, IntrNum);
#endif
	}

	return Status;

}

#endif
