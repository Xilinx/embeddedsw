/******************************************************************************
* Copyright (C) 2001 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xwdttb.h
* @addtogroup wdttb_v5_3
* @{
* @details
*
* The Xilinx watchdog timer/timebase component supports the Xilinx legacy
* watchdog timer/timebase and window watchdog timer hardware. More detailed
* description of the driver operation for each function can be found in the
* xwdttb.c file.
*
* The Xilinx watchdog timer/timebase driver supports both legacy and window
* features:
* Features in legacy watchdog timer:
*   - Polled mode
*   - enabling and disabling (if allowed by the hardware) the watchdog timer
*   - restarting the watchdog.
*   - reading the timebase.
*
* Features in window watchdog timer:
*   - Configurable close and open window periods.
*   - Enabling and disabling Fail Counter (FC).
*   - Enabling and disabling Program Sequence Monitor (PSM).
*   - Enabling and disabling Second Sequence Timer (SST).
*   - Setting interrupt assertion point in second window.
*   - Controlling the write access to the complete address space.
*   - Always enable.
*
* The window watchdog timer always enable feature enables watchdog timer
* forever. It can only be disabled by applying the reset to the processor or
* core.
*
* It is the responsibility of the application to provide an interrupt handler
* for the timebase and the watchdog and connect them to the interrupt
* system if interrupt driven mode is desired.
*
* The legacy watchdog timer/timebase component ALWAYS generates an interrupt
* output when:
*   - the watchdog expires the first time
*   - the timebase rolls over
* and ALWAYS generates a reset output when the watchdog timer expires a second
* time. This is not configurable in any way from the software driver's
* perspective.
*
* The window watchdog timer asserts an interrupt when
*   - the watchdog reaches at the interrupt programmed point in second window
* and ALWAYS generates reset output
*   - when single bad event occur if fail count disable,
*   - if fail counter is 7 and bad event happens.
*
* The Timebase is reset to 0 when the Watchdog Timer is enabled.
*
* If the hardware interrupt signal is not connected, polled mode is the only
* option (using IsWdtExpired) for the legacy watchdog and GetIntrStatus option
* for the window watchdog. Reset output will occur for the second watchdog
* timeout regardless. Polled mode for the timebase rollover is just reading
* the contents of the register and seeing if the MSB has transitioned from 1
* to 0.
*
* The IsWdtExpired function is used for polling the watchdog timebase timer
* and it is also used to check if the watchdog was the cause of the last reset.
* In this situation, call Initialize then call IsWdtExpired. If the result is
* true watchdog timeout caused the last system reset. It is then acceptable to
* further initialize the component which will reset this bit.
*
* The XWdtTb_GetIntrStatus is used for polling the window watchdog timer and it
* is used to check if interrupt programmed point has reached.
*
* This driver is intended to be RTOS and processor independent. It works with
* physical addresses only. Any needs for dynamic memory management, threads
* or thread mutual exclusion, virtual memory, or cache control must be
* satisfied by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a ecm  08/16/01 First release
* 1.00b jhl  02/21/02 Repartitioned driver for smaller files
* 1.00b rpm  04/26/02 Made LookupConfig public and added XWdtTb_Config
* 1.10b mta  03/23/07 Updated to new coding style
* 1.11a sdm  08/22/08 Removed support for static interrupt handlers from the
*		      MDD file
* 2.00a ktn  22/10/09 The driver is updated to use HAL processor APIs/macros.
*		      The following macros defined in xwdttb_l.h file have been
*		      removed - XWdtTb_mEnableWdt, XWdtTb_mDisbleWdt,
*		      XWdtTb_mRestartWdt, XWdtTb_mGetTimebaseReg and
*		      XWdtTb_mHasReset.
*		      Added the XWdtTb_ReadReg and XWdtTb_WriteReg
*		      macros. User should XWdtTb_ReadReg/XWdtTb_WriteReg to
*		      achieve the desired functionality of the macros that
*		      were removed.
* 3.0   adk  19/12/13 Updated as per the New Tcl API's
* 3.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
* 4.0   sha  12/17/15 Added Window WDT feature with basic mode.
*                     Changed XWdtTb_GetTbValue to inline function.
*                     Adherence to MISRA-C guidelines.
* 4.0   sha  01/29/16 Added XWdtTb_Event enum type.
*                     Updated XWdtTb_Config structure with Window WDT
*                     parameters.
*                     Updated XWdtTb core structure with config parameter and
*                     removed RegBaseAddress parameter.
*
*                     Added following static inline functions:
*                     XWdtTb_GetTbValue, XWdtTb_SetRegSpaceAccessMode,
*                     XWdtTb_GetRegSpaceAccessMode, XWdtTb_GetLastEvent,
*                     XWdtTb_GetFailCounter, XWdtTb_IsResetPending,
*                     XWdtTb_GetIntrStatus, XWdtTb_IsWrongCfg.
*
*                     Added following functions:
*                     XWdtTb_AlwaysEnable, XWdtTb_ClearLastEvent,
*                     XWdtTb_ClearResetPending, XWdtTb_IntrClear,
*                     XWdtTb_SetByteCount, XWdtTb_GetByteCount,
*                     XWdtTb_SetByteSegment, XWdtTb_GetByteSegment,
*                     XWdtTb_EnableSst, XWdtTb_DisableSst, XWdtTb_EnablePsm,
*                     XWdtTb_DisablePsm, XWdtTb_EnableFailCounter,
*                     XWdtTb_DisableFailCounter, XWdtTb_EnableExtraProtection,
*                     XWdtTb_DisableExtraProtection, XWdtTb_SetWindowCount,
*                     XWdtTb_CfgInitialize.
* 4.0   sha  02/17/16 Removed 3.1 version and added CR# 867425 change in
*                     4.0 version.
* 4.1   adk  23/12/16 Fix race conition in the tcl CR#966068
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
* 4.2   ms   04/18/17 Modified tcl file to add suffix U for all macros
*                     definitions of wdttb in xparameters.h
* 4.3   srm  01/27/18 Added XWdtTb_ProgramWDTWidth to pragram the WDT width
*            01/30/18 Added doxygen tags
* 4.4   aru  11/15/18 Replaced "Xil_AssertVoid" as "Xil_AssertNonvoid"
*                     in XWdtTb_ProgramWDTWidth().
* 4.4   sne  03/01/19 Fixed violations according to MISRAC-2012 standards
*                     modified the code for below violations,
*                     No brackets to then/else,
*                     Literal value requires a U suffix,Function return
*                     type inconsistent,Logical conjunctions need brackets,
*                     Declared the pointer param as Pointer to const,
*                     Procedure has more than one exit point.
* 4.4   sne  03/04/19 Added support for Versal ( Generic Watchdog and
*                     Window Watchdog timer).
*                     Added following functions:
*                     XWdtTb_IsGenericWdtFWExpired, XWdtTb_SetSSTWindow
*                     XWdtTb_SetGenericWdtWindow.
* 4.5   nsk  08/07/19 Updated testapp tcl to generate polled mode
*                     example, when Wdttb interrupt pin is not connected
*                     CR# 1035919.
* 4.5   nsk  08/07/19 Fixed the warnings while generating test app
* 4.5   sne  06/25/19 Fixed Coverity warning.
* 4.5   sne  09/27/19 Updated Tcl file for WWDT & AXI Timebase WDT IP.
*		      Updated driver to support for WWDT and AXI Timebase WDT.
*		      While accessing AXI Timebase WDT appending "C" to base
*		      address for getting AXI Watchdog offsets.
* 5.0	sne  12/30/19 Updated example files with "Successfully ran"string.CR#1050724
* 5.0	sne  01/31/20 Removed compare value registers write while configuring Generic
*		      watchdog window timer.CR#1052544
* 5.0	sne  02/27/20 Reorganize the driver source and Fixed doxygen warnings.
*		      Added XWdtTb_ConfigureWDTMode function.
* 5.1	sne  05/04/20 Fixed violations according to MISRAC-2012 standards.
*
* </pre>
*
******************************************************************************/

#ifndef XWDTTB_H		/* prevent circular inclusions */
#define XWDTTB_H		/**< by using protection macros */

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xwdttb_l.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/

/**
 * This typedef contains enumeration of different events in basic mode.
 */
typedef enum {
	XWDTTB_NO_BAD_EVENT = 0,	/**< No bad event */
	XWDTTB_RS_KICK_EVENT,		/**< Restart kick or disable attempt in
					  *  first window */
	XWDTTB_TSR_MM_EVENT,		/**< TSR mismatch */
	XWDTTB_SEC_WIN_EVENT		/**< Second window overflow */
} XWdtTb_Event;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID of the device */
	UINTPTR BaseAddr;	/**< Base address of the device */
	u32 EnableWinWdt;	/**< Flag for Window WDT enable */
	u32 MaxCountWidth;	/**< Maximum width of first timer */
	u32 SstCountWidth;	/**< Maximum width of Second Sequence Timer */
	u32 IsPl;		/**< IsPl, 1= AXI Timebase ,0= WWDT  */
} XWdtTb_Config;

/**
 * The XWdtTb driver instance data. The user is required to allocate a
 * variable of this type for every watchdog/timer device in the system.
 * A pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	XWdtTb_Config Config;	/**< Hardware Configuration */
	u32 IsReady;		/**< Device is initialized and ready */
	u32 IsStarted;		/**< Device watchdog timer is running */
	u32 EnableFailCounter;	/**< Fail counter, 0 = Disable, 1 = Enable */
       u32 EnableWinMode;      /**<Enable Window WDT Method,0= DIsable,1=Enable*/
} XWdtTb;

/************************** Variable Definitions *****************************/
extern XWdtTb_Config XWdtTb_ConfigTable[];	/**< Configuration table */

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
* @brief
*
* This function returns the current contents of the timebase.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	The contents of the timebase.
*
* @note		None.
*
******************************************************************************/
static inline u32 XWdtTb_GetTbValue(const XWdtTb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->EnableWinMode == 0U);

	/* Return the contents of the timebase register */
	return XWdtTb_ReadReg(InstancePtr->Config.BaseAddr, XWT_TBR_OFFSET);
}
/*****************************************************************************/
/**
* @brief
*
* This function controls the read/write access to the complete Window WDT
* register space.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
* @param	AccessMode specifies a value that needs to be used to provide
*		read/write access to the register space.
*		- 1 = Window WDT register space is writable.
*		- 0 = Window WDT register space is read only.
*
* @return	None.
*
* @note		When register space is set read only, writes to any register
*		are ignored and does not lead to good or bad event generation.
*
******************************************************************************/
static inline void XWdtTb_SetRegSpaceAccessMode(const XWdtTb *InstancePtr,
						u32 AccessMode)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->EnableWinMode == 1U);
	Xil_AssertVoid((AccessMode == 0U) || (AccessMode == 1U));

	/* Write access mode */
	XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_MWR_OFFSET,
		AccessMode);
}

/*****************************************************************************/
/**
* @brief
*
* This function provides Window WDT register space read only or writable.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return
*		- 1 = Window WDT register space is writable.
*		- 0 = Window WDT register space is read only.
*
* @note		None.
*
******************************************************************************/
static inline u32 XWdtTb_GetRegSpaceAccessMode(const XWdtTb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->EnableWinMode == 1U);

	/* Read master write control register and return register space read
	 * only or writable
	 */
	return (XWdtTb_ReadReg(InstancePtr->Config.BaseAddr, XWT_MWR_OFFSET) &
		XWT_MWR_MWC_MASK);
}

/*****************************************************************************/
/**
* @brief
*
* This function provides the last bad event and read even after system reset.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	One of the enumeration value described in XWdtTb_Event.
*
* @note		Event can be read after a system reset to determine if the
*		reset was caused by a watchdog timeout.
*
******************************************************************************/
static inline u32 XWdtTb_GetLastEvent(const XWdtTb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->EnableWinMode == 1U);

	/* Read enable status register and return last bad event(s) */
	return ((XWdtTb_ReadReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET) &
		XWT_ESR_LBE_MASK) >> XWT_ESR_LBE_SHIFT);
}

/*****************************************************************************/
/**
* @brief
*
* This function provides fail count value.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return	Fail counter value. The default value is 5.
*
* @note
*		- When fail counter is enabled, it gets incremented for every
*		bad event unless 7 or decrement ed for every good event unless
*		0. If fail counter is 7 and another bad event happens, reset is
*		generated.
*		- When fail counter is disabled, bad event leads to reset.
*
******************************************************************************/
static inline u32 XWdtTb_GetFailCounter(const XWdtTb *InstancePtr)
{

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->EnableWinMode == 1U);

	/* Read enable status register and return fail counter value */
	return ((XWdtTb_ReadReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET) &
		XWT_ESR_FCV_MASK) >> XWT_ESR_FCV_SHIFT);
}

/*****************************************************************************/
/**
* @brief
*
* This function states that whether the reset is pending or not when Secondary
* Sequence Timer(SST) counter has started.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return
*		- 1 = window watchdog reset is pending.
*		- 0 = window watchdog reset is not pending.
*
* @note		None.
*
******************************************************************************/
static inline u32 XWdtTb_IsResetPending(const XWdtTb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->EnableWinMode == 1U);

	/* Read enable status register and return reset pending bit */
	return ((XWdtTb_ReadReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET) &
		XWT_ESR_WRP_MASK) >> XWT_ESR_WRP_SHIFT);
}

/*****************************************************************************/
/**
* @brief
*
* This function states that whether window watchdog timer has reached at the
* interrupt programmed point in second window.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return
*		- 1 = when window watchdog timer has reached at the interrupt
*		programmed point in second window.
*		- 0 = when window watchdog timer did not reach at the
*		 interrupt programmed point in second window.
*
* @note		None.
*
******************************************************************************/
static inline u32 XWdtTb_GetIntrStatus(const XWdtTb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->EnableWinMode == 1U);

	/* Read enable status register and return interrupt status */
	return ((XWdtTb_ReadReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET) &
		XWT_ESR_WINT_MASK) >> XWT_ESR_WINT_SHIFT);
}

/*****************************************************************************/
/**
* @brief
*
* This function states wrong configuration when second window count is set to
* zero.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be
*		worked on.
*
* @return
*		- 1 = if second window count is set to zero.
*		- 0 = if second window count is not set to zero.
*
* @note		None.
*
******************************************************************************/
static inline u32 XWdtTb_IsWrongCfg(const XWdtTb *InstancePtr)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->EnableWinMode == 1U);

	/* Read enable status register and return wrong configuration value */
	return ((XWdtTb_ReadReg(InstancePtr->Config.BaseAddr, XWT_ESR_OFFSET) &
		XWT_ESR_WCFG_MASK) >> XWT_ESR_WCFG_SHIFT);
}

/*****************************************************************************/
/**
* @brief
*
* This function sets the count value for the  Second Sequence Timer window.
*
* @param     InstancePtr is a pointer to the XWdtTb instance to be
*            worked on.
* @param     SST_window_config specifies the first window count value.
*
* @return    None.
*
* @note
*            This function must be called before Window WDT start/enable
*            or after Window WDT stop/disable.
*            - For SSTWR window, This option provides additional time to software
*              by delaying the inevitable wwdt_reset assertion/generation by
*              SC count delay
*
******************************************************************************/

static inline void XWdtTb_SetSSTWindow(const XWdtTb *InstancePtr, u32 SST_window_config)
{
        /* Verify arguments. */
        Xil_AssertVoid(InstancePtr != NULL);
        Xil_AssertVoid(InstancePtr->EnableWinMode == (u32)TRUE);

        /*  Write SST window count value */
        XWdtTb_WriteReg(InstancePtr->Config.BaseAddr, XWT_SSTWR_OFFSET,SST_window_config);
}

/*****************************************************************************/
/**
* @brief
*
* This function set configure WDT mode.
*
* @param     InstancePtr is a pointer to the XWdtTb instance to be
*            worked on.
* @param     Mode specifies the GWDT or WWDT. 0 for GWDT and 1 for WWDT.
*
* @return    -XST_SUCESS, if Mode with in the range.
*			 -XST_FAILURE,if Mode is out side of range.
*
******************************************************************************/

static inline u32 XWdtTb_ConfigureWDTMode(XWdtTb *InstancePtr, u32 Mode)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	if ((Mode == (u32) 0) || (Mode == (u32)1)) {
		InstancePtr->EnableWinMode = Mode;
		return XST_SUCCESS;
	} else {
		return XST_FAILURE;
	}
}

/************************** Function Prototypes ******************************/

/*
 * Required functions in xwdttb.c
 */
 s32 XWdtTb_CfgInitialize(XWdtTb *InstancePtr, const XWdtTb_Config *CfgPtr,
				UINTPTR EffectiveAddr);

s32 XWdtTb_Initialize(XWdtTb *InstancePtr, u16 DeviceId);

void XWdtTb_Start(XWdtTb *InstancePtr);

s32 XWdtTb_Stop(XWdtTb *InstancePtr);

u32 XWdtTb_IsWdtExpired(const XWdtTb *InstancePtr);
u32 XWdtTb_IsGenericWdtFWExpired(const XWdtTb *InstancePtr);

void XWdtTb_RestartWdt(const XWdtTb *InstancePtr);

XWdtTb_Config *XWdtTb_LookupConfig(u16 DeviceId);

/* Window WDT functions implemented in xwdttb.c */
void XWdtTb_AlwaysEnable(const XWdtTb *InstancePtr);
void XWdtTb_ClearLastEvent(const XWdtTb *InstancePtr);
void XWdtTb_ClearResetPending(const XWdtTb *InstancePtr);
void XWdtTb_IntrClear(const XWdtTb *InstancePtr);

void XWdtTb_SetByteCount(const XWdtTb *InstancePtr, u32 ByteCount);
u32 XWdtTb_GetByteCount(const XWdtTb *InstancePtr);
void XWdtTb_SetByteSegment(const XWdtTb *InstancePtr, u32 ByteSegment);
u32 XWdtTb_GetByteSegment(const XWdtTb *InstancePtr);
void XWdtTb_EnableSst(const XWdtTb *InstancePtr);
void XWdtTb_DisableSst(const XWdtTb *InstancePtr);
void XWdtTb_EnablePsm(const XWdtTb *InstancePtr);
void XWdtTb_DisablePsm(const XWdtTb *InstancePtr);
void XWdtTb_EnableFailCounter(XWdtTb *InstancePtr);
void XWdtTb_DisableFailCounter(XWdtTb *InstancePtr);
void XWdtTb_EnableExtraProtection(const XWdtTb *InstancePtr);
void XWdtTb_DisableExtraProtection(const XWdtTb *InstancePtr);

void XWdtTb_SetWindowCount(const XWdtTb *InstancePtr, u32 FirstWinCount,
				u32 SecondWinCount);
void XWdtTb_SetGenericWdtWindow(const XWdtTb *InstancePtr, u32 GWOR_config);
u32 XWdtTb_ProgramWDTWidth(const XWdtTb *InstancePtr, u32 width);

/*
 * Self-test functions in xwdttb_selftest.c
 */
s32 XWdtTb_SelfTest(const XWdtTb *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
