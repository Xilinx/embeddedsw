/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XSCUWDT_H
#define XSCUWDT_H

/****************************************************************************/
/**
*
* @file xscuwdt.h
* @addtogroup Overview
* @{
* @brief Defines the interface of the XScuWdt SCU watchdog timer driver.
*
* @details
*
* The Xilinx SCU watchdog timer driver (XScuWdt) supports the Xilinx SCU private
* watchdog timer hardware.
*
* The XScuWdt driver supports the following features:
* - Watchdog mode
* - Timer mode
* - Auto reload (timer mode only)
*
* The watchdog counter register is a down counter and starts decrementing when
* the watchdog is started.
*
* In watchdog mode, when the counter reaches 0, the Reset flag is set in the
* Reset status register. The WDRESETREQ pin is then asserted, causing a system
* reset. The Reset flag is not reset by normal processor reset and is cleared
* when written with a value of 1. This enables the application to differentiate
* a normal reset and a reset caused by watchdog time-out. The application must
* call XScuWdt_RestartWdt() periodically to keep the watchdog from timing out.
*
* Use the IsWdtExpired function to check if the watchdog was the cause
* of the last reset. In this situation, call Initialize then call IsWdtExpired.
* If the result is true, watchdog timeout caused the last system reset. The
* application then needs to clear the Reset flag.
*
* In timer mode, when the counter reaches 0, the Event flag is set in the
* Interrupt status register and if interrupts are enabled, interrupt ID 30 is
* set as pending in the interrupt distributor. The IsTimerExpired function
* is used to check if the watchdog counter has decremented to 0 in timer mode.
* If auto-reload mode is enabled, the Counter register is automatically reloaded
* from the Load register.
*
* <b> Initialization and Configuration </b>
*
* The device driver enables higher layer software (for example, an application)
* to communicate with the Watchdog Timer.
*
* The XScuWdt_CfgInitialize() API initializes the Watchdog Timer. First call
* the XScuWdt_LookupConfig() API, which returns the Configuration structure
* pointer. Pass this pointer as a parameter to the XScuWdt_CfgInitialize() API.
*
* <b>Interrupts</b>
*
* The SCU Watchdog Timer supports interrupts in Timer mode.
*
* This driver does not provide an Interrupt Service Routine (ISR) for the
* device. The application is responsible for providing one if needed. Refer
* to the interrupt example provided with this driver for details on using the
* Timer in interrupt mode.
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*
* <b> Asserts </b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier. By default, asserts are turned on. Leave
* asserts on during development.
*
* <b> Building the driver </b>
*
* The XScuWdt driver is composed of several source files. This allows building
* and linking only the parts of the driver that are necessary.
*
* <br><br>
*
* NOTE:
* The watchdog timer is not a part of the snoop control unit as indicated
* by the prefix "scu" in the name of the driver.
* It is an independent module in APU.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- ---------------------------------------------
* 1.00a sdm 01/15/10 First release
* 1.02a  sg 07/17/12 Included xil_assert.h for CR 667947. This is an issue
*		     when the xstatus.h in the common driver overwrites
*		     the xstatus.h of the standalone BSP during the
*		     libgen.
* 2.1 	sk  02/26/15 Modified the code for MISRA-C:2012 compliance.
*       ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                    generation.
* 2.3   mus 08/31/20 Updated makefile to support parallel make and
*                    incremental builds. It would help to reduce compilation
*                    time.
* 2.3	sne 09/16/20 Fixed MISRA-C violations.
* 2.4	sne 02/04/21 Fixed Doxygen warnings.
* 2.5   asa 07/18/23 Added support for system device tree based workflow
*                    decoupling flow.
* 2.7   vlt  12/29/25 Update Doxygen comments to include SDT flow details.
*       vlt  01/27/26 Fixed codespell issues.
* 2.8   vn   09/30/26 Align Doxygen comments with AMD standards
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xscuwdt_hw.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * @brief Holds configuration information for the device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID of device. */
#else
	char *Name;		/**< Unique name of the device. */
#endif
	UINTPTR BaseAddr;	/**< Register base address. */
#ifdef SDT
	u32 IntrId;             /**< Bits[11:0] Interrupt-id Bits[15:12]
				 * trigger type and level flags. */
	UINTPTR IntrParent;     /**< Bit[0] Interrupt parent type Bit[64/32:1]
				 * Parent base address. */
#endif
} XScuWdt_Config;

/**
 * @brief Stores the XScuWdt driver instance data.
 *
 * Allocate a variable of this type for every watchdog/timer device in the
 * system. A pointer to a variable of this type is then passed to the driver
 * API functions.
 */
typedef struct {
	XScuWdt_Config Config;	/**< Hardware configuration. */
	u32 IsReady;		/**< Device is initialized and ready. */
	u32 IsStarted;		/**< Device watchdog timer is running. */
} XScuWdt;

/************************** Variable Definitions *****************************/
/**
 * @brief Holds the configuration entry for each SCU watchdog timer instance.
 */
extern XScuWdt_Config XScuWdt_ConfigTable[];

/***************** Macros (Inline Functions) Definitions *********************/
/****************************************************************************/
/**
*
* @brief Checks if the watchdog has timed out and caused the last reset.
*
* @param[in]	InstancePtr Pointer to the XScuWdt instance.
*
* @return
*		- TRUE if the watchdog has expired.
*		- FALSE if the watchdog has not expired.
*
* @note		C-style signature:
*		int XScuWdt_IsWdtExpired(XScuWdt *InstancePtr)
*
******************************************************************************/
#define XScuWdt_IsWdtExpired(InstancePtr)				\
	((XScuWdt_ReadReg((InstancePtr)->Config.BaseAddr,		\
			  XSCUWDT_RST_STS_OFFSET) &			\
	  XSCUWDT_RST_STS_RESET_FLAG_MASK) == XSCUWDT_RST_STS_RESET_FLAG_MASK)

/****************************************************************************/
/**
*
* @brief Checks if the watchdog counter has reached 0 in timer mode.
*
* @param[in]	InstancePtr Pointer to the XScuWdt instance.
*
* @return
*		- TRUE if the watchdog has expired.
*		- FALSE if the watchdog has not expired.
*
* @note		C-style signature:
*		int XScuWdt_IsTimerExpired(XScuWdt *InstancePtr)
*
******************************************************************************/
#define XScuWdt_IsTimerExpired(InstancePtr)				\
	((XScuWdt_ReadReg((InstancePtr)->Config.BaseAddr,		\
			  XSCUWDT_ISR_OFFSET) &				\
	  XSCUWDT_ISR_EVENT_FLAG_MASK) == XSCUWDT_ISR_EVENT_FLAG_MASK)

/****************************************************************************/
/**
*
* @brief Restarts the watchdog timer.
*
* This macro reads the watchdog load register and writes the same value to the
* load register to update the counter register. An application must call this
* function periodically to keep the watchdog from asserting the WDRESETREQ
* reset request output pin.
*
* @param[in]	InstancePtr Pointer to the XScuWdt instance.
*
* @note		C-style signature:
*		void XScuWdt_RestartWdt(XScuWdt *InstancePtr)
*
******************************************************************************/
#define XScuWdt_RestartWdt(InstancePtr)					 \
	XScuWdt_LoadWdt((InstancePtr),					 \
			(XScuWdt_ReadReg((InstancePtr)->Config.BaseAddr, \
					 XSCUWDT_LOAD_OFFSET)))

/****************************************************************************/
/**
*
* @brief Writes to the watchdog timer load register.
*
* This also updates the watchdog counter register with the new value. Use this
* macro to change the time-out value.
*
* @param[in]	InstancePtr Pointer to the XScuWdt instance.
* @param[in]	Value Value to write to the Watchdog Load register.
*
* @note		C-style signature:
*		void XScuWdt_LoadWdt(XScuWdt *InstancePtr, u32 Value)
*
******************************************************************************/
#define XScuWdt_LoadWdt(InstancePtr, Value)				\
	XScuWdt_WriteReg((InstancePtr)->Config.BaseAddr,		\
			 XSCUWDT_LOAD_OFFSET, (Value))

/****************************************************************************/
/**
*
* @brief Puts the watchdog timer in Watchdog mode.
*
* This macro sets the WD mode bit of the Watchdog control register.
*
* @param[in]	InstancePtr Pointer to the XScuWdt instance.
*
* @note		C-style signature:
*		void XScuWdt_SetWdMode(XScuWdt *InstancePtr)
*
******************************************************************************/
#define XScuWdt_SetWdMode(InstancePtr)					  \
	XScuWdt_WriteReg((InstancePtr)->Config.BaseAddr,		  \
			 XSCUWDT_CONTROL_OFFSET,			  \
			 (XScuWdt_ReadReg((InstancePtr)->Config.BaseAddr, \
					  XSCUWDT_CONTROL_OFFSET) |			  \
			  (XSCUWDT_CONTROL_WD_MODE_MASK)))

/****************************************************************************/
/**
*
* @brief Puts the watchdog timer in Timer mode.
*
* This macro writes 0x12345678 and 0x87654321 successively to the Watchdog
* Disable Register.
* The software must write 0x12345678 and 0x87654321 successively to the
* Watchdog Disable Register so that the watchdog mode bit in the Watchdog
* Control Register is set to zero.
*
* @param[in]	InstancePtr Pointer to the XScuWdt instance.
*
* @note		C-style signature:
*		void XScuWdt_SetTimerMode(XScuWdt *InstancePtr)
*
******************************************************************************/
#define XScuWdt_SetTimerMode(InstancePtr)				\
	{									\
		XScuWdt_WriteReg((InstancePtr)->Config.BaseAddr,		\
				 XSCUWDT_DISABLE_OFFSET,				\
				 XSCUWDT_DISABLE_VALUE1);			\
		XScuWdt_WriteReg((InstancePtr)->Config.BaseAddr,		\
				 XSCUWDT_DISABLE_OFFSET,				\
				 XSCUWDT_DISABLE_VALUE2);			\
	}

/****************************************************************************/
/**
*
* @brief Gets the contents of the watchdog control register.
*
* @param[in]	InstancePtr Pointer to the XScuWdt instance.
*
* @return	Contents of the watchdog control register.
*
* @note		C-style signature:
*		u32 XScuWdt_GetControlReg(XScuWdt *InstancePtr)
*
******************************************************************************/
#define XScuWdt_GetControlReg(InstancePtr)				\
	XScuWdt_ReadReg((InstancePtr)->Config.BaseAddr,			\
			XSCUWDT_CONTROL_OFFSET)

/****************************************************************************/
/**
*
* @brief Writes to the watchdog control register.
*
* @param[in]	InstancePtr Pointer to the XScuWdt instance.
* @param[in]	ControlReg Value to write to the watchdog control register.
*
* @note		C-style signature:
*		void XScuWdt_SetControlReg(XScuWdt *InstancePtr, u32 ControlReg)
*
******************************************************************************/
#define XScuWdt_SetControlReg(InstancePtr, ControlReg)			\
	XScuWdt_WriteReg((InstancePtr)->Config.BaseAddr,		\
			 XSCUWDT_CONTROL_OFFSET, (ControlReg))

/****************************************************************************/
/**
*
* @brief Enables auto-reload mode.
*
* @param[in]	InstancePtr Pointer to the XScuWdt instance.
*
* @note		C-style signature:
*		void XScuWdt_EnableAutoReload(XScuWdt *InstancePtr)
*
******************************************************************************/
#define XScuWdt_EnableAutoReload(InstancePtr)				\
	XScuWdt_SetControlReg((InstancePtr),				\
			      (XScuWdt_GetControlReg(InstancePtr) |	\
			       XSCUWDT_CONTROL_AUTO_RELOAD_MASK))

/************************** Function Prototypes ******************************/

/*
 * Lookup configuration in xscuwdt_sinit.c.
 */
#ifndef SDT
XScuWdt_Config *XScuWdt_LookupConfig(u16 DeviceId);
#else
XScuWdt_Config *XScuWdt_LookupConfig(UINTPTR BaseAddress);
#endif

/*
 * Selftest function in xscuwdt_selftest.c
 */
s32 XScuWdt_SelfTest(XScuWdt *InstancePtr);

/*
 * Interface functions in xscuwdt.c
 */
s32 XScuWdt_CfgInitialize(XScuWdt *InstancePtr,
			  XScuWdt_Config *ConfigPtr, u32 EffectiveAddress);

void XScuWdt_Start(XScuWdt *InstancePtr);

void XScuWdt_Stop(XScuWdt *InstancePtr);


#ifdef __cplusplus
}
#endif

/** @} */
#endif	/* end of protection macro */
