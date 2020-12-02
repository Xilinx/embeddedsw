/******************************************************************************
* Copyright (C) 2001 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xwdttb.c
* @addtogroup wdttb_v5_1
* @{
*
* Contains the required functions of the XWdtTb driver. See xwdttb.h for a
* description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a ecm  08/16/01 First release
* 1.00b jhl  02/21/02 Repartitioned the driver for smaller files
* 1.00b rpm  04/26/02 Made LookupConfig public
* 1.10b mta  03/23/07 Updated to new coding style
* 2.00a ktn  10/22/09 Updated to use the HAL processor APIs/macros.
* 4.0   sha  12/17/15 Added Window WDT feature with basic mode.
*                     Moved XWdtTb_LookupConfig definition to xwdttb_sinit.c.
*                     Adherence to coding and Doxygen guidelines.
*                     Removed included xil_io, xil_types, xparameters and
*                     xil_assert header files.
*                     Moved XWdtTb_GetTbValue to xwdttb.h file.
*                     Adherence to MISRA-C guidelines.
* 4.0   sha  01/29/16 Added functions for Window WDT feature:
*                     XWdtTb_AlwaysEnable, XWdtTb_ClearLastEvent,
*                     XWdtTb_ClearResetPending, XWdtTb_IntrClear,
*                     XWdtTb_SetByteCount, XWdtTb_GetByteCount,
*                     XWdtTb_SetByteSegment, XWdtTb_GetByteSegment,
*                     XWdtTb_EnableSst, XWdtTb_DisableSst, XWdtTb_EnablePsm,
*                     XWdtTb_DisablePsm, XWdtTb_EnableFailCounter,
*                     XWdtTb_DisableFailCounter, XWdtTb_EnableExtraProtection,
*                     XWdtTb_DisableExtraProtection, XWdtTb_SetWindowCount,
*                     XWdtTb_EnableWinWdt, XWdtTb_DisableWinWdt,
*                     XWdtTb_CfgInitialize.
*
*                     Updated functions with Window WDT feature:
*                     XWdtTb_Start, XWdtTb_Stop, XWdtTb_IsWdtExpired,
*                     XWdtTb_RestartWdt.
*
*                     Modified lines with exceeding maximum 80 chars.
*                     Changed multi line comments to single line comments
*                     wherever required.
* 4.3   srm  01/27/18 Added XWdtTb_ProgramWDTWidth which can program the
*					  width of WDT
*            01/30/18 Added doxygen tags
* 4.4   aru  11/15/18 Replaced "Xil_AssertVoid" as "Xil_AssertNonvoid"
*                     in XWdtTb_ProgramWDTWidth().
* 4.4   sne  02/28/19 Added Static functions for Window WDT feature:
*                     XWdtTb_EnableTiWdt,XWdtTb_DisableTiWdt.
* 4.4   sne  03/01/19 Fixed violations according to MISRAC-2012 standards
*                     modified the code for below violations,
*                     No brackets to then/else,
*                     Literal value requires a U suffix,Function return
*                     type inconsistent,Logical conjunctions need brackets,
*                     Declared the pointer param as Pointer to const,
*                     Procedure has more than one exit point.
* 4.4   sne  03/04/19 Added support for Versal( Generic Watchdog and
*                     Window Watchdog Timer). Added below functions:
*                     XWdtTb_EnableGenericWdt,XWdtTb_DisableGenericWdt,
*                     XWdtTb_EnableTimebaseWdt,XWdtTb_DisableTimebaseWdt,
*                     XWdtTb_IsGenericWdtFWExpired,XWdtTb_SetGenericWdtWindow.
* 4.5  sne  06/25/19 Fixed Coverity warning in driver file.
* 4.5  sne   09/27/19 Added common driver support for Window watchdog Timer
*		      and AXI Timebase watchdog timer.
* 5.0  sne   01/31/20 Removed compare value registers write in
*		      XWdtTb_SetGenericWdtWindow function.
* 5.0  sne   02/27/20 Reorganize the driver source and Fixed doxygen warnings.
* 5.0  sne   03/09/20 Fixed MISRA-C violations.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xwdttb.h"
#include "xwdttb_config.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/
/**
*
* This function enables Window Watchdog Timer feature.
*
* @param       InstancePtr is a pointer to the XWdtTb instance to be
*              worked on.
*
* @return      None.
*
* @note        This will generate the first kick and start first window. This
*              auto clears MWC bit to make address space read only.
*
******************************************************************************/
static inline void XWdtTb_EnableWinWdt(XWdtTb *InstancePtr)
{
	u32 RegValue;

	/* Indicate that the device is started before we enable it */
	InstancePtr->IsStarted = XIL_COMPONENT_IS_STARTED;

	/* Read enable status register and update WEN bit */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
				  XWT_ESR_OFFSET) | XWT_ESR_WEN_MASK;

	/* Write enable status register with updated WEN value */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET,
			RegValue);
}

/*****************************************************************************/
/**
*
* This function enables generic Watchdog Timer feature.
*
* @param        InstancePtr is a pointer to the XWdtTb instance to be
*               worked on.
*
* @return       None.
*
* @note         This will Start the Generic Watchdog timer.Starts
*               the First window.
*
******************************************************************************/
static inline void XWdtTb_EnableGenericWdt(XWdtTb *InstancePtr)
{
	/* Indicate that the device is started before we enable it */
	InstancePtr->IsStarted = XIL_COMPONENT_IS_STARTED;
	/* Enable the Generic Watchdog Timer */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_GWCSR_OFFSET,
			((u32)XWT_GWCSR_GWEN_MASK));
}

/*****************************************************************************/
/**
*
* This function Disable Generic Watchdog Timer feature.
*
* @param        InstancePtr is a pointer to the XWdtTb instance to be
*               worked on.
*
* @return
*               - XST_SUCESS, if  Generic  WDT feature is disabled.
*               - XST_FAILURE, if Generic  WDT feature is not disabled.
*
* @note         This will Disable Generic Watchdog Timer.
*
******************************************************************************/
static inline s32 XWdtTb_DisableGenericWdt(XWdtTb *InstancePtr)
{
	u32 ControlStatusRegister0;
	s32 Status;
	ControlStatusRegister0 = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
						XWT_GWCSR_OFFSET);
	ControlStatusRegister0 &= (~(u32)XWT_GWCSR_GWEN_MASK);
	/* Disable the GWEN bit */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_GWCSR_OFFSET,
			ControlStatusRegister0);
	InstancePtr->IsStarted = (u32)0U;
	Status = (s32)XST_SUCCESS;
	return Status;
}

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief
*
* This function initializes the AXI Timebase Watchdog Timer core. This function
* must be called prior to using the core. Initialization of the core includes
* setting up the instance data and ensuring the hardware is in a quiescent
* state.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
* @param	CfgPtr points to the configuration structure associated with
*		the AXI Timebase Watchdog Timer core.
* @param	EffectiveAddr is the base address of the device. If address
*		translation is being used, then this parameter must reflect the
*		virtual base address. Otherwise, the physical address should be
*		used.
*
* @return
*		- XST_SUCCESS if initialization was successful.
*		- XST_DEVICE_IS_STARTED if the device has already been started.
*
* @note		None.
*
******************************************************************************/
s32 XWdtTb_CfgInitialize(XWdtTb *InstancePtr, const XWdtTb_Config *CfgPtr,
				UINTPTR EffectiveAddr)
{
	s32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(CfgPtr != NULL);
	Xil_AssertNonvoid(EffectiveAddr != (UINTPTR)0);

	/*
	 * If the device is started, disallow the initialize and return a
	 * status indicating it is started. This allows the user to stop the
	 * device and reinitialize, but prevents a user from inadvertently
	 * initializing.
	 */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		Status = (s32)XST_DEVICE_IS_STARTED;
	}
        else {
       InstancePtr->Config.DeviceId = CfgPtr->DeviceId;
	InstancePtr->Config.EnableWinWdt = CfgPtr->EnableWinWdt;
	InstancePtr->Config.MaxCountWidth = CfgPtr->MaxCountWidth;
	InstancePtr->Config.SstCountWidth = CfgPtr->SstCountWidth;
	InstancePtr->Config.IsPl = CfgPtr->IsPl;
	if(InstancePtr->Config.EnableWinWdt == 1U) {
		InstancePtr->Config.BaseAddr = EffectiveAddr + 0xCU;
	} else {
		InstancePtr->Config.BaseAddr = EffectiveAddr;
	}
	InstancePtr->IsStarted = (u32)0;
	InstancePtr->EnableFailCounter = (u32)0;
	if (InstancePtr->Config.IsPl == (u32)0) {
		InstancePtr->EnableWinMode = (u32)0U;
		/* Reset all the Generic WDT Registers */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_GW_WR_OFFSET,XWT_GW_WR_MASK);
	} else {
		InstancePtr->EnableWinMode = CfgPtr->EnableWinWdt;
	}
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	Status = (s32)XST_SUCCESS;
        }
	return Status;
}

/*****************************************************************************/
/**
* @brief
*
* Initialize a specific legacy/window watchdog timer/timebase instance/driver.
* This function must be called before other functions of the driver are called.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
* @param	DeviceId is the unique id of the device controlled by this
*		XWdtTb instance. Passing in a device id associates the generic
*		XWdtTb instance to a specific device, as chosen by the caller
*		or application developer.
*
* @return
*		- XST_SUCCESS if initialization was successful
*		- XST_DEVICE_IS_STARTED if the device has already been started
*		- XST_DEVICE_NOT_FOUND if the configuration for device ID was
*		not found
*
* @note		None.
*
******************************************************************************/
s32 XWdtTb_Initialize(XWdtTb *InstancePtr, u16 DeviceId)
{
	const XWdtTb_Config *ConfigPtr;
	s32 Status;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * If the device is started, disallow the initialize and return a
	 * status indicating it is started. This allows the user to stop the
	 * device and reinitialize, but prevents a user from inadvertently
	 * initializing.
	 */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		Status = (s32)XST_DEVICE_IS_STARTED;
		goto End;
	}

	ConfigPtr = XWdtTb_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		Status = (s32)XST_DEVICE_NOT_FOUND;
		goto End;
	}

	InstancePtr->Config = *ConfigPtr;
	InstancePtr->IsStarted = (u32)0;
	InstancePtr->EnableFailCounter = (u32)0;
        InstancePtr->EnableWinMode = (u32)0U;
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	Status = (s32)XST_SUCCESS;
End:
	return Status;
}

/*****************************************************************************/
/**
* @brief
*
* This function starts the legacy or window watchdog timer.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note
*		- For legacy, the Timebase is reset to 0 when the Watchdog
*		Timer is started. The Timebase is always incrementing.
*		- For window, this generates first kick and starts the first
*		window.
*		This step auto clears MWC bit to make address space read only.
*
******************************************************************************/
void XWdtTb_Start(XWdtTb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Check whether Window Mode is enabled */
        if(InstancePtr->EnableWinMode == (u32)TRUE) {
                /* Enable Window WDT */
                XWdtTb_EnableWinWdt(InstancePtr);
        } else {
		if (InstancePtr->Config.IsPl == (u32)0) {
                /* WWDT supports Generic watchdog timer & Window WDT features*/
                /* Enable Generic Watchdog Timer */
                XWdtTb_EnableGenericWdt(InstancePtr);
		} else {
                /* Enable Timebase Watchdog Timer */
                XWdtTb_EnableTimebaseWdt(InstancePtr);
		}
        }
}

/*****************************************************************************/
/**
* @brief
*
* This function disables the legacy or window watchdog timer.
*
* It is the caller's responsibility to disconnect the interrupt handler
* of the watchdog timer from the interrupt source, typically an interrupt
* controller, and disable the interrupt in the interrupt controller.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
* @return
*		- XST_SUCCESS if the legacy/window watchdog was stopped
*		successfully.
*		- XST_NO_FEATURE if disable is not supported in legacy watchdog
*		timer.
*		- XST_FAILURE if the window watchdog timer cannot be stopped.
*
* @note
*		- For legacy, the hardware configuration controls this
*		functionality. If it is not allowed by the hardware the
*		failure will be returned and the timer will continue without
*		interruption.
*
******************************************************************************/
s32 XWdtTb_Stop(XWdtTb *InstancePtr)
{
	s32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Check whether Window Mode is enabled */
        if(InstancePtr->EnableWinMode == (u32)TRUE) {
                /* Disable Window WDT */
                Status = XWdtTb_DisableWinWdt(InstancePtr);
        }
        else {
		if (InstancePtr->Config.IsPl == (u32)0) {
                /* Disable Generic Watchdog Timer */
                Status = XWdtTb_DisableGenericWdt(InstancePtr);
		} else {
                /* Disable Timebase Watchdog timer */
                Status = XWdtTb_DisableTimebaseWdt(InstancePtr);
		}
        }
	return Status;
}

/*****************************************************************************/
/**
* @brief
*
* This function checks if the legacy watchdog timer has expired or window
* watchdog timer either in second window or not in second window.
* This function is used for polled mode in legacy watchdog timer.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.

* @return
*		- TRUE, if the legacy watchdog timer has expired or window
*		watchdog timer is not in second window.
*		- FALSE if the legacy watchdog is not expired or window
*		watchdog is in second window.
*
* @note		None.
*
******************************************************************************/
u32 XWdtTb_IsWdtExpired(const XWdtTb *InstancePtr)
{
	u32 ControlStatusRegister0;
        u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Check Whether is Window WDT Mode enabled */
        if(InstancePtr->EnableWinMode == (u32)TRUE) {
		/* Read status control register and get second window value */
		ControlStatusRegister0 =
			(XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
				XWT_ESR_OFFSET) & XWT_ESR_WSW_MASK) >>
					XWT_ESR_WSW_SHIFT;
		if (ControlStatusRegister0 == (u32)TRUE) {
			Status = (u32)FALSE;
		} else {
			Status = (u32)TRUE;
		}
        }
        else {
		if (InstancePtr->Config.IsPl == (u32)0) {
               /* Read the current contents */
                ControlStatusRegister0 =XWdtTb_ReadReg (InstancePtr->Config.BaseAddr,XWT_GWCSR_OFFSET);
                /* Check whether state and reset status */
                if ((ControlStatusRegister0 & XWT_GWCSR_GWS2_MASK) != (u32)FALSE)
                {
                        Status = (u32)TRUE;
                }
                else {
                        Status = (u32)FALSE;
                }
		} else {
		/* Read the current contents */
		ControlStatusRegister0 =
			XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
				XWT_TWCSR0_OFFSET);
		/* The watchdog has expired if either of the bits are set */
		/* Check whether state and reset status */
		if ((ControlStatusRegister0 & (XWT_CSR0_WRS_MASK | XWT_CSR0_WDS_MASK)) != (u32)FALSE) {
			Status = (u32)TRUE;
		}
		else {
			Status = (u32)FALSE;
		}
		}
        }
	return Status;
}
/*****************************************************************************/
/**
* @brief
*
* This function checks if the Generic watchdog timer has First window expired
* or not.
*
*
* @param        InstancePtr is a pointer to the XWdtTb instance to be
*               worked on.

* @return
*               - TRUE, if the Generic watchdog timer First window has expired.
*               - FALSE if the Generic watchdog is not expired First window.
*
* @note         None.
*
******************************************************************************/
u32 XWdtTb_IsGenericWdtFWExpired(const XWdtTb *InstancePtr)
{
        u32 Status;
        u32 ControlStatusRegister0;

	Xil_AssertNonvoid(InstancePtr != NULL);

        /* Read the current contents */
        ControlStatusRegister0 =XWdtTb_ReadReg (InstancePtr->Config.BaseAddr,XWT_GWCSR_OFFSET);
        /* Check whether state and reset status */
        if ((ControlStatusRegister0 & XWT_GWCSR_GWS1_MASK) != (u32)FALSE)
        {
                Status = (u32)TRUE;
        }
        else
        {
                Status = (u32)FALSE;
        }
        return Status;
}

/*****************************************************************************/
/**
* @brief
*
* This function restarts the legacy or window watchdog timer. An application
* needs to call this function periodically to keep the timer from asserting
* the reset output.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XWdtTb_RestartWdt(const XWdtTb *InstancePtr)
{
	u32 ControlStatusRegister0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

        /* Check whether is Window WDT Mode enabled */
        if(InstancePtr->EnableWinMode == (u32)TRUE) {
		/* Read enable status register and update second window bit */
		ControlStatusRegister0 =
			XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
				XWT_ESR_OFFSET) | (u32)XWT_ESR_WSW_MASK;

		/* Write control status register to restart the timer */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET,
			ControlStatusRegister0);
	}
        else {
		if (InstancePtr->Config.IsPl == (u32)0) {
		/*  Read enable status register and update Refresh Register  bit */
		ControlStatusRegister0 =
				XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
						XWT_GWRR_OFFSET) | (u32)XWT_GWRR_MASK;
		/* Write control status register to restart the timer */
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_GWRR_OFFSET,
				ControlStatusRegister0);
		} else {
		/*
		 * Read the current contents of TCSR0 so that subsequent writes
		 * won't destroy any other bits.
		 */
		ControlStatusRegister0 =
			XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
				XWT_TWCSR0_OFFSET);
		/*
		 * Clear the bit that indicates the reason for the last
		 * system reset, WRS and the WDS bit, if set, by writing
		 * 1's to TCSR0
		 */
		ControlStatusRegister0 |= ((u32)XWT_CSR0_WRS_MASK |
			(u32)XWT_CSR0_WDS_MASK);

		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr,
			XWT_TWCSR0_OFFSET, ControlStatusRegister0);
		}
        }
}
/*****************************************************************************/
/**
* @brief
*
* This function keeps Window Watchdog Timer always enabled.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note		This must be called before Window WDT is enabled. Once Window
*		WDT is enabled, it can only be disabled by applying reset.
*
******************************************************************************/
void XWdtTb_AlwaysEnable(const XWdtTb *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

	/* Read master write control register and update always enable */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
		XWT_MWR_OFFSET) | XWT_MWR_AEN_MASK;

	/* Write master write control register to enable once feature */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_MWR_OFFSET,
		RegValue);
}
/*****************************************************************************/
/**
* @brief
*
* This function clears event(s) that present after system reset.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note		Event can be cleared by applying reset to the core followed
*		by writing 111. Writing any other pattern has no effect.
*
******************************************************************************/
void XWdtTb_ClearLastEvent(const XWdtTb *InstancePtr)
{
	u32 RegValue;
	u32 SecWindow;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read enable status register and update last bad event bits */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_ESR_OFFSET) | XWT_ESR_LBE_MASK;

	SecWindow = (RegValue & XWT_ESR_WSW_MASK) >> XWT_ESR_WSW_SHIFT;

	/*
	 * Check WDT in second window. If WDT is in second window, toggle WSW
	 * bit to avoid restart kick before clearing last bad events
	 */
	if (SecWindow == (u32)XWT_ONE) {
		RegValue &= ~((u32)XWT_ESR_WSW_MASK);
	}

	/* Write enable status register with updated events and WSW bit */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET,
		RegValue);
}

/*****************************************************************************/
/**
* @brief
*
* This function clears the window watchdog reset pending.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XWdtTb_ClearResetPending(const XWdtTb *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read enable status register and update reset bit */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_ESR_OFFSET) | XWT_ESR_WRP_MASK;

	/* Write enable status register with updated reset bit */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET,
		RegValue);
}

/*****************************************************************************/
/**
* @brief
*
* This function clears window watchdog timer interrupt (WINT) bit.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XWdtTb_IntrClear(const XWdtTb *InstancePtr)
{
	u32 RegValue;
	u32 SecWindow;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read enable status register and update WINT bit */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
		XWT_ESR_OFFSET) | XWT_ESR_WINT_MASK;

	SecWindow = (RegValue & XWT_ESR_WSW_MASK) >> XWT_ESR_WSW_SHIFT;

	/*
	 * Check WDT in second window. If WDT is in second window, toggle WSW
	 * bit to avoid restart kick before clearing interrupt programmed
	 * point
	 */
	if (SecWindow == (u32)XWT_ONE) {
		RegValue &= ~((u32)XWT_ESR_WSW_MASK);
	}

	/* Write enable status register with updated WINT and WSW bit */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET,
		RegValue);
}

/*****************************************************************************/
/**
* @brief
*
* This function sets byte count to determine the interrupt assertion point
* in the second window configuration.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
* @param	ByteCount specifies the selected byte count value to be set in
*		the second window configuration.
*
* @return	None.
*
* @note
*		- This function must be called before Window WDT start/enable
*		or after Window WDT stop/disable.
*		- This function must be used along with XWdtTb_SetByteSegment.
*
******************************************************************************/
void XWdtTb_SetByteCount(const XWdtTb *InstancePtr, u32 ByteCount)
{
	u32 RegValue;
	u32 Count;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_FCR_OFFSET);

	/* Reposition byte count */
	Count = (ByteCount << XWT_FCR_SBC_SHIFT) & XWT_FCR_SBC_MASK;
	RegValue |= Count;

	/* Write function control register with selected byte count */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET,
		RegValue);
}

/*****************************************************************************/
/**
* @brief
*
* This function provides byte count value of the selected byte count in the
* second window configuration.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	Byte count value of the selected byte count in the second
*		window configuration.
*
* @note		None.
*
******************************************************************************/
u32 XWdtTb_GetByteCount(const XWdtTb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read function control register and return selected byte count */
	return ((XWdtTb_ReadReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET) &
		XWT_FCR_SBC_MASK) >> XWT_FCR_SBC_SHIFT);
}

/*****************************************************************************/
/**
* @brief
*
* This function sets byte segment selection to determine the interrupt
* assertion point in the second window configuration.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
* @param	ByteSegment specifies the byte segment selected.
*		- 0 = Second window byte 0 selected.
*		- 1 = Second window byte 1 selected.
*		- 2 = Second window byte 2 selected.
*		- 3 = Second window byte 3 selected.
*
* @return	None.
*
* @note
*		- This function must be called before Window WDT start/enable
*		or after Window WDT stop/disable.
*		- This function must be used along with XWdtTb_SetByteCount.
*
******************************************************************************/
void XWdtTb_SetByteSegment(const XWdtTb *InstancePtr, u32 ByteSegment)
{
	u32 SegmentNum;
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);
        Xil_AssertVoid(ByteSegment < (u32)XWT_MAX_BYTE_SEGMENT);

	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_FCR_OFFSET);

	/* Reposition byte segment selection */
	SegmentNum = (ByteSegment << XWT_FCR_BSS_SHIFT) & XWT_FCR_BSS_MASK;
	RegValue |= SegmentNum;

	/* Write function control register with selected byte segment */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET,
		RegValue);
}

/*****************************************************************************/
/**
* @brief
*
* This function provides byte segment selection in the second window
* configuration.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	Byte segment selection value as
*		- 0 = Second window byte 0 selected.
*		- 1 = Second window byte 1 selected.
*		- 2 = Second window byte 2 selected.
*		- 3 = Second window byte 3 selected.
*
* @note		None.
*
******************************************************************************/
u32 XWdtTb_GetByteSegment(const XWdtTb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read function control register and return selected byte segment */
	return ((XWdtTb_ReadReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET) &
		XWT_FCR_BSS_MASK) >> XWT_FCR_BSS_SHIFT);
}

/*****************************************************************************/
/**
* @brief
*
* This function enables Second Sequence Timer (SST) function.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note
*		- This function must be called before Window WDT start/enable
*		or after Window WDT stop/disable.
*		- SST provides additional time to software by delaying the
*		inevitable window watchdog reset generation by SST count (SC)
*		delay. This is an independent function and can be enabled in
*		any mode w/ or w/o other options.
*
******************************************************************************/
void XWdtTb_EnableSst(const XWdtTb *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read function control register and update SSTE bit */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_FCR_OFFSET) | XWT_FCR_SSTE_MASK;

	/* Write function control register with updated SSTE value */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET,
		RegValue);
}

/*****************************************************************************/
/**
* @brief
*
* This function disables Second Sequence Timer (SST) function.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note		This function must be called before Window WDT start/enable or
*		after Window WDT stop/disable.
*
******************************************************************************/
void XWdtTb_DisableSst(const XWdtTb *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read function control register and update SSTE bit */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
		XWT_FCR_OFFSET) & (~XWT_FCR_SSTE_MASK);

	/* Write function control register with updated SSTE value */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET,
		RegValue);
}

/*****************************************************************************/
/**
* @brief
*
* This function enables Program Sequence Monitor (PSM) function.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note
*		- This function must be called before Window WDT start/enable
*		or after Window WDT stop/disable.
*		- PSM enables Task Signature Register comparison. When PSM is
*		enabled, core checks and compares the contents of TSR0 and TSR1
*		registers at the restart kick/disablement of Window WDT in
*		second window. If they match, no effect. If they do not match,
*		reset is generated either immediately when SST is disabled or
*		after SST count delay when SST is enabled.
*
******************************************************************************/
void XWdtTb_EnablePsm(const XWdtTb *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read function control register and update PSME bit */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_FCR_OFFSET) | XWT_FCR_PSME_MASK;

	/* Write function control register with updated PSME value */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET,
		RegValue);
}

/*****************************************************************************/
/**
* @brief
*
* This function disables Program Sequence Monitor (PSM) function.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note		This function must be called before Window WDT start/enable or
*		after Window WDT stop/disable.
*
******************************************************************************/
void XWdtTb_DisablePsm(const XWdtTb *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read function control register and update PSME bit */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
		XWT_FCR_OFFSET) & (~XWT_FCR_PSME_MASK);

	/* Write function control register with updated PSME value */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET,
		RegValue);
}

/*****************************************************************************/
/**
* @brief
*
* This function enables Fail Counter (FC) function.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note
*		- This function must be called before Window WDT start/enable
*		or after Window WDT stop/disable.
*		- When fail counter is enabled, reset is generated when fail
*		counter is 7 and another bad event happens.
*		- When fail counter is disabled, one bad event triggers reset
*		either immediately when SST is disabled or after SST count
*		delay when SST is enabled.
*
******************************************************************************/
void XWdtTb_EnableFailCounter(XWdtTb *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read function control register and update FCE bit */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_FCR_OFFSET) | XWT_FCR_FCE_MASK;

	/* Write function control register with updated FCE value */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET,
		RegValue);

	InstancePtr->EnableFailCounter = (u32)1;
}

/*****************************************************************************/
/**
* @brief
*
* This function disables Fail Counter (FC) function.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note		This function must be called before Window WDT start/enable or
*		after Window WDT stop/disable.
*
******************************************************************************/
void XWdtTb_DisableFailCounter(XWdtTb *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read function control register and update FCE bit */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
		XWT_FCR_OFFSET) & (~XWT_FCR_FCE_MASK);

	/* Write function control register with updated FCE value */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET,
		RegValue);

	InstancePtr->EnableFailCounter = (u32)0;
}

/*****************************************************************************/
/**
* @brief
*
* This function provides extra safeguard against unintentional clear of WEN
* bit.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note		This function must be called before Window WDT start/enable or
*		after Window WDT stop/disable.
*
******************************************************************************/
void XWdtTb_EnableExtraProtection(const XWdtTb *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read function control register and update WDP bit */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_FCR_OFFSET) | XWT_FCR_WDP_MASK;

	/* Write function control register with updated WDP value */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET,
		RegValue);
}

/*****************************************************************************/
/**
* @brief
*
* This function enables unintentional clear of WEN bit.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	None.
*
* @note		This function must be called before Window WDT start/enable or
*		after Window WDT stop/disable.
*
******************************************************************************/
void XWdtTb_DisableExtraProtection(const XWdtTb *InstancePtr)
{
	u32 RegValue;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Read function control register and update WDP bit */
	RegValue = XWdtTb_ReadReg(InstancePtr->Config.BaseAddr,
			XWT_FCR_OFFSET) & (~XWT_FCR_WDP_MASK);

	/* Write function control register with updated WDP value */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FCR_OFFSET,
		RegValue);
}

/*****************************************************************************/
/**
* @brief
*
* This function sets the count value for the first and second window.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
* @param	FirstWinCount specifies the first window count value.
* @param	SecondWinCount specifies the second window count value.
*
* @return	None.
*
* @note
*		This function must be called before Window WDT start/enable
*		or after Window WDT stop/disable.
*		- For first window, it is recommended that minimum non-zero
*		value should be 15 or more.
*		- For second window, minimum value should be sufficiently large
*		to complete required AXILite write transactions at system
*		level.
*		- Setting second window count value to zero causes wrong
*		configuration and disables Window WDT feature by clearing
*		WEN bit irrespective of WDP settings.
*
******************************************************************************/
void XWdtTb_SetWindowCount(const XWdtTb *InstancePtr, u32 FirstWinCount,
				u32 SecondWinCount)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /* Write first window count value */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_FWR_OFFSET,
		FirstWinCount);

	/* Write second window count value */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_SWR_OFFSET,
		SecondWinCount);
}
/*****************************************************************************/
/**
* @brief
*
* This function sets the count value for the GWDT_Compare_value_reg0 ,GWDT_Compare_value_reg1 &
* GWDT_Offset_regs .
*
* @param     InstancePtr is a pointer to the XWdtTb instance to be
*            worked on.
* @param     GWOR_config    specifies the GWDT_Offset_reg count value.
* @return    None.
*
* @note
*            This function must be called before Window WDT start/enable
*            or after Window WDT stop/disable.
*
******************************************************************************/
void XWdtTb_SetGenericWdtWindow(const XWdtTb *InstancePtr, u32 GWOR_config)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	/* Write GWDT_Offset_reg count value*/
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr,XWT_GWOR_OFFSET,GWOR_config);
}

/*****************************************************************************/
/**
* @brief
*
* This function programs the width of Watchdog Timer.
*
* @param	InstancePtr - InstancePtr is a pointer to the XWdtTb instance to be
*		    worked on.
* @param	width - width of the Watchdog Timer.
*
* @return
*		- XST_SUCESS, if window mode is disabled and the width is
*		  programmed correctly.
*		- XST_FAILURE, if Window mode is enabled or if the width is
*                  not in the range of 8-31
*
* @note
*		- This function is applicable only when the window mode is
*		  disabled.
*		- This function should be called before starting the timer.
*		  Valid values for the width are 8-31. Programming any other
*		  value returns failure.
*
******************************************************************************/
u32 XWdtTb_ProgramWDTWidth(const XWdtTb *InstancePtr, u32 width)
{
        u32 Status;
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
        /* Programming width of the Watchdog Timer, width values in between 8 to 31*/
	if((InstancePtr->EnableWinMode == ((u32)FALSE)) && ((width>=(u32)XWT_START_VALUE) && (width<=(u32)XWT_END_VALUE)))
	{
		XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_MWR_OFFSET, width);
                Status =(u32)XST_SUCCESS;
	}
	else
        {
                Status =(u32)XST_FAILURE;
        }
        return Status;
}
/** @} */
