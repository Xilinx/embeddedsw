/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xgpiops_intr.c
* @brief Implements GPIO PS interrupt handling functions.
*
* This file contains functions related to GPIO interrupt handling.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   01/18/10 First Release
* 2.2	sk	 10/13/14 Used Pin number in Bank instead of pin number
* 					  passed to API's. CR# 822636
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.1	kvn  04/13/15 Add support for Zynq Ultrascale+ MP. CR# 856980.
* 3.1   aru  07/13/18 Resolved doxygen reported warnings. CR# 1006331.
* 3.4   aru  08/09/18 Resolved cppcheck warnings.
* 3.4   aru  08/17/18 Resolved MISRA-C mandatory violations. CR# 1007751
* 3.5   sne  03/01/19 Fixes violations according to MISRAC-2012
*                     in safety mode and modified the code such as
*                     Use of mixed mode arithmetic,Declared the pointer param
*                     as Pointer to const,Casting operation to a pointer,
*                     Literal value requires a U suffix.
* 3.5   sne  03/14/19 Added Versal support.
* 3.5   sne  03/20/19 Fixed multiple interrupts problem CR#1024556.
* 3.6	sne  06/12/19 Fixed IAR compiler warning.
* 3.6   sne  08/14/19 Added interrupt handler support on versal.
* 3.13  gm   03/15/24 Added multi-core interrupt support.
* 3.16  vn   07/30/26 Fix Doxygen warnings
* 3.16  vn   09/10/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

/** @addtogroup gpiops Overview
 * @{
 */

/***************************** Include Files *********************************/

#include "xgpiops.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
*
* @brief Enables the interrupts for the specified pins in the specified bank.
*
* @param[in,out] InstancePtr Pointer to the XGpioPs instance.
* @param[in]     Bank Bank number of the GPIO to operate on.
* @param[in]     Mask Bit mask of the pins for which to enable interrupts.
*                Bit positions of 1 are enabled. Bit positions of 0 keep the
*                previous setting.
*
******************************************************************************/
void XGpioPs_IntrEnable(XGpioPs *InstancePtr, u8 Bank, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertVoid(Bank < InstancePtr->MaxBanks);
#ifdef versal
        if(InstancePtr->PmcGpio == (u32)TRUE) {
                Xil_AssertVoid(Bank != XGPIOPS_TWO);
        } else {
                Xil_AssertVoid((Bank !=XGPIOPS_ONE) && (Bank !=XGPIOPS_TWO));
        }
#endif

	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTEN_OFFSET, Mask);

	/* Setup mask for CPU */
	InstancePtr->CoreIntrMask[Bank] |= Mask;
}

/*****************************************************************************/
/**
*
* @brief Enables the interrupt for the specified pin.
*
* @param[in,out] InstancePtr Pointer to the XGpioPs instance.
* @param[in]     Pin Pin number for which to enable the interrupt.
*
******************************************************************************/
void XGpioPs_IntrEnablePin(XGpioPs *InstancePtr, u32 Pin)
{
	u8 Bank;
	u8 PinNumber;
	u32 IntrReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Pin < InstancePtr->MaxPinNum);

	/* Get the Bank number and Pin number within the bank. */
#ifdef versal
	XGpioPs_GetBankPin(InstancePtr,(u8)Pin, &Bank, &PinNumber);
#else
	XGpioPs_GetBankPin((u8)Pin, &Bank, &PinNumber);
#endif

	IntrReg = ((u32)1 << (u32)PinNumber);
	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTEN_OFFSET, IntrReg);

	/* Setup mask for CPU */
	InstancePtr->CoreIntrMask[Bank] |= IntrReg;
}

/*****************************************************************************/
/**
*
* @brief Disables the interrupts for the specified pins in the specified bank.
*
* @param[in,out] InstancePtr Pointer to the XGpioPs instance.
* @param[in]     Bank Bank number of the GPIO to operate on.
* @param[in]     Mask Bit mask of the pins for which to disable interrupts.
*                Bit positions of 1 are disabled. Bit positions of 0 keep the
*                previous setting.
*
******************************************************************************/
void XGpioPs_IntrDisable(XGpioPs *InstancePtr, u8 Bank, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Bank < InstancePtr->MaxBanks);
#ifdef versal
        if(InstancePtr->PmcGpio == (u32)TRUE) {
                Xil_AssertVoid(Bank != XGPIOPS_TWO);
        } else {
                Xil_AssertVoid((Bank !=XGPIOPS_ONE) && (Bank !=XGPIOPS_TWO));
        }
#endif

	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTDIS_OFFSET, Mask);

	/* Setup mask for CPU */
	InstancePtr->CoreIntrMask[Bank] &= ~Mask;
}

/*****************************************************************************/
/**
*
* @brief Disables the interrupts for the specified pin.
*
* @param[in,out] InstancePtr Pointer to the XGpioPs instance.
* @param[in]     Pin Pin number for which to disable the interrupt.
*
******************************************************************************/
void XGpioPs_IntrDisablePin(XGpioPs *InstancePtr, u32 Pin)
{
	u8 Bank;
	u8 PinNumber;
	u32 IntrReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Pin < InstancePtr->MaxPinNum);

	/* Get the Bank number and Pin number within the bank. */
#ifdef versal
	XGpioPs_GetBankPin(InstancePtr,(u8)Pin, &Bank, &PinNumber);
#else
	XGpioPs_GetBankPin((u8)Pin, &Bank, &PinNumber);
#endif

	IntrReg = ((u32)1 << (u32)PinNumber);
	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTDIS_OFFSET, IntrReg);

	/* Setup mask for CPU */
	InstancePtr->CoreIntrMask[Bank] &= ~IntrReg;
}

/*****************************************************************************/
/**
*
* @brief Returns the interrupt enable status for a bank.
*
* @param[in]   InstancePtr Pointer to the XGpioPs instance.
* @param[in]   Bank Bank number of the GPIO to operate on.
*
* @return      Enabled interrupts in a 32-bit format. Bit positions with 1
*              indicate that the interrupt for that pin is enabled. Bit
*              positions with 0 indicate that the interrupt for that pin is
*              disabled.
*
******************************************************************************/
u32 XGpioPs_IntrGetEnabled(const XGpioPs *InstancePtr, u8 Bank)
{
	u32 IntrMask;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertNonvoid(Bank < InstancePtr->MaxBanks);
#ifdef versal
        if(InstancePtr->PmcGpio == (u32)TRUE) {
                Xil_AssertNonvoid(Bank != XGPIOPS_TWO);
        } else {
                Xil_AssertNonvoid((Bank !=XGPIOPS_ONE) && (Bank !=XGPIOPS_TWO));
        }
#endif

	IntrMask = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				    ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				    XGPIOPS_INTMASK_OFFSET);
	return (~IntrMask);
}

/*****************************************************************************/
/**
*
* @brief Returns whether interrupts are enabled for the specified pin.
*
* @param[in]   InstancePtr Pointer to the XGpioPs instance.
* @param[in]   Pin Pin number for which to get the interrupt enable status.
*
* @return
*              - TRUE if the interrupt is enabled.
*              - FALSE if the interrupt is disabled.
*
******************************************************************************/
u32 XGpioPs_IntrGetEnabledPin(const XGpioPs *InstancePtr, u32 Pin)
{
	u8 Bank;
	u8 PinNumber;
	u32 IntrReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Pin < InstancePtr->MaxPinNum);

	/* Get the Bank number and Pin number within the bank. */
#ifdef versal
	XGpioPs_GetBankPin(InstancePtr,(u8)Pin, &Bank, &PinNumber);
#else
	XGpioPs_GetBankPin((u8)Pin, &Bank, &PinNumber);
#endif

	IntrReg  = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				    ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				    XGPIOPS_INTMASK_OFFSET);

	return (((IntrReg & ((u32)1 << PinNumber)) != (u32)0)? (u32)FALSE : (u32)TRUE);
}

/*****************************************************************************/
/**
*
* @brief Returns the interrupt status read from Interrupt Status Register.
*
* @param[in]   InstancePtr Pointer to the XGpioPs instance.
* @param[in]   Bank Bank number of the GPIO to operate on.
*
* @return      The value read from Interrupt Status Register.
*
******************************************************************************/
u32 XGpioPs_IntrGetStatus(const XGpioPs *InstancePtr, u8 Bank)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
        Xil_AssertNonvoid(Bank < InstancePtr->MaxBanks);
#ifdef versal
        if(InstancePtr->PmcGpio == (u32)TRUE) {
                Xil_AssertNonvoid(Bank != XGPIOPS_TWO);
        } else {
                Xil_AssertNonvoid((Bank !=XGPIOPS_ONE) && (Bank !=XGPIOPS_TWO));
        }
#endif

	return XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				XGPIOPS_INTSTS_OFFSET);
}

/*****************************************************************************/
/**
*
* @brief Returns the interrupt status of the specified pin.
*
* @param[in]   InstancePtr Pointer to the XGpioPs instance.
* @param[in]   Pin Pin number for which to get the interrupt status.
*
* @return
*              - TRUE if the interrupt has occurred.
*              - FALSE if the interrupt has not occurred.
*
******************************************************************************/
u32 XGpioPs_IntrGetStatusPin(const XGpioPs *InstancePtr, u32 Pin)
{
	u8 Bank;
	u8 PinNumber;
	u32 IntrReg;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Pin < InstancePtr->MaxPinNum);

	/* Get the Bank number and Pin number within the bank. */
#ifdef versal
	XGpioPs_GetBankPin(InstancePtr,(u8)Pin, &Bank, &PinNumber);
#else
	XGpioPs_GetBankPin((u8)Pin, &Bank, &PinNumber);
#endif

	IntrReg = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				   ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				   XGPIOPS_INTSTS_OFFSET);

	return (((IntrReg & ((u32)1 << PinNumber)) != (u32)0)? (u32)TRUE : (u32)FALSE);
}

/*****************************************************************************/
/**
*
* @brief Clears the pending interrupts specified by the provided mask.
*
* Call this function after the software services the pending interrupts.
*
* @param[in]   InstancePtr Pointer to the XGpioPs instance.
* @param[in]   Bank Bank number of the GPIO to operate on.
* @param[in]   Mask Mask of the interrupts to clear. Bit positions of 1 are
*              cleared. Bit positions of 0 do not change the previous
*              interrupt status.
*
******************************************************************************/
void XGpioPs_IntrClear(const XGpioPs *InstancePtr, u8 Bank, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Bank < InstancePtr->MaxBanks);
#ifdef versal
        if(InstancePtr->PmcGpio == (u32)TRUE) {
                Xil_AssertVoid(Bank != XGPIOPS_TWO);
        } else {
                Xil_AssertVoid((Bank !=XGPIOPS_ONE) && (Bank !=XGPIOPS_TWO));
        }
#endif

	/* Clear the currently pending interrupts. */
	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTSTS_OFFSET, Mask);
}

/*****************************************************************************/
/**
*
* @brief Clears the specified pending interrupt.
*
* Call this function after the software services the pending interrupts.
*
* @param[in]   InstancePtr Pointer to the XGpioPs instance.
* @param[in]   Pin Pin number for which to clear the interrupt status.
*
******************************************************************************/
void XGpioPs_IntrClearPin(const XGpioPs *InstancePtr, u32 Pin)
{
	u8 Bank;
	u8 PinNumber;
	u32 IntrReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Pin < InstancePtr->MaxPinNum);

	/* Get the Bank number and Pin number within the bank. */
#ifdef versal
	XGpioPs_GetBankPin(InstancePtr,(u8)Pin, &Bank, &PinNumber);
#else
	XGpioPs_GetBankPin((u8)Pin, &Bank, &PinNumber);
#endif

	/* Clear the specified pending interrupts. */
	IntrReg = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				   ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				   XGPIOPS_INTSTS_OFFSET);

	IntrReg &= ((u32)1 << PinNumber);
	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTSTS_OFFSET, IntrReg);
}

/*****************************************************************************/
/**
*
* @brief Sets the interrupt type, polarity, and on-any setting of bank pins.
*
* Sets the Interrupt Type, Interrupt Polarity, and Interrupt On Any for the
* specified GPIO bank pins.
*
* @param[in]   InstancePtr Pointer to the XGpioPs instance.
* @param[in]   Bank Bank number of the GPIO to operate on.
* @param[in]   IntrType 32-bit mask of the interrupt type. 0 means Level
*              Sensitive and 1 means Edge Sensitive.
* @param[in]   IntrPolarity 32-bit mask of the interrupt polarity. 0 means
*              Active Low or Falling Edge and 1 means Active High or Rising
*              Edge.
* @param[in]   IntrOnAny 32-bit mask of the interrupt trigger for edge
*              triggered interrupts. 0 means trigger on single edge using the
*              configured interrupt polarity and 1 means trigger on both
*              edges.
*
* @note        Use this function to set the interrupt related properties of
*              all the pins in the specified bank. The previous state of the
*              pins is not maintained. To change the interrupt properties of
*              a single GPIO pin, use XGpioPs_SetPinIntrType().
*
******************************************************************************/
void XGpioPs_SetIntrType(const XGpioPs *InstancePtr, u8 Bank, u32 IntrType,
			  u32 IntrPolarity, u32 IntrOnAny)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Bank < InstancePtr->MaxBanks);
#ifdef versal
        if(InstancePtr->PmcGpio == (u32)TRUE) {
                Xil_AssertVoid(Bank != XGPIOPS_TWO);
        } else {
                Xil_AssertVoid((Bank !=XGPIOPS_ONE) && (Bank !=XGPIOPS_TWO));
        }
#endif

	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTTYPE_OFFSET, IntrType);

	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTPOL_OFFSET, IntrPolarity);

	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTANY_OFFSET, IntrOnAny);
}

/*****************************************************************************/
/**
*
* @brief Gets the interrupt type, polarity, and on-any setting of bank pins.
*
* Gets the Interrupt Type, Interrupt Polarity, and Interrupt On Any for the
* specified GPIO bank pins.
*
* @param[in]   InstancePtr Pointer to the XGpioPs instance.
* @param[in]   Bank Bank number of the GPIO to operate on.
* @param[out]  IntrType Returns the 32-bit mask of the interrupt type. 0 means
*              Level Sensitive and 1 means Edge Sensitive.
* @param[out]  IntrPolarity Returns the 32-bit mask of the interrupt
*              polarity. 0 means Active Low or Falling Edge and 1 means
*              Active High or Rising Edge.
* @param[out]  IntrOnAny Returns the 32-bit mask of the interrupt trigger for
*              edge triggered interrupts. 0 means trigger on single edge
*              using the configured interrupt polarity and 1 means trigger on
*              both edges.
*
******************************************************************************/
void XGpioPs_GetIntrType(const XGpioPs *InstancePtr, u8 Bank, u32 *IntrType,
			  u32 *IntrPolarity, u32 *IntrOnAny)

{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Bank < InstancePtr->MaxBanks);
#ifdef versal
        if(InstancePtr->PmcGpio == (u32)TRUE) {
                Xil_AssertVoid(Bank != XGPIOPS_TWO);
        } else {
                Xil_AssertVoid((Bank !=XGPIOPS_ONE) && (Bank !=XGPIOPS_TWO));
        }
#endif

	*IntrType = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				     ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				     XGPIOPS_INTTYPE_OFFSET);

	*IntrPolarity = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
					 ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
					 XGPIOPS_INTPOL_OFFSET);

	*IntrOnAny = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				      ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				      XGPIOPS_INTANY_OFFSET);
}

/*****************************************************************************/
/**
*
* @brief Sets the IRQ type of a single GPIO pin.
*
* @param[in]   InstancePtr Pointer to the XGpioPs instance.
* @param[in]   Pin Pin number whose IRQ type is set.
* @param[in]   IrqType IRQ type for the GPIO pin. Use the XGPIOPS_IRQ_TYPE_*
*              constants defined in xgpiops.h to specify the IRQ type.
*
******************************************************************************/
void XGpioPs_SetIntrTypePin(const XGpioPs *InstancePtr, u32 Pin, u8 IrqType)
{
	u32 IntrTypeReg;
	u32 IntrPolReg;
	u32 IntrOnAnyReg;
	u8 Bank;
	u8 PinNumber;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Pin < InstancePtr->MaxPinNum);
	Xil_AssertVoid(IrqType <= XGPIOPS_IRQ_TYPE_LEVEL_LOW);

	/* Get the Bank number and Pin number within the bank. */
#ifdef versal
	XGpioPs_GetBankPin(InstancePtr,(u8)Pin, &Bank, &PinNumber);
#else
	XGpioPs_GetBankPin((u8)Pin, &Bank, &PinNumber);
#endif

	IntrTypeReg = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				       ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				       XGPIOPS_INTTYPE_OFFSET);

	IntrPolReg = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				      ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				      XGPIOPS_INTPOL_OFFSET);

	IntrOnAnyReg = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
					((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
					XGPIOPS_INTANY_OFFSET);

	switch (IrqType) {
		case XGPIOPS_IRQ_TYPE_EDGE_RISING:
			IntrTypeReg |= ((u32)1 << (u32)PinNumber);
			IntrPolReg |= ((u32)1 << (u32)PinNumber);
			IntrOnAnyReg &= ~((u32)1 << (u32)PinNumber);
			break;
		case XGPIOPS_IRQ_TYPE_EDGE_FALLING:
			IntrTypeReg |= ((u32)1 << (u32)PinNumber);
			IntrPolReg &= ~((u32)1 << (u32)PinNumber);
			IntrOnAnyReg &= ~((u32)1 << (u32)PinNumber);
			break;
		case XGPIOPS_IRQ_TYPE_EDGE_BOTH:
			IntrTypeReg |= ((u32)1 << (u32)PinNumber);
			IntrOnAnyReg |= ((u32)1 << (u32)PinNumber);
			break;
		case XGPIOPS_IRQ_TYPE_LEVEL_HIGH:
			IntrTypeReg &= ~((u32)1 << (u32)PinNumber);
			IntrPolReg |= ((u32)1 << (u32)PinNumber);
			break;
		case XGPIOPS_IRQ_TYPE_LEVEL_LOW:
			IntrTypeReg &= ~((u32)1 << (u32)PinNumber);
			IntrPolReg &= ~((u32)1 << (u32)PinNumber);
			break;
		default:
			/* Default statement is added for MISRA C compliance. */
			break;
	}

	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTTYPE_OFFSET, IntrTypeReg);

	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTPOL_OFFSET, IntrPolReg);

	XGpioPs_WriteReg(InstancePtr->GpioConfig.BaseAddr,
			  ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
			  XGPIOPS_INTANY_OFFSET, IntrOnAnyReg);
}

/*****************************************************************************/
/**
*
* @brief Returns the IRQ type of a given GPIO pin.
*
* @param[in]   InstancePtr Pointer to the XGpioPs instance.
* @param[in]   Pin Pin number whose IRQ type is obtained.
*
* @return      IRQ type of the given pin.
*
* @note        Use the XGPIOPS_IRQ_TYPE_* constants defined in xgpiops.h to
*              interpret the IRQ type returned by this function.
*
******************************************************************************/
u8 XGpioPs_GetIntrTypePin(const XGpioPs *InstancePtr, u32 Pin)
{
	u32 IntrType;
	u32 IntrPol;
	u32 IntrOnAny;
	u8 Bank;
	u8 PinNumber;
	u8 IrqType;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Pin < InstancePtr->MaxPinNum);

	/* Get the Bank number and Pin number within the bank. */
#ifdef versal
	XGpioPs_GetBankPin(InstancePtr,(u8)Pin, &Bank, &PinNumber);
#else
	XGpioPs_GetBankPin((u8)Pin, &Bank, &PinNumber);
#endif

	IntrType = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				    ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				    XGPIOPS_INTTYPE_OFFSET) & ((u32)1 << PinNumber);

	if (IntrType == ((u32)1 << PinNumber)) {

		IntrOnAny = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				     ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				     XGPIOPS_INTANY_OFFSET) & ((u32)1 << PinNumber);

		IntrPol = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				   ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				   XGPIOPS_INTPOL_OFFSET) & ((u32)1 << PinNumber);


		if (IntrOnAny == ((u32)1 << PinNumber)) {
			IrqType = XGPIOPS_IRQ_TYPE_EDGE_BOTH;
		} else if (IntrPol == ((u32)1 << PinNumber)) {
			IrqType = XGPIOPS_IRQ_TYPE_EDGE_RISING;
		} else {
			IrqType = XGPIOPS_IRQ_TYPE_EDGE_FALLING;
		}
	} else {

		IntrPol = XGpioPs_ReadReg(InstancePtr->GpioConfig.BaseAddr,
				   ((u32)(Bank) * XGPIOPS_REG_MASK_OFFSET) +
				   XGPIOPS_INTPOL_OFFSET) & ((u32)1 << PinNumber);

		if (IntrPol == ((u32)1 << PinNumber)) {
			IrqType = XGPIOPS_IRQ_TYPE_LEVEL_HIGH;
		} else {
			IrqType = XGPIOPS_IRQ_TYPE_LEVEL_LOW;
		}
	}

	return IrqType;
}

/*****************************************************************************/
/**
*
* @brief Sets the status callback function.
*
* XGpioPs_IntrHandler() calls the callback function when an interrupt
* occurs.
*
* @param[in,out] InstancePtr Pointer to the XGpioPs instance.
* @param[in]     CallBackRef Upper layer callback reference passed back when
*                the callback function is invoked.
* @param[in]     FuncPointer Pointer to the callback function.
*
* @note          The handler is called within interrupt context, so it should
*                do its work quickly and queue potentially time-consuming work
*                to a task-level thread.
*
******************************************************************************/
void XGpioPs_SetCallbackHandler(XGpioPs *InstancePtr, void *CallBackRef,
				 XGpioPs_Handler FuncPointer)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(FuncPointer != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	InstancePtr->Handler = FuncPointer;
	InstancePtr->CallBackRef = CallBackRef;
}

/*****************************************************************************/
/**
*
* @brief Handles GPIO interrupts.
*
* Checks the interrupt status registers of all the banks to determine the
* actual bank in which an interrupt has been triggered. It then calls the
* upper layer callback handler set by XGpioPs_SetBankHandler(). The
* callback is called when an interrupt occurs.
*
* @param[in]   InstancePtr Pointer to the XGpioPs instance.
*
* @note        This function does not save and restore the processor context,
*              so you must provide this processing.
*
******************************************************************************/
void XGpioPs_IntrHandler(const XGpioPs *InstancePtr)
{
	u8 Bank;
	u32 Mask;
	u32 IntrStatus;
	u32 IntrEnabled;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	for (Bank = 0U; Bank < InstancePtr->MaxBanks; Bank++) {
#ifdef versal
		if(InstancePtr->PmcGpio == (u32)TRUE) {
			if(Bank == XGPIOPS_TWO) {
				continue;
			}
		} else {
			if((Bank == XGPIOPS_ONE) || (Bank == XGPIOPS_TWO)) {
				continue;
			}
		}
#endif
		IntrStatus = XGpioPs_IntrGetStatus(InstancePtr, Bank);
		IntrEnabled = XGpioPs_IntrGetEnabled(InstancePtr,Bank);
		Mask = IntrStatus & IntrEnabled & InstancePtr->CoreIntrMask[Bank];

		if (Mask != (u32)0)
		{
			XGpioPs_IntrClear(InstancePtr, Bank, Mask);
			InstancePtr->Handler(InstancePtr->CallBackRef, Bank, Mask);
		}
	}
}
/** @} */
