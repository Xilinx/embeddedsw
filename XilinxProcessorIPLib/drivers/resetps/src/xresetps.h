/******************************************************************************
* Copyright (C) 2017 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XRESETPS_H
#define XRESETPS_H

/*****************************************************************************/
/**
*
* @file xresetps.h
* @addtogroup resetps Overview
* @{
* @brief Defines the interface of the XResetPs reset controller driver.
*
* @details
*
* The Xilinx Reset Controller driver supports the following features:
*   - Assert reset for a specific peripheral.
*   - Deassert reset for a specific peripheral.
*   - Pulse reset for a specific peripheral.
*   - Get reset status for a specific peripheral.
*
* This driver is intended to be real-time operating system (RTOS) and
* processor independent. It works with physical addresses only. The layer
* above this driver must satisfy any needs for dynamic memory management,
* threads or thread mutual exclusion, virtual memory, or cache control.
*
* <pre>
* MODIFICATION HISTORY:
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00  cjp    09/05/17 First release
* 1.1   Nava   04/20/18 Fixed compilation warnings.
* 1.2   cjp    04/27/18 Updated for clockps interdependency
* 1.2   sd     07/20/18 Fixed Doxygen Reported warnings.
* 1.4   sk     09/01/20 Updated the Makefile to support parallel make
*                       execution.
* 1.5   sk     11/30/21 Fix compilation warnings reported with "-Wundef" flag.
* 1.6   sd     07/07/23 Added SDT support.
* 1.7   sd     11/07/23 Remove the unused pss ref frequency
* 1.8   vlt    12/30/25 Update Doxygen comments to include SDT flow details.
* 1.8   vlt    03/14/26 Updated BaseAddress type from u32 to UINTPTR
*                       to support 64-bit addressing.
* 1.9   vn     09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xresetps_hw.h"

#if defined (__aarch64__)
#include "xil_smc.h"
#endif

/************************** Constant Definitions *****************************/
/** @name Device ID and instance defines
 *
 * Defines the device ID and number of instances for resetps. Resetps uses
 * hardware that is common with another driver, so these wrapper defines are
 * required.
 * @{
 */
#ifndef SDT
/** Number of reset instances. */
#define XPAR_XRESETPS_NUM_INSTANCES    (XPAR_XCRPSU_NUM_INSTANCES)
/** Reset device ID. */
#define XPAR_XRESETPS_DEVICE_ID        (XPAR_XCRPSU_0_DEVICE_ID)
#endif
/** @} */

/** @name Supported reset action constants
 *
 * Defines constants for supported and unsupported reset actions.
 * @{
 */
#define XRESETPS_SUP                   1 /**< Reset action supported. */
#define XRESETPS_NOSUP                 0 /**< Reset action not supported. */
/** @} */

/**************************** Type Definitions *******************************/
/**
 * @brief Holds configuration information for the device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;                    /**< Unique ID of the device. */
#else
	char *Name;                     /**< Name of the device. */
	UINTPTR BaseAddress;                /**< Base address of the device. */
#endif
} XResetPs_Config;

/**
 * @brief Holds the XResetPs driver instance data.
 *
 * Allocate a variable of this type for every reset controller device in the
 * system. Pass a pointer to a variable of this type to the driver API
 * functions.
 */
typedef struct {
	XResetPs_Config Config;          /**< Hardware configuration. */
} XResetPs;

/**
 * @brief Defines the type of pulse reset to execute for peripherals.
 *
 * Three types of pulse reset are possible:
 *   1. Pulse with no delay and no power state validation.
 *   2. Pulse with delay but no power state validation.
 *   3. Pulse with delay and power state validation.
 */
typedef enum {
	XRESETPS_PT_NO_DLY_NO_PSCHK,     /**< No delay, no power state check. */
	XRESETPS_PT_DLY_NO_PSCHK,        /**< Delay, no power state check. */
	XRESETPS_PT_DLY_PSCHK,           /**< Delay, power state check. */
	XRESETPS_PT_INVALID,             /**< Invalid pulse type. */
} XResetPs_PulseTypes;

/**
 * @brief Defines reset actions on the peripherals.
 */
typedef enum {
	XRESETPS_RSTACT_RELEASE,         /**< Release reset. */
	XRESETPS_RSTACT_ASSERT,          /**< Assert reset. */
	XRESETPS_RSTACT_PULSE,           /**< Pulse reset. */
} XresetPs_ResetAction;

/**
 * @brief Defines reset IDs of peripherals mapped to PMUFW reset IDs.
 *
 * These reset IDs are not offset by 1000 and are relative.
 */
typedef enum {
	XRESETPS_RSTID_START,            /**< First reset ID. */
	XRESETPS_RSTID_PCIE_CFG = XRESETPS_RSTID_START, /**< PCIe CFG reset. */
	XRESETPS_RSTID_PCIE_BRIDGE,      /**< PCIe bridge reset. */
	XRESETPS_RSTID_PCIE_CTRL,        /**< PCIe CTRL reset. */
	XRESETPS_RSTID_DP,               /**< DP reset. */
	XRESETPS_RSTID_SWDT_CRF,         /**< SWDT CRF reset. */
	XRESETPS_RSTID_AFI_FM5,          /**< AFI FM5 reset. */
	XRESETPS_RSTID_AFI_FM4,          /**< AFI FM4 reset. */
	XRESETPS_RSTID_AFI_FM3,          /**< AFI FM3 reset. */
	XRESETPS_RSTID_AFI_FM2,          /**< AFI FM2 reset. */
	XRESETPS_RSTID_AFI_FM1,          /**< AFI FM1 reset. */
	XRESETPS_RSTID_AFI_FM0,          /**< AFI FM0 reset. */
	XRESETPS_RSTID_GDMA,             /**< GDMA reset. */
	XRESETPS_RSTID_GPU_PP1,          /**< GPU PP1 reset. */
	XRESETPS_RSTID_GPU_PP0,          /**< GPU PP0 reset. */
	XRESETPS_RSTID_GPU,              /**< GPU reset. */
	XRESETPS_RSTID_GT,               /**< GT reset. */
	XRESETPS_RSTID_SATA,             /**< SATA reset. */
	XRESETPS_RSTID_ACPU3_PWRON,      /**< ACPU3 power-on reset. */
	XRESETPS_RSTID_ACPU2_PWRON,      /**< ACPU2 power-on reset. */
	XRESETPS_RSTID_ACPU1_PWRON,      /**< ACPU1 power-on reset. */
	XRESETPS_RSTID_ACPU0_PWRON,      /**< ACPU0 power-on reset. */
	XRESETPS_RSTID_APU_L2,           /**< APU L2 reset. */
	XRESETPS_RSTID_ACPU3,            /**< ACPU3 reset. */
	XRESETPS_RSTID_ACPU2,            /**< ACPU2 reset. */
	XRESETPS_RSTID_ACPU1,            /**< ACPU1 reset. */
	XRESETPS_RSTID_ACPU0,            /**< ACPU0 reset. */
	XRESETPS_RSTID_DDR,              /**< DDR reset. */
	XRESETPS_RSTID_APM_FPD,          /**< FPD APM reset. */
	XRESETPS_RSTID_SOFT,             /**< Soft reset. */
	XRESETPS_RSTID_GEM0,             /**< GEM0 reset. */
	XRESETPS_RSTID_GEM1,             /**< GEM1 reset. */
	XRESETPS_RSTID_GEM2,             /**< GEM2 reset. */
	XRESETPS_RSTID_GEM3,             /**< GEM3 reset. */
	XRESETPS_RSTID_QSPI,             /**< QSPI reset. */
	XRESETPS_RSTID_UART0,            /**< UART0 reset. */
	XRESETPS_RSTID_UART1,            /**< UART1 reset. */
	XRESETPS_RSTID_SPI0,             /**< SPI0 reset. */
	XRESETPS_RSTID_SPI1,             /**< SPI1 reset. */
	XRESETPS_RSTID_SDIO0,            /**< SDIO0 reset. */
	XRESETPS_RSTID_SDIO1,            /**< SDIO1 reset. */
	XRESETPS_RSTID_CAN0,             /**< CAN0 reset. */
	XRESETPS_RSTID_CAN1,             /**< CAN1 reset. */
	XRESETPS_RSTID_I2C0,             /**< I2C0 reset. */
	XRESETPS_RSTID_I2C1,             /**< I2C1 reset. */
	XRESETPS_RSTID_TTC0,             /**< TTC0 reset. */
	XRESETPS_RSTID_TTC1,             /**< TTC1 reset. */
	XRESETPS_RSTID_TTC2,             /**< TTC2 reset. */
	XRESETPS_RSTID_TTC3,             /**< TTC3 reset. */
	XRESETPS_RSTID_SWDT_CRL,         /**< SWDT CRL reset. */
	XRESETPS_RSTID_NAND,             /**< NAND reset. */
	XRESETPS_RSTID_ADMA,             /**< ADMA reset. */
	XRESETPS_RSTID_GPIO,             /**< GPIO reset. */
	XRESETPS_RSTID_IOU_CC,           /**< IOU CC reset. */
	XRESETPS_RSTID_TIMESTAMP,        /**< Timestamp reset. */
	XRESETPS_RSTID_RPU_R50,          /**< RPU R50 reset. */
	XRESETPS_RSTID_RPU_R51,          /**< RPU R51 reset. */
	XRESETPS_RSTID_RPU_AMBA,         /**< RPU AMBA reset. */
	XRESETPS_RSTID_OCM,              /**< OCM reset. */
	XRESETPS_RSTID_RPU_PGE,          /**< RPU PGE reset. */
	XRESETPS_RSTID_USB0_CORERESET,   /**< USB0 core reset. */
	XRESETPS_RSTID_USB1_CORERESET,   /**< USB1 core reset. */
	XRESETPS_RSTID_USB0_HIBERRESET,  /**< USB0 hibernation reset. */
	XRESETPS_RSTID_USB1_HIBERRESET,  /**< USB1 hibernation reset. */
	XRESETPS_RSTID_USB0_APB,         /**< USB0 APB reset. */
	XRESETPS_RSTID_USB1_APB,         /**< USB1 APB reset. */
	XRESETPS_RSTID_IPI,              /**< IPI reset. */
	XRESETPS_RSTID_APM_LPD,          /**< LPD APM reset. */
	XRESETPS_RSTID_RTC,              /**< RTC reset. */
	XRESETPS_RSTID_SYSMON,           /**< SYSMON reset. */
	XRESETPS_RSTID_AFI_FM6,          /**< AFI FM6 reset. */
	XRESETPS_RSTID_LPD_SWDT,         /**< LPD SWDT reset. */
	XRESETPS_RSTID_FPD,              /**< FPD reset. */
	XRESETPS_RSTID_RPU_DBG1,         /**< RPU debug 1 reset. */
	XRESETPS_RSTID_RPU_DBG0,         /**< RPU debug 0 reset. */
	XRESETPS_RSTID_DBG_LPD,          /**< LPD debug reset. */
	XRESETPS_RSTID_DBG_FPD,          /**< FPD debug reset. */
	XRESETPS_RSTID_APLL,             /**< APLL reset. */
	XRESETPS_RSTID_DPLL,             /**< DPLL reset. */
	XRESETPS_RSTID_VPLL,             /**< VPLL reset. */
	XRESETPS_RSTID_IOPLL,            /**< IOPLL reset. */
	XRESETPS_RSTID_RPLL,             /**< RPLL reset. */
	XRESETPS_RSTID_GPO3PL0,          /**< GPO3 PL0 reset. */
	XRESETPS_RSTID_GPO3PL1,          /**< GPO3 PL1 reset. */
	XRESETPS_RSTID_GPO3PL2,          /**< GPO3 PL2 reset. */
	XRESETPS_RSTID_GPO3PL3,          /**< GPO3 PL3 reset. */
	XRESETPS_RSTID_GPO3PL4,          /**< GPO3 PL4 reset. */
	XRESETPS_RSTID_GPO3PL5,          /**< GPO3 PL5 reset. */
	XRESETPS_RSTID_GPO3PL6,          /**< GPO3 PL6 reset. */
	XRESETPS_RSTID_GPO3PL7,          /**< GPO3 PL7 reset. */
	XRESETPS_RSTID_GPO3PL8,          /**< GPO3 PL8 reset. */
	XRESETPS_RSTID_GPO3PL9,          /**< GPO3 PL9 reset. */
	XRESETPS_RSTID_GPO3PL10,         /**< GPO3 PL10 reset. */
	XRESETPS_RSTID_GPO3PL11,         /**< GPO3 PL11 reset. */
	XRESETPS_RSTID_GPO3PL12,         /**< GPO3 PL12 reset. */
	XRESETPS_RSTID_GPO3PL13,         /**< GPO3 PL13 reset. */
	XRESETPS_RSTID_GPO3PL14,         /**< GPO3 PL14 reset. */
	XRESETPS_RSTID_GPO3PL15,         /**< GPO3 PL15 reset. */
	XRESETPS_RSTID_GPO3PL16,         /**< GPO3 PL16 reset. */
	XRESETPS_RSTID_GPO3PL17,         /**< GPO3 PL17 reset. */
	XRESETPS_RSTID_GPO3PL18,         /**< GPO3 PL18 reset. */
	XRESETPS_RSTID_GPO3PL19,         /**< GPO3 PL19 reset. */
	XRESETPS_RSTID_GPO3PL20,         /**< GPO3 PL20 reset. */
	XRESETPS_RSTID_GPO3PL21,         /**< GPO3 PL21 reset. */
	XRESETPS_RSTID_GPO3PL22,         /**< GPO3 PL22 reset. */
	XRESETPS_RSTID_GPO3PL23,         /**< GPO3 PL23 reset. */
	XRESETPS_RSTID_GPO3PL24,         /**< GPO3 PL24 reset. */
	XRESETPS_RSTID_GPO3PL25,         /**< GPO3 PL25 reset. */
	XRESETPS_RSTID_GPO3PL26,         /**< GPO3 PL26 reset. */
	XRESETPS_RSTID_GPO3PL27,         /**< GPO3 PL27 reset. */
	XRESETPS_RSTID_GPO3PL28,         /**< GPO3 PL28 reset. */
	XRESETPS_RSTID_GPO3PL29,         /**< GPO3 PL29 reset. */
	XRESETPS_RSTID_GPO3PL30,         /**< GPO3 PL30 reset. */
	XRESETPS_RSTID_GPO3PL31,         /**< GPO3 PL31 reset. */
	XRESETPS_RSTID_RPU_LS,           /**< RPU LS reset. */
	XRESETPS_RSTID_PS_ONLY,          /**< PS-only reset. */
	XRESETPS_RSTID_PL,               /**< PL reset. */
	XRESETPS_RSTID_END = XRESETPS_RSTID_PL, /**< Last reset ID. */
} XResetPs_RstId;

/**
 * @brief Defines possible values for reset status of peripherals.
 */
typedef enum {
	XRESETPS_RESETRELEASED,          /**< Reset is released. */
	XRESETPS_RESETASSERTED           /**< Reset is asserted. */
} XResetPs_RstStatus;

/***************** Macros (Inline Functions) Definitions *********************/
/*****************************************************************************/
/**
*
* @brief Sets the supported reset actions.
*
* @param[in]   ResetSupport Indicates whether reset status check is supported.
* @param[in]   PulseSupport Indicates whether pulse reset action is supported.
* @param[in]   AssertSupport Indicates whether reset assert and deassert are
*              supported.
*
* @return      Supported reset actions.
*
* @note        Bit fields decide the supported actions as follows: BIT 1 is
*              assert/deassert, BIT 2 is pulse, and BIT 3 is reset status.
*              A set bit indicates that the corresponding action is supported,
*              and vice versa.
*              <br><br>
*              C-style signature:
*              u8 XRESETPS_SUPPORTED_ACT(u8 ResetSupport, u8 PulseSupport,
*                                        u8 AssertSupport)
*
******************************************************************************/
#define XRESETPS_SUPPORTED_ACT(ResetSupport, PulseSupport, AssertSupport) \
	((ResetSupport << 2) | (PulseSupport << 1) | AssertSupport)

/*****************************************************************************/
/**
*
* @brief Checks whether assert/deassert reset is supported.
*
* @param[in]   Actions Supported reset actions.
*
* @return      1 if supported, or 0 if not supported.
*
* @note        C-style signature:
*              u8 XRESETPS_CHK_ASSERT_SUPPORT(u8 Actions)
*
******************************************************************************/
#define XRESETPS_CHK_ASSERT_SUPPORT(Actions) ((Actions & 0x1))

/*****************************************************************************/
/**
*
* @brief Checks whether pulse reset is supported.
*
* @param[in]   Actions Supported reset actions.
*
* @return      1 if supported, or 0 if not supported.
*
* @note        C-style signature:
*              u8 XRESETPS_CHK_PULSE_SUPPORT(u8 Actions)
*
******************************************************************************/
#define XRESETPS_CHK_PULSE_SUPPORT(Actions)  ((Actions & 0x2) >> 1)

/*****************************************************************************/
/**
*
* @brief Checks whether reset status check is supported.
*
* @param[in]   Actions Supported reset actions.
*
* @return      1 if supported, or 0 if not supported.
*
* @note        C-style signature:
*              u8 XRESETPS_CHK_STATUS_SUPPORT(u8 Actions)
*
******************************************************************************/
#define XRESETPS_CHK_STATUS_SUPPORT(Actions) ((Actions & 0x4) >> 2)

/*****************************************************************************/
/**
*
* @brief Reads the given register.
*
* @param[in]   RegAddress Address of the register to read.
*
* @return      The 32-bit value of the register.
*
* @note        C-style signature:
*              u32 XResetPs_ReadReg(u32 RegAddress)
*
******************************************************************************/
#define XResetPs_ReadReg(RegAddress) \
	Xil_In32((u32)RegAddress)

/*****************************************************************************/
/**
*
* @brief Writes the given register.
*
* @param[in]   RegAddress Address of the register to write.
* @param[in]   Data 32-bit value to write to the register.
*
* @note        C-style signature:
*              void XResetPs_WriteReg(u32 RegAddress, u32 Data)
*
******************************************************************************/
#define XResetPs_WriteReg(RegAddress, Data) \
	Xil_Out32((u32)RegAddress, (u32)Data)

/************************** Function Prototypes ******************************/

/*
 * Lookup configuration in xresetps_sinit.c.
 */
#ifndef SDT
XResetPs_Config *XResetPs_LookupConfig(u16 DeviceId);
#else
XResetPs_Config *XResetPs_LookupConfig(UINTPTR BaseAddress);
#endif

/*
 * Interface functions in xresetps.c
 */
XStatus XResetPs_CfgInitialize(XResetPs *InstancePtr,
			       XResetPs_Config *ConfigPtr);
XStatus XResetPs_ResetAssert(XResetPs *InstancePtr,
			     const XResetPs_RstId ResetID);
XStatus XResetPs_ResetDeassert(XResetPs *InstancePtr,
			       const XResetPs_RstId ResetID);
XStatus XResetPs_ResetPulse(XResetPs *InstancePtr,
			    const XResetPs_RstId ResetID);
XStatus XResetPs_ResetStatus(XResetPs *InstancePtr,
			     const XResetPs_RstId ResetID, XResetPs_RstStatus *Status);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* end of protection macro */
