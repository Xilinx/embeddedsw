/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XGPIOPS_H
#define XGPIOPS_H

/*****************************************************************************/
/**
*
* @file xgpiops.h
* @addtogroup gpiops Overview
* @{
* @brief Defines the software API for the Xilinx PS GPIO driver.
*
* @details
*
* The Xilinx PS GPIO driver supports the Xilinx PS GPIO Controller.
*
* The GPIO Controller supports the following features:
* - Six banks
* - Masked writes (there are no masked reads)
* - Bypass mode
* - Configurable Interrupts (Level/Edge)
*
* This driver is intended to be RTOS and processor independent. The layer
* above this driver must satisfy any needs for dynamic memory management,
* threads or thread mutual exclusion, virtual memory, or cache control.
*
* The following lists show the GPIO bank and pin mapping for each platform.
*
* - Zynq: PS GPIO contains four banks and 118 pins.
*   - Bank 0 MIO pins 0 - 31
*   - Bank 1 MIO pins 32 - 53
*   - Bank 2 MIO pins 54 - 85
*   - Bank 3 EMIO signals 86 - 117
* - Zynq Ultrascale+ MP: PS GPIO contains six banks and 174 pins.
*   - Bank 0 MIO pins 0 - 25
*   - Bank 1 MIO pins 26 - 51
*   - Bank 2 MIO pins 52 - 77
*   - Bank 3 EMIO signals 78 - 109
*   - Bank 4 EMIO signals 110 - 141
*   - Bank 5 EMIO signals 142 - 173
* - Versal: Two GPIOPS instances, PMC GPIO and PS GPIO.
*   - PMC GPIO contains four banks and 116 pins.
*     - Bank 0 MIO pins 0 - 25
*     - Bank 1 MIO pins 26 - 51
*     - Bank 3 EMIO signals 52 - 83
*     - Bank 4 EMIO signals 84 - 115
*   - PS GPIO contains two banks and 58 pins.
*     - Bank 0 MIO pins 0 - 25
*     - Bank 3 EMIO signals 26 - 57
*
* This driver supports all the features listed above, if applicable.
*
* <b>Driver Description</b>
*
* The device driver enables higher layer software (for example, an
* application) to communicate with the GPIO.
*
* <b>Interrupts</b>
*
* The driver provides interrupt management functions and an interrupt handler.
* You need to provide callback functions when you use this driver. An
* interrupt handler example is available with the driver.
*
* <b>Threads</b>
*
* This driver is not thread safe. The layer above this driver must satisfy
* any needs for threads or thread mutual exclusion.
*
* <b>Asserts</b>
*
* All Xilinx drivers use asserts to enforce constraints on argument values.
* To turn off asserts on a system-wide basis, define the NDEBUG identifier at
* compile time. By default, asserts are turned on. It is recommended that you
* leave asserts on during development.
*
* <b>Building the driver</b>
*
* The XGpioPs driver is composed of several source files. This structure
* allows you to build and link only those parts of the driver that are
* necessary.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   01/15/10 First Release
* 1.01a sv   04/15/12 Removed the APIs XGpioPs_SetMode, XGpioPs_SetModePin
*                     XGpioPs_GetMode, XGpioPs_GetModePin as they are not
*		      relevant to Zynq device.The interrupts are disabled
*		      for output pins on all banks during initialization.
* 1.02a hk   08/22/13 Added low level reset API
* 2.1   hk   04/29/14 Use Input data register DATA_RO for read. CR# 771667.
* 2.2	sk	 10/13/14 Used Pin number in Bank instead of pin number
* 					  passed to APIs. CR# 822636
* 3.00  kvn  02/13/15 Modified code for MISRA-C:2012 compliance.
* 3.1	kvn  04/13/15 Add support for Zynq Ultrascale+ MP. CR# 856980.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
*       ms   04/05/17 Added tabspace for return statements in functions of
*                     gpiops examples for proper documentation while
*                     generating doxygen.
* 3.3   ms   04/17/17 Added notes about gpio input and output pin description
*                     for zcu102 and zc702 boards in polled and interrupt
*                     example, configured Interrupt pin to input pin for
*                     proper functioning of interrupt example.
* 3.4   aru  08/17/18 Resolved MISRA-C mandatory violations. CR# 1007751
* 3.5   sne  03/01/19 Fixes violations according to MISRAC-2012
*                     in safety mode and modified the code such as
*                     Use of mixed mode arithmetic,Declared the pointer param
*                     as Pointer to const,Casting operation to a pointer,
*                     Literal value requires a U suffix.
* 3.5   sne  03/14/19 Added Versal support.
* 3.6   mus  04/05/19 Replaced XPLAT_versal macro with XPLAT_VERSAL, to be in
*                     sync with standalone BSP
* 3.6	sne  06/12/19 Fixed IAR compiler warning.
* 3.6   sne  08/14/19 Added interrupt handler support on versal.
* 3.7	sne  12/04/19 Reverted versal examples support.
* 3.8   sne  08/28/20 Modify Makefile to support parallel make execution.
* 3.8	sne  09/17/20 Added description for Versal PS and PMC GPIO pins.
* 3.9	sne  03/15/21 Fixed MISRA-C violations.
* 3.11  sg   02/23/23 Update bank and pin mapping information.
* 3.12  gm   07/11/23 Added SDT support.
* 3.13  gm   03/15/24 Remove const of XGpioPs InstancePtr from function proto
*                     type of XGpioPs_IntrEnable, XGpioPs_IntrDisable,
*                     XGpioPs_IntrEnablePin and XGpioPs_IntrDisablePin
*                     to add multi-core interrupt support.
* 3.14  bkv  02/20/25 Changed data type of effective address from u32 to UINTPTR
*                     to support both 32-bit and 64-bit platforms.
* 3.15  vlt  12/30/25 Update Doxygen comments to include SDT flow details.
* 3.15  vlt  03/14/26 Updated BaseAddress type from u32 to UINTPTR
*                     to support 64-bit addressing.
* 3.16  vn   09/10/26 Align Doxygen comments with AMD standards
*
* </pre>
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xgpiops_hw.h"
#include "xplatform_info.h"

/************************** Constant Definitions *****************************/

/** @name Interrupt types
 *
 * The following constants define the interrupt types that you can set for
 * each GPIO pin.
 * @{
 */
/** Interrupt on rising edge. */
#define XGPIOPS_IRQ_TYPE_EDGE_RISING	0x00U
/** Interrupt on falling edge. */
#define XGPIOPS_IRQ_TYPE_EDGE_FALLING	0x01U
/** Interrupt on both edges. */
#define XGPIOPS_IRQ_TYPE_EDGE_BOTH	0x02U
/** Interrupt on high level. */
#define XGPIOPS_IRQ_TYPE_LEVEL_HIGH	0x03U
/** Interrupt on low level. */
#define XGPIOPS_IRQ_TYPE_LEVEL_LOW	0x04U
/** @}*/

/** Maximum number of pins in a GPIO bank. */
#define XGPIOPS_BANK_MAX_PINS		(u32)32
#define XGPIOPS_BANK0			0x00U  /**< GPIO bank 0. */
#define XGPIOPS_BANK1			0x01U  /**< GPIO bank 1. */
#define XGPIOPS_BANK2			0x02U  /**< GPIO bank 2. */
#define XGPIOPS_BANK3			0x03U  /**< GPIO bank 3. */
#define XGPIOPS_BANK4			0x04U  /**< GPIO bank 4. */
#define XGPIOPS_BANK5			0x05U  /**< GPIO bank 5. */

/** Maximum number of banks in a Zynq Ultrascale+ MP GPIO device. */
#define XGPIOPS_MAX_BANKS_ZYNQMP		0x06U
/** Maximum number of banks in a Zynq GPIO device. */
#define XGPIOPS_MAX_BANKS		0x04U
/** Maximum number of banks across all platforms. */
#define XGPIOPS_MAX_BANKS_CNT		0x06U

/**
 * Maximum number of pins in the Zynq Ultrascale+ MP GPIO device.
 *
 * Pins 0 - 25 are in bank 0, 26 - 51 in bank 1, 52 - 77 in bank 2,
 * 78 - 109 in bank 3, 110 - 141 in bank 4, and 142 - 173 in bank 5.
 */
#define XGPIOPS_DEVICE_MAX_PIN_NUM_ZYNQMP	(u32)174
/**
 * Maximum number of pins in the Zynq GPIO device.
 *
 * Pins 0 - 31 are in bank 0, 32 - 53 in bank 1, 54 - 85 in bank 2, and
 * 86 - 117 in bank 3.
 */
#define XGPIOPS_DEVICE_MAX_PIN_NUM	(u32)118

/**************************** Type Definitions *******************************/

/*****************************************************************************/
/**
 * @brief GPIO interrupt handler callback type.
 *
 * This handler data type allows you to define a callback function that
 * handles the interrupts for the GPIO device. The application that uses this
 * driver is expected to define a handler of this type to support interrupt
 * driven mode. The handler executes in an interrupt context, so keep the
 * processing it performs to a minimum.
 *
 * @param[in]   CallBackRef Callback reference passed in by the upper layer
 *              when it sets the callback functions for a GPIO bank. The
 *              driver passes it back to the upper layer when it invokes the
 *              callback. Its type is not important to the driver component,
 *              so it is a void pointer.
 * @param[in]   Bank Bank for which the interrupt status has changed.
 * @param[in]   Status Interrupt status of the GPIO bank.
 *
 *****************************************************************************/
typedef void (*XGpioPs_Handler) (void *CallBackRef, u32 Bank, u32 Status);

/**
 * @brief Holds configuration information for a device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID of the device. */
#else
	char *Name;             /**< Name of the device. */
#endif
	UINTPTR BaseAddr;		/**< Register base address. */
#ifdef SDT
	u16 IntrId;		/**< Interrupt ID in bits[11:0], and trigger
				  * type and level flags in bits[15:12]. */
	UINTPTR IntrParent;	/**< Interrupt parent type in bit[0], and
				  * parent base address in bits[64/32:1]. */
#endif
} XGpioPs_Config;

/**
 * @brief XGpioPs driver instance data.
 *
 * Allocate a variable of this type for the GPIO device in the system. Pass a
 * pointer to a variable of this type to the driver API functions.
 */
typedef struct {
	XGpioPs_Config GpioConfig;	/**< Device configuration. */
	u32 IsReady;			/**< Device is initialized and ready. */
	XGpioPs_Handler Handler;	/**< Status handlers for all banks. */
	void *CallBackRef;	/**< Callback reference for bank handlers. */
	u32 Platform;			/**< Platform data. */
	u32 MaxPinNum;			/**< Maximum number of pins in the GPIO
					  * device. */
	u8 MaxBanks;			/**< Maximum number of banks in the GPIO
					  * device. */
	u32 PmcGpio;		/**< Flag for accessing PS GPIO for Versal. */
	u32 CoreIntrMask[XGPIOPS_MAX_BANKS_CNT]; /**< Interrupt mask
						   * per core. */
} XGpioPs;

/************************** Variable Definitions *****************************/
/**
 * @brief Holds the configuration entry for each GPIO instance.
 */
extern XGpioPs_Config XGpioPs_ConfigTable[];

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/* Functions in xgpiops.c */
s32 XGpioPs_CfgInitialize(XGpioPs *InstancePtr, const XGpioPs_Config *ConfigPtr,
			   UINTPTR EffectiveAddr);

/* Bank APIs in xgpiops.c */
u32 XGpioPs_Read(const XGpioPs *InstancePtr, u8 Bank);
void XGpioPs_Write(const XGpioPs *InstancePtr, u8 Bank, u32 Data);
void XGpioPs_SetDirection(const XGpioPs *InstancePtr, u8 Bank, u32 Direction);
u32 XGpioPs_GetDirection(const XGpioPs *InstancePtr, u8 Bank);
void XGpioPs_SetOutputEnable(const XGpioPs *InstancePtr, u8 Bank, u32 OpEnable);
u32 XGpioPs_GetOutputEnable(const XGpioPs *InstancePtr, u8 Bank);
#ifdef versal
void XGpioPs_GetBankPin(const XGpioPs *InstancePtr,u8 PinNumber,u8 *BankNumber, u8 *PinNumberInBank);
#else
void XGpioPs_GetBankPin(u8 PinNumber,u8 *BankNumber, u8 *PinNumberInBank);
#endif

/* Pin APIs in xgpiops.c */
u32 XGpioPs_ReadPin(const XGpioPs *InstancePtr, u32 Pin);
void XGpioPs_WritePin(const XGpioPs *InstancePtr, u32 Pin, u32 Data);
void XGpioPs_SetDirectionPin(const XGpioPs *InstancePtr, u32 Pin, u32 Direction);
u32 XGpioPs_GetDirectionPin(const XGpioPs *InstancePtr, u32 Pin);
void XGpioPs_SetOutputEnablePin(const XGpioPs *InstancePtr, u32 Pin, u32 OpEnable);
u32 XGpioPs_GetOutputEnablePin(const XGpioPs *InstancePtr, u32 Pin);

/* Diagnostic functions in xgpiops_selftest.c */
s32 XGpioPs_SelfTest(XGpioPs *InstancePtr);

/* Functions in xgpiops_intr.c */
/* Bank APIs in xgpiops_intr.c */
void XGpioPs_IntrEnable(XGpioPs *InstancePtr, u8 Bank, u32 Mask);
void XGpioPs_IntrDisable(XGpioPs *InstancePtr, u8 Bank, u32 Mask);
u32 XGpioPs_IntrGetEnabled(const XGpioPs *InstancePtr, u8 Bank);
u32 XGpioPs_IntrGetStatus(const XGpioPs *InstancePtr, u8 Bank);
void XGpioPs_IntrClear(const XGpioPs *InstancePtr, u8 Bank, u32 Mask);
void XGpioPs_SetIntrType(const XGpioPs *InstancePtr, u8 Bank, u32 IntrType,
			  u32 IntrPolarity, u32 IntrOnAny);
void XGpioPs_GetIntrType(const XGpioPs *InstancePtr, u8 Bank, u32 *IntrType,
			  u32 *IntrPolarity, u32 *IntrOnAny);
void XGpioPs_SetCallbackHandler(XGpioPs *InstancePtr, void *CallBackRef,
			     XGpioPs_Handler FuncPointer);
void XGpioPs_IntrHandler(const XGpioPs *InstancePtr);

/* Pin APIs in xgpiops_intr.c */
void XGpioPs_SetIntrTypePin(const XGpioPs *InstancePtr, u32 Pin, u8 IrqType);
u8 XGpioPs_GetIntrTypePin(const XGpioPs *InstancePtr, u32 Pin);

void XGpioPs_IntrEnablePin(XGpioPs *InstancePtr, u32 Pin);
void XGpioPs_IntrDisablePin(XGpioPs *InstancePtr, u32 Pin);
u32 XGpioPs_IntrGetEnabledPin(const XGpioPs *InstancePtr, u32 Pin);
u32 XGpioPs_IntrGetStatusPin(const XGpioPs *InstancePtr, u32 Pin);
void XGpioPs_IntrClearPin(const XGpioPs *InstancePtr, u32 Pin);

/* Functions in xgpiops_sinit.c */
#ifndef SDT
XGpioPs_Config *XGpioPs_LookupConfig(u16 DeviceId);
#else
XGpioPs_Config *XGpioPs_LookupConfig(UINTPTR BaseAddress);
#endif
#ifdef __cplusplus
}
#endif

/** @} */
#endif /* end of protection macro */
